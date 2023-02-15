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
#include "SocketBuffer.h"
#include "Log.h"

struct SocketData
{
    int fd;
    event* ev_write;
    event* ev_read;
    SocketBuffer buf;
};
class SocketPair
{
    public:
    SocketPair(){}
    ~SocketPair()
    {
        if(a.ev_read!=nullptr)
            event_free(a.ev_read);
        if(b.ev_read!=nullptr)
            event_free(b.ev_read);
        if(a.ev_write!=nullptr)
            event_free(a.ev_write);
        if(b.ev_write!=nullptr)
            event_free(b.ev_write);
        shutdown(a.fd,SHUT_RDWR);
        shutdown(b.fd,SHUT_RDWR);
        close(a.fd);
        close(b.fd);

    }
    SocketData a;
    SocketData b;
    event* cur;
    
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