

#ifdef _WIN32

#include "OSSharedMemory.h"
#include "windows.h"

struct OS_SharedMemory_Priv
{
	HANDLE hShm;
	void* pData;
	int nSize;
};

OS_SharedMemory::OS_SharedMemory()
:m_Priv(NULL)
{
}

OS_SharedMemory::~OS_SharedMemory()
{
	if(m_Priv)
	{
		OS_SharedMemory_Priv* priv = (OS_SharedMemory_Priv*) m_Priv;
		if(priv->pData) UnmapViewOfFile(priv->pData);
		CloseHandle(priv->hShm);
		delete priv;
	}
}

int OS_SharedMemory::Init(const char* name, int size)
{
	OS_SharedMemory_Priv* priv = new OS_SharedMemory_Priv;
	if(!priv) return -1;
	m_Priv = priv;

	SECURITY_ATTRIBUTES   sa;   
	sa.nLength=sizeof(sa);   
	sa.lpSecurityDescriptor=NULL;   
	sa.bInheritHandle=TRUE;   
	HANDLE hShm=CreateFileMappingA(INVALID_HANDLE_VALUE,&sa,PAGE_READWRITE,0, size, name);   
	if (hShm == NULL)
	{ 
		delete priv;
		m_Priv = NULL;
		return -1;
	}

	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		//printf("Already exist.\n");
	}

	priv->hShm = hShm;
	priv->pData = NULL;
	priv->nSize = size;

	return 0;
}

int OS_SharedMemory::Attach()
{
	OS_SharedMemory_Priv* priv = (OS_SharedMemory_Priv*) m_Priv;
	if(!priv) return -1;

	if(priv->pData) return 0;

	void* pBuf = MapViewOfFile(priv->hShm,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,                   
		0,                   
		priv->nSize);           

	if (pBuf == NULL) 
	{ 
		//printf("Could not map view of file (%d).\n", GetLastError()); 
		return -1;
	}

	priv->pData = pBuf;

	return 0;
}

void OS_SharedMemory::Detach()
{
	OS_SharedMemory_Priv* priv = (OS_SharedMemory_Priv*) m_Priv;
	if(!priv) return;

	if(!priv->pData) return;

	UnmapViewOfFile(priv->pData);
	priv->pData = NULL;
}

void* OS_SharedMemory::Data()
{
	OS_SharedMemory_Priv* priv = (OS_SharedMemory_Priv*) m_Priv;
	if(!priv) return NULL;
	return priv->pData;
}

#endif


