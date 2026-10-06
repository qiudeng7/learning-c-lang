#include "app.h"
#include "routes.h"
#include <onion/onion.h>
#include <stdio.h>

int app_run(const char *port)
{
    /* Onion 接管 socket、HTTP 解析、响应编码和连接生命周期。 */
    onion *server = onion_new(O_ONE_LOOP); /* 串行处理请求，方便追踪执行路径。 */
    /* 容器内监听所有接口，宿主机的访问范围由 Docker 端口映射决定。 */
    onion_set_hostname(server, "0.0.0.0");
    onion_set_port(server, port);
    onion_set_timeout(server, 5000);
    onion_set_max_post_size(server, 8192);
    onion_set_max_file_size(server, 8192);
    /* 先注册路由，再进入框架的监听循环；循环退出后释放应用。 */
    routes_register(onion_root_url(server));
    printf("Starting Onion server on http://0.0.0.0:%s\n", port);
    fflush(stdout);
    int result = onion_listen(server);
    onion_free(server);
    return result == 0 ? 0 : 1;
}
