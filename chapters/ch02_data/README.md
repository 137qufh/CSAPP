# 第 2 章 · 信息的表示与处理

英文提要：Bits, bytes, two's complement, shifts, and IEEE-754 binary32. Demos print representations; exercises cover overflow and bit-level float operations. The bitcraft lab is the longer puzzle set for this chapter.

## 概念

- 进制转换与字节序（小端机器上的对象表示）
- 无符号与补码的同一位模式、不同解释
- 零扩展、符号扩展、截断
- 加减乘溢出；向零的移位除法
- 逻辑移位与算术移位
- IEEE-754：符号、阶码、尾数；零、次正规、无穷、NaN
- 舍入的直觉（bitcraft / 练习里用向偶数舍入做半精度）

## 构建与运行

```bash
make -C chapters/ch02_data
./chapters/ch02_data/demo_bytes
./chapters/ch02_data/demo_float
./chapters/ch02_data/demo_convert 0xA5
make -C chapters/ch02_data test-solution
```

本章练习在 `-fwrapv` 下编译，这样补码环绕是确定的教学行为。

## 练习

| 文件 | 难度 | 要求 |
| --- | --- | --- |
| `exercises/ex_overflow.c` | ★★☆ | `add_ok` / `sub_ok` / `mul_ok` / `saturating_add` |
| `exercises/ex_floatbits.c` | ★★★ | 只用整数运算识别特殊值、取绝对值、乘二、向零转换成 int |

更完整的位运算限制题在 `labs/bitcraft`。

## 自测问题

1. 位模式 `0xFFFFFFFF` 作为 `int` 和作为 `unsigned` 分别是多少？
2. 把 `short` 值 `-1` 赋给 `int` 时，高 16 位为什么全是 1？
3. `(int)0x80000000` 右移 4 位和 `(unsigned)0x80000000` 右移 4 位，结果差在哪里？
4. 为什么 `INT_MIN` 取相反数在 32 位补码里放不下？
5. 二进制 32 位浮点里，`1.0` 的阶码字段为什么是 127 而不是 0？
6. 两个正的 `int` 相加得到负数，说明发生了什么？用 `add_ok` 怎样判断？
