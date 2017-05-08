
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "osapi.h"
#ifdef _WIN32
#include <windows.h>
#endif


const std::string&  ExePath()
{
	static std::string strPath;
	if(strPath.length() == 0)
	{
		char buf[512];
		int n = 0;

#ifdef _WIN32
		GetModuleFileNameA(NULL, buf, 512);
#else
		n = readlink("/proc/self/exe", buf, 512);
		if(n<=0) buf[0] = 0;
		else buf[n] = 0;
#endif

		std::string fullpath = buf;
		for(int i=0; i<(int)fullpath.length(); i++)
		{
			if(fullpath[i] == '\\') fullpath[i] = '/';
		}

		int pos = (int)fullpath.rfind('/');
		strPath = fullpath.substr(0, pos + 1);	
	}

	return strPath;
}

int OS_Log::Write(int level, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	return 0;
}

