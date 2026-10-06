CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g -O0
LDFLAGS = -static
SOURCES = main.c server.c http.c router.c
HEADERS = server.h http.h router.h

.PHONY: all clean
all: simple-http-server

simple-http-server: Makefile $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $@

clean:
	rm -f simple-http-server
