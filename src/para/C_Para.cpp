//@file:C_Para.cpp
//@brief: 包含类C_Para 的方法实现
//@author:wangzhongping@oristartech.com
//dade:2012-07-12

#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "C_Para.h"
#include "ec_config.h"
#include "log/MyLogger.h"

extern MyLogger g_main_logwrite;
#define loginfo(strlog,...)    g_main_logwrite.PrintLog(MyLogger::INFO,strlog,##__VA_ARGS__)
#define logerror(strlog,...)   g_main_logwrite.PrintLog(MyLogger::ERROR,strlog,##__VA_ARGS__)
#define logdebug(strlog,...)   g_main_logwrite.PrintLog(MyLogger::DEBUG,strlog,##__VA_ARGS__)
#define logfatal(strlog,...)   g_main_logwrite.PrintLog(MyLogger::FATAL,strlog,##__VA_ARGS__)

C_Para *C_Para::m_pInstance = NULL;

C_Para::C_Para()
{
    m_WeightThreshold = 0.1;
    m_IsSpeedPriority = false;
    m_InvalidateShowThresholdSec = 10;
	pthread_rwlock_init(&m_rwlk_main,NULL);

}
C_Para::~C_Para()
{
	pthread_rwlock_destroy(&m_rwlk_main);
}
C_Para* C_Para::GetInstance()
{
	if(m_pInstance == NULL)
	{
		m_pInstance = new C_Para;
	}
	return m_pInstance;
}

