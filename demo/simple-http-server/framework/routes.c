#include "routes.h"
#include <onion/block.h>
#include <onion/handler.h>
#include <onion/request.h>
#include <onion/response.h>
#include <string.h>

static onion_connection_status reply(onion_response *response, int status,
                                    const char *type, const char *body, size_t length)
{
    onion_response_set_code(response, status);
    onion_response_set_header(response, "Content-Type", type);
    onion_response_set_header(response, "Connection", "close");
    /* 正文长度由字节数决定，二进制正文不能用 strlen 计算。 */
    onion_response_set_length(response, length);
    if (length > 0 && onion_response_write(response, body, length) < 0)
        return OCS_CLOSE_CONNECTION;
    /* 通知 Onion 发送完响应后关闭连接，不继续等待下一条请求。 */
    return OCS_CLOSE_CONNECTION;
}

static onion_connection_status text(onion_response *response, int status, const char *body)
{
    return reply(response, status, "text/plain; charset=utf-8", body, strlen(body));
}

static onion_connection_status method_error(onion_response *response)
{
    onion_response_set_header(response, "Allow", "GET, POST");
    return text(response, 405, "Method not allowed\n");
}

static onion_connection_status home(void *context, onion_request *request,
                                    onion_response *response)
{
    (void)context;
    /* flags 同时包含方法和其他状态，掩码只取出方法部分。 */
    int method = onion_request_get_flags(request) & OR_METHODS;
    if (method == OR_GET) return text(response, 200, "Hello from a C HTTP server!\n");
    if (method == OR_POST) return text(response, 404, "Not found\n");
    return method_error(response);
}

static onion_connection_status echo(void *context, onion_request *request,
                                    onion_response *response)
{
    (void)context;
    int flags = onion_request_get_flags(request);
    int method = flags & OR_METHODS;
    if (method == OR_GET) return text(response, 404, "Not found\n");
    if (method != OR_POST) return method_error(response);
    /* 框架会将表单解析为字段；回声接口使用原始正文而非表单数据。 */
    const onion_block *body = onion_request_get_data(request);
    const char *content_length = onion_request_get_header(request, "Content-Length");
    if (body == NULL && content_length != NULL && strcmp(content_length, "0") != 0)
        return text(response, 415, "Use Content-Type: application/octet-stream\n");
    const char *data = body ? onion_block_data(body) : "";
    size_t length = body ? (size_t)onion_block_size(body) : 0;
    /* 正文归 Onion 所有，处理函数只在当前请求中借用。 */
    return reply(response, 200, "application/octet-stream", data, length);
}

static onion_connection_status not_found(void *context, onion_request *request,
                                         onion_response *response)
{
    (void)context;
    int method = onion_request_get_flags(request) & OR_METHODS;
    if (method != OR_GET && method != OR_POST) return method_error(response);
    return text(response, 404, "Not found\n");
}

void routes_register(onion_url *urls)
{
    /* 路径不含开头的 /；增加处理函数和注册项即可扩展接口。 */
    onion_url_add_handler(urls, "^$", onion_handler_new(home, NULL, NULL));
    onion_url_add_handler(urls, "^echo$", onion_handler_new(echo, NULL, NULL));
    /* 兜底路由最后注册，否则会先匹配所有路径。 */
    onion_url_add_handler(urls, "^.*$", onion_handler_new(not_found, NULL, NULL));
}
