#pragma once
#include <functional>

class Buffer
{
private:
	char* m_Data;
	size_t m_Size;
	size_t m_Used;
public:
	Buffer();
	Buffer(size_t size);
	Buffer(const Buffer& buffer);
	Buffer(Buffer&& buffer)noexcept;
	void operator=(const Buffer& buffer);
	void operator=(Buffer&& buffer);
	~Buffer();
	void Push(size_t size,const char* data);
	void Push(size_t size, std::function<void(char*)> writer);
	void ResetSize(size_t size);
	void Clear();
	inline const char* GetData()const { return m_Data; }
	inline const size_t GetSize()const { return m_Size; }
	inline const size_t GetUsed()const { return m_Used; }
};