# 第 7 章 · 链接

英文提要：Separate translation units become one process image. This chapter builds that picture with relocation records, strong and weak symbols, a static archive, a shared library, `dlopen`, and three ways to interpose on `malloc`.

## 概念

- 一个 `.c` 先变成可重定位目标文件。外部函数和全局变量的最终地址此时还不知道
- 重定位记录告诉链接器：哪一条指令、哪一个符号、按什么方式填地址
- 强符号是普通的全局定义。弱符号用 `__attribute__((weak))` 标出。两者同名时留下强符号；两个强符号同名则链接失败
- 静态库是若干 `.o` 打成的归档。链接器只抽出能消掉未解析符号的成员
- 共享库用 `-fPIC -shared` 生成。可执行文件用 `DT_RUNPATH=$ORIGIN` 在自己所在目录找到它
- `dlopen` / `dlsym` 把“找库、找符号”推迟到运行时
- 插入 `malloc` 有三个时机：本文件里的宏、链接期 `--wrap`、运行期 `LD_PRELOAD`。后两种拦不住 libc 内部已经绑定好的调用

## 构建与运行

```bash
make -C chapters/ch07_linking
./chapters/ch07_linking/demo_static_multi
./chapters/ch07_linking/demo_weak
./chapters/ch07_linking/demo_ar
./chapters/ch07_linking/demo_so
bash chapters/ch07_linking/scripts/run_so.sh
./chapters/ch07_linking/demo_dlopen
./chapters/ch07_linking/demo_interpose_compile
./chapters/ch07_linking/demo_interpose_wrap
bash chapters/ch07_linking/scripts/run_preload.sh
make -C chapters/ch07_linking reloc
make -C chapters/ch07_linking test-solution
```

`demo_static_multi` 由 `src/main.c`、`src/add.c`、`src/mul.c` 分别编译再链接，输出 `sum=7 product=12`。`demo_weak` 同时链入强符号 `int value = 42` 和弱符号 `int value`，打印 `value=42`。

静态库与共享库用的是同一份 `greet`：

```bash
gcc -c -o build/greet.o src/greet.c
ar rcs libscgreet.a build/greet.o
gcc -fPIC -c -o build/greet_pic.o src/greet.c
gcc -shared -Wl,-soname,libscgreet.so -o libscgreet.so build/greet_pic.o
```

`make test` 编译并运行练习里的桩代码。`make test-solution` 运行参考答案，并执行各个演示；任何一个演示非 0 退出，该目标就失败。

## 练习

| 文件 | 难度 | 要求 |
| --- | --- | --- |
| `exercises/ex_symbols.c` | ★★☆ | 给定两个文件里同一符号的种类（未定义、弱、强），判断链接是否成功，以及留下哪一边的定义 |

## 自测问题

1. `build/add.o` 里对 `printf` 没有引用，对 `add` 却有一个全局定义。链接 `demo_static_multi` 时，谁提供 `printf`？
2. 把 `src/weak_value.c` 里的弱属性去掉，再和 `src/strong_value.c` 链在一起，链接器会报什么性质的错误？
3. 为什么 `demo_ar` 的链接命令把 `libscgreet.a` 放在 `demo_ar.c` 后面，而不是前面？
4. 共享库不加 `-fPIC`，在 x86-64 上链接 `-shared` 时通常会遇到什么问题？
5. `demo_so` 的 `DT_RUNPATH` 写成 `$ORIGIN` 之后，把可执行文件和 `libscgreet.so` 一起拷到另一个目录，为什么往往仍然能启动？
6. `demo_dlopen` 没有在编译期写上 `greet` 的声明并链接它，程序怎么找到这个函数？
7. 编译期宏、`--wrap`、`LD_PRELOAD` 三种办法，哪一种改得到“只在当前 `.c` 里写出的 `malloc`”，哪一种能在不重新编译目标程序的情况下生效？
