
#ifndef  _OSAPI_TIME_H
#define _OSAPI_TIME_H

/* 微秒级时间处理
    关于秒级的处理，请使用标准C的函数
*/

struct OS_TimeVal
{
	int tv_sec;
	int tv_usec;  

	OS_TimeVal To(const OS_TimeVal& other)
	{
		OS_TimeVal delta;
		delta.tv_usec = other.tv_usec - tv_usec;
		delta.tv_sec = other.tv_sec - tv_sec;
		if(delta.tv_usec < 0)
		{
			delta.tv_usec += 1000000; // 把微秒部分换算成正值
			delta.tv_sec -= 1;
		}
		return delta;
	}

	int MsTo(const OS_TimeVal& other)
	{
		return (other.tv_usec - tv_usec)/1000 + (other.tv_sec - tv_sec) *1000;
	}
};

class OS_Time
{
public:
	static void GetLocalTime(OS_TimeVal* tv);
};

#endif

