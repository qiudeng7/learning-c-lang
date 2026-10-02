FROM debian:bookworm-slim AS build

RUN apt-get update \
    && apt-get install -y --no-install-recommends gcc libc6-dev binutils \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
COPY quickstart/main.c main.c
RUN gcc -std=c17 -Wall -Wextra -Wpedantic -c main.c -o main.o \
    && gcc main.o -o hello-dynamic \
    && gcc -static main.o -o hello-static

FROM debian:bookworm-slim AS with-libc
COPY --from=build /work/hello-dynamic /hello-dynamic
COPY --from=build /work/hello-static /hello-static
CMD ["/hello-dynamic"]

FROM scratch AS without-libc
COPY --from=build /work/hello-dynamic /hello-dynamic
COPY --from=build /work/hello-static /hello-static
CMD ["/hello-static"]

FROM alpine:3.22 AS musl-build
RUN apk add --no-cache gcc musl-dev binutils
WORKDIR /work
COPY quickstart/main.c main.c
RUN gcc -std=c17 -Wall -Wextra -Wpedantic -c main.c -o main.o \
    && gcc main.o -o hello-dynamic \
    && gcc -static main.o -o hello-static

FROM alpine:3.22 AS with-musl
COPY --from=musl-build /work/hello-dynamic /hello-dynamic
COPY --from=musl-build /work/hello-static /hello-static
CMD ["/hello-dynamic"]

FROM scratch AS without-musl
COPY --from=musl-build /work/hello-dynamic /hello-dynamic
COPY --from=musl-build /work/hello-static /hello-static
CMD ["/hello-static"]
