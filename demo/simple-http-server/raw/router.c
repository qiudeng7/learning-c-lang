#include "router.h"
#include <string.h>

static HttpResponse home(const HttpRequest *request)
{
    (void)request;
    return http_text(200, "Hello from a C HTTP server!\n");
}

static HttpResponse echo(const HttpRequest *request)
{
    /* 借用请求正文，不复制；发送响应之前，请求缓冲区必须还活着。 */
    return (HttpResponse){200, "application/octet-stream",
                          request->body, request->body_length};
}

/* 增加处理函数和表项即可新增 GET/POST 路由，socket 和 HTTP 层不用改。 */
static const HttpRoute routes[] = {
    {"GET", "/", home},
    {"POST", "/echo", echo},
};

HttpResponse router_dispatch(const HttpRequest *request)
{
    if (strcmp(request->method, "GET") != 0 && strcmp(request->method, "POST") != 0)
        return http_text(405, "Method not allowed\n");
    for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); ++i) {
        if (strcmp(request->method, routes[i].method) == 0 &&
            strcmp(request->path, routes[i].path) == 0)
            return routes[i].handler(request);
    }
    return http_text(404, "Not found\n");
}
