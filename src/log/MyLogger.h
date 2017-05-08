#ifndef MYLOGGER_H_ 
#define MYLOGGER_H_ 

#include <string>
#include <stdio.h>
#include <stdarg.h>

#define LOG_BUFF_SIZE  2048
class MyLogger
{
public:
	MyLogger();
	~MyLogger(); 

public:
	typedef enum _LEVEL_M
	{
		TRACE = 0,
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL,
		OFF
	 }LEVEL_M;
	

#define msgFormatA(buf,size,msg) \
	do { \
    	va_list ap; \
    	va_start(ap, msg); \
    	vsnprintf(buf, size, msg, ap); \
    	va_end(ap); \
    	} while(0)

 
public:
	void Init(std::string prop_file,std::string module_name);
    void PrintLog(LEVEL_M level,const char *msg,...);

private:
	std::string m_modulename;
};

#endif
