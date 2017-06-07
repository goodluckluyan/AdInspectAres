#ifndef DOWNLOADMGR_H
#define DOWNLOADMGR_H
#include <list>
#include <pthread.h>
//#include "parser_xml.h"
#include "database/CppMySQL3DB.h"

typedef struct _DownLoadInfoItem
{
   time_t start;
   int duration;
   std::string savepath;
   std::string filename;
   std::string uuid;
}DownLoadInfoItem;

typedef struct _DBLoginInfo
{
    std::string ip;
    int port;
    std::string username;
    std::string passwd;
    std::string dbname;
}DBLoginInfo;

enum FILESTATUS{ACTIVITY,INACTIVITY};
typedef int (*CompleteDLT_CallBack)(void *userdate,int HallID,int CameraPos,time_t Start,
                                    int Duration,std::string FilePath,bool bResultVideo,
                                    std::string uuid);

class CDownLoadMgr
{
public:

    CDownLoadMgr();
    ~CDownLoadMgr();

    // 设置回调函数
    static void SetDownloadCompete_BCFun(void * userdata,CompleteDLT_CallBack pfun);

    // 初始化
    int Init(int HallID, int CameraPos, DBLoginInfo &DBinfo, std::string wsip, int wsport,int TimeStep =900);

    // 添加一天的下载任务
    bool AddDayTask(struct tm &day);

    // 添加一个当前时间-steptime为开始时间的下载任务
    bool AddDownTask();

    // 添加最小时间段的任务，比如说十五分钟
    bool AddDownTask(DownLoadInfoItem &DLItem);

    // 添加监播结果视频下载任务
    bool AddResultSampleDownTask(DownLoadInfoItem &taskitem);


    // 下载视频文件，同步方式下载，任务下发给nvr后轮询检测数据库
    // 下载完成后计入下载任务表：下载任务号，路径，是否下载完成，是否检测完龙标
    void ProcessDownTask();

    //更新完成文件状态
    bool UpdateDownFileStatus_DB(std::string taskid,int status);
private:

    // tcp 发送接收
    int SendAndRecvResponse(const std::string &request, std::string &response, int delayTime=3);

     // 获取http的内容
    int GetHttpContent(const std::string &http, std::string &response);

    // 填充http头
    int InvokerWebServer(std::string &xml,std::string &strResponse);

    // 解析xml的返回
    bool ParseDownLoadXml(std::string &xml,long long &downloadid ,std::string &filename);

    // 调用webservice 下载视频文件
    bool DownVidoFile(DownLoadInfoItem &task,long long &downloadid,std::string &filename);

    // 查询是否下载完成    
    bool QueryISDownDone_DB(long long &downloadid);

    // 插入完成表
    bool InsertDownTask_DB(std::string taskid,int hallid,int cpos,std::string &start,std::string &end,
                                        std::string filename,bool bResultVedio=false);



    //  查询完成文件状态
    bool QueryCompletFileInfo_DB(std::string taskid,int &status,std::string &filepath);


private:
    pthread_cond_t m_cond;
    pthread_mutex_t m_mutx;

    int m_HallID;
    int m_CameraPos;
    std::list<DownLoadInfoItem> m_lstDownloadTask;
    std::list<DownLoadInfoItem> m_lstSampleDownloadTask;
    static CompleteDLT_CallBack m_pDownloadCompeteFun;
    static void * m_Userdata;

    std::string m_NVRIP;
    int m_NVRPort;

    // 每天的开始放映的时间段如：9:00 - 23:59:59，以秒开始结束
    int m_startsec;
    int m_endsec;

    // 每次下载的时间长度
    int m_TimeStep;

    // 用于保存最后一次任务结束的时间，m_lastsec+900就是这次的任务
    int m_lastsec;

    DBLoginInfo m_DBinfo;

    // 保存路径
    std::string m_savepath;
};


#endif // DOWNLOADMGR_H
