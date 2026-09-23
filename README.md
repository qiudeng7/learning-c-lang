# 学 C 语言

从一个熟练的 python / typescript 开发者视角开始学习C语言，先学语法，后续的目标是接触一些系统编程知识。

C语言官网: https://www.c-language.org/ , 这里介绍了C 语言的生态和学习资源。

C 语言国际标准委员会（WG14）： https://open-std.org/jtc1/sc22/wg14/ ， 该组织负责制定C语言标准。

## 关于技术组织

这个WG14的目录可以看出来他的组织树，JTC(Joint Technical Committee, 联合技术委员会) 是 ISO(International Organization for Standardization,国际标准化组织) 和 IEC(International Electrotechnical Commission,国际电工委员会)的交叉组织，JTC。


然后 JTC 下面再分SC（Subcommittee，分技术委员会）

```
ISO/IEC JTC 1
│
├── SC 22   编程语言、环境、系统软件接口
├── SC 27   信息安全、网络安全和隐私
├── SC 35   用户界面
├── SC 41   IoT / Digital Twin
```

C语言的标准制定工作就属于SC22，SC22的全名是 `Programming languages, their environments and system software interfaces`，即编程语言、其环境以及系统软件接口。

然后SC22下面分WG，Working Group，工作组，比如：

```
SC22
│
├── WG5
│   └── Fortran
├── WG14
│   └── C
├── WG21
│   └── C++
├── WG23
│   └── Programming Language Vulnerabilities
└── WG24
    └── Linux
```

还有一些我见过的其他技术组织，比如孵化k8s的CNCF，Linux基金会，ECMA的标准制定组织，他们和WG14是什么关系呢？

ISO、Ecma、W3C 属于标准组织，负责制定HTML规范、语言规范、Web API规范等；

而 Linux 基金会属于项目的治理组织，它为为开源项目提供治理、资金、法务、基础设施、社区等支持。

开源项目背后的治理组织也可能是商业化的，比如 Ubuntu 背后的治理者是商业公司 Canonical，Red Hat治理Fedora。

```
开源世界
│
├── Linux Foundation
│ 
│   ├── Linux Kernel
│   ├── Zephyr
│   ├── Yocto Project
│   └── ...
│ 
│   └── CNCF（Cloud Native Computing Foundation）
│     
│       ├── Kubernetes        容器编排
│       ├── containerd        容器运行时
│       ├── etcd              分布式 KV / K8s 核心依赖
│       ├── Prometheus        监控
│       ├── Envoy             网络代理
│       ├── CoreDNS           DNS
│       ├── Helm              K8s 包管理
│       ├── Argo              GitOps / Workflow
│       ├── Flux              GitOps
│       ├── Cilium            eBPF 网络
│       ├── Istio             Service Mesh
│       ├── OpenTelemetry     可观测性
│       ├── Harbor            镜像仓库
│       └── ...
│
├── Apache Software Foundation（ASF）
│ 
│   ├── Apache HTTP Server    Web Server
│   ├── Kafka                 消息 / 流平台
│   ├── Spark                 大数据计算
│   ├── Flink                 流计算
│   ├── Hadoop                大数据
│   ├── Maven                 Java 构建工具
│   ├── Tomcat                Java Web 容器
│   ├── Airflow               工作流调度
│   ├── Cassandra             分布式数据库
│   └── ...
│
├── Eclipse Foundation
│ 
│   ├── Eclipse IDE           Java IDE
│   ├── Temurin               OpenJDK 发行版
│   ├── Jetty                 Java Web Server
│   ├── Mosquitto             MQTT Broker
│   ├── GlassFish             Jakarta EE 实现
│   ├── Theia                 Web / Cloud IDE 平台
│   └── ...
│
└── SPI（Software in the Public Interest）
    ├── Debian                Linux 发行版
    ├── Arch Linux            Linux 发行版
    ├── Gentoo Linux          Linux 发行版
    ├── PostgreSQL            数据库
    ├── FFmpeg                音视频处理
    ├── LibreOffice           Office 套件
    ├── systemd               Linux 系统管理
    ├── OpenZFS               文件系统
    ├── OpenEmbedded          嵌入式 Linux 构建
    ├── Lua                   编程语言
    └── ...
```

这里提一下这个 SPI(Software in the Public Interest, 公共利益软件组织)，它是一个美国的一个非营利组织，主要为自由软件和开源项目提供法律、财务、资产管理等组织支持，SPI 并不强调某个技术领域，而是给各种独立开源社区提供一个可以处理捐款、资产、合同等事务的法律实体。

## 选择规范版本

C 语言版本众多，我和AI多轮对话之后，整理出我觉得需要了解的版本相关的常识如下，基本上以C17为蓝本来学，C23的新增接口在平时遇到一个就学一个。

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

## 编译器和工具链

对于C语言，Linux 下有两套常用的开发套件，一套是 **GNU Compiler Collection, GCC**，一条是 **LLVM** 和 **Clang**

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

## 快速开始: 编译器

由于我学习 C 语言的目的是学 Linux 系统编程，所以我选择GCC。

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

[`quickstart-1/main.c`](quickstart-1/main.c) 是第一个实验程序。进入目录并编译：

```bash
cd quickstart-1
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

如果把这条 GCC 命令拆成预处理、编译、汇编和链接四个阶段，可以使用 `file` 查看每个阶段产生的文件：

```bash
file main.c main.i main.s main.o hello
```

输出：

```text
main.c: C source, ASCII text
main.i: C source, ASCII text
main.s: assembler source, ASCII text
main.o: ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), not stripped
hello:  ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=2b150903791d5c6bc554cf735a4fe4d952b0fc3c, for GNU/Linux 4.4.0, not stripped
```

其中 `main.o` 和 `hello` 都采用 ELF 格式，但是file的输出结果有差异，关键就在于动态链接，详细关于链接的部分见[编译流程](docs/compilation-process.md)。

