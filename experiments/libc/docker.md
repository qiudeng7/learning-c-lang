# libc 实验用到的 Docker 基础

[实验脚本](run.sh) 自动构建镜像并启动测试容器。镜像保存程序及其运行环境，容器是从镜像启动的实例。在仓库根目录运行 `bash experiments/libc/run.sh` 前，需要安装 Docker，并能执行 `docker version`。

## 编译镜像和运行镜像

[Dockerfile](Dockerfile) 安装编译工具，在 Debian、Alpine 和 Arch 中编译源码。`FROM` 选择基础镜像，`RUN` 执行构建命令，`COPY` 复制文件，`WORKDIR` 设置工作目录。`AS` 给构建阶段命名，脚本用 `docker build --target` 选择要构建的阶段。

[runtime.Dockerfile](runtime.Dockerfile) 使用四种运行环境，并将相同的八个产物复制到 `/programs`。编译器留在编译镜像中，运行镜像只需要程序和相应的运行库。检查镜像则保留 `readelf`、`nm` 等工具，供脚本验证链接情况。

Arch 的运行镜像同时安装 glibc 和 musl。scratch 从空文件系统开始，只放入八个程序，没有共享 libc 和动态链接器。[Docker 对 scratch 的说明](https://docs.docker.com/build/building/base-images/)

## 脚本如何执行测试

`docker run` 从镜像启动容器，镜像名后的程序路径指定要运行的程序。`--rm` 在程序结束后删除测试容器，构建好的镜像仍然保留。脚本记录退出码和输出，并逐个检查八个程序在四种环境中的运行结果。[Docker 的运行命令说明](https://docs.docker.com/reference/cli/docker/container/run/)

实验使用的临时容器和临时构建文件会由脚本清理，报告保存在 `results` 目录。回到 [编译流程的 libc 实验](../../docs/compilation-process.md#libc-实验)，可以查看设计与结果说明。
