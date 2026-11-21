# cachecraft · 组相联缓存与分块转置

英文提要：An original set-associative cache with LRU, a small trace format, and a blocked matrix transpose. You implement the simulator. The driver only parses the trace and prints three counters.

## 目标

写一个组相联缓存模型，读入自己的 trace，统计命中、缺失和驱逐。再用 8×8 的瓦片做矩阵转置，让访问流比按列写目标矩阵更省缺失。

参数：

- `s`：组索引位数，组数是 `2^s`
- `E`：每组的行数
- `b`：块内偏移位数，块大小是 `2^b` 字节
- 标记取地址低 64 位里剩下的高位

判定：

- 命中：该组里有一条有效行，标记相同。把这一行变成最近使用。
- 冷缺失：没有命中，但组里还有无效行。装入新行，记一次缺失，不记驱逐。
- 驱逐：没有命中，组已满。丢掉最久未使用的行，记一次缺失和一次驱逐，新行成为最近使用。
- 加载和存储使用同一套规则（写分配）。`M` 先对整个地址范围做一遍加载，再做一遍存储。
- 一次操作若跨过块边界，每个被碰到的块记一次；范围按起始地址和长度覆盖到的块计算。练习用的 trace 都按块对齐，模型仍然要能处理跨块。
- `I` 行忽略，不访问缓存。

## 评分

| 项目 | 分值 |
| --- | --- |
| 模拟器的命中、缺失、驱逐与单元测试一致 | 60 |
| 转置元素正确，且 8×8 分块访问流的缺失少于朴素按列写入 | 40 |

分块那 40 分比较的是地址流：测试在参考模型上重放两种遍历（`s=5 E=1 b=5`，32×32 的 `int` 矩阵，同时计源矩阵的加载和目标矩阵的存储）。朴素版本按行读、按列写；分块版本以 8 为边长。分块缺失应当更少，并且低于测试里写死的上限。`solutions/transpose.c` 是这种瓦片的一种写法。起点 `exercises/transpose.c` 已经能算出正确元素，它按列写目标矩阵。

`exercises/csim_core.c` 先不计数，所以 `make test` 在你补上模拟器之前会失败。`make test-solution` 跑参考答案。

## Trace

每行一次记录，十六进制地址可以带 `0x`，也可以不带。长度是十进制字节数。空行和以 `#` 开头的行忽略。

```
L 1fff0000,4
S 1fff0004,4
M 1fff0000,8
I 400000,1
```

`L` 加载，`S` 存储，`M` 先加载再存储，`I` 忽略。

一个可以直接手算的小例子（不是评分文件）。`s=1 E=1 b=1`，也就是 2 组、每组 1 行、块 2 字节：

```
L 0,2
L 4,2
L 0,2
```

`0` 和 `4` 落在同一组、标记不同。三次都缺失，后两次还要驱逐：

```
hits: 0  misses: 3  evictions: 2
```

## 运行

```bash
make -C labs/cachecraft
./labs/cachecraft/csim -s 2 -E 1 -b 2 -t labs/cachecraft/traces/simple.trace
./labs/cachecraft/csim-ref -s 2 -E 1 -b 2 -t labs/cachecraft/traces/simple.trace
make -C labs/cachecraft test
make -C labs/cachecraft test-solution
```

成功时驱动只打印一行，两个数字之间是两个空格：

```
hits: N  misses: N  evictions: N
```

`csim` 链接 `exercises/csim_core.c`。`csim-ref` 链接参考实现。评分用的三条 trace 是 `traces/simple.trace`、`traces/stride.trace`、`traces/conflict.trace`。

## 自测问题

1. 冷缺失和驱逐缺失都会让 misses 加一。什么情况下 evictions 不加？
2. 同一组里有两行，先访问 A，再访问 B，再访问 A，然后来一个新标记 C。被丢掉的应该是 A 还是 B？
3. `M` 打在一个从未见过的块上，命中和缺失各是多少？
4. 块大小 4 字节时，从地址 `0x10` 开始的 8 字节访问要记几个块？
5. 转置时为什么按列写 `dst[j][i]` 会把同一缓存块拆到很多次循环以后才写完？8×8 的瓦片改变了哪一种局部性？
