#include "callback.h"

std::unordered_map<int,SenderReceiverStatus> status_map;

void receiver_cb(int fd, short events, void* _args)
{

}
void sender_cb(int fd, short events, void* _args)
{

}
void connect_cb(int fd, short events, void* _args)
{
    ConnectCallBackArgs* args=(ConnectCallBackArgs*)_args;
    if(events&EV_TIMEOUT)
    {
        event* ev = event_new(NULL, -1, 0, NULL, NULL);
        //delete in method_selection_cb
        ReplyCallBackArgs* replyCallBackArgs=new ReplyCallBackArgs;
        replyCallBackArgs->base=args->base;
        replyCallBackArgs->cur=ev;
        replyCallBackArgs->status=2;
        replyCallBackArgs->request=std::move(args->request);
        event_assign(ev, args->base, fd, EV_WRITE | EV_PERSIST, reply_cb, replyCallBackArgs);
        event_add(ev, NULL);

        delete args;
        shutdown(fd,SHUT_RDWR);
        close(fd);
    }
    
    SenderReceiverStatus client_destin_status={args->client_socket,false,fd,false};
    status_map[args->client_socket]=client_destin_status;
    SenderReceiverStatus destin_client_status={fd,false,args->client_socket,false};
    status_map[fd]=client_destin_status;
    

    event* ev = event_new(NULL, -1, 0, NULL, NULL);
    //delete in method_selection_cb
    ReplyCallBackArgs* replyCallBackArgs=new ReplyCallBackArgs;
    replyCallBackArgs->base=args->base;
    replyCallBackArgs->cur=ev;
    replyCallBackArgs->status=1;
    replyCallBackArgs->request=std::move(args->request);
    event_assign(ev, args->base, fd, EV_WRITE | EV_PERSIST, reply_cb, replyCallBackArgs);
    event_add(ev, NULL);

    event* receiver_ev = event_new(NULL, -1, 0, NULL, NULL);
    //delete in method_selection_cb
    event_assign(receiver_ev, args->base, fd, EV_WRITE | EV_PERSIST, receiver_cb, receiver_ev);
    event_add(ev, NULL);

    event* sender_ev = event_new(NULL, -1, 0, NULL, NULL);
    //delete in method_selection_cb
    event_assign(sender_ev, args->base, fd, EV_WRITE | EV_PERSIST, sender_cb, sender_ev);
    event_add(ev, NULL);


    delete args;
}
void reply_cb(int fd, short events, void* _args)
{
    
}
void request_cb(int fd, short events, void* _args)
{
    std::cout<<"request_cb function call"<<std::endl;
    RequestCallBackArgs* args=(RequestCallBackArgs*)_args;
    char buf[256];
    ssize_t recv_ret=recv(fd,buf,256,0);
    //network error
    if(recv_ret==-1)
    {
        LOG_ERROR("request recv error");
        //close socket
        shutdown(fd,SHUT_RDWR);
        close(fd);
        //release resource
        event_free(args->cur);
        delete args;
        return;
    }
    std::cout<<"buffer size: "<<args->buf.GetSize()<<std::endl;
    std::cout<<"buffer used: "<<args->buf.GetUsed()<<std::endl;
    LOG_CALL(args->buf.Push(recv_ret,buf));
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

    //test
    std::cout<<"-----request-------------"<<std::endl;
    std::cout<<"client socket: "<<fd<<std::endl;
    print_request(args->buf.GetData());
    
    if(args->buf.GetData()[3]!=0x01)
    {
        event* ev = event_new(NULL, -1, 0, NULL, NULL);
        //delete in method_selection_cb
        ReplyCallBackArgs* replyCallBackArgs=new ReplyCallBackArgs;
        replyCallBackArgs->base=args->base;
        replyCallBackArgs->cur=ev;
        replyCallBackArgs->status=0;
        replyCallBackArgs->request=std::move(args->buf);
        event_assign(ev, args->base, fd, EV_WRITE | EV_PERSIST, reply_cb, replyCallBackArgs);
        event_add(ev, NULL);
    }
    else
    {
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
        event_add(ev, NULL);
    }
    

    //release resource
    LOG_CALL(event_free(args->cur));
    delete args;
    
}
void method_selection_cb(int fd, short events, void* _args)
{
    MethodSelectionCallBackArgs* args=(MethodSelectionCallBackArgs*)_args;
    ssize_t send_ret=send(fd,args->buf.GetData()+args->send_cnt,args->buf.GetUsed()-args->send_cnt,0);
    //newwork error
    if(send_ret==-1)
    {
        std::cout<<"client socket: "<<fd<<std::endl;
        LOG_ERROR("method selection send error");
        //close socket
        shutdown(fd,SHUT_RDWR);
        close(fd);
        //release resource
        event_free(args->cur);
        delete args;
        return;
    }
    args->send_cnt+=send_ret;
    // there is a data left
    if(args->send_cnt!=args->buf.GetUsed())
        return;
    // test
    std::cout<<"--------send method selection------------"<<std::endl;
    std::cout<<"client socket: "<<fd<<std::endl;
    print_method_selection(args->buf.GetData());

    event* ev = event_new(NULL, -1, 0, NULL, NULL);
    // delete in request_cb
    RequestCallBackArgs* requestCallBackArgs=new RequestCallBackArgs(10);
    requestCallBackArgs->base=args->base;
    requestCallBackArgs->cur=ev;
    event_assign(ev, args->base, fd, EV_READ | EV_PERSIST, request_cb, requestCallBackArgs);
    event_add(ev, NULL);
    // release resource
    LOG_CALL(event_free(args->cur));
    LOG_CALL(delete args);
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
    if(recv_ret==-1)
    {
        LOG_ERROR("greeting recv error");
        shutdown(fd,SHUT_RDWR);
        close(fd);
        event_free(args->cur);
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
    // test
    std::cout<<"--------greeting---------"<<std::endl;
    std::cout<<"client socket: "<<fd<<std::endl;
    print_greeting(args->buf.GetData());

    

    event* ev = event_new(NULL, -1, 0, NULL, NULL);
    //delete in method_selection_cb
    MethodSelectionCallBackArgs* methodSelectionCallBackArgs=new MethodSelectionCallBackArgs(2);
    methodSelectionCallBackArgs->base=args->base;
    methodSelectionCallBackArgs->cur=ev;
    CreateMethodSelection(args->buf,methodSelectionCallBackArgs->buf);
    methodSelectionCallBackArgs->send_cnt=0;
    event_assign(ev, args->base, fd, EV_WRITE | EV_PERSIST, method_selection_cb, methodSelectionCallBackArgs);
    event_add(ev, NULL);

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
    std::cout<<"----------a client link!--------------"<<std::endl;
    std::cout<<"client socket: "<<client_socket<<std::endl;
    set_socket_nonblock(client_socket);
    
    event* ev = event_new(NULL, -1, 0, NULL, NULL);
    // delete in greeting_cb
    GreetingCallBackArgs* greetingCallBackArgs =new GreetingCallBackArgs;
    greetingCallBackArgs->base=args->base;
    greetingCallBackArgs->cur=ev;
    event_assign(ev, args->base, client_socket, EV_READ | EV_PERSIST, greeting_cb, greetingCallBackArgs);
    event_add(ev, NULL);
}
