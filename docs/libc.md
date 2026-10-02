# libc：标准库实现从哪里来

在 [编译流程](compilation-process.md) 中，我们使用 GCC 将 quickstart 的代码编译、链接为 `hello`，并通过 `file` 看到它是动态链接程序。`stdio.h` 提供了 `printf` 的声明，但函数的实现由谁提供？这一篇先说明 C 标准、GCC 和 libc 的分工，再用 glibc 和 musl 分别实验，观察动态链接和静态链接如何影响程序的依赖与运行。

## C 标准、GCC 和 libc 的分工

C 标准规定了标准库的接口和行为，具体代码需要由库实现提供。提供这些实现的 C 库通常称为 libc。glibc 是其中一种，全称是 GNU C Library，也是下面第一组 Linux 实验使用的实现。我们调用的 `printf`，具体执行的代码就由它提供。[glibc 官方介绍](https://www.gnu.org/software/libc/manual/html_node/Introduction.html)

回到 quickstart：GCC 把我们写的代码编译成目标文件，随后组织链接，把目标文件与所需的库连接起来。使用 GCC 的普通链接命令时，它会按工具链配置自动加入 C 库，因此我们不用在命令中逐一列出这些库文件。GCC 自身也提供部分库设施，但 `printf` 等标准库函数的主体实现需要由 libc 提供。[GCC 对标准库的说明](https://gcc.gnu.org/onlinedocs/gcc/Standard-Libraries.html)、[默认库的链接规则](https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html)

glibc 提供的接口还超出了 C 标准库。例如，后续系统编程会用到的 `read`、`write`、`fork` 属于 POSIX 接口；POSIX 是规定类 Unix 系统接口的一套标准。glibc 也提供 GNU 自己的扩展接口，所以“glibc 提供的函数”和“C 标准规定的函数”范围并不完全相同。

glibc 是用户态库。像字符串处理这样的工作可以在用户态完成；需要读写文件或创建进程时，相关实现会通过系统调用请求内核服务。因此，使用 libc 接口是理解“用户态程序怎样使用操作系统”的一个入口。

### 常见的 libc 实现

不同系统采用不同的 libc 实现。下表列出几种常见实现及其使用环境；后面的实验使用 Debian 中的 glibc 和 Alpine Linux 中的 musl。

| 实现 | 常见环境 | 简要特点 |
| --- | --- | --- |
| [glibc](https://sourceware.org/glibc/) | GNU/Linux 系统，例如本次使用的 Debian | 提供 C 标准库，以及 POSIX、GNU 等接口 |
| [musl](https://wiki.musl-libc.org/) | Linux；Alpine Linux 使用它 | 注重简洁和较小体积，也支持静态链接 |
| [Bionic](https://android.googlesource.com/platform/bionic/) | Android | Android 的 C 库，项目还包含数学库和动态链接器 |
| [newlib](https://sourceware.org/newlib/info.html) | 嵌入式系统 | 面向嵌入式环境，底层服务需要与目标平台适配 |

#### glibc 与 musl 的差异

glibc 和 musl 都提供 C 标准库和 POSIX 接口，也都支持动态链接与静态链接。它们的差异主要体现在扩展接口、实现规模和运行环境上。

下表中的扩展库接口，是 C 和 POSIX 标准接口之外的函数或能力。它与 [编译流程](compilation-process.md) 中提到的 GNU 语言扩展属于不同层面：前者由库提供，后者由编译器支持。动态链接器负责在程序启动时加载共享库，表中的路径以 x86-64 Linux 为例。

| 对比项 | glibc | musl |
| --- | --- | --- |
| 设计侧重点 | 提供丰富的接口和扩展，并兼顾已有程序的兼容性 | 注重实现简洁、较小体积和标准接口的行为 |
| 扩展库接口 | 提供较多 GNU 扩展库接口，很多软件会依赖这些接口 | 提供部分 GNU 扩展库接口，但并不完整复现 glibc 的接口和行为 |
| 静态链接 | 支持，后面的实验会直接验证 | 同样支持，较小的实现规模是它用于静态程序的一个吸引点 |
| 动态链接器 | 常见路径是 `/lib64/ld-linux-x86-64.so.2` | 通常使用 `/lib/ld-musl-x86_64.so.1` |
| 常见运行环境 | 例如本次使用的 Debian | 例如 Alpine Linux |

对写源码的人来说，依赖标准接口通常更便于在两者之间移植；如果代码使用了 glibc 特有接口，换到 musl 时可能需要调整。例如，`printf` 通常用 `%d` 这样的格式符指定输出方式；glibc 还允许程序注册自定义的格式处理函数，让它处理额外的格式符。musl 不提供同样的扩展机制，依赖这一功能的代码就需要改写。[musl 项目介绍](https://wiki.musl-libc.org/)、[与 glibc 的功能差异](https://wiki.musl-libc.org/functional-differences-from-glibc)

对运行程序的人来说，还要区分源码可以移植和已编译文件可以直接运行。一个在 Debian 中动态链接 glibc 的程序，已经记录了 glibc 的动态链接器和库依赖；直接复制到 Alpine，并不会自动改用 musl。通常需要用 musl 提供的头文件和库重新编译、链接，或者为程序提供所需的 glibc 运行环境。[Alpine 的 musl 说明](https://wiki.alpinelinux.org/wiki/Musl)、[musl 动态链接器路径](https://wiki.musl-libc.org/guidelines-for-distributions)

#### glibc 和 musl 可以在同一个系统中共存吗

可以。Linux 系统可以同时安装 glibc 和 musl，让不同程序分别使用它们。例如，Debian 默认使用 glibc，也提供 musl 的库、开发文件和工具包。安装 musl 后，已有的 glibc 程序仍然使用 glibc；编译新程序时，可以用 `musl-gcc` 选择 musl 的头文件和库。`musl-gcc` 是配置 GCC 使用 musl 的包装脚本。[Debian 的 musl 工具包说明](https://packages.debian.org/bookworm/musl-tools)

动态程序使用哪套 libc，在链接时就已经确定。前面列出的两个动态链接器路径可以同时存在：glibc 程序启动时使用它指定的 glibc 动态链接器，musl 程序则使用 musl 的动态链接器，再由各自的链接器加载所需共享库。程序不会因为系统新安装了另一套 libc 就自动切换。[musl 的安装与使用说明](https://wiki.musl-libc.org/getting-started.html)、[动态链接器路径约定](https://wiki.musl-libc.org/guidelines-for-distributions)

因此，同一个系统中可以同时运行两类程序；构建时也可以保留两套开发环境，分别选择对应的头文件和库。下面使用 Debian 和 Alpine 分开实验，是为了方便观察各自的链接结果和运行依赖。

下面用同一份 quickstart 源码，分别观察两种 libc 在动态链接和静态链接时的依赖与运行结果。文件大小也会记录在实验末尾，供对照这次生成的程序。

为了便于已经学过 Docker 的读者阅读，本文不在行文中穿插 Docker 的使用方式。本次实验用到的基础知识放在 [本次 libc 实验的 Docker 用法](docker-for-libc.md) 中；没有 Docker 基础的读者可以先读这一篇，再回来进行实验。

## glibc 的动态链接与静态链接

### 准备 glibc 编译环境

实验使用仓库中的 [Dockerfile](../experiments/libc/Dockerfile)，基于 `debian:bookworm-slim` 安装 GCC、glibc 开发文件和 binutils。binutils 提供下面使用的 `nm`、`readelf` 等工具。

在**仓库根目录、宿主机终端**构建编译镜像，再进入它：

```bash
docker build -f experiments/libc/Dockerfile --target build -t learning-c-libc:build .
docker run --rm -it learning-c-libc:build sh
```

容器的工作目录是 `/work`，其中已有 quickstart 源码、目标文件和两种可执行文件。也可以手动重复构建过程：先生成目标文件，再将同一个目标文件分别链接为动态版和静态版。

```bash
# 缩写参数
gcc -std=c17 -Wall -Wextra -Wpedantic -c main.c -o main.o
gcc main.o -o hello-dynamic
gcc -static main.o -o hello-static

# 全拼参数
gcc --std=c17 --warn-all --warn-extra --warn-pedantic --compile main.c --output main.o
gcc main.o --output hello-dynamic
gcc --static main.o --output hello-static
```

`-static`（全拼 `--static`）要求在链接时使用静态库，影响的是链接阶段。这次只编译一份 `main.o`，然后改变链接方式，方便把差异直接对应到链接行为。[GCC 的静态链接选项](https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html)

### 链接器使用的库文件在哪里

以下检查都在刚才的**编译容器内**执行。`-print-file-name=文件名`（全拼 `--print-file-name=文件名`）让 GCC 按库搜索路径查找指定文件，并打印路径。先用它查找两种链接方式对应的 C 库文件：

```bash
# 缩写参数
gcc -print-file-name=libc.so
gcc -print-file-name=libc.a
# 全拼参数
gcc --print-file-name=libc.so
gcc --print-file-name=libc.a
```

两种写法得到相同结果，每组输出是：

```text
/usr/lib/gcc/x86_64-linux-gnu/12/../../../x86_64-linux-gnu/libc.so
/usr/lib/gcc/x86_64-linux-gnu/12/../../../x86_64-linux-gnu/libc.a
```

`libc.a` 把多个已编译的目标文件打包在一起，供静态链接时取用。在本次 Debian 环境中，`libc.so` 则是一个文本形式的链接器脚本，告诉链接器去哪里查找 `libc.so.6` 等文件；`libc.so.6` 才是程序运行时加载的共享库。因此，链接时查到的文件与运行时加载的文件，名称和用途有所区别。

这也说明，`-std=c17` 选择的是编译器的语言模式，使用哪套库实现则取决于工具链和系统提供的库文件。

### 从目标文件到动态链接程序

先查看目标文件中尚未定义的符号，`nm` 的 `-u` 选项只列出这一类符号：

```bash
nm -u main.o
```

完整输出：

```text
                 U printf
                 U puts
```

`U` 表示目标文件引用了这个符号，但没有提供定义。源码中的第一个 `printf` 在本次编译中被转换成了 `puts` 调用，所以这里会看到两者。

接着查看动态版程序的动态链接信息。`readelf -d` 显示 ELF 的动态节，这是文件中记录共享库依赖等动态链接信息的部分：

```bash
readelf -d hello-dynamic
```

完整输出：

```text

Dynamic section at offset 0x2de0 contains 26 entries:
  Tag        Type                         Name/Value
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
 0x000000000000000c (INIT)               0x1000
 0x000000000000000d (FINI)               0x119c
 0x0000000000000019 (INIT_ARRAY)         0x3dd0
 0x000000000000001b (INIT_ARRAYSZ)       8 (bytes)
 0x000000000000001a (FINI_ARRAY)         0x3dd8
 0x000000000000001c (FINI_ARRAYSZ)       8 (bytes)
 0x000000006ffffef5 (GNU_HASH)           0x3a0
 0x0000000000000005 (STRTAB)             0x488
 0x0000000000000006 (SYMTAB)             0x3c8
 0x000000000000000a (STRSZ)              148 (bytes)
 0x000000000000000b (SYMENT)             24 (bytes)
 0x0000000000000015 (DEBUG)              0x0
 0x0000000000000003 (PLTGOT)             0x3fe8
 0x0000000000000002 (PLTRELSZ)           48 (bytes)
 0x0000000000000014 (PLTREL)             RELA
 0x0000000000000017 (JMPREL)             0x620
 0x0000000000000007 (RELA)               0x560
 0x0000000000000008 (RELASZ)             192 (bytes)
 0x0000000000000009 (RELAENT)            24 (bytes)
 0x000000006ffffffb (FLAGS_1)            Flags: PIE
 0x000000006ffffffe (VERNEED)            0x530
 0x000000006fffffff (VERNEEDNUM)         1
 0x000000006ffffff0 (VERSYM)             0x51c
 0x000000006ffffff9 (RELACOUNT)          3
 0x0000000000000000 (NULL)               0x0
```

这次重点看 `NEEDED` 中的 `Shared library: [libc.so.6]`：目标文件中对库函数的引用，到了链接后的程序里，对应到了一个具体的共享库依赖。运行程序时，动态链接器需要找到并加载这套库；相关函数的实现没有全部复制进我们的可执行文件。

### 静态链接后有什么变化

查看静态版的同一类信息：

```bash
readelf -d hello-static
```

完整输出：

```text
There is no dynamic section in this file.
```

静态版没有动态节。它使用的 libc 实现代码已经在链接时从静态库中取入程序。

为了观察这个区别，再只查找静态版中 `printf` 和 `puts` 的已定义符号：

```bash
nm --defined-only hello-static | grep -E ' (printf|puts)$'
```

这条筛选命令的完整输出：

```text
000000000040a000 T printf
00000000004102f0 W puts
```

这里的 `--defined-only` 让 `nm` 只列出已有定义的符号，后面的筛选则只保留 `printf` 和 `puts`。两者都出现在结果中，说明这个可执行文件里已经有它们的定义。结合前面目标文件里的 `U`，可以看到：静态链接把需要的实现放进了可执行文件，运行时无需再加载 `libc.so.6` 来提供它们。

也可以比较两份文件的字节数：

```bash
wc -c hello-dynamic hello-static
```

本次结果：

```text
 16008 hello-dynamic
762520 hello-static
778528 total
```

静态链接从 `libc.a` 中取用程序需要的目标文件。静态版包含了这些库代码及相关运行支持，所以文件更大。

### 在 Debian 和 scratch 中运行

在编译容器中执行 `exit`，回到**仓库根目录的宿主机终端**，再构建两个运行镜像：

```bash
docker build -f experiments/libc/Dockerfile --target with-libc -t learning-c-libc:with-libc .
docker build -f experiments/libc/Dockerfile --target without-libc -t learning-c-libc:without-libc .
```

`with-libc` 阶段使用 Debian 基础镜像，具有 glibc 运行环境；`without-libc` 阶段使用 `FROM scratch`，只复制两份可执行文件，没有动态链接器、共享库或 shell。两个镜像里都存在 `/hello-dynamic` 和 `/hello-static`。

先运行以下三种组合：

```bash
docker run --rm learning-c-libc:with-libc /hello-dynamic
docker run --rm learning-c-libc:with-libc /hello-static
docker run --rm learning-c-libc:without-libc /hello-static
```

每条命令都成功输出：

```text
Hello, C!
40 + 2 = 42
```

最后在空镜像中运行动态版：

```bash
docker run --rm learning-c-libc:without-libc /hello-dynamic
```

本次失败输出为：

```text
exec /hello-dynamic: no such file or directory
```

这里程序文件确实已被 Dockerfile 复制进去，但它请求的 `/lib64/ld-linux-x86-64.so.2` 不存在，内核无法完成启动；这个空镜像中也没有 `libc.so.6`。这里的报错指向缺失的动态链接器。[Linux 动态链接器说明](https://man7.org/linux/man-pages/man8/ld.so.8.html)

本次实验的对比如下：

| 观察项 | 动态版 | 静态版 |
| --- | --- | --- |
| libc 实现如何提供 | 运行时加载 `libc.so.6` | 链接时把需要的库代码放进程序 |
| `readelf -d` | 存在 `NEEDED libc.so.6` | 没有动态节 |
| 本次文件大小 | 16,008 字节 | 762,520 字节 |
| Debian 运行镜像 | 成功 | 成功 |
| 只含两份程序的 scratch 镜像 | 缺少动态链接器，启动失败 | 成功 |

这说明“使用 C 标准库”和“运行时加载共享库”是两个不同的问题：两份程序都使用了 glibc 提供的实现，区别在于实现代码何时进入程序的运行环境。

## musl 的动态链接与静态链接

这一组使用 Alpine Linux 提供的 musl，检查动态版的共享库依赖和静态版的函数定义，再将两份程序分别放到 Alpine 与 scratch 中运行。源码和链接选项沿用 glibc 实验，便于观察换用 libc 后的结果。

### 准备 musl 编译环境

Alpine 的 GCC 配置为使用 musl 的头文件和库。我们可以继续使用 `-std=c17` 等编译参数，链接时会选用 musl；语言标准参数与 libc 实现的选择是两件事。

在**仓库根目录的宿主机终端**构建并进入 musl 编译容器：

```bash
docker build -f experiments/libc/Dockerfile --target musl-build -t learning-c-libc:musl-build .
docker run --rm -it learning-c-libc:musl-build sh
```

构建阶段会安装 GCC、musl 的开发文件和 binutils，并在 `/work` 中生成 `main.o`、`hello-dynamic` 和 `hello-static`。它使用的源码和 GCC 命令与 [glibc 编译实验](#准备-glibc-编译环境) 相同：先编译一份目标文件，再分别进行普通链接和静态链接。进入容器后可以直接检查这些产物，也可以用前面的 GCC 命令重新生成。

### 动态版依赖什么

以下命令在**musl 编译容器内**执行。先查看动态版的依赖：

```bash
readelf -d hello-dynamic
```

完整输出：

```text

Dynamic section at offset 0x2de8 contains 24 entries:
  Tag        Type                         Name/Value
 0x0000000000000001 (NEEDED)             Shared library: [libc.musl-x86_64.so.1]
 0x000000000000000c (INIT)               0x1000
 0x000000000000000d (FINI)               0x11e6
 0x0000000000000019 (INIT_ARRAY)         0x3dd8
 0x000000000000001b (INIT_ARRAYSZ)       8 (bytes)
 0x000000000000001a (FINI_ARRAY)         0x3de0
 0x000000000000001c (FINI_ARRAYSZ)       8 (bytes)
 0x000000006ffffef5 (GNU_HASH)           0x388
 0x0000000000000005 (STRTAB)             0x4b8
 0x0000000000000006 (SYMTAB)             0x3b0
 0x000000000000000a (STRSZ)              180 (bytes)
 0x000000000000000b (SYMENT)             24 (bytes)
 0x0000000000000015 (DEBUG)              0x0
 0x0000000000000003 (PLTGOT)             0x3fa8
 0x0000000000000002 (PLTRELSZ)           72 (bytes)
 0x0000000000000014 (PLTREL)             RELA
 0x0000000000000017 (JMPREL)             0x630
 0x0000000000000007 (RELA)               0x570
 0x0000000000000008 (RELASZ)             192 (bytes)
 0x0000000000000009 (RELAENT)            24 (bytes)
 0x000000000000001e (FLAGS)              BIND_NOW
 0x000000006ffffffb (FLAGS_1)            Flags: NOW PIE
 0x000000006ffffff9 (RELACOUNT)          3
 0x0000000000000000 (NULL)               0x0
```

与 glibc 那一组对照，重点仍是 `NEEDED`：这里记录的是 `libc.musl-x86_64.so.1`，而不是 `libc.so.6`。同一份源码经过不同的链接环境，生成的程序会依赖不同的共享库。

再看启动它需要的动态链接器。`readelf -l` 显示程序装载信息，这里通过 `grep interpreter` 只保留指定动态链接器的那一行：

```bash
readelf -l hello-dynamic | grep interpreter
```

筛选命令的完整输出：

```text
      [Requesting program interpreter: /lib/ld-musl-x86_64.so.1]
```

这条路径说明程序启动时需要 musl 的动态链接器。前面的 glibc 动态版指定的是 `/lib64/ld-linux-x86-64.so.2`；两份程序要求的启动环境不同。

### 静态版是否还依赖共享 musl

继续检查静态版：

```bash
readelf -d hello-static
```

完整输出：

```text
There is no dynamic section in this file.
```

它没有动态节。再按前面的方式查看两个库函数是否已经有定义：

```bash
nm --defined-only hello-static | grep -E ' (printf|puts)$'
```

筛选命令的完整输出：

```text
000000000040147d T printf
000000000040151b T puts
```

这里的两个 `T` 表示符号定义在代码区域。它们已经进入静态版可执行文件，所以运行这个程序不需要共享 musl 来提供这两个函数。

查看文件大小：

```bash
wc -c hello-dynamic hello-static
```

完整输出：

```text
    18408 hello-dynamic
   141128 hello-static
   159536 total
```

这次 musl 静态版也比动态版大；将需要的库代码放进程序，仍然会增加可执行文件的体积。

### 在 Alpine 和 scratch 中运行

在编译容器中执行 `exit`，回到**仓库根目录的宿主机终端**，构建两种运行环境：

```bash
docker build -f experiments/libc/Dockerfile --target with-musl -t learning-c-libc:with-musl .
docker build -f experiments/libc/Dockerfile --target without-musl -t learning-c-libc:without-musl .
```

`with-musl` 使用 Alpine 基础镜像，具有 musl 运行环境；`without-musl` 使用 scratch，只复制两份程序，没有动态链接器、共享库或 shell。先运行以下三种组合：

```bash
docker run --rm learning-c-libc:with-musl /hello-dynamic
docker run --rm learning-c-libc:with-musl /hello-static
docker run --rm learning-c-libc:without-musl /hello-static
```

每条命令都成功输出：

```text
Hello, C!
40 + 2 = 42
```

最后在 scratch 中运行动态版：

```bash
docker run --rm learning-c-libc:without-musl /hello-dynamic
```

本次失败输出为：

```text
exec /hello-dynamic: no such file or directory
```

程序文件已经复制进镜像，启动失败是因为 scratch 中没有它指定的 `/lib/ld-musl-x86_64.so.1`。musl 的实验结果与 glibc 一致：普通链接生成的动态版需要相应的运行环境，使用 `-static` 生成的静态版则可以在这个 scratch 镜像中运行。

| 观察项 | musl 动态版 | musl 静态版 |
| --- | --- | --- |
| 共享库依赖 | `libc.musl-x86_64.so.1` | 不需要共享 musl |
| 动态链接器 | `/lib/ld-musl-x86_64.so.1` | 不需要 |
| Alpine 运行镜像 | 成功 | 成功 |
| 只含两份程序的 scratch 镜像 | 启动失败 | 成功 |

## 两组实验的文件大小

动态链接与静态链接影响的是库实现如何进入程序的运行环境，也会影响可执行文件的大小。两组实验得到以下结果：

| 编译环境 | 动态版 | 静态版 |
| --- | --- | --- |
| Debian、GCC 12.2.0、glibc 2.36 | 16,008 字节 | 762,520 字节 |
| Alpine、GCC 14.2.0、musl 1.2.5 | 18,408 字节 | 141,128 字节 |

这次生成的 musl 静态版比 glibc 静态版小，而 musl 动态版略大。表中的动态版大小只计算可执行文件，共享库保存在运行环境中。
