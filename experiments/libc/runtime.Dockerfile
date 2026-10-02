FROM learning-c-libc:debian-build AS inspect
COPY artifacts/ /programs/

FROM debian:bookworm-slim AS debian-run
COPY artifacts/ /programs/
CMD ["/programs/debian-glibc-dynamic"]

FROM alpine:3.22 AS alpine-run
COPY artifacts/ /programs/
CMD ["/programs/alpine-musl-dynamic"]

FROM learning-c-libc:arch-libs AS arch-run
COPY artifacts/ /programs/
CMD ["/programs/arch-glibc-dynamic"]

FROM scratch AS scratch-run
COPY artifacts/ /programs/
CMD ["/programs/debian-glibc-static"]
