//@file:C_ThreadData.h
//@brief: 包含C_ThreadData：内部包含线程运行需要的数据，控制线程运行。
//@author: wangzhongping@oristartech.com
//date: 2012-05-23
#ifndef THREAD_DATA
#define THREAD_DATA

#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include "C_CS.h"
#include "../timeTask/C_Task.h"
#include "C_constDef.h"
#include "C_ErrorDef.h"

class C_ThreadData
{
public:
	C_ThreadData()
	{
		m_iState = INIT_STATE;
		m_iResumeCount = 0;
		m_iCommand = -1;
		m_pGsoap = NULL;
		m_pCommandPara = NULL;
		m_iErrorNumber = 0;
		m_strError = "";
		m_iRunType = RUN_UNKNOWN_TYPE;
		m_pReturn = NULL;
		m_iStartTime = -1;
		m_iWorkTime = 1800;
		m_pTask = NULL;
		m_bQuit = false;
		pthread_cond_init(&cond,NULL);
	}

	~C_ThreadData()
	{
		pthread_cond_destroy(&cond);
	}

	//挂起线程。
	//返回值：无。
	//参数：newState 线程挂起后设置的状态。iResumeCount 需要调用resume的次数
	void suspend(Thread_State newState, int iResumeCount = 1)
	{
		m_ThreadCS.EnterCS();
		if(m_iState == INIT_STATE || m_iState == RUN_STATE || m_iState == RUN_LOCKED_STATE)
		{
			m_iResumeCount = iResumeCount;
			m_iState = newState;
			pthread_cond_wait(&cond,&(m_ThreadCS.m_CS));

		}
		else
		{
//			C_LogManage::GetInstance()->WriteLog(ULOG_FATAL,LOG_MODEL_THREADMGR,0,ERROR_THREAD_STATE,"线程状态错误");
			//printf("suspend error pThreadData:%x\n",(unsigned int)this);
		}
		m_ThreadCS.LeaveCS();
			
	}

	//激活线程运行。
	void resume()
	{
		m_ThreadCS.EnterCS();
		//wzp on 2012-12-13 modify
	/*	if(--m_iResumeCount == 0 && m_iState == SUSPEND_LOCKED_STATE)
		{
			pthread_cond_broadcast(&cond);
		}
		m_iState = RUN_STATE;*/
	  if(--m_iResumeCount == 0 && m_iState == SUSPEND_LOCKED_STATE)
		{
			
			pthread_cond_signal(&cond) ;
			
			m_iState = RUN_STATE;	

			// when bug，use.
            struct timeval tv;
            gettimeofday(&tv, NULL);
            m_iStartTime = tv.tv_sec;
			
		}		
		//wzp end.
		m_ThreadCS.LeaveCS();
	}
	
	//判断当前线程是否空闲；
	Thread_State IsIdlState()
	{
		m_CS.EnterCS();
		Thread_State oldState = m_iState;
		if(m_iState == SUSPEND_IDL_STATE)
		{
			m_iState = SUSPEND_LOCKED_STATE;
		}
		m_CS.LeaveCS();
		return oldState;
	}
	
	//判断当前线程是否超时
	int IsTimeoutState(int iCurTime)
	{
		m_CS.EnterCS();
		int iResult = -1;
		if(m_iState == RUN_STATE && iCurTime - m_iStartTime > m_iWorkTime)
		{
			//m_iState = RUN_LOCKED_STATE; //wzp delete on 2012-12-13 
//			C_LogManage *pLogManage = C_LogManage::GetInstance();
			char tmp[1024];
			memset(tmp, 0, 1024);
			if(m_iRunType == 1)
			{
				sprintf(tmp, "Thread Timeout threadId:%u m_iRunType:%d TimeTaskId:%d",(unsigned int)m_hThread, m_iRunType,
					m_pTask->m_iCommandNumber);
			}
			else
			{
				sprintf(tmp, "Thread Timeout threadId:%u m_iRunType:%d ",(unsigned int)m_hThread, m_iRunType);
			}
//			iResult = pLogManage->CreateLogNumber(ULOG_FATAL,LOG_MODEL_THREADMGR,0,ERROR_THREAD_TIMEOUT);
//			pLogManage->WriteLog(iResult,tmp);
			//printf("%s\n",tmp);
			iResult = 0;
		}
		m_CS.LeaveCS();
		return iResult;
	}
	Thread_State SetState(Thread_State state)
	{
		Thread_State oldState;
		m_CS.EnterCS();
		oldState = m_iState;
		m_iState = state;
		m_CS.LeaveCS();
		return oldState;
	}

	Thread_State GetState()
	{
		return m_iState;
	}

	C_ThreadData* GetThreadData(pthread_t &threadId)
	{
		C_ThreadData*  pData = NULL;
		m_CS.EnterCS();
		if(pthread_equal(m_hThread, threadId) != 0)
		{
			pData =  this;
		}
		m_CS.LeaveCS();
		return 	pData;			
	}
	
public:

  C_Task * m_pTask;
  int m_iCommand;
  void *m_pGsoap;
  void *m_pCommandPara;
  void *m_pReturn;

  //当前操作的开始时间
  int m_iStartTime;

  // 当前操作的预计执行时间长度 单位：秒
  int m_iWorkTime;
  int m_iErrorNumber;
  std::string m_strError;

  //0:webservice; 1:timeTask
  int m_iRunType;
  C_CS m_CS;
  C_CS m_ThreadCS;
  volatile int m_iResumeCount;
  pthread_t  m_hThread;
  pthread_cond_t cond;
  bool m_bQuit;
 

private:

	volatile Thread_State m_iState; 
};
#endif //THREAD_DATA;

