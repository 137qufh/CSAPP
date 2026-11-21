# 第 3 章 · 程序的机器级表示

英文提要：Original x86-64 demos for data movement, arithmetic, condition codes, loops, jump tables, calls, and aggregate layout, plus one hand-written GAS function. Exercises ask for alignment and the natural layout of a small struct. The longer reading puzzle is `labs/asmcraft`.

## 概念

- 数据传送：`movq` 复制 64 位；`movl` 写低 32 位并把寄存器高位清零
- `leaq` 做地址算术：基址 + 变址 × 比例，这条指令本身不读内存
- 算术与移位：加、减、乘、除；有符号右移补符号位，无符号右移补 0
- 条件码 ZF、SF、CF、OF：比较或运算之后，用 `setcc` / `jcc` 消费它们
- 循环就是比较加上向前或向后的条件跳转
- 连续 `case` 的 `switch` 在 `-O1` / `-Og` 下常变成跳转表
- System V AMD64 调用：六个整数寄存器传参，返回值在 `%rax`，再往后的参数在栈上
- 递归调用每一层都留下返回地址；开了帧指针时 `%rbp` 指向当前帧
- 数组按元素大小线性寻址；结构体为了对齐插入填充；联合体成员重叠
- 栈保护的想法：返回前核对金丝雀。ASLR 让每次运行的栈地址不同

## 构建与运行

```bash
make -C chapters/ch03_machine
make -C chapters/ch03_machine asm
make -C chapters/ch03_machine test-solution
./chapters/ch03_machine/demo_mov
./chapters/ch03_machine/demo_switch
./chapters/ch03_machine/demo_asm
gdb -q -x chapters/ch03_machine/gdb/walk.gdb --args ./chapters/ch03_machine/demo_call
```

`asm` 把每个 `demo_*.c` 编成带注释的汇编，放在 `build/`。编译选项是 `gcc -Og -S -fno-asynchronous-unwind-tables -fno-omit-frame-pointer`，另外加了 `-fverbose-asm`，便于对照操作数。手写函数在 `asm/add_loop.s`，由 `demo_asm` 按 System V 约定调用。

`make test` 编译练习里的桩代码，桩还没填时测试会失败。`make test-solution` 编译 `solutions/ex_layout.c`。

## 练习

| 文件 | 难度 | 要求 |
| --- | --- | --- |
| `exercises/ex_layout.c` | ★★☆ | `align_up`；按自然对齐给出 `{ char a; int b; short c; }` 的 `off_a`、`off_b`、`off_c`、`span` |

头文件里写了为什么要有填充。参考答案在 `solutions/ex_layout.c`。反汇编阅读练习在 `labs/asmcraft`。

## 自测问题

1. `movl %eax, %ebx` 为什么会把 `%rbx` 的高 32 位清成 0？`movb` 会不会同样清掉目标寄存器的其余位？
2. `leaq (%rdi,%rsi,4), %rax` 算出的是哪个地址？这条指令读不读内存？
3. 有符号比较里，“小于”为什么要同时看 SF 和 OF，而不能只看结果的符号？
4. 无符号“低于”看的是 CF。找一对数，使有符号比较和无符号比较得出相反的结论。
5. 在 `-Og` 生成的 `for` 循环里，条件跳转通常放在循环体的哪一头？
6. `switch` 的 `case` 连成一片时，编译器为什么更愿意做跳转表？稀疏的 `case` 呢？
7. System V AMD64 的前六个整数参数用哪些寄存器？第七个放在哪里？返回值用哪个？
8. `fact` 每递归一层，栈上至少多出什么？`%rbp` 在开了帧指针时指向哪里？
9. `struct { char a; int b; short c; }` 里，`b` 前面的 3 个字节是干什么的？结构体末尾为什么还要再补齐？
10. 金丝雀放在局部变量和保存的返回地址之间。函数返回前比较的是哪两个值？不相等说明什么？
11. 同一个程序连跑两次，局部变量地址可能不同。这是哪一种装载机制造成的？
