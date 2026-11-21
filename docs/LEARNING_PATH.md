# 学习路径

英文提要：Twelve chapters, one lab where the ideas become a program you can run, then a capstone that wires the machine together. A steady pace is one chapter a week plus its lab in the weeks that have one. Do the demos before the exercises. Check the reference with `make test-solution` only after you have tried.

## 每周

| 周 | 目录 | 做到这里算过关 |
| --- | --- | --- |
| 1 | `chapters/ch01_tour` | 能自己讲清预处理、编译、汇编、链接；Amdahl 练习通过 |
| 2 | `chapters/ch02_data` + `labs/bitcraft` | 溢出判断和浮点位模式练习通过；bitcraft 功能测试和 opcheck 都过 |
| 3 | `chapters/ch03_machine` + `labs/asmcraft` | 能用 gdb 看栈和跳转表；asmcraft 五关全清 |
| 4 | `chapters/ch04_arch` | 能读 MY86 汇编，说清 load-use 停顿和分支冲刷；`demo_pipe` 的直线代码周期数 ≈ 指令数 + 4 |
| 5 | `chapters/ch05_opt` | 三种求和在所有样例上结果一致；能解释为什么双累加器减少依赖链 |
| 6 | `chapters/ch06_memory` + `labs/cachecraft` | 行优先和列优先的校验和相同；cachecraft 的命中/缺失/替换与参考一致 |
| 7 | `chapters/ch07_linking` | 静态库、共享库、`dlopen`、三种插桩都能跑出预期字符串 |
| 8 | `chapters/ch08_ecf` + `labs/shellcraft` | fork 题的进程数正确；shell 能跑管道、重定向和 `cd` |
| 9 | `chapters/ch09_vm` + `labs/allocraft` | 能从地址里拆出 VPN；allocraft 三条轨迹利用率 ≥ 0.50 且 `mm_check` 通过 |
| 10 | `chapters/ch10_io` | `copy_file` 能拷贝空文件和大文件 |
| 11 | `chapters/ch11_network` | 本机 echo 和静态页、CGI 风格动态页都返回预期正文 |
| 12 | `chapters/ch12_concurrent` + `labs/proxycraft` | 计数器在 4 个线程下精确；代理对同一 URL 只访问源站一次 |
| 收束 | `capstone` | `make -C capstone run` 打印 `demo-ok`，并且你能指出每个章对应哪一个源文件 |

## 怎么用参考答案

练习的空函数在 `exercises/`。参考在 `solutions/`。章节里的 `make test` 编译你的文件，`make test-solution` 编译参考。仓库根上的 `make test` 为了让克隆下来就能验证，跑的是参考答案。

先写，再看参考。看的时候对差异，不要整文件覆盖。
