# allocraft · 隐式空闲链表分配器（第 9 章）

英文提要：Implement `mm_init`, `mm_malloc`, `mm_free`, `mm_realloc`, and `mm_check` on a simulated heap (`mem_sbrk` only). The reference is an implicit free list with boundary tags, first fit, splitting, and immediate coalescing.

## 接口

```c
int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void *ptr);
void *mm_realloc(void *ptr, size_t size);
int mm_check(void);   /* 0 means the heap is consistent */
```

块大小按 8 字节对齐。`mm_check` 应确认：头尾标记一致、没有相邻的两个空闲块、堆以一个已分配的序言块和一个大小为 0 的结尾块收束。

## 轨迹

`traces/*.trace` 的每一行是：

```
a 64        # 分配 64 字节，按出现顺序编号 0,1,2...
f 1         # 释放 1 号
r 0 128     # 把 0 号调整到 128 字节，旧内容必须保留
```

驱动会在每一步写入并核对填充字节，并打印利用率（峰值请求字节 / 堆大小）。参考实现应不低于 0.50。

## 评分

| 项目 | 分值 |
| --- | --- |
| 三条轨迹正确（填充、realloc 保留、mm_check） | 70 |
| 每条利用率 ≥ 0.50 | 30 |

## 运行

```bash
make -C labs/allocraft test-solution
```

学生从 `exercises/mm.c` 写起。`make test` 链接你的文件。
