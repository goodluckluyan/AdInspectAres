#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <getopt.h>
#include <tr1/memory>
#include <execinfo.h>
#include "utility/FileEx.h"
#include "./log/MyLogger.h"
#include "MainProcess.h"


using namespace std;

MyLogger g_main_logwrite;
MyLogger g_download_logwrite;
MyLogger g_compare_logwrite;
MyLogger g_markjob_logwrite;
MyLogger g_templet_logwrite;
MyLogger g_db_logwrite;

#define loginfo(strlog,...)   g_main_logwrite.PrintLog(MyLogger::INFO,strlog,##__VA_ARGS__)
#define logerror(strlog,...)   g_main_logwrite.PrintLog(MyLogger::ERROR,strlog,##__VA_ARGS__)
#define logdebug(strlog,...)   g_main_logwrite.PrintLog(MyLogger::DEBUG,strlog,##__VA_ARGS__)
#define logfatal(strlog,...)   g_main_logwrite.PrintLog(MyLogger::FATAL,strlog,##__VA_ARGS__)

//程序退出标识
int g_nRunType = 0;
bool g_bAresQuit = false;
void * CompareEngine::m_ptrUser = NULL;
BC_CompareDone CompareEngine::m_ptrCompareDoneFun = NULL;
CompleteDLT_CallBack CDownLoadMgr::m_pDownloadCompeteFun = NULL;
void * CDownLoadMgr::m_Userdata = NULL;
pfunc_callback_mark MarkJob::m_pfunc_markcallback = NULL;
void* MarkJob::m_puserdata = NULL;

tr1::shared_ptr<CMainProcess>  ptr_MainProcess(new CMainProcess);

void QuitHandler(int sig)
{
//    Log("接收到退出信号，执行QuitHandler");
    g_bAresQuit = true;
}

//打印产品信息
void PrintProductInfo()
{
    // 输出软件说明信息
//    Log("#------------------------------------------------------------------------------#");
//    Log("#                      <<<<<AdInspectAres 1.0.0.1>>>>>                         #");
//    Log("#1.修改了自动测试任务中长时间的延时时不能被快速打断的问题                               #");
//    Log("#------------------------------------------------------------------------------#");
//    Log("# 命令行说明：");
//    Log("#  help : 打印命令行帮助说明");
//    Log("#  print -t 打印任务信息。");
//    Log("#------------------------------------------------------------------------------#");
}

//输入命令控制
void controller(std::tr1::shared_ptr<CMainProcess> &pMainProcess)
{
    // 打开标准输入设备
    int fdStdin;
    if((fdStdin = open("/dev/stdin", O_RDWR | O_NONBLOCK)) <= 0)
    {
//        Log("can not open stdin file !");
    }

    char c[50];
    std::string user_input;
    while (!g_bAresQuit)
    {
        // 任务轮询
        ptr_MainProcess->Run();

        // 获取输入
        int nSize = read(fdStdin, c, 50);
        if(nSize <= 0)
        {
            OS_Thread::Msleep(100);
            continue;
        }
        user_input.append(c, nSize);
        if ('\n' != c[nSize -1])
        {
            continue;
        }
        user_input.resize(user_input.size() - 1);


        if (user_input == "quit")
        {
            break;
        }
        else if("help" == user_input)
        {

            PrintProductInfo();
        }
        else if (user_input.size() >= 8 && (0 == user_input.compare(0, 5, "print", 5)))
        {
            do
            {
                std::string::size_type nPos = user_input.find('-');
                if(std::string::npos == nPos)
                {
                    break;
                }
                std::string substr;
                substr = user_input.substr(nPos + 1);
                if (substr.empty())
                {
                    break;
                }


                if(substr=="t")
                {
                    pMainProcess->Print(1);
                }
            }
            while (0);
        }
        user_input.clear();
    }


    close(fdStdin);

}

