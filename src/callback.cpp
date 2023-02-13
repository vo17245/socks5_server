#include "callback.h"
#include "SocketBuffer.h"
#include "Log.h"
std::unordered_map<int,SocketBuffer> socket_buffer_map;

void talk_cb(int fd, short events, void* _args)
{
    TalkCallbackArgs* args=(TalkCallbackArgs*)_args;
    if(events&EV_TIMEOUT)
    {
        goto ERROR;
    }
    else if(events&EV_READ)
    {
        auto iter=socket_buffer_map.find(fd);
        if(iter==socket_buffer_map.end())
        {
            ERROR("socket {0} not found in socket_buffer_map");
            goto ERROR;
        }
        int recv_ret=iter->second.Push(fd,256);
        if(recv_ret<=0)
            goto ERROR;
    }
    else if(events&EV_WRITE)
    {
        auto iter=socket_buffer_map.find(args->socket_with);
        if(iter==socket_buffer_map.end())
        {
            ERROR("socket {0} not found in socket_buffer_map");
            goto ERROR;
        }
        if((iter->second.recv_buf.GetUsed()-iter->second.buf_send_cnt)!=0)
        {
            int send_ret=iter->second.Pop(fd,iter->second.recv_buf.GetUsed()-iter->second.buf_send_cnt);
            if(send_ret<=0)
                goto ERROR;
        }
    }

    return;
    ERROR:
    event_free(args->cur);
    event_free(args->event_with);
    shutdown(fd,SHUT_RDWR);
    close(fd);
    shutdown(args->socket_with,SHUT_RDWR);
    close(args->socket_with);
    socket_buffer_map.erase(fd);
    socket_buffer_map.erase(args->socket_with);
    delete args;
    return;
}
void reply_cb(int fd, short events, void* _args)
{
    ReplyCallBackArgs* args=(ReplyCallBackArgs*)_args;
    ssize_t send_ret;
    //event timeout
    if(events&EV_TIMEOUT)
    {
        goto ERROR;
    }
    send_ret=send(fd,args->reply.GetData()+args->send_cnt,args->reply.GetUsed()-args->send_cnt,0);
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

        
    DEBUG_CALL(print_reply(args->reply.GetData()));
    // client destin 开始通信 
    {
        // set event
        event* ev_client = event_new(NULL, -1, 0, NULL, NULL);
        event* ev_destin = event_new(NULL, -1, 0, NULL, NULL);

        TalkCallbackArgs* client_talk_args=new TalkCallbackArgs;
        client_talk_args->base=args->base;
        client_talk_args->cur=ev_client;
        client_talk_args->socket_with=args->destin_socket;
        client_talk_args->event_with=ev_destin;
       
        TalkCallbackArgs* destin_talk_args=new TalkCallbackArgs;
        destin_talk_args->base=args->base;
        destin_talk_args->cur=ev_destin;
        destin_talk_args->socket_with=fd;
        destin_talk_args->event_with=ev_client;
        event_assign(ev_client, args->base, fd, EV_WRITE |EV_READ| EV_PERSIST, talk_cb,client_talk_args);
        event_add(ev_client,nullptr);

        event_assign(ev_destin, args->base, args->destin_socket, EV_WRITE |EV_READ| EV_PERSIST, talk_cb,destin_talk_args);
        event_add(ev_destin,nullptr);

        // add socket status to status_map
        //client
        SocketBuffer client_status;
        std::pair<int,SocketBuffer> client_pair(fd,std::move(client_status));
        socket_buffer_map.insert(std::move(client_pair));
        //destin
        SocketBuffer destin_status;
        std::pair<int,SocketBuffer> destin_pair(args->destin_socket,std::move(destin_status));
        socket_buffer_map.insert(std::move(destin_pair));
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
    RequestCallBackArgs* args=(RequestCallBackArgs*)_args;
    char buf[256];
    ssize_t recv_ret=recv(fd,buf,256,0);
    //network error
    if(recv_ret<=0)
    {
        event_free(args->cur);
        shutdown(fd,SHUT_RDWR);
        close(fd);
        delete args;
        return;
    }
    args->buf.Push(recv_ret,buf);
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
        NET_CALL(create_tcp_socket(destin_socket));
        NET_CALL(set_socket_nonblock(destin_socket));
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
    ssize_t send_ret=send(fd,args->buf.GetData()+args->send_cnt,args->buf.GetUsed()-args->send_cnt,0);
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
    char buf[257];
    ssize_t recv_ret=recv(fd,buf,257,0);
    // network error
    if(recv_ret<=0)
    {
        LOG_ERROR("greeting recv error");
        event_free(args->cur);
        shutdown(fd,SHUT_RDWR);
        close(fd);
        delete args;
        return;
    }
    args->buf.Push(recv_ret,buf);
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
    if(client_socket==-1)
    {
        LOG_ERROR("accept error");
        return;
    }
    set_socket_nonblock(client_socket);
    
    event* ev = event_new(NULL, -1, 0, NULL, NULL);
    // delete in greeting_cb
    GreetingCallBackArgs* greetingCallBackArgs =new GreetingCallBackArgs;
    greetingCallBackArgs->base=args->base;
    greetingCallBackArgs->cur=ev;
    event_assign(ev, args->base, client_socket, EV_READ | EV_PERSIST, greeting_cb, greetingCallBackArgs);
    event_add(ev,nullptr);
}
