# 表达式与控制流

先从简单的开始，条件、循环、除法、自增：

```c
#include <stdio.h>

int main(void)
{
    int total = 0;

    for (int i = 1; i <= 3; i++) {
        // 整数除法向零截断：
        // 1 / 2 = 0;
        // 2 / 2 = 1;
        // 3 / 2 = 1;
        // 如果是 -3 / 2 会得到 -1 而不是 -2
        total += i / 2;
    }

    if (total > 5) {
        printf("total = %d\n", total);
    } else {
        printf("too small, total = %d\n", total); // total = 2
    }

    // 1. 运算类型由操作数决定，赋值目标不会改变除法规则
    double a = 3 / 2;         // 1.0：先做整数除法，再转成 double
    double b = 3 / 2.0;       // 1.5：浮点除法
    double c = (double)3 / 2; // 1.5：先转换一个操作数
    printf("a = %.1f, b = %.1f, c = %.1f\n", a, b, c);

    // 2. 取余与整数除法配套
    int d = 7 % 3;           // 1
    int e = -7 % 3;          // -1：非零余数与被除数同号
    printf("d = %d, e = %d\n", d, e);

    // 3. 条件判断：0 为假，非零为真
    if (-3) {
        printf("yes\n");     // 会执行
    }

    // 4. 比较和逻辑运算的结果是 int，值为 0 或 1
    int f = (3 > 2);         // 1
    int g = !42;             // 0
    printf("f = %d, g = %d\n", f, g);

    // 5. && 和 || 会短路，写法和 TS 相似，但结果不是操作数
    int h = 5 && 8;          // 1；TS 中得到 8
    int j = 0 || 8;          // 1；TS 中得到 8
    printf("h = %d, j = %d\n", h, j);

    // 6. 自增表达式的值
    int n = 3;
    int old = n++;           // old = 3，n = 4
    int next = ++n;          // next = 5，n = 5
    printf("old = %d, next = %d, n = %d\n", old, next, n);

    return 0;
}
```

## 数字类型

C 的基本类型中除了 int 还有一个 `unsigned int` 无符号整数：

| 类型 | 取值范围 |
| --- | --- |
| `int` | −2³¹ ～ 2³¹−1 |
| `unsigned int` | 0 ～ 2³²−1 |

两种类型占用同样的空间，无符号类型把所有位用于表示非负数，因此能表示更大的正整数。常见环境中的有符号整数使用补码，不能简单理解成一个符号位加若干位数值。

但 C 没有规定 `int` 必须是 32 位，如此设计是为了适应不同硬件的整数运算能力。普通的小整数可以用 `int`；如果文件格式或网络协议要求精确的位宽，可以使用 `<stdint.h>` 提供的类型：

```c
#include <stdint.h>

int32_t temperature = -10; // 有符号，精确 32 位
uint32_t count = 100;      // 无符号，精确 32 位
```

这些精确位宽类型由能够支持它们的平台提供；目前使用的 Linux 环境支持它们。

### 类型转换

赋值可以发生隐式转换；也可以用 `(类型)` 显式转换表达式：

```c
int a = 3.9;             // 3：在可表示范围内，浮点转整数向零截断
int b = -3.9;            // -3
double c = (double)3 / 2; // 1.5：先把 3 转成 double，再做除法
```

混合类型运算也会转换操作数，尤其要留意有符号与无符号整数：

```c
int negative = -1;
unsigned int positive = 1;
int less = negative < positive; // 0，表示假
```

在这个比较中，`negative` 转成了 `unsigned int`。`-1` 转换后的值是该类型的最大值，因此比 `1` 大。如果 `unsigned int` 是 32 位，相当于比较 `4294967295 < 1`。

### 超出表示范围会怎样？

不同类型的规则不同：

| 情况 | 结果 |
| --- | --- |
| 有符号整数运算溢出 | 未定义行为，不能依赖结果回绕 |
| 无符号整数运算超出范围 | 按该类型最大值加一取模，发生回绕 |
| 浮点数无法精确表示某个值 | 保存近似值，计算可能产生舍入误差 |

