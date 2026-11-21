# 第 6 章 · 存储器层次

英文提要：Row-major and column-major walks, stride, a short memory mountain, and a direct-mapped cache you can read on one screen. The full set-associative simulator is a separate lab and is not linked from here.

## 概念

- 时间局部性：刚用过的数据，很快还会再用。空间局部性：用了一个地址，旁边的地址很可能马上用到。
- 行优先存放时，`a[r*cols + c]` 和它右边的元素挨在一起。按行加总是顺着存放顺序走；按列加总跳过一整行。
- 步长：每隔几个元素读一次。步长越大，一块缓存里真正用到的元素越少。
- 存储器山：工作集从小到大，步长从密到疏，纵轴是每次访问的纳秒数。演示只跑四档大小、三种步长，方便几秒内看完。
- 直接映射缓存：地址拆成标记、组索引、块内偏移。组里只有一条线。空位上的缺失是冷缺失；挤掉已有的线还要再记一次驱逐。

## 构建与运行

```bash
make -C chapters/ch06_memory
./chapters/ch06_memory/demo_locality
./chapters/ch06_memory/demo_stride
./chapters/ch06_memory/demo_mountain
./chapters/ch06_memory/demo_cache
python3 chapters/ch06_memory/scripts/plot_mountain.py chapters/ch06_memory/memory_mountain.csv
make -C chapters/ch06_memory test
make -C chapters/ch06_memory test-solution
```

`demo_mountain` 在当前目录写下 `memory_mountain.csv`，列是 `size_bytes,stride,ns_per_access`，并打印文件路径。工作集为 4 KB、32 KB、256 KB、2 MB，步长（以 4 字节元素计）为 1、8、32。`plot_mountain.py` 能导入 matplotlib 时写出 `memory_mountain.png`；否则打印 `matplotlib not installed, CSV only` 并返回 0。C 测试不依赖 matplotlib。

`demo_locality` 的两份校验和必须相同，并且总是返回 0。计时只供观察。

## 练习

| 函数 | 难度 | 要求 |
| --- | --- | --- |
| `row_sum` | ★☆☆ | 按行遍历，累加 `a[r*cols + c]` |
| `col_sum` | ★★☆ | 按列遍历同一批元素，总和与按行相同 |

起点在 `exercises/ex_locality.c`，两个函数先返回 0。参考答案在 `solutions/ex_locality.c`。`rows == 0` 或 `cols == 0` 时和为 0。

## 自测问题

1. 行优先矩阵里，`a[r][c]` 和 `a[r][c+1]` 相差几个元素？和 `a[r+1][c]` 相差几个元素？
2. 块大小 32 字节、元素 4 字节、步长 8 个元素时，一次装入的块里有几个元素会被这次扫描用到？
3. 反复扫一块 4 KB、步长为 1 的缓冲区，为什么按“每次访问”计往往比扫一遍 2 MB 更便宜？
4. 直接映射缓存里，冷缺失和冲突缺失都会让 misses 加一。哪一种还会让 evictions 加一？
5. 存储器山里步长从 1 改成 32 时，同一次工作集的每次访问时间通常怎么变？原因是空间局部性变了什么？
