# 选 GCC 还是 Clang

对于 C 语言，Linux 下有两套常用的开发套件，一套是 **GNU Compiler Collection, GCC**，一条是 **LLVM** 和 **Clang**。

GCC 是 GNU 提供的一整套成熟、多语言、跨架构的编译器集合，除了 C 还支持 C++、Fortran、Ada、Go、D 等，核心入口是一条 `gcc` 命令，比如 `gcc main.c -o main`（全拼 `gcc main.c --output main`），会直接执行完整的 C 编译流程。

LLVM 相当于一个接口设计良好的用来开发新语言的库，新语言可以拿来用，而 GCC 是一个完整的面向普通开发者的产品，如果有人想开发新的语言，GCC 并不方便复用。

很多上层产品都在使用 LLVM，Clang 是其中一套针对 C/C++/Objective-C 的编译器，其他常见的还有：

- Rust：rustc 前端负责 Rust 语法、类型系统、借用检查等，再将程序 lowering 到 LLVM IR，由 LLVM 完成优化和目标代码生成。
- Swift：Apple 主导的 Swift 编译器同样大量依赖 LLVM 后端完成优化和跨架构代码生成。
- Julia：使用 LLVM 做 JIT 编译和机器码生成，这也是 Julia 能兼顾动态语言体验与高性能的重要基础。
- Zig：自身实现语言前端，并可使用 LLVM 作为重要的代码生成后端之一。
- Kotlin/Native：把 Kotlin 编译成本地机器码时，底层也依赖 LLVM。
- 部分 GPU 编译器和 DSL：很多 CUDA、OpenCL、着色器、机器学习编译器或领域专用语言，也会复用 LLVM 的 IR、优化器或后端。


从使用体验上来讲，Clang 和 GCC 很多时候可以无缝替换，但还是有差异点：

1. 报错信息的风格不同，一般来讲 Clang 的可读性更好；
2. 二者支持的编译参数高度相似，但并不完全相同；
3. 对非标准 C 扩展的支持不同；
4. 优化结果和编译速度可能不同；
5. 周边工具有两套相似的生态。

另外由于 Linux 是 GCC 编译的，许多 Linux 程序用 GCC 编译会更保险一些。