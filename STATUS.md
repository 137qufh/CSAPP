# 实现状态

英文提要：What is complete enough to build and grade, and what is intentionally smaller than a full course lab.

## 已完成，`make test-solution` 会跑

- `common/`：错误包装、自写的缓冲 I/O、单调时钟、minitest
- 第 1–12 章：每章有中文说明、可运行 demo、带 TODO 的练习、参考答案、`make test` / `make test-solution`
- `labs/bitcraft`：30 道位运算 / binary32 题，`opcheck.py` 检查运算符
- `labs/asmcraft`：五阶段原创 defuse，源码只在 `solutions/`，发布二进制会 strip
- `labs/cachecraft`：组相联 LRU 模拟器 + 分块转置的缺失对比
- `labs/shellcraft`：解析器已给出；参考 `eval` 支持内建、管道、重定向、后台、`jobs`/`fg`/`bg`。没有控制终端时不做 `tcsetpgrp` 作业控制
- `labs/allocraft`：隐式空闲链表，边界标记，立即合并，`mm_check`，三条轨迹
- `labs/proxycraft`：每连接一线程的 HTTP/1.0 代理，8 项 LRU，只拨号 IPv4 数字地址
- `capstone/`：MY86 顺序核 + 两级页表/TLB + cache 统计 + 模块链接器 + allocraft 作为 guest 堆 + HTTP JSON/HTML。`make run` 即 `--demo`

## 故意做得比“完整课程实验”小的地方

- MY86 是顺序核在 capstone 里跑；五级流水线在第 4 章单独验证，不在 capstone 里执行会改寄存器的 `svc`
- shell 的作业控制在管道测试里是进程级 `waitpid`，不是一个完整的终端前台进程组
- cache 在 capstone 里只统计命中，不缓存数据；功能上每次仍访问后备存储
- 分配器是隐式链表，不是分离空闲链表
- 代理只实现 GET，不实现 CONNECT、持久连接或 HTTPS
- memory mountain 的图依赖本机有没有 matplotlib；没有就只留 CSV

这些都写在对应 README 里，不影响主路径的构建和测试。
