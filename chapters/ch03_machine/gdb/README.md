# gdb 走读

英文提要：`walk.gdb` stops at `main`, prints a backtrace and the registers, and disassembles a few instructions. It is a reading aid for the chapter demos.

## 怎么跑

先在本章目录编好 demo：

```bash
make
gdb -q -x gdb/walk.gdb ./demo_call
```

把 `./demo_call` 换成 `./demo_asm`、`./demo_switch` 或其他 demo 即可。脚本不写死文件名，被调试的程序由命令行传入。

## 脚本里的每一步

1. `set confirm off` 和 `set pagination off`：一次跑完，不因确认或翻页停住。
2. `set disassembly-flavor att`：用 AT&T 语法，和 `gcc -S`、`asm/add_loop.s` 一致。
3. `break main`：在 `main` 入口下断点。
4. `run`：启动进程，停在断点上。
5. `backtrace`：打印调用栈。刚进 `main` 时通常只有这一帧。
6. `info registers`：看通用寄存器。整数返回值以后会出现在 `%rax` / `%eax`。
7. `disassemble main`：反汇编整个 `main`。
8. `x/8i $pc`：从当前 `%rip` 起再看 8 条指令。
9. `quit`：退出。

## 交互时可以接着敲

脚本停在入口，适合自己再往下走。这些命令只是在读程序，不是在改控制流：

- `disassemble fact`：看递归函数里的 `push`、`call`、`ret`。
- `break fact` 之后 `run`，再用 `info frame` 看 `%rbp` 和返回地址。
- `finish`：跑完当前函数，返回值在 `%rax`。
- `disassemble /s main`：源码和汇编对照。本章 demo 带 `-g`。

System V AMD64 速记：整数参数顺序是 `%rdi %rsi %rdx %rcx %r8 %r9`，返回值在 `%rax`。demo 默认 `-O1`，很小的静态函数可能被内联。`make asm` 用 `-Og -fno-omit-frame-pointer` 生成 `build/*.s`，帧指针还在，更好读。
