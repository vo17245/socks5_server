#include<event.h>
#include<string.h>  
#include <iostream>
#include<arpa/inet.h>
#include<unistd.h>  
#include "debug_tools.h"
#include <unordered_map>
#include "Buffer.h"

bool str_to_ip4(const char* str,uint32_t& ip);
bool str_to_ip6(const char* str,char* ip);
bool create_server_socket(int& sock,uint32_t ip,uint16_t port);
bool set_socket_nonblock(int sock);