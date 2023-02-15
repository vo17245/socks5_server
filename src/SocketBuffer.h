#pragma once
#include "Buffer.h"


class SocketBuffer
{
    public:
    SocketBuffer();
    SocketBuffer(SocketBuffer&& status);
    ~SocketBuffer();

    void delete_has_send();
    ssize_t Push(int sock,int size);
    ssize_t Pop(int sock,int size);
    inline const size_t getrest()const{return recv_buf.GetUsed()-buf_send_cnt;}
    Buffer recv_buf;
    size_t buf_send_cnt;
};