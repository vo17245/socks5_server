#include "SocketBuffer.h"
#include<arpa/inet.h>
#include<unistd.h>
#include "debug_tools.h" 

SocketBuffer::SocketBuffer()
    :recv_buf(32),buf_send_cnt(0)
{
}
SocketBuffer::SocketBuffer(SocketBuffer&& status)
    :recv_buf(std::move(status.recv_buf))
{
    buf_send_cnt=status.buf_send_cnt;
}
SocketBuffer::~SocketBuffer(){}
Buffer recv_buf;
size_t buf_send_cnt;
void SocketBuffer::delete_has_send()
{
    Buffer buf(recv_buf.GetUsed()-buf_send_cnt);
    buf.Push(recv_buf.GetUsed()-buf_send_cnt,recv_buf.GetData()+buf_send_cnt);
    recv_buf=std::move(buf);
    buf_send_cnt=0;
}
ssize_t SocketBuffer::Push(int sock,int size)
{
    Buffer buf(size);
    ssize_t ret=recv(sock,(char*)buf.GetData(),size,0);
    if(ret<=0)
        return ret;
    recv_buf.Push(ret,buf.GetData());
    return ret;
}
ssize_t SocketBuffer::Pop(int sock,int size)
{
    ssize_t ret=send(sock,((char*)recv_buf.GetData())+buf_send_cnt,recv_buf.GetUsed()-buf_send_cnt,MSG_NOSIGNAL);
    if(ret<=0)
        return -1;
    buf_send_cnt+=ret;
    if(buf_send_cnt>1024)
    {
        delete_has_send();
    }
        
    return ret;
}