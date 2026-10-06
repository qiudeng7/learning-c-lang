# simple-http-server

从 socket 到 tcp 到 HTTP，写一个能被 curl 调用的服务器，分有无框架两个版本。

- [framework/](framework/)：使用 Onion 框架，默认端口 8081。
- [raw/](raw/)：使用 POSIX socket 手写 HTTP，默认端口 8080。

都提供 `GET /` 和 `POST /echo`。

## 快速开始

本机只需要 Docker、make 和 curl，先启动 Docker 服务。

在仓库根目录执行：

```bash
cd demo/simple-http-server
make
```

这一步会在 Docker 中安装编译工具，构建 Onion 和两个静态链接的程序，再分别生成只包含程序的 scratch 镜像。

在两个终端分别启动：

```bash
make run-raw
```

```bash
make run-framework
```

端口只映射到本机，按 Ctrl+C 停止并删除容器。

## 测试使用

```bash
# 手写版用 8080，框架版改为 8081
curl --noproxy '*' -i http://127.0.0.1:8080/
# 200，Hello from a C HTTP server!

curl --noproxy '*' -i -H 'Content-Type: application/octet-stream' \
    --data-binary 'hello socket' http://127.0.0.1:8080/echo
# 200，hello socket
```