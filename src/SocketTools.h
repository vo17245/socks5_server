#pragma once 
#include <unordered_map>
class SocketTools
{
    private:
    //just save socket
    std::unordered_map<int,bool> m_SocketMap;
    public:
    SocketTools();
    ~SocketTools();
    bool CreateTcpSocket4(int& sock);
    bool CloseSocket(int sock);
    void CloseAllSocket();
    
};