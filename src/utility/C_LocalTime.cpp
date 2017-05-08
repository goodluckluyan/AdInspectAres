//@file:C_LocalTime.cpp
//@brief: 包含类C_LocalTime的实现。
//@author:wangzhongping@oristartech.com
//dade:2012-07-15

#include "C_LocalTime.h"
C_LocalTime * C_LocalTime::m_pInstance = NULL;
C_LocalTime::C_LocalTime()
{

}

C_LocalTime::~C_LocalTime()
{
	if(m_pInstance != NULL)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
	
}

C_LocalTime * C_LocalTime::GetInstance()
{
	if(m_pInstance == NULL)
	{
		m_pInstance = new C_LocalTime;
	}
	return m_pInstance;
}

int C_LocalTime::LocalTime(time_t* time, tm &TM)
{
	m_CS.EnterCS();
	tm *ptime = localtime(time);
  if(ptime == NULL)
  {
  	 m_CS.LeaveCS();
     return -1; //Add ErrorCtrl and Log
  }
  TM.tm_year = ptime->tm_year;
  TM.tm_mon = ptime->tm_mon;
  TM.tm_mday = ptime->tm_mday;
  TM.tm_hour = ptime->tm_hour;
  TM.tm_min = ptime->tm_min;
  TM.tm_sec = ptime->tm_sec;
  m_CS.LeaveCS();
  return 0;
}
