# 第 11 章 · 网络编程

英文提要：Byte order, numeric address lookup, a loopback echo, and a small original HTTP/1.0 server that serves static files and one CGI-style program. The client uses sockets directly.

## 概念

- 多字节整数在内存里的顺序和在网络上的顺序不一定相同。`htonl` / `ntohl` 负责 32 位值，端口号用 `htons` / `ntohs`。
- `getaddrinfo` 加上 `AI_NUMERICHOST` 只解析数字地址，不会去查 DNS。本章只用 `127.0.0.1`。
- 监听套接字负责 `bind` + `listen`。`accept` 返回另一只已连接套接字，数据走这只新描述符。端口写 0 时由内核分配，再用 `getsockname` 读出来。
- TCP 上一次 `read` 不一定是一条应用层消息。演示里的回显按换行把一行收齐。
- HTTP/1.0 在这里只接受 GET。响应带 `Content-Length` 和 `Connection: close`，处理完就关掉连接。
- `/index.html` 映射到 `www/index.html`。`/cgi/hello?name=lin` 执行 `www/cgi-bin/hello`，查询串放进 `QUERY_STRING`。程序如果只打印正文，服务器补上状态行。

## 构建与运行

在本章目录里运行，服务器按相对路径找 `www/`。直接执行演示时，程序会先切到自己所在的目录。

```bash
make -C chapters/ch11_network
./chapters/ch11_network/demo_byteorder
./chapters/ch11_network/demo_echo
./chapters/ch11_network/demo_http_demo
make -C chapters/ch11_network test-solution
```

`demo_http_server 0` 会打印实际端口并停在接受循环里（最多几秒，演示进程装了闹钟）。不带参数时，它自己发一次本地请求然后退出。

客户端示例：

```bash
./chapters/ch11_network/demo_http_client http://127.0.0.1:PORT/index.html
```

## 练习

| 文件 | 难度 | 要求 |
| --- | --- | --- |
| `exercises/ex_parse.c` | ★★☆ | `parse_request_line`：只接受 GET，路径必须非空且以 `/` 开头，查询串留在路径里，缺版本或其它方法返回 -1 |

## 自测问题

1. 在小端机器上把 `0x12345678` 交给 `htonl`，作为主机整数打印时为什么常常变成 `0x78563412`？线上的第一个字节仍然是什么？
2. 监听套接字和 `accept` 返回的套接字，各自由谁来 `read` / `write`？
3. 为什么本章的服务器要自己写 `Content-Length`，而不是依赖对端“读到对方关闭”？
4. `/cgi/hello?name=lin` 里，程序为什么看不到浏览器的原始套接字，只能看见 `QUERY_STRING`？
5. `AI_NUMERICHOST` 省掉了哪一次网络往返？把它拿掉、再去解析一个名字，失败模式会有什么不同？
6. 请求行 `GET /cgi/hello?name=lin HTTP/1.0` 里，路径要不要先删掉 `?` 后面的部分，才算解析成功？
