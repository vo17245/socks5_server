#include "callback.h"
#include "SocketBuffer.h"
#include "Log.h"

extern event_base* base;
const size_t SOCK_BUF_SIZE=512;
char sock_buf[SOCK_BUF_SIZE];
void read_cb(int fd, short events, void* _args);
void write_cb(int fd, short events, void* _args)
{
    DEBUG("---------write_cb---------");
    SocketPair* pair=(SocketPair*)_args;
    if(events&EV_TIMEOUT)
    {
        goto ERROR;
    }
    {
        SocketData* data;
        SocketData* data_with;
        if(fd==pair->a.fd)
        {
            data=&(pair->a);
            data_with=&(pair->b);
        }
        else
        {
            data=&(pair->b);
            data_with=&(pair->a);
        }
        ssize_t ret=data_with->buf.Pop(fd,data_with->buf.getrest());
        if(ret<=0)
        {
            goto ERROR;
        }
        if(data_with->buf.getrest()==0)
        {
            event_free(data->ev_write);
            data->ev_write=nullptr;
            event* ev_read = event_new(NULL, -1, 0, NULL, NULL);
            data_with->ev_read=ev_read;
            event_assign(ev_read,base, data_with->fd, EV_READ |EV_PERSIST,read_cb ,pair);
            event_add(ev_read,nullptr);
        }
    }
    return;
    ERROR:
    DEBUG_CALL(delete pair);
    
}
void read_cb(int fd, short events, void* _args)
{
    DEBUG("---------read_cb---------");
    SocketPair* pair=(SocketPair*)_args;
    if(events&EV_TIMEOUT)
    {
        goto ERROR;
    }

    {
        SocketData* data;
        SocketData* data_with;
        if(fd==pair->a.fd)
        {
            data=&(pair->a);
            data_with=&(pair->b);
        }
        else
        {
            data=&(pair->b);
            data_with=&(pair->a);
        }
        ssize_t ret=data->buf.Push(fd,256);
        if(ret<=0)
            goto ERROR;
        event* ev = event_new(NULL, -1, 0, NULL, NULL);
        data_with->ev_write=ev;
        event_assign(ev,base, data_with->fd, EV_WRITE |EV_PERSIST,write_cb ,pair);
        event_add(ev,nullptr);
        event_free(data->ev_read);
        data->ev_read=nullptr;
    }
    
    return;
    ERROR:
    DEBUG_CALL(delete pair);
}
void reply_cb(int fd, short events, void* _args)
{
    DEBUG("---------reply_cb---------");
    ReplyCallBackArgs* args=(ReplyCallBackArgs*)_args;
    ssize_t send_ret;
    //event timeout
    if(events&EV_TIMEOUT)
    {
        goto ERROR;
    }
    send_ret=send(fd,args->reply.GetData()+args->send_cnt,args->reply.GetUsed()-args->send_cnt,MSG_NOSIGNAL);
    if(send_ret==-1)
    {
        goto ERROR;
    }
    args->send_cnt+=send_ret;
    //there is a data left
    if(args->send_cnt!=args->reply.GetUsed())
        return;
    if(args->status!=1)
        goto ERROR;

        
    // client destin 开始通信 
    {
        // set event
        // client -> a
        // destin -> b
        event* ev_client_read = event_new(NULL, -1, 0, NULL, NULL);
        event* ev_destin_read = event_new(NULL, -1, 0, NULL, NULL);
        SocketPair* sockpair=new SocketPair;
        sockpair->a.ev_read=ev_client_read;
        sockpair->b.ev_read=ev_destin_read;
        sockpair->a.ev_write=nullptr;
        sockpair->b.ev_write=nullptr;
        sockpair->a.fd=fd;
        sockpair->b.fd=args->destin_socket;
        event_assign(ev_client_read, args->base, fd, EV_READ| EV_PERSIST,read_cb ,sockpair);
        event_add(ev_client_read,nullptr);
        event_assign(ev_destin_read, args->base, args->destin_socket, EV_READ| EV_PERSIST, read_cb,sockpair);
        event_add(ev_destin_read,nullptr);
    }
    event_free(args->cur);
    delete args;
    return;


    ERROR:
    event_free(args->cur);
    shutdown(fd,SHUT_RDWR);
    close(fd);
    if(args->status==1)
    {
        shutdown(args->destin_socket,SHUT_RDWR);
        close(args->destin_socket);
    }
    delete args;
    return;

}



