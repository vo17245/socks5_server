#include "net_tools.h"
#include "Log.h"

bool create_server_socket(int& sock,uint32_t ip,uint16_t port)
{
    sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock==-1)
        return false;
    sockaddr_in addr;
    addr.sin_family = AF_INET;  
    addr.sin_addr.s_addr = ip;
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

bool create_tcp_socket(int& sock)
{
    sock=socket(AF_INET,SOCK_STREAM,0);
    DEBUG("create a tcp socket fd={0}",sock);
    return sock!=-1;
}

bool connect4(int& client_socket,uint32_t ip,uint16_t port)
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;  
    addr.sin_addr.s_addr = ip;
    addr.sin_port = htons(port);
    int len=sizeof(sockaddr_in);
    int ret=connect(client_socket,(sockaddr*)&addr,len);
    return ret!=-1;
}
