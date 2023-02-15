#pragma once
#include <string>
#include <iostream>

char hex_to_char(char hex);
std::string byte_to_string(uint8_t byte);

void print_call_error(const char* func,const char* file,int line);
void print_error(const char* error,const char* file,int line);
void print_debug_msg(const char* msg,const char* file,int line);


#ifdef CONFIG_DEBUG
    #define ASSERT(x) if(!(x)) {ERROR("[ASSERT]{0} in {2}:{3}",#x,__FILE__,__LINE__);exit(-1);}
    #define LOG_ERROR(x) print_error(x,__FILE__,__LINE__)
    #define LOG_DEBUG(x) print_debug_msg(x,__FILE__,__LINE__)
    #define LOG_CALL(x) std::cout<<#x<<" in "<<__FILE__<<":"<<__LINE__<<std::endl;\
    std::cout<<"start"<<std::endl;\
    x;\
    std::cout<<"end"<<std::endl;
    #define DEBUG_CALL(x) x;
#else
    #define ASSERT(x) x;
    #define LOG_ERROR(x) ;
    #define LOG_DEBUG(x) ;
    #define LOG_CALL(x) x;
    #define DEBUG_CALL(x) ;
#endif


void print_greeting(const char* greeting);
void print_method_selection(const char* method_selection);
void print_request(const char* request);
std::string ip_to_str4(uint32_t ip);
std::string byte_to_string_dec(uint8_t val);
void print_reply(const char* reply);