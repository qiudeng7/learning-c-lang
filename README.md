# 学 C 语言

作为一个熟练的 python / typescript 开发者，我注意到我总是在使用解释器提供的接口，包括第三方库许多也是基于解释器接口开发的，解释器屏蔽了很多操作系统的细节，那么我想知道操作系统到底给用户态程序提供了什么？经过了解，该领域就是所谓的“系统编程”，于是我尝试入门 C 语言 linux 系统编程，但是我此前从未接触过 C，除了 py 和 ts 以外的语言也只浅浅接触过 go 和 java。

C语言官网: https://www.c-language.org/ , 这里介绍了C 语言的生态和学习资源。

C 语言国际标准委员会（WG14）： https://open-std.org/jtc1/sc22/wg14/ ，该组织负责制定C语言标准，关于这类技术组织的话题分叉我单独写到了[docs/tech-organization.md](./docs/tech-organization.md)

## 选择规范版本

C 语言版本众多，基本上以 C17 为蓝本来学，C23 的新增接口在平时遇到一个就学一个，对常见版本有一个如下印象即可

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

## quickstart

在 linux 中进行 C 开发有两套可选的 C 工具链，一套是GCC，一套是clang+llvm，由于我学习 C 语言的目的是学 Linux 系统编程，所以我选择安装 GCC，关于二者的具体选择可以见下方的[选 GCC 还是 Clang](#选-gcc-还是-clang)

在 Arch Linux 中安装 GCC：

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
gcc -std=c17 -Wall -Wextra -Wpedantic main.c -o hello
```

这里的参数分别表示：

- `-std=c17`：按照 C17 标准编译；
- `-Wall -Wextra -Wpedantic`：开启一组常用警告，帮助发现可疑代码和非标准写法；
- `main.c`：输入的 C 源文件；
- `-o hello`：把生成的可执行文件命名为 `hello`。

运行生成的程序：

```bash
./hello
```

预期输出：

```text
Hello, C!
40 + 2 = 42
```

当然上面是一口气执行完了gcc的四个步骤，你也可以把中间文件都展开，拆成预处理、编译、汇编和链接四个阶段。

有两种方式，一种是一口气执行完但是保留中间步骤 `gcc -std=c17 -Wall -Wextra -Wpedantic -save-temps main.c -o hello`，还有一种是分步执行，下面的每一步表示

```bash
# 1. 预处理
gcc -std=c17 --preprocess main.c --output main.i
# 2. 编译
gcc -std=c17 --assemble main.i --output main.s
# 3. 汇编
gcc -std=c17 --compile main.s --output main.o
# 4. 链接
gcc main.o --output hello
```

用file命令查看这些文件: `file main.c main.i main.s main.o hello`

输出：

```text
main.c: C source, ASCII text
main.i: C source, ASCII text
main.s: assembler source, ASCII text
main.o: ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), not stripped
hello:  ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=2b150903791d5c6bc554cf735a4fe4d952b0fc3c, for GNU/Linux 4.4.0, not stripped
```

对编译的具体流程说明很长，我们岔开一个话题在 [编译流程](docs/compilation-process.md) 中聊，同时也说明了这里为什么会有两个 ELF 文件但是具体跟随的信息不同。


### 选 GCC 还是 Clang

对于 C 语言，Linux 下有两套常用的开发套件，一套是 **GNU Compiler Collection, GCC**，一条是 **LLVM** 和 **Clang**

GCC 是 GNU 提供的一整套成熟、多语言、跨架构的编译器集合，除了C还支持C++、Fortran、Ada、Go、D等，核心入口是一条gcc命令，比如 `gcc main.c -o main`，会直接执行完整的C编译流程。

LLVM 相当于一个接口设计良好的用来开发新语言的库，新语言可以拿来用，而 GCC 是一个完整的面向普通开发者的产品，如果有人想开发新的语言，GCC并不方便复用。

很多上层产品都在使用LLVM，Clang是其中一套针对 C/C++/Objective-C 的编译器，其他常见的还有

- Rust：rustc 前端负责 Rust 语法、类型系统、借用检查等，再将程序 lowering 到 LLVM IR，由 LLVM 完成优化和目标代码生成。
- Swift：Apple 主导的 Swift 编译器同样大量依赖 LLVM 后端完成优化和跨架构代码生成。
- Julia：使用 LLVM 做 JIT 编译和机器码生成，这也是 Julia 能兼顾动态语言体验与高性能的重要基础。
- Zig：自身实现语言前端，并可使用 LLVM 作为重要的代码生成后端之一。
- Kotlin/Native：把 Kotlin 编译成本地机器码时，底层也依赖 LLVM。
- 部分 GPU 编译器和 DSL：很多 CUDA、OpenCL、着色器、机器学习编译器或领域专用语言，也会复用 LLVM 的 IR、优化器或后端。


从使用体验上来讲，Clang 和 GCC 很多时候可以无缝替换，但还是有差异点:

1. 报错信息的风格不同，一般来讲 Clang 的可读性更好
2. 二者支持的编译参数高度相似，但并不完全相同
3. 对非标准 C 扩展的支持不同
4. 优化结果和编译速度可能不同
5. 周边工具有两套相似的生态

另外由于 Linux 是 GCC 编译的，许多 Linux 程序用 GCC 编译会更保险一些。
