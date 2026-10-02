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

程序通常不会把所有代码都写在同一个源文件中。假设 `main.c` 调用了另一个文件提供的 `add`：

```c
// main.c
int add(int left, int right);

int main(void)
{
    return add(1, 2);
}
```

```c
// math.c
int add(int left, int right)
{
    return left + right;
}
```

分别编译时，`main.o` 只知道自己需要名为 `add` 的函数，`math.o` 则包含 `add` 的实现：

```text
main.o ──引用 add──┐
                   ├──链接器──→ 可执行文件
math.o ──定义 add──┘
```

链接器主要需要完成这些工作：

- 合并来自不同目标文件的代码段和数据段；
- 为函数、全局变量等符号寻找定义；
- 安排代码和数据的最终布局；
- 根据最终布局修正需要重定位的地址；
- 写入操作系统加载程序所需的信息。

在当前实验中，`main.o` 自己定义了 `main`，但它使用的 `printf` 等函数由 **C 标准库** 提供（具体实现会在 [libc](libc.md) 这个话题中说明）。可以用 `nm` 观察目标文件中的符号：

```bash
nm main.o
```

输出类似
```
0000000000000000 T main
                 U printf
                 U puts
```

`T main` 表示 `main` 定义在这个目标文件的代码段中；`U printf` 等记录表示该符号尚未定义，需要链接时从别处获得。

## 静态链接和动态链接

链接器获得库代码主要有两种方式。

### 静态链接

静态链接会从静态库中取出程序需要的代码，复制到最终的可执行文件中。Linux 静态库通常使用 `.a` 扩展名。

```text
程序的目标文件 ─┐
               ├──静态链接 ──> 一个包含所需代码的可执行文件
静态库 .a     ─┘
```

这样生成的程序对相应动态库的运行时依赖较少，但通常会带来这些代价：

- 每个可执行文件都可能保存一份相同的库代码；
- 可执行文件通常更大；
- 库修复后，程序通常需要重新链接才能获得新实现。

### 动态链接

动态链接不会在构建时把共享库的完整实现复制进可执行文件，而是在可执行文件中记录运行时需要的库和符号。

Linux 动态链接库通常使用 `.so`（shared object）扩展名，Windows 动态链接库通常使用 `.dll`（dynamic-link library）扩展名：

```text
Linux                         Windows

hello                         hello.exe
  └── 需要 libc.so.6            └── 需要某个 system.dll
        └── 提供 printf                └── 提供所需函数
```

Linux 启动动态链接程序时，大致会经历以下过程：

1. 内核读取可执行文件，创建进程并建立初始内存映射；
2. 内核根据可执行文件中的信息启动动态链接器；
3. 动态链接器加载程序依赖的 `.so`；
4. 动态链接器把程序中的外部符号引用绑定到实际实现；
5. 完成必要的重定位和初始化后，程序开始执行。

因此，可执行文件并不是因为动态链接才存在：程序既可以静态链接，也可以动态链接。`.so` 和 `.dll` 才是专门为共享代码和动态链接服务的文件。

## ELF 和 PE 格式

可执行文件和动态链接库都需要描述机器码、数据、内存布局、依赖、导入导出和重定位信息。因为它们有大量共同需求，操作系统通常使用同一套二进制文件格式表达它们。

Linux 等类 Unix 系统主要使用 ELF（Executable and Linkable Format）：

```text
ELF
├── main.o       可重定位目标文件
├── hello        可执行文件
└── libc.so.6    动态链接库
```

Windows 使用 PE/COFF 体系：

```text
PE/COFF
├── main.obj     COFF 可重定位目标文件
├── hello.exe    PE 可执行文件
└── example.dll  PE 动态链接库
```

因此，文件扩展名、文件用途和内部格式是三个不同概念：

| 维度 | 示例 |
|---|---|
| 链接方式 | 静态链接、动态链接 |
| 文件用途 | 目标文件、可执行文件、动态链接库 |
| 文件格式 | Linux 的 ELF、Windows 的 PE/COFF |

`.exe` 和 `.dll` 表示 Windows 文件的常见用途和命名约定，PE 才是它们内部采用的文件格式。同样，Linux 可执行文件、`.o` 和 `.so` 都采用 ELF 格式。

## 从 file 输出看链接结果

在 [quickstart](../README.md#quickstart和编译流程) 中，我们已经完成了四个阶段，并执行过 `file main.c main.i main.s main.o hello`。现在回到那份输出：

```text
main.c: C source, ASCII text
main.i: C source, ASCII text
main.s: assembler source, ASCII text
main.o: ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), not stripped
hello:  ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=2b150903791d5c6bc554cf735a4fe4d952b0fc3c, for GNU/Linux 4.4.0, not stripped
```

`main.c` 和 `main.i` 被识别为 C 源码，`main.s` 被识别为汇编源码，三者都是文本；`main.o` 和 `hello` 则都是 ELF 二进制文件。结合前面的格式说明，我们主要看这两个 ELF 文件在链接前后有什么变化。

它们共有的 `ELF 64-bit LSB` 表示采用 64 位 ELF 格式，数据按小端字节序存储；`x86-64` 表示面向的 CPU 架构。真正体现用途区别的是后面的 `relocatable` 和 `pie executable`：`main.o` 是可重定位目标文件，仍等待链接器组合代码、解决符号引用；`hello` 已经具有可执行程序的布局和入口。这里的 PIE（Position-Independent Executable，位置无关可执行文件）还允许程序在不同的加载基址运行。

`hello` 的 `dynamically linked` 表示这个程序采用动态链接，运行时需要加载共享库；后面的 `interpreter /lib64/ld-linux-x86-64.so.2` 则指出内核应启动哪个动态链接器，由它完成共享库加载等工作。这里的 interpreter 是 ELF 装载机制中的动态链接器，与 Python 的语言解释器职责不同。[Linux 动态链接器说明](https://man7.org/linux/man-pages/man8/ld.so.8.html)

现在我们知道了 `printf` 的声明来自头文件，目标文件里仍有对它的引用，链接后程序会依赖共享库。接下来要追问的是：这些库的具体实现从哪里来，静态链接时又有什么变化？继续阅读 [libc：标准库实现从哪里来](libc.md)，沿用 quickstart 的代码验证这两件事。
