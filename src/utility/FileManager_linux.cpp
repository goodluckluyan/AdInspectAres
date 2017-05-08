#include <ftw.h>


#include "FileManager_linux.h"
#include <iconv.h>
#include "log/MyLogger.h"


#if 0
typedef struct _REPORT_STATUS
{
	char status[20];
	char level[20];
	char module[50];
	char error_code_str[50];
	char message[1024];
	char message_la[1024];
}REPORT_STATUS;
#endif

#define STDIN 0
#if 0
#include "./xmlparser/include/XmlParserModule.h"
#endif

FILE_INFOS *g_pfile_infos;
char g_path[255];
FINDFILE_TYPE g_findtype;
int g_filecount;
int g_dircount;
int g_depth;

extern MyLogger g_boot_logwrite;


#define COPYING_SPEED	20*1024*1024	///20M/s

#define UNIT_COPY_SIZE 	65536	//65536
#define UNIT_HASH_SIZE 	65536	//65536

#define MAX_DIR_COUNT		5000
#define MAX_FILE_COUNT		50000
#define MAX_DIR_DEPTH		6

#define DEBUG_PRINT	0

////gkm add 20141216 start
#define MIN_RATE_COPY       1048576     //1024*1024  1M
#define MAX_RATE_COPY       104857600   //1024*1024*100  100M
////gkm add 20141216 end

//#if defined(_WIN32)
//void copy_thread(LPVOID pvoid)
//#else
//void *copy_thread(LPVOID pvoid)
//#endif
//{
//	CFileManager *pproc = (CFileManager *)pvoid;
//
//	pproc->copy_procedure();
//	
//	pthread_exit(0);
//}

int file_seek_g(const char *fpath,const struct stat *sb,int flag)
{
	int ret = 0;
	CFileManager filemanager;
	FILE_INFO *pfileinfo = NULL;
	
	if(strcmp(fpath,g_path))
	{
		switch(g_findtype)
		{
		case FIND_DIR:

//			if( S_ISDIR(sb->st_mode))
			if( FTW_D == flag)
			{
				filemanager.NewSpaceFileInfo(&pfileinfo);
				strcpy(pfileinfo->pfilefullname,fpath);
				pfileinfo->filetype = FILEINFO_DIR;
				pfileinfo->filesize = sb->st_size;
				stat64(pfileinfo->pfilefullname,&pfileinfo->file_st);
			
				g_pfile_infos->push_back(pfileinfo);
			}
			break;
		case FIND_FILE:
//			if( S_ISREG(sb->st_mode))
			if( FTW_F == flag )
			{
				filemanager.NewSpaceFileInfo(&pfileinfo);
				strcpy(pfileinfo->pfilefullname,fpath);
				pfileinfo->filetype = FILEINFO_DIR;
				pfileinfo->filesize = sb->st_size;
				stat64(pfileinfo->pfilefullname,&pfileinfo->file_st);
			
				g_pfile_infos->push_back(pfileinfo);
			}
			break;
		case FIND_ALL:
//			if( S_ISDIR(sb->st_mode) || S_ISREG(sb->st_mode))
			if( FTW_D == flag || FTW_F == flag)
			{
				filemanager.NewSpaceFileInfo(&pfileinfo);
				strcpy(pfileinfo->pfilefullname,fpath);

				if( FTW_D == flag)
				{
					pfileinfo->filetype = FILEINFO_DIR;
				}
				else
				{
					pfileinfo->filetype = FILEINFO_FILE;
				}
				pfileinfo->filesize = sb->st_size;
				stat64(pfileinfo->pfilefullname,&pfileinfo->file_st);
  			
				g_pfile_infos->push_back(pfileinfo);
			}
			break;
		default:
			break;

		}
	}
	
	return ret;
}

CFileManager::CFileManager(void)
{
	memset(m_descfullpath,0,sizeof(m_descfullpath));
	memset(m_sourcefullpath,0,sizeof(m_sourcefullpath));
	m_size_copyingfile = 0;
	m_size_copiedelapsed = 0;
	m_copy_state = COPY_STOP;
	m_bcopying = 0;	
	m_isrunning = 0;
    /// gkm add 20141216 start
    m_min_loop_counter = MIN_RATE_COPY / UNIT_COPY_SIZE;
    m_max_loop_counter = MAX_RATE_COPY / UNIT_COPY_SIZE;

    m_size_copiedelapsed_real = 0;
    m_copy_rate = MAX_RATE_COPY + 1024;
    m_copy_rate_real = 0;
    m_wait_loop_counter = m_max_loop_counter;
    /// gkm add 20141216 end

	m_thread_isexist = 0;

	///EmptyLang();


	m_pf_read = NULL;
	UninitReadFile();
}

CFileManager::~CFileManager(void)
{
}
////
int CFileManager::PingIsOk(const char *pipaddress,int *pisok)
{
	int ret = RET_SUCCESS;
	FILE *pf = NULL;
	char pstrcommand[255];
	char pstrresult[255];
	char pingok_str[100];
	int timeout = 2;
	int counter = 2;

	if(NULL == pipaddress ||
		NULL == pisok ||
		(!strcmp(pipaddress,"")))
	{
		return PARAMETER_ERROR;
	}

	*pisok = 0;

	pf = NULL;
	memset(pstrcommand,0,sizeof(pstrcommand));
	memset(pstrresult,0,sizeof(pstrresult));
	memset(pingok_str,0,sizeof(pingok_str));

	sprintf(pstrcommand,"ping -w %d -c %d %s",timeout,counter,pipaddress);
	
//	sprintf(pingok_str,"%d packets transmitted, %d received, 0% packet loss",counter,timeout);
//	sprintf(pingok_str,"received, 0%% packet loss");
	sprintf(pingok_str,"bytes from %s",pipaddress);

#if 0
	printf("command is:%s\n",pstrcommand);
#endif

	pf = popen(pstrcommand,"r");
	if(NULL == pf)
	{
		printf("%s:command executed failed\n",pstrcommand);
		return ret;
	}

	while(NULL != fgets(pstrresult,sizeof(pstrresult),pf))
	{
		if('\n' == pstrresult[strlen(pstrresult)-1])
		{
				pstrresult[strlen(pstrresult)-1] = '\0';
		}

#if 0
		if(NULL != pstrresult)
		{
			printf("%s=>%s\n",pstrcommand,pstrresult);
		}
#endif 

		if(!(*pisok))
		{
			
			if(NULL != pstrresult)
			{
				if(NULL != strstr(pstrresult,pingok_str))
				{
					*pisok = 1;
				}
				else
				{
					*pisok = 0;
				}
	
			}			
		}

	}

	if(-1 == pclose(pf))
	{
#if 0
		printf("%s:command closed failed\n",pstrcommand);
#endif

	}

#if 0
	if(NULL != pstrresult)
	{
		printf("%s result is:%d\n",pstrcommand,*pisok);
	}
#endif

#if 0
		printf("pstrcommand:%s\n",pstrcommand);
		printf("isok:%d\n",*pisok);
#endif
	

	return ret;
}
int CFileManager::IsDir(const char *pdirpath,int *pisdir)
{
	int ret = RET_SUCCESS;
	FILE *pf = NULL;
	char pstrcommand[255];
	char pstrresult[255];

	if(NULL == pdirpath ||
		NULL == pisdir)
	{
		return PARAMETER_ERROR;
	}

	*pisdir = 0;

#if 0
	pf = NULL;
	memset(pstrcommand,0,sizeof(pstrcommand));
	memset(pstrresult,0,sizeof(pstrresult));

	sprintf(pstrcommand,"ls -l -d %s 2>/dev/null | awk '{print $1}'",pdirpath);

	pf = popen(pstrcommand,"r");
	if(NULL == pf)
	{
		printf("%s:command executed failed\n",pstrcommand);
		return ret;
	}

	while(NULL != fgets(pstrresult,sizeof(pstrresult),pf))
	{
		if('\n' == pstrresult[strlen(pstrresult)-1])
		{
				pstrresult[strlen(pstrresult)-1] = '\0';
		}

#if DEBUG_PRINT
		if(NULL != pstrresult)
		{
			printf("%s=>%s\n",pstrcommand,pstrresult);
		}
#endif 

	}

	if(-1 == pclose(pf))
	{
#if 0
		printf("%s:command closed failed\n",pstrcommand);
#endif

	}

	if( NULL == pstrresult ||
		(!strcmp(pstrresult,"")))
	{
		//printf("%s:result is null\n",pstrcommand);
		*pisdir = 0;
	}
	else
	{
		if('d' == pstrresult[0])
		{
			*pisdir = 1;
		}
		else
		{
			*pisdir = 0;
		}

		//printf("%s:result is:%s\n",pstrcommand,pstrresult);
	}
#endif

#if 1
	char dirpath_buff[255]={'\0'};
	FILE_INFO *pfileinfo = NULL;

	NewSpaceFileInfo(&pfileinfo);

	memset(dirpath_buff,0,sizeof(dirpath_buff));
	sprintf(dirpath_buff,"%s",pdirpath);
	SubtractSeparator(dirpath_buff,1,1);

	ret = GetFileInfo(dirpath_buff,pfileinfo);

	if(ret)
	{
		DeleteSpaceFileInfo(&pfileinfo);
		return ret;
	}

	if(FILEINFO_DIR==pfileinfo->filetype)
	{
		*pisdir=1;
	}

	DeleteSpaceFileInfo(&pfileinfo);
#endif


#if DEBUG_PRINT
	printf("pstrcommand:%s\n",pstrcommand);
	printf("%s:isdir:%d\n",pdirpath,*pisdir);
#endif	

	return ret;
}
//int CFileManager::EmptyLang()
//{
//	int ret = RET_SUCCESS;
//	FILE *pf = NULL;
//	char pstrcommand[255];
//	char pstrresult[255];
//
//#if 0
//
//	pf = NULL;
//	memset(pstrcommand,0,sizeof(pstrcommand));
//	memset(pstrresult,0,sizeof(pstrresult));
//
//	sprintf(pstrcommand,"export LANG=\"\"");
//
//	pf = popen(pstrcommand,"r");
//	if(NULL == pf)
//	{
//		printf("%s:command executed failed\n",pstrcommand);
//		return ret;
//	}
//#endif
//
//	ret = setenv("LANG","",1);
//	
//#if 0
//	pf = NULL;
//	memset(pstrcommand,0,sizeof(pstrcommand));
//	memset(pstrresult,0,sizeof(pstrresult));
//
//	sprintf(pstrcommand,"export");
//
//	pf = popen(pstrcommand,"r");
//	if(NULL == pf)
//	{
//		printf("%s:command executed failed\n",pstrcommand);
//		return ret;
//	}
//
//	while(NULL != fgets(pstrresult,sizeof(pstrresult),pf))
//	{
//		if('\n' == pstrresult[strlen(pstrresult)-1])
//		{
//				pstrresult[strlen(pstrresult)-1] = '\0';
//		}
//		
//#if DEBUG_PRINT
//		if(NULL != pstrresult)
//		{
//			printf("%s=>%s\n",pstrcommand,pstrresult);
//		}
//#endif		
//		
//	}
//
//	if(-1 == pclose(pf))
//	{
//#if 0
//		printf("%s:command closed failed\n",pstrcommand);
//#endif
//
//	}
//
//#endif	
//
//	return ret;
//}
int CFileManager::ReplaceCharInString(char *pbuff,int max_len,char ch_from,char ch_to)
{
	int ret = RET_SUCCESS;
	char *ptemp = NULL;
	int index=0;
	if(NULL == pbuff)
	{
		return ret;
	}
	ptemp = pbuff;
		
	while((ptemp[index] != '\0') &&
				(index < max_len))
	{
		if(ptemp[index] == ch_from)
		{
			ptemp[index]=ch_to;
		}
		index++;
	}
		
	return ret;
}
int CFileManager::ReplaceStr(char *pbuff_src,const char *pbuff_from,const char *pbuff_to)
{
	int ret = 0;
	int ilen_from = 0;
	int ilen_to = 0;
	int ilen_temp = 0;
    char pbuff_clip[2048];
    char pbuff_temp[4096];
    char *ppos_str = NULL;
    char *pbuff_src_temp = NULL;

	if( NULL == pbuff_src ||
		(!strcmp(pbuff_src,"")) ||
		NULL == pbuff_from ||
		(!strcmp(pbuff_from,"")) ||
		NULL == pbuff_to ||
		(!strcmp(pbuff_to,"")))
	{
		return ret;
	}

	if(!strcmp(pbuff_from,pbuff_to))
	{
		return ret;
	}

    memset(pbuff_temp,0,4096);

    pbuff_src_temp = pbuff_src;
    ilen_from = strlen(pbuff_from);
	ilen_to = strlen(pbuff_to);


    while(1)
	{

        ppos_str = strstr(pbuff_src_temp, pbuff_from);

        if(NULL == ppos_str)
        {
            strcat(pbuff_temp,pbuff_src_temp);
            break;
        }
        else
        {
            ilen_temp = ppos_str - pbuff_src_temp;
            memset(pbuff_clip,0,2048);
            strncpy(pbuff_clip,pbuff_src_temp, ilen_temp);
            strcat(pbuff_temp,pbuff_clip);
            strcat(pbuff_temp,pbuff_to);
            pbuff_src_temp += ilen_temp + ilen_from;
        }
	}

    sprintf(pbuff_src,"%s",pbuff_temp);

	return 0;
}
int CFileManager::DeleteCharA(char *pbuff_src,char ch)
{
    int ret = 0;
    char pbuff_temp[4096];
    char *ptemp = NULL;
    int ilen = 0;

    if( NULL == pbuff_src ||
        (!strcmp(pbuff_src,"")))
    {
        return ret;
    }

    //printf("pbuff_src:%s\n",pbuff_src);

    ptemp=pbuff_src;

    while((*ptemp) != '\0')
    {
        if((*ptemp) == ch)
        {
             char *ptemp_1 = ptemp;
             char *ptemp_2 = ptemp_1+1;
             while( ((*ptemp_1++) = (*ptemp_2++)) != '\0')
             {
                 //printf("%c\n",*ptemp_1);
             }
        }
        ptemp++;


    }


    //printf("pbuff_src:%s\n",pbuff_src);

    return 0;
}
int CFileManager::code_convert(const char *pcharset_from,
						const char *pcharset_to,
						char *pbuff_in,
						size_t len_in,
						char *pbuff_out,
						size_t maxlen_out,
						size_t *plen_out)
{
	int ret = RET_SUCCESS;
	char *pbuff_in_bak = NULL;
	char *pbuff_out_bak = NULL;
	iconv_t cd;

	cd = iconv_open(pcharset_to,pcharset_from);

	if(cd==0)
	{
		return -1;
	}

	pbuff_in_bak = pbuff_in;
	pbuff_out_bak = pbuff_out;

	if(iconv(cd,&pbuff_in,&len_in,&pbuff_out,plen_out)==-1)
	{
		return -1;
	}

	iconv_close(cd);

	*plen_out = pbuff_out - pbuff_out_bak;
	memset(pbuff_out,0,maxlen_out - *plen_out);

	int temp = *plen_out;

	pbuff_in = pbuff_in_bak;
	pbuff_out = pbuff_out_bak;

	return ret;
 }
