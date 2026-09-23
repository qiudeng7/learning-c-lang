# C 程序的编译流程

执行下面这条命令时，GCC 会把 C 源代码转换成 Linux 可以加载运行的程序：

```bash
gcc -std=c17 -Wall -Wextra -Wpedantic main.c -o hello
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

本文使用 [`quickstart-1/main.c`](../quickstart-1/main.c) 进行实验。下面的命令都在 `quickstart-1` 目录中执行：

```bash
cd quickstart-1
```

## 预处理

预处理器先处理以 `#` 开头的指令：

- 将 `#include <stdio.h>` 替换为对应头文件的内容；
- 展开 `#define` 定义的宏；
- 根据 `#if`、`#ifdef` 等条件决定保留哪些代码；
- 移除注释。

只执行预处理：

```bash
gcc -std=c17 -E main.c -o main.i
```

`main.i` 仍然是 C 代码，可以直接作为文本查看。它通常比 `main.c` 大很多，因为其中已经展开了 `stdio.h` 及其间接包含的其他头文件。

头文件通常只提供声明。例如，`stdio.h` 告诉编译器 `printf` 接受哪些参数、返回什么类型，但 `printf` 的具体机器码并不会因为 `#include <stdio.h>` 就被复制到 `main.i` 中。

## 编译

编译器读取预处理后的 C 代码，完成语法分析、类型检查和必要的优化，然后生成面向当前 CPU 架构的汇编代码：

```bash
gcc -std=c17 -S main.i -o main.s
```

`main.s` 是文本文件，其中包含 x86-64 汇编指令。它已经非常接近机器指令，但仍使用助记符、标签和符号名称方便工具处理。

这里的“编译”是狭义的编译阶段。它和日常所说的“编译整个程序”不是同一个范围。

## 汇编

汇编器把汇编代码转换成机器码，并将机器码和相关元数据写入目标文件：

```bash
gcc -c main.s -o main.o
```

`main.o` 已经包含 CPU 可以执行的机器指令，但它还是一个半成品，不能直接作为程序运行。使用 `file` 可以看到它是一个可重定位目标文件：

```bash
file main.o
```

输出类似：

```text
main.o: ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), not stripped
```

其中 `relocatable` 表示“可重定位”。目标文件里的代码和数据还没有最终运行地址，对外部函数的引用也可能没有解决，这些信息要留给链接器处理。

## 为什么还需要链接

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

在当前实验中，`main.o` 自己定义了 `main`，但它使用的 `printf` 等函数由 C 标准库提供。可以用 `nm` 观察目标文件中的符号：

```bash
nm main.o
```

输出中的 `T main` 表示 `main` 定义在这个目标文件的代码段中；`U printf` 等记录表示该符号尚未定义，需要链接时从别处获得。具体符号可能因编译器版本和优化行为而略有不同。

## 静态链接和动态链接

链接器获得库代码主要有两种方式。

### 静态链接

静态链接会从静态库中取出程序需要的代码，复制到最终的可执行文件中。Linux 静态库通常使用 `.a` 扩展名。

```text
程序的目标文件 ─┐
                 ├──静态链接──→ 一个包含所需代码的可执行文件
静态库 .a      ─┘
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

## ELF、PE与文件用途

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

`.exe` 和 `.dll` 表示 Windows 文件的常见用途和命名约定，PE 才是它们内部采用的文件格式。同样，Linux 可执行文件、`.o` 和 `.so` 都可能采用 ELF 格式。

## 链接当前程序

把 `main.o` 链接成可执行文件：

```bash
gcc main.o -o hello
```

这里仍然使用 `gcc`，因为它会替我们传入 C 程序所需的启动文件、C 标准库和其他默认链接参数。也可以直接调用 `ld`，但那需要手动提供这些细节，不适合作为第一次链接实验。

查看最终文件：

```bash
file hello
```

输出类似：

```text
hello: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, ...
```

它与 `main.o` 的信息不同：

- `main.o` 是 `relocatable`，等待链接器安排地址和解决符号；
- `hello` 是 `pie executable`，已经具备可执行程序的布局和入口；
- `dynamically linked` 表示它运行时还需要动态链接库；
- `interpreter` 指动态链接器，不是 Python 那种语言解释器。

## 检查 ELF 文件

`readelf` 可以读取 ELF 内部信息。先比较目标文件和可执行文件的类型与入口地址：

```bash
readelf -h main.o | grep -E 'Type:|Entry point'
readelf -h hello | grep -E 'Type:|Entry point'
```

在本实验中，关键结果是：

```text
main.o  Type: REL   Entry point: 0x0
hello   Type: DYN   Entry point: 非零地址
```

`REL` 表示可重定位目标文件。现代 Linux 发行版通常默认生成 PIE（Position-Independent Executable，位置无关可执行文件）；PIE 在 ELF 头中使用 `DYN` 类型，使操作系统可以配合 ASLR 将程序加载到随机地址。这里的 `DYN` 不代表 `hello` 是一个 `.so`，还需要结合 ELF 的其他信息判断文件用途。

继续查看动态链接信息：

```bash
# 查看内核应该启动哪个动态链接器
readelf -l hello | grep interpreter

# 查看程序声明依赖的共享库
readelf -d hello | grep NEEDED
```

在当前环境中，可以看到动态链接器 `/lib64/ld-linux-x86-64.so.2`，以及共享库 `libc.so.6`。从构建到运行的关系最终变成：

```text
main.c
  ↓ 预处理、编译、汇编
main.o
  ↓ 链接
hello
  ↓ Linux 内核启动，动态链接器加载依赖
libc.so.6 等共享库
  ↓
程序开始执行
```

需要注意，ELF 中记录的程序入口通常不是 C 源码里的 `main`。系统会先从 C 运行时提供的启动代码进入，完成运行环境初始化，再调用 `main`。
