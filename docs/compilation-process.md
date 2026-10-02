# C 程序的编译流程

在 [README 的 quickstart](../README.md#quickstart和编译流程) 中，我们已经用 GCC 编译并运行了 `main.c`，保留了中间文件，再用 `file` 查看它们的类型。本文接着这次实验，解释预处理、编译、汇编和链接各自做了什么，以及相关参数和输出的含义。示例环境是 x86-64 Linux，工具链使用 GCC。

回看 quickstart 使用的这条命令，GCC 会把 C 源代码转换成 Linux 可以加载运行的程序：

```bash
# 缩写参数
gcc -std=c17 -Wall -Wextra -Wpedantic main.c -o hello
# 全拼参数
gcc --std=c17 --warn-all --warn-extra --warn-pedantic main.c --output hello
```

日常交流中，人们经常把整个过程统称为“编译”。严格区分时，这条命令实际上包含预处理、编译、汇编和链接四个阶段，`gcc` 是负责组织这些阶段的编译器驱动程序。

```text
main.c  C 源代码
  ↓ 预处理
main.i  展开后的 C 源代码
  ↓ 编译
main.s  汇编代码
  ↓ 汇编
main.o  可重定位目标文件
  ↓ 链接
hello   可执行文件
```

由于作者的兴趣倾向和能力限制，本文主要说明预处理和链接的环节，编译和汇编会说得简单一些。

## 预处理

示例代码中的 `#include` 是**预处理指令（preprocessor directive）**。**预处理器（preprocessor）**先把源文件识别为预处理记号，再执行宏展开、条件编译和包含头文件等操作；它不会像后续的编译器那样分析完整的 C 语法、检查类型或理解程序语义。

预处理指令以 `#` 开头，`#` 前可以有空白，例如 `#include`、`#define` 和 `#if`。默认情况下，代码中的注释也会从预处理输出中移除。

### 头文件和 #include

头文件通常使用 `.h` 作为文件扩展名，用来存放可以被多个源文件共享的声明，例如函数声明、类型定义、宏和常量声明。

头文件和 C 源文件并不是两种不同的语言：它们都可以包含 C 声明和预处理指令。区别主要来自用途约定，头文件通常保存供多个源文件共享的声明，而源文件通常保存函数实现。头文件也不一定能脱离包含它的源文件单独编译。

C 标准库提供了一些可以直接使用的头文件。例如，`stdio.h`（standard input/output）声明了输入输出相关的函数和类型，`printf` 的声明就来自这个头文件。


```c
#include <stdio.h>

int main(void) {
    printf("hello\n");
    return 0;
}
```

其他常见的标准库头文件包括：

- `stdlib.h`：动态内存分配、程序退出、数字转换等；
- `string.h`：字符串和内存区域操作，例如 `strlen`、`memcpy`；
- `stdint.h`：具有明确宽度的整数类型，例如 `int32_t`；
- `stddef.h`：`size_t`、`ptrdiff_t`、`NULL` 等基础定义；
- `errno.h`：错误码 `errno` 及相关定义。

使用尖括号的 `#include <stdio.h>` 通常表示从编译器和系统配置的头文件目录中查找；使用双引号的 `#include "calculator.h"` 通常表示优先从当前项目中查找。这是一种常见的查找约定，具体搜索路径还取决于编译器参数和构建环境。

**对比 `#include` 和模块机制**：

1. 可以把 `#include` 的效果近似理解为：在当前位置继续处理目标文件的内容，但仅仅只是词法层面的处理，而不像解释器语言可以现场执行。
2. `#include <stdio.h>` 会带来其中以及间接包含的头文件所提供的类型和声明。对当前程序来说，最关键的是获得 `printf` 的声明，告诉编译器它接受哪些参数、返回什么类型；`printf` 的具体机器码不会因此被复制到 `main.i` 中。
3. Python、JavaScript 等语言的 `import` 属于语言级模块机制，通常还涉及命名空间、导出项、模块加载和执行等规则；不同语言的具体规则并不相同，不能直接等同于 C 的文本包含。
4. `#include` 也不只是机械地复制最终文本：被包含的文件里还可能有宏、条件编译和其他 `#include`，预处理器会继续处理这些指令。

### 宏和条件编译

宏可以理解为预处理器执行的文本替换规则。`#define BUFFER_SIZE 1024` 定义的是一个没有参数的宏：预处理器遇到 `BUFFER_SIZE` 时，会把它替换成 `1024`，然后编译器再处理替换后的代码。

**宏**也可以定义参数，写法是在宏名后面紧跟括号和参数名。使用时虽然也写成类似函数调用的形式，但它实际进行的是代码片段替换。一个实际使用场景是计算数组中有多少个元素：

```c
#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

int values[] = {10, 20, 30};
size_t count = ARRAY_COUNT(values);  // 预处理后大致为 sizeof(values) / sizeof((values)[0])
```

这里的 `array` 是宏参数，不是函数参数；`ARRAY_COUNT(values)` 也不是一次函数调用，而是在编译前展开成一段表达式。这个宏的参数位于 `sizeof` 中，通常不会被求值。

更一般地说，如果某个函数式宏在展开结果中多次使用同一个参数，传入带副作用的表达式就可能产生意外结果。例如：

```c
#define MAX(left, right) ((left) > (right) ? (left) : (right))

int maximum = MAX(index++, 0); // index++ 可能被求值两次
```


**条件编译**让预处理器根据某个宏是否定义，决定一段代码是否交给编译器处理。例如，可以用它控制调试日志是否启用：

```c
#include <stdio.h>

#if defined(DEBUG)
#define LOG(message) fprintf(stderr, "debug: %s\n", (message))
#else
#define LOG(message) ((void)0)
#endif

int main(void) {
    LOG("program started");
    return 0;
}
```


没有定义 `DEBUG` 时，`LOG("program started")` 会被替换为空操作；使用 `gcc -DDEBUG main.c`（全拼 `gcc --define-macro=DEBUG main.c`）编译时，`LOG` 才会展开为输出调试信息的代码。`#if` 和 `#endif` 标记条件代码块，`#else` 提供条件不成立时的另一条分支；也可以使用 `#ifdef` 和 `#ifndef` 判断一个宏是否已经定义。条件编译常用于调试日志、平台差异和可选功能。

### 阅读 .i 文件

先在 `quickstart` 目录中生成 `.i` 文件：

```bash
# 缩写参数
gcc -std=c17 -E main.c -o main.i
# 全拼参数
gcc --std=c17 --preprocess main.c --output main.i
```

`.i` 文件的主体仍然是将要交给编译器处理的 C 代码，但其中还会出现行标记和 GCC 扩展等内容。它是预处理器交给编译器的结果，并不是只供人阅读的、纯粹的 ISO C 源文件。

我们的 `quickstart/main.c` 中只显式写了一个预处理指令 `#include`。在 `.i` 文件末尾仍能看到自己编写的代码；它前面的部分包括 GCC 自动加入的预处理环境，以及 `stdio.h` 和它间接包含的其他头文件的展开结果。


先看我们自己的部分，开头是一段行标记，下面是自己的代码：

```c
# 2 "main.c" 2


# 3 "main.c"
int main(void)
{
    int left = 40;
    int right = 2;

    printf("Hello, C!\n");
    printf("%d + %d = %d\n", left, right, left + right);

    return 0;
}
```

再搜索 `printf`，可以找到 `extern int printf(const char *__restrict __format, ...);`。这是一条函数声明，没有函数体，因此可以验证 `stdio.h` 提供的是 `printf` 的接口信息，而不是它的具体实现。

接着看行标记。GCC 使用下面的格式记录后续文本来自哪个源文件和哪一行：

```text
# 行号 "源文件名" 可选标志
```

例如：

```c
# 1 "/usr/include/stdio.h" 1 3
```

- 行号 `1`：后续内容来自 `stdio.h` 第 1 行；
- 标志 `1`：开始进入一个新文件；
- 标志 `3`：后续内容来自系统头文件。

标志的含义如下：

- `1`：进入一个新文件；
- `2`：从被包含的文件返回原文件；
- `3`：后续内容来自系统头文件；
- `4`：按隐式 `extern "C"` 处理，主要与 C++ 有关。

有些行标记没有任何标志，例如：

```c
# 3 "main.c"
```

它只是在更新来源位置，表示后续内容来自 `main.c` 第 3 行。下面这个行标记仍然带有标志 `3`，但没有表示进入或返回文件的标志 `1`、`2`：

```c
# 28 "/usr/include/stdio.h" 3
```

它表示后续内容来自系统头文件 `stdio.h` 第 28 行。中间一些内容可能因为条件编译、宏定义或其他预处理指令而没有直接出现在输出里。


`main.i` 开头有这样一段：

```c
# 1 "/usr/include/features-time64.h" 1 3
# 20 "/usr/include/features-time64.h" 3
# 1 "/usr/include/bits/wordsize.h" 1 3
# 21 "/usr/include/features-time64.h" 2 3
```

可以这样读：

1. 进入 features-time64.h
2. 后续来源位置切换到它的第 20 行
3. 它又包含了 bits/wordsize.h，于是进入 wordsize.h，一直处理到 wordsize.h 结束
4. 返回 `features-time64.h`，从第 21 行继续


接下来再看 `.i` 文件的开头：

```c
# 0 "main.c"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3
# 0 "<command-line>" 2
# 1 "main.c"
# 1 "/usr/include/stdio.h" 1 3
```

`<built-in>` 和 `<command-line>` 是 GCC 使用的逻辑来源名，不是磁盘上的真实文件。`<built-in>` 主要对应编译器预定义的宏，`<command-line>` 对应编译命令引入的宏和配置。`stdc-predef.h` 是当前 GCC 和 glibc 环境自动包含的预定义头文件；其他编译器和系统产生的文件头可能不同。

最后，在文件头和我们自己代码之间，主要是 `stdio.h` 及其间接包含的头文件展开出的函数声明、类型定义和结构体声明等内容。第一次阅读不需要逐行弄懂它们，可以像查找 `printf` 一样，根据自己使用的标识符反向寻找相关声明。

如果只想观察头文件包含关系，可以让 GCC 单独打印包含树：

```bash
# 缩写参数
gcc -std=c17 -E -H main.c -o /dev/null
# 全拼参数
gcc --std=c17 --preprocess --trace-includes main.c --output /dev/null
```

其中，`-E` 是 `--preprocess` 的缩写，表示只进行预处理；`-H` 是 `--trace-includes` 的缩写，表示打印头文件的包含关系。

输出类似:
```text
. /usr/include/stdio.h
.. /usr/include/bits/libc-header-start.h
... /usr/include/features.h
.... /usr/include/features-time64.h
..... /usr/include/bits/wordsize.h
..... /usr/include/bits/timesize.h
...... /usr/include/bits/wordsize.h
.... /usr/include/sys/cdefs.h
..... /usr/include/bits/wordsize.h
..... /usr/include/bits/long-double.h
.... /usr/include/gnu/stubs.h
..... /usr/include/gnu/stubs-64.h
.. /usr/lib/gcc/x86_64-pc-linux-gnu/16/include/stddef.h
.. /usr/lib/gcc/x86_64-pc-linux-gnu/16/include/stdarg.h
.. /usr/include/bits/types.h
... /usr/include/bits/wordsize.h
... /usr/include/bits/timesize.h
.... /usr/include/bits/wordsize.h
... /usr/include/bits/typesizes.h
... /usr/include/bits/time64.h
.. /usr/include/bits/types/__fpos_t.h
... /usr/include/bits/types/__mbstate_t.h
.. /usr/include/bits/types/__fpos64_t.h
.. /usr/include/bits/types/__FILE.h
.. /usr/include/bits/types/FILE.h
.. /usr/include/bits/types/struct_FILE.h
... /usr/include/bits/wordsize.h
.. /usr/include/bits/stdio_lim.h
.. /usr/include/bits/floatn.h
... /usr/include/bits/floatn-common.h
.... /usr/include/bits/long-double.h
Multiple include guards may be useful for:
/usr/include/bits/libc-header-start.h
/usr/include/bits/time64.h
/usr/include/bits/typesizes.h
/usr/include/features-time64.h
/usr/include/gnu/stubs-64.h
/usr/include/gnu/stubs.h
/usr/lib/gcc/x86_64-pc-linux-gnu/16/include/stddef.h
```


## 编译和汇编

编译器负责把C 代码生成汇编代码；汇编器再把汇编代码转换成机器码。编译器和汇编器的实现思路涉及多个领域，我们这里只简单提一下：

- **词法和语法分析**：识别代码中的名字、运算符和表达式，确定它们怎样组成程序。
- **语义分析与类型系统**：检查名字是否声明、操作是否符合类型规则、函数调用是否符合声明。
- **中间表示与优化**：把程序转换成便于分析和变换的形式，在保持语言规定行为的前提下调整代码。
- **体系结构与代码生成**：根据目标 CPU 的指令和寄存器，把程序转换成具体的汇编代码。

本节的主要话题是通过 gcc 参数来调整编译行为，可以结合之前的示例gcc编译命令一起来读

```bash
# 缩写参数
gcc -std=c17 -Wall -Wextra -Wpedantic main.c -o hello
# 全拼参数
gcc --std=c17 --warn-all --warn-extra --warn-pedantic main.c --output hello
```

我们可以通过参数选择 C 语言标准、启用警告检查，并决定警告是否让构建失败；也可以调整优化方式、保留调试信息，或只检查代码而不生成文件。下面按这六种用途分别说明。

### 语言标准

用 `-std`或`--std` 选择 GCC 使用的 C 语言模式。常见写法包括 `-std=c99`（`--std=c99`）、`-std=c11`、`-std=c17`和 `-std=c23`，项目要求哪个版本，就显式指定哪个版本；本文使用 `c17`。

如果代码使用 GNU 扩展，可以选择对应的 GNU 方言：模式名有 `gnu99`、`gnu11`、`gnu17` 和 `gnu23`，参数写法例如 `-std=gnu17`。

GNU 扩展是 GCC 在标准 C 之外提供的一组语言特性，例如 `typeof`、语句表达式 `({ ... })`、嵌套函数、`__attribute__` 和 `case 1 ... 5:` 这类写法。

详细可以了解：

- [GCC 的语言标准选项](https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html)
- [GCC 语言扩展参考](https://gcc.gnu.org/onlinedocs/gcc/C-Extensions.html)

### Warning相关

警告用于提示可能有问题的写法，最常用的是下面三个：

- `-Wall`：开启一批最常用、价值较高的警告。注意它不是 All warnings，并不会开启所有警告。
- `-Wextra`：在 -Wall 基础上再开启一批额外警告。
- `-Wpedantic`：对不符合当前 C 标准、依赖 GCC 扩展的代码给出警告。

其他可能会遇到的：

- `-Wconversion`：隐式类型转换可能改变值时警告，例如 long → int、有符号/无符号转换等。
- `-Wshadow`：局部变量遮蔽外层同名变量时警告。
- `-Werror`：把已有 warning 当成 error，这会在出现警告时阻止构建

详细可参考 [GCC 的警告选项](https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html)

### 优化选项

优化会改变生成代码的组织方式，例如提前计算常量表达式、删除不影响结果的计算，或把某些函数调用展开到调用处，常用的有：

- `-O0`（全拼 `--optimize=0`）关闭大多数优化，便于观察接近源码的执行过程；
- `-Og`（全拼 `--optimize=g`）启用适合调试的优化，在编译速度、调试体验和生成代码之间取得折中；
- `-O2`（全拼 `--optimize=2`）启用较多优化，常用于关注运行效率的构建。

优化通常会增加编译时间，也可能让源码与实际执行过程的对应关系变得不直观：某个变量可能被消除，多条语句可能合并，逐行调试时也可能出现跳行。优化等级更高不保证具体程序一定更快，效果需要测量；对具有未定义行为的代码，例如有符号整数溢出，也不能依赖它在某个优化等级下恰好表现正常。

详细可参考 [GCC 的优化选项](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html)

### 调试信息

`-g` 让 GCC 在产物中记录调试信息，一般不单独使用，要配合debugger来调试。[GCC 的调试信息选项](https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html)

### 只检查代码

`-fsyntax-only`（全拼 `--syntax-only`）让 GCC 只检查代码，不生成目标文件或可执行程序。虽然名字里有“syntax”，它也会进行相关语义检查，例如检查标识符是否声明、类型使用是否符合规则，并报告已启用检查发现的警告。修改代码后只想快速查看诊断时，可以把它与语言标准、警告选项组合使用。


详细可参考 [GCC 的诊断选项](https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html)

## 链接

之前我们在编译期只用到了依赖的头文件和接口声明，而真正的运行需要调用对应的实现，链接的作用是让我们的源码在运行时能找到相关的实现，分静态和动态两种方式，静态就是直接把依赖代码和自己的代码合并到同一个二进制里，动态就是把依赖编译成动态链接库，在运行的时候再去找动态链接库来调用，负责寻找程序依赖的动态链接库的程序叫做动态链接器。

我们以 C 标准库实现(通常称为 libc)为例来讲链接，一般解释语言程序想要使用标准库的话，都是解释器内置的直接 import 即可，但是 C 标准库需要链接，下面是常见的 libc 实现：

| 实现 | 常见环境 | 简要特点 |
| --- | --- | --- |
| [glibc](https://sourceware.org/glibc/) | linux 发行版大多采用 glibc | 提供 C 标准库，以及 POSIX、GNU 等接口 |
| [musl](https://wiki.musl-libc.org/) | Linux；Alpine 和 Void Linux 使用它 | 注重简洁和较小体积，也支持静态链接 |
| [Bionic](https://android.googlesource.com/platform/bionic/) | Android | Android 的 C 库，项目还包含数学库和动态链接器 |
| [UCRT（Universal C Runtime）](https://learn.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features?view=msvc-170) | Windows，现代 Microsoft C/C++ 工具链 | 提供 C 标准库函数及 Microsoft 扩展，是微软 C 运行时的组成部分 |
| [Apple Libc](https://github.com/apple-oss-distributions/Libc) | macOS | 提供 C 标准库功能，通过系统库 [libSystem](https://developer.apple.com/library/archive/documentation/Porting/Conceptual/PortingUnix/compiling/compiling.html) 与其他基础接口一起供程序使用 |
| [newlib](https://sourceware.org/newlib/info.html) | 嵌入式系统 | 面向嵌入式环境，底层服务需要与目标平台适配 |


其他系统的当做科普，我们主要聊一下 linux 用的 glibc 和 musl，他们都提供两样东西： C 标准库和 POSIX 接口，也都支持动态链接与静态链接，但差异如下：

| 对比项 | glibc | musl |
| --- | --- | --- |
| 设计侧重点 | 提供丰富的接口和扩展，并兼顾已有程序的兼容性 | 注重实现简洁、较小体积和标准接口的行为 |
| 扩展库接口 | 提供较多 GNU 扩展库接口，很多软件会依赖这些接口 | 提供部分 GNU 扩展库接口，但并不完整复现 glibc 的接口和行为 |
| 静态链接 | 支持，后面的实验会直接验证 | 同样支持，较小的实现规模是它用于静态程序的一个吸引点 |
| 动态链接器 | 常见路径是 `/lib64/ld-linux-x86-64.so.2` | 通常使用 `/lib/ld-musl-x86_64.so.1` |


下面我们来做一个小实验来具体感受一下 libc 和链接方式。

## libc 实验

本次实验可以直接通过一个 Bash 脚本完成，设计思路是用 docker 来启动三个构建环境 —— 只有 glibc(debian)，只有 musl(alpine)，同时有 glibc 和 musl(稍作处理的arch)，然后分别进行静态链接和动态链接的构建，其中 arch 同时进行 glibc 和 musl 的动静态构建，一共可以得到八个构建产物，然后分别在 debian/alpine/arch/scratch 环境尝试运行这八个产物，看能否运行成功。

可以直接执行 `bash experiments/libc/run.sh` 来运行实验(需要 docker 和 python3)，实验报告会输出到 experiments/libc/results/report.md 。

构建环境和产物如下表所示

| 构建环境 | 使用的 libc | 编译入口 | 链接方式 | 产物 |
| --- | --- | --- | --- | --- |
| Debian | glibc | `gcc` | 动态、静态 | `debian-glibc-dynamic`、`debian-glibc-static` |
| Alpine | musl | `gcc` | 动态、静态 | `alpine-musl-dynamic`、`alpine-musl-static` |
| Arch | glibc | `gcc` | 动态、静态 | `arch-glibc-dynamic`、`arch-glibc-static` |
| Arch | musl | `musl-gcc` | 动态、静态 | `arch-musl-dynamic`、`arch-musl-static` |

构建产物如下：

| 二进制 | 体积 | 动态链接器 | 共享库依赖 |
| --- | --- | --- | --- |
| debian-glibc-dynamic | 15.6 KiB | `/lib64/ld-linux-x86-64.so.2` | `libc.so.6` |
| debian-glibc-static | 744.6 KiB | 无 | 无 |
| alpine-musl-dynamic | 18.0 KiB | `/lib/ld-musl-x86_64.so.1` | `libc.musl-x86_64.so.1` |
| alpine-musl-static | 137.8 KiB | 无 | 无 |
| arch-glibc-dynamic | 15.6 KiB | `/lib64/ld-linux-x86-64.so.2` | `libc.so.6` |
| arch-glibc-static | 836.7 KiB | 无 | 无 |
| arch-musl-dynamic | 15.0 KiB | `/lib/ld-musl-x86_64.so.1` | `libc.so` |
| arch-musl-static | 38.6 KiB | 无 | 无 |

然后我们测试把这八个产物拿去运行，运行环境如下:

| 运行环境 | 提供的 libc |
| --- | --- |
| Debian | glibc |
| Alpine | musl |
| Arch | glibc 和 musl |
| scratch | 无共享 libc 和动态链接器 |

脚本在每个环境中运行八个程序，共检查 32 个结果。成功要求程序退出码为 0，并输出 `Hello, C!` 和 `40 + 2 = 42`；失败则记录退出码和错误信息。

| 二进制 | Debian | Alpine | Arch（两套 libc） | scratch |
| --- | --- | --- | --- | --- |
| debian-glibc-dynamic | 成功 | 失败 | 成功 | 失败 |
| debian-glibc-static | 成功 | 成功 | 成功 | 成功 |
| alpine-musl-dynamic | 失败 | 成功 | 成功 | 失败 |
| alpine-musl-static | 成功 | 成功 | 成功 | 成功 |
| arch-glibc-dynamic | 成功 | 失败 | 成功 | 失败 |
| arch-glibc-static | 成功 | 成功 | 成功 | 成功 |
| arch-musl-dynamic | 失败 | 成功 | 成功 | 失败 |
| arch-musl-static | 成功 | 成功 | 成功 | 成功 |


实验结果说明，静态链接的产物确实要比动态链接体积更大，因为依赖实现被我们嵌入进了二进制，相应的好处是我们的静态链接产物在任何环境都可以运行；动态链接产物体积更小，虽然可能会运行失败但同一个 linux 系统也可以同时装两套 libc。

基于本次实验，我进一步提出三个问题：
1. 为什么 arch 下的 musl static 比 alpine 的 musl static 体积更小，arch glibc static 产物体积却比 debian glibc static 大一些？
2. 我听说如果两个程序要动态链接的依赖版本不同，会容易产生冲突，动态链接器在这种情况下到底是如何工作的？
3. 如果我们自己用 C 写一个库，如何以静态链接或动态链接的形式给别人使用？

后两个问题有点脱离话题范畴，稍后我们会在其他文档解决这两个问题，先来关注第一个问题。
