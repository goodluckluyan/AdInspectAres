#ifndef MAINPROCESS_H
#define MAINPROCESS_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include <tr1/memory>
#include <stdio.h>
#include "utility/osapi/osapi.h"
#include "timeTask/C_TaskList.h"
#include "MarkJob/MarkJob.h"
#include "utility/C_Time.h"
#include "CompareEngine.h"
#include "downloadmgr.h"
#include "parser_xml.h"

extern MyLogger g_main_logwrite;

#define loginfo(strlog,...)    g_main_logwrite.PrintLog(MyLogger::INFO,strlog,##__VA_ARGS__)
#define logerror(strlog,...)   g_main_logwrite.PrintLog(MyLogger::ERROR,strlog,##__VA_ARGS__)
#define logdebug(strlog,...)   g_main_logwrite.PrintLog(MyLogger::DEBUG,strlog,##__VA_ARGS__)
#define logfatal(strlog,...)   g_main_logwrite.PrintLog(MyLogger::FATAL,strlog,##__VA_ARGS__)

enum VideoFileStatus
{
    READY,
    CHECKING,
    CHECKDONE,
    READYCOMPARE,
    COMPARE,
    COMPLETE,
    DIRTY
};


struct VideoFile
{
    VideoFile(int hallid,int camerapos,time_t start,int duration,std::string filepath)
    {
        HallID = hallid;
        CameraPos = camerapos;
        Start = start;
        Duration = duration;
        FilePath = filepath;
        Status = READY;
        DecodecPos = 0;
        char buf[64]={'\0'};
        C_Time ctm;
        ctm.setTimeInt(start);
        std::string strctm;
        sprintf(buf,"%d-%d-%d%d%d%d%d-%d",hallid,camerapos,ctm.getYear(),ctm.getMonth(),
                ctm.getDay(),ctm.getHour(),ctm.getMinute(),duration);
        UUID = buf;
        ptrBufferLoop = NULL;

    }

    std::string UUID;         // 唯一标识
    int HallID;               // 厅号
    int CameraPos;            // 前置或后置
    time_t Start;             // 开始时间
    int Duration;             // 持续时间
    std::string FilePath;     // 文件路径
    int DecodecPos;           // 解码开始位置
    VideoFileStatus Status;
    void *ptrBufferLoop;

};

struct TempletInfos
{


    TempletInfos( std::string &id, string &order_no,string &ad_name, int cinema_num,
                   std::string &start,std::string &end,int show_order,int type,int show_type,std::string &module_path)
    {
        Id = id;
        OrderNO = order_no;
        AdName = ad_name;
        CinemaNum = cinema_num;
        Start = start;
        End=end;
        ShowOder = show_order;
        Type = type;
        ModulePath = module_path;
        ShowType = show_type;
    }

    ~TempletInfos()
    {
        logdebug("delete TempletInfo");
    }
    std::string Id;
    string OrderNO;
    string AdName;
    int CinemaNum;
    std::string Start;
    std::string End;
    int ShowOder;
    int Type;
    int ShowType;
    std::string ModulePath;
};

typedef std::tr1::shared_ptr<TempletInfos> ptrTempletInfo;
typedef std::tr1::shared_ptr<VideoFile> ptrVideoFile;
typedef std::list<ptrVideoFile> lsVideoFile;
typedef std::tr1::shared_ptr<MARK_JOB_ITEM> ptrMarkJobItem;
typedef std::tr1::shared_ptr<CompareEngine> ptrCompareEngine;

class CMainProcess
{
public:

    // 默认构造函数
    CMainProcess(void);

    // 析构函数
    ~CMainProcess(void);

    // 初始化
    bool Init();

    // 反初始化
    bool DeInit();

    // 打印工作状态
    void Print(int );

    // 添加模板
    int WS_AddInspectModule(std::string &id,string &OrderNO,string &AdName,int CinemaNum,std::string& start,
                            std::string & end,int ShowOder,int Type,int ShowType,std::string &ModulePath );

    // 删除模板
    int WS_DelInspectModule(std::string &uuid);

    // 视频文件下载完成回调
    static int BC_VideoDownLoadComplete(void *ptr,int HallID,int CameraPos,
                                        time_t Start,int Duration,std::string FilePath,
                                        bool bResultVedio,std::string uuid);

    // 检测龙标完成回调
    static int BC_CheckLongbiaoComplete(void *userdata,void *pmarkjobresult);

    // 比较完成回调
    static int BC_CompareComplete(void *ptr,std::string &taskid,
                                  std::map<std::string,TempletMatch> &templetResult,
                                  std::vector<suspicious_show> &vecss);

    int CompareComplete(std::string &taskid);

    // 任务命令执行体，程序主框架核心函数
    int Exec(int iCommandNumber,void *pPara);

    // 任务轮询
    bool Run();

    // 是否初始化完成
    bool IsInit()
    {
        return m_bInit;
    }

    // 生成模板
    int CreateTempletFeatrue();

    int GetCheckDelay(int icommander);
private:
    //  对已下载的视频文件适时加入龙标检测，并对完成的项目进行删除
    int TaskDispatch();

    // 创建模板时开辟内存空间，释放时使用DeleteItemSpace
    void NewTableItemSpace(TASK_ITEM **task_item);

    // 释放模板创建时开辟的内存
    void DeleteItemSpace(TASK_ITEM **task_item);

    // 清零NewTableItemSpace开辟的空间
    void ClearTableItemSpace(TASK_ITEM *task_item);

    // 龙标出现时间插到龙标表
    bool InsertLongbiao_DB(std::string taskid,time_t timets,int hallid);

    // 获取影院信息
    bool GetCinemaInfo_DB();

    // 更新结果视频文件的数据库状态
    bool UpdateResultDownloadStat_DB(std::string uuid,std::string strStart);
private:
    // 下载管理对象
    CDownLoadMgr m_DownloadMgr;

    // 检测龙标对象
    MarkJob m_MarkTask;

    // 模板生成对象
    TempletManager m_TempletMgr;

    // 比对模块,按多厅设计
    std::map<int,ptrCompareEngine> m_mapCompareEnginePtr;


private:
    // 下载完成视频文件列表,按多厅设计
    std::map<int,lsVideoFile> m_mapVideoFile;

    // 龙标检测当前任务hash表,按多厅设计
    std::map<int,ptrVideoFile> m_mapCurTaskFile;



private:
    // 任务管理对象
    C_TaskList m_TaskMgr;

    // 下载完成视频文件列表互斥体
    C_CS m_mutxVideoFileMap;

    // 龙标检测当前任务hash表互斥体
    C_CS m_mutxCurTaskMap;


    // 删除模板互斥体
    C_CS m_mutxDelTemplet;


    // 初始化标记
    bool m_bInit;

    // 创建模块生产者消费者同步控制
    pthread_cond_t m_condCreateTemple;
    C_CS m_mutxCreateTemple;
    std::list<ptrTempletInfo> m_lstCreateTempleTask;

    // 影院信息
    std::string m_strCity;
    std::string m_strCinemaName;

};



#endif

