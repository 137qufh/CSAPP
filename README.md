# Systems Craft · 计算机系统练习

英文提要：An original, chapter-by-chapter C and Linux workbook. It follows the same ideas people meet in a systems course — data, machine code, a small CPU, caches, linking, processes, virtual memory, I/O, networks, concurrency — with its own labs (`bitcraft`, `cachecraft`, `shellcraft`, `allocraft`, `proxycraft`, `asmcraft`) and a capstone that runs them as one machine. It does not contain textbook text, official lab handouts, or official starter code.

## 环境

Linux x86-64。需要 `gcc`、`make`、`gdb`、`objdump` / `readelf`（binutils）、`python3`。

```bash
bash scripts/check_env.sh
```

Mac 或 Windows 用容器：

```bash
docker build -t syscraft .
docker run --rm -it -v "$PWD":/work syscraft
```

`.devcontainer/` 指向同一份 Dockerfile。

## 快速开始

```bash
make            # 编译全部 demo、实验和 capstone
make test       # 与 make test-solution 相同：用参考答案跑通
make -C chapters/ch02_data test-solution
make -C labs/bitcraft test-solution
make -C capstone run
```

章节内部：

- `make test` 编译 `exercises/` 里的桩代码（一开始会失败，这是留给你的）
- `make test-solution` 编译 `solutions/` 并应通过

## 目录

| 路径 | 内容 |
| --- | --- |
| `common/` | 错误包装、缓冲 I/O、计时、minitest |
| `chapters/ch01_tour` … `ch12_concurrent` | 演示、练习、参考答案 |
| `labs/bitcraft` | 第 2 章位运算 |
| `labs/asmcraft` | 第 3 章 gdb 练习，源码只在 `solutions/` |
| `labs/cachecraft` | 第 6 章 cache 模拟器和转置 |
| `labs/shellcraft` | 第 8 章 shell |
| `labs/allocraft` | 第 9 章 malloc |
| `labs/proxycraft` | 第 11–12 章并发代理 |
| `capstone/` | 把各章收成一台可运行的小机器 |
| `docs/LEARNING_PATH.md` | 12 周节奏 |
| `docs/CONCEPTS.md` | 概念到文件 |
| `STATUS.md` | 完成了什么、哪里故意做小 |

根 Makefile 还有 `make ch04`、`make lab-shellcraft`、`make capstone` 这种目标。

## 学习顺序

先跑该章的 `demo_*`，再做 `exercises/`，最后才看 `solutions/`。路线和自测问题在每章 README（中文在前）和 `docs/LEARNING_PATH.md`。

代码里的名字和注释是英文，方便和编译器、调试器的输出对照。说明文档中文在前。
