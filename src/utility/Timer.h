/*******************************************************************************
* 版权所有 (C) 2012
* 
* 文件名称： Timer.h
* 文件标识： 
* 内容摘要： 定时器处理
* 其它说明： 
* 当前版本： V1.0
* 作    者： 白福铭
* 完成日期： 2012-10-26
*******************************************************************************/
#pragma once

#include <map>
#include <queue>
#include "osapi/osapi.h"

class CTimer
{

	// 事件映射表，事件－上次执行时间、时间间隔
	typedef std::map<int, std::pair<OS_TimeVal, int> > EventMap;

public:

	// 构造函数
	CTimer()
	{
		m_csEventTime.Init();
		m_csEventExec.Init();

		m_TimeStop.Init();
		m_ExecStop.Init();

		m_TimeThread.Run(TH_Time, this);
		m_ExecThread.Run(TH_Exec, this);
	}

	// 析构函数
	virtual ~CTimer()
	{
		Stop();
	}

	// 设置定时器
	void SetTimer(
		int nEventID,							// 事件ID
		int nTime,								// 时间间隔，单位：毫秒
		bool bExec = false						// 立即执行
		)
	{
		m_csEventTime.Lock();
		OS_TimeVal tmNow;
		OS_Time::GetLocalTime(&tmNow);
		m_mapEvent[nEventID] = std::make_pair(tmNow, nTime);
		m_csEventTime.Unlock();

		if (bExec)
		{
			// 立即加入执行队列
			m_csEventExec.Lock();
			m_queEvent.push(nEventID);
			m_csEventExec.Unlock();
		}
	}

	// 撤销定时器
	void KillTimer(
		int nEventID							// 事件ID
		)
	{
		m_csEventTime.Lock();
		EventMap::iterator it = m_mapEvent.find(nEventID);
		if (it != m_mapEvent.end())
		{
			m_mapEvent.erase(it);
		}
		m_csEventTime.Unlock();
	}

	// 停止工作
	void Stop()
	{
		m_TimeStop.Post();
		m_ExecStop.Post();

		OS_Thread::Join(&m_TimeThread);
		OS_Thread::Join(&m_ExecThread);
	}

protected:

	// 定时器处理
	virtual void OnTimer(
		int nEventID							// 事件ID
		)
	{}

	// 计时线程函数
	static void TH_Time(void * lp)
	{
		CTimer *pThis = (CTimer *)lp;
		if (NULL != pThis)
		{
			pThis->TimeProc();
		}
	}

	// 计时处理
	void TimeProc()
	{
		OS_TimeVal tmNow;
		while (-1 == m_TimeStop.Wait(1))
		{
			m_csEventTime.Lock();
			for (EventMap::iterator it = m_mapEvent.begin(); it != m_mapEvent.end(); ++it)
			{
				OS_Time::GetLocalTime(&tmNow);
				if (it->second.first.MsTo(tmNow) >= it->second.second)
				{
					// 达到定时间隔
					m_csEventExec.Lock();
					m_queEvent.push(it->first);
					m_csEventExec.Unlock();

					it->second.first = tmNow;
				}
				else if (it->second.first.MsTo(tmNow) < 0)
				{
					// 时间发生反向跳变
					it->second.first = tmNow;
				}
			}
			m_csEventTime.Unlock();
		}
	}

	// 执行线程函数
	static void TH_Exec(void * lp)
	{
		CTimer *pThis = (CTimer *)lp;
		if (NULL != pThis)
		{
			pThis->ExecProc();
		}
	}

	// 执行处理
	void ExecProc()
	{
		while (-1 == m_ExecStop.Wait(1))
		{
			m_csEventExec.Lock();
			if (m_queEvent.empty())
			{
				m_csEventExec.Unlock();
				continue;
			}
			unsigned int nEventID = m_queEvent.front();
			m_queEvent.pop();
			m_csEventExec.Unlock();

			// 定时器执行
			OnTimer(nEventID);
		}
	}

protected:

	// 计时事件映射表，事件－上次执行时间
	EventMap m_mapEvent;

	// 计时事件保护
	OS_Mutex m_csEventTime;

	// 执行事件队列
	std::queue<unsigned long> m_queEvent;

	// 执行事件保护
	OS_Mutex m_csEventExec;

	// 计时线程
	OS_Thread m_TimeThread;

	// 计时线程停止事件
	OS_Semaphore m_TimeStop;

	// 执行线程
	OS_Thread m_ExecThread;

	// 执行线程停止事件
	OS_Semaphore m_ExecStop;
};
