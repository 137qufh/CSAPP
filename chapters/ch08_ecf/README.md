# 第 8 章 · 异常控制流

英文提要：Processes and signals move control off the straight path through `main`. These demos fork, exec, wait, inspect a short-lived zombie, block and accept `SIGUSR1`, and jump back with `siglongjmp`. Every child is reaped. Nothing sleeps longer than 20 ms.

## 概念

- 异常控制流：硬件中断、陷阱、故障，以及内核用信号转交给进程的那一部分
- `fork` 复制调用进程。父子从 `fork` 的返回处各自继续，返回值用来区分彼此
- `execve` 用另一个程序替换当前进程的地址空间，pid 不变
- `waitpid` 取走子进程的终止状态。在此之前，已退出的子进程是僵尸
- 父进程若先退出，子进程会被重新挂到 init 或子收割者。那是真正的孤儿。本目录的演示不制造这个竞态：父进程一直活着，只打印 `getppid`
- 处理器函数里只写 `volatile sig_atomic_t`。打印留给普通代码
- 信号被阻塞时仍可变成未决。`sigpending` 查看未决集合，`sigsuspend` 在解开阻塞的同时等待
- `setjmp` 第一次返回 0。`sigsetjmp` 还能保存信号掩码，`siglongjmp` 从保存点再次返回

## 构建与运行

```bash
make -C chapters/ch08_ecf
./chapters/ch08_ecf/demo_fork
./chapters/ch08_ecf/demo_exec
./chapters/ch08_ecf/demo_wait
./chapters/ch08_ecf/demo_zombie
./chapters/ch08_ecf/demo_orphan
./chapters/ch08_ecf/demo_signal
./chapters/ch08_ecf/demo_mask
./chapters/ch08_ecf/demo_suspend
./chapters/ch08_ecf/demo_setjmp
make -C chapters/ch08_ecf test-solution
```

`make test` 编译并运行练习里的桩代码。`make test-solution` 运行参考答案，并按退出码烟测每个演示。演示本身失败（非 0 退出）会让该目标失败。`demo_zombie` 在读不到 `/proc` 时打印 `proc-unavailable` 并仍然退出 0；读到了就打印 `state=Z` 或 `state=not-Z`，然后 `waitpid`。

## 练习

| 文件 | 难度 | 要求 |
| --- | --- | --- |
| `exercises/ex_forkcount.c` | ★★☆ | 三个 `fork` 写法，算出模式结束时一共有多少个进程。不要在函数里真的调用 `fork` |
| `exercises/ex_signals.c` | ★★★ | 阻塞 `SIGUSR1` 并 `raise`，用 `sigpending` 确认它未决，再恢复原来的掩码并清掉未决信号 |

## 自测问题

1. `fork` 之后，父子的局部变量一开始为什么值相同？之后修改为什么互不影响？
2. 子进程里用 `_exit` 而不是 `exit`，少做了哪一步？这和 `fork` 前忘记 `fflush` 有什么关系？
3. `demo_exec` 里如果两个 `execve` 都失败，子进程为什么必须 `_exit`，而不能直接 `return` 到父进程的代码后面？
4. 僵尸占用的是什么资源？父进程的哪一次调用把它清掉？
5. 为什么 `demo_orphan` 不让父进程先退出？若真的先退出，子进程的 `getppid` 会变成谁？
6. 信号处理函数里调用 `printf` 可能和主程序正在进行的 `printf` 撞车。本目录的处理器用什么办法避开这件事？
7. `demo_mask` 在安装处理器之前就 `raise`。进程为什么没有被 `SIGUSR1` 的默认动作杀掉？
8. `sigsuspend` 和“先 `sigprocmask` 解开阻塞，再调用 `pause`”相比，少了哪一个窗口？
9. `sigsetjmp` 的第二个参数非 0 时多保存了什么？`siglongjmp` 回到的是哪一次调用？
10. 连续三次无条件 `fork`，为什么到达后面语句的进程是 8 个，而不是 6 个？
