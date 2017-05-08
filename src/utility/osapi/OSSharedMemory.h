
#ifndef _OSAPI_SHAREDMEMORY_H
#define _OSAPI_SHAREDMEMORY_H


class OS_SharedMemory
{
public:
	OS_SharedMemory();
	~OS_SharedMemory();

	int Init(const char* name, int size);

	int Attach();
	void Detach();
	void* Data();

private:
	void* m_Priv;
};




#endif