void  C_Para::DestoryInstance()
{
	if(m_pInstance != NULL)
	{
		delete m_pInstance ;
		m_pInstance = NULL;
	}

}
int C_Para::ReadPara()
{
	int iResult = -1;
	ec_config config;
    char a[64];
	memset(a,0,64);

	char tmp[256];
	char buf[256];
	memset(tmp, 0, 256);
	memset(buf, 0, 256);

	sprintf(tmp,"/proc/%d/exe",getpid());
	readlink(tmp,buf,256);
	string str = buf;
	size_t iPos = -1;
	if((iPos =str.rfind('/')) == string::npos)
	{
		return -1;
	}
	m_strInipath = str.substr(0,iPos);
	string strInipath = m_strInipath;
    strInipath += "/Ares.cfg";

	iResult = config.readvalue("PARA","DBServiceIP", a, strInipath.c_str());
	if(iResult != 0)
	{
		return iResult;
	}
    m_DB_IP = a;
    loginfo("reading config ,database ip :%s",m_DB_IP.c_str());

	memset(a,0,64);
	iResult = config.readvalue("PARA","DBServicePort",a, strInipath.c_str());
	if(iResult != 0)
	{
		return iResult;
	}
	int iTmp  =  atoi(a); 
	if(iTmp <= 0)
	{
		return -1;
	}
    m_DB_Port = (unsigned short) iTmp;
    loginfo("reading config ,database port :%d",m_DB_Port);


	memset(a,0,64);    
	iResult = config.readvalue("PARA","DBUserName",a,strInipath.c_str());
	if(iResult != 0)
	{
		return iResult;
	}
    m_DB_User = a;
    loginfo("reading config ,database username :%s",m_DB_User.c_str());

	memset(a,0,64);
	iResult = config.readvalue("PARA","DBPWD",a,strInipath.c_str());
	if(iResult != 0)
	{
		return iResult;
	}
    m_DB_Passwd = a;
    loginfo("reading config ,database passwd :%s",m_DB_Passwd.c_str());

	memset(a,0,64);
	iResult = config.readvalue("PARA","LogPath",a,strInipath.c_str());
	if(iResult != 0)
	{
		return iResult;
	}
	m_strLogPath = a; 
    loginfo("reading config ,log path :%s",m_strLogPath.c_str());
	
	memset(a,0,64);
	iResult = config.readvalue("PARA","WebServicePort",a,strInipath.c_str());
	if(iResult != 0)
	{
		return iResult;
	}
	m_nWebServicePort = atoi(a) ;
    m_nWebServicePort = m_nWebServicePort <= 0 ?12342 :m_nWebServicePort;
    loginfo("reading config ,WebService Port :%d",m_nWebServicePort);

	memset(a,0,64);
    iResult = config.readvalue("PARA","AdShowSec",a,strInipath.c_str());
	if(iResult != 0)
	{
		return iResult;
	}
    m_AdShow_sec = atoi(a) ;
    loginfo("reading config ,Show Adertisement Time(sec):%d",m_AdShow_sec);

    memset(a,0,64);
    iResult = config.readvalue("PARA","VideoFileSplitSec",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_VideoFile_SplitSec = atoi(a) ;
    loginfo("reading config ,video file split stage(sec):%d",m_VideoFile_SplitSec);

    memset(a,0,64);
    iResult = config.readvalue("PARA","Decoderate",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_Decode_rate = atoi(a) ;
    m_Max_frame_count = m_AdShow_sec*m_Decode_rate;
    loginfo("reading config ,decode frame rate persecond:%d",m_Decode_rate);
    loginfo("reading config ,buffer max frame:%d(AdShowSec*DecodeRate)",m_Max_frame_count);

    memset(a,0,64);
    iResult = config.readvalue("PARA","MarkPath",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_Mark_path = a ;
    if(m_Mark_path.substr(0,1)==".")
    {
        m_Mark_path.replace(0,1,m_strInipath);
    }
    loginfo("reading config ,Longbiao templet path:%s",m_Mark_path.c_str());


    memset(a,0,64);
    iResult = config.readvalue("PARA","DownloadVedioPath",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_Download_Vedio_path = a ;
    if(m_Download_Vedio_path.substr(0,1)==".")
    {
        m_Download_Vedio_path.replace(0,1,m_strInipath);
    }
    loginfo("reading config ,Vedio file of download path :%s",m_Download_Vedio_path.c_str());

    memset(a,0,64);
    iResult = config.readvalue("PARA","MarkStorePath",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_Mark_Store_path = a ;
    if(m_Mark_Store_path.substr(0,1)==".")
    {
        m_Mark_Store_path.replace(0,1,m_strInipath);
    }
    loginfo("reading config ,save path what success of compare inspect image to longbiao :%s",
            m_Mark_Store_path.c_str());

    memset(a,0,64);
    iResult = config.readvalue("PARA","TempletStorePath",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_Templet_Store_path = a ;
    if(m_Templet_Store_path.substr(0,1)==".")
    {
        m_Templet_Store_path.replace(0,1,m_strInipath);
    }
    loginfo("reading config ,save path what templet image  :%s",m_Templet_Store_path.c_str());

    memset(a,0,64);
    iResult = config.readvalue("PARA","TempletVideoPath",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_Templet_Video_path = a ;
    loginfo("reading config ,templet video store path  :%s",m_Templet_Video_path.c_str());

    memset(a,0,64);
    iResult = config.readvalue("PARA","DecodeWidth",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_Decode_width = atoi(a) ;
    loginfo("reading config ,Decode Width:%d",m_Decode_width);


    memset(a,0,64);
    iResult = config.readvalue("PARA","DecodeHeight",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_Decode_height = atoi(a) ;
    loginfo("reading config ,Decode height:%d",m_Decode_height);

    memset(a,0,64);
    iResult = config.readvalue("PARA","IsGray",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_bIsGray = atoi(a)==1?true:false ;
    loginfo("reading config ,is transform gray image:%s",m_bIsGray?"true":"false");

    memset(a,0,64);
    iResult = config.readvalue("PARA","CompareRect_left",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_CompareRect.left = atoi(a) ;

    memset(a,0,64);
    iResult = config.readvalue("PARA","CompareRect_top",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_CompareRect.top = atoi(a) ;

    memset(a,0,64);
    iResult = config.readvalue("PARA","CompareRect_right",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_CompareRect.right = atoi(a) ;

    memset(a,0,64);
    iResult = config.readvalue("PARA","CompareRect_bottom",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_CompareRect.bottom = atoi(a) ;
    loginfo("reading config ,compare image area:l:%d t:%d r:%d b:%d",
            m_CompareRect.left,m_CompareRect.top,m_CompareRect.right,m_CompareRect.bottom);

    memset(a,0,64);
    iResult = config.readvalue("PARA","MatchResutlStorePath",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_match_store_path = a ;
    if(m_match_store_path.substr(0,1)==".")
    {
        m_match_store_path.replace(0,1,m_strInipath);
    }
    loginfo("reading config ,save path what match success:%s",m_match_store_path.c_str());

    memset(a,0,64);
    iResult = config.readvalue("PARA","ShowingJobStartTime",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    str = a ;
    m_ShowingJobStartSec = atoi(str.substr(0,str.find(":")).c_str())*3600+atoi(str.substr(str.find(":")+1).c_str())*60;


    memset(a,0,64);
    iResult = config.readvalue("PARA","ShowingJobEndTime",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    str = a ;
    m_ShowingJobEndSec = atoi(str.substr(0,str.find(":")).c_str())*3600+atoi(str.substr(str.find(":")+1).c_str())*60+59;
    loginfo("reading config ,cinema show time:%d-%d",m_ShowingJobStartSec,m_ShowingJobEndSec);


    memset(a,0,64);
    iResult = config.readvalue("PARA","WeightThreshold",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_WeightThreshold = atof(a);
    loginfo("reading config ,WeightThreshold:%f",m_WeightThreshold);

    memset(a,0,64);
    iResult = config.readvalue("PARA","IsSpeedPriority",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_IsSpeedPriority = atoi(a)==1?true:false;
    loginfo("reading config ,IsSpeedPriority:%s",m_IsSpeedPriority?"Yes":"No");

    memset(a,0,64);
    iResult = config.readvalue("PARA","InvalidateShowThresholdSec",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    m_InvalidateShowThresholdSec = atoi(a);
    loginfo("reading config ,InvalidateShowThresholdSec:%d",m_InvalidateShowThresholdSec);


    memset(a,0,64);
    iResult = config.readvalue("PARA","FeatrueType",a,strInipath.c_str());
    if(iResult != 0)
    {
        return iResult;
    }
    std::string tmpstr = a;
    std::transform(tmpstr.begin(),tmpstr.end(),tmpstr.begin(),std::towlower);
    m_FeatrueType = tmpstr;
    loginfo("reading config ,m_FeatrueType:%s",m_FeatrueType.c_str());


	return 0;
}


