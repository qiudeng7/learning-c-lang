# 编译环境提供 GCC（musl）、make；所有构建工具留在这里。
ARG BUILD_IMAGE=alpine:3.23
FROM ${BUILD_IMAGE} AS build
ARG ALPINE_MIRROR=https://dl-cdn.alpinelinux.org/alpine
RUN sed -i "s|https://dl-cdn.alpinelinux.org/alpine|${ALPINE_MIRROR}|g" /etc/apk/repositories \
    && apk add --no-cache gcc musl-dev make
WORKDIR /src

FROM build AS raw-build
COPY raw/ ./raw/
RUN make -C raw

FROM build AS framework-build
RUN apk add --no-cache bash git cmake g++
COPY framework/ ./framework/
RUN make -C framework

# scratch 没有 shell 或动态库，仅复制静态链接的程序。
FROM scratch AS raw
COPY --from=raw-build /src/raw/simple-http-server /simple-http-server
USER 65534:65534
EXPOSE 8080
ENTRYPOINT ["/simple-http-server"]
CMD ["8080"]

FROM scratch AS framework
COPY --from=framework-build /src/framework/simple-http-server /simple-http-server
USER 65534:65534
EXPOSE 8081
ENTRYPOINT ["/simple-http-server"]
CMD ["8081"]
