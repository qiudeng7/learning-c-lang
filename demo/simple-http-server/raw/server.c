#define _POSIX_C_SOURCE 200809L
#include "server.h"
#include "http.h"
#include "router.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

static void stop_server(int signal_number)
{
    (void)signal_number;
    /* 退出进程，由内核关闭 socket；也能中断正在等待的慢速请求。 */
    _exit(0);
}

static void handle_connection(int client)
{
    /* 栈上的存储归当前函数所有，request/response 只是借用其中的正文。 */
    char buffer[HTTP_HEADER_LIMIT + HTTP_BODY_LIMIT + 1];
    HttpRequest request;
    /* 读请求 → 路由处理 → 发响应；解析失败时直接生成错误响应。 */
    int result = http_read_request(client, buffer, sizeof(buffer), &request);
    if (result < 0) return;
    HttpResponse response = result == 0 ? router_dispatch(&request)
                                       : http_text(result, "Request rejected\n");
    if (http_send_response(client, &response, strcmp(request.method, "HEAD") == 0) < 0)
        perror("send response");
}

int server_run(unsigned short port)
{
    struct sigaction action = {.sa_handler = stop_server};
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, NULL) < 0 || sigaction(SIGTERM, &action, NULL) < 0) {
        perror("sigaction");
        return 1;
    }
    /* AF_INET 使用 IPv4，SOCK_STREAM 选择 TCP 字节流。 */
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) { perror("socket"); return 1; }
    /* 允许重新绑定仍处于 TIME_WAIT 的旧连接所用的本地地址。 */
    int reuse = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
        goto failure;
    /* 监听容器所有 IPv4 接口；端口和地址按网络字节序填写。 */
    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };
    /* bind 绑定地址；listen 开始监听，16 是待接受连接的队列参数。 */
    if (bind(listener, (struct sockaddr *)&address, sizeof(address)) < 0) goto failure;
    if (listen(listener, 16) < 0) goto failure;
    printf("Listening on http://0.0.0.0:%hu\n", port);
    fflush(stdout);
    for (;;) {
        /* accept 阻塞等待连接，返回新的连接 socket，listener 继续监听。 */
        int client = accept(listener, NULL, NULL);
        if (client < 0) {
            if (errno == EINTR) continue;
            goto failure;
        }
        /* 每次阻塞读写最多等 5 秒，避免断开的客户端一直占住服务器。 */
        struct timeval timeout = {.tv_sec = 5, .tv_usec = 0};
        if (setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0 ||
            setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
            perror("set timeout");
        } else {
            handle_connection(client);
        }
        /* 每个连接只处理一个请求；失败路径也在这里释放连接。 */
        close(client);
    }
/* 初始化或监听失败时，统一释放已创建的监听 socket。 */
failure:
    perror("server");
    close(listener);
    return 1;
}
