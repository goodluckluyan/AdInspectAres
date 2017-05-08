#include "C_Time.h"
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "C_LocalTime.h"
#include <string.h>
#include <stdlib.h>
C_Time::C_Time()
{

}

C_Time::~C_Time()
{

}

int C_Time::getTimeInt()
{
    return (int)mktime(&m_time);
}
void C_Time::getTimeStr(string &strTime)
{
    char str[64];
    memset(str,0,64);
    sprintf(str,"%d-%02d-%02d %02d:%02d:%02d", 1900+m_time.tm_year,
		m_time.tm_mon+1, m_time.tm_mday, m_time.tm_hour,
		m_time.tm_min, m_time.tm_sec);
     strTime = str;
}
void C_Time::getDateStr(string &strDate)
{
    char str[64];
    memset(str,0,64);
    sprintf(str,"%d-%02d-%02d", 1900+m_time.tm_year,
		m_time.tm_mon+1, m_time.tm_mday);
     strDate = str;   
}
int C_Time::setCurTime()
{
    time_t tmpTime;
    tmpTime = time(NULL);
    //After add CS;
    //wangzhongping delete  in 2012-7-15
    //tm *ptime = localtime(&tmpTime);
    //if(ptime == NULL)
    //{
     //   return -1;
    //}
    //m_time.tm_year = ptime->tm_year;
    //m_time.tm_mon = ptime->tm_mon;
    //m_time.tm_mday = ptime->tm_mday;
    //m_time.tm_hour = ptime->tm_hour;
    //m_time.tm_min = ptime->tm_min;
    //m_time.tm_sec = ptime->tm_sec;
    //delete end;
    
    //wangzhongping add  in 2012-7-15
    return C_LocalTime::GetInstance()->LocalTime(&tmpTime,m_time);
    // add end;
    
}
int C_Time::setTimeInt(int itime)
{
	//wangzhongping delete  in 2012-7-15
    //tm *ptime = localtime((time_t*)&itime);
    //if(ptime == NULL)
    //{
     //   return -1;
    //}
    //m_time.tm_year = ptime->tm_year;
   // m_time.tm_mon = ptime->tm_mon;
    //m_time.tm_mday = ptime->tm_mday;
   // m_time.tm_hour = ptime->tm_hour;
    //m_time.tm_min = ptime->tm_min;
    //m_time.tm_sec = ptime->tm_sec;
    //return 0;
    //delete end
    
     //wangzhongping add  in 2012-7-15
    time_t itm = itime;
    int ret = C_LocalTime::GetInstance()->LocalTime(&itm,m_time);
    return ret;
    // add end;
    
}
int C_Time::setTimeStr(string strTime)
{
    if(strTime.empty())
		return -1;
	
	//yyyy-mm-dd hh:mm:ss
	string::size_type beg = 0;
	string::size_type end = strTime.find('-', beg);
    if(end < beg)
    {
        return -1;
    }
	m_time.tm_year = atoi(strTime.substr(beg, end-beg).c_str()) - 1900;
    if(m_time.tm_year < 0)
    {
        return -1;
    }
	beg = end + 1;
	end = strTime.find('-', beg);
    if(end < beg)
    {
        return -1;
    }
	m_time.tm_mon = atoi(strTime.substr(beg, end-beg).c_str()) - 1;
    if(m_time.tm_mon < 0 || m_time.tm_mon>11)
    {
        return -1;
    }
	beg = end + 1;
	end = strTime.find(' ', beg);
    if(end < beg)
    {
        return -1;
    }
	m_time.tm_mday = atoi(strTime.substr(beg, end-beg).c_str());
    if(m_time.tm_mday<1 || m_time.tm_mday >31)
    {
        return -1;
    }
	beg = end + 1;
	end = strTime.find(':', beg);
    if(end < beg)
    {
        return -1;
    }
	m_time.tm_hour = atoi(strTime.substr(beg, end-beg).c_str());
    if(m_time.tm_hour<0 || m_time.tm_hour>23)
    {
        return -1;
    }
	beg = end + 1;
	end = strTime.find(':', beg);
    if(end < beg)
    {
        return -1;
    }
	m_time.tm_min = atoi(strTime.substr(beg, end-beg).c_str());
    if(m_time.tm_min<0 || m_time.tm_min>59)
    {
        return -1;
    }
	beg = end + 1;
	m_time.tm_sec = atoi(strTime.substr(beg).c_str());
    if(m_time.tm_sec<0 || m_time.tm_sec>59)
    {
        return -1;
    }
    
	m_time.tm_isdst = 0;

    return 0;
}



//@author liuhongjun@oristartech.com
//date 2012-07-19
//added begin
int  C_Time::setTimeTStr(string &strTime)
{
	if(strTime.empty())
		return -1;

	//yyyy-mm-dd hh:mm:ss
	string::size_type beg = 0;
	string::size_type end = strTime.find('-', beg);
	if(end < beg)
	{
		return -1;
	}
	m_time.tm_year = atoi(strTime.substr(beg, end-beg).c_str()) - 1900;
	if(m_time.tm_year < 0)
	{
		return -1;
	}
	beg = end + 1;
	end = strTime.find('-', beg);
	if(end < beg)
	{
		return -1;
	}
	m_time.tm_mon = atoi(strTime.substr(beg, end-beg).c_str()) - 1;
	if(m_time.tm_mon < 0 || m_time.tm_mon>11)
	{
		return -1;
	}
	beg = end + 1;
	end = strTime.find('T', beg);
	if(end < beg)
	{
		return -1;
	}
	m_time.tm_mday = atoi(strTime.substr(beg, end-beg).c_str());
	if(m_time.tm_mday<1 || m_time.tm_mday >31)
	{
		return -1;
	}
	beg = end + 1;
	end = strTime.find(':', beg);
	if(end < beg)
	{
		return -1;
	}
	m_time.tm_hour = atoi(strTime.substr(beg, end-beg).c_str());
	if(m_time.tm_hour<0 || m_time.tm_hour>23)
	{
		return -1;
	}
	beg = end + 1;
	end = strTime.find(':', beg);
	if(end < beg)
	{
		return -1;
	}
	m_time.tm_min = atoi(strTime.substr(beg, end-beg).c_str());
	if(m_time.tm_min<0 || m_time.tm_min>59)
	{
		return -1;
	}
	beg = end + 1;
	m_time.tm_sec = atoi(strTime.substr(beg).c_str());
	if(m_time.tm_sec<0 || m_time.tm_sec>59)
	{
		return -1;
	}

	m_time.tm_isdst = 0;

	return 0;
}
void C_Time::getTimeTStr(string &strTime)
{
	char str[64] = {0};
	sprintf(str,"%d-%02d-%02dT%02d:%02d:%02d", 1900+m_time.tm_year,
		m_time.tm_mon+1, m_time.tm_mday, m_time.tm_hour,
		m_time.tm_min, m_time.tm_sec);
	strTime = str;
}

void C_Time::getHhMmSs(string &strTime)
{
	char str[64] = {0};
	sprintf(str,"%02d:%02d:%02d", m_time.tm_hour,
		m_time.tm_min, m_time.tm_sec);
	strTime = str;
}
//added end
