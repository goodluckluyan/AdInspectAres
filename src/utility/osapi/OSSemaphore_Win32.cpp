
#include "OSSemaphore.h"

#ifdef _WIN32
#include <windows.h>

struct OS_Semaphore_Priv
{
	HANDLE hSem;
};

OS_Semaphore::OS_Semaphore()
:m_Priv (NULL)
{
}

OS_Semaphore::~OS_Semaphore()
{
	if(m_Priv)
	{
		OS_Semaphore_Priv* priv = (OS_Semaphore_Priv*) m_Priv;
		CloseHandle(priv->hSem);
		delete priv;
	}
}

int OS_Semaphore::Init(int initial_value)
{
	OS_Semaphore_Priv* priv = new OS_Semaphore_Priv;
	if(!priv) return -1;
	m_Priv = priv;

	priv->hSem = CreateSemaphore(NULL, initial_value, 5000, NULL);
	if(priv->hSem == NULL)
	{
		delete priv;
		m_Priv = NULL;
		return -1;
	}

	return 0;
}

int OS_Semaphore::Wait(bool bPost/* = true*/)
{
	OS_Semaphore_Priv* priv = (OS_Semaphore_Priv*) m_Priv;
	if(!priv) return -1;

	WaitForSingleObject(priv->hSem, INFINITE);

	if (bPost)
	{
		Post();
	}

	return 0;
}

int OS_Semaphore::Wait(int ms, bool bPost/* = true*/)
{
	OS_Semaphore_Priv* priv = (OS_Semaphore_Priv*) m_Priv;
	if(!priv) return -1;

	DWORD  ret = WaitForSingleObject(priv->hSem, ms);	
	if( ret == WAIT_OBJECT_0)
	{
		if (bPost)
		{
			Post();
		}

		return 0; // success
	}
	if( ret == WAIT_TIMEOUT)
	{
		return -1; // timeout
	}
	return -1;
}

void OS_Semaphore::Post()
{
	OS_Semaphore_Priv* priv = (OS_Semaphore_Priv*) m_Priv;
	if(!priv) return;

	ReleaseSemaphore(priv->hSem, 1, NULL);
}


#endif // _WIN32


