#include "http.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>

static const char *reason(int status)
{
    switch (status) {
    case 200: return "OK";
    case 400: return "Bad Request";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 408: return "Request Timeout";
    case 413: return "Content Too Large";
    case 417: return "Expectation Failed";
    case 431: return "Request Header Fields Too Large";
    case 501: return "Not Implemented";
    default: return "Internal Server Error";
    }
}

HttpResponse http_text(int status, const char *text)
{
    return (HttpResponse){status, "text/plain; charset=utf-8", text, strlen(text)};
}

static int receive(int fd, char *buffer, size_t length)
{
    ssize_t n;
    do {
        n = recv(fd, buffer, length, 0);
    } while (n < 0 && errno == EINTR); /* 被信号打断时重试，而非当作网络错误。 */
    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return -408;
    return n <= 0 ? -1 : (int)n;
}

int http_read_request(int fd, char *buffer, size_t capacity, HttpRequest *request)
{
    size_t used = 0;
    char *end = NULL;
    memset(request, 0, sizeof(*request));
    if (capacity < HTTP_HEADER_LIMIT + HTTP_BODY_LIMIT + 1) return 500;

    /* TCP 没有请求边界，一次 recv 不一定得到完整的 HTTP 头。 */
    while (end == NULL) {
        if (used == HTTP_HEADER_LIMIT) return 431;
        int n = receive(fd, buffer + used, HTTP_HEADER_LIMIT - used);
        if (n < 0) return n == -408 ? 408 : -1;
        used += (size_t)n;
        /* 加终止符供字符串函数查找头部；额外预留的一个字节用于这里。 */
        buffer[used] = '\0';
        end = strstr(buffer, "\r\n\r\n"); /* 空行分隔头部和正文。 */
        /* 零字节不能出现在头部，但正文允许包含任意二进制数据。 */
        size_t text_length = end ? (size_t)(end - buffer) + 4 : used;
        if (memchr(buffer, '\0', text_length) != NULL) return 400;
    }
    size_t header_length = (size_t)(end - buffer) + 4;
    char *line_end = strstr(buffer, "\r\n");
    if (line_end == NULL) return 400;
    /* 就地分割请求行，例如 POST /echo HTTP/1.1；不额外复制字符串。 */
    *line_end = '\0';
    char version[16], extra;
    /* 字段宽度限制写入长度；extra 用来拒绝请求行中的第四个字段。 */
    if (sscanf(buffer, "%15s %1023s %15s %c", request->method,
               request->path, version, &extra) != 3) return 400;
    if (strcmp(version, "HTTP/1.1") != 0 && strcmp(version, "HTTP/1.0") != 0)
        return 400;
    if (request->path[0] != '/') return 400;

    size_t content_length = 0;
    int has_length = 0, has_host = 0;
    char *line = line_end + 2;
    /* 逐行解析 Name: Value；头部字段名比较不区分大小写。 */
    while (line < end) {
        line_end = strstr(line, "\r\n");
        if (line_end == NULL) return 400;
        *line_end = '\0';
        char *colon = strchr(line, ':');
        if (colon == NULL || colon == line) return 400;
        for (char *p = line; p < colon; ++p) {
            if (!isalnum((unsigned char)*p) && strchr("!#$%&'*+-.^_`|~", *p) == NULL)
                return 400;
        }
        *colon = '\0';
        char *value = colon + 1;
        while (*value == ' ' || *value == '\t') ++value;
        char *tail = value + strlen(value);
        while (tail > value && (tail[-1] == ' ' || tail[-1] == '\t')) *--tail = '\0';
        /* 重复长度会让正文边界含糊；这里只接受一个非负十进制长度。 */
        if (strcasecmp(line, "Content-Length") == 0) {
            if (has_length || *value == '\0') return 400;
            has_length = 1;
            for (char *p = value; *p; ++p) {
                if (!isdigit((unsigned char)*p)) return 400;
                content_length = content_length * 10 + (size_t)(*p - '0');
                if (content_length > HTTP_BODY_LIMIT) return 413;
            }
        } else if (strcasecmp(line, "Transfer-Encoding") == 0) {
            return 501; /* 本 demo 只通过 Content-Length 确定正文长度。 */
        } else if (strcasecmp(line, "Expect") == 0) {
            return 417;
        } else if (strcasecmp(line, "Host") == 0) {
            if (has_host || *value == '\0') return 400;
            has_host = 1;
        }
        line = line_end + 2;
    }
    if (strcmp(version, "HTTP/1.1") == 0 && !has_host) return 400;
    size_t total = header_length + content_length;
    /* 读头部时可能已收到部分正文，只继续读取尚缺的字节。 */
    while (used < total) {
        int n = receive(fd, buffer + used, total - used);
        if (n < 0) return n == -408 ? 408 : 400;
        used += (size_t)n;
    }
    request->body = buffer + header_length;
    request->body_length = content_length;
    return 0;
}

static int send_all(int fd, const char *data, size_t length)
{
    /* send 可能只发送一部分；循环推进数据指针，直到全部发送完。 */
    while (length > 0) {
        /* 对端断开时返回错误，MSG_NOSIGNAL 避免 SIGPIPE 终止服务器。 */
        ssize_t n = send(fd, data, length, MSG_NOSIGNAL);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return -1;
        data += n;
        length -= (size_t)n;
    }
    return 0;
}

int http_send_response(int fd, const HttpResponse *response, int head_only)
{
    char header[512];
    /* HTTP 响应先发状态行和头部，空行之后才是正文。 */
    int n = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n"
        "Connection: close\r\n%s\r\n",
        response->status, reason(response->status), response->content_type,
        response->body_length, response->status == 405 ? "Allow: GET, POST\r\n" : "");
    if (n < 0 || (size_t)n >= sizeof(header)) return -1;
    if (send_all(fd, header, (size_t)n) < 0) return -1;
    /* HEAD 保留响应头里的正文长度，但不发送正文。 */
    return head_only ? 0 : send_all(fd, response->body, response->body_length);
}
