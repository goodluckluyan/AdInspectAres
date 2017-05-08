//@file:C_Task.h
//@brief: C_Task 定义了定时任务的数据结构和操作。
//@author: wangzhongping@oristartech.com
//date: 2012-05-23

#ifndef TMS20_TASK
#define TMS20_TASK

#include "../C_constDef.h"
#include "threadManage/C_CS.h"
#include <list>
#include <string>

class C_Task
{
public:
	C_Task();
    C_Task(void * ptr);
	~C_Task();

	TASK_STATE IsIdle();

	TASK_STATE ISDelete();

	//返回值0为符合条件成功返回 ；非0为不符合条件）。
	int IsEnableRun(int iCurTime);

	TASK_STATE GetTaskState();

	//m_iTaskState 在非 TASK_IDLE_STATE 可以调用此函数。
	void SetTaskState(TASK_STATE newState);
	
	void SetCommandNumber(int nCmd);
	
	void ReInit(int nDelaySec = 0);

	int Exec();

	void ExeFinal();

	int GetCommandNumber()
	{
		return  m_iCommandNumber;
	}
public:
	// 要执行的操作编号；
	int m_iCommandNumber;

	// 要执行的操作的参数；
	void * m_pPara;

	// 任务类型
	enum TASK_TYPE m_emTaskType;


	
	// 任务开始时间，适用于定时任务
	int m_iStartTime;

	//任务状态 
	TASK_STATE m_iTaskState; 

private:
	C_CS m_CS;

    void * m_ptrInvoker;


};
#endif //TMS20_TASK

