#pragma once
#include "Buffer.h"


class SocketBuffer
{
    public:
    SocketBuffer();
    SocketBuffer(SocketBuffer&& status);
    ~SocketBuffer();

    void delete_has_send();
    int Push(int sock,int size);
    int Pop(int sock,int size);

    Buffer recv_buf;
    size_t buf_send_cnt;

};