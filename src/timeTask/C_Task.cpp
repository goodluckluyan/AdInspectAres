//@file:C_Task.cpp
//@brief: ...
//@author: wangzhongping@oristartech.com
//date: 2012-05-23

#include "C_Task.h"
#include "MainProcess.h"
#include <sys/time.h>
C_Task::C_Task()
{
	m_iCommandNumber = 0;
	m_iTaskState = TASK_IDLE_STATE;
	m_pPara = NULL;
	m_iStartTime = -1;
	m_ptrInvoker = NULL;
	m_emTaskType = NULL_TASK;
}

C_Task::C_Task(void * ptr)
{
	m_iCommandNumber = 0;
	m_iTaskState = TASK_IDLE_STATE;
	m_pPara = NULL;
	m_iStartTime = -1;
	m_ptrInvoker = ptr;
	m_emTaskType = NULL_TASK;
}


C_Task::~C_Task()
{
}

/*******************************************************************************
* 函数名称：	IsIdle
* 功能描述：	获取任务状态
* 输入参数：	
* 输出参数：	
* 返 回 值：	任务状态，
* 其它说明：	检测 m_iTaskState 如果为 TASK_IDLE_STATE； 把m_iTaskState
				设置为TASK_NO_STATE 返回值为TASK_IDLE_STATE 时；表示调用成功其他为失败。
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2014-09-01					创建
*******************************************************************************/
TASK_STATE C_Task::IsIdle()
{
	m_CS.EnterCS();
	TASK_STATE oldState = m_iTaskState;
	if(m_iTaskState == TASK_IDLE_STATE)
	{
		m_iTaskState = TASK_LOCKED_STATE;
	}
	m_CS.LeaveCS();
	return oldState;
}

/*******************************************************************************
* 函数名称：	ISDelete
* 功能描述：	设置任务为删除状态
* 输入参数：	
* 输出参数：	
* 返 回 值：	任务状态
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2014-09-01					创建
*******************************************************************************/
TASK_STATE C_Task::ISDelete()
{
	
	TASK_STATE oldState = TASK_IDLE_STATE;
	m_CS.EnterCS();

	oldState = m_iTaskState;

	if(oldState != TASK_RUNNING_STATE)
	{
		ReInit();
	}
	else
	{
		m_iTaskState = TASK_DELETE_STATE;
	}

	m_CS.LeaveCS();

	return oldState;		
}


/*******************************************************************************
* 函数名称：	IsEnableRun
* 功能描述：	判断任务是否可以运行
* 输入参数：	
* 输出参数：	
* 返 回 值：	返回值0为符合条件成功返回 ；非0为不符合条件）
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2014-09-01					创建
*******************************************************************************/
int C_Task::IsEnableRun(int iCurTime)
{
	if(m_iCommandNumber == 0)
	{
		return 1;
	}

    //printf("id:%d starttime:%d curtime:%d\n",m_iCommandNumber,m_iStartTime,iCurTime);
	if(m_iTaskState == TASK_NO_STATE && m_emTaskType == TIME_TASK 
		&& m_iStartTime <= iCurTime)
	{
		m_iTaskState = TASK_RUNNING_STATE;
		return 0;
	}

	if(m_iTaskState == TASK_NO_STATE && m_emTaskType == ONCE_TASK
		&& m_iStartTime <= iCurTime)
	{
		m_iTaskState = TASK_RUNNING_STATE;
		return 0;
	}

	if(m_iTaskState == TASK_NO_STATE && m_emTaskType == ALWAYS_TASK)
	{
		m_iTaskState = TASK_RUNNING_STATE;
		return 0;
	}

	return 1;
}

// 获取命令字
TASK_STATE C_Task::GetTaskState()
{
	return m_iTaskState;
}

// m_iTaskState 在非 TASK_IDLE_STATE 可以调用此函数。
void C_Task::SetTaskState(TASK_STATE newState)
{
	m_iTaskState = newState;
}

// 设置命令字
void C_Task::SetCommandNumber(int nCmd)
{
	m_iCommandNumber = nCmd;
}


/*******************************************************************************
* 函数名称：	ReInit
* 功能描述：	根据任务类型对任务进行重初始化
* 输入参数：	
* 输出参数：	
* 返 回 值：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2014-09-01					创建
*******************************************************************************/
void C_Task::ReInit(int nDelaySec)
{
	if(TIME_TASK == m_emTaskType && nDelaySec != 0)
	{
        struct timeval tv;
        gettimeofday(&tv, NULL);
        m_iStartTime  = tv.tv_sec+ nDelaySec;
		m_iTaskState = TASK_NO_STATE;
	}
	else if( ONCE_TASK == m_emTaskType && 0 == nDelaySec )
	{
		m_iTaskState = TASK_LOCKED_STATE;
		m_iCommandNumber = 0;
		m_iStartTime = -1;
		m_iTaskState = TASK_IDLE_STATE;
		m_emTaskType = NULL_TASK;
	}
	else if( ALWAYS_TASK == m_emTaskType)
	{
		m_iTaskState = TASK_NO_STATE;
	}
	
}

 
/*******************************************************************************
* 函数名称：	Exec
* 功能描述：	任务执行，由不同的模块完成
* 输入参数：	
* 输出参数：	
* 返 回 值：	0为成功，-1为失败，2为没有相应的命令处理程序
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2014-09-01					创建
*******************************************************************************/
int  C_Task::Exec()
{
	int nRet = 0;
	if(GetTaskState() == TASK_DELETE_STATE)
	{

        ReInit();
		return 0;
	}

	if(m_ptrInvoker == NULL)
	{
		return -1;
	}

	SetTaskState(TASK_RUNNING_STATE);
    nRet = ((CMainProcess*)m_ptrInvoker)->Exec(m_iCommandNumber,m_pPara);
	ExeFinal();
	return nRet;
}

/*******************************************************************************
* 函数名称：	ExeFinal
* 功能描述：	执行结束后的处理，为下次任务做准备
* 输入参数：	
* 输出参数：	
* 返 回 值：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2014-09-01					创建
*******************************************************************************/
void C_Task::ExeFinal()
{

	switch(m_emTaskType)
	{
	case TIME_TASK:
		{
        int nDelay = ((CMainProcess*)m_ptrInvoker)->GetCheckDelay(m_iCommandNumber);
        ReInit(nDelay);
        SetTaskState(TASK_NO_STATE);
		}
		break;
	case ONCE_TASK:
	case ALWAYS_TASK:
		ReInit();
		break;
	}

	if(GetTaskState() == TASK_DELETE_STATE)
	{
		ReInit();
	}
	
	
}
