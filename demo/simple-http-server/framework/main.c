#include "app.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    const char *port = argc == 2 ? argv[1] : "8081";
    char *end;
    errno = 0;
    /* 端口字符串转数字，校验端口数字是否合法 */
    long number = strtol(port, &end, 10);
    if (argc > 2 || errno || end == port || *end || number < 1 || number > 65535) {
        fprintf(stderr, "Usage: %s [port: 1..65535]\n", argv[0]);
        return 1;
    }
    return app_run(port);
}
