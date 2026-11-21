# 第 5 章 · 优化程序性能

英文提要：Measure a loop with a monotonic clock, then rewrite it without changing the integer result. Hoist invariant work, unroll, keep two accumulators, and add independent products in pairs.

## 概念

- 用 `clock_gettime(CLOCK_MONOTONIC)` 量一段循环的墙钟时间，再除以迭代次数，得到每次迭代的纳秒数。这是演示，不拿来判对错。
- 代码移动：循环里每次都算、结果却不变的表达式，应当提到循环外。典型例子是把 `strlen` 放进循环条件，长度不变，比较次数却变成平方级。
- 循环展开：一次迭代处理 2 个或 4 个元素，循环控制变少。长度不是展开因子的倍数时，要用收尾循环把剩下的元素加完。
- 多个累加器：一条 `sum += a[i]` 的链必须等前一次加法结束。分成两个累加器后，整数和不变，依赖变短。
- 重新结合：`(a[0]*b[0] + a[1]*b[1]) + (a[2]*b[2] + a[3]*b[3])` 与从左到右的点积，对整数是同一个和。
- `(a[0]+a[1])*(b[0]+b[1])` 是另一个式子，不能拿来替换点积。

## 构建与运行

```bash
make -C chapters/ch05_opt
./chapters/ch05_opt/demo_cpe
./chapters/ch05_opt/demo_motion
./chapters/ch05_opt/demo_accum
./chapters/ch05_opt/demo_unroll
make -C chapters/ch05_opt test          # exercises/ex_opt.c
make -C chapters/ch05_opt test-solution # solutions/ex_opt.c
```

计时会随机器负载浮动。四个演示在结果一致时返回 0；`demo_cpe` 只打印表格，不根据快慢失败。

## 练习

| 函数 | 难度 | 要求 |
| --- | --- | --- |
| `sum_naive` | ★☆☆ | 把 `a[0..n-1]` 加进 `long`，`n == 0` 时返回 0 |
| `sum_unroll2` | ★★☆ | 每次取两个元素，奇数长度收尾，和与朴素版相同 |
| `sum_dual_accum` | ★★☆ | 两个累加器，最后相加，和不变 |
| `dot_reassoc` | ★★☆ | 点积按独立的一对乘积相加；乘法用宽整数，避免 32 位截断 |

起点在 `exercises/ex_opt.c`，四个函数先返回 0。参考答案在 `solutions/ex_opt.c`。

## 自测问题

1. 字符串长度为 n 时，把 `strlen` 放进循环条件，字符会被重复查看大约多少次？提到循环外之后呢？
2. 两个累加器算出的整数和与一个累加器相同。变短的是数据依赖，还是元素个数？
3. 为什么 `(a[0]*b[0] + a[1]*b[1])` 仍然是点积的一部分，而 `(a[0]+a[1])*(b[0]+b[1])` 不是？
4. 展开因子为 4、`n = 6` 时，主循环做完后收尾还要补几个元素？
5. 演示程序打印的 ns/iter 为什么还不是“每元素周期数”？要换成周期还缺哪个量？
