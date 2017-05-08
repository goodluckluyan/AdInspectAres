//@file:C_constDef.h
//@brief: 包含各种状态信息定义。
//@author:luyan@oristartech.com
//dade:2014-09-12

#ifndef IMONITOR_CONST_DEFINE
#define IMONITOR_CONST_DEFINE
#include "threadManage/C_CS.h"
#include <vector>
#include <string>
#include <map>


//const define

// 线程池中线程的个数。
const int THREAD_COUNT_IN_POOL = 10;

//任务ID
const int TASK_NUMBER_GET_MODULE_FEATRUE    = 0x0101;
const int TASK_NUMBER_MARK_TASKMGR          = 0x0102;
const int TASK_NUMBER_VIDEOCOMPARE_MGR      = 0x0103;
const int TASK_NUMBER_DOWNLOADVIDEO         = 0x0104;
const int TASK_NUMBER_TASKDISPATCH          = 0x0105;
const int TASK_NUMBER_ADDDOWNLOAD           = 0x0106;

//日志级别。
const int ULOG_DEBUG	= 0;
const int ULOG_INFO     = 1;
const int ULOG_ERROR	= 2;
const int ULOG_FATAL	= 3;
const int UDEFAULT_LOG_LEVEL = 0;

//日志所属模块
const int LOG_MODEL_THREADMGR = 1;
const int LOG_MODEL_TIMETASK = 2;
const int LOG_MODEL_DB = 3;
const int LOG_MODEL_WEBS = 4;
const int LOG_MODEL_JOBS = 5;
const int LOG_MODEL_OTHER = 6;
const int LOG_MODEL_LOGMGR = 7;

// 线程状态。
enum Thread_State
{
	INIT_STATE = 0, //初始
	RUN_STATE = 1,  //运行
	SUSPEND_IDL_STATE = 2, //空闲
	SUSPEND_FINISH_STATE = 3,//操作执行完成而暂停。
	RUN_LOCKED_STATE = 4,  // 运行时锁定。
	SUSPEND_LOCKED_STATE = 5 ,// 暂停时锁定。
	QUIT_STATE =6	// 线程结束。
};

// 线程运行类型，用于区分线程执行操作的类型。
enum Thread_Run_Type
{
	RUN_WEBSERVICE_TYPE = 0,	//线程正在执行Webservice调用。
	RUN_TIMETASK_TYPE = 1,		// 线程正在执行定时任务。
	RUN_UNKNOWN_TYPE = 2		// 线程未执行操作。
};

// 任务类型
enum TASK_TYPE
{
	NULL_TASK,// 空
	TIME_TASK,// 定时任务
	ONCE_TASK,// 一次任务
	ALWAYS_TASK// 固定任务
};

// 任务状态；
enum TASK_STATE
{
	//空闲
	TASK_IDLE_STATE = 0,

	// 任务未开始执行
	TASK_NO_STATE =1,

	// 任务正在执行。
	TASK_RUNNING_STATE = 2,

	// 任务执行完成。
	TASK_FINISH_STATE  = 3,

	// 任务需要被删除
	TASK_DELETE_STATE  = 4,

	//空闲时被锁定
	TASK_LOCKED_STATE  = 5
};














#endif 
