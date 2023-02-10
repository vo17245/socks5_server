#include<event.h>
#include<string.h>  
#include <iostream>
#include<arpa/inet.h>
#include<unistd.h>  
#include "debug_tools.h"
#include <unordered_map>
#include "Buffer.h"
#include "net_tools.h"
#include "callback.h"


int main()
{
    const char str[]="0.0.0.0";
    uint16_t port=1080;
    uint32_t ip;
    Call(str_to_ip4(str,ip));
    int server_socket;
    Call(create_server_socket(server_socket,ip,port));
    Call(set_socket_nonblock(server_socket));
    std::cout<<"create server socket succeed"<<std::endl;
    std::cout<<"ip: "<<str<<std::endl;
    std::cout<<"port: "<<port<<std::endl;
    std::unordered_map<int,Buffer> method_selection;
    event_base* base = event_base_new();
    AcceptCallbackArgs args={base};
    event* ev_accept = event_new(base, server_socket, EV_READ | EV_PERSIST, accept_cb, &args);
    event_add(ev_accept, NULL);
    event_base_dispatch(base);
    
}

