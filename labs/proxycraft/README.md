# proxycraft · 并发 HTTP/1.0 代理（第 11–12 章）

英文提要：A threaded HTTP/1.0 forward proxy. It accepts absolute-form GET requests, dials a numeric IPv4 origin, and stores small 200 responses in an 8-slot LRU. A second fetch of the same URL must not touch the origin again.

## 评分

| 项目 | 分值 |
| --- | --- |
| 正确转发 `/a` 与 `/b` | 50 |
| 同一 URL 第二次命中缓存（origin 计数不增加） | 50 |

只监听 `127.0.0.1`。启动后向 stdout 打印 `port N` 并 flush，测试用这个端口连上来。

## 运行

```bash
make -C labs/proxycraft test-solution
./labs/proxycraft/proxy-solution
```

学生实现放在 `exercises/proxy.c`。请求行形如：

```
GET http://127.0.0.1:PORT/a HTTP/1.0
```

把路径和 Host 转成发给源站的普通 HTTP/1.0 请求。缓存键用完整 URL。对象大于 32 KiB 不缓存。每个连接一个 detached 线程，缓存用一把互斥锁保护。
