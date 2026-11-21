# 概念索引

英文提要：Where each idea lives. No quotations from any textbook.

| 概念 | 位置 |
| --- | --- |
| 编译流水线 gcc -E/-S/-c | `chapters/ch01_tour/scripts/pipeline.sh` |
| ELF / readelf / objdump | `chapters/ch01_tour/scripts/elf_peek.sh` |
| Amdahl | `chapters/ch01_tour/demo_amdahl.c`, `exercises/ex_amdahl.c` |
| 字节序与对象表示 | `chapters/ch02_data/demo_bytes.c` |
| 补码、截断、扩展 | `chapters/ch02_data/demo_int.c` |
| 移位乘除 | `chapters/ch02_data/demo_shift.c` |
| IEEE-754 binary32 | `chapters/ch02_data/demo_float.c`, `exercises/ex_floatbits.c` |
| 溢出检测 | `chapters/ch02_data/exercises/ex_overflow.c` |
| 受限位运算 | `labs/bitcraft` |
| x86-64 数据传送、算术、条件码 | `chapters/ch03_machine/demo_mov.c`, `demo_arith.c`, `demo_cc.c` |
| 循环与跳转表 | `chapters/ch03_machine/demo_loop.c`, `demo_switch.c` |
| 调用与递归 | `chapters/ch03_machine/demo_call.c`, `asm/add_loop.s` |
| 结构体对齐 | `chapters/ch03_machine/demo_array.c`, `exercises/ex_layout.c` |
| 栈保护（只讲防御） | `chapters/ch03_machine/demo_safe_stack.c` |
| gdb | `chapters/ch03_machine/gdb/walk.gdb` |
| 自己的逆向练习 | `labs/asmcraft` |
| MY86 编码、汇编器、顺序 CPU | `chapters/ch04_arch/my86.c` |
| 五级流水线、前推、停顿 | `chapters/ch04_arch/my86.c` (`my86_run_pipe`), `demo_pipe` |
| 与或非构成加法器 | `chapters/ch04_arch/demo_gates.c` |
| CPE 计时 | `chapters/ch05_opt/demo_cpe.c` |
| 代码外提、循环展开、多累加器、重关联 | `chapters/ch05_opt/demo_motion.c`, `demo_unroll.c`, `demo_accum.c` |
| 空间局部性、步长 | `chapters/ch06_memory/demo_locality.c`, `demo_stride.c` |
| memory mountain | `chapters/ch06_memory/demo_mountain.c`, `scripts/plot_mountain.py` |
| 组相联 LRU cache | `labs/cachecraft` |
| 多文件静态链接 | `chapters/ch07_linking/src/main.c` |
| 强符号 / 弱符号 | `chapters/ch07_linking/src/strong_value.c` |
| 静态库 / 共享库 / dlopen | `chapters/ch07_linking` `demo_ar`, `demo_so`, `demo_dlopen` |
| 编译期 / --wrap / LD_PRELOAD | `chapters/ch07_linking/src/demo_interpose_*.c`, `preload.c` |
| 重定位查看 | `chapters/ch07_linking/scripts/reloc_peek.sh` |
| fork / exec / wait | `chapters/ch08_ecf/demo_fork.c`, `demo_exec.c`, `demo_wait.c` |
| 僵尸 | `chapters/ch08_ecf/demo_zombie.c` |
| 信号、屏蔽、sigsuspend、sigsetjmp | `chapters/ch08_ecf/demo_signal.c`, `demo_mask.c`, `demo_suspend.c`, `demo_setjmp.c` |
| shell、作业、管道、重定向 | `labs/shellcraft` |
| 地址空间地图 | `chapters/ch09_vm/demo_maps.c` |
| mmap 与 COW | `chapters/ch09_vm/demo_mmap.c` |
| 两级页表 + TLB | `chapters/ch09_vm/demo_tlb.c`, `capstone/memsys.c` |
| 显式空闲链表 | `chapters/ch09_vm/demo_explicit.c` |
| 隐式空闲链表 malloc | `labs/allocraft/solutions/mm.c` |
| mark-sweep | `chapters/ch09_vm/demo_gc.c` |
| 短计数与稳健 I/O | `common/src/sc_bio.c`, `chapters/ch10_io/demo_short.c` |
| 文件元数据、目录、dup2 | `chapters/ch10_io/demo_stat.c`, `demo_dir.c`, `demo_dup2.c` |
| 字节序、getaddrinfo | `chapters/ch11_network/demo_byteorder.c`, `demo_getaddr.c` |
| TCP echo | `chapters/ch11_network/demo_echo.c` |
| HTTP/1.0 静态 + CGI 风格 | `chapters/ch11_network/http_server.c` |
| 进程 / select / epoll / 线程服务器 | `chapters/ch12_concurrent` |
| 生产者-消费者、读写锁 | `chapters/ch12_concurrent/demo_pc.c`, `demo_rw.c` |
| 死锁与固定加锁顺序 | `chapters/ch12_concurrent/demo_deadlock.c` |
| 并发代理 + LRU | `labs/proxycraft` |
| 整机 | `capstone/` |
