# C 语言 Linux 系统编程

作为一个熟练的 python / typescript 开发者，我注意到我总是在使用别人提供的接口，要么是第三方库的接口，要么就是解释器提供的 builtin、标准库，解释器屏蔽了很多操作系统的细节，这和我在操作系统书中看到的并不太一致。我想知道操作系统到底给用户态程序提供了什么？经过了解，我从未接触过的“系统编程”可以解答我的这些问题。于是我尝试入门 C 语言 linux 系统编程，但是我此前从未接触过 C，除了 py 和 ts 以外的语言也只浅浅接触过 go 和 java，基于以上背景，我开始探索 C 语言 linux 系统编程。

本仓库是我对 C 语言系统编程的探索记录，主要学习方式是问题导向，因此内容和个人兴趣强相关，会有很多话题分叉，知识来源以 AI 为主，辅以网络文档资料和实验验证。

## 为什么选择 C 语言

创建这个仓库的时候其实没有仔细考虑这个问题，后来才发现其实 python 也提供了`import posix`，理论上通过 python 我也可以回答背景介绍中的问题。但毕竟其他语言都没有 C 这么原生，而且学 C 并非无用，从兴趣上说我很想对比 C 和我接触的现代语言来了解编程语言的发展，从实用角度来说很多 C 开发的项目以后我也可以尝试阅读了，所以我决定继续通过 C 来接触系统编程。

## C 语言官方网站和技术组织

我首先尝试接触接触 C 语言官方的相关网站：
- C 语言官网: https://www.c-language.org/ ，这里介绍了C 语言的生态和学习资源。
- C 语言国际标准委员会（WG14）： https://open-std.org/jtc1/sc22/wg14/ ，该组织负责制定C语言标准。

常见的技术组织之间是什么关系？该话题分叉我单独写到了[docs/tech-organization.md][]，很短，全当科普。

[docs/tech-organization.md]: ./docs/tech-organization.md

## 选择规范版本

C 语言版本众多，在目前的 2026 年，基本上以 C17 为蓝本来学，C23 的新增接口在平时遇到一个就学一个，对常见版本有一个如下印象即可

```
C89 / C90
C 第一次正式标准化。
大量经典 C 代码和历史兼容性问题可以追溯到这个时代。

C99
C 的第一次大规模现代化。
加入了很多后来成为现代 C 基础的能力，但当年的编译器和项目接受速度较慢，
因此很多项目长期停留在 C89/C90 风格。

C11
C 的第二次重要现代化。
尤其补充了原子操作、内存模型、线程等现代系统编程能力。
是现代 C 非常重要的一个基准版本。

C17
基本可以理解为 C11 的修订版。
几乎没有重要的新语言特性，主要修正和澄清 C11 的问题。
因此 C11/C17 经常可以视作同一个时代。

C23
新一轮较大的语言现代化，也是当前最新正式 C 标准。
加入 nullptr、typeof、constexpr、#embed 等大量新能力。
标准本身已经正式发布，但生态对新特性的支持和既有项目采用仍需要时间。

C2Y
下一版 C 标准，目前仍在制定中。
```

## quickstart和编译流程

在 linux 中进行 C 开发有两套可选的 C 工具链，一套是GCC，一套是clang+llvm，由于我学习 C 语言的目的是学 Linux 系统编程，GCC 会更合适。关于二者的具体选择可以见[选 GCC 还是 Clang][]，这个话题分叉比较简短，五分钟读完。

[选 GCC 还是 Clang]: docs/gcc-vs-clang.md

下面开始一个简单的 C 语言 quickstart，在 Arch Linux 中安装 GCC：

```bash
# 只安装 GCC
sudo pacman -S gcc

# 或者安装常用的基础开发工具，其中包含 GCC、make 等工具
sudo pacman -S base-devel
```

安装后查看 GCC 版本，确认命令可以正常执行：

```bash
gcc --version
```

[`quickstart/main.c`][] 是第一个实验程序。进入目录并编译：

[`quickstart/main.c`]: quickstart/main.c

```bash
cd quickstart
# 缩写参数
gcc -std=c17 -Wall -Wextra -Wpedantic main.c -o hello
# 全拼参数
gcc --std=c17 --warn-all --warn-extra --warn-pedantic main.c --output hello
```

我将从这里开始以后都同时提供 gcc 命令的缩写和全拼，以同时方便使用和记忆

这里的参数分别表示：

- `-std=c17`（全拼 `--std=c17`）：按照 C17 标准编译；
- `-Wall -Wextra -Wpedantic`（全拼 `--warn-all --warn-extra --warn-pedantic`）：开启一组常用警告，帮助发现可疑代码和非标准写法；
- `main.c`：输入的 C 源文件；
- `-o hello`（全拼 `--output hello`）：把生成的可执行文件命名为 `hello`。

运行生成的程序：

```bash
./hello

Hello, C!
40 + 2 = 42
```

我们可以展开了解一下中间的编译步骤，可以拆成预处理、编译、汇编和链接四个阶段：
```bash
# 一口气执行完，保留中间文件
# 缩写参数
gcc -std=c17 -Wall -Wextra -Wpedantic -save-temps main.c -o hello
# 全拼参数
gcc --std=c17 --warn-all --warn-extra --warn-pedantic --save-temps main.c --output hello

# 分步执行：
# 1. 预处理
# 缩写参数
gcc -std=c17 -E main.c -o main.i
# 全拼参数
gcc --std=c17 --preprocess main.c --output main.i

# 2. 编译
# 缩写参数
gcc -std=c17 -S main.i -o main.s
# 全拼参数
gcc --std=c17 --assemble main.i --output main.s

# 3. 汇编
# 缩写参数
gcc -c main.s -o main.o
# 全拼参数
gcc --compile main.s --output main.o

# 4. 链接
# 缩写参数
gcc main.o -o hello
# 全拼参数
gcc main.o --output hello
```

然后用 file 命令查看这些文件:

```bash
file main.c main.i main.s main.o hello

main.c: C source, ASCII text
main.i: C source, ASCII text
main.s: assembler source, ASCII text
main.o: ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), not stripped
hello:  ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=2b150903791d5c6bc554cf735a4fe4d952b0fc3c, for GNU/Linux 4.4.0, not stripped
```

可以看到前三个是文本格式，后两个是 ELF 二进制格式。

这个 quickstart 有很多门道可讲，我不想在 readme 中展开，我另写了一个知识稠密的文档，按照编译流程组织，仔细讲了预处理，简单介绍常用编译参数，最后在链接阶段解释了动态链接、静态链接，libc、glibc、musl，以及做了一个链接相关的小实验。

展开文档：[编译阶段][]

[编译阶段]: docs/compilation-stages.md

## demo: 实现一个简单 http 服务器

枯燥的语法手册就不看了，直接开始读demo，demo是纯AI写的一个简单的http服务器，当然这个demo要求读者了解http服务器是怎样工作的，这样我们的注意力才不会停留在具体的代码逻辑，如果你不了解 http 服务器的工作方式你可能更适合自己用 AI 写一个其他 demo。然后通过 demo 我们来重点从两个角度去读：

1. 一个多文件、有依赖库的 C 项目如何组织和构建
2. C 的一些语言设计

我让 gpt 分别用框架和手搓两种方式去写，demo在 [demo/simple-http-server][]，可以通过 readme 快速尝试一下这个项目。

[demo/simple-http-server]: demo/simple-http-server/README.md

### 构建

这个项目根目录通过 make 调用 docker 来编排构建流程，一般C、C++、Go 的项目会使用 make 或者 Cmake。Dockerfile 一共四个构建阶段，前两个是手搓版和框架版的build容器，都基于alpine构建musl静态链接版本，后两个是基于 scratch 的运行容器。

把 make 抽象过的命令拼接一下，可以发现最后对两个项目执行的gcc命令分别是:

```bash
# 手搓版，在 raw/ 中执行
gcc -std=c17 -Wall -Wextra -Wpedantic -g -O0 -static \
    main.c server.c http.c router.c \
    -o simple-http-server

# 框架版，在 framework/ 中执行两步
./prepare-onion.sh
gcc -I.deps/onion-src/src -I.deps/onion-build/src \
    -std=c17 -Wall -Wextra -Wpedantic -g -O0 -static \
    main.c app.c routes.c \
    .deps/onion-build/src/onion/libonion_static.a \
    -pthread -o simple-http-server
```

手搓版的参数我们基本都认识，主要陌生的地方在于它编译了很多文件，但只有一个输出 `simple-http-server`。为什么多个c文件可以只得到一个二进制而不是每个文件各自一个二进制？实际上在链接之前、汇编之后，确实是每个文件都各自得到一个二进制，在链接之前的阶段，每一个C文件的编译过程都只关心各自内部的内容，以及头文件中的接口和类型定义，编译器基本只检查接口能不能对得上。到了链接阶段，链接器才会把二进制文件互相对接起来（包括我们自己写的代码和其他库的代码），然后再设置 main 函数为整个程序的入口，这样多个文件才能合成一个二进制文件。

