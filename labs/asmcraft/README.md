# asmcraft · 五阶段阅读练习（第 3 章）

英文提要：An original five-phase reading puzzle. `defuse` is a small program we wrote; the assigned file is the stripped binary. Recover five lines of input with gdb and objdump. The C source under `solutions/` is for the instructor.

## 目标

`defuse` 从标准输入读 **5 行**，每行对应一个阶段。程序不打印提示符。

- 某一行符合该阶段的隐藏规则时，打印 `phase N cleared`
- 第一次不符合时，打印 `phase N failed`，进程以状态码 1 退出
- 五行全部通过后，再打印 `all phases cleared`，状态码 0

请阅读我们自己的这个小程序，把五行答案写进 `exercises/answers.txt`。不要去找现成的实验手册，这个谜题的规则只存在于二进制里。

`solutions/defuse.c` 和 `solutions/answers.txt` 是教师用的。作业对象是去掉符号的 `defuse`，请直接读它。

## 评分

把 `exercises/answers.txt` 整份送进 `defuse`（也就是测试脚本做的那次运行）。五行都打印 `phase N cleared`，并且最后一行是 `all phases cleared`、退出码为 0，记 **100** 分。否则记 0 分。

## 构建与运行

```bash
make -C labs/asmcraft
./labs/asmcraft/defuse < labs/asmcraft/exercises/answers.txt
make -C labs/asmcraft test            # 用你的 answers.txt，没解出来时会失败
make -C labs/asmcraft test-solution   # 教师核对
```

`make` 生成两个文件：

| 文件 | 给谁 | 怎么编的 |
| --- | --- | --- |
| `defuse` | 学生 | `-O1`，再 `strip` 掉符号 |
| `defuse_debug` | 教师 | `-g -O0`，保留符号 |

## 用 objdump 和 gdb 读

先看控制流，确认 `main` 里五次检查的顺序：

```bash
objdump -d -M att defuse | less
```

符号已经被剥掉，函数名不会出现。对照调用、比较和间接跳转，把五个阶段分开。教师如果要对着名字看，用 `defuse_debug`：

```bash
objdump -d defuse_debug | less
gdb -q ./defuse
```

gdb 里常用的只是观察命令：

```
break main
run < exercises/answers.txt
disassemble
info registers
x/16i $pc
x/s $rdi
x/8gx 地址
```

`x/s` 用来看某个寄存器指向的字符，`x/wx` 或 `x/gx` 用来看表里的一个字。看到间接 `call` 或 `jmp *` 时，先把表的地址打出来，再逐项反汇编表里的目标。链表就是顺着指针读下一个结点里的整数。

这些步骤是在读我们自己的程序。本练习不涉及构造输入去破坏栈、也不讨论绕过保护。
