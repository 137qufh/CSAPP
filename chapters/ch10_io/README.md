# 第 10 章 · 系统级 I/O

英文提要：File descriptors, short counts on read/write, file metadata, directory scans, dup2 redirection, and the gap between stdio buffering and the write system call.

## 概念

- 打开文件得到的是小整数描述符，不是 FILE 指针。read / write / close 直接对描述符操作。
- 一次 read 或 write 可以少传字节。管道容量、信号中断、磁盘边界都会造成短计数，调用方要用循环把逻辑上的 n 个字节补齐。
- 公共库里的 `sc_readn` / `sc_writen` 就是这种循环。本章的 `demo_short` 用它们把 100000 字节送过一根管道。
- `stat` 给出大小和类型。普通文件的 `st_size` 是字节数；目录的大小不是“里面文件的总长度”。
- `readdir` 不保证字母序。要稳定输出，就自己收集名字再 `qsort`。
- `dup2` 让两个描述符指向同一个打开文件。把目标接到标准输出后再 `exec`，新程序的 printf 就写进那个文件。
- `fwrite` 先进入用户态缓冲区。`fflush` 把缓冲区交给内核，`fsync` 再要求内核把数据落到设备上。`write` 不经过 FILE 缓冲区。

## 构建与运行

```bash
make -C common all
make -C chapters/ch10_io
./chapters/ch10_io/demo_rw
./chapters/ch10_io/demo_short
./chapters/ch10_io/demo_dir
make -C chapters/ch10_io test-solution
```

`demo_short` 链接 `common/libscraft.a`。其余演示只使用系统调用。

## 练习

| 文件 | 难度 | 要求 |
| --- | --- | --- |
| `exercises/ex_copy.c` | ★★☆ | `copy_file`：用 read/write 循环拷贝整个文件，短计数要重试，空文件也要得到空的目标 |

参考实现在 `solutions/ex_copy.c`。`make test` 编译练习草稿，`make test-solution` 编译参考实现并顺带跑一遍演示程序。

## 自测问题

1. 向一根几乎写满的管道请求写入 100000 字节时，`write` 为什么常常提前返回？返回值小于请求长度算错误吗？
2. 只调用一次 `read` 就认为“文件读完了”，会在什么输入上丢数据？
3. `dup2(fd, 1)` 之后，描述符 1 和 `fd` 指向的打开文件是同一个吗？只关闭 `fd` 时，已经写到标准输出的数据还在吗？
4. 比较 `fwrite` 和 `write` 的字节数之前，为什么要先 `fflush`？`fsync` 额外保证了哪一层？
5. 两次 `readdir` 扫同一个目录，名字顺序可以不一样。演示程序靠什么把输出稳定下来？
6. 对一个刚写入 11 字节的普通文件做 `stat`，`S_ISREG` 和 `st_size` 各自应该告诉你什么？
