# 第 1 章 · 计算机系统漫游

英文提要：A tour of the compilation pipeline, the ELF image, and Amdahl's law. One C program is preprocessor text, assembly, an object file, and finally a process with code, data, heap, and stack.

## 概念

- 编译系统四阶段：预处理、编译、汇编、链接
- ELF 目标文件与可执行文件（`readelf` / `objdump` / `nm`）
- 进程的地址空间：代码、已初始化数据、BSS、堆、栈
- 存储层次与缓存的存在（后续章节展开）
- 操作系统与硬件对程序的抽象
- Amdahl 定律：局部加速的整体上限

## 构建与运行

```bash
make -C chapters/ch01_tour
./chapters/ch01_tour/demo_hello
./chapters/ch01_tour/demo_amdahl 0.8 5
make -C chapters/ch01_tour pipeline
make -C chapters/ch01_tour elf
make -C chapters/ch01_tour test-solution
```

## 练习

| 文件 | 难度 | 要求 |
| --- | --- | --- |
| `exercises/ex_amdahl.c` | ★☆☆ | 实现加速比，以及为达到目标加速比所需的局部加速因子 |

`make test` 编译并运行你的代码；`make test-solution` 运行参考答案。

## 自测问题

1. `gcc -c` 和 `gcc` 不带 `-c` 的差别是什么？产物分别是什么文件？
2. 为什么全局未初始化变量占 BSS，而不是把一长串零写进可执行文件？
3. 若某函数只占运行时间的 20%，把它变成无限快，整体最多快多少？
4. `demo_hello` 打印的五个地址，从上到下（数值从高到低）通常是什么顺序？在你的机器上验证一次。
5. 预处理阶段展开的是什么，为什么 `.i` 文件往往比 `.c` 更长？
