//@file:C_TaskList.cpp
//@brief: ...
//@author: wangzhongping@oristartech.com date: 2012-05-23
//@modify: luyan date: 2017-04-23

#include "threadManage/C_ThreadManage.h"
#include "C_TaskList.h"
#include "C_ErrorDef.h"
#include "utility/C_Time.h"



C_TaskList::C_TaskList()
{
	
}
C_TaskList::~C_TaskList()
{
	std::list<C_Task *>::iterator it = m_TackList.begin();
	for(;it != m_TackList.end();it++)
	{
		C_Task * ptrTask = * it;
		if(ptrTask)
		{
			delete ptrTask;
		}
	}
	m_TackList.clear();
}

/*******************************************************************************
* 函数名称：	InitTaskList
* 功能描述：	初始化任务队列
* 输入参数：	
* 输出参数：	
* 返 回 值：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2014-09-01					创建
*******************************************************************************/
int C_TaskList::InitTaskList(void * ptrInvoker)
{

    //计算当天的0点时间
    std::string str;
    C_Time t1,t2;
    t1.setCurTime();
    t1.getDateStr(m_strCurDate);
    str = m_strCurDate + " 00:00:00";
    t2.setTimeStr(str);
    m_iZeroTime = t2.getTimeInt();

    //获取当前时间。
    gettimeofday(&m_CurTV, NULL);


    int iTaskCount = 100;
	C_Task *pTask = NULL;
	for(int i=0; i<iTaskCount; ++i)
	{
		pTask = new C_Task(ptrInvoker);
		m_TackList.push_back(pTask);
	}
	return 0;
}


/*******************************************************************************
* 函数名称：	GetIdleTask
* 功能描述：	获取空闲任务
* 输入参数：	任务指针的指针
* 输出参数：	
* 返 回 值：	0：成功，非0失败
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2014-09-01					创建
*******************************************************************************/
int C_TaskList::GetIdleTask(C_Task **ppTask)
{
	C_GuardCS guard(&m_cs);
	std::list<C_Task*>::iterator it = m_TackList.begin();
	for(; it != m_TackList.end(); ++it)
	{
		if((*it)->IsIdle() == TASK_IDLE_STATE)
		{
			*ppTask = (C_Task*)(*it);
			return 0;
		}			
	}
//	C_LogManage::GetInstance()->WriteLog(ULOG_FATAL,LOG_MODEL_TIMETASK,0,ERROR_TASK_LIST_FULL,"任务对列已满，无空闲任务。");
//	return C_LogManage::GetInstance()->CreateLogNumber(3,18,0,ERROR_TASK_LIST_FULL);
    return 1;
	
}

/*******************************************************************************
* 函数名称：	DeleteTask
* 功能描述：	删除任务
* 输入参数：	命令号
* 输出参数：	
* 返 回 值：	0：成功，非0失败
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2014-09-01					创建
*******************************************************************************/
bool C_TaskList::DeleteTask(int nCommandNumber)
{
	bool bRet = false;
	C_GuardCS guard(&m_cs);
	std::list<C_Task*>::iterator it = m_TackList.begin();
	for(; it != m_TackList.end(); ++it)
	{
		if((*it)->GetCommandNumber() == nCommandNumber)
		{
			it = m_TackList.erase(it);
			bRet = true;
			break;
		}			
	}
	
	return bRet;

}

/*******************************************************************************
* 函数名称：	AddTask
* 功能描述：	增加任务到任务队列
* 输入参数：	iTaskNum：命令字
				pPara :参数指针
				iStarTime:定时任务开时时间，0为单次任务，-1为固定任务
* 输出参数：	
* 返 回 值：	0：成功，非0失败
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2014-09-01				创建
*******************************************************************************/
int C_TaskList::AddTask(int iTaskNum,  void *pPara, int iStartTime)
{
	C_Task *pTask;
	int iResult = GetIdleTask(&pTask);
	if(iResult != 0)
	{
		return iResult;
	}
	pTask->m_iCommandNumber = iTaskNum;
	
	pTask->m_pPara = pPara;
	if(0 == iStartTime )
	{
		pTask->m_emTaskType= ONCE_TASK;
		pTask->m_iStartTime = 0;
	}
	else if(-1 == iStartTime)
	{
		pTask->m_emTaskType= ALWAYS_TASK;
		pTask->m_iStartTime = 0;
	}
	else
	{	
		pTask->m_emTaskType = TIME_TASK;
		pTask->m_iStartTime = iStartTime;
	}

	pTask->SetTaskState(TASK_NO_STATE);
	return 0;
}

/*******************************************************************************
* 函数名称：	AddTask
* 功能描述：	增加任务到任务队列
* 输入参数：	iTaskNum：命令字
				pPara :参数指针
				iStarTime:定时任务开时时间，0为单次任务，-1为固定任务
* 输出参数：	
* 返 回 值：	0：成功，非0失败
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2014-09-01				创建
*******************************************************************************/
int C_TaskList::AddTask(int iTaskNum,  void *pPara, int iStartTime,int nType)
{
	C_Task *pTask;
	int iResult = GetIdleTask(&pTask);
	if(iResult != 0)
	{
		return iResult;
	}
	
	if(nType != ONCE_TASK && nType != ALWAYS_TASK && nType != TIME_TASK )
	{
		return -1;
	}

	pTask->m_iCommandNumber = iTaskNum;
	pTask->m_pPara = pPara;
	pTask->m_emTaskType = (TASK_TYPE )nType;
	pTask->m_iStartTime = iStartTime;
	pTask->SetTaskState(TASK_NO_STATE);
	return 0;
}


/*******************************************************************************
* 函数名称：	RunTasks
* 功能描述：	获取线程并执行定时任务及其它任务
* 输入参数：	iCurTime：当前时间（秒）
* 输出参数：	
* 返 回 值：	0：成功，非0失败
* 其它说明：	
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2014-09-01				创建
*******************************************************************************/
int C_TaskList::RunTasks()
{
    int iCurTime = GetCursecond();
	C_ThreadManage *pThreadManage = C_ThreadManage::GetInstance();
	C_ThreadData *pThreadData = NULL;
	int iResult = -1;

	C_GuardCS guard(&m_cs);
	std::list<C_Task*>::iterator it = m_TackList.begin();
	for(;it != m_TackList.end(); ++it)
	{

		if((*it)->IsEnableRun(iCurTime) == 0)
		{
			iResult = pThreadManage->GetIdlThread(&pThreadData);
			if(iResult != 0)
			{
//				LOGINFFMT(0,"Busy! No idle thread used !");
				return -1;
			}
			pThreadData->m_iRunType = 1;
			pThreadData->m_pTask = (*it);
			pThreadData->resume();
		}
	}
	return 0;
}

int C_TaskList::GetCursecond()
{

    gettimeofday(&m_CurTV, NULL);
    if(m_CurTV.tv_sec - m_iZeroTime >= 86400)
    {

        m_iZeroTime += 86400;
        C_Time t1;
        t1.setCurTime();
        t1.getDateStr(m_strCurDate);
    }
    return m_CurTV.tv_sec ;
}
