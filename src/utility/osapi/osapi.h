

/******************************************************************
*    osapi  - 为跨平台C++编程而设计
*       本套接口适用于win32/linux平台, 封装了thread, mutex, semaphore, shared memory , 
*        socket, bigint等接口和类型。
*    
*
*    作者  邵发  
*    始于  2011-06-24
*    email:  shaofa@vip.163.com
******************************************************************/

#ifndef _OSAPI_H
#define _OSAPI_H

#include <stdio.h>
#include <time.h>
#include <string>

#include "OSThread.h"
#include "OSMutex.h"
#include "OSSemaphore.h"
#include "OSSocket.h"
#include "OSIntType.h"
#include "OSSharedMemory.h"
#include "OSTime.h"


// 取程序所在目录
// 以斜线作为分隔符，末尾带有分隔符
const std::string& ExePath();


/* OS_Log:
	日志接口, osapi里默认实现为printf, 用户根据自己的需要继承自己的子类
*/

#define LOG_ERR			0
#define LOG_WARN		1
#define LOG_NORMAL	2
#define LOG_DBG			3

#define LOG_L0			10
#define LOG_L1			11
#define LOG_L2			12
#define LOG_L3			13
#define LOG_L4			14
#define LOG_L5			15
#define LOG_L6			16
#define LOG_L7			17
#define LOG_L8			18
#define LOG_L9			19

class OS_Log
{
public:
	virtual int Write(int level, const char* fmt, ...) ;
};

#endif
