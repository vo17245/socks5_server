#include "Buffer.h"
#include <string.h>

//test
#include "Log.h"

Buffer::Buffer()
	:m_Data(nullptr),m_Used(0),m_Size(0)
{
}
Buffer::Buffer(size_t size)
	:m_Used(0), m_Size(size)
{
	m_Data = new char[m_Size];
}
Buffer::Buffer(const Buffer& buffer)
	:m_Used(buffer.m_Used),m_Size(buffer.m_Size)
{
	m_Data = new char[m_Size];
	memcpy(m_Data, buffer.m_Data, m_Used);
}
Buffer::Buffer(Buffer&& buffer)noexcept
	:m_Used(buffer.m_Used), m_Size(buffer.m_Size)
{
	m_Data = buffer.m_Data;
	buffer.m_Data = nullptr;
}
void Buffer::operator=(const Buffer& buffer)
{
	m_Used = buffer.m_Used;
	m_Size = buffer.m_Size;
	m_Data = new char[m_Size];
	memcpy(m_Data, buffer.m_Data, m_Used);
}
void Buffer::operator=(Buffer&& buffer)
{
	if(m_Data!=nullptr)
		delete m_Data;
	m_Used = buffer.m_Used;
	m_Size = buffer.m_Size;
	m_Data = buffer.m_Data;
	buffer.m_Data = nullptr;
}
Buffer::~Buffer()
{
	DEBUG("Buffer::~Buffer()");
	if (m_Data != nullptr)
		delete m_Data;
}
void Buffer::Push(size_t size, const char* data)
{
	if ((m_Size - m_Used) < size)
		ResetSize(m_Used + size);
	memcpy(m_Data + m_Used, data, size);
	m_Used += size;
}
void Buffer::ResetSize(size_t size)
{
	char* p = new char[size];
	if (m_Data != nullptr)
	{
		size_t to_copy = size > m_Used ? m_Used : size;
		memcpy(p, m_Data, to_copy);
		delete m_Data;	
	}
	m_Data = p;
	m_Size = size;
}
void Buffer::Clear()
{
	m_Used = 0;
}
void Buffer::Push(size_t size, std::function<void(char*)> writer)
{
	if ((m_Size - m_Used) < size)
		ResetSize(m_Used + size);
	writer(m_Data + m_Used);
	m_Used += size;
}
