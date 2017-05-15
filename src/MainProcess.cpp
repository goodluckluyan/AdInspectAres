#include <pthread.h>
#include "threadManage/C_ThreadData.h"
#include "threadManage/C_ThreadManage.h"
#include "timeTask/C_TaskList.h"
#include "MainProcess.h"
#include "para/C_Para.h"
#include "utility/FileEx.h"
#include "log/MyLogger.h"
#include "CompareEngine.h"
#include "utility/C_LocalTime.h"

// 程序退出标识
extern bool g_bAresQuit;
extern MyLogger g_main_logwrite;
int g_iscanconnected_database = 1;
char buff_temp[512] = {'\0'};
#define loginfo(strlog,...)    g_main_logwrite.PrintLog(MyLogger::INFO,strlog,##__VA_ARGS__)
#define logerror(strlog,...)   g_main_logwrite.PrintLog(MyLogger::ERROR,strlog,##__VA_ARGS__)
#define logdebug(strlog,...)   g_main_logwrite.PrintLog(MyLogger::DEBUG,strlog,##__VA_ARGS__)
#define logfatal(strlog,...)   g_main_logwrite.PrintLog(MyLogger::FATAL,strlog,##__VA_ARGS__)

/*******************************************************************************
    * 函数名称：	CMainProcess
    * 功能描述：	构造函数
    * 输入参数：
    * 输出参数：
    * 返 回 值：
    * 其它说明：
    * 修改日期		修改人	      修改内容
    * ------------------------------------------------------------------------------
    * 2017-04-29 	卢岩	      创建
    *******************************************************************************/
CMainProcess::CMainProcess(void)
{
    m_bInit = false;

    pthread_cond_init(&m_condCreateTemple,NULL);
}

/*******************************************************************************
    * 函数名称：	~CMainProcess
    * 功能描述：	析构函数
    * 输入参数：
    * 输出参数：
    * 返 回 值：
    * 其它说明：
    * 修改日期		修改人	      修改内容
    * ------------------------------------------------------------------------------
    * 2017-04-29 	卢岩	      创建
    *******************************************************************************/
