#include "net_tools.h"

bool create_server_socket(int& sock,uint32_t ip,uint16_t port)
{
    sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock==-1)
        return false;
    sockaddr_in addr;
    addr.sin_family = AF_INET;  
    addr.sin_addr.s_addr = ip;
    uint16_t high=port&0xff00;
    uint16_t low=port&0x00ff;
    addr.sin_port = htons(port);
    int ret;
    ret=::bind(sock, (sockaddr*)&addr, sizeof(sockaddr_in));
    if(ret<0)
        return false;
    ret=::listen(sock, 10);
    if(ret<0)
        return false;
    return true; 
}
bool set_socket_nonblock(int sock)
{ 
    int ret=evutil_make_socket_nonblocking(sock);
    return ret==0;
}
bool str_to_ip4(const char* str,uint32_t& ip)
{
    int ret=inet_pton(AF_INET,str,&ip);
    return ret==1;
}
bool str_to_ip6(const char* str,char* ip)
{
    int ret=inet_pton(AF_INET6,str,ip);
    return ret==1;
}