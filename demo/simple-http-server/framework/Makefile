CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -g -O0
LDFLAGS = -static
CPPFLAGS = -I.deps/onion-src/src -I.deps/onion-build/src
ONION_LIBRARY = .deps/onion-build/src/onion/libonion_static.a
SOURCES = main.c app.c routes.c
HEADERS = app.h routes.h

.PHONY: all deps clean
all: simple-http-server

deps:
	bash prepare-onion.sh

$(ONION_LIBRARY): prepare-onion.sh
	bash prepare-onion.sh

simple-http-server: Makefile $(SOURCES) $(HEADERS) $(ONION_LIBRARY)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(SOURCES) $(ONION_LIBRARY) -pthread -o $@

clean:
	rm -f simple-http-server