//主线程执行
void run()
{
    // 设置当前路径
    CFileEx::SetCurDirectory(CFileEx::GetExeDirectory().c_str());

    // 初始化主模块
    if (!ptr_MainProcess->Init())
    {
        OS_Thread::Sleep(3);
        return;
    }

    // 启动输入循环
    controller(ptr_MainProcess);

    // 反初始化主模块
    ptr_MainProcess->DeInit();
}

//守护进程
void InitDaemon(void)
{
#ifndef _WIN32
    int pid;
    int i;

    if(pid=fork())
        exit(0);//是父进程，结束父进程
    else if(pid< 0)
        exit(1);//fork失败，退出

    //是第一子进程，后台继续执行
    setsid();//第一子进程成为新的会话组长和进程组长
    //并与控制终端分离
    if(pid=fork())
        exit(0);//是第一子进程，结束第一子进程
    else if(pid< 0)
        exit(1);//fork失败，退出
    //是第二子进程，继续
    //第二子进程不再是会话组长

    for(i=0;i< sysconf(_SC_OPEN_MAX);++i)//关闭打开的文件描述符
        close(i);
    //chdir("/tmp");//改变工作目录到/tmp
    umask(0);//重设文件创建掩模
    return;
#else
    return;
#endif
}


void sig_fun(int iSigNum)
{

    char strInfo[1024];
    sprintf(strInfo, "revc a signal number:%d ", iSigNum);
//    pLogManage->WriteLog(ULOG_FATAL,LOG_MODEL_OTHER,0,ERROR_SIGCATCH_FUN,strInfo);

    void *stack_p[20];
    char **stack_info;
    int size;
    size = backtrace( stack_p, sizeof(stack_p));
    stack_info = backtrace_symbols( stack_p, size);
    for(int i=0; i<size; ++i)
    {
//        pLogManage->WriteLog(ULOG_FATAL,LOG_MODEL_OTHER,0,ERROR_SIGCATCH_FUN,stack_info[i]);
    }
    free(stack_info);
    if(0 == g_nRunType)
    {
        fflush(NULL);
    }
}

void sigterm(int signo)
{
  syslog(LOG_ERR,"catch TERM Signal !");
  g_bAresQuit = true;
}

void sighup(int signo)
{
    syslog(LOG_ERR,"catch HUP Signal !");
}

void InitSigFun()
{
    if(signal(SIGINT,sig_fun) == SIG_ERR)
    {
//        pLogManage->WriteLog(ULOG_FATAL,LOG_MODEL_OTHER,0,ERROR_SIGCATCH_FUN,"add signal Number:SIGINT");
    }
    if(signal(SIGALRM,sig_fun) == SIG_ERR)
    {
//        pLogManage->WriteLog(ULOG_FATAL,LOG_MODEL_OTHER,0,ERROR_SIGCATCH_FUN,"add signal Number:SIGALRM");
    }

    //防止关闭终端后，程序在后台运行，所有不处理SIGHUP信号，使用默认处理方式即结束进程
// 	if(signal(SIGHUP,sig_fun) == SIG_ERR)
// 	{
// 		pLogManage->WriteLog(ULOG_FATAL,LOG_MODEL_OTHER,0,ERROR_SIGCATCH_FUN,"add signal Number:SIGHUP");
// 	}
    if(signal(SIGPIPE,SIG_IGN) == SIG_ERR)
    {
//        pLogManage->WriteLog(ULOG_FATAL,LOG_MODEL_OTHER,0,ERROR_SIGCATCH_FUN,"add signal Number:SIGPIPE");
    }
    if(signal(SIGPOLL,sig_fun) == SIG_ERR)
    {
//        pLogManage->WriteLog(ULOG_FATAL,LOG_MODEL_OTHER,0,ERROR_SIGCATCH_FUN,"add signal Number:SIGPOLL");
    }
    if(signal(SIGPROF,sig_fun) == SIG_ERR)
    {
//        pLogManage->WriteLog(ULOG_FATAL,LOG_MODEL_OTHER,0,ERROR_SIGCATCH_FUN,"add signal Number:SIGPROF");
    }
    if(signal(SIGSTKFLT,sig_fun) == SIG_ERR)
    {
//        pLogManage->WriteLog(ULOG_FATAL,LOG_MODEL_OTHER,0,ERROR_SIGCATCH_FUN,"add signal Number:SIGSTKFLT");
    }
// 	if(signal(SIGTERM,sig_fun) == SIG_ERR)
// 	{
// 		pLogManage->WriteLog(ULOG_FATAL,LOG_MODEL_OTHER,0,ERROR_WEBSERVICE_CREATE_TRREAD,"add signal Number:SIGTERM");
// 	}
    if(signal(SIGUSR1,sig_fun) == SIG_ERR)
    {
//        pLogManage->WriteLog(ULOG_FATAL,LOG_MODEL_OTHER,0,ERROR_SIGCATCH_FUN,"add signal Number:SIGUSR1");
    }
    if(signal(SIGUSR2,sig_fun) == SIG_ERR)
    {
//        pLogManage->WriteLog(ULOG_FATAL,LOG_MODEL_OTHER,0,ERROR_SIGCATCH_FUN,"add signal Number:SIGUSR2");
    }
    if(signal(SIGVTALRM,sig_fun) == SIG_ERR)
    {
//        pLogManage->WriteLog(ULOG_FATAL,LOG_MODEL_OTHER,0,ERROR_SIGCATCH_FUN,"add signal Number:SIGVTALRM");
    }
    if(signal(SIGIO,sig_fun) == SIG_ERR)
    {
//        pLogManage->WriteLog(ULOG_FATAL,LOG_MODEL_OTHER,0,ERROR_SIGCATCH_FUN,"add signal Number:SIGIO");
    }
    if(signal(SIGABRT,sig_fun) == SIG_ERR)
    {
//        pLogManage->WriteLog(ULOG_FATAL,LOG_MODEL_OTHER,0,ERROR_SIGCATCH_FUN,"add signal Number:SIGABRT");
    }

    // 忽略子进程结束时发送的SIGCLD信号 ，防止僵尸进程
    signal(SIGCHLD,SIG_IGN);
}


//守护进程标识
bool bSaveProce = false;

int main(int argc, char* argv[])
{

    char oc;
    std::string th;
    char ec;
    bool matchd = false;
    bool matchs = false;

    while((oc=getopt(argc,argv,"t:e:i:m:"))!=-1)
    {
        switch(oc)
        {
//        case 'm':
//            matchd = true;
//            moduledir=std::string(optarg);
//            break;
//        case 'i':
//            matchs = true;
//            inspectfile=std::string(optarg);
//            break;
//        case 't':
//            th = std::string(optarg);
//            break;
//        case ':':
//            ec = (char )optopt;
//            std::cout<<"argument error,invalidate argument "<<ec<<std::endl;
//            break;
//        case '?':
//            std::cout<<"lack option argument!"<<std::endl;
//            break;
        }
    }

    g_main_logwrite.Init("./log.conf","Main");
    g_markjob_logwrite.Init("./log.conf","Main");
    g_templet_logwrite.Init("./log.conf","Main");
    g_download_logwrite.Init("./log.conf","Main");
    g_compare_logwrite.Init("./log.conf","Main");
    g_db_logwrite.Init("./log.conf","Main");



    PrintProductInfo();

    loginfo("#------------------------------------------------------------------------------#");
    loginfo("#                              AdInspect Startup                                    #");
    loginfo("#------------------------------------------------------------------------------#");


    run();


    // 程序退出标记
    loginfo("#------------------------------------------------------------------------------#");
    loginfo("#                               AdInspect End！                                   #");
    loginfo("#------------------------------------------------------------------------------#");
    return 0;


}
