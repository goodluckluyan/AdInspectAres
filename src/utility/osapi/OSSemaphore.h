
#ifndef  _OSAPI_SEMAPHORE_H
#define _OSAPI_SEMAPHORE_H

class OS_Semaphore
{
public:
	OS_Semaphore();
	~OS_Semaphore();

	int Init(int initial_value = 0); // 创建

	int Wait(bool bPost = true);
	int Wait(int ms, bool bPost = true);
	void Post();

private:
	void* m_Priv;
};


#endif

