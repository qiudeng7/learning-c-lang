#ifndef SIMPLE_HTTP_H
#define SIMPLE_HTTP_H

#include <stddef.h>

#define HTTP_HEADER_LIMIT 8192
#define HTTP_BODY_LIMIT 8192

typedef struct {
    char method[16];
    char path[1024];
    /* body 指向调用者的缓冲区，只在处理当前请求时有效。 */
    const char *body;
    size_t body_length;
} HttpRequest;

typedef struct {
    int status;
    const char *content_type;
    const char *body;
    size_t body_length;
} HttpResponse;

/* 成功返回 0；协议错误返回 HTTP 状态码；连接断开返回 -1。 */
int http_read_request(int fd, char *buffer, size_t capacity, HttpRequest *request);
int http_send_response(int fd, const HttpResponse *response, int head_only);
HttpResponse http_text(int status, const char *text);

#endif
