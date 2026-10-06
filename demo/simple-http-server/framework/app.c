#include "app.h"
#include "routes.h"
#include <onion/onion.h>
#include <stdio.h>

int app_run(const char *port)
{
    /* Onion 接管 socket、HTTP 解析、响应编码和连接生命周期。 */
    onion *server = onion_new(O_ONE_LOOP);
    onion_set_hostname(server, "0.0.0.0");
    onion_set_port(server, port);
    onion_set_timeout(server, 5000);
    onion_set_max_post_size(server, 8192);
    onion_set_max_file_size(server, 8192);
    routes_register(onion_root_url(server));
    printf("Starting Onion server on http://0.0.0.0:%s\n", port);
    fflush(stdout);
    int result = onion_listen(server);
    onion_free(server);
    return result == 0 ? 0 : 1;
}
