#include "callback.h"




void request_cb(int fd, short events, void* _args)
{
    
}
class MethodSelectionCallBackArgs
{
    public:
    MethodSelectionCallBackArgs(){}
    MethodSelectionCallBackArgs(size_t buf_size):buf(buf_size){}
    ~MethodSelectionCallBackArgs(){}
    event_base* base;
    event* cur;
    Buffer buf;
    size_t send_cnt;
};
void method_selection_cb(int fd, short events, void* _args)
{
    MethodSelectionCallBackArgs* args=(MethodSelectionCallBackArgs*)_args;
    ssize_t send_ret=send(fd,args->buf.GetData()+args->send_cnt,args->buf.GetUsed()-args->send_cnt,0);
    //newwork error
    if(send_ret==-1)
    {
        std::cout<<"client socket: "<<fd<<std::endl;
        ERROR("method selection send error");
        shutdown(fd,SHUT_RDWR);
        close(fd);
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
    // release resource
    shutdown(fd,SHUT_RDWR);
    close(fd);
    event_free(args->cur);
    delete args;
}
struct GreetingCallBackArgs
{
    event_base* base;
    event* cur;
    Buffer buf;

};
bool CreateMethodSelection(const Buffer& greeting,Buffer& method_selection)
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
        ERROR("greeting recv error");
        shutdown(fd,SHUT_RDWR);
        close(fd);
        event_free(args->cur);
        delete args;
        return;
    }
    args->buf.Push(recv_ret,buf);
    // incompelete greeting
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
struct AcceptCallbackArgs
{
    event_base* base;
};
void accept_cb(int fd, short events, void* _args)
{
    AcceptCallbackArgs* args=(AcceptCallbackArgs*)_args;
    sockaddr_in client_addr;
    unsigned int client_addr_len=sizeof(sockaddr_in);
    int client_socket=accept(fd,(sockaddr*)&client_addr,&client_addr_len);
    if(client_socket==-1)
    {
        ERROR("accept error");
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
