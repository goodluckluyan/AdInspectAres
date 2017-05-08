
#ifndef _OSAPI_THREAD_H
#define _OSAPI_THREAD_H

// 线程函数回调
typedef void (*OS_Thread_Proc)(void * lp);

class OS_Thread
{
public:
	OS_Thread();
	virtual ~OS_Thread();

	// 创建并启动
	virtual int Run(OS_Thread_Proc thproc = NULL, void * lp = NULL, bool bDetach = false);

	// 等待和收回资源
	static void Join(OS_Thread* thrd);

	// Sleep函数
	static void Msleep(int ms);
	static void Sleep(int s);

public:
	virtual int Routine();

private:
	void* m_Priv;
};


#endif