int CFileManager::IsExistPath(const char *ppath,int *pisexist)
{
	int ret = RET_SUCCESS;
	
#if 0	
	FILE *pf = NULL;
	char pstrcommand[255];
	char *pstrresult = NULL;
	int isdir = 0;

	if(NULL == ppath ||
		NULL == pisexist)
	{
		return PARAMETER_ERROR;
	}

	*pisexist = 0;

	IsDir(ppath,&isdir);
	if(isdir)
	{
		*pisexist = 1;

#if 0
		printf("isexist:%d\n",*pisexist);
#endif
		return ret;
	}

	pf = NULL;
	pstrresult = new char[1048576];
	memset(pstrcommand,0,sizeof(pstrcommand));
	memset(pstrresult,0,1048576);

	sprintf(pstrcommand,"ls -l %s 2>/dev/null",ppath);

	pf = popen(pstrcommand,"r");
	if(NULL == pf)
	{
		printf("%s:command executed failed\n",pstrcommand);
		delete [] pstrresult;
		pstrresult = NULL;
		return ret;
	}

	while(NULL != fgets(pstrresult,sizeof(pstrresult),pf))
	{
		if('\n' == pstrresult[strlen(pstrresult)-1])
		{
				pstrresult[strlen(pstrresult)-1] = '\0';
		}

#if DEBUG_PRINT
		if(NULL != pstrresult)
		{
			printf("%s=>%s\n",pstrcommand,pstrresult);
		}
#endif 
	}

	if(-1 == pclose(pf))
	{
#if 0
		printf("%s:command closed failed\n",pstrcommand);
#endif
	}

	if( NULL == pstrresult ||
		(!strcmp(pstrresult,"")))
	{
		//printf("%s:result is null\n",pstrcommand);
		*pisexist = 0;
	}
	else
	{
		if(NULL == strstr(pstrresult,ppath))
		{
			*pisexist = 0;
		}
		else
		{
			*pisexist = 1;
		}

		//printf("%s:result is:%s\n",pstrcommand,pstrresult);
	}
	
	delete [] pstrresult;
	pstrresult = NULL;

#else

	struct stat64 filedata;
	*pisexist = 0;
	ret = stat64( ppath, &filedata );

	if(-1 == ret)
	{
		return ret;
	}

	*pisexist = 1;
	ret = 0;
	
#endif	


#if DEBUG_PRINT
	printf("%s:isexist:%d",ppath,*pisexist);
#endif	

	return ret;
}
int CFileManager::CreateDir(const char *pdirpath)
{
	int ret = RET_SUCCESS;
	FILE *pf = NULL;
	char pstrcommand[255];
	char pstrresult[255];
	int isdir = 0;
	int isexist = 0;

	if(NULL == pdirpath)
	{
		return PARAMETER_ERROR;
	}

	IsExistPath(pdirpath,&isexist);

	if(isexist)
	{
		IsDir(pdirpath,&isdir);

		if(isdir)
		{
			return ret;
		}
		else
		{
			DeleteDirOrFile(pdirpath);
		}
	}

	pf = NULL;
	memset(pstrcommand,0,sizeof(pstrcommand));
	memset(pstrresult,0,sizeof(pstrresult));

	sprintf(pstrcommand,"mkdir -p %s",pdirpath);

	pf = popen(pstrcommand,"r");
	if(NULL == pf)
	{
		printf("%s:command executed failed\n",pstrcommand);
		return ret;
	}

	while(NULL != fgets(pstrresult,sizeof(pstrresult),pf))
	{
		if('\n' == pstrresult[strlen(pstrresult)-1])
		{
				pstrresult[strlen(pstrresult)-1] = '\0';
		}

#if DEBUG_PRINT
		if(NULL != pstrresult)
		{
			printf("%s=>%s\n",pstrcommand,pstrresult);
		}
#endif 
	}


	if(-1 == pclose(pf))
	{
#if 0
		printf("%s:command closed failed\n",pstrcommand);
#endif
	}

#if DEBUG_PRINT
	printf("%s:result is:%s\n",pstrcommand,pstrresult);
#endif

	return ret;
}
int CFileManager::CreateFile(const char *pfilefullpath)
{
	int ret = RET_SUCCESS;
	FILE *pf = NULL;
	char pstrcommand[255];
	char pstrresult[255];

	if(NULL == pfilefullpath)
	{
		return PARAMETER_ERROR;
	}

	DeleteDirOrFile(pfilefullpath);

	pf = NULL;
	memset(pstrcommand,0,sizeof(pstrcommand));
	memset(pstrresult,0,sizeof(pstrresult));

	sprintf(pstrcommand,"touch %s",pfilefullpath);

	pf = popen(pstrcommand,"r");
	if(NULL == pf)
	{
		printf("%s:command executed failed\n",pstrcommand);
		return ret;
	}

	while(NULL != fgets(pstrresult,sizeof(pstrresult),pf))
	{
		if('\n' == pstrresult[strlen(pstrresult)-1])
		{
				pstrresult[strlen(pstrresult)-1] = '\0';
		}

#if DEBUG_PRINT
		if(NULL != pstrresult)
		{
			printf("%s=>%s\n",pstrcommand,pstrresult);
		}
#endif 
	}

	if(-1 == pclose(pf))
	{
#if 0
		printf("%s:command closed failed\n",pstrcommand);
#endif
	}

#if DEBUG_PRINT
	printf("%s:result is:%s\n",pstrcommand,pstrresult);
#endif

	return ret;
}
int CFileManager::DeleteDirOrFile(const char *ppath)
{
	int ret = RET_SUCCESS;
	FILE *pf = NULL;
	char pstrcommand[255];
	char pstrresult[255];

	if(NULL == ppath)
	{
		return PARAMETER_ERROR;
	}

	pf = NULL;
	memset(pstrcommand,0,sizeof(pstrcommand));
	memset(pstrresult,0,sizeof(pstrresult));

	sprintf(pstrcommand,"rm -rf %s",ppath);

	pf = popen(pstrcommand,"r");
	if(NULL == pf)
	{
		printf("%s:command executed failed\n",pstrcommand);
		return ret;
	}

	while(NULL != fgets(pstrresult,sizeof(pstrresult),pf))
	{
		if('\n' == pstrresult[strlen(pstrresult)-1])
		{
				pstrresult[strlen(pstrresult)-1] = '\0';
		}

#if DEBUG_PRINT
		if(NULL != pstrresult)
		{
			printf("%s=>%s\n",pstrcommand,pstrresult);
		}
#endif 
	}


	if(-1 == pclose(pf))
	{
#if 0
		printf("%s:command closed failed\n",pstrcommand);
#endif
	}

#if DEBUG_PRINT
	printf("%s:result is:%s\n",pstrcommand,pstrresult);
#endif

	return ret;
}
int CFileManager::GetFileFullNamesByPath(const char *ppath,char (*pfilefullnames)[BUFF_SIZE_50],int maxcount,int max_len,int *pfilefullname_count)
{
	int ret = RET_SUCCESS;
	FILE *pf = NULL;
	char pstrcommand[255];
	char pstrresult[255];
	int filefullname_count = 0;
	CFileManager filemanager;

	if(NULL == ppath ||
		NULL == pfilefullnames ||
		0 >= maxcount ||
		0 >= max_len ||
		NULL ==pfilefullname_count)
	{
		return PARAMETER_ERROR;
	}

	if(!strcmp(ppath,""))
	{
		return ret;
	}

	pf = NULL;
	memset(pstrcommand,0,sizeof(pstrcommand));
	memset(pstrresult,0,sizeof(pstrresult));

//	sprintf(pstrcommand,"ls -l %s | awk '{printf $9}'",ppath);
	sprintf(pstrcommand,"ls %s",ppath);

	pf = popen(pstrcommand,"r");
	if(NULL == pf)
	{
		printf("%s:command executed failed\n",pstrcommand);
		return ret;
	}

	while(NULL != fgets(pstrresult,sizeof(pstrresult),pf))
	{
		if('\n' == pstrresult[strlen(pstrresult)-1])
		{
				pstrresult[strlen(pstrresult)-1] = '\0';
		}
		
#if DEBUG_PRINT
		if(NULL != pstrresult)
		{
			printf("%s=>%s\n",pstrcommand,pstrresult);
		}
#endif 		

		if(!strcmp(pstrresult,""))
		{
			continue;
		}


		if(filefullname_count > maxcount)
		{
			break;
		}

		memset(pfilefullnames[filefullname_count],0,max_len);
		strcpy(pfilefullnames[filefullname_count],ppath);
		filemanager.AddSeparator(pfilefullnames[filefullname_count],1,1);
		strcat(pfilefullnames[filefullname_count],pstrresult);

#if DEBUG_PRINT
		printf("pfilefullnames[%d]:%s\n",filefullname_count,pfilefullnames[filefullname_count]);
#endif

		filefullname_count++;
	}

	if(-1 == pclose(pf))
	{
#if 0
		printf("%s:command closed failed\n",pstrcommand);
#endif

	}

	*pfilefullname_count = filefullname_count;

	return ret;
}
int CFileManager::IsEmptyDir(const char *ppath,int *pisempty)
{
	int ret = RET_SUCCESS;
	FILE *pf = NULL;
	char pstrcommand[255];
	char pstrresult[255];

	if(NULL == ppath ||
		NULL == pisempty)
	{
		return PARAMETER_ERROR;
	}

	if(!strcmp(ppath,""))
	{
		return ret;
	}

	pf = NULL;
	memset(pstrcommand,0,sizeof(pstrcommand));
	memset(pstrresult,0,sizeof(pstrresult));

	sprintf(pstrcommand,"ls -l %s | grep \"total 0\"",ppath);

	pf = popen(pstrcommand,"r");
	if(NULL == pf)
	{
		printf("%s:command executed failed\n",pstrcommand);
		return ret;
	}

	while(NULL != fgets(pstrresult,sizeof(pstrresult),pf))
	{
		if('\n' == pstrresult[strlen(pstrresult)-1])
		{
				pstrresult[strlen(pstrresult)-1] = '\0';
		}
		
#if DEBUG_PRINT
		if(NULL != pstrresult)
		{
			printf("%s=>%s\n",pstrcommand,pstrresult);
		}
#endif		
		
	}

	if(-1 == pclose(pf))
	{
#if 0
		printf("%s:command closed failed\n",pstrcommand);
#endif

	}

	if( NULL == pstrresult ||
		(!strcmp(pstrresult,"")))
	{
		//printf("%s:result is null\n",pstrcommand);
		*pisempty = 0;
	}
	else
	{
		//printf("%s:result is:%s\n",pstrcommand,pstrresult);
		*pisempty = 1;
	}
	
#if DEBUG_PRINT
	printf("pstrcommand:%s\n",pstrcommand);
	printf("ppath:%s\n",ppath);
	printf("isempty:%d\n",*pisempty);
#endif	

	return ret;
}
void CFileManager::file_seek(const char *dir, int depth,FILE_INFOS *pfileinfos)   
{   
    DIR *dp = NULL;   
    struct dirent *entry = NULL;   
    struct stat statbuf;
	char pfullpath[255];

	g_depth++;

#if 0
	printf("enter file_seek----------------------------------\n");
	printf("dir:%s\n",dir);
	printf("g_dircount:%d\n",g_dircount);
	printf("g_filecount:%d\n",g_filecount);
	printf("g_depth:%d\n",g_depth);
#endif


	if(g_depth > MAX_DIR_DEPTH)
	{
		g_depth-=1;

#if 0
		printf("exit file_seek++++++++++++++++++++++++++++++++++\n");
		printf("dir:%s\n",dir);
		printf("g_dircount:%d\n",g_dircount);
		printf("g_filecount:%d\n",g_filecount);
		printf("g_depth:%d\n",g_depth);
#endif

		return;
	}

//	usleep(100);
	if( FIND_FILE == g_findtype )
	{
		if(g_filecount >= MAX_FILE_COUNT)
		{
			g_depth-=1;

#if 0
			printf("exit file_seek++++++++++++++++++++++++++++++++++\n");
			printf("dir:%s\n",dir);
			printf("g_dircount:%d\n",g_dircount);
			printf("g_filecount:%d\n",g_filecount);
			printf("g_depth:%d\n",g_depth);
#endif

			return;
		}
	}
	else if(  FIND_DIR == g_findtype) 
	{
		if(g_dircount >= MAX_DIR_COUNT)
		{
			g_depth-=1;

#if 0
			printf("exit file_seek++++++++++++++++++++++++++++++++++\n");
			printf("dir:%s\n",dir);
			printf("g_dircount:%d\n",g_dircount);
			printf("g_filecount:%d\n",g_filecount);
			printf("g_depth:%d\n",g_depth);
#endif


			return;
		}
	}
	else if( FIND_ALL == g_findtype)
	{
		if( g_dircount >= MAX_DIR_COUNT &&
			g_filecount >= MAX_FILE_COUNT)
		{
			g_depth-=1;


#if 0
			printf("exit file_seek++++++++++++++++++++++++++++++++++\n");
			printf("dir:%s\n",dir);
			printf("g_dircount:%d\n",g_dircount);
			printf("g_filecount:%d\n",g_filecount);
			printf("g_depth:%d\n",g_depth);
#endif

			return;
		}
	}
	else 
	{
		return;
	}

    if((dp = opendir(dir)) == NULL) 
	{   
        fprintf(stderr,"cannot open directory: %s\n", dir);   
		g_depth-=1;

#if 0
		printf("exit file_seek++++++++++++++++++++++++++++++++++\n");
		printf("dir:%s\n",dir);
		printf("g_dircount:%d\n",g_dircount);
		printf("g_filecount:%d\n",g_filecount);
		printf("g_depth:%d\n",g_depth);
#endif

		return;   
    }  

    chdir(dir);

    while( NULL !=(entry = readdir(dp))) 
	{   
		FILE_INFO *pfileinfo = NULL;
		
		if( FIND_FILE == g_findtype )
		{
			if(g_filecount >= MAX_FILE_COUNT)
			{
				break;
			}
		}
		else if(  FIND_DIR == g_findtype) 
		{
			if(g_dircount >= MAX_DIR_COUNT)
			{
				break;
			}
		}
		else if( FIND_ALL == g_findtype)
		{
			if( g_dircount >= MAX_DIR_COUNT &&
				g_filecount >= MAX_FILE_COUNT)
			{
				break;
			}
		}
		else 
		{
			break;
		}
		
		lstat(entry->d_name,&statbuf);

//	usleep(100);

		if(strcmp(".",entry->d_name) == 0 ||    
			strcmp("..",entry->d_name) == 0) 
		{
			continue;   
		}

		memset(pfullpath,0,255);
		strcpy(pfullpath,dir);
		strcat(pfullpath,"/");
		strcat(pfullpath,entry->d_name);


		if(S_ISREG(statbuf.st_mode))
		{
			if( FIND_FILE == g_findtype || FIND_ALL == g_findtype)
			{
				NewSpaceFileInfo(&pfileinfo);
				strcpy(pfileinfo->pfilepath,dir);
				strcat(pfileinfo->pfilepath,"/");
				strcpy(pfileinfo->pfilename,entry->d_name);
				strcpy(pfileinfo->pfilefullname,pfullpath);
				pfileinfo->filetype = FILEINFO_FILE;
				pfileinfo->filesize = statbuf.st_size;
				stat64(pfileinfo->pfilefullname,&pfileinfo->file_st);

				pfileinfos->push_back(pfileinfo);
				
				g_filecount++;
#if 0
				printf("11111111\n");
				printf("g_dircount:%d\n",g_dircount);
				printf("g_filecount:%d\n",g_filecount);
				printf("g_depth:%d\n",g_depth);
				printf("22222222\n");
#endif
	
			}
		}
		else if(S_ISDIR(statbuf.st_mode))
		{
			if( FIND_DIR == g_findtype || FIND_ALL == g_findtype)
			{
				NewSpaceFileInfo(&pfileinfo);

				strcpy(pfileinfo->pfilepath,dir);
				strcat(pfileinfo->pfilepath,"/");
				strcpy(pfileinfo->pfilename,entry->d_name);
				strcpy(pfileinfo->pfilefullname,pfullpath);
				strcat(pfileinfo->pfilefullname,"/");
				pfileinfo->filetype = FILEINFO_DIR;
				pfileinfo->filesize = statbuf.st_size;
				stat64(pfileinfo->pfilefullname,&pfileinfo->file_st);

				pfileinfos->push_back(pfileinfo);
				
				g_dircount++;

#if 0
				printf("aaaaaaaa\n");
				printf("pfullpath:%s\n",pfullpath);
				printf("g_dircount:%d\n",g_dircount);
				printf("g_filecount:%d\n",g_filecount);
				printf("g_depth:%d\n",g_depth);
				printf("bbbbbbbb\n");
#endif
				
			}

			file_seek(pfullpath,depth+4,pfileinfos);   
		}
		else
		{
			printf("unknown\n");
			continue;
		}
	}

    chdir(".."); 
    closedir(dp);   

	g_depth-=1;


#if 0
	printf("exit file_seek++++++++++++++++++++++++++++++++++\n");
	printf("dir:%s\n",dir);
	printf("g_dircount:%d\n",g_dircount);
	printf("g_filecount:%d\n",g_filecount);
	printf("g_depth:%d\n",g_depth);
#endif


	return ;
}   
//int CFileManager::FindFileByPath(const char *pfilepath, FILE_INFOS *pfileinfos, FINDFILE_TYPE findFileType, int isSubDir)
//{
//	int ret = 0;
//	
//	g_filecount = 0;
//	g_dircount = 0;
//	g_depth = 0;
//	
//	EmptyLang();	
//
//	memset(g_path,0,255);
//	strcpy(g_path,pfilepath);
//
//	if(strcmp(g_path,"/"))
//	{
//		char *ptemp = NULL;
//		ptemp = strrchr(g_path,'/');
//
//		if(NULL != ptemp) 
//		{
//
//#if SMS_DEBUG_PRINT
//			printf("%d\n",strlen(g_path));
//			printf("%d\n",ptemp - g_path);
//#endif
//
//			if((strlen(g_path)-1) == (ptemp - g_path))
//			{
//				ptemp[0] = '\0';
//			}
//		}
//	}
//
//
//#if 0
//	g_pfile_infos = pfileinfos;
//#endif
//
//	g_findtype = findFileType;
//
//#if 0
//	//ret = ftw("/home/gkm/SMS/dcps/",file_seek,0);
//	//ret = ftw("/home/gkm/SMS/dcps/",CFileManager::file_seek,0);
//	//ret = ftw("/home/gkm/SMS/dcps/",this->file_seek(),0);
//	ret = ftw(g_path,file_seek_g,0);        
//#endif
//
//#if 1
//	file_seek(g_path,0,pfileinfos);
//
//#if 0
//	printf("depth:%d\n",g_depth);
//#endif
//
//#endif
//	
//	return ret;
//}
int CFileManager::GetLocalPath(char *ppath,int maxlen)
{
	int ret = 0;
	FILE *pf = NULL;
	char pstrcommand[255];
	char pstrresult[255];

	if(NULL == ppath)
	{
		return PARAMETER_ERROR;
	}

	memset(ppath,0,maxlen);

	pf = NULL;
	memset(pstrcommand,0,sizeof(pstrcommand));
	memset(pstrresult,0,sizeof(pstrresult));

	sprintf(pstrcommand,"pwd");

	pf = popen(pstrcommand,"r");
	if(NULL == pf)
	{
		printf("%s:command executed failed\n",pstrcommand);
		return ret;
	}

	while(NULL != fgets(pstrresult,sizeof(pstrresult),pf))
	{
		if('\n' == pstrresult[strlen(pstrresult)-1])
		{
				pstrresult[strlen(pstrresult)-1] = '\0';
		}
		
#if DEBUG_PRINT
		if(NULL != pstrresult)
		{
			printf("%s=>%s\n",pstrcommand,pstrresult);
		}
#endif 		
		
	}

	if(-1 == pclose(pf))
	{
#if 0
		printf("%s:command closed failed\n",pstrcommand);
#endif
	}

	if( NULL == pstrresult ||
		(!strcmp(pstrresult,"")))
	{
		printf("%s:result is null\n",pstrcommand);
	}	
	else
	{
		//printf("%s:result is:%s\n",pstrcommand,pstrresult);
		strcpy(ppath,pstrresult);
	}

	return ret;
}
int CFileManager::GetFileInfo(const char *pfileFullName, FILE_INFO *pfileinfo)
{
	int ret = 0;

	if( NULL ==pfileFullName ||
		(!strcmp("",pfileFullName)) ||
		NULL == pfileinfo)
	{
		return ERROR_INPUT_PARMETER_INVALID;
	}

	ClearSpaceFileInfo(pfileinfo);

	ret = stat64( pfileFullName, &pfileinfo->file_st );

	if(ret)
	{
		return ERROR_FILE_NOT_EXIST;
	}


	sprintf(pfileinfo->pfilefullname,"%s",pfileFullName);

	GetFilePath(pfileinfo->pfilefullname,1,pfileinfo->pfilepath,255);
	GetFileName(pfileinfo->pfilefullname,1,pfileinfo->pfilename,255);

	pfileinfo->filesize = pfileinfo->file_st.st_size;

#if 0
	if(S_ISREG(pfileinfo->file_st.st_mode))
	{
		pfileinfo->filetype = FILEINFO_FILE;
	}
	else if(S_ISDIR(pfileinfo->file_st.st_mode))
	{
		pfileinfo->filetype = FILEINFO_DIR;

	}
	else if(S_ISLNK(pfileinfo->file_st.st_mode))
	{
		pfileinfo->filetype = FILEINFO_LINK;
	}
	else
	{
		pfileinfo->filetype = FILEINFO_OTHRER;
	}
#endif

	if((pfileinfo->file_st.st_mode&S_IFREG)==S_IFREG)
	{
		pfileinfo->filetype = FILEINFO_FILE;
	}
	else if((pfileinfo->file_st.st_mode&S_IFDIR)==S_IFDIR)
	{
		pfileinfo->filetype = FILEINFO_DIR;

	}
	else if((pfileinfo->file_st.st_mode&S_IFLNK)==S_IFLNK)
	{
		pfileinfo->filetype = FILEINFO_LINK;
	}
	else
	{
		pfileinfo->filetype = FILEINFO_OTHRER;
	}

	return 0;
}
int CFileManager::GetFilePath(const char *pfileFullName,int type,char *pfilePath,int maxlen_filepath)
{
	int iRet = 0;
	char separator;
	char buff_temp[255];
	char *ptemp = NULL;
	strcpy(buff_temp,pfileFullName);

	if( NULL == pfileFullName ||
		NULL == pfilePath)
	{
		return iRet;
	}

	if(!type)
	{
		separator = '\\';
	}
	else
	{
		separator = '/';
	}

	memset(pfilePath,0,maxlen_filepath);

	if((strlen(pfileFullName)==1) &&
		(pfileFullName[0] == separator))
	{
		strcpy(pfilePath,pfileFullName);
		return iRet;
	}

	SubtractSeparator(buff_temp,1,1);

	ptemp = strrchr(buff_temp,separator);

	if(NULL != ptemp)
	{
		*ptemp = '\0';
		sprintf(pfilePath,"%s%c",buff_temp,separator);
	}

	return iRet;
}
int CFileManager::GetFileName(const char *pfileFullName,int type,char *pfileName,int maxlen_filename)
{
	int iRet = 0;
	char buff_temp[255];
	char separator;
	char *ptemp = NULL;

	if( NULL == pfileFullName ||
		NULL == pfileName)
	{
		return iRet;
	}

	if(!type)
	{
		separator = '\\';
	}
	else
	{
		separator = '/';
	}

	memset(pfileName,0,maxlen_filename);

	if(( 1== strlen(pfileFullName)) &&
		(pfileFullName[0] == separator))
	{
		return iRet;
	}

	strcpy(buff_temp,pfileFullName);

	SubtractSeparator(buff_temp,1,1);

	ptemp = strrchr(buff_temp,separator);

	if(NULL != ptemp)
	{
		ptemp++;
		strcpy(pfileName,ptemp);
	}
	else
	{
		strcpy(pfileName,pfileFullName);
	}

	return iRet;
}
int CFileManager::GetFilterString(const char *strSrc, char *strFilter, char *strDest)
{
	int ret = 0;
	return ret;
}
int CFileManager::GetFileSize(const char *pfullpath,long long *pfilesize)
{
	int ret = 0;
	struct stat64 filedata;

	ret = stat64( pfullpath, &filedata );

	if(-1 == ret)
	{
		return ret;
	}

	*pfilesize = filedata.st_size;

#if 0
	printf("path:%s\nsize:%lld\n",m_sourcefullpath,*pfilesize);
#endif


	return ret;
}
int CFileManager::GetFileSizeS(const char *pfullpath,char *psize,int maxlen)
{
	int ret = 0;
	long long filesize = 0;

	if( NULL == pfullpath||
		NULL == psize)
	{
		return 1;
	}

	memset(psize,0,maxlen);
	
	ret = GetFileSize(pfullpath,&filesize);

	if(ret)
	{
		return ret;
	}

	sprintf(psize,"%lld",filesize);

	return ret;
}
int CFileManager::GetDiskInfo_S(const char *pdiskdir,
				char *pdisksize,
				int maxlen_disksize,
				char *pdiskused,
				int maxlen_diskused,
				char *pdiskavail,
				int maxlen_diskavail,
				char *pdiskusedpercent,
				int maxlen_diskusedpercent
				)
{
	int ret = 0;   

	if( NULL == pdiskdir ||
		NULL == pdisksize || 
		maxlen_disksize < 0 ||
		NULL == pdiskused || 
		maxlen_diskused < 0 ||
		NULL == pdiskavail || 
		maxlen_diskavail < 0 ||
		NULL == pdiskusedpercent || 
		maxlen_diskusedpercent < 0)

	{
		return ret;
	}


#if 0
	struct statfs sf;

	statfs("/",&sf);

	printf("struct statfs:f_bavail:%lld\n",sf.f_bavail);
#endif

	char diskinfo[5][50];
	int m = 0;
	int n = 0;

       	
	char pstrcommand[255];
    char pstrresult[255];
    FILE *pf = NULL;


	memset(pdisksize,0,maxlen_disksize);
	memset(pdiskused,0,maxlen_diskused);
	memset(pdiskavail,0,maxlen_diskavail);
	memset(pdiskusedpercent,0,maxlen_diskusedpercent);


	memset(pstrcommand,0,sizeof(pstrcommand));
	memset(pstrresult,0,sizeof(pstrresult));

//	strcpy(pstrcommand,"df -lh ");
	strcpy(pstrcommand,"df -h ");
	strcat(pstrcommand,pdiskdir);


    pf = popen(pstrcommand,"r");
    if(NULL == pf)
    {
		printf("%s:command executed failed\n",pstrcommand);
		return 1;
    }

    while(NULL != fgets(pstrresult,sizeof(pstrresult),pf))
    {
        if('\n' == pstrresult[strlen(pstrresult)-1])
        {
                pstrresult[strlen(pstrresult)-1] = '\0';
        }

#if DEBUG_PRINT
		if(NULL != pstrresult)
		{
			printf("%s=>%s\n",pstrcommand,pstrresult);
		}
#endif 
	}

    if(-1 == pclose(pf))
    {
#if 0 	
			printf("%s:command closed failed\n",pstrcommand);
#endif			
    }

	memset(diskinfo,0,sizeof(diskinfo));

	if( NULL == pstrresult ||
		(!strcmp(pstrresult,"")))
	{
		printf("%s:result is null\n",pstrcommand);
	}
	else
	{
		for(int i = 0; i<strlen(pstrresult); i++)
		{
			if(isblank(pstrresult[i]))
			{
				continue;
			}

			diskinfo[m][n] = pstrresult[i];
			n++;

			if(isblank(pstrresult[i+1]))
			{
				m++;
				n=0;
			}		

		}	
	
		//printf("%s:result is:%s\n",pstrcommand,pstrresult);
	}

	strcpy(pdisksize,diskinfo[1]);
	strcpy(pdiskused,diskinfo[2]);
	strcpy(pdiskavail,diskinfo[3]);
	strcpy(pdiskusedpercent,diskinfo[4]);
	
#if DEBUG_PRINT
	printf("pstrcommand:%s\n",pstrcommand);
	printf("pdiskdir:%s\n",pdiskdir);
	printf("pdisksize:%s\n",pdisksize);
	printf("pdiskused:%s\n",pdiskused);
	printf("pdiskavail:%s\n",pdiskavail);
	printf("pdiskusedpercent:%s\n",pdiskusedpercent);
#endif	
	
	
	return ret;
}
int CFileManager::GetMaxUnitSize(char *pmaxunit,int maxlen_maxunit,long long maxsize)
{
	int ret = 0;
	int index = 0;
	float maxunit = 0.0;
		
	while(maxsize >= 1024)
	{
		maxunit = maxsize / 1024.0;
		maxsize = (long long)maxunit;
		index++;
	}

	memset(pmaxunit,0,maxlen_maxunit);

	if( 0 == index )
	{
		sprintf(pmaxunit,"%4.2fByte",maxunit);
	}
	else if( 1 == index )
	{
		sprintf(pmaxunit,"%4.2fK",maxunit);
	}
	else if( 2 == index )
	{
		sprintf(pmaxunit,"%4.2fM",maxunit);
	}
	else if( 3 == index )
	{
		sprintf(pmaxunit,"%4.2fG",maxunit);
	}
	else
	{
		sprintf(pmaxunit,"%4.2fT",maxunit);
	}


	return ret;
}
bool CFileManager::IsExistFile(const char *pfullpath)
{
	int ret = 0;
	struct stat64 filedata;

	ret = stat64( pfullpath, &filedata );

	if( 0 == ret )
	{
		ret = 1;
	}
	else
	{
		ret = 0;
	}

	return ret;
}
int CFileManager::CorrectPathString(char *ppath,int maxlen,int type)
{
	int ret = 0;
	int len = 0;
	char separator;
	int index = 0;
	int bhave_separator = 0;
	char pbuff_path[255];

	if( NULL == ppath )
	{
		return 1;
	}

	if(!type)
	{
		separator = '\\';
	}
	else
	{
		separator = '/';
	}


	memset(pbuff_path,0,255);

	len = strlen(ppath);
	bhave_separator = 0;

	for( int i = 0; i < len; i++)
	{
		if(ppath[i] == separator)
		{
			if(!bhave_separator)
			{
				pbuff_path[index++] = separator;
				bhave_separator = 1;
			}
		}
		else
		{
			pbuff_path[index++] = ppath[i];
			bhave_separator = 0;
		}
	}

	memset(ppath,0,maxlen);

	strcpy(ppath,pbuff_path);

	return ret;
}
int CFileManager::GeneratePathString(char *ppath,int type,int maxlen,const char *pathmsg,...)
{
	int ret = 0;
	va_list argp;
	char pbuff_path[255];
	char pbuff_temp[255];
	int index = 0;
	char *para = NULL;
	int argno = 0;

	if( NULL == ppath ||
		NULL == pathmsg)
	{
		return 1;
	}
	
//	memset(ppath,0,maxlen);

	memset(pbuff_path,0,255);
	memset(pbuff_temp,0,255);

	if(strcmp(pathmsg,""))
	{
		strcpy(pbuff_path,pathmsg);
		AddSeparator(pbuff_path,1,type);

		strcpy(&pbuff_temp[index],pbuff_path);

		index = strlen(pbuff_temp);	
	}

	va_start(argp,pathmsg);

	while(argno++ < 20)
	{
		para = va_arg(argp,char *);

		if(!strcmp(para,""))
		{
			continue;
		}

		if(!strcmp(para,"args_end"))
		{
			break;
		}

		memset(pbuff_path,0,255);
		strcpy(pbuff_path,para);
		AddSeparator(pbuff_path,1,type);

		strcpy(&pbuff_temp[index],pbuff_path);

		index = strlen(pbuff_temp);
	}

	va_end(argp);

	SubtractSeparator(pbuff_temp,1,type);

	ret = CorrectPathString(pbuff_temp,maxlen,type);

	memset(ppath,0,maxlen);

	strcpy(ppath,pbuff_temp);

#if 0
	printf("ppath:%s",ppath);
#endif

	return ret;
}
int CFileManager::AddSeparator(char *ppath,int flag,int type)
{
	int ret = 0;
	int len = 0;
	char separator;
	char pbuff_path[255];

//#ifdef _WIN32
//#else
//#endif	
	if(!type)
	{
		separator = '\\';
	}
	else
	{
		separator = '/';
	}

	ret = SubtractSeparator(ppath,flag,type);

	len = strlen(ppath);

	memset(pbuff_path,0,255);

	if( 0 == flag)
	{
		pbuff_path[0] = separator;
		memcpy(&pbuff_path[1],ppath,len);
	}
	else if( 1 == flag )
	{
		memcpy(pbuff_path,ppath,len);
		pbuff_path[len] = separator;

	}
	else
	{
		pbuff_path[0] = separator;
		memcpy(&pbuff_path[1],ppath,len);
		pbuff_path[len] = separator;
	}

	memset(ppath,0,len);
	strcpy(ppath,pbuff_path);

	return ret;

}
int CFileManager::SubtractSeparator(char *ppath,int flag,int type)
{
	int ret = 0;
	int len = 0;
	char separator;
	int index = 0;
	char pbuff_path[255];

	if( NULL == ppath )
	{
		return 1;
	}

	len = strlen(ppath);

//#ifdef _WIN32
//#else
//#endif	
	if(!type)
	{
		separator = '\\';
	}
	else
	{
		separator = '/';
	}

	if( 0 == flag)
	{
		index = 0;	/// 从路径起始位置向后查找
	}
	else if( 1 == flag )
	{
		index = strlen(ppath) -1; /// 从路径结束位置向前查找
	}
	else
	{
		ret = SubtractSeparator(ppath,0,type);
		ret = SubtractSeparator(ppath,1,type);
		return ret;
	}

	while(ppath[index] == separator)
	{
		if( 0== flag)
		{
			index++;
		}
		else
		{
			index--;
		}
	}

	memset(pbuff_path,0,255);

	if( 0 == flag )
	{
		memcpy(pbuff_path,&ppath[index],len - index);
	}
	else
	{
		memcpy(pbuff_path,ppath,index + 1);
	}

	memset(ppath,0,len);

	strcpy(ppath,pbuff_path);


	return ret;

}
int CFileManager::ConrrectToHaveESCPathString(char *ppath,int maxlen,int type)
{
	int ret = 0;
	int len = 0;
	char escchar = '\\';
	char separator;
	int index = 0;
	int bhave_separator = 0;
	char pbuff_path[255];

	if( NULL == ppath )
	{
		return 1;
	}

	if(!type)
	{
		separator = '\\';
	}
	else
	{
		separator = '/';
	}

	memset(pbuff_path,0,255);

	len = strlen(ppath);
	bhave_separator = 0;

	for( int i = 0; i < len; i++)
	{
		if(ppath[i] == separator)
		{
			if(!bhave_separator)
			{
				pbuff_path[index++] = escchar;
				pbuff_path[index++] = ppath[i];
				bhave_separator = 1;
			}
		}
		else
		{
			pbuff_path[index++] = ppath[i];
			bhave_separator = 0;
		}
	}

	memset(ppath,0,maxlen);

	strcpy(ppath,pbuff_path);

	return ret;
}
int CFileManager::GetPostFixInFileName(char *ppostfix,int maxlen,const char *pfilename)
{
	int ret = 0;
	char *ptemp = NULL;
	char separator = '.';
	char buff[255];

	if(NULL == ppostfix ||
		NULL == pfilename)
	{
		return 1;
	}

	memset(buff,0,sizeof(buff));
	strcpy(buff,pfilename);

//	ptemp = memrchr(buff,separator,strlen(pfilename));
	ptemp = strrchr(buff,separator);

	if( NULL != ptemp)
	{
		memset(ppostfix,0,maxlen);
		strcpy(ppostfix,ptemp);
	}

	return ret;
}
int CFileManager::GetNameInFileName(char *pname,int maxlen,const char *pfilename)
{
	int ret = 0;
	char *ptemp = NULL;
	char separator = '.';
	char buff[255];

	if(NULL == pname ||
		NULL == pfilename)
	{
		return 1;
	}

	memset(buff,0,sizeof(buff));
	strcpy(buff,pfilename);

//	ptemp = memrchr(buff,separator,strlen(pfilename));
	ptemp = strrchr(buff,separator);

	if( NULL != ptemp)
	{
		memset(pname,0,maxlen);
		memcpy(pname,pfilename,ptemp - pfilename);
	}

	return ret;
}
int CFileManager::NewSpaceFileInfo(FILE_INFO **pfileinfo)
{
	int ret = 0;

	if( NULL != *pfileinfo)
	{
		return ret;
	}

	*pfileinfo = new FILE_INFO;

	(*pfileinfo)->pfilefullname = new char[255];
	(*pfileinfo)->pfilename = new char[255];
	(*pfileinfo)->pfilepath = new char[255];

	ClearSpaceFileInfo(*pfileinfo);


	return ret;
}
int CFileManager::DeleteSpaceFileInfo(FILE_INFO **pfileinfo)
{
	int ret = 0;

	if( NULL == *pfileinfo)
	{
		return ret;
	}

	ClearSpaceFileInfo(*pfileinfo);

	if( NULL != (*pfileinfo)->pfilefullname)
	{
		delete [] (*pfileinfo)->pfilefullname;
		(*pfileinfo)->pfilefullname = NULL;
	}

	if( NULL != (*pfileinfo)->pfilename)
	{
		delete [] (*pfileinfo)->pfilename;
		(*pfileinfo)->pfilename = NULL;
	}

	if( NULL != (*pfileinfo)->pfilepath)
	{
		delete [] (*pfileinfo)->pfilepath;
		(*pfileinfo)->pfilepath = NULL;
	}

	if( NULL != *pfileinfo)
	{
		delete *pfileinfo;
		*pfileinfo = NULL;
	}

	return ret;
}
int CFileManager::ClearSpaceFileInfo(FILE_INFO *pfileinfo)
{
	int ret = 0;
	if( NULL == pfileinfo)
	{
		return ret;
	}

	memset(pfileinfo->pfilefullname,0,255);
	memset(pfileinfo->pfilename,0,255);
	memset(pfileinfo->pfilepath,0,255);

	pfileinfo->filetype = FILEINFO_FILE;
	pfileinfo->filesize = 0;
	memset(&pfileinfo->file_st,0,sizeof(pfileinfo->file_st));

	return ret;

}
int CFileManager::DeleteSpaceFileInfos(FILE_INFOS *pfileinfos)
{
	int ret = 0;

	if(NULL == pfileinfos)
	{
		return ret;
	}

	if(pfileinfos->size() > 0)
	{
		for(int i = 0; i<pfileinfos->size(); i++)
		{
			FILE_INFO *ptemp = (FILE_INFO *)((*pfileinfos)[i]);
			DeleteSpaceFileInfo(&ptemp);
		}

		pfileinfos->clear();
	}

	return ret;
}
//int CFileManager::DeleteFIle_M(const char *pfilefullpath)
//{
//	int ret = 0;
//
//	ret = remove(pfilefullpath);
//
//	return ret;
//}
//int CFileManager::CopyFile_M(const char *pdestfullpath,const char *psrcfullpath)
//{
//	int ret = 0;
//	int bisexist_src = 0;
//	
//	if(m_bcopying)
//	{
//		printf("copy is running already\n");
//		return 0;
//	}	
//	
//	m_bcopying = 1;
//	m_copy_state = COPY_COPYING;
//	m_copy_control_stop = 0;
//
//	memset(m_descfullpath,0,sizeof(m_descfullpath));
//	memset(m_sourcefullpath,0,sizeof(m_sourcefullpath));
//
//	strcpy(m_descfullpath,pdestfullpath);
//	strcpy(m_sourcefullpath,psrcfullpath);
//	
//	bisexist_src = IsExistFile(psrcfullpath);
//	if(!bisexist_src)
//	{
//		m_bcopying = 0;
//		m_copy_state = COPY_FINISHED_ERROR;
//		m_copy_control_stop = 0;
//		return -1;
//	}
//	
//	long long size_copyingfile = 0;
//	ret = GetFileSize(psrcfullpath,&size_copyingfile);
//	if(0 == size_copyingfile)
//	{
//		m_bcopying = 0;
//		m_copy_state = COPY_FINISHED_ERROR;
//		m_copy_control_stop = 0;
//		return -2;
//	}
//
//	pthread_attr_t attr;
//	unsigned long pthread_copy_proc;
//	pthread_attr_init(&attr);
//    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
//	ret = pthread_create(&pthread_copy_proc, &attr, copy_thread, this);
////	ret = pthread_create(&pthread_copy_proc, NULL, copy_thread, this);
//	pthread_attr_destroy (&attr);
//
//
//    m_time_elapsed_start_real= time(&m_time_elapsed_start_real);
//	if( ret )
//	{
//		m_bcopying = 0;
//		m_copy_state = COPY_FINISHED_ERROR;
//		m_copy_control_stop = 0;
//
//		printf("file copy procedure thread create failed:%d\n",ret);
//
//		return -1;
//	}
//	else
//	{
//		int runningcounter = 0;
//		while(1)
//		{
//			//printf("stopping:m_isplaying:%d\n",m_isplaying);
//	
//			if(m_isrunning || 
//				(!m_bcopying))
//			{
//				break;
//			}
//	
//			runningcounter++;
//	
//			if(runningcounter > 100)
//			{
//				break;
//			}
//			msleep(100);
//		}
//	
//		printf("copy timeout:%d,100\n",runningcounter);	
//	}
//
//
//	return 0;
//}
//int CFileManager::CopyFile_M1(const char *pdestfullpath,const char *psrcfullpath)
//{
//	int ret = 0;
//	char *pdatabuff = NULL;
//	int unit_copy_size = 0;
//	long long destfile_size = 0;
//	long long size_copyingfile = 0;
//	long long size_copiedelapsed = 0;
//
//	if( NULL == pdestfullpath ||
//		NULL == psrcfullpath ||
//		(!strcmp(pdestfullpath,"")) ||
//		(!strcmp(psrcfullpath,"")) )
//	{
//		return 1;
//	}
//
//	if(!IsExistFile(psrcfullpath))
//	{		
//		return -1;
//	}
//
//	pdatabuff = new char[UNIT_COPY_SIZE];
//
//
//	GetFileSize(psrcfullpath,&size_copyingfile);
//	if(0 == size_copyingfile)
//	{
//		return -2;
//	}
//
//#if 0
//	printf("enter copy\n");
//	printf("src path:%s\nsize:%lld\n",psrcfullpath,size_copyingfile);
//	printf("dest path:%s\n",pdestfullpath);
//#endif
//
//
//	FILE *pf_desc = NULL;
//	FILE *pf_src = NULL;
//
//	if(IsExistFile(pdestfullpath))
//	{
//#if 0			
//		DeleteDirOrFile(pdestfullpath);
//#endif
//
//		GetFileSize(pdestfullpath,&size_copiedelapsed);
//
//		pf_desc = fopen(pdestfullpath,"ab+");
//
//#if SMS_DEBUG_PRINT
//		printf("file is exist:%lld\n",size_copiedelapsed);
//#endif
//
//		pf_src = fopen(psrcfullpath,"rb");
//		if(NULL == pf_src)
//		{
//			delete [] pdatabuff;
//			pdatabuff = NULL;
//
//			return -1;
//		}	
//		
//		ret = fseek(pf_src,size_copiedelapsed,SEEK_SET);
//		if(ret)
//		{
//			printf("seek error\n");
//
//			delete [] pdatabuff;
//			pdatabuff = NULL;
//
//			if(NULL != pf_src)
//			{
//				ret = fclose(pf_src);
//				pf_src = NULL;
//			}
//
//			return -1;			
//		}
//	}
//	else
//	{
//		pf_desc = fopen(pdestfullpath,"wb");
//		pf_src = fopen(psrcfullpath,"rb");
//	}
//
//
//	while( size_copiedelapsed < size_copyingfile)
//	{
//		unit_copy_size = UNIT_COPY_SIZE;
//		unit_copy_size = fread(pdatabuff,1,unit_copy_size,pf_src);
//		if(unit_copy_size <= 0)
//		{
//			delete [] pdatabuff;
//			pdatabuff = NULL;
//
//			if(NULL != pf_src)
//			{
//				ret = fclose(pf_src);
//				pf_src = NULL;
//			}
//
//			if(NULL != pf_desc)
//			{
//				ret = fclose(pf_desc);
//				pf_desc = NULL;
//			}
//
//			return ret;
//		}
//
//
//		ret = fwrite(pdatabuff,1,unit_copy_size,pf_desc);
//		if(ret <= 0)
//		{
//
//			delete [] pdatabuff;
//			pdatabuff = NULL;
//
//			if(NULL != pf_src)
//			{
//				ret = fclose(pf_src);
//				pf_src = NULL;
//			}
//
//			if(NULL != pf_desc)
//			{
//				ret = fclose(pf_desc);
//				pf_desc = NULL;
//			}
//
//			return 1;
//		}
//
//
//		size_copiedelapsed += unit_copy_size;
//	}
//
//
//	if(NULL != pf_src)
//	{
//		ret = fclose(pf_src);
//		pf_src = NULL;
//	}
//
//	if(NULL != pf_desc)
//	{
//		ret = fclose(pf_desc);
//		pf_desc = NULL;
//	}
//
//	delete [] pdatabuff;
//	pdatabuff = NULL;
//	
//#if 0
//	msleep(1000);
//#endif	
//	
//#if 0
//	printf("exit copy\n");
//#endif
//
//	return ret;
//}
//int CFileManager::StopCopyFile_M()
//{
//	int ret = 0;
//	m_copy_control_stop = 1;
//
//#if defined(_WIN32)
//	Sleep(1000);
//#else
//	sleep(1);
//#endif
//
//	return ret;
//}
//int CFileManager::MoveFile_M(const char *pdestfullpath,const char *psrcfullpath)
//{
//	int ret = 0;
//	return ret;
//}
#if 0
/// 边拷贝边hash校验
int CFileManager::copy_procedure()
{
	int ret = 0;
	char *pdatabuff = NULL;
	int unit_copy_size = 0;
	long long destfile_size = 0;
	CHashUuidTool hashuuid_tool;
	int copyingspeed = 0;

///////////////
	memset(m_buff_filehash,0,sizeof(m_buff_filehash));
	m_len_filehash = 0;
	m_size_copyingfile = 0;
	m_size_copiedelapsed = 0;
	m_copy_control_stop = 0;	
///////////////


	pdatabuff = new char[UNIT_COPY_SIZE];

	GetFileSize(m_sourcefullpath,&m_size_copyingfile);

#if SMS_DEBUG_PRINT
	printf("enter copy\n");
	printf("src path:%s\nsize:%lld\n",m_sourcefullpath,m_size_copyingfile);
	printf("dest path:%s\n",m_descfullpath);
#endif


	time(&m_time_copystart);
	hashuuid_tool.InitHash();


	FILE *pf_desc = NULL;
	FILE *pf_src = NULL;

	if(IsExistFile(m_descfullpath))
	{
		GetFileSize(m_descfullpath,&destfile_size);

		pf_desc = fopen(m_descfullpath,"ab+");

#if SMS_DEBUG_PRINT
		printf("file is exist:%lld\n",destfile_size);
#endif

		while( m_size_copiedelapsed < destfile_size)
		{
			unit_copy_size = UNIT_COPY_SIZE;
			unit_copy_size = fread(pdatabuff,1,unit_copy_size,pf_desc);
			hashuuid_tool.CalcHash(pdatabuff,unit_copy_size);

			m_size_copiedelapsed += unit_copy_size;

			if(m_copy_control_stop)
			{
#if SMS_DEBUG_PRINT
				printf("copying is stopped\n");
#endif

				break;
			}


		}

		pf_src = fopen(m_sourcefullpath,"rb");
		ret = fseek(pf_src,m_size_copiedelapsed,SEEK_SET);
		if(ret)
		{
			printf("seek error\n");
		}

#if SMS_DEBUG_PRINT
			printf("middle hash is ok:%lld\n",m_size_copiedelapsed);
#endif

	}
	else
	{
		pf_desc = fopen(m_descfullpath,"wb");
		pf_src = fopen(m_sourcefullpath,"rb");
	}



	while( m_size_copiedelapsed < m_size_copyingfile)
	{
		if(m_copy_control_stop)
		{

#if SMS_DEBUG_PRINT
			printf("copying is stopped\n");
#endif
			break;
		}


#if 1
		unit_copy_size = UNIT_COPY_SIZE;
		unit_copy_size = fread(pdatabuff,1,unit_copy_size,pf_src);
		ret = fwrite(pdatabuff,1,unit_copy_size,pf_desc);

		hashuuid_tool.CalcHash(pdatabuff,unit_copy_size);

		m_size_copiedelapsed += unit_copy_size;

#endif


	}


	if(NULL != pf_src)
	{
		ret = fclose(pf_src);
		pf_src = NULL;
	}

	if(NULL != pf_desc)
	{
		ret = fclose(pf_desc);
		pf_desc = NULL;
	}

	hashuuid_tool.StopHash();

	hashuuid_tool.GetHash(m_buff_filehash,&m_len_filehash);


#if SMS_DEBUG_PRINT
	printf("exit copy hash:%s\nlen:%d\n",m_buff_filehash,m_len_filehash);
#endif


	delete [] pdatabuff;
	pdatabuff = NULL;
	
	m_copy_state = COPY_FINISHED;

	return ret;

}
#endif

