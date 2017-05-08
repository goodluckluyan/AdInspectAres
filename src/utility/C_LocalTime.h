//@file:C_LocalTime.h
//@brief: ������C_LocalTime��
//C_LocalTime Ϊ�˷�ֹϵͳ����localtimeͬʱ������̵߳��ò�����
//ʱ������ ��localtime������װ��һ�����������档
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
		//�����ڲ��û��ⷽʽ����localtime();
		//����ֵ0 �ɹ��� ����Ϊ������ 
		//���� ��time [in] ��Ҫת����ʱ��ֵ�ĵ�ַ
		//TM [out] �洢ת�����ʱ�䣻
    int LocalTime(time_t* time, tm &TM);
private:
		static C_LocalTime* m_pInstance;
		C_CS m_CS;
protected:
		C_LocalTime();
};
#endif //TMS20_LOCAL_TIME