/*
* status==0 --> address type is not support
* status==1 --> connect succeed
* status==2 --> connect timeout
*/
static bool create_reply(const Buffer& request,Buffer& reply,int status)
{
    switch (status)
    {
    case 0:
        reply.Push(2,[](char* buf){
            buf[0]=0x05;
            buf[1]=0x08;
        });
        return true;
        break;
    case 1:
        reply.Push(4,[](char* buf){
            buf[0]=0x05;
            buf[1]=0x00;
            buf[2]=0x00;
            buf[3]=0x01;
        });
        reply.Push(6,&request.GetData()[4]);
        return true;
        break;
    case 2:
        reply.Push(2,[](char* buf){
            buf[0]=0x05;
            buf[1]=0x04;
        });
        break;
    default:
        return false;
        break;
    }
    return true;
}
void connect_cb(int fd, short events, void* _args)
{
    DEBUG("---------connect_cb---------");
    ConnectCallBackArgs* args=(ConnectCallBackArgs*)_args;
    if(events&EV_TIMEOUT)
    {
        // 链接destin超时，发送错误信息reply到client
        event* ev = event_new(NULL, -1, 0, NULL, NULL);
        //delete in reply_cb
        ReplyCallBackArgs* replyCallBackArgs=new ReplyCallBackArgs;
        replyCallBackArgs->base=args->base;
        replyCallBackArgs->cur=ev;
        replyCallBackArgs->status=2;
        create_reply(args->request,replyCallBackArgs->reply,2);
        event_assign(ev, args->base, fd, EV_WRITE | EV_PERSIST, reply_cb, replyCallBackArgs);
        event_add(ev,nullptr);

        event_free(args->cur);
        shutdown(fd,SHUT_RDWR);
        close(fd);
        
        delete args;

        return;
    }
    

    event* ev = event_new(NULL, -1, 0, NULL, NULL);
    //delete in reply_cb
    ReplyCallBackArgs* replyCallBackArgs=new ReplyCallBackArgs;
    replyCallBackArgs->base=args->base;
    replyCallBackArgs->cur=ev;
    replyCallBackArgs->status=1;
    replyCallBackArgs->destin_socket=fd;
    create_reply(args->request,replyCallBackArgs->reply,1);
    event_assign(ev, args->base, args->client_socket, EV_WRITE | EV_PERSIST, reply_cb, replyCallBackArgs);
    event_add(ev,nullptr);

    event_free(args->cur);
    delete args;
}

