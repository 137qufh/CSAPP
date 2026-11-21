# shellcraft · 小型 shell（第 8 章）

英文提要：A small shell with builtins, pipelines, redirection, and a job table. Foreground commands are waited on; background commands are reaped from a SIGCHLD handler. The parser is provided. You implement `eval_line`.

## 目标

在 `exercises/eval.c` 里完成命令执行：

- 内建：`quit` / `exit`、`cd`、`jobs`、`fg %id`、`bg %id`
- 外部程序：`fork` + `execvp`，父进程 `waitpid`
- 管道 `|`，重定向 `<` `>` `>>`，后台 `&`
- `SIGCHLD` 里用 `waitpid(..., WNOHANG)` 回收后台进程；`SIGINT` 转发给前台进程
- 处理函数里只做 async-signal-safe 的事（这里只改标志和 `waitpid`）

## 评分

| 项目 | 分值 |
| --- | --- |
| 解析回归（已提供，应保持通过） | 20 |
| echo / 重定向 / 管道 / cd / quit | 60 |
| 后台作业能被 `jobs` 看到并且 `fg` 能等它结束 | 20 |

参考实现：`solutions/eval.c`。

## 运行

```bash
make -C labs/shellcraft test-solution
./labs/shellcraft/shell-solution
```

非交互时不打印提示符，便于用脚本测试。交互时提示符是 `scsh> `。作业控制在没有控制终端时仍然可以 `command &`、`jobs`、`fg`，只是没有真正的终端进程组。

## 示例

```
scsh> /bin/echo hello | /bin/cat
hello
scsh> /bin/sleep 5 &
[1] 12345
scsh> jobs
[1] running /bin/sleep 5
scsh> fg %1
```
