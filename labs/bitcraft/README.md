# bitcraft · 位级谜题（第 2 章）

英文提要：Original bit and binary32 puzzles with an operator checker. Fill in `exercises/bits.c`. Do not use the operators a puzzle's comment leaves off the `allow` list. `opcheck.py` reads those comments and rejects extra operators, control flow, calls, and oversized constants.

## 目标

用受限的整数运算实现 22 个位谜题和 8 个 binary32 位级操作，体会补码、移位和 IEEE-754 编码。

## 评分

| 项目 | 分值 |
| --- | --- |
| 功能测试全部通过 | 70 |
| `opcheck.py` 通过（运算符、控制流、常量上限） | 30 |

参考答案在 `solutions/bits.c`，用它对照思路，先自己写。

## 运行

```bash
make -C labs/bitcraft test          # 你的 exercises/bits.c
make -C labs/bitcraft test-solution # 参考答案
python3 labs/bitcraft/opcheck.py labs/bitcraft/exercises/bits.c
```

每个函数上方的注释是契约，例如：

```
/* puzzle: rotate_left
 * allow: << >> | & -
 * max_ops: 12
 * const_max: 32
 * control: no
 */
```

`control: no` 表示不能写 `if` / 循环 / `?:` / `&&` / `||`。`control: yes` 的浮点题可以写 `if`，仍然不能用浮点类型或库函数。

## 谜题一览

整数：`bitwise_or` `bitwise_xor` `bitwise_nand` `low_mask` `is_zero` `is_negative` `sign_mask` `abs_val` `isolate_lowbit` `clear_lowbit` `is_power2` `odd_parity` `rotate_left` `byte_reverse` `swap_adjacent_bytes` `mul_by_15` `logical_not` `is_equal` `tmax_value` `tmin_value` `popcount` `fits_short`

浮点位模式：`float_is_zero` `float_is_denorm` `float_is_inf` `float_is_nan` `float_abs_bits` `float_neg_bits` `float_twice_bits` `float_half_bits`（向偶数舍入）
