#include<event.h>
#include<string.h>  
#include <iostream>
#include<arpa/inet.h>
#include<unistd.h>  
#include <unordered_map>
#include "Buffer.h"
#include "net_tools.h"
#include "callback.h"
#include "debug_tools.h"
#include "Log.h"

timeval timeout;
event_base* base;

void init()
{
    //init event timeout
    timeout.tv_sec=5;
    timeout.tv_usec=0;
    //init libevent
    base = event_base_new();
}
int main()
{
    init();
    const char str[]="0.0.0.0";
    uint16_t port=1080;
    uint32_t ip;
    bool ret;
    ret=str_to_ip4(str,ip);
    SOCK_ASSERT(ret!=false);
    int server_socket;
    ret=create_server_socket(server_socket,ip,port);
    SOCK_ASSERT(ret!=false);
    ret=set_socket_nonblock(server_socket);
    SOCK_ASSERT(ret!=false);
    INFO("create server socket succeed");
    INFO("ip: {0}",str);
    INFO("port: {0}",port);
    
    AcceptCallbackArgs args={base};
    event* ev_accept = event_new(base, server_socket, EV_READ | EV_PERSIST, accept_cb, &args);
    event_add(ev_accept, NULL);
    event_base_dispatch(base);
    
}