然后看框架版的构建流程，大体上分两步，一步是构建onion框架，一步是构建我们自己的应用。

prepare-onion.sh 脚本负责构建 onion 框架，该脚本分三步，第一步把 onion 框架源码克隆到 .deps/onion-src/，后两步主要是调用 onion 项目本身提供的构建命令，onion 原项目使用了 CMake，这是一个相比 make 的更上层的编排工具，它的大致使用逻辑是需要先生成一个 build 目录，然后在 build 目录内使用 make 执行构建（当然cmake也支持 make 以外的工具的编排，只是onion选择了make）。可以看到 prepare-onion.sh 的末尾有两个 cmake 命令，分别就是生成构建目录，以及构建一个 onion_static 的目标。onion_static 这个构建会把 onion 的源码都编译成机器码但是不进行链接，通过一个叫 ar 的工具把这些 .o 文件打包成 .a，后续我们自己的项目就可以利用 .a 来进行静态链接了，这一切都由 onion 的 cmake 配置文件提供，我们只需要知道如何使用 .a 来构建自己的应用即可。

然后看我们自己的应用如何构建，框架版的参数多了 `-I` 参数和 `.deps/onion-build/src/onion/libonion_static.a`，`-I` 参数是添加头文件的搜索目录，这里添加的是onion框架的头文件，第二个就和之前解释的一样了，提供 onion 构建出来的二进制文件给 gcc，链接器会自动链接他们。

大致流程如下图所示，[在 tldraw 查看][]

[在 tldraw 查看]: https://www.tldraw.com/p/criM_sSj38Kii_arhOJjS?d=v-18.-308.3554.2044.page

![alt text](<assets/README/shapes at 26-10-07 16.16.41.svg>)

### 语法和代码

这部分我们主要看一下 C 的语言设计，同样也是挑一些我感兴趣的讲。

下面是阅读代码的时候的一些简单的感受:


看到 `framework/main.c` 里有个的 `strtol`，这是 string to long 的意思，来自 `stdlib`，这个标准库提供了一些常用的类型处理、简单数学运算、读取环境变量等功能，在解释语言里，这类东西基本都 builtin 了。我们之前看到C在预处理之后的 .i 文件里也过了一遍 builtin，我们来看看 C builtin了哪些东西：

不 include 也不 import 就可以直接用的东西，在解释语言里是有一个明确的统一的解释器 builtin 的概念，但是 C 语言里需要区分不同的来源，有 C 规范和 GCC 扩展的的关键字和预定宏，关键字比如 `int`、`struct`、`if`，预定义宏比如 `__FILE__`（当前源文件名，C规范的）、`__LINE__` (当前行号，C规范的)、`__BYTE_ORDER__` (当前平台的字节序，GCC 扩展的)；还有一个来源是 [GCC 的内置函数][]，这个就比较类似解释器内置函数，比如 `__builtin_strlen(text)`（获取字符串长度），前面 .i 文件里面的 builtin 指的是 GCC 内置函数。

[GCC 的内置函数]: https://gcc.gnu.org/onlinedocs/gcc/Library-Builtins.html

对于 “解释语言里是有一个明确的统一的解释器 builtin 的概念” 这个细节需要补充解释一下，python 可以说是有统一的 builtin 来源的 [python的builtin参考][]，但是 js 由于他的生态也分 ECMA 标准和具体的实现，所以也需要区分规范定义的 [js 标准内置对象][] 和 [nodejs 全局对象][]，所以这个统不统一其实主要看生态统不统一，从 python 到 js，我们可以看到语言规范和解释器实现未必统一（当然 python 也有非官方实现，但是python的非官方实现的影响力和知名度都要比js生态中的bun和deno小的多，我一下子都不太能说出一些 python 非官方实现的例子），从 js 到 C 我们可以看到其实原来规范、标准库、编译器、链接器都是可以换的。

[python的builtin参考]: https://docs.python.org/3/builtins/index.html
[js 标准内置对象]: https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects
[nodejs 全局对象]: https://nodejs.org/api/globals.html


1. 函数前面加 static 表示无法被其他 c 文件链接，常见的设计。

2. onion 路由函数的开头都写了一句 `(void)context;`，这句话从语法上讲看起来做了一个类型转换，但其实只是因为我们没有用到 context，但是 onion 又实际传递了该参数，所以假装调用一下，避免编译器警告未使用该参数，没有实际作用。




## Todo

1. C 的稍复杂项目如何组织（make）
2. 如果我们要自己写一个库该怎么做
3. posix 接口以及 linux 机制