void request_cb(int fd, short events, void* _args)
{
    DEBUG("---------request_cb---------");
    RequestCallBackArgs* args=(RequestCallBackArgs*)_args;

    ssize_t recv_ret=recv(fd,sock_buf,SOCK_BUF_SIZE,0);
    //network error
    if(recv_ret<=0)
    {
        event_free(args->cur);
        shutdown(fd,SHUT_RDWR);
        close(fd);
        delete args;
        return;
    }
    args->buf.Push(recv_ret,sock_buf);
    // received request incomplete
    if(args->buf.GetUsed()<4)
        return;
    if(args->buf.GetData()[3]==0x01 && args->buf.GetUsed()<10)
        return;
    if(args->buf.GetData()[3]==0x04 && args->buf.GetUsed()<22)
        return;
    if(args->buf.GetData()[3]==0x03 )
    {
        if(args->buf.GetUsed()<5)
        {
            return;
        }
        else
        {
            if(args->buf.GetUsed()<5+args->buf.GetData()[4])
            {
                return;
            }
        }
    }

    
    if(args->buf.GetData()[3]!=0x01)
    {
        //请求的地址类型不支持，不进行链接,直接发送失败reply
        event* ev = event_new(NULL, -1, 0, NULL, NULL);
        //delete in method_selection_cb
        ReplyCallBackArgs* replyCallBackArgs=new ReplyCallBackArgs;
        replyCallBackArgs->base=args->base;
        replyCallBackArgs->cur=ev;
        replyCallBackArgs->status=0;
        create_reply(args->buf,replyCallBackArgs->reply,0);
        event_assign(ev, args->base, fd, EV_WRITE | EV_PERSIST, reply_cb, replyCallBackArgs);
        event_add(ev,nullptr);
    }
    else
    {
        //尝试链接destin,添加connect事件
        int destin_socket;
        bool ret;
        ret=create_tcp_socket(destin_socket);
        SOCK_ASSERT(ret!=false);
        ret=set_socket_nonblock(destin_socket);
        SOCK_ASSERT(ret!=false);
        connect4(destin_socket,*((uint32_t*)&(args->buf.GetData()[4])),htons(*((uint16_t*)&(args->buf.GetData()[8]))));

        event* ev = event_new(NULL, -1, 0, NULL, NULL);
        //delete in method_selection_cb
        ConnectCallBackArgs* connectCallBackArgs=new ConnectCallBackArgs;
        connectCallBackArgs->base=args->base;
        connectCallBackArgs->cur=ev;
        connectCallBackArgs->request=std::move(args->buf);
        connectCallBackArgs->client_socket=fd;
        event_assign(ev, args->base, destin_socket, EV_WRITE | EV_PERSIST, connect_cb, connectCallBackArgs);
        event_add(ev,nullptr);
    }
    

    //release resource
    event_free(args->cur);
    delete args;
    
}
void method_selection_cb(int fd, short events, void* _args)
{
    MethodSelectionCallBackArgs* args=(MethodSelectionCallBackArgs*)_args;
    ssize_t send_ret=send(fd,args->buf.GetData()+args->send_cnt,args->buf.GetUsed()-args->send_cnt,MSG_NOSIGNAL);
    //newwork error
    if(send_ret==-1)
    {
        event_free(args->cur);
        shutdown(fd,SHUT_RDWR);
        close(fd);
        delete args;
        return;
    }
    args->send_cnt+=send_ret;
    // there is a data left
    if(args->send_cnt!=args->buf.GetUsed())
        return;


    event* ev = event_new(NULL, -1, 0, NULL, NULL);
    // delete in request_cb
    RequestCallBackArgs* requestCallBackArgs=new RequestCallBackArgs(10);
    requestCallBackArgs->base=args->base;
    requestCallBackArgs->cur=ev;
    event_assign(ev, args->base, fd, EV_READ | EV_PERSIST, request_cb, requestCallBackArgs);
    event_add(ev,nullptr);
    // release resource
    event_free(args->cur);
    delete args;
}
static bool CreateMethodSelection(const Buffer& greeting,Buffer& method_selection)
{
    //VER
    method_selection.Push(1,[](char* buf){
        buf[0]=0x05;
    });
    //METHOD
    // 查找client 是否支持method=0x00 (不进行认证)
    const char* g=greeting.GetData();
    int i;
    for(i=0;i<g[1];i++)
    {
        if(g[i+2]==0x00)
            break;
    }
    if(i==g[1])
    {
        //不支持
        method_selection.Push(1,[](char* buf){
            buf[0]=0xff;
        });
    }
    else
    {
        method_selection.Push(1,[](char* buf){
            buf[0]=0x00;
        });
    }
    return true;
}
void greeting_cb(int fd, short events, void* _args)
{
    GreetingCallBackArgs* args=(GreetingCallBackArgs*)_args;
    
    ssize_t recv_ret=recv(fd,sock_buf,SOCK_BUF_SIZE,0);
    // network error
    if(recv_ret<=0)
    {
        event_free(args->cur);
        shutdown(fd,SHUT_RDWR);
        close(fd);
        delete args;
        return;
    }
    args->buf.Push(recv_ret,sock_buf);
    // received greeting incomplete 
    if(args->buf.GetUsed()<2)
        return;
    if((args->buf.GetUsed()<args->buf.GetData()[1]+2))
        return;
    // invalid greeting
    if((args->buf.GetUsed()<args->buf.GetData()[1]+2))
    {
        event_free(args->cur);
        delete args;
        shutdown(fd,SHUT_RDWR);
        close(fd);
        return;
    }


    

    event* ev = event_new(NULL, -1, 0, NULL, NULL);
    //delete in method_selection_cb
    MethodSelectionCallBackArgs* methodSelectionCallBackArgs=new MethodSelectionCallBackArgs(2);
    methodSelectionCallBackArgs->base=args->base;
    methodSelectionCallBackArgs->cur=ev;
    CreateMethodSelection(args->buf,methodSelectionCallBackArgs->buf);
    methodSelectionCallBackArgs->send_cnt=0;
    event_assign(ev, args->base, fd, EV_WRITE | EV_PERSIST, method_selection_cb, methodSelectionCallBackArgs);
    event_add(ev,nullptr);

    // release event resource
    event_free(args->cur);
    delete args;
}

void accept_cb(int fd, short events, void* _args)
{
    AcceptCallbackArgs* args=(AcceptCallbackArgs*)_args;
    sockaddr_in client_addr;
    unsigned int client_addr_len=sizeof(sockaddr_in);
    int client_socket=accept(fd,(sockaddr*)&client_addr,&client_addr_len);
    SOCK_ASSERT(client_socket!=-1);
    bool ret;
    ret=set_socket_nonblock(client_socket);
    SOCK_ASSERT(ret!=false);
    event* ev = event_new(NULL, -1, 0, NULL, NULL);
    // delete in greeting_cb
    GreetingCallBackArgs* greetingCallBackArgs =new GreetingCallBackArgs;
    greetingCallBackArgs->base=args->base;
    greetingCallBackArgs->cur=ev;
    event_assign(ev, args->base, client_socket, EV_READ | EV_PERSIST, greeting_cb, greetingCallBackArgs);
    event_add(ev,nullptr);
}
