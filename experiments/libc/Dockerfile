FROM debian:bookworm-slim AS debian-build
RUN apt-get update \
    && apt-get install -y --no-install-recommends gcc libc6-dev binutils \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /work
COPY main.c build.sh ./
RUN sh build.sh debian glibc gcc

FROM alpine:3.22 AS alpine-build
RUN apk add --no-cache gcc musl-dev binutils
WORKDIR /work
COPY main.c build.sh ./
RUN sh build.sh alpine musl gcc

FROM archlinux:base AS arch-libs
RUN pacman -Syu --noconfirm musl \
    && pacman -Scc --noconfirm

FROM arch-libs AS arch-build
RUN pacman -S --noconfirm gcc binutils \
    && pacman -Scc --noconfirm
WORKDIR /work
COPY main.c build.sh ./
RUN sh build.sh arch glibc gcc \
    && sh build.sh arch musl musl-gcc
