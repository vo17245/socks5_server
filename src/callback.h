#include<event.h>
#include<string.h>  
#include <iostream>
#include<arpa/inet.h>
#include<unistd.h>  
#include "debug_tools.h"
#include <unordered_map>
#include "Buffer.h"
#include "net_tools.h"


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
bool CreateMethodSelection(const Buffer& greeting,Buffer& method_selection);
void greeting_cb(int fd, short events, void* _args);
void accept_cb(int fd, short events, void* _args);