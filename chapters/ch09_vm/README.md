# 第 9 章 · 虚拟内存

英文提要：Address-space layout from `/proc/self/maps`, copy-on-write via `MAP_PRIVATE`, a two-level page table with a small TLB, an explicit free list, and a mark-sweep toy. The graded allocator is `labs/allocraft`.

## 概念

- 页、页偏移、虚拟页号
- 多级页表与 TLB（全相联、LRU）
- `mmap` 匿名私有映射与写时复制
- `/proc/self/maps` 里的代码段、堆、栈
- 隐式空闲链表（见 allocraft）与显式空闲链表
- 标记-清除垃圾回收：根、可达、清扫

## 构建与运行

```bash
make -C chapters/ch09_vm
make -C chapters/ch09_vm test-solution
```

`demo_mmap` 打印 `cow-ok`。`demo_tlb` 先命中再制造未命中。`demo_gc` 回收不可达对象。`demo_explicit` 用空闲块里的 next 指针做 LIFO 复用。

## 练习

| 文件 | 难度 | 要求 |
| --- | --- | --- |
| `exercises/ex_addr.c` | ★☆☆ | 由页移位数算出 VPN 和页内偏移 |

## 自测问题

1. 页大小 4 KiB 时，虚拟地址的低多少位是页内偏移？剩下的位为什么叫虚拟页号？
2. TLB 命中时还要查页表吗？未命中时查完页表要更新哪一项？
3. `MAP_PRIVATE` 的父子进程一开始可以共享物理页，谁先写谁就会拿到新的副本。父进程读到的应该是写之前的值还是写之后的值？
4. 显式空闲链表把 next 指针放在哪里？分配之后这块内存还能不能再被链表使用？
5. 标记-清除里，没有被标记但仍标着 live 的对象，清扫阶段应该做什么？
