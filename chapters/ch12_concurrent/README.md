# 第 12 章 · 并发编程

英文提要：Process-per-connection servers, select and epoll, threads, a mutex, a bounded buffer, a readers-writer lock, a prethreaded queue, and lock ordering that avoids deadlock. Tests stay on 127.0.0.1 and join every thread.

## 概念

- 每个连接 `fork` 一次时，父进程要回收子进程。`SIGCHLD` 里用 `waitpid(..., WNOHANG)` 循环收尸，避免僵尸。
- `select` 和 `epoll` 告诉你哪只描述符现在可读。它们是同步点，不靠 `sleep` 碰运气。
- 线程共享地址空间。互不重叠的数组片段可以各自求和，最后由 `pthread_join` 把结果收齐。
- `counter++` 不是一条原子指令。互斥量把“读-改-写”包成临界区，四个线程各加 10000 次必须得到 40000。
- 有界缓冲区用两个计数信号量表示空槽和满槽，再用一把锁保护下标。生产 1000 个数，消费端的校验和必须一致。
- `pthread_rwlock` 允许多个读者并存，写者独占。全部线程 join 之后，最终值不再变化。
- 预先创建的工作线程从队列里取已连接套接字。队列的互斥量和条件变量负责“有活干”和“有空位”，`accept` 留在主线程。
- 两把锁如果各按相反顺序获取，就会循环等待。按锁的地址从小到大获取，两边的顺序就一样了。

本章的网络演示只连接 `127.0.0.1`，端口用 0 让内核分配。每个这样的程序都装了 5 秒闹钟，并且会关掉自己打开的描述符。

## 构建与运行

```bash
make -C chapters/ch12_concurrent
./chapters/ch12_concurrent/demo_threads
./chapters/ch12_concurrent/demo_race
./chapters/ch12_concurrent/demo_prethread
make -C chapters/ch12_concurrent test-solution
```

链接时带 `-pthread`。不要在已经创建线程之后再 `fork` 出服务器；进程模型和线程模型分成不同的演示。

## 练习

| 文件 | 难度 | 要求 |
| --- | --- | --- |
| `exercises/ex_counter.c` | ★★☆ | `safe_counter_t`：`sc_add` / `sc_get` 在互斥量里改和读，四个线程各加 1000 后 `sc_get` 为 4000 |

## 自测问题

1. 为什么 `counter++` 写在一条 C 语句里，两个线程仍然可能丢掉更新？丢掉的是哪一次内存写入？
2. `SIGCHLD` 处理函数里为什么要用 `WNOHANG`，并且要循环到 `waitpid` 说没有孩子为止？
3. 生产者看见 empty 信号量为 0 时，它停在哪个调用里？此刻环形缓冲区还有空槽吗？
4. `select` 返回时你知道的是“可以读了”还是“已经把字节读进来了”？`epoll` 对监听套接字的回答有什么不同？
5. 预先创建线程的服务器里，如果工作线程自己去 `accept`，主线程还要不要把连接放进队列？本章的写法把哪一步留在主线程？
6. 线程 A 先锁地址较小的互斥量，线程 B 也按同样的地址顺序锁，循环等待还成立吗？
7. 读者锁允许几个读者同时进来？写者进来时，已经在读的读者要先怎样？
