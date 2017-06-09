#ifndef _TIME_CTRL
#define _TIME_CTRL
#include <time.h>
#include <string>
#include <stdio.h>
#include <stddef.h>

using namespace std;
class C_Time
{
public:
    C_Time();
    ~C_Time();
    void getTimeStr(std::string &strTime);
    void getDateStr(string &strDate);
    int getTimeInt();
    int setTimeStr(string strTime);
    int setTimeInt(int itime);
    int setCurTime();
    int getMillisecond();
    int getYear()
    {
        return m_time.tm_year +1900;
    }
    int getMonth()
    {
        return m_time.tm_mon +1; 
    }
    int getDay()
    {
        return m_time.tm_mday;
    }
    int getHour()
    {
        return m_time.tm_hour;
    }
    int getMinute()
    {
        return m_time.tm_min;
    }
    int getSecond()
    {
        return m_time.tm_sec;
    }

	int getWeek()
	{
		return m_time.tm_wday;
	}

    tm gettm()
    {
        return m_time;
    }

	//@author liuhongjun@oristartech.com
	//date 2012-07-19
	//added begin
	int setTimeTStr(string &strTime);
	void getTimeTStr(string &strTime);
	void getHhMmSs(string &strTime);
	//added end


private:
     tm m_time;
};
#endif //_TIME_CTRL
