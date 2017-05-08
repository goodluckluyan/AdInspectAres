#include "C_SoapServer.h"
//#include "log/C_LogManage.h"
#include "threadManage/C_ThreadManage.h"
#include <sys/socket.h> 
using namespace std;

static int ExitSoapSystem = 0;
const int MaxClientAmount = 32;
bool g_bWebServiceQuit = false;
struct soap csoapExtern;

void* MonitorSoapServerMain(void *port)
{
	struct soap *csoapSon[MaxClientAmount];
	for(int i=0; i<MaxClientAmount; i++)
	{
		csoapSon[i] = NULL;
	}

	soap_init(&csoapExtern);
	csoapExtern.fget = MonitorForBrowser;
	soap_set_mode(&csoapExtern, SOAP_C_UTFSTRING);
	csoapExtern.bind_flags = SO_REUSEADDR;
	int result = soap_bind(&csoapExtern, NULL, *(unsigned short*)port, MaxClientAmount);
	if(result < 0)
	{
		cout<<"Soap bind failed, the port can not be connected."<<endl;
		exit(1);
	}

	csoapExtern.send_timeout = 5;
	csoapExtern.recv_timeout = 5;
	csoapExtern.accept_timeout = 5;//like tcp timeval, 5 second
	csoapExtern.max_keep_alive = MaxClientAmount;//it can accept 32 client at same time;
	csoapExtern.socket_flags = MSG_NOSIGNAL; // set to MSG_NOSIGNAL to disable sigpipe
//	csoapExtern.accept_flags = SO_NOSIGPIPE; // or this to prevent sigpipe 

	while(!g_bWebServiceQuit)
	{
		result = soap_accept(&csoapExtern);
		if(result < 0)
		{
			//soap_print_fault(&csoapExtern, stderr);
			continue;
		}
		else
		{
			//printf("MonitorSoapServerMain:recv a webservice request!\n");//recv a request,success
		}

		if(g_bWebServiceQuit)
		{
			break;
		}

		int i=0;
		for(i=0; i<MaxClientAmount; i++)
		{
			if(csoapSon[i] == NULL)
				break;
		}
		if(i == MaxClientAmount)
		{//gsoap can not provide services
			soap_destroy(&csoapExtern);
			soap_end(&csoapExtern);
//			C_LogManage::GetInstance()->WriteLog(ULOG_FATAL,LOG_MODEL_WEBS, 0, 0, "The client more than MaxClientAmount.");
			continue;
		}

		csoapSon[i] = soap_copy(&csoapExtern);
		if(!csoapSon[i])
		{
			continue;
		}

		C_ThreadManage *pThreadManage = C_ThreadManage::GetInstance();
		C_ThreadData *pThreadData = NULL;
		if(pThreadManage->GetIdlThread(&pThreadData) != 0)
		{
			//duheqing 2012-7-17
			soap_destroy(csoapSon[i]);
			soap_end(csoapSon[i]);
			soap_done(csoapSon[i]);
			free(csoapSon[i]);
			csoapSon[i] = NULL;
		
//			C_LogManage::GetInstance()->WriteLog(ULOG_FATAL,LOG_MODEL_WEBS, 0, 0, "GetIdlThread failed.");

			continue;
		}
		
		pThreadData->m_pGsoap = (void *)(&csoapSon[i]);
		pThreadData->m_iRunType = 0;
		pThreadData->resume();
	}

	soap_done(&csoapExtern);
	return NULL;
}

int MonitorForBrowser(struct soap *soapHandle)
{
	ifstream fin("mons.wsdl", ios::binary);
	if(!fin.is_open())
	{
		return 404;
	}

	soapHandle->http_content = "text/xml";
	soap_response(soapHandle, SOAP_FILE);

	while(!fin.eof())
	{
		fin.read(soapHandle->tmpbuf, sizeof(soapHandle->tmpbuf));
		if(soap_send_raw(soapHandle, soapHandle->tmpbuf, fin.gcount()))
		{
			break;
		}
	}
	
	fin.close();
	soap_end_send(soapHandle);
	return 0;
}


void *process_request(void *csoap)
{//<<soapdoc2.pdf>> page 45
	soap_serve(*(struct soap**)csoap);

	soap_destroy(*(struct soap**)csoap);
	soap_end(*(struct soap**)csoap);
	soap_done(*(struct soap**)csoap);

	free(*(struct soap**)csoap);
	*(struct soap**)csoap = NULL;

	return NULL;
}


void ExitSoapService()
{
	ExitSoapSystem = 1;
}