CMainProcess::~CMainProcess(void)
{
    DeInit();
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
    * 2017-04-19 	卢岩	      创建
    *******************************************************************************/
bool CMainProcess::Init()
{
//    Log("运行：CMainProcess::Init()");
    if (m_bInit)
    {
//        Log("警告：CMainProcess::Init重复的初始化！");
        return false;
    }


    C_Para *para = C_Para::GetInstance();
    para->ReadPara();


    //初试化线程
    C_ThreadManage *pThreadManage = C_ThreadManage::GetInstance();
    int iResult = pThreadManage->InitThreadData();
    if(iResult != 0)
    {
        return -1;
    }

    iResult = pThreadManage->InitWebserviceThread();
    if(iResult != 0)
    {
        return -1;
    }

    // 初始化任务队列
    m_TaskMgr.InitTaskList(this);

    // 初始化模板生成模块
    m_TempletMgr.InitDatabase(para->m_DB_IP.c_str(),para->m_DB_User.c_str(),
                              para->m_DB_Passwd.c_str(),"AdInspect",para->m_DB_Port);

    // 初始化xercesc
    try
    {
        xercesc::XMLPlatformUtils::Initialize();
    }
    catch( xercesc::XMLException& e )
    {
        char* message = xercesc::XMLString::transcode( e.getMessage() );
        xercesc::XMLString::release( &message );
    }

    // 创建下载管理模块
    DBLoginInfo db_login;
    db_login.ip = para->m_DB_IP;
    db_login.username = para->m_DB_User;
    db_login.passwd = para->m_DB_Passwd;
    db_login.port = para->m_DB_Port;

    m_DownloadMgr.SetDownloadCompete_BCFun(static_cast<void*>(this),BC_VideoDownLoadComplete);
    m_DownloadMgr.Init(3,1,db_login,"127.0.0.1",12343,para->m_VideoFile_SplitSec);
    struct tm cur;
    time_t tmpTime;
    time(&tmpTime);
    C_LocalTime::GetInstance()->LocalTime(&tmpTime,cur);
    m_DownloadMgr.AddDayTask(cur);

    // 初始化当前龙标检测任务表
    int allhall[1] = {3};
    int hallcnt = sizeof(allhall)/sizeof(int);
    for(int i=0;i<hallcnt;i++)
    {
        ptrVideoFile nullptr;
        m_mapCurTaskFile.insert(std::pair<int,ptrVideoFile>(allhall[i],nullptr));
    }

    Rect rt;
    rt.left = para->m_CompareRect.left;
    rt.top = para->m_CompareRect.top;
    rt.right = para->m_CompareRect.right;
    rt.bottom = para->m_CompareRect.bottom;
    CFileEx::CreateFolder((char *)para->m_Mark_path.c_str());
    m_MarkTask.Initialize(para->m_Max_frame_count,(char *)para->m_Mark_path.c_str(),rt);

    // 初始化比对模块
    for(int i=0;i<hallcnt;i++)
    {
        int hallno = allhall[i];
        ptrCompareEngine ptr = std::tr1::shared_ptr<CompareEngine>(new CompareEngine(hallno,&m_TempletMgr));
        CompareEngine::SetBCFun(this,BC_CompareComplete);
        ptr->Init(para->m_DB_IP,para->m_DB_User,para->m_DB_Passwd,
                  "oristarmr",para->m_DB_Port,para->m_CompareRect);
        m_mapCompareEnginePtr.insert(std::pair<int,ptrCompareEngine>(hallno,ptr));


    }


    // AddInitTask
    C_Time curtm;
    curtm.setCurTime();
    m_TaskMgr.InitTaskList(this);
    m_TaskMgr.AddTask(TASK_NUMBER_GET_MODULE_FEATRUE,NULL,0,ALWAYS_TASK);
    m_TaskMgr.AddTask(TASK_NUMBER_MARK_TASKMGR,NULL,0,ALWAYS_TASK);
    m_TaskMgr.AddTask(TASK_NUMBER_VIDEOCOMPARE_MGR,NULL,0,ALWAYS_TASK);
    m_TaskMgr.AddTask(TASK_NUMBER_DOWNLOADVIDEO,NULL,0,ALWAYS_TASK);

    // 定时任务
    m_TaskMgr.AddTask(TASK_NUMBER_TASKDISPATCH,NULL,curtm.getTimeInt()+3,TIME_TASK);
    m_TaskMgr.AddTask(TASK_NUMBER_ADDDOWNLOAD,NULL,curtm.getTimeInt()+para->m_VideoFile_SplitSec,TIME_TASK);



    m_bInit = true;
    return true;
}

/*******************************************************************************
    * 函数名称：	GetCheckDelay
    * 功能描述：	获取定时时间
    * 输入参数：
    * 输出参数：
    * 返 回 值：	true - 成功，false - 失败
    * 其它说明：
    * 修改日期		修改人	      修改内容
    * ------------------------------------------------------------------------------
    * 2017-04-29 	卢岩	      创建
    *******************************************************************************/
int CMainProcess::GetCheckDelay(int iCommandNumber)
{
    int ret = 0;
    C_Para* para = C_Para::GetInstance();
    switch (iCommandNumber)
    {
    case TASK_NUMBER_TASKDISPATCH:
        ret = 3;
        break;
     case TASK_NUMBER_ADDDOWNLOAD:
        ret = para->m_VideoFile_SplitSec;
        break;
    default:
        break;
    }
    return ret;
}

/*******************************************************************************
    * 函数名称：	DeInit
    * 功能描述：	反初始化
    * 输入参数：
    * 输出参数：
    * 返 回 值：	true - 成功，false - 失败
    * 其它说明：
    * 修改日期		修改人	      修改内容
    * ------------------------------------------------------------------------------
    * 2017-04-29 	卢岩	      创建
    *******************************************************************************/
bool CMainProcess::DeInit()
{
    loginfo("运行：CMainProcess::DeInit()");
    if (!m_bInit)
    {
        loginfo("警告：CMainProcess::DeInit重复的反初始化！");
        return false;
    }

    C_ThreadManage::DestoryInstance();
    C_Para::DestoryInstance();

    m_bInit = false;
    return true;
}

/*******************************************************************************
    * 函数名称：	Run
    * 功能描述：	任务轮询
    * 输入参数：
    * 输出参数：
    * 返 回 值：	true - 成功，false - 失败
    * 其它说明：
    * 修改日期		修改人	      修改内容
    * ------------------------------------------------------------------------------
    * 2017-04-29 	卢岩	      创建
    *******************************************************************************/
bool CMainProcess::Run()
{
    //轮询定时任务。
    m_TaskMgr.RunTasks();
    return true;
}

/*******************************************************************************
    * 函数名称：	Exec
    * 功能描述：	任务命令执行体
    * 输入参数：
    * 输出参数：
    * 返 回 值：	true - 成功，false - 失败
    * 其它说明：
    * 修改日期		修改人	      修改内容
    * ------------------------------------------------------------------------------
    * 2017-04-29 	卢岩	      创建
    *******************************************************************************/
int CMainProcess::Exec(int iCommandNumber,void *pPara)
{
    int nResult = 0;
    switch(iCommandNumber)
    {
    case TASK_NUMBER_GET_MODULE_FEATRUE:
        CreateTempletFeatrue();
        break;
    case TASK_NUMBER_MARK_TASKMGR:
        break;
    case TASK_NUMBER_VIDEOCOMPARE_MGR:
        break;
    case TASK_NUMBER_DOWNLOADVIDEO:
        m_DownloadMgr.ProcessDownTask();
        break;

        // 以下为定时任务
    case TASK_NUMBER_TASKDISPATCH:
        TaskDispatch();
        break;
    case TASK_NUMBER_ADDDOWNLOAD:
        m_DownloadMgr.AddDownTask();
        break;
    default:
        nResult = 2;
    }
    return nResult;
}

/*******************************************************************************
    * 函数名称：	TaskDispatch
    * 功能描述：  定时检测，对空闲的厅把视频文件加入龙标检测,把检测到龙标的厅进行比对
    * 输入参数：
    * 输出参数：
    * 返 回 值：	true - 成功，false - 失败
    * 其它说明：
    * 修改日期		修改人	      修改内容
    * ------------------------------------------------------------------------------
    * 2017-04-29 	卢岩	      创建
    *******************************************************************************/
int CMainProcess::TaskDispatch()
{

    // 查找空闲的龙标检测厅，空闲的标准为厅的当先任务文件指针为空
    // 对比完成后或没检测出龙标才能把当前任务文件指针设成空，这样形成了是龙标检测和比对的同步
    std::vector<int> vecIdleHall,vecCompareHall;
    {
        C_GuardCS guard(&m_mutxCurTaskMap);
        std::map<int,ptrVideoFile>::iterator it = m_mapCurTaskFile.begin();
        for(;it!=m_mapCurTaskFile.end();it++)
        {
            ptrVideoFile &ptr = it->second;
            if(ptr == NULL)
            {
                loginfo("Find %d hall idle,ready mark job",it->first);
                vecIdleHall.push_back(it->first);
            }
            else if(ptr->Status == READYCOMPARE)
            {
                loginfo("Find %d hall idle,ready compare job",it->first);
                vecCompareHall.push_back(it->first);
            }

        }
    }

    // 找到厅的待检测龙标文件
    std::vector<ptrMarkJobItem> vecMarkJobItem;
    C_Para *para =C_Para::GetInstance();
    int len = vecIdleHall.size();
    for(int i = 0 ;i < len ;i++)
    {
        C_GuardCS guard(&m_mutxVideoFileMap);
        std::map<int,lsVideoFile>::iterator fit = m_mapVideoFile.find(vecIdleHall[i]);
        if(fit != m_mapVideoFile.end())
        {
            lsVideoFile &lsVF = fit->second;
            loginfo("%d hall vedio file %d",fit->first,(int)lsVF.size());
            lsVideoFile::iterator it = lsVF.begin();
            for(;it != lsVF.end();it++)
            {
                ptrVideoFile &ptrVF = *it;
                loginfo("%d hall vedio file %s %d",fit->first,ptrVF->FilePath.c_str(),ptrVF->Status);
                if(READY == ptrVF->Status )
                {
                     C_GuardCS guard(&m_mutxCurTaskMap);
                    ptrMarkJobItem mji(new MARK_JOB_ITEM);
                    mji->max_frame_count = para->m_Max_frame_count;
                    strcpy(mji->videopath,ptrVF->FilePath.c_str());
                    mji->decode_start_pos = ptrVF->DecodecPos;
                    mji->decode_rate = para->m_Decode_rate;
                    strcpy(mji->mark_store_path,para->m_Mark_Store_path.c_str());
                    mji->decode_width = para->m_Decode_width;
                    mji->decode_height = para->m_Decode_height;
                    mji->bisfind_mark_flag = true;
                    mji->bisadjust_aspect = true;
                    mji->bisgray = true;
                    mji->time_base = ptrVF->Start;
                    strcpy(mji->markjob_id,ptrVF->UUID.c_str());
                    vecMarkJobItem.push_back(mji);
                    m_mapCurTaskFile[fit->first] = ptrVF;
                    break;
                }
            }
        }
    }

    // 创建龙标检测任务
    len = static_cast<int>(vecMarkJobItem.size());
    if(len==0)
    {
        loginfo("Not find download file what READY task");
    }

    for(int i = 0;i < len ; i++)
    {
        ptrMarkJobItem &mji =  vecMarkJobItem[i];
        int ret = m_MarkTask.CreateMarkJob(mji.get(),BC_CheckLongbiaoComplete,static_cast<void*>(this));
        if(0 == ret)
        {

             loginfo("Create Mark Job Success:%s(p:%d r:%d w:%d h:%d ts:%d) ",
                     mji->videopath,mji->decode_start_pos,mji->decode_rate,mji->decode_width,
                     mji->decode_height,mji->time_base);
        }
        else
        {
            loginfo("Create Mark Job Fail:%s(p:%d r:%d w:%d h:%d ts:%d) ",
                    mji->videopath,mji->decode_start_pos,mji->decode_rate,mji->decode_width,
                    mji->decode_height,mji->time_base);
        }

    }

    // 对所有厅准备比对的开始比对
    len = vecCompareHall.size();
    for(int i = 0;i < len ;i++)
    {
        int hallid = vecCompareHall[i];
        std::map<int,ptrCompareEngine>::iterator fit = m_mapCompareEnginePtr.find(hallid);
        if(fit != m_mapCompareEnginePtr.end())
        {
            C_GuardCS guard(&m_mutxVideoFileMap);
            ptrCompareEngine& ptr = fit->second;
            if(ptr->Compare(m_mapCurTaskFile[hallid]->UUID,
                            static_cast<FrameBufferLoop*>(m_mapCurTaskFile[hallid]->ptrBufferLoop))==0)
            {
                m_mapCurTaskFile[hallid]->Status=COMPARE;
                loginfo("Create Compare Job Success:%s(loopbuffer cnt:%d) ",
                        m_mapCurTaskFile[hallid]->UUID.c_str(),
                        (static_cast<FrameBufferLoop*>(m_mapCurTaskFile[hallid]->ptrBufferLoop))->GetFrameCount());
            }
            else
            {
                loginfo("Create Compare Job Fail:%s(loopbuffer address:%x) ",
                        m_mapCurTaskFile[hallid]->UUID.c_str(),m_mapCurTaskFile[hallid]->ptrBufferLoop);
            }
        }
    }

    // 对厅文件列表进行维护
    {
        C_GuardCS guard(&m_mutxVideoFileMap);
        std::map<int,lsVideoFile>::iterator it = m_mapVideoFile.begin();
        for(;it != m_mapVideoFile.end();it++)
        {
            lsVideoFile &lsVF = it->second;
            int cnt = 0;

            //标记dirty
            lsVideoFile::reverse_iterator rit = lsVF.rbegin();
            for(;rit != lsVF.rend();rit++)
            {
                // 不对最后一个完成的文件进行处理，理由是如果宕机重启后需要重新解码因为新的文件
                // 如果有龙标最后一个文件可能有广告
                ptrVideoFile &ptrVF = *rit;
                if(COMPLETE == ptrVF->Status)
                {
                    cnt++;
                    if(cnt > 1)
                    {
                        //  设置文件为非活跃状态，即已经不需要再进行龙标检测和对比了
                        m_DownloadMgr.UpdateDownFileStatus_DB(ptrVF->UUID,INACTIVITY);
                        ptrVF->Status = DIRTY;
                    }
                }
            }

             // 删除标记
             lsVideoFile::iterator dit = lsVF.begin();
             for(;dit != lsVF.end();dit++)
             {
                 ptrVideoFile &ptrVF = *dit;
                 if(DIRTY == ptrVF->Status)
                 {
                     lsVF.erase(dit++);
                 }
             }
        }
    }



    return 0;
}

/*******************************************************************************
    * 函数名称：	Print
    * 功能描述：	打印工作状态
    * 输入参数：	enModuleType		-- 模块类型
    *           strParam			-- 打印附加参数
    * 输出参数：
    * 返 回 值：
    * 其它说明：
    * 修改日期		修改人	      修改内容
    * ------------------------------------------------------------------------------
    * 2017-04-29 	卢岩	      创建
    *******************************************************************************/
void CMainProcess::Print(int enType)
{
    switch(enType)
    {
    case 1:
        loginfo("cur task running");
        break;
    default:
        break;
    }
}


/*******************************************************************************
    * 函数名称：	WS_AddInspectModule
    * 功能描述：	webserivce 接口执行函数，添加广告模板
    * 输入参数：
    * 输出参数：
    * 返 回 值：
    * 其它说明：
    * 修改日期		修改人	      修改内容
    * ------------------------------------------------------------------------------
    *  2017-04-29 	 卢岩	      创建
    *******************************************************************************/
int CMainProcess::WS_AddInspectModule(std::string &id,string &OrderNO,string &AdName,int CinemaNum,std::string& start,
                                      std::string & end,int ShowOder,int Type,std::string &ModulePath )
{


    std::string basepath = C_Para::GetInstance()->m_Templet_Video_path;
    if(basepath.rfind("/")!=basepath.size()-1)
    {
        basepath+="/";
    }
    std::string mp4path =basepath + ModulePath;

    loginfo("ws invoke(id:%s OriderNo:%s AdName:%s CinemaNum:%d)\n",id.c_str(),OrderNO.c_str(),AdName.c_str(),CinemaNum);
    loginfo("ws invoke(start:%s end:%s showorder:%d type:%d modulepath:%s)\n",start.c_str(),end.c_str(),ShowOder,Type,mp4path.c_str());
    m_mutxCreateTemple.EnterCS();
    ptrTempletInfo ptr(new TempletInfos(id,OrderNO,AdName,CinemaNum,start,end,ShowOder,Type,mp4path));
    m_lstCreateTempleTask.push_back(ptr);
    pthread_cond_signal(&m_condCreateTemple);
    m_mutxCreateTemple.LeaveCS();
    return 0;
}


/*******************************************************************************
    * 函数名称：	BC_VideoDownLoadComplete
    * 功能描述：	视频文件下载完成回调
    * 输入参数：
    * 输出参数：
    * 返 回 值：
    * 其它说明：
    * 修改日期		修改人	      修改内容
    * ------------------------------------------------------------------------------
    *  2017-04-29 	 卢岩	      创建
    *******************************************************************************/
int CMainProcess::BC_VideoDownLoadComplete(void * ptr,int HallID,int CameraPos,time_t Start,
                                           int Duration,std::string FilePath,bool bResultVideo)
{
    char buff[32]={'\0'};
    strftime(buff,32,"%Y-%m-%d %H:%M:%S",localtime(&Start));
    if(!bResultVideo)
    {
        // 录播待比对视频
        CMainProcess * pthis = (CMainProcess*)ptr;
        C_GuardCS guard(&pthis->m_mutxVideoFileMap);
        pthis->m_mapVideoFile[HallID].push_back(ptrVideoFile(new VideoFile(HallID,CameraPos,Start,Duration,FilePath)));
        loginfo("vedio file download complete( %d hall %d camera file:%s start:%s,duration:%d) ",
                HallID,CameraPos,FilePath.c_str(),buff,Duration);

    }
    else
    {
        // 比对结果视频
        loginfo("ResultVedio(%d-%d-%s-%d %s) DownLoad Complete!\n",
               HallID,CameraPos,buff,Duration,FilePath.c_str());
    }

    return 0;
}


/*******************************************************************************
    * 函数名称：	BC_CheckLongbiaoComplete
    * 功能描述：	检测龙标完成回调
    * 输入参数：
    * 输出参数：
    * 返 回 值：
    * 其它说明：
    * 修改日期		修改人	      修改内容
    * ------------------------------------------------------------------------------
    *  2017-04-29 	 卢岩	      创建
    *******************************************************************************/
int  CMainProcess::BC_CheckLongbiaoComplete(void *userdata,void *pmarkjobresult)
{
    CMainProcess * pthis = (CMainProcess*)userdata;
    MARK_JOB_RESULT * ptrResult = static_cast<MARK_JOB_RESULT*>(pmarkjobresult);

    std::string taskid = ptrResult->markjob_id;
    std::string hallid = taskid.substr(0,taskid.find("-"));
    int hallnum = atoi(hallid.c_str());
    bool bFindLongbiao = false;

    ptrVideoFile tmpCurVideoFile;
    {
        C_GuardCS guard(&pthis->m_mutxCurTaskMap);
        std::map<int,ptrVideoFile>::iterator fit = pthis->m_mapCurTaskFile.find(hallnum);
        if(fit != pthis->m_mapCurTaskFile.end())
        {
            tmpCurVideoFile = fit->second;
            if(tmpCurVideoFile->UUID != taskid)
            {
                return -1;
            }

            // 对没有龙标的视频文件设置为完成
            if(!ptrResult->bisfind)
            {
                C_GuardCS guard(&pthis->m_mutxVideoFileMap);
                tmpCurVideoFile->Status = COMPLETE;
                tmpCurVideoFile->DecodecPos = tmpCurVideoFile->Duration;
                fit->second.reset();
                loginfo("Not find Longbiao ,all task complete(uuid:%s path:%s) ",
                        tmpCurVideoFile->UUID.c_str(),tmpCurVideoFile->FilePath.c_str());
            }
            else
            {
                //  对有龙标的视频文件设置为准备比对
                C_GuardCS guard(&pthis->m_mutxVideoFileMap);
                tmpCurVideoFile->ptrBufferLoop = ptrResult->pframeloop;
                tmpCurVideoFile->Status = READYCOMPARE;
                tmpCurVideoFile->DecodecPos = ptrResult->find_pos+1;
                loginfo("Find Longbiao ,Ready compare task (uuid:%s %d) ",
                        tmpCurVideoFile->UUID.c_str(),ptrResult->find_pos );
            }
        }
    }
}


/*******************************************************************************
    * 函数名称：	BC_CompareComplete
    * 功能描述：	比对完成回调
    * 输入参数：
    * 输出参数：
    * 返 回 值：
    * 其它说明：
    * 修改日期		修改人	      修改内容
    * ------------------------------------------------------------------------------
    *  2017-04-29 	 卢岩	      创建
    *******************************************************************************/
int CMainProcess::BC_CompareComplete(void *ptr,
                                     std::string &taskid,std::map<std::string,TempletMatch> &templetResult)
{
    CMainProcess *pthis = (CMainProcess*)ptr;
    std::map<std::string,TempletMatch>::iterator it = templetResult.begin();
    for(;it != templetResult.end();it++)
    {
        TempletMatch &tm = it->second;
        if(tm.vecMatch.size()!=0 && tm.inspect_ts_start != 0 )
        {
            DownLoadInfoItem dlii ;
            dlii.start = tm.inspect_ts_start;
            dlii.duration = tm.inspect_ts_end - tm.inspect_ts_start + 1;
            dlii.savepath = tm.resultpath;
            dlii.filename = tm.vediofilename;
            pthis->m_DownloadMgr.AddResultSampleDownTask(dlii);
            loginfo("Compare complete,Downlaod result vedio file(start:%d duration:%d path:%s) ",
                    dlii.start,dlii.duration,std::string(dlii.savepath+"/"+ dlii.filename).c_str());
        }


    }
    pthis->CompareComplete(taskid);
    return 0;
}


int CMainProcess::CompareComplete(std::string &taskid)
{
    std::string hallid = taskid.substr(0,taskid.find("_"));
    int hallnum = atoi(hallid.c_str());


    ptrVideoFile tmpCurVideoFile;
    {
        C_GuardCS gard(&m_mutxCurTaskMap);
        std::map<int,ptrVideoFile>::iterator fit = m_mapCurTaskFile.find(hallnum);
        if(fit != m_mapCurTaskFile.end())
        {
            tmpCurVideoFile = fit->second;
            if(tmpCurVideoFile != NULL && tmpCurVideoFile->UUID != taskid)
            {
                return -1;
            }

            // 设置为完成，完成标识所有操作已经完成，清空当前指针
            if(tmpCurVideoFile != NULL && abs(tmpCurVideoFile->DecodecPos - tmpCurVideoFile->Duration)<=10)
            {
                C_GuardCS gardv(&m_mutxVideoFileMap);
                tmpCurVideoFile->Status = COMPLETE;
                loginfo("Compare complete ,all task complete(uuid:%s path:%s) ",
                        tmpCurVideoFile->UUID.c_str(),tmpCurVideoFile->FilePath.c_str());
                fit->second.reset();

            }
            else if(tmpCurVideoFile != NULL && abs(tmpCurVideoFile->DecodecPos - tmpCurVideoFile->Duration)>10)
            {
                C_GuardCS gardv(&m_mutxVideoFileMap);

                // 对没有检测完成的文件设置为继续检测龙标，清空当前指针
                tmpCurVideoFile->DecodecPos+=10;
                if(tmpCurVideoFile->DecodecPos > tmpCurVideoFile->Duration)
                {
                    tmpCurVideoFile->DecodecPos = tmpCurVideoFile->Duration;
                    tmpCurVideoFile->Status = COMPLETE;
                    loginfo("Compare complete ,all task complete(uuid:%s path:%s) ",
                            tmpCurVideoFile->UUID.c_str(),tmpCurVideoFile->FilePath.c_str());
                }
                else
                {
                     tmpCurVideoFile->Status = READY;
                     loginfo("Compare complete ,all task complete but mark vediofile uncomplete,"
                             "so this file will continue mark! (uuid:%s path:%s decode_pos:%d duration:%d) ",
                             tmpCurVideoFile->UUID.c_str(),tmpCurVideoFile->FilePath.c_str(),
                             tmpCurVideoFile->DecodecPos,tmpCurVideoFile->Duration);
                }


                fit->second.reset();

            }
        }


    }
    return 0;
}



/*******************************************************************************
    * 函数名称：	CreateTempletFeatrue
    * 功能描述：	创建模板
    * 输入参数：
    * 输出参数：
    * 返 回 值：
    * 其它说明：
    * 修改日期		修改人	      修改内容
    * ------------------------------------------------------------------------------
    *  2017-04-29 	 卢岩	      创建
    *******************************************************************************/
int CMainProcess::CreateTempletFeatrue()
{
    m_mutxCreateTemple.EnterCS();
    if(m_lstCreateTempleTask.size() == 0)
    {
        pthread_cond_wait(&m_condCreateTemple,static_cast<pthread_mutex_t*>(m_mutxCreateTemple.getmutx()));
    }
    m_mutxCreateTemple.LeaveCS();

    C_Para * para = C_Para::GetInstance();
    if(m_lstCreateTempleTask.size()!=0)
    {
        ptrTempletInfo &task = m_lstCreateTempleTask.front();

        TASK_ITEM * item = new TASK_ITEM;
        NewTableItemSpace(&item);

        // 填充任务结构
        int pos = task->ModulePath.rfind("/");
        std::string filename = task->ModulePath.substr(pos+1);
        std::string vediofilepath =task->ModulePath.substr(0,pos);

        std::string pathbase = C_Para::GetInstance()->m_Templet_Store_path;
        if(pathbase.rfind("/")!=pathbase.size()-1)
        {
            pathbase+="/";
        }
        std::string TempletPath = pathbase + task->Id;
        CFileEx::CreateFolder(TempletPath.c_str());

        TempletInfos *ptr = task.get();
        strncpy(item->uuid,task->Id.c_str(),task->Id.size());
        strncpy(item->orig_fileName,task->AdName.c_str(),task->AdName.size());
        strncpy(item->fileName,filename.c_str(),filename.size());
        strncpy(item->filePath,vediofilepath.c_str(),vediofilepath.size());

        strcpy(item->startDateTime,task->Start.c_str());
        strcpy(item->endDateTime,task->End.c_str());

        strcpy(item->hall_id,"0");
        sprintf(item->frequency,"%d",para->m_Decode_rate);
        strcpy(item->type,"sift");
        sprintf(item->dstVideoWidth,"%d",para->m_Decode_width);
        sprintf(item->dstVideoHeight,"%d",para->m_Decode_height);
        sprintf(item->ad_order,"%d",task->ShowOder);
        strcpy(item->featureFilePath,TempletPath.c_str());

        // 执行创建
        m_TempletMgr.CreateTaskTemplet(item);

        DeleteItemSpace(&item);
        delete item;


        m_mutxCreateTemple.EnterCS();
        m_lstCreateTempleTask.pop_front();
        m_mutxCreateTemple.LeaveCS();
    }
}

/*******************************************************************************
    * 函数名称：	NewTableItemSpace
    * 功能描述：
    * 输入参数：
    * 输出参数：
    * 返 回 值：
    * 其它说明：
    * 修改日期		修改人	      修改内容
    * ------------------------------------------------------------------------------
    *  2017-04-29 	 卢岩	      创建
    *******************************************************************************/
void CMainProcess::NewTableItemSpace(TASK_ITEM **task_item)
{
    if(NULL==*task_item)
    {
        return;
    }

    *task_item=(TASK_ITEM *)new TASK_ITEM;


    (*task_item)->id=new char[BUFF_SIZE_50];
    (*task_item)->uuid=new char[BUFF_SIZE_50];
    (*task_item)->orig_fileName=new char[BUFF_SIZE_255];
    (*task_item)->fileName=new char[BUFF_SIZE_255];
    (*task_item)->filePath=new char[BUFF_SIZE_255];
    (*task_item)->startDateTime=new char[BUFF_SIZE_50];
    (*task_item)->endDateTime=new char[BUFF_SIZE_50];
    (*task_item)->hall_id=new char[BUFF_SIZE_50];
    (*task_item)->frameRate=new char[BUFF_SIZE_50];
    (*task_item)->frequency=new char[BUFF_SIZE_50];
    (*task_item)->type=new char[BUFF_SIZE_50];
    (*task_item)->description=new char[BUFF_SIZE_50];
    (*task_item)->fullFilePath=new char[BUFF_SIZE_255];
    (*task_item)->videoHeight=new char[BUFF_SIZE_50];
    (*task_item)->videoWidth=new char[BUFF_SIZE_50];
    (*task_item)->dstVideoHeight=new char[BUFF_SIZE_50];
    (*task_item)->dstVideoWidth=new char[BUFF_SIZE_50];
    (*task_item)->realDuration=new char[BUFF_SIZE_50];
    (*task_item)->ad_order=new char[BUFF_SIZE_50];
    (*task_item)->featureFilePath=new char[BUFF_SIZE_255];

    ClearTableItemSpace(*task_item);
}

/*******************************************************************************
    * 函数名称：	DeleteItemSpace
    * 功能描述：
    * 输入参数：
    * 输出参数：
    * 返 回 值：
    * 其它说明：
    * 修改日期		修改人	      修改内容
    * ------------------------------------------------------------------------------
    *  2017-04-29 	 卢岩	      创建
    *******************************************************************************/
void CMainProcess::DeleteItemSpace(TASK_ITEM **task_item)
{
    if(NULL==(*task_item))
    {
        return;
    }

//    ClearTableItemSpace(*task_item);

    if(NULL!=(*task_item)->uuid)
    {
        delete [] (*task_item)->id;
        (*task_item)->id=NULL;
        delete [] (*task_item)->uuid;
        (*task_item)->uuid=NULL;
    }
    if(NULL!=(*task_item)->orig_fileName)
    {
        delete [] (*task_item)->orig_fileName;
        (*task_item)->orig_fileName=NULL;
    }
    if(NULL!=(*task_item)->fileName)
    {
        delete [] (*task_item)->fileName;
        (*task_item)->fileName=NULL;
    }
    if(NULL!=(*task_item)->filePath)
    {
        delete [] (*task_item)->filePath;
        (*task_item)->filePath=NULL;
    }
    if(NULL!=(*task_item)->startDateTime)
    {
        delete [] (*task_item)->startDateTime;
        (*task_item)->startDateTime=NULL;
    }
    if(NULL!=(*task_item)->endDateTime)
    {
        delete [] (*task_item)->endDateTime;
        (*task_item)->endDateTime=NULL;
    }
    if(NULL!=(*task_item)->hall_id)
    {
        delete [] (*task_item)->hall_id;
        (*task_item)->hall_id=NULL;
    }
    if(NULL!=(*task_item)->frameRate)
    {
        delete [] (*task_item)->frameRate;
        (*task_item)->frameRate=NULL;
    }
    if(NULL!=(*task_item)->frequency)
    {
        delete [] (*task_item)->frequency;
        (*task_item)->frequency=NULL;
    }
    if(NULL!=(*task_item)->type)
    {
        delete [] (*task_item)->type;
        (*task_item)->type=NULL;
    }
    if(NULL!=(*task_item)->description)
    {
        delete [] (*task_item)->description;
        (*task_item)->description=NULL;
    }
    if(NULL!=(*task_item)->fullFilePath)
    {
        delete [] (*task_item)->fullFilePath;
        (*task_item)->fullFilePath=NULL;
    }
    if(NULL!=(*task_item)->videoHeight)
    {
        delete [] (*task_item)->videoHeight;
        (*task_item)->videoHeight=NULL;
    }
    if(NULL!=(*task_item)->videoWidth)
    {
        delete [] (*task_item)->videoWidth;
        (*task_item)->videoWidth=NULL;
    }
    if(NULL!=(*task_item)->dstVideoHeight)
    {
        delete [] (*task_item)->dstVideoHeight;
        (*task_item)->dstVideoHeight=NULL;
    }
    if(NULL!=(*task_item)->dstVideoWidth)
    {
        delete [] (*task_item)->dstVideoWidth;
        (*task_item)->dstVideoWidth=NULL;
    }
    if(NULL!=(*task_item)->realDuration)
    {
        delete [] (*task_item)->realDuration;
        (*task_item)->realDuration=NULL;
    }

    if(NULL!=(*task_item)->ad_order)
    {
        delete [] (*task_item)->ad_order;
        (*task_item)->ad_order=NULL;
    }

    if(NULL!=(*task_item)->featureFilePath)
    {
        delete [] (*task_item)->featureFilePath;
        (*task_item)->featureFilePath=NULL;
    }

}

void CMainProcess::ClearTableItemSpace(TASK_ITEM *task_item)
{
    if(NULL==task_item)
    {
        return;
    }
    if(NULL!=task_item->uuid)
    {
        memset(task_item->id,0,BUFF_SIZE_50);
        memset(task_item->uuid,0,BUFF_SIZE_50);
    }

    if(NULL!=task_item->orig_fileName)
    {
        memset(task_item->orig_fileName,0,BUFF_SIZE_255);
    }
    if(NULL!=task_item->fileName)
    {
        memset(task_item->fileName,0,BUFF_SIZE_255);
    }
    if(NULL!=task_item->filePath)
    {
        memset(task_item->filePath,0,BUFF_SIZE_255);
    }
    if(NULL!=task_item->startDateTime)
    {
        memset(task_item->startDateTime,0,BUFF_SIZE_50);
    }
    if(NULL!=task_item->endDateTime)
    {
        memset(task_item->endDateTime,0,BUFF_SIZE_50);
    }
    if(NULL!=task_item->hall_id)
    {
        memset(task_item->hall_id,0,BUFF_SIZE_50);
    }
    if(NULL!=task_item->frameRate)
    {
        memset(task_item->frameRate,0,BUFF_SIZE_50);
    }
    if(NULL!=task_item->frequency)
    {
        memset(task_item->frequency,0,BUFF_SIZE_50);
    }
    if(NULL!=task_item->type)
    {
        memset(task_item->type,0,BUFF_SIZE_50);
    }
    if(NULL!=task_item->description)
    {
        memset(task_item->description,0,BUFF_SIZE_50);
    }
    if(NULL!=task_item->fullFilePath)
    {
        memset(task_item->fullFilePath,0,BUFF_SIZE_255);
    }
    if(NULL!=task_item->videoHeight)
    {
        memset(task_item->videoHeight,0,BUFF_SIZE_50);
    }
    if(NULL!=task_item->videoWidth)
    {
        memset(task_item->videoWidth,0,BUFF_SIZE_50);
    }
    if(NULL!=task_item->dstVideoHeight)
    {
        memset(task_item->dstVideoHeight,0,BUFF_SIZE_50);
    }
    if(NULL!=task_item->dstVideoWidth)
    {
        memset(task_item->dstVideoWidth,0,BUFF_SIZE_50);
    }
    if(NULL!=task_item->realDuration)
    {
        memset(task_item->realDuration,0,BUFF_SIZE_50);
    }
    if(NULL!=task_item->ad_order)
    {
        memset(task_item->ad_order,0,BUFF_SIZE_50);
    }
    if(NULL!=task_item->featureFilePath)
    {
        memset(task_item->featureFilePath,0,BUFF_SIZE_255);
    }
}


