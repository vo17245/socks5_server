#include "debug_tools.h"
#include <iostream>
#include<arpa/inet.h>

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
    std::cout<<"Error: "<<error<<" in "<<file<<":"<<line<<""<<std::endl;
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

std::string byte_to_string_dec(uint8_t val)
{
	std::string ret;
	uint8_t a=val%10;
	val=val/10;
	uint8_t b=val%10;
	val=val/10;
	if(val==0)
	{
		if(b==0)
		{
			ret.push_back(a+'0');
		}
		else
		{
			ret.push_back(b+'0');
			ret.push_back(a+'0');
		}

	}
	else
	{
		ret.push_back(val+'0');
		ret.push_back(b+'0');
		ret.push_back(a+'0');
	}
	return ret;
}

std::string ip_to_str4(uint32_t ip)
{
	std::string ret;
	ret+=byte_to_string_dec(*(((char*)&ip)));
	for(int i=1;i<4;i++)
	{
		ret.push_back('.');
		ret+=byte_to_string_dec(*(((char*)&ip)+i));
	}
	return ret;
}

void print_request(const char* request)
{
	std::cout<<"VER: "<<byte_to_string(request[0])<<std::endl;
	std::cout<<"REP: "<<byte_to_string(request[1])<<std::endl;
	std::cout<<"RSV: "<<byte_to_string(request[2])<<std::endl;
	std::cout<<"ATYP: "<<byte_to_string(request[3])<<std::endl;
	if(request[3]==0x01)
	{
		std::cout<<"DST.ADDR: "<<ip_to_str4(*((uint32_t*)&request[4]))<<std::endl;
		std::cout<<"DST.PORT: "<<ntohs(*((uint16_t*)&request[8]))<<std::endl;;
	}
	else if(request[3]==0x03)
	{
		std::cout<<"domain name length: "<<byte_to_string(request[4])<<std::endl;
		std::cout<<"domain: ";
		for(int i=0;i<(uint8_t)request[4];i++)
		{
			std::cout<<request[5+i];
		}
		std::cout<<std::endl;
	}
	else if(request[3]==0x04)
	{
		std::cout<<"DST.ADDR: ";
		for(int i=0;i<16;i++)
		{
			std::cout<<byte_to_string(request[4+i]);
		}
		std::cout<<std::endl;
		std::cout<<"DST.PORT: "<<ntohs(*((uint16_t*)&request[20]))<<std::endl;
	}
}

void print_debug_msg(const char* msg,const char* file,int line)
{
	std::cout<<"debug: "<<msg<<" in "<<file<<":"<<line<<std::endl;
}

void print_reply(const char* reply)
{
	std::cout<<"VER: "<<byte_to_string(reply[0])<<std::endl;
	std::cout<<"REP: "<<byte_to_string(reply[1])<<std::endl;
	std::cout<<"RSV: "<<byte_to_string(reply[2])<<std::endl;
	std::cout<<"ATYP: "<<byte_to_string(reply[3])<<std::endl;
	if(reply[3]==0x01)
	{
		//ipv4
		std::cout<<"BND.ADDR: "<<ip_to_str4(*((uint32_t*)&reply[4]))<<std::endl;
		std::cout<<"BND.PORT: "<<ntohs(*((uint16_t*)&reply[8]))<<std::endl;
	}
	else if(reply[3]==0x03)
	{
		std::cout<<"domain name length: "<<byte_to_string(reply[4])<<std::endl;
		std::cout<<"domain: ";
		for(int i=0;i<(uint8_t)reply[4];i++)
		{
			std::cout<<reply[5+i];
		}
		std::cout<<std::endl;
	}
	else if(reply[3]==0x04)
	{
		std::cout<<"DST.ADDR: ";
		for(int i=0;i<16;i++)
		{
			std::cout<<byte_to_string(reply[4+i]);
		}
		std::cout<<std::endl;
		std::cout<<"DST.PORT: "<<ntohs(*((uint16_t*)&reply[20]))<<std::endl;
	}
}