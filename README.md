# socks5_server

## 介绍

socks5_server 是一个轻量级单线程异步io socks5服务器

目前只支持linux

## 使用

配合firefox浏览器的socks5代理功能实现使用代理服务器浏览网页

注意: socks5_server 不对传输的数据进行加密，可以配合stunnel使用

## 编译

socks5_server 使用libevent封装的epoll,需要添加libevent的include路径，并链接libevent.so

使用premake 作为编译工具

```
premake gmake
make config=release
```

## Q&A

Q: 为什么我使用了stunnel还是不能访问google

A: 因为dns污染和dns抢答,使用https方式的dns并使用一个可靠的dns服务器可以解决问题

