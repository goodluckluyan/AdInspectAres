//@file:C_LocalTime.h
//@brief: 包含类C_LocalTime。
//C_LocalTime 为了防止系统调用localtime同时被多个线程调用产生的
//时间紊乱 把localtime（）封装到一个单件类里面。
//@author:wangzhongping@oristartech.com
//dade:2012-07-15

#ifndef TMS20_LOCAL_TIME
#define TMS20_LOCAL_TIME

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "../threadManage/C_CS.h"
class C_LocalTime
{
public:
		~C_LocalTime();
		static C_LocalTime* GetInstance();
		//函数内部用互斥方式调用localtime();
		//返回值0 成功， 其他为错误码 
		//参数 ：time [in] 需要转换的时间值的地址
		//TM [out] 存储转换后的时间；
    int LocalTime(time_t* time, tm &TM);
private:
		static C_LocalTime* m_pInstance;
		C_CS m_CS;
protected:
		C_LocalTime();
};
#endif //TMS20_LOCAL_TIME
