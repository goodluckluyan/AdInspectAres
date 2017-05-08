

#ifndef _WIN32
//#if 1
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "OSThread.h"


struct OS_Thread_Priv
{
	pthread_t hThread;
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

static void* OS_Thread_Proc_Linux(void* param)
{
	/*OS_Thread* thrd = (OS_Thread*) param;
	thrd->Routine();
	return NULL;*/

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

	return NULL;
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
		pthread_t hThread;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		int result = pthread_create(&hThread, &attr, OS_Thread_Proc_Linux, param);
		pthread_attr_destroy(&attr);

		if (0 != result)
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
		if (0 != pthread_create(&priv->hThread, NULL, OS_Thread_Proc_Linux, param))
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
		pthread_join(priv->hThread, NULL);

		// 删除资源
		delete priv;
		thrd->m_Priv = NULL;
	}
}

void OS_Thread::Msleep(int ms)
{
	//::usleep(ms * 1000);
	// 好像使用nanosleep更好

	timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

void OS_Thread::Sleep(int s)
{
	::sleep(s);
}

int OS_Thread::Routine()
{
	return 0;
}




#endif // ! _WIN32


