
#include "OSTime.h"

#ifdef _WIN32
#include <windows.h>
#include <time.h>
void OS_Time::GetLocalTime(OS_TimeVal* tv)
{
	SYSTEMTIME systime;
	::GetLocalTime(&systime);

	tm _tm;
	_tm.tm_year = systime.wYear - 1900;
	_tm.tm_mon = systime.wMonth - 1;
	_tm.tm_mday = systime.wDay;
	_tm.tm_hour = systime.wHour;
	_tm.tm_min = systime.wMinute;
	_tm.tm_sec = systime.wSecond;

	tv->tv_sec = (int) ::mktime(&_tm);
	tv->tv_usec = systime.wMilliseconds * 1000;
}

#else
#include <sys/time.h> 
#include <stdio.h>

void OS_Time::GetLocalTime(OS_TimeVal* tv)
{
	timeval tv_now;
	gettimeofday(&tv_now, NULL);
	tv->tv_sec = tv_now.tv_sec;
	tv->tv_usec = tv_now.tv_usec;
}

#endif

