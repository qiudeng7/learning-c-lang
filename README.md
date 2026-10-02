# C 语言 Linux 系统编程

作为一个熟练的 python / typescript 开发者，我注意到我总是在使用别人提供的接口，要么是第三方库的接口，要么就是解释器提供的 builtin、标准库，解释器屏蔽了很多操作系统的细节，这和我在操作系统书中看到的并不太一致。我想知道操作系统到底给用户态程序提供了什么？经过了解，我从未接触过的“系统编程”可以解答我的这些问题。于是我尝试入门 C 语言 linux 系统编程，但是我此前从未接触过 C，除了 py 和 ts 以外的语言也只浅浅接触过 go 和 java，基于以上背景，我开始探索 C 语言 linux 系统编程。

本仓库是我对 C 语言系统编程的探索记录，主要学习方式是问题导向，因此内容和个人兴趣强相关，会有很多话题分叉，知识来源以 AI 为主，辅以网络文档资料和实验验证。

## 为什么选择 C 语言

创建这个仓库的时候其实没有仔细考虑这个问题，后来才发现其实 python 也提供了`import posix`，理论上通过 python 我也可以回答背景介绍中的问题。但毕竟其他语言都没有 C 这么原生，而且学 C 并非无用，从兴趣上说我很想对比 C 和我接触的现代语言来了解编程语言的发展，从实用角度来说很多 C 开发的项目以后我也可以尝试阅读了，所以我决定继续通过 C 来接触系统编程。

## C 语言官方网站和技术组织

我首先尝试接触接触 C 语言官方的相关网站：
- C 语言官网: https://www.c-language.org/ ，这里介绍了C 语言的生态和学习资源。
- C 语言国际标准委员会（WG14）： https://open-std.org/jtc1/sc22/wg14/ ，该组织负责制定C语言标准。

常见的技术组织之间是什么关系？该话题分叉我单独写到了[docs/tech-organization.md](./docs/tech-organization.md)，很短，全当科普。

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

在 linux 中进行 C 开发有两套可选的 C 工具链，一套是GCC，一套是clang+llvm，由于我学习 C 语言的目的是学 Linux 系统编程，GCC 会更合适。关于二者的具体选择可以见[选 GCC 还是 Clang](docs/gcc-vs-clang.md)，这个话题分叉比较简短，五分钟读完。

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

[`quickstart/main.c`](quickstart/main.c) 是第一个实验程序。进入目录并编译：

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

展开文档：[编译流程](docs/compilation-process.md)

## Todo

1. C 的一些基本语法和概念
2. C 的稍复杂项目如何组织（make）
3. 如果我们要自己写一个库该怎么做
4. posix 接口以及 linux 机制
