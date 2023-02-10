#include "debug_tools.h"
#include <iostream>
char hex_to_char(char hex)
{
    char ret=0x10;
	hex = hex & 0x0f;
	if (!(hex ^ 0x00))
		ret= '0';
	else if (!(hex ^ 0x01))
		ret= '1';
	else if (!(hex ^ 0x02))
		ret= '2';
	else if (!(hex ^ 0x03))
		ret= '3';
	else if (!(hex ^ 0x04))
		ret= '4';
	else if (!(hex ^ 0x05))
		ret= '5';
	else if (!(hex ^ 0x06))
		ret= '6';
	else if (!(hex ^ 0x07))
		ret= '7';
	else if (!(hex ^ 0x08))
		ret= '8';
	else if (!(hex ^ 0x09))
		ret= '9';
	else if (!(hex ^ 0x0a))
		ret= 'a';
	else if (!(hex ^ 0x0b))
		ret= 'b';
	else if (!(hex ^ 0x0c))
		ret= 'c';
	else if (!(hex ^ 0x0d))
		ret= 'd';
	else if (!(hex ^ 0x0e))
		ret= 'e';
	else if(!(hex ^ 0x0f))
		ret= 'f';
    return ret;
}
std::string byte_to_string(uint8_t byte)
{
	std::string s = "0x";
	s.push_back(hex_to_char((byte >> 4)));
	s.push_back(hex_to_char((byte)));
	return s;
}
void print_call_error(const char* func,const char* file,int line)
{
    std::cout<<func<<" error in "<<file<<":"<<line<<std::endl;
}

void print_error(const char* error,const char* file,int line)
{
    std::cout<<"Error: "<<error<<" in "<<file<<":"<<line<<std::endl;
}

void print_greeting(const char* greeting)
{
	std::cout<<"VER: "<<byte_to_string(greeting[0])<<std::endl;
	std::cout<<"NMETHODS: "<<byte_to_string(greeting[1])<<std::endl;
	std::cout<<"METHODS: ";
	for(int i=0;i<greeting[1];i++)
	{
		std::cout<<byte_to_string(greeting[i+2])<<" ";
	}
	std::cout<<std::endl;
}


void print_method_selection(const char* method_selection)
{
	std::cout<<"VER: "<<byte_to_string(method_selection[0])<<std::endl;
	std::cout<<"METHOD: "<<byte_to_string(method_selection[1])<<std::endl;
}