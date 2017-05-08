

#ifndef _WIN32

#include "OSSharedMemory.h"

#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

struct OS_SharedMemory_Priv
{
	int hShm;
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
		//if(priv->pData) UnmapViewOfFile(priv->pData);
		//CloseHandle(priv->hShm);
		delete priv;
	}
}

int OS_SharedMemory::Init(const char* name, int size)
{
	OS_SharedMemory_Priv* priv = new OS_SharedMemory_Priv;
	if(!priv) return -1;
	m_Priv = priv;

	// 需要一个真实的文件路径作为标识符
	char pathname[256];
	sprintf(pathname, "/tmp/shm.key.%s", name);
	FILE* fp = fopen(pathname, "wt");
	fclose(fp);

	// 生成一个唯一的key(int)
	key_t iKey = ftok(pathname, 1);
	if(iKey < 0)
	{
		printf("failed to make key for shared memory.\n");
		return -1;
	}
	
	// 创建共享内存，如果已经存在，则打开它
	int hShm = shmget(iKey, size, IPC_CREAT | 0666) ;
	if(hShm < 0)
	{
		printf("failed to create shared memory.\n");
		delete priv;
		m_Priv = NULL;
		return -1;

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

	void* pBuf = shmat(priv->hShm, NULL, 0);     
	if (pBuf == NULL) 
	{ 
		printf("cannot attach shared memory.\n");
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

	shmdt(priv->pData);
	priv->pData = NULL;
}

void* OS_SharedMemory::Data()
{
	OS_SharedMemory_Priv* priv = (OS_SharedMemory_Priv*) m_Priv;
	if(!priv) return NULL;
	return priv->pData;
}

#endif



