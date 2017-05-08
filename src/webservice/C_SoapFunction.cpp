#include "mons.nsmap"
#include "soapH.h"
#include <iostream>
#include <cstdio>
#include <tr1/memory>
#include "MainProcess.h"
#include "C_constDef.h"


extern std::tr1::shared_ptr<CMainProcess> ptr_MainProcess ;
#define  LOGINFFMT(errid,fmt,...)  C_LogManage::GetInstance()->WriteLogFmt(ULOG_INFO,LOG_MODEL_JOBS,0,errid,fmt,##__VA_ARGS__)


int mons__AddInspectModule(struct soap* cSoap,std::string id,std::string OrderNO,std::string AdName,int CinemaNum,std::string start,
                           std::string end,int ShowOder,int Type,std::string ModulePath,int&ret)
{
   ret = ptr_MainProcess->WS_AddInspectModule(id,OrderNO,AdName,CinemaNum,start,end,ShowOder,Type,ModulePath);
   return 0;
}


