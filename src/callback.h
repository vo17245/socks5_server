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

struct SenderReceiverStatus
{
    int sender;
    bool sender_is_ready;
    int receiver;
    bool receiver_is_ready;
};
class ReplyCallBackArgs
{
    public:
    ReplyCallBackArgs(){}
    ~ReplyCallBackArgs(){}
    event_base* base;
    event* cur;
    Buffer reply;
    Buffer request;
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