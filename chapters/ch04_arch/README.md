# 第 4 章 · 处理器体系结构（MY86）

英文提要：An original 32-bit teaching ISA called MY86, with an assembler, a sequential simulator, and a 5-stage pipeline that forwards and stalls. A separate file builds adders out of AND/OR/NOT. This is not the Y86 encoding from any textbook.

## 概念

- 指令编码：定长 32 位，opcode / rd / rs / rt / imm16
- 寄存器：`r0` 恒为 0，`lr`=`r6`，`sp`=`r7`（约定）
- 顺序解释器：取指、译码、执行、访存、写回
- 五级流水线：取指、译码、执行、访存、写回
- 数据冒险：ALU 结果前推；load-use 停顿一拍
- 控制冒险：分支在执行级解决，预测不跳转，跳转则冲刷
- 用与、或、非搭出异或、选择器和行波进位加法器

## 指令（立即数是有符号 16 位）

| 指令 | 含义 |
| --- | --- |
| `add/sub/and/or/xor/slt/sll/srl/sra rd, rs, rt` | 寄存器运算，移位量取 rt 低 5 位 |
| `addi/andi/ori/xori rd, rs, imm` | 立即数；andi/ori/xori 对 imm 零扩展 |
| `li rd, imm` / `lui rd, imm` | 装入低 16 位（符号扩展）/ 高 16 位 |
| `lw/lb/sw/sb rd, imm(rs)` | 访存。sw/sb 把 rd 存出去 |
| `beq/bne rs, rt, label` | PC 相对：`target = pc+4+offset` |
| `j label` | 伪指令，`beq r0, r0, label` |
| `jal label` / `jr rs` | 调用（返回地址进 lr）与寄存器跳转 |
| `svc 1` / `svc 2` | 把 r1 的低 8 位写入输出缓冲 / 十进制打印 r1 |
| `halt` | 停机 |

数据段从地址 `0x4000` 开始。`.data` `.word` `.byte` `.ascii` `.space` 可用。

## 构建与运行

```bash
make -C chapters/ch04_arch
./chapters/ch04_arch/demo_seq chapters/ch04_arch/programs/sum.my
./chapters/ch04_arch/demo_pipe chapters/ch04_arch/programs/sum.my
./chapters/ch04_arch/demo_gates
make -C chapters/ch04_arch test-solution
```

`demo_pipe` 每个周期打印 F/D/E/M/W。无冒险的直线代码周期数约为指令数 + 4。

## 练习

| 文件 | 难度 | 要求 |
| --- | --- | --- |
| `exercises/ex_hazard.c` | ★★☆ | 判断相邻两条指令是否构成 load-use |
| `exercises/ex_adder.c` | ★★☆ | 只用与或非实现异或、二选一和 4 位加法 |

## 自测问题

1. 为什么 r0 硬连线为 0 能少掉一类 `addi rd, rs, 0` 的特殊情况，同时又让 `beq r0, r0` 变成无条件跳转？
2. load 的结果最早在哪一级才能被下一条指令用到？中间差的那一拍流水线寄存器里应该放什么？
3. 分支若在执行级才比较完，预测不跳转时，跳转成功要丢掉几条已经取到的指令？
4. `jal` 把返回地址放进 lr。返回地址是 `pc` 还是 `pc+4`？为什么必须是后者，这条 ISA 又没有延迟槽？
5. 行波进位加法器的关键路径随位数怎样变长？选择器在 ALU 里扮演什么角色？
