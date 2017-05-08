
#ifndef _OSAPI_MUTEX_H
#define _OSAPI_MUTEX_H
#include<stdio.h>
#include<stdlib.h>
class OS_Mutex
{
public:
	OS_Mutex(bool bCreate = false);
	~OS_Mutex();

	int Init(); // 创建

	int Lock();
	int TryLock();
	void Unlock();
    void *getmutx();
private:
	void* m_Priv;

};

class OS_MutexGuard
{
public:
	OS_MutexGuard(OS_Mutex *pMutex)
	{
		m_pMutex = pMutex;
		if (NULL != m_pMutex)
		{
			m_pMutex->Lock();
		}
	}

	~OS_MutexGuard()
	{
		if (NULL != m_pMutex)
		{
			m_pMutex->Unlock();
		}
	}

private:
	OS_Mutex* m_pMutex;
};



#endif
