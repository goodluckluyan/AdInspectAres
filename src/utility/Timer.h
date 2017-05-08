/*******************************************************************************
* ��Ȩ���� (C) 2012
* 
* �ļ����ƣ� Timer.h
* �ļ���ʶ�� 
* ����ժҪ�� ��ʱ������
* ����˵���� 
* ��ǰ�汾�� V1.0
* ��    �ߣ� �׸���
* ������ڣ� 2012-10-26
*******************************************************************************/
#pragma once

#include <map>
#include <queue>
#include "osapi/osapi.h"

class CTimer
{

	// �¼�ӳ����¼����ϴ�ִ��ʱ�䡢ʱ����
	typedef std::map<int, std::pair<OS_TimeVal, int> > EventMap;

public:

	// ���캯��
	CTimer()
	{
		m_csEventTime.Init();
		m_csEventExec.Init();

		m_TimeStop.Init();
		m_ExecStop.Init();

		m_TimeThread.Run(TH_Time, this);
		m_ExecThread.Run(TH_Exec, this);
	}

	// ��������
	virtual ~CTimer()
	{
		Stop();
	}

	// ���ö�ʱ��
	void SetTimer(
		int nEventID,							// �¼�ID
		int nTime,								// ʱ��������λ������
		bool bExec = false						// ����ִ��
		)
	{
		m_csEventTime.Lock();
		OS_TimeVal tmNow;
		OS_Time::GetLocalTime(&tmNow);
		m_mapEvent[nEventID] = std::make_pair(tmNow, nTime);
		m_csEventTime.Unlock();

		if (bExec)
		{
			// ��������ִ�ж���
			m_csEventExec.Lock();
			m_queEvent.push(nEventID);
			m_csEventExec.Unlock();
		}
	}

	// ������ʱ��
	void KillTimer(
		int nEventID							// �¼�ID
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

	// ֹͣ����
	void Stop()
	{
		m_TimeStop.Post();
		m_ExecStop.Post();

		OS_Thread::Join(&m_TimeThread);
		OS_Thread::Join(&m_ExecThread);
	}

protected:

	// ��ʱ������
	virtual void OnTimer(
		int nEventID							// �¼�ID
		)
	{}

	// ��ʱ�̺߳���
	static void TH_Time(void * lp)
	{
		CTimer *pThis = (CTimer *)lp;
		if (NULL != pThis)
		{
			pThis->TimeProc();
		}
	}

	// ��ʱ����
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
					// �ﵽ��ʱ���
					m_csEventExec.Lock();
					m_queEvent.push(it->first);
					m_csEventExec.Unlock();

					it->second.first = tmNow;
				}
				else if (it->second.first.MsTo(tmNow) < 0)
				{
					// ʱ�䷢����������
					it->second.first = tmNow;
				}
			}
			m_csEventTime.Unlock();
		}
	}

	// ִ���̺߳���
	static void TH_Exec(void * lp)
	{
		CTimer *pThis = (CTimer *)lp;
		if (NULL != pThis)
		{
			pThis->ExecProc();
		}
	}

	// ִ�д���
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

			// ��ʱ��ִ��
			OnTimer(nEventID);
		}
	}

protected:

	// ��ʱ�¼�ӳ����¼����ϴ�ִ��ʱ��
	EventMap m_mapEvent;

	// ��ʱ�¼�����
	OS_Mutex m_csEventTime;

	// ִ���¼�����
	std::queue<unsigned long> m_queEvent;

	// ִ���¼�����
	OS_Mutex m_csEventExec;

	// ��ʱ�߳�
	OS_Thread m_TimeThread;

	// ��ʱ�߳�ֹͣ�¼�
	OS_Semaphore m_TimeStop;

	// ִ���߳�
	OS_Thread m_ExecThread;

	// ִ���߳�ֹͣ�¼�
	OS_Semaphore m_ExecStop;
};
