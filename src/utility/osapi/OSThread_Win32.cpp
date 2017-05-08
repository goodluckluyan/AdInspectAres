
#ifdef _WIN32

#include <windows.h>
#include <process.h> 

#include "OSThread.h"

struct OS_Thread_Priv
{
	HANDLE hThread;
};

struct OS_Thread_Param
{
	OS_Thread *thrd;
	OS_Thread_Proc thproc;
	void *lp;

	OS_Thread_Param()
		: thrd(NULL)
		, thproc(NULL)
		, lp(NULL)
	{}
};

OS_Thread::OS_Thread() 
: m_Priv(NULL)
{
}

OS_Thread::~OS_Thread()
{
	if(m_Priv)
	{
		OS_Thread_Priv* priv = (OS_Thread_Priv*) m_Priv;
		delete priv;
	}
}

static DWORD WINAPI OS_Thread_Proc_Win32_1(LPVOID param)
{
	OS_Thread* thrd = (OS_Thread*) param;
	thrd->Routine();

	return 0;
}

static void OS_Thread_Proc_Win32_2(void* param)
{
	/*OS_Thread* thrd = (OS_Thread*) param;
	thrd->Routine();*/

	OS_Thread_Param *thparam = (OS_Thread_Param *)param;
	OS_Thread  *thrd = thparam->thrd;
	OS_Thread_Proc thproc = thparam->thproc;
	void *lp = thparam->lp;
	delete thparam;

	if (NULL != thproc)
	{
		thproc(lp);
	}
	else
	{
		thrd->Routine();
	}
}

static unsigned int WINAPI OS_Thread_Proc_Win32_3(void* param)
{
	/*OS_Thread* thrd = (OS_Thread*) param;
	thrd->Routine();
	return 0;*/

	OS_Thread_Param *thparam = (OS_Thread_Param *)param;
	OS_Thread  *thrd = thparam->thrd;
	OS_Thread_Proc thproc = thparam->thproc;
	void *lp = thparam->lp;
	delete thparam;

	if (NULL != thproc)
	{
		thproc(lp);
	}
	else
	{
		thrd->Routine();
	}

	return 0;
}

int OS_Thread::Run(OS_Thread_Proc thproc/* = NULL*/, void * lp/* = NULL*/, bool bDetach/* = false*/)
{
	// 创建线程参数
	OS_Thread_Param *param = new OS_Thread_Param;
	param->thrd = this;
	param->thproc = thproc;
	param->lp = lp;

	if (bDetach)
	{
		// 创建自动回收线程
		HANDLE hThread = (HANDLE) _beginthread(OS_Thread_Proc_Win32_2, 0, param);
		if(NULL == hThread)
		{
			delete param;
			return -1;
		}
	}
	else
	{
		// 创建私有结构
		OS_Thread_Priv* priv = new OS_Thread_Priv;
		if(!priv) return -1;

		m_Priv = priv;

		// 创建线程
		unsigned int thrdaddr;
		priv->hThread = (HANDLE) _beginthreadex(NULL, 0, OS_Thread_Proc_Win32_3, param, 0, &thrdaddr);

		if(NULL == priv->hThread)
		{
			delete param;
			delete priv;
			m_Priv = NULL;
			return -1;
		}
	}

	return 0;
}

void OS_Thread::Join(OS_Thread* thrd)
{
	OS_Thread_Priv* priv = (OS_Thread_Priv*) thrd->m_Priv;
	if(priv)
	{
		WaitForSingleObject(priv->hThread, INFINITE);
 		CloseHandle(priv->hThread);

		// 删除资源
		delete priv;
		thrd->m_Priv = NULL;
	}
}

void OS_Thread::Msleep(int ms)
{
	::Sleep(ms);
}

void OS_Thread::Sleep(int s)
{
	::Sleep(s * 1000);
}

int OS_Thread::Routine()
{
	return 0;
}

#endif  //_WIN32


