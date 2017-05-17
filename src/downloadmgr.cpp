#include "C_ErrorDef.h"
#include "parser_xml.h"
#include "utility/C_Time.h"
#include "utility/C_HttpParser.h"
#include "utility/C_TcpTransport.h"
#include "downloadmgr.h"
#include "para/C_Para.h"
#include "utility/FileEx.h"


extern bool g_bAresQuit;
extern MyLogger g_download_logwrite;
#define loginfo(strlog,...)   g_download_logwrite.PrintLog(MyLogger::INFO,strlog,##__VA_ARGS__)
#define logerror(strlog,...)   g_download_logwrite.PrintLog(MyLogger::ERROR,strlog,##__VA_ARGS__)
#define logdebug(strlog,...)   g_download_logwrite.PrintLog(MyLogger::DEBUG,strlog,##__VA_ARGS__)
#define logfatal(strlog,...)   g_download_logwrite.PrintLog(MyLogger::FATAL,strlog,##__VA_ARGS__)


void CDownLoadMgr::SetDownloadCompete_BCFun(void * userdata,CompleteDLT_CallBack pfun)
{
    m_pDownloadCompeteFun = pfun;
    m_Userdata = userdata;
}


/*******************************************************************************
* 函数名称：	CDownLoadMgr
* 功能描述：	构造函数
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
CDownLoadMgr::CDownLoadMgr()
{
    pthread_mutex_init(&m_mutx,NULL);
    pthread_cond_init(&m_cond,NULL);
}

/*******************************************************************************
* 函数名称：	~CDownLoadMgr
* 功能描述：	析构函数
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
CDownLoadMgr::~CDownLoadMgr()
{
    pthread_mutex_lock(&m_mutx);
    pthread_cond_signal(&m_cond);
    pthread_mutex_unlock(&m_mutx);
}

/*******************************************************************************
* 函数名称：	Init
* 功能描述：	初始化
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
int CDownLoadMgr::Init(int HallID, int CameraPos, DBLoginInfo &DBinfo, std::string wsip, int wsport,int TimeStep)
{
    m_HallID = HallID;
    m_CameraPos = CameraPos;
    m_NVRIP = wsip;
    m_NVRPort = wsport;
    m_TimeStep = TimeStep;

     C_Para* para = C_Para::GetInstance();
     char tmpbuf[512]={'\0'};
     m_savepath = para->m_Download_Vedio_path;
     if(m_savepath.rfind("/")!= m_savepath.size()-1)
     {
         m_savepath += "/";
     }
     snprintf(tmpbuf,512,"%s%d", m_savepath.c_str(),m_HallID);
     m_savepath = tmpbuf;
     CFileEx::CreateFolder(m_savepath.c_str());


    m_startsec = para->m_ShowingJobStartSec;
    m_endsec = para->m_ShowingJobEndSec;
    if(m_DownLoad_DB.open(DBinfo.ip.c_str(),DBinfo.username.c_str(),
            DBinfo.passwd.c_str(),"NVRControl") == -1)
    {
            logerror("NVRControl database open failed!\n");
            return -1;
    }


    if(m_CompleteFile_DB.open(DBinfo.ip.c_str(),DBinfo.username.c_str(),
            DBinfo.passwd.c_str(),"AdInspect") == -1)
    {
            logerror("AdInspect databases open failed!\n");
            return -1;
    }


}

/*******************************************************************************
* 函数名称：	AddDownTask
* 功能描述：	添加下载任务
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
bool CDownLoadMgr::AddDownTask(DownLoadInfoItem &taskitem)
{
    pthread_mutex_lock(&m_mutx);
    m_lstDownloadTask.push_back(taskitem);
    pthread_cond_signal(&m_cond);
    pthread_mutex_unlock(&m_mutx);
}


/*******************************************************************************
* 函数名称：	AddSampleDownTask
* 功能描述：	添加下载任务
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
bool CDownLoadMgr::AddResultSampleDownTask(DownLoadInfoItem &taskitem)
{
    pthread_mutex_lock(&m_mutx);
    m_lstSampleDownloadTask.push_back(taskitem);
    pthread_cond_signal(&m_cond);
    pthread_mutex_unlock(&m_mutx);

    loginfo("Add result download task (%d-%d %s %s) ", taskitem.start, taskitem.duration,
            taskitem.savepath.c_str(),taskitem.filename.c_str());
}

/*******************************************************************************
* 函数名称：	AddHaskTask
* 功能描述：	添加下载任务，以最后一次的结束时间为开始时间
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
bool CDownLoadMgr::AddDownTask()
{

    C_Para *para = C_Para::GetInstance();
    DownLoadInfoItem item;
    if(m_lastsec == 0)
    {
        time_t tm;
        time(&tm);
        m_lastsec = (int)tm-m_TimeStep;

    }


    // 当前日期的零点秒数
    time_t iCurTime;
    time(&iCurTime);
    char str[20]={'\0'};
    sprintf(str,"%d-%02d-%02d", 1900+localtime(&iCurTime)->tm_year,localtime(&iCurTime)->tm_mon+1,
            localtime(&iCurTime)->tm_mday);
    C_Time t2;
    std::string curday = std::string(str) + " 00:00:00";
    t2.setTimeStr(curday);
    int iStarTime = t2.getTimeInt() + m_startsec;
    int iEndTime = t2.getTimeInt() + m_endsec;

    // 判断在不在放映时间内
    if(m_lastsec+1 > iEndTime || m_lastsec + 1 < iStarTime)
    {
        C_Time cur,start,end;
        cur.setTimeInt(m_lastsec);
        std::string curTime,strStart,strEnd;
        cur.getTimeStr(curTime);

        start.setTimeInt(iStarTime);
        start.getTimeStr(strStart);
        end.setTimeInt(iEndTime);
        end.getTimeStr(strEnd);
        loginfo("Current time not in show time range(%s %s-%s)",curTime.c_str(),
                strStart.c_str(),strEnd.c_str());
        return false;
    }

    item.start = m_lastsec + 1;
    item.savepath = m_savepath;
    int itemend = item.start + m_TimeStep-1;
    if(itemend > iEndTime)
    {
        item.duration = iEndTime-item.start + 1;
    }
    else
    {
         item.duration = m_TimeStep;
    }


    AddDownTask(item);
    m_lastsec = item.start + item.duration;

    loginfo("Add last time period  download task (%ud-%ud)", static_cast<unsigned int>(item.start),
            static_cast<unsigned int>(item.duration));
}

/*******************************************************************************
* 函数名称：	ProcessDownTask
* 功能描述：	任务轮询
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
void CDownLoadMgr::ProcessDownTask()
{

    pthread_mutex_lock(&m_mutx);

    if(m_lstSampleDownloadTask.size()==0 && m_lstDownloadTask.size() == 0)
    {
        pthread_cond_wait(&m_cond,&m_mutx);
    }

    pthread_mutex_unlock(&m_mutx);

    std::string filepath;
    if(m_lstSampleDownloadTask.size()!=0)
    {
        DownLoadInfoItem &task = m_lstSampleDownloadTask.front();
        char taskid[32]={'\0'};
        snprintf(taskid,32,"%d-%d-%d-%d",m_HallID,m_CameraPos,task.start,task.duration);

        // 转换成可读时间
        C_Time t1,t2;
        t1.setTimeInt(task.start);
        t2.setTimeInt(task.start+task.duration-1);
        std::string strStart,strEnd;
        t1.getTimeStr(strStart);
        t2.getTimeStr(strEnd);

        // 开始下载
        long long downloadid;
        int i = 0;
        while(i<3)
        {
            i++;
            if(DownVidoFile(task,downloadid,filepath))
            {
                loginfo("execute result download task %lld(%d-%d %s:%s)",downloadid,
                        strStart.c_str() ,strEnd.c_str(),task.savepath.c_str(),filepath.c_str());
                break;
            }
             sleep(1);
        }

        if(i==3)
        {
            logerror("execute result download task failed(%s-%s %s)",
                     strStart.c_str() ,strEnd.c_str(),task.savepath.c_str());
            pthread_mutex_lock(&m_mutx);
            m_lstSampleDownloadTask.pop_front();
            pthread_mutex_unlock(&m_mutx);
            return;
        }

        // 轮询下载是否完成
        bool bComplete = false;
        while(1)
        {
            if(QueryISDownDone_DB(downloadid))
            {
                bComplete = true;
                break;
            }

            if(g_bAresQuit)
            {
                return ;
            }
            ::sleep(1);
        }

        // 完成后记录到下载完成表
        InsertDownTask_DB(taskid,m_HallID,m_CameraPos,strStart,strEnd,filepath,true);

        // 完成后通过回调函数通知主模块
        if(bComplete && m_pDownloadCompeteFun != NULL)
        {

            m_pDownloadCompeteFun(m_Userdata,m_HallID,m_CameraPos,task.start,
                                  task.duration,filepath,true);
        }

        pthread_mutex_lock(&m_mutx);
        m_lstSampleDownloadTask.pop_front();
        pthread_mutex_unlock(&m_mutx);

    }
    else if(m_lstDownloadTask.size()!=0)
    {


        DownLoadInfoItem &task = m_lstDownloadTask.front();
        char taskid[32]={'\0'};
        snprintf(taskid,32,"%d-%d-%d-%d",m_HallID,m_CameraPos,task.start,task.duration);

         // 转换成可读时间
        C_Time t1,t2;
        std::string strStart,strEnd;
        t1.setTimeInt(task.start);
        t1.getTimeStr(strStart);
        t2.setTimeInt(task.start + task.duration - 1);
        t2.getTimeStr(strEnd);

        // 是否已经存在
        int status;
        if(QueryCompletFileInfo_DB(taskid,status,filepath))
        {
            // 状态为INACTIVITY说明已经不需要再进行龙标检测和对比了
            if(status == INACTIVITY)
            {
                pthread_mutex_lock(&m_mutx);
                m_lstDownloadTask.pop_front();
                pthread_mutex_unlock(&m_mutx);
                return ;
            }
            else//没有COMPLETE 就说明这个文件会在检测龙标时用到（存在龙标或广告）
            {
                if(m_pDownloadCompeteFun != NULL)
                {
                    m_pDownloadCompeteFun(m_Userdata,m_HallID,m_CameraPos,task.start,
                                          task.duration,filepath,false);
                }
            }
        }
        else
        {
            // 开始下载
            long long downloadid;
            int i = 0;
            while(i<3)
            {
                i++;
                if(DownVidoFile(task,downloadid,filepath))
                {
                    loginfo("execute download task %lld(%s-%s %s:%s)",downloadid,
                           strStart.c_str() ,strEnd.c_str(),task.savepath.c_str(),filepath.c_str());
                    break;
                }
                sleep(1);
            }

            if(i==3)
            {
                logerror("execute download task failed(%s-%s %s)",
                         strStart.c_str() ,strEnd.c_str(),task.savepath.c_str());

                pthread_mutex_lock(&m_mutx);
                m_lstDownloadTask.pop_front();
                pthread_mutex_unlock(&m_mutx);
                return;
            }

            // 轮询下载是否完成
            bool bComplete = false;
            while(1)
            {
                if(QueryISDownDone_DB(downloadid))
                {
                    bComplete = true;
                    break;
                }

                if(g_bAresQuit)
                {
                    return ;
                }
                ::sleep(1);
            }

            // 完成后记录到下载完成表
            InsertDownTask_DB(taskid,m_HallID,m_CameraPos,strStart,strEnd,filepath);

            // 完成后通过回调函数通知主模块
            if(bComplete && m_pDownloadCompeteFun != NULL)
            {

                m_pDownloadCompeteFun(m_Userdata,m_HallID,m_CameraPos,task.start,
                                      task.duration,filepath,false);
            }

        }

        pthread_mutex_lock(&m_mutx);
        m_lstDownloadTask.pop_front();
        pthread_mutex_unlock(&m_mutx);
    }

}


/*******************************************************************************
* 函数名称：	ProcessDownTask
* 功能描述：	添加一天的下载任务
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
bool CDownLoadMgr::AddDayTask(struct tm &day)
{
    //计算当天的0点时间
    int iCurTime = mktime(&day);
    char str[20]={'\0'};
    sprintf(str,"%d-%02d-%02d", 1900+day.tm_year,
        day.tm_mon+1, day.tm_mday);
    C_Time t2;
    std::string curday = std::string(str) + " 00:00:00";
    t2.setTimeStr(curday);
    int iZeroTime = t2.getTimeInt() + m_startsec;

    int endsec = std::min((int)iCurTime,t2.getTimeInt()+m_endsec);
    for(;iZeroTime < endsec;iZeroTime += m_TimeStep)
    {
        DownLoadInfoItem item;
        item.start = iZeroTime;
        item.duration = m_TimeStep;
        item.savepath = m_savepath;
        if(item.start+item.duration>endsec)
        {
            break;
        }
        AddDownTask(item);
        m_lastsec = item.start+item.duration-1;
    }
}


/*******************************************************************************
* 函数名称：	SendAndRecvResponse
* 功能描述：	 把调用xml串以http方式发送到服务端并接收返回xml
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
int CDownLoadMgr::SendAndRecvResponse(const std::string &request, std::string &response, int delayTime)
{
    if(m_NVRIP.empty())
    {
        return 0;
    }

    TcpTransport tcp;
    int result = tcp.TcpConnect(m_NVRIP.c_str(), m_NVRPort);
    if(result < 0)
    {
//        LOGFATFMT(ULOG_ERROR,"CMonitorSensor::SendAndRecvResponse TcpConnect %s:%d Fail !\n",strIP.c_str(), m_nPort);
        result = tcp.TcpConnect(m_NVRIP.c_str(), m_NVRPort);
        if(result < 0)
        {
            return  ERROR_SENSOR_TCP_CONNECT;
        }

    }

    result = tcp.BlockSend(request.c_str(), request.size());
    if(result < 0)
    {
//        LOGFATFMT(0,"CMonitorSensor::SendAndRecvResponse Tcp Send %s Fail !\n",request.c_str());
        return  ERROR_SENSOR_TCP_SEND;
    }


    char buffer[1024];
    timeval timeOut;
    timeOut.tv_sec = delayTime;
    timeOut.tv_usec = 0;
    response.clear();
    while((result = tcp.SelectRecv(buffer, 1024-1, timeOut)) >= 0)
    {
        if(result == 0)
            break;
        buffer[result] = 0;
        response += buffer;

        if(response.find(":Envelope>") != std::string::npos)
            break;
        timeOut.tv_sec = 2;
        timeOut.tv_usec = 0;
    }

    return result <0 ? ERROR_SENSOR_TCP_RECV : 0;

}


/*******************************************************************************
* 函数名称：	GetHttpContent
* 功能描述：	获取http中的xml
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
int CDownLoadMgr::GetHttpContent(const std::string &http, std::string &response)
{
    HttpResponseParser httpResponse;
    int result = httpResponse.SetHttpResponse(http);
    if(result != 0)
    {
        return result;
    }

    response = httpResponse.GetContent();
    return httpResponse.GetStatus();
}


/*******************************************************************************
* 函数名称：	InvokerWebServer
* 功能描述：	调用webservice接口
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
int CDownLoadMgr::InvokerWebServer(std::string &xml,std::string &strResponse)
{
    HttpRequestParser request;
    request.SetMethod("POST");
    request.SetUri("/");
    request.SetVersion("HTTP/1.1");
    request.SetHost(m_NVRIP.c_str());
    request.SetContentType("text/xml; charset=utf-8");
    request.SetContent(xml);
    request.SetSoapAction("");
    std::string strHttp = request.GetHttpRequest();

    int result = SendAndRecvResponse(strHttp, strResponse);

    return result;

}


// 获取另一台主机的调度程序的状态
/*******************************************************************************
* 函数名称：	InvokerWebServer
* 功能描述：	调用webservice接口
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
bool CDownLoadMgr::DownVidoFile(DownLoadInfoItem &task,long long &downloadid,std::string &filename)
{

    C_Time t1,t2;
    t1.setTimeInt(task.start);
    t2.setTimeInt(task.start+task.duration-1);



    char buf[255]={'\0'};

    std::string xml = "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" ";
    xml += "xmlns:mons=\"http://tempuri.org/mons.xsd\"><soapenv:Header/>";
    xml += "<soapenv:Body><mons:DownLoadByTime>";
    xml += "<id>0</id>";
    snprintf(buf,255,"<nAudiNU>%d</nAudiNU><nDevicePo>%d</nDevicePo>",m_HallID,m_CameraPos);
    xml += std::string(buf);
    snprintf(buf,255,"<tmStart><dwYear>%d</dwYear><dwMonth>%d</dwMonth><dwDay>%d</dwDay>",t1.getYear(),t1.getMonth(),t1.getDay());
    xml += std::string(buf);
    snprintf(buf,255,"<dwHour>%d</dwHour><dwMinute>%d</dwMinute><dwSecond>%d</dwSecond></tmStart>",t1.getHour(),t1.getMinute(),t1.getSecond());
    xml += std::string(buf);
    snprintf(buf,255,"<tmEnd><dwYear>%d</dwYear><dwMonth>%d</dwMonth><dwDay>%d</dwDay>",t2.getYear(),t2.getMonth(),t2.getDay());
    xml += std::string(buf);
    snprintf(buf,255,"<dwHour>%d</dwHour><dwMinute>%d</dwMinute><dwSecond>%d</dwSecond></tmEnd>",t2.getHour(),t2.getMinute(),t2.getSecond());
    xml += std::string(buf);
    snprintf(buf,255,"<storePath>%s</storePath>",task.savepath.c_str());
    xml += std::string(buf);
    snprintf(buf,255,"<mp4name>%s</mp4name>",task.filename.c_str());
    xml += std::string(buf);
    xml += "</mons:DownLoadByTime></soapenv:Body></soapenv:Envelope>";



    // 通过http方式调用另一个调度软件的WebService服务
    std::string strResponse;
    int nInvokeRes = InvokerWebServer(xml,strResponse);
    if( nInvokeRes == ERROR_SENSOR_TCP_RECV || nInvokeRes == ERROR_SENSOR_TCP_CONNECT
            || nInvokeRes == ERROR_SENSOR_TCP_SEND)
    {
        return false;
    }

    // 提取xml
    std::string retXml;
    int result = GetHttpContent(strResponse, retXml);
    if(retXml.empty())
    {
//        LOGFATFMT(0,"GetOtherMonitorState:Parse Fail! xml is empty!\n");
        return false;
    }

    // 解析xml读取结果
    bool bRet = false;
    if(ParseDownLoadXml(retXml,downloadid,filename))
    {
        bRet = true;
    }

    return bRet;
}
using namespace xercesc;
/*******************************************************************************
* 函数名称：	ParseDownLoadXml
* 功能描述：     解析xml的返回
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
bool CDownLoadMgr::ParseDownLoadXml(std::string &retXml,long long &downloadid,std::string &filename )
{
    XercesDOMParser *ptrParser = new  XercesDOMParser;
    ptrParser->setValidationScheme(  XercesDOMParser::Val_Never );
    ptrParser->setDoNamespaces( true );
    ptrParser->setDoSchema( false );
    ptrParser->setLoadExternalDTD( false );
    InputSource* ptrInputsource = new  MemBufInputSource((XMLByte*)retXml.c_str(), retXml.size(), "bufId");

    try
    {
        ptrParser->parse(*ptrInputsource);
        DOMDocument* ptrDoc = ptrParser->getDocument();

        // 读取downloadID节点
        DOMNodeList *ptrNodeList = ptrDoc->getElementsByTagName(C2X("id"));
        if(ptrNodeList == NULL)
        {
//            LOGFAT(ERROR_PARSE_MONITORSTATE_XML,
//                   "ParseDownLoadXml:没有找到downloadID节点");
            return false;
        }
        else
        {
            DOMNode* ptrNode = ptrNodeList->item(0);
            char * pid = XMLString::transcode(ptrNode->getFirstChild()->getNodeValue());
            std::string str_state =  pid;
            if(!str_state.empty())
            {
                downloadid = atoll(str_state.c_str()) ;
            }
            XMLString::release(&pid);

            // 添加下载任务失败
            if(downloadid == 0)
            {
                return false;
            }
        }

        // 读取downloadID节点
        DOMNodeList *ptrPathNodeList = ptrDoc->getElementsByTagName(C2X("fileNameMp4"));
        if(ptrNodeList == NULL)
        {
//            LOGFAT(ERROR_PARSE_MONITORSTATE_XML,
//                   "ParseDownLoadXml:没有找到downloadID节点");
            return false;
        }
        else
        {
            DOMNode* ptrNode = ptrPathNodeList->item(0);
            char * path = XMLString::transcode(ptrNode->getFirstChild()->getNodeValue());
            filename =  path;
            XMLString::release(&path);
        }
    }
    catch(  XMLException& e )
    {
        char* message =  XMLString::transcode( e.getMessage() );
        XMLString::release( &message );
//        LOGFAT(ERROR_PARSE_MONITORSTATE_XML,message);
        delete ptrParser;
        ptrInputsource = NULL;
        delete ptrInputsource;
        ptrParser = NULL;
    }


    delete ptrParser;
    delete ptrInputsource;
    ptrInputsource = NULL;
    ptrParser = NULL;
    return true;
}

/*******************************************************************************
* 函数名称：	QueryISDownDone_DB
* 功能描述：	查询数据库视频文件是否下载完成
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
bool CDownLoadMgr::QueryISDownDone_DB(long long &downloadid)
{

    // 读取NVRControl数据库的downLoad表
    char sql[256]={'\0'};
    snprintf(sql,sizeof(sql),
             "select HallNo,IPCposotion,startTime,ipcIP from downLoad where id=%lld",downloadid);

    int nResult;
    CppMySQLQuery query = m_DownLoad_DB.querySQL(sql,nResult);
    int nRows = 0 ;
    if((nRows = query.numRow()) == 0)
    {
            return false;
    }

    query.seekRow(0);
    std::string tmpstr = query.getStringField("ipcIP");
    if(tmpstr=="end")
    {
            return true;
    }
    else
    {
            return false;
    }
}

/*******************************************************************************
* 函数名称：	QueryIsExsitDownTask_DB
* 功能描述：	调用webservice接口
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
bool CDownLoadMgr::UpdateDownFileStatus_DB(std::string taskid,int status)
{

     char sql[1024]={'\0'};
    snprintf(sql,1024,"update  readymark set status=%d where taskid=\"%s\"",
                                         status,taskid.c_str());
    int nResult = m_CompleteFile_DB.execSQL(sql);
    if(nResult != -1)
    {
            loginfo("CInvoke:update DownLoadComplete:status database OK<%s>",sql);
            return true;
    }
    else
    {
            logerror("CInvoke:update DownLoadComplete:status database FAILED<%s>",sql);
            return false;
    }
}

/*******************************************************************************
* 函数名称：	InsertDownTask_DB
* 功能描述：	插入完成表，此表保存下载完成后检测完成或找到龙标并比对完成的文件记录
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
bool CDownLoadMgr::InsertDownTask_DB(std::string taskid,int hallid,int cpos,std::string &start,std::string &end,
                                    std::string filename,bool bResultVedio)
{

    char sql[1024]={'\0'};
    snprintf(sql,1024,"insert into "
             "readymark(taskid,hallid,cpos,start,end,filefullpath,status,isresult) "
             "values(\"%s\",%d,%d,\"%s\",\"%s\",\"%s\",%d,%d)",
             taskid.c_str(),hallid,cpos,start.c_str(),end.c_str(),filename.c_str(),ACTIVITY,bResultVedio?1:0);//状态初始值为1

    int nResult = m_CompleteFile_DB.execSQL(sql);
    if(nResult != -1)
    {
//            LOGINFFMT(0,"CInvoke:update system_config:db_synch database OK<%s>",sql);
            return true;
    }
    else
    {
//            LOGINFFMT(0,"CInvoke:update system_config:db_synch database FAILED<%s>",sql);
            return false;
    }

}



/*******************************************************************************
* 函数名称：	QueryDownTaskInfo_DB
* 功能描述：	调用webservice接口
* 输入参数：
* 输出参数：
* 返 回 值：	true - 成功，false - 失败
* 其它说明：
* 修改日期		修改人	      修改内容
* ------------------------------------------------------------------------------
* 2017-04-29 	卢岩	      创建
*******************************************************************************/
bool CDownLoadMgr::QueryCompletFileInfo_DB(std::string taskid,int &status,std::string &filepath)
{

    // 读取hallinfo表,初始化sms信息
    int nResult;
    char sql[1024]={'\0'};
    snprintf(sql,1024,"select * from readymark where taskid=\"%s\"",taskid.c_str());
    CppMySQLQuery query = m_CompleteFile_DB.querySQL(sql,nResult);
    int nRows = 0 ;
    if((nRows = query.numRow()) == 0)
    {
//            LOGERRFMT(ERROR_READSMSTABLE_NOROW,"C_HallList Initial failed,hallinfo talbe no rows!\n");
            return false;
    }


    query.seekRow(0);
    status = atoi(query.getStringField("status"));
    filepath = query.getStringField("filefullpath");
    return true;

}