```c
#include <limits.h>

unsigned int wrapped = UINT_MAX;
wrapped += 1; // 0：无符号整数回绕

// int overflow = INT_MAX + 1; // 有符号整数溢出，不是可靠的回绕方式

double sum = 0.1 + 0.2;
printf("%.17f\n", sum); // 常见环境中输出 0.30000000000000004
```


### 位运算

位运算直接操作整数的二进制位，常用于组合、检查和修改标志。

```c
unsigned int a = 6; // 二进制 0110
unsigned int b = 3; // 二进制 0011

unsigned int c = a & b;  // 0010，即 2：两位都是 1 才得到 1
unsigned int d = a | b;  // 0111，即 7：至少一位是 1 就得到 1
unsigned int e = a ^ b;  // 0101，即 5：两位不同才得到 1
unsigned int f = ~a;     // 所有位翻转，包括上面没写出的高位

unsigned int g = a << 1; // 1100，即 12：左移一位，低位补 0
unsigned int h = a >> 1; // 0011，即 3：无符号整数右移，高位补 0
```

一个实际用途是权限标志：

```c
unsigned int READ  = 1u << 0; // 001；u 表示无符号整数常量
unsigned int WRITE = 1u << 1; // 010
unsigned int EXEC  = 1u << 2; // 100

unsigned int permissions = READ | WRITE; // 011：可读、可写

if ((permissions & READ) != 0) {
    printf("can read\n");
}

permissions |= EXEC;   // 添加执行权限：111
permissions &= ~WRITE; // 移除写入权限：101
permissions ^= READ;   // 翻转读取权限：100
```


## 赋值表达式

C 的赋值不仅修改变量，还会产生一个值：赋值后左边变量的值。这一点与 TS 类似。

```c
int x;
int y;

y = (x = 3); // x = 3，y = 3
```

因此下面的代码合法，但容易是笔误：

```c
if (x = 3) {              // 赋值，不是比较
    printf("always\n");  // x 被设为 3，非零为真
}
```

比较应该写 `x == 3`。开启 GCC 警告通常能发现这种可疑写法。

赋值表达式也有实用场景，例如读取字符：

```c
int ch;

while ((ch = getchar()) != EOF) {
    putchar(ch);
}
```

`getchar`、`putchar` 和 `EOF` 来自 `<stdio.h>`。`getchar` 返回读到的字符值，输入结束或读取出错时返回 `EOF`；用 `int` 保存结果，以便区分字符值和 `EOF`。

每次循环依次完成：读取字符、保存到 `ch`、判断结果是否为 `EOF`。

这里的括号很重要：

```c
ch = getchar() != EOF;   // 相当于 ch = (getchar() != EOF)
```

这样保存的是比较结果 `0` 或 `1`，而不是读到的字符。

## goto

`goto` 跳到同一个函数内的指定标签，标签写成 `名字:`：

```c
#include <stdio.h>

int main(void)
{
    printf("before\n");

    goto finish;

    printf("skipped\n"); // 不会执行

finish:
    printf("after\n");
    return 0;
}
```

Python 和 TS 有 `try/finally` 和异常机制。C 没有这些语法，常用 `goto` 将失败路径汇集到资源清理处：

```c
#include <stdio.h>

int main(void)
{
    int status = 1;
    FILE *file = fopen("data.txt", "w");

    if (file == NULL) {
        return status; // 没有打开文件，无需关闭
    }

    if (fputs("hello\n", file) == EOF) {
        goto cleanup; // 写入失败，仍然需要关闭文件
    }

    status = 0;

cleanup:
    if (fclose(file) == EOF) {
        status = 1;
    }

    return status;
}
```

暂时把 `FILE *` 理解为“已打开文件的句柄”即可。`fopen` 的 `"w"` 模式创建文件，已有文件会被清空；`fputs` 写入文本，`fclose` 关闭文件。

这个例子的重点是：成功和失败路径都能经过同一处清理代码，减少重复和遗漏。
