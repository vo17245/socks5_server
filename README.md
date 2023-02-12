# socks5_server

## 使用

配合firefox浏览器的socks5代理功能实现使用代理服务器浏览网页
注意: socks5_server 不对传输的数据进行加密，可以配合stunnel使用，实现数据加密

## 编译

socks5_server 使用libevent封装的epoll,需要添加libevent的include路径，并链接libevent.so

使用premake 作为编译工具

```
premake gmake
make config=release
```