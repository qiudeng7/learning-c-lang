# 技术组织 

话题由 C 语言的标准制定组织展开：

C 语言国际标准委员会（WG14）： https://open-std.org/jtc1/sc22/wg14/ ， 该组织负责制定C语言标准。

## C 语言标准制定组织

这个 WG14 的目录可以看出来他的组织树，JTC(Joint Technical Committee, 联合技术委员会) 是 ISO(International Organization for Standardization,国际标准化组织) 和 IEC(International Electrotechnical Commission,国际电工委员会)的交叉组织，JTC。

然后 JTC 下面再分SC（Subcommittee，分技术委员会）

```
ISO/IEC JTC 1
│
├── SC 22   编程语言、环境、系统软件接口
├── SC 27   信息安全、网络安全和隐私
├── SC 35   用户界面
├── SC 41   IoT / Digital Twin
```

C语言的标准制定工作就属于 SC22，SC22的全名是 `Programming languages, their environments and system software interfaces`，即编程语言、其环境以及系统软件接口。

然后 SC22 下面分 WG(Working Group, 工作组)，比如：

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

几个 WG 没有公开具体的人数，但是我们可以看到参会人员数量，Fortran 和 C的一般是二十到五十个人，C++ 的有一两百人参会。

## 其他组织

还有一些我见过的其他技术组织，比如孵化 k8s 的 CNCF，Linux 基金会，ECMA 的标准制定组织，他们和 WG14 是什么关系呢？

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
│   ├── ...
│   └── CNCF（Cloud Native Computing Foundation）
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
