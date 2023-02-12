#pragma once
#include<event.h>
#include<string.h>  
#include <iostream>
#include<arpa/inet.h>
#include<unistd.h>  
#include "debug_tools.h"
#include <unordered_map>
#include "Buffer.h"
#include "net_tools.h"

class SocketBuffer
{
    public:
    SocketBuffer()
        :recv_buf(32),buf_send_cnt(0)
    {

    }
    SocketBuffer(SocketBuffer&& status)
        :recv_buf(std::move(status.recv_buf))
    {
        buf_send_cnt=status.buf_send_cnt;
    }
    ~SocketBuffer(){}
    Buffer recv_buf;
    size_t buf_send_cnt;
    void delete_has_send()
    {
        Buffer buf(recv_buf.GetUsed()-buf_send_cnt);
        buf.Push(recv_buf.GetUsed()-buf_send_cnt,recv_buf.GetData()+buf_send_cnt);
        recv_buf=std::move(buf);
        buf_send_cnt=0;
    }
    int Push(int sock,int size)
    {
        Buffer buf(size);
        ssize_t ret=recv(sock,(char*)buf.GetData(),size,0);
        if(ret<=0)
            return ret;
        recv_buf.Push(ret,buf.GetData());
        return ret;
    }
    int Pop(int sock,int size)
    {
        ssize_t ret=send(sock,((char*)recv_buf.GetData())+buf_send_cnt,recv_buf.GetUsed()-buf_send_cnt,0);
        if(ret<=0)
            return -1;
        buf_send_cnt+=ret;
        if(buf_send_cnt>1024)
            delete_has_send();
        return ret;
    }
};
class TalkCallbackArgs
{
    public:
    TalkCallbackArgs(){}
    ~TalkCallbackArgs(){}
    event_base* base;
    event* cur;
    event* event_with;
    // destin socket or client socket
    int socket_with;
};
class ReplyCallBackArgs
{
    public:
    ReplyCallBackArgs():send_cnt(0){}
    ~ReplyCallBackArgs(){}
    event_base* base;
    event* cur;
    Buffer reply;
    int destin_socket;
    size_t send_cnt;
    /*
    * status==0 --> address type is not support
    * status==1 --> connect succeed
    * status==2 --> connect timeout
    */
    int status;
};
class ConnectCallBackArgs
{
    public:
    event_base* base;
    event* cur;
    Buffer request;
    int client_socket;
    
};
class RequestCallBackArgs
{
    public:
    RequestCallBackArgs(){}
    RequestCallBackArgs(size_t buf_size):buf(buf_size){}
    ~RequestCallBackArgs(){}
    event_base* base;
    event* cur;
    Buffer buf;
};
class MethodSelectionCallBackArgs
{
    public:
    MethodSelectionCallBackArgs(){}
    MethodSelectionCallBackArgs(size_t buf_size):buf(buf_size){}
    ~MethodSelectionCallBackArgs(){}
    event_base* base;
    event* cur;
    Buffer buf;
    size_t send_cnt;
};
struct GreetingCallBackArgs
{
    event_base* base;
    event* cur;
    Buffer buf;

};
struct AcceptCallbackArgs
{
    event_base* base;
};

void request_cb(int fd, short events, void* _args);
void method_selection_cb(int fd, short events, void* _args);
void greeting_cb(int fd, short events, void* _args);
void accept_cb(int fd, short events, void* _args);