#define SECONDE_INCREASE	1

//int CFileManager::copy_procedure()
//{
//	int ret = 0;
//	char *pdatabuff = NULL;
//	int unit_copy_size = 0;
//	int monitor_counter = 0;	/// 监控计数  每2G打印一次
//    int elapsed_seconds = 0;  ///gkm add 20141216
//	char buff_temp[255]={'\0'};
//
//     /// gkm add 20141215 start
//    struct timeval tv;
//    fd_set readfds;
//    tv.tv_sec = 1;
//    tv.tv_usec = 0;////1000000
//     /// gkm add 20141215 end
//
//	m_copy_state = COPY_COPYING;
//	m_size_copyingfile = 0;
//	m_size_copiedelapsed = 0;
//	m_copy_control_stop = 0;	
/////////////////
//	m_thread_isexist = 1;
//
//	pdatabuff = new char[UNIT_COPY_SIZE];
//
//	ret = GetFileSize(m_sourcefullpath,&m_size_copyingfile);
//	
//	if(ret)
//	{
//		m_copy_state = COPY_FINISHED_ERROR;
//		m_bcopying = 0;
//		m_thread_isexist = 0;
//
//		delete [] pdatabuff;
//		pdatabuff = NULL;
//
//
//		return 1;
//	}
//
//	m_isrunning = 1;
//
//
//#if 1
//	printf("copy enter\n");
//	printf("src path:%s\nsize:%lld\n",m_sourcefullpath,m_size_copyingfile);
//	printf("dest path:%s\n",m_descfullpath);
//#endif
//
//#if 1
//	int lwpid;
//	lwpid=syscall(SYS_gettid);
//	memset(buff_temp, 0, sizeof(buff_temp));
//	sprintf(buff_temp, "dddddd:file copy procedure enter,lwpid:%d\n",lwpid);
//	g_boot_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
//#endif     
//
//
//	FILE *pf_desc = NULL;
//	FILE *pf_src = NULL;
//
//	if(IsExistFile(m_descfullpath))
//	{
//		GetFileSize(m_descfullpath,&m_size_copiedelapsed);
//
//		pf_desc = fopen(m_descfullpath,"ab+");
//
//#if SMS_DEBUG_PRINT
//		printf("file is exist:%lld\n",m_size_copiedelapsed);
//#endif
//
//		pf_src = fopen(m_sourcefullpath,"rb");
//		if(NULL == pf_src)
//		{
//			m_copy_state = COPY_FINISHED_ERROR;
//			m_bcopying = 0;
//			m_isrunning = 0;
//			m_thread_isexist = 0;
//
//			delete [] pdatabuff;
//			pdatabuff = NULL;
//
//			return 1;
//		}	
//		
//		ret = fseek(pf_src,m_size_copiedelapsed,SEEK_SET);
//		if(ret)
//		{
//			m_copy_state = COPY_FINISHED_ERROR;
//			printf("seek error\n");
//			m_bcopying = 0;
//			m_isrunning = 0;
//			m_thread_isexist = 0;
//
//			delete [] pdatabuff;
//			pdatabuff = NULL;
//
//			if(NULL != pf_src)
//			{
//				ret = fclose(pf_src);
//				pf_src = NULL;
//			}
//
//			return 1;			
//		}
//
//	}
//	else
//	{
//		pf_desc = fopen(m_descfullpath,"wb");
//		if(NULL == pf_desc)
//		{
//			m_copy_state = COPY_FINISHED_ERROR;
//			m_bcopying = 0;
//			m_isrunning = 0;
//			m_thread_isexist = 0;
//
//			delete [] pdatabuff;
//			pdatabuff = NULL;
//
//			return 1;
//		}	
//		pf_src = fopen(m_sourcefullpath,"rb");
//		if(NULL == pf_src)
//		{
//			m_copy_state = COPY_FINISHED_ERROR;
//			m_bcopying = 0;
//			m_isrunning = 0;
//			m_thread_isexist = 0;
//
//			delete [] pdatabuff;
//			pdatabuff = NULL;
//
//			if(NULL != pf_desc)
//			{
//				ret = fclose(pf_desc);
//				pf_desc = NULL;
//			}
//
//			return 1;
//		}	
//	}
//
//    /// gkm add 20141216 start
//    time_t time_current = 0;
//    m_size_copiedelapsed_real = 0;
//    m_time_elapsed_start_real= time(&m_time_elapsed_start_real);
//    /// gkm add 20141216 end
//
//	int fddest = 0;
//	fddest = fileno(pf_desc);
//
//#if SECONDE_INCREASE
//	int limit_counter = 0;
//
//#ifdef _WIN32
//	struct _timeb tb_start;
//	struct _timeb tb_end;
//
//	_ftime(&tb_start);
//#else
//	struct timeval tv_start;
//	struct timeval tv_end;
//	struct timezone tz;
//	gettimeofday(&tv_start,&tz);
//#endif
//#endif
//
//	while( m_size_copiedelapsed < m_size_copyingfile)
//	{
//		if(m_copy_control_stop)
//		{
//			m_copy_state = COPY_STOP;
//			m_bcopying = 0;
//
//#if 1
//			printf("copying is stopped\n");
//#endif
//			break;
//		}
//
//#if 0
//		////////////
//		time_t time1;
//		time(&time1);
//		printf("aaaaaaaaaaaaaaaaaaaa,time1:%d\n",time1);
//		printf("usb m_wait_loop_counter:%d\n",m_wait_loop_counter);
//		/////////////
//#endif
//
//#if !SECONDE_INCREASE
//#ifdef _WIN32
//		struct _timeb tb_start;
//		struct _timeb tb_end;
//
//		_ftime(&tb_start);
//#else
//		struct timeval tv_start;
//		struct timeval tv_end;
//		struct timezone tz;
//		gettimeofday(&tv_start,&tz);
//#endif
//#endif
//
//        for(int i=0; i<m_wait_loop_counter; i++)
//        {
//
//            if(m_copy_control_stop)
//            {
//                m_copy_state = COPY_STOP;
//                m_bcopying = 0;
//
//#if 1
//                printf("copying is stopped\n");
//#endif
//                break;
//            }
//
//			unit_copy_size = UNIT_COPY_SIZE;
//			unit_copy_size = fread(pdatabuff,1,unit_copy_size,pf_src);
//			if(unit_copy_size <= 0)
//			{
//				m_copy_state = COPY_FINISHED_ERROR;
//				m_bcopying = 0;
//				m_isrunning = 0;
//				m_thread_isexist = 0;
//
//				delete [] pdatabuff;
//				pdatabuff = NULL;
//
//				if(NULL != pf_src)
//				{
//					ret = fclose(pf_src);
//					pf_src = NULL;
//				}
//
//				if(NULL != pf_desc)
//				{
//					ret = fclose(pf_desc);
//					pf_desc = NULL;
//				}
//
//				/// gkm add 20141216 start
//				m_size_copiedelapsed_real = 0;
//				/// gkm add 20141216 end
//
//
//				return 1;
//			}
//		
//			ret = fwrite(pdatabuff,1,unit_copy_size,pf_desc);
//			if(ret <= 0)
//			{
//				m_copy_state = COPY_FINISHED_ERROR;
//				m_bcopying = 0;
//				m_isrunning = 0;
//				m_thread_isexist = 0;
//
//				delete [] pdatabuff;
//				pdatabuff = NULL;
//
//				if(NULL != pf_src)
//				{
//					ret = fclose(pf_src);
//					pf_src = NULL;
//				}
//
//				if(NULL != pf_desc)
//				{
//					ret = fclose(pf_desc);
//					pf_desc = NULL;
//				}
//
//				/// gkm add 20141216 start
//				m_size_copiedelapsed_real = 0;
//				/// gkm add 20141216 end
//
//				return 1;
//			}
//
//			m_size_copiedelapsed += unit_copy_size;
//
//			m_size_copiedelapsed_real += unit_copy_size;
//			if( m_size_copiedelapsed >=  m_size_copyingfile)
//			{
//				break;
//			}
//
//			monitor_counter++;
//        }/// for loop end
//
//#if SECONDE_INCREASE
//		limit_counter++;
//#endif
//
//		fsync(fddest);
//
//
//#ifdef _WIN32
//		_ftime(&tb_end);
//#else
//		gettimeofday(&tv_end,&tz);
//#endif
//
//		int sleep_counter = 0;
//#ifdef _WIN32
//		sleep_counter = (tb_end.time - tb_start.time)*1000 + (tb_end.millitm - tb_start.millitm);
//	
//#if SECONDE_INCREASE		
//		sleep_counter = 1000*limit_counter - sleep_counter;
//#else
//		sleep_counter = 1000 - sleep_counter;
//#endif
//#else
//		sleep_counter = (tv_end.tv_sec - tv_start.tv_sec)*1000000 + (tv_end.tv_usec - tv_start.tv_usec);
//#if SECONDE_INCREASE		
//		sleep_counter = 1000000*limit_counter - sleep_counter;
//#else
//		sleep_counter = 1000000 - sleep_counter;
//#endif
//#endif
//
//        time_current= time(&time_current);
//        elapsed_seconds = time_current - m_time_elapsed_start_real;
//        if(elapsed_seconds)
//        {
//            m_copy_rate_real = m_size_copiedelapsed_real / elapsed_seconds;
//        }
//
//#if 0
//		////////////
//		time_t time2;
//		time(&time2);
//		printf("bbbbbbbbbbbbbbbbbbbb,time2:%d\n",time2);
//		printf("m_copy_rate:%d,MAX_RATE_COPY:%d,sleep_counter:%d,m_size_copiedelapsed_real:%lld,elapsed_seconds:%d,m_copy_rate_real:%d\n",
//			m_copy_rate,
//			MAX_RATE_COPY,
//			sleep_counter,
//			m_size_copiedelapsed_real,
//			elapsed_seconds,
//			m_copy_rate_real);
//		/////////////
//#endif
//
//        if(m_copy_rate <= MAX_RATE_COPY)
//        {
//#ifndef _WIN32
//
//#if 0
//            tv.tv_sec = 0;
//            tv.tv_usec = sleep_counter;////1000000
//            FD_ZERO(&readfds);
//            FD_SET(STDIN, &readfds);
//            /* don't care about writefds and exceptfds: */
//            select(STDIN+1, &readfds, NULL, NULL, &tv);
//
//            if (FD_ISSET(STDIN, &readfds))
//            {
//                printf("A key was pressed!\n");
//            }
//            else
//            {
//                //printf("Timed out.\n");
//            }
//
//#endif
//
//#if 1
//			if(sleep_counter>0)
//			{
//				//fsync(fddest);
//				usleep(sleep_counter);
//			}
//			else
//			{
//#if SECONDE_INCREASE		
//				/// 复位开始时间
//				usleep(1000000);
//				gettimeofday(&tv_start,&tz);
//				limit_counter = 0;
//#endif
//			}
//#endif
//
//#else
//			msleep(sleep_counter);
//#endif
//        }
//
//		int printf_counter = 0;	
//		printf_counter = 1073741824 / UNIT_COPY_SIZE;	/// 1G=1024*1024*1024=1073741824
//		if(printf_counter <= monitor_counter)
//		{	///每1G打印一次 
//			monitor_counter = 0;
//			printf("---------------------------------------\n");
//			printf("sleep_counter:%d\n",sleep_counter);
//            printf("setting rate:%d\n",m_copy_rate);
//            printf("real rate:%d\n",m_copy_rate_real);
//			printf("copying:%s\nsize:%lld\n",m_sourcefullpath,m_size_copiedelapsed);
//			printf("---------------------------------------\n");
//
//		}
//
//	}
//
//	if(NULL != pf_src)
//	{
//		ret = fclose(pf_src);
//		pf_src = NULL;
//	}
//
//	if(NULL != pf_desc)
//	{
//		ret = fclose(pf_desc);
//		pf_desc = NULL;
//	}
//
//#if 1
//	msleep(3000);
//#endif
//
//#if 1
//	printf("copy exit:%s\n",m_sourcefullpath);
//#endif
//
//
//	delete [] pdatabuff;
//	pdatabuff = NULL;
//	
//	if(COPY_STOP != m_copy_state)
//	{
//		m_copy_state = COPY_FINISHED;
//	}
//
//	m_bcopying = 0;
//	m_copy_control_stop = 0;
//	m_isrunning = 0;
//	m_thread_isexist = 0;
//	
//    /// gkm add 20141216 start
//    m_size_copiedelapsed_real = 0;
//    /// gkm add 20141216 end
//
//#if 1
//	lwpid=syscall(SYS_gettid);
//	memset(buff_temp, 0, sizeof(buff_temp));
//	sprintf(buff_temp, "dddddd:file copy procedure exit,lwpid:%d\n",lwpid);
//	g_boot_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
//#endif  
//
//
//	return ret;
//
//}
int CFileManager::GetCopyingState(FILE_COPY_STATE *pstate,
							long long *psize_total,
							long long *psize_elpased,
							long long *psize_remain,
							int *pcopyingbytespersecond)
{
	int ret = 0;
	*pstate = m_copy_state;
	*psize_total = m_size_copyingfile;

	if((COPY_STOP == m_copy_state) ||
		(COPY_FINISHED == m_copy_state) ||
		(COPY_FINISHED_ERROR == m_copy_state))
	{
		while(m_thread_isexist)
		{
			printf("waitting copy thread to exit\n");
            usleep(50000);
		}
	}

	if(COPY_FINISHED == m_copy_state)
	{
		*psize_elpased = m_size_copyingfile;
	}
	else
	{
		*psize_elpased = m_size_copiedelapsed;
	}

	*psize_remain = m_size_copyingfile - m_size_copiedelapsed;
	
	if( (0 == m_copy_rate_real) ||
			(m_copy_rate_real > (COPYING_SPEED*10)))
	{
		m_copy_rate_real = COPYING_SPEED;
	}
	

	*pcopyingbytespersecond = m_copy_rate_real;

	return ret;
}
int CFileManager::IsCopying(int *piscopying)
{
	int ret = 0;
	*piscopying = m_bcopying;
	return ret;
}
int CFileManager::StopCopy()
{
	int ret = 0;
	m_copy_control_stop = 1;
	
	while(m_thread_isexist)
	{
		printf("stop:waitting copy thread to exit\n");
        usleep(50000);
	}
			
	return ret;
}
int CFileManager::ResetCopyState()
{
	int ret = 0;
	
	if(!m_bcopying)
	{
		m_bcopying = 0;
		m_copy_state = COPY_STOP;
		m_copy_control_stop = 0;
		m_size_copyingfile = 0;
		m_size_copiedelapsed = 0;
		m_isrunning = 0;

		memset(m_descfullpath,0,sizeof(m_descfullpath));
		memset(m_sourcefullpath,0,sizeof(m_sourcefullpath));

        /// gkm add 20141216 start
        m_size_copiedelapsed_real = 0;
        m_copy_rate = MAX_RATE_COPY + 1024;
        m_copy_rate_real = 0;
        m_wait_loop_counter = m_max_loop_counter;
        /// gkm add 20141216 end

	}

	return ret;
}
int CFileManager::SetCopyRate(int copyrate_m)
{
    int ret = 0;
	int copyrate = 0;

		if(0>=copyrate_m)
		{
			copyrate = MAX_RATE_COPY + 1024;		
		}
		else
		{
			copyrate = copyrate_m * 1024 * 1024;
		}

#if 0
		printf("copy rate:%d\n",copyrate);
#endif

		if(copyrate == m_copy_rate)
		{
			return ret;
		}

		m_copy_rate = copyrate;


    if(m_copy_rate < MIN_RATE_COPY)
    {
        m_wait_loop_counter = m_min_loop_counter;
    }
    else if(m_copy_rate > MAX_RATE_COPY)
    {
        m_wait_loop_counter = m_max_loop_counter;
    }
    else
    {
        m_wait_loop_counter = m_copy_rate / UNIT_COPY_SIZE;
    }

    m_size_copiedelapsed_real = 0;
    m_time_elapsed_start_real= time(&m_time_elapsed_start_real);

    return ret;
}
int CFileManager::GetCopyRate(int *pcopyrate_m)
{
    int ret = 0;
    *pcopyrate_m = m_size_copiedelapsed_real / 1048576;
    return ret;
}
#if 0
int CFileManager::GetHash(char *pbuff_hash,int *plen_hash,int max_len)
{
	int ret = 0;
	if(NULL == pbuff_hash || 
		NULL == plen_hash ||
		(!m_bfinished))
	{
		return 1;
	}

#if SMS_DEBUG_PRINT
	printf("GetHash hash:%s\nlen:%d\n",m_buff_filehash,m_len_filehash);
#endif


	memset(pbuff_hash,0,max_len);

	memcpy(pbuff_hash,m_buff_filehash,m_len_filehash);

	*plen_hash = m_len_filehash;

	return ret;
}
#endif
int CFileManager::GetCopyingSpeed(int *pcopyingbytespersecond)
{
	int ret = 0;
	long long elapsedseconds = 0;
	long long elapsedsize = 0;

	if(NULL == pcopyingbytespersecond)
	{
		return 1;
	}

	ret = GetCopiedElapsedSeconds((int *)&elapsedseconds);

	elapsedsize = m_size_copiedelapsed;

	if( (0 >= elapsedseconds) ||
		( 0 >= elapsedsize) ||
		ret)
	{
		*pcopyingbytespersecond = COPYING_SPEED;
	}
	else
	{
		*pcopyingbytespersecond = elapsedsize / elapsedseconds;
	}

	ret = 0;

#if SMS_DEBUG_PRINT
	printf("copying speed:%d\n",*pcopyingbytespersecond);
#endif

	return ret;
}
/// 获取已经拷贝的秒数
int CFileManager::GetCopiedElapsedSeconds(int *pelapsedseconds)
{
	int ret = 0;
	int elapsedseconds = 0;
	time_t time_cur;

	if(NULL == pelapsedseconds)
	{
		return 1;
	}

	*pelapsedseconds = 0;

	if(COPY_FINISHED == m_copy_state )
	{
		return 1;
	}

	time(&time_cur);

	elapsedseconds = time_cur - m_time_copystart;

	*pelapsedseconds = elapsedseconds;

#if SMS_DEBUG_PRINT
	printf("elapse seconds:%d\n",*pelapsedseconds);
#endif

	return ret;
}

/// 根据文件大小获取拷贝所需要的时间
int CFileManager::GetCopySeondsBySize(int *pcopiedseconds,long long filesize)
{
	int ret = 0;
	int copyingbytespersecond = 0;

	if(NULL == pcopiedseconds ||
		filesize <= 0)
	{
		return 1;
	}

	ret = GetCopyingSpeed(&copyingbytespersecond);

	if(ret)
	{
		return ret;
	}

	*pcopiedseconds = filesize / copyingbytespersecond;

#if SMS_DEBUG_PRINT
	printf("filesize: %lld bytes\n",filesize);
	printf("copy speed: %d bytes/second\n",copyingbytespersecond);
	printf("copy seconds:%d\n",*pcopiedseconds);
#endif


	return ret;
	
}
int CFileManager::testsplit()
{
	int ret = RET_SUCCESS;
	int i = 0;
	int buffer_count = 0;
	char buffer_test[255];
	int buffer_test_len = 0;
	char split_buffers[100][20];
	
	memset(buffer_test,0,sizeof(buffer_test));
    sprintf(buffer_test,"10 0,30,50,,,60");
	buffer_test_len = strlen(buffer_test);
	
	ret = split_parse(buffer_test,buffer_test_len,split_buffers,100,20,&buffer_count,",",1);

	for(i = 0; i < buffer_count; i++)
	{
#if 1
		printf("buffer[%d]:%s\n",i,split_buffers[i]);
#endif

	}

#if 1
	printf("buffer_count:%d\n",buffer_count);
#endif

	return ret;
}
int CFileManager::split_parse(char *pbuffer,int buffer_len,char (*split_buffers)[20],int maxcount,int max_len,int *pbuffer_count,char *psplit_tmp,int split_tmp_len)
{

	int ret = RET_SUCCESS;
	int i = 0;
	int buffer_count = 0;
	int buffer_index = 0;
	char *pbuffer_temp = NULL;


	if(NULL == split_buffers ||
		NULL == pbuffer_count)
	{
		return PARAMETER_ERROR;
	}

	for(i = 0; i < maxcount; i++)
	{
		memset(split_buffers[i],0,max_len);
	}
	

	pbuffer_temp = pbuffer;
	buffer_index = 0;
    i = 0;

    while(i < buffer_len)
	{
		char *ptemp = NULL;
		int pos_len = 0;
		int index = 0;
		int j = 0;
				
		ptemp = strstr(pbuffer_temp,psplit_tmp);

        if(NULL == ptemp)
        {
            while( i < buffer_len)
            {
                if(isdigit(pbuffer[i]))
                {
                    split_buffers[buffer_index][index++] = pbuffer[i];
                    i++;
                }
                else
                {
                    i = buffer_len;
                    break;
                }
            }

            if(index!=0)
            {
#if 1
                printf("split_buffers[%d]:%s\n",buffer_index,split_buffers[buffer_index]);
#endif
                buffer_index++;
            }
        }
        else
        {
            pos_len =  ptemp - pbuffer_temp;

            if(pos_len<=0)
            {
                pbuffer_temp+=split_tmp_len;
                i += split_tmp_len;
                continue;
            }

            ptemp -= pos_len;

            for( j = 0; j < pos_len; j++)
            {
                if(isdigit(ptemp[j]))
                {
                    split_buffers[buffer_index][index++] = ptemp[j];
                }
                else
                {
                    j = pos_len;
                    break;
                }
            }

            pbuffer_temp+=j+split_tmp_len;
             i += j+split_tmp_len;

#if 1
        printf("split_buffers[%d]:%s\n",buffer_index,split_buffers[buffer_index]);
#endif

             buffer_index++;
        }

	

		   

	}	
		

	*pbuffer_count = buffer_index;

	return ret;

}
int CFileManager::killmountlast(const char *pmountdest)
{
	int ret = 0;
	FILE *pf = NULL;
	char pstrcommand[255];
	char pstrresult[255];
	char buff_pid[20];

	if(NULL == pmountdest)
	{
		return PARAMETER_ERROR;
	}

	memset(buff_pid,0,sizeof(buff_pid));

	pf = NULL;
	memset(pstrcommand,0,sizeof(pstrcommand));
	memset(pstrresult,0,sizeof(pstrresult));

	//sprintf(pstrcommand,"ps -ef | grep -m1 \'%s'| awk \'{print $2}\'",pmountdest );
	//sprintf(pstrcommand,"ps -ef | grep \'%s\'| awk \'{print $2}\'",pmountdest );
    //sprintf(pstrcommand,"netstat -tunlp | grep \'12340\'| awk \'{printf $7}\'");
    sprintf(pstrcommand,"netstat -tunlp | grep \'12340\'| awk \'{printf $7}\' | sed \'s/\\/mount.ntfs-3g//g\'" );
    printf("pstrcommand:%s\n",pstrcommand );

	pf = popen(pstrcommand,"r");
	if(NULL == pf)
	{
		printf("%s:command executed failed\n",pstrcommand);
		return ret;
	}

	while(NULL != fgets(pstrresult,sizeof(pstrresult),pf))
	{
		if('\n' == pstrresult[strlen(pstrresult)-1])
		{
				pstrresult[strlen(pstrresult)-1] = '\0';
		}
		
#if DEBUG_PRINT
		if(NULL != pstrresult)
		{
			printf("%s=>%s\n",pstrcommand,pstrresult);
		}
#endif 		
		
	}

	if(-1 == pclose(pf))
	{
#if 0
		printf("%s:command closed failed\n",pstrcommand);
#endif
	}

	if( NULL == pstrresult ||
		(!strcmp(pstrresult,"")))
	{
		printf("%s:result is null\n",pstrcommand);
	}	
	else
	{
		//printf("%s:result is:%s\n",pstrcommand,pstrresult);
		pf = NULL;

		strcpy(buff_pid,pstrresult);
		printf("buff_pid:%s\n",buff_pid);

		memset(pstrcommand,0,sizeof(pstrcommand));
		memset(pstrresult,0,sizeof(pstrresult));

		sprintf(pstrcommand,"kill -9 %s", buff_pid);
		printf("pstrcommand:%s\n",pstrcommand );

		pf = popen(pstrcommand,"r");
		if(NULL == pf)
		{
			printf("%s:command executed failed\n",pstrcommand);
			return ret;
		}

		while(NULL != fgets(pstrresult,sizeof(pstrresult),pf))
		{
			if('\n' == pstrresult[strlen(pstrresult)-1])
			{
					pstrresult[strlen(pstrresult)-1] = '\0';
			}
		
//#if DEBUG_PRINT
#if 1
			if(NULL != pstrresult)
			{
				printf("%s=>%s\n",pstrcommand,pstrresult);
			}
#endif 		
		
		}

		pclose(pf);
		pf = NULL;

	}

	return ret;
}
int CFileManager::get_exe_path( char * buf, int count)
{
    int i = 0;
    int rslt = 0;
    
    rslt = readlink("/proc/self/exe", buf, count - 1);
    if (rslt < 0 || (rslt >= count - 1))
    {
        return NULL;
    }
    
    buf[rslt] = '\0';
    for (i = rslt; i >= 0; i--)
    {
        //printf("buf[%d] %c\n", i, buf[i]);
        if (buf[i] == '/')
        {
            buf[i + 1] = '\0';
            break;
        }
    }
    
    return rslt;
}
int CFileManager::GetImbSM_time(const char *pipaddress,const char *pchain,const char *pprivate,char *presult,int maxlen)
{
		int ret = 0;
		FILE *pf = NULL;
		char pstrcommand[255];
		char pstrresult[255];
	
		if(NULL == pipaddress ||
			NULL == pchain||
			NULL == pprivate||
			NULL == presult
			)
		{
			return PARAMETER_ERROR;
		}
	
		memset(presult,0,sizeof(maxlen));
	
		pf = NULL;
		memset(pstrcommand,0,sizeof(pstrcommand));
		memset(pstrresult,0,sizeof(pstrresult));
	

		sprintf(pstrcommand,"./mvc2smtool -n %s -s %s,%s QuerySm",pipaddress,pchain,pprivate );
		printf("pstrcommand:%s\n",pstrcommand );
	
		pf = popen(pstrcommand,"r");
		if(NULL == pf)
		{
			printf("%s:command executed failed\n",pstrcommand);
			return ret;
		}
	
		while(NULL != fgets(pstrresult,sizeof(pstrresult),pf))
		{
			if('\n' == pstrresult[strlen(pstrresult)-1])
			{
					pstrresult[strlen(pstrresult)-1] = '\0';
			}
			
#if DEBUG_PRINT
			if(NULL != pstrresult)
			{
				printf("%s=>%s\n",pstrcommand,pstrresult);
			}
#endif 	

			if(NULL != pstrresult)
			{
				strcat(presult,pstrresult);
			}

			
		}
	
		if(-1 == pclose(pf))
		{
#if 0
			printf("%s:command closed failed\n",pstrcommand);
#endif
		}
	
		if( NULL == pstrresult ||
			(!strcmp(pstrresult,"")))
		{
			printf("%s:result is null\n",pstrcommand);
		}	
		else
		{
			printf("%s:result is:%s\n",pstrcommand,pstrresult);
		}
	
		return ret;

}
int CFileManager::SetImbSM_time(const char *pipaddress,const char *pchain,const char *pprivate,int diff_seconds)
{
		int ret = 0;
		FILE *pf = NULL;
		char pstrcommand[255];
		char pstrresult[255];

	
		if(NULL == pipaddress ||
			NULL == pchain||
			NULL == pprivate
			)
		{
			return PARAMETER_ERROR;
		}

	
		pf = NULL;
		memset(pstrcommand,0,sizeof(pstrcommand));
		memset(pstrresult,0,sizeof(pstrresult));
	

		sprintf(pstrcommand,"./mvc2smtool -n %s -s %s,%s adjustTime %d",pipaddress,pchain,pprivate,diff_seconds );
		printf("pstrcommand:%s\n",pstrcommand );
	
		pf = popen(pstrcommand,"r");
		if(NULL == pf)
		{
			printf("%s:command executed failed\n",pstrcommand);
			return ret;
		}
	
		while(NULL != fgets(pstrresult,sizeof(pstrresult),pf))
		{
			if('\n' == pstrresult[strlen(pstrresult)-1])
			{
					pstrresult[strlen(pstrresult)-1] = '\0';
			}
			
#if DEBUG_PRINT
			if(NULL != pstrresult)
			{
				printf("%s=>%s\n",pstrcommand,pstrresult);
			}
#endif 		
			
		}
	
		if(-1 == pclose(pf))
		{
#if 0
			printf("%s:command closed failed\n",pstrcommand);
#endif
		}
	
		if( NULL == pstrresult ||
			(!strcmp(pstrresult,"")))
		{
			printf("%s:result is null\n",pstrcommand);
		}	
		else
		{
			printf("%s:result is:%s\n",pstrcommand,pstrresult);
		}
	
		return ret;

}
int CFileManager::InitReadFile(const char *pfilefullpath,long long &file_size)
{
	int ret = 0;
	int isexist=0;
	char fullpath_read[255]={'\0'};
	long long totalsize_readfile=0;

	if(NULL==pfilefullpath ||
		(!strcmp(pfilefullpath,"")))
	{
		return ERROR_FILEMANAGER_MODULE_INPUT_PARMETER_INVALID;
	}

	sprintf(fullpath_read,"%s",pfilefullpath);

	if(!IsExistFile(fullpath_read))
	{
		return ERROR_FILEMANAGER_MODULE_FILE_NOT_EXIST;
	}

	UninitReadFile();

	sprintf(m_fullpath_read,"%s",fullpath_read);

	GetFileSize(m_fullpath_read,&m_totalsize_readfile);
	file_size = m_totalsize_readfile;

	if(!file_size)
	{
		return ERROR_FILEMANAGER_MODULE_FILE_ISEMPTY;
	}

	m_pf_read = fopen(m_fullpath_read,"rb");

	return ret;
}
int CFileManager::ReadFile(unsigned char *pbuff,int max_len,int &len,long long &elapsed_size,int &isfinished)
{
	int ret = 0;
	int data_len=0;

	if(NULL == m_pf_read)
	{
		return ERROR_FILEMANAGER_MODULE_READFILE_NOT_INITIALIZE;
	}

	if(m_isfinished_read)
	{
		return ERROR_FILEMANAGER_MODULE_READFILE_READ_OVER;
	}

	memset(pbuff,0,max_len);

	data_len = fread(pbuff,1,max_len,m_pf_read);

	if(data_len < 0)
	{
		m_errorcode_read = data_len;
		return ERROR_FILEMANAGER_MODULE_READFILE_READ_ERROR;
	}
	else
	{
		m_elapsed_readsize+=data_len;

		if(m_elapsed_readsize == m_totalsize_readfile)
		{
			isfinished = 1;
			m_isfinished_read = 1;
		}
	}

	return ret;
}
int CFileManager::UninitReadFile()
{
	int ret = 0;

	if(NULL != m_pf_read)
	{
		fclose(m_pf_read);
	}

	m_totalsize_readfile = 0;
	m_elapsed_readsize = 0;
	m_isfinished_read=0;
	m_pf_read = NULL;
	m_errorcode_read = 0;
	memset(m_fullpath_read,0,sizeof(m_fullpath_read));

	return ret;

}
//int CFileManager::GetErrorString(int errorcode,REPORT_STATUS *preportstatus)
//{
//	int ret = 0;
//
//	memset(preportstatus,0,sizeof(REPORT_STATUS));
//
//	sprintf(preportstatus->status,"failed");
//	sprintf(preportstatus->error_code_str,"%d",errorcode);
//	sprintf(preportstatus->level,"error");
//    sprintf(preportstatus->module,"filemanager");
//
//
//	switch(errorcode)
//	{
//	case ERROR_FILEMANAGER_MODULE_INPUT_PARMETER_INVALID:
//		sprintf(preportstatus->message,"input parameter is error");
//        sprintf(preportstatus->message_la,"输入参数错误");
//		break;
//	case ERROR_FILEMANAGER_MODULE_FILE_NOT_EXIST:
//		sprintf(preportstatus->message,"failed to connect database");
//        sprintf(preportstatus->message_la,"文件不存在");
//		break;
//	case ERROR_FILEMANAGER_MODULE_READFILE_NOT_INITIALIZE:
//		sprintf(preportstatus->message,"failed to read file,not initialize");
//        sprintf(preportstatus->message_la,"读取文件失败，没有初始化");
//		break;
//	case ERROR_FILEMANAGER_MODULE_FILE_ISEMPTY:
//		sprintf(preportstatus->message,"file is empty");
//        sprintf(preportstatus->message_la,"文件为空");
//		break;
//	case ERROR_FILEMANAGER_MODULE_READFILE_READ_OVER:
//		sprintf(preportstatus->message,"failed to read file,is read over already");
//        sprintf(preportstatus->message_la,"读取文件失败，已经读取完毕");
//		break;
//	case ERROR_FILEMANAGER_MODULE_READFILE_READ_ERROR:
//		sprintf(preportstatus->message,"failed to read file error code:%d",m_errorcode_read);
//        sprintf(preportstatus->message_la,"读取文件失败，错误码:%d",m_errorcode_read);
//		break;
//	default:
//		sprintf(preportstatus->status,"successfull");
//		sprintf(preportstatus->error_code_str,"0");
//		sprintf(preportstatus->message,"is successfull");
//        sprintf(preportstatus->message_la,"成功");
//        sprintf(preportstatus->level,"normal");
//        sprintf(preportstatus->module,"usercontroller");
//		break;
//	}
//
//	return ret;
//
//}
