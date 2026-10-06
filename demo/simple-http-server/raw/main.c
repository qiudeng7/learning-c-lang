#include "server.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    long port = 8080;
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        return 1;
    }
    if (argc == 2) {
        char *end;
        errno = 0;
        port = strtol(argv[1], &end, 10);
        if (errno || end == argv[1] || *end || port < 1 || port > 65535) {
            fprintf(stderr, "Port must be an integer between 1 and 65535\n");
            return 1;
        }
    }
    return server_run((unsigned short)port);
}
