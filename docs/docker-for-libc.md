# 本次 libc 实验的 Docker 用法

[libc 实验](libc.md) 使用 Docker 提供 Debian、Alpine 和只含程序文件的运行环境。这篇说明实验命令中的 Docker 用法，帮助你构建这些环境、进入容器检查文件，再运行编译好的程序。

开始前需要安装 Docker，并确认当前用户可以运行 `docker version`。以下 Docker 命令都在宿主机的仓库根目录执行。

## 镜像、容器和 Dockerfile

镜像保存程序及其运行所需的文件，容器是从镜像启动的实例。我们先构建包含 GCC 和 libc 开发文件的镜像，再启动容器进行检查；运行实验时，则从另一些镜像启动容器，观察程序有没有所需的共享库。

[实验 Dockerfile](../experiments/libc/Dockerfile) 描述这些镜像如何构建。`FROM` 选择基础镜像，`RUN` 在构建时执行命令，`COPY` 将文件复制进镜像，`WORKDIR` 设置后续命令的工作目录。文件中的 `CMD` 指定容器默认启动的命令。

## 构建编译镜像

以 glibc 实验为例：

```bash
docker build -f experiments/libc/Dockerfile --target build -t learning-c-libc:build .
```

`-f` 指定 Dockerfile。最后的 `.` 将当前仓库目录作为构建上下文，也就是 Dockerfile 可以取得源文件的范围；实验通过 `COPY quickstart/main.c main.c` 取得源码。

同一个 Dockerfile 中有多个以 `FROM` 开头的构建阶段，`AS` 为阶段命名。`--target build` 选择名为 `build` 的阶段作为构建结果，`-t learning-c-libc:build` 给生成的镜像命名。冒号后的 `build` 是镜像标签，可以用来区分不同用途的镜像。

首次构建会下载基础镜像和软件包。这个阶段还会在 `/work` 中编译源码，生成 `main.o`、`hello-dynamic` 和 `hello-static`，因此进入容器后可以直接检查它们。

## 进入编译容器

```bash
docker run --rm -it learning-c-libc:build sh
```

`docker run` 从指定镜像启动容器。`-it` 让你可以交互操作，镜像名后面的 `sh` 指定启动 shell。进入后，当前目录是 Dockerfile 设置的 `/work`；此时执行的 GCC、`nm` 和 `readelf` 命令都在容器中运行。

输入 `exit` 会退出 shell 并回到宿主机。`--rm` 会在容器退出后删除容器，构建好的镜像仍然保留。这些实验命令没有挂载宿主机目录，容器内重新生成的文件保存在容器里。

musl 实验的操作相同，只需使用正文中的 `musl-build` 阶段和镜像名。

## 构建不同的运行环境

glibc 实验使用两种运行镜像：

```bash
docker build -f experiments/libc/Dockerfile --target with-libc -t learning-c-libc:with-libc .
docker build -f experiments/libc/Dockerfile --target without-libc -t learning-c-libc:without-libc .
```

`with-libc` 从 Debian 基础镜像构建，提供 glibc 运行环境。`COPY --from=build` 从前面的编译阶段取得两份程序，复制为 `/hello-dynamic` 和 `/hello-static`。

`without-libc` 从 `scratch` 构建。`scratch` 表示从空的文件系统开始，这个阶段只复制两份程序，所以没有共享库、动态链接器或 shell。这正是实验需要的对照环境。[Docker 对 scratch 的说明](https://docs.docker.com/build/building/base-images/)

musl 对应的阶段是 `with-musl` 和 `without-musl`：前者使用 Alpine，后者同样从 scratch 开始，并从 `musl-build` 取得程序。

## 运行程序并观察结果

```bash
docker run --rm learning-c-libc:with-libc /hello-dynamic
```

镜像名后面的 `/hello-dynamic` 指定容器启动时直接执行的程序，替代 Dockerfile 中的默认 `CMD`。它的输出显示在当前终端；程序结束后，容器也结束，并由 `--rm` 删除。[Docker 的运行命令说明](https://docs.docker.com/reference/cli/docker/container/run/)

正文中的运行命令都采用这种方式。编译镜像通过 `sh` 进入交互环境，运行镜像直接启动可执行文件。接下来回到 [glibc 编译实验](libc.md#准备-glibc-编译环境)，按正文依次检查依赖并比较运行结果。
