#include "SocketTools.h"
#include<arpa/inet.h>
#include<unistd.h> 

bool SocketTools::CreateTcpSocket4(int &sock)
{
    sock=socket(AF_INET,SOCK_STREAM,0);
    m_SocketMap[sock]=true;
    return sock!=-1;
}

bool SocketTools::CloseSocket(int sock)
{
    shutdown(sock,SHUT_RDWR);
    close(sock);
    m_SocketMap.erase(sock);
    return false;
}

void SocketTools::CloseAllSocket()
{
    int sock;
    for(const auto& pair:m_SocketMap)
    {
        sock=pair.first;
        shutdown(sock,SHUT_RDWR);
        close(sock);
    }
    m_SocketMap.clear();
}
