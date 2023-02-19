#pragma once
#include <string>
#include <iostream>
#include "Log.h"

char hex_to_char(char hex);
std::string byte_to_string(uint8_t byte);

void print_call_error(const char* func,const char* file,int line);
void print_error(const char* error,const char* file,int line);
void print_debug_msg(const char* msg,const char* file,int line);

#define ASSERT(x) if(!(x)) {ERROR("[ASSERT]{0} in {1}:{2}",#x,__FILE__,__LINE__);exit(-1);}
#define SOCK_ASSERT(x) if(!(x)) {ERROR("[SOCK_ASSERT]{0} errno={1} in {2}:{3}",#x,errno,__FILE__,__LINE__);exit(-1);}
#ifdef CONFIG_DEBUG
    
    #define LOG_ERROR(x) print_error(x,__FILE__,__LINE__)
    #define LOG_DEBUG(x) print_debug_msg(x,__FILE__,__LINE__)
    #define DEBUG_CALL(x) DEBUG("{0} start in {1}:{2}",#x,__FILE__,__LINE__);\
    x;\
    DEBUG("{0} end in {1}:{2}",#x,__FILE__,__LINE__);
#else
    #define LOG_ERROR(x) ;
    #define LOG_DEBUG(x) ;
    #define LOG_CALL(x) x;
    #define DEBUG_CALL(x) x;
#endif


void print_greeting(const char* greeting);
void print_method_selection(const char* method_selection);
void print_request(const char* request);
std::string ip_to_str4(uint32_t ip);
std::string byte_to_string_dec(uint8_t val);
void print_reply(const char* reply);