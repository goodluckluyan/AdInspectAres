//@file:C_ThreadManage.cpp
//@brief: ...
//@author: wangzhongping@oristartech.com
//date: 2012-05-23


#include "C_ThreadManage.h"
#include "C_ErrorDef.h"
#include "para/C_Para.h"
#include "webservice/C_SoapServer.h"


extern bool g_bWebServiceQuit;
C_ThreadManage* C_ThreadManage::m_pInstance = NULL;
int Webservicefunc(C_ThreadData *pThreadData)
{
	process_request(pThreadData->m_pGsoap);
	pThreadData->m_pGsoap = NULL;
	pThreadData->m_iRunType = RUN_UNKNOWN_TYPE;
	return 0;
}

void * Threadfunc(void* pvPara)
{
	C_ThreadData *pThreadData = (C_ThreadData*) pvPara;
	pThreadData->suspend(SUSPEND_IDL_STATE);
	int iResult = -1;
	int iDeviceConnectionState = -1;
	
	string strError;
	
//	C_LogManage *pLogManage = C_LogManage::GetInstance();
	char strLog[1024];
	
	while(1)
	{
		if(pThreadData->m_bQuit)
		{
			break;
		}

		if(pThreadData->m_iRunType == 0)
		{
			if(Webservicefunc(pThreadData) != 0)
			{
//				LOG(0,"Webservicefunc failed!\n");//Add ErrorCtrl and log;
			}
			if(pThreadData->m_bQuit)
			{
				break;
			}
			pThreadData->suspend(SUSPEND_IDL_STATE);

			continue;
		}
		else if(pThreadData->m_iRunType == 1)
		{		
			int nResult = pThreadData->m_pTask->Exec();
			if(pThreadData->m_bQuit)
			{
				break;
			}
			if(nResult  == 0)
			{
				pThreadData->suspend(SUSPEND_IDL_STATE);

			}	
			else if(2 == nResult)
			{
//				C_LogManage *pLogManage = C_LogManage::GetInstance();
				char a[1024];
				memset(a, 0, 1024);
				sprintf(a,"线程的任务编号没有找到位执行相关的处理函数 CommandNumber:%d", pThreadData->m_pTask->m_iCommandNumber); 
				//printf("%s\n",a);
//				iResult = pLogManage->CreateLogNumber(ULOG_FATAL,LOG_MODEL_THREADMGR,0,ERROR_THREAD_TASK_NUMBER_NO_FIND);
//				pLogManage->WriteLog(iResult,a);
				pThreadData->m_pTask->ReInit();			
				pThreadData->suspend(SUSPEND_IDL_STATE);
			}
		}
		else
		{
			if(pThreadData->m_bQuit)
			{
				break;
			}
//			C_LogManage *pLogManage = C_LogManage::GetInstance();
			char a[1024];
			memset(a, 0, 1024);
			sprintf(a,"线程的任务类型错误。 CommandNumber:%d ", pThreadData->m_pTask->m_iCommandNumber); 
			//printf("%s\n",a);
//			iResult = pLogManage->CreateLogNumber(ULOG_FATAL,LOG_MODEL_THREADMGR,0,ERROR_THREAD_TASK_TYPE);
//			pLogManage->WriteLog(iResult,a);
			pThreadData->m_pTask->ReInit();					
			pThreadData->suspend(SUSPEND_IDL_STATE);
		}
	}
	pThreadData->SetState(QUIT_STATE);
	//printf("Thread %x Exit!\n",pThreadData);
	return (void*)0;
}

C_ThreadManage* C_ThreadManage::GetInstance()
{
    if(m_pInstance == NULL)
    {
        m_pInstance = new C_ThreadManage;
    }
    return m_pInstance;
}

void  C_ThreadManage::DestoryInstance()
{
	if(m_pInstance != NULL)
	{
		delete m_pInstance;
	}
	m_pInstance = NULL;
}

C_ThreadManage::C_ThreadManage()
{

}
C_ThreadManage::~C_ThreadManage()
{
	DeInit();
}

void  C_ThreadManage::DeInit()
{
	g_bWebServiceQuit = true;
	pthread_join(m_hWebserviceThread,NULL);
//	LOGINFFMT(0,"WebService Thread Exit!\n");


	int nLen = m_ThreadDataList.size();
	int i = 0 ;
	while(i < nLen )
	{
		C_ThreadData *ptr;
		if(GetIdlThread(&ptr)!=0)
		{
			continue;
		}
		else
		{
			ptr->m_bQuit = true;
			ptr->m_iRunType = 3;
			ptr->resume();
			i++;
		}
	}
	
	list<C_ThreadData* >::iterator it = m_ThreadDataList.begin();
	for( ;it != m_ThreadDataList.end() ;it++)
	{
		C_ThreadData * ptr = *it;
		if(ptr != NULL)
		{
			while(1)
			{
				if(ptr->GetState() == QUIT_STATE)
					break;
			}
			delete ptr;
		}
	}
	m_ThreadDataList.clear();
//	LOGINFFMT(0,"Thread Pool Exit!\n");
	
	

}
int C_ThreadManage::GetIdlThread(C_ThreadData **pThreadData)
{
		m_cs.EnterCS();
		list<C_ThreadData*>::iterator itTmp = m_itIdl;
		do
		{
			if(m_itIdl == m_ThreadDataList.end())
			{
				m_itIdl = m_ThreadDataList.begin();
			}

			if((*m_itIdl)->IsIdlState() == SUSPEND_IDL_STATE)
			{
				*pThreadData = (C_ThreadData*)(*m_itIdl);
				if(++m_itIdl == m_ThreadDataList.end())
				{
					m_itIdl = m_ThreadDataList.begin();
				}
				m_cs.LeaveCS() ;
				return 0;
			}
// 			else
// 			{
// 				printf("Current Thread State:%d\n",(int)(*m_itIdl)->GetState());
// 			}

			m_itIdl ++;

		}while(itTmp != m_itIdl);
//		C_LogManage::GetInstance()->WriteLog(ULOG_FATAL,LOG_MODEL_THREADMGR,0,ERROR_THREAD_LIST_FULL,
//			"no idel thread!");
		m_cs.LeaveCS();

    return ERROR_THREAD_LIST_FULL;
}

int C_ThreadManage::GetFinishThread(C_ThreadData **pThreadData)
{
		list<C_ThreadData*>::iterator itTmp = m_itFinish;
		do
		{
			if(m_itFinish == m_ThreadDataList.end())
			{
				m_itFinish = m_ThreadDataList.begin();
			}
			if((*m_itFinish)->GetState() == SUSPEND_FINISH_STATE)
			{
				*pThreadData = (C_ThreadData*)(*m_itFinish);
				if(++m_itFinish == m_ThreadDataList.end())
				{
					m_itFinish = m_ThreadDataList.begin();
				}
				return 0;
			}
			m_itFinish ++;
		}while(itTmp != m_itFinish);
    return INFO_NO_CONDITION_THREAD;
}

int C_ThreadManage::GetTimeoutThead(C_ThreadData **pThreadData,int iCurTime)
{
		list<C_ThreadData*>::iterator itTmp = m_itTimeout;
		do
		{
			if(m_itTimeout == m_ThreadDataList.end())
			{
				m_itTimeout = m_ThreadDataList.begin();
			}
			if((*m_itTimeout)->IsTimeoutState(iCurTime) == 0)
			{
				*pThreadData = (C_ThreadData*)(*m_itTimeout);
				if(++m_itTimeout == m_ThreadDataList.end())
				{
					m_itTimeout = m_ThreadDataList.begin();
				}
				return 0;
			}
			m_itTimeout ++;
		}while(itTmp != m_itTimeout);

    return INFO_NO_CONDITION_THREAD;	

}

int C_ThreadManage::InitThreadData()
{
//	C_LogManage *pLog = C_LogManage::GetInstance();
	//int iThreadCount = 50;
	C_ThreadData *pThreadData = NULL;
	pthread_attr_t attr;
	std::string strError; 
	if(pthread_attr_init(&attr) != 0)
	{
//		pLog->WriteLog(ULOG_FATAL,LOG_MODEL_THREADMGR,0,ERROR_INIT_THREAD_ATTRIB,"初试化线程属性数据错误。");
		return ERROR_INIT_THREAD_ATTRIB;
	}
	if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
	{
//		pLog->WriteLog(ULOG_FATAL,LOG_MODEL_THREADMGR,0,ERROR_SET_THREAD_ATTRIB,"设置线程属性数据错误。");
		return ERROR_SET_THREAD_ATTRIB;		
	}
	for(int i=0; i< THREAD_COUNT_IN_POOL; ++i)
	{
		pThreadData = new C_ThreadData;
		if(pthread_create(&pThreadData->m_hThread, &attr, Threadfunc, (void*)pThreadData) != 0)
		{
//			pLog->WriteLog(ULOG_FATAL,LOG_MODEL_THREADMGR,0,ERROR_CREATE_TRREAD,"创建线程错误。");
			return ERROR_CREATE_TRREAD;		
		}
		m_ThreadDataList.push_back(pThreadData);
	}
	m_itIdl = m_ThreadDataList.begin();
	m_itFinish = m_ThreadDataList.begin();
	m_itTimeout = m_ThreadDataList.begin();
	return 0;
}	

int C_ThreadManage::GetThreadData(pthread_t &threadId,C_ThreadData **pThreadData)
{
	list<C_ThreadData*>::iterator	it= m_ThreadDataList.begin();
	for(; it != m_ThreadDataList.end(); ++it)
	{
			if((*pThreadData = (*it)->GetThreadData(threadId)) != NULL)
			{
				return 0;		
			} 
	}
	*pThreadData = NULL;
//	C_LogManage *pLog = C_LogManage::GetInstance();
//	pLog->WriteLog(ULOG_FATAL,LOG_MODEL_THREADMGR,0,ERROR_NO_FIND_THREADDATA_OF_ID,"根据threadID没有找到对应的C_ThreadData。");
	return ERROR_NO_FIND_THREADDATA_OF_ID;
}

void C_ThreadManage::CheckTimeoutThread()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int iCurTime = tv.tv_sec;
    list<C_ThreadData*>::iterator itTmp = m_ThreadDataList.begin();
    for(; itTmp != m_ThreadDataList.end(); itTmp++)
    {
        (*itTmp)->IsTimeoutState(iCurTime);
    }
    return;
}


//WebService Listen Thread
void * WebServiceListneFunc(void* pvPara)
{
	
	unsigned short usPort = C_Para::GetInstance()->m_nWebServicePort;
	void* iResult = MonitorSoapServerMain((void*)&usPort);
	if(iResult != NULL)
	{
		return iResult;
	}
	
	return (void*)0;
}


int C_ThreadManage::InitWebserviceThread()
{
	
//	C_LogManage *pLog = C_LogManage::GetInstance();

	//C_ThreadData *pThreadData = NULL;
	pthread_attr_t attr;
	std::string strError; 
	if(pthread_attr_init(&attr) != 0)
	{
//		pLog->WriteLog(ULOG_FATAL,LOG_MODEL_THREADMGR,0,ERROR_INIT_WEBSERVICE_THREAD_ATTRIB,"初试化webservice线程属性数据错误。");
		return ERROR_INIT_WEBSERVICE_THREAD_ATTRIB;
	}
// 	if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
// 	{
// 		pLog->WriteLog(ULOG_FATAL,LOG_MODEL_THREADMGR,0,ERROR_SET_WEBSERVICE_THREAD_ATTRIB,"设置webservice线程属性数据错误。");
// 		return ERROR_SET_WEBSERVICE_THREAD_ATTRIB;		
// 	}

	if(pthread_create(&m_hWebserviceThread, &attr, WebServiceListneFunc, NULL) != 0)
	{
//		pLog->WriteLog(ULOG_FATAL,LOG_MODEL_THREADMGR,0,ERROR_WEBSERVICE_CREATE_TRREAD,"创建webservice线程错误。");
		return ERROR_WEBSERVICE_CREATE_TRREAD;		
	}		
	return 0;
}


//20140305 by xiaozhengxiu add
// Pointer to array of locks.
static pthread_mutex_t *lock_cs=NULL;

// locking 回调函数，由openssl库回调，向 openssl 库 提供 lock/unlock，或更详细些 read lock 或 write lock 的功能
// Locking callback. The type, file and line arguments are
// ignored. The file and line may be used to identify the site of the
// call in the OpenSSL library for diagnostic purposes if required.
static void ecos_locking_callback(int mode, int type, const char *file, int line)
{
//     if (mode & CRYPTO_LOCK)
//     {
//         pthread_mutex_lock(&(lock_cs[type]));
//     }
//     else
//     {
//         pthread_mutex_unlock(&(lock_cs[type]));
//     }
}

// thraed id 回调函数，由openssl回调，向 openssl 库提供当前线程号
// Thread id callback.
static unsigned long ecos_thread_id_callback(void)
{
//    unsigned long ret;

 //   return (unsigned long)pthread_self();
//    return(ret);

//    return (unsigned long)pthread_thread_get_id(pthread_thread_self());
}

// 多线程保护的初始化
// This function allocates and initializes the lock array
// and registers the callbacks. This should be called
// after the OpenSSL library has been initialized and
// before any new threads are created.  
void C_ThreadManage::create_ssl_lock(void)
{
//    int i;

    // 动态创建 lock 数组
    // Allocate lock array according to OpenSSL's requirements
    //lock_cs=(pthread_mutex_t *)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
//	  if (!lock_cs)
//        return;

    // lock 数组初始化
    // Initialize the locks
//     for (i=0; i<CRYPTO_num_locks(); i++)
//     {
//         pthread_mutex_init(&(lock_cs[i]),NULL);
//     }

    //  注册两个回调函数
    // Register callbacks
//     CRYPTO_set_id_callback((unsigned long(*)())ecos_thread_id_callback);
//     CRYPTO_set_locking_callback(ecos_locking_callback);
}

// 多线程保护的反初始化
// This function deallocates the lock array and deregisters the
// callbacks. It should be called after all threads have
// terminated.  
void C_ThreadManage::ssl_lock_cleanup(void)
{
 //   int i;
//	  if (!lock_cs)
//       return;

    // 清空 locking 回调函数
    // Deregister locking callback. No real need to
    // deregister id callback.
    //CRYPTO_set_locking_callback(NULL);

    // 销毁初始化时分配的 lock 数组
    // Destroy the locks
//     for (i=0; i<CRYPTO_num_locks(); i++)
//     {
//         pthread_mutex_destroy(&(lock_cs[i]));
//     }

   // 销毁初始化时分配的 lock 数组
    // Release the lock array.
    //OPENSSL_free(lock_cs);
 //   lock_cs = NULL;
}

