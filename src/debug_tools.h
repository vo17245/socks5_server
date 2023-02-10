#include <string>

char hex_to_char(char hex);
std::string byte_to_string(uint8_t byte);

void print_call_error(const char* func,const char* file,int line);
void print_error(const char* error,const char* file,int line);
#ifdef DEBUG
    #define Call(x) \
    if((x)==false)\
    {\
        print_call_error(#x,__FILE__,__LINE__);\
    }
    #define ERROR(x) print_error(x,__FILE__,__LINE__)
#else
    #define Call(x)
    #define ERROR(x)
#endif


void print_greeting(const char* greeting);
void print_method_selection(const char* method_selection);