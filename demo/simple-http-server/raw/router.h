#ifndef SIMPLE_ROUTER_H
#define SIMPLE_ROUTER_H
#include "http.h"

typedef HttpResponse (*HttpHandler)(const HttpRequest *request);
typedef struct {
    const char *method;
    const char *path;
    HttpHandler handler;
} HttpRoute;

HttpResponse router_dispatch(const HttpRequest *request);
#endif
