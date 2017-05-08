#ifndef _FILEMANAGER_LINUX_H
#define _FILEMANAGER_LINUX_H
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <vector>
#include <string.h>
#include <stdarg.h>
#include <dirent.h> 
#include <memory.h>
#include <ctype.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/time.h>

using namespace std;

#ifndef LPVOID
#define LPVOID void *
#endif

#ifndef BUFF_SIZE_50
#define BUFF_SIZE_50 50
#endif

/// GKM:应用程序类型 DLL EXE
typedef enum _EXE_TYPE { EXE, DLL }EXE_TYPE, *pEXE_TYPE;

/// GKM:文件查找类型
typedef enum { FIND_DIR, FIND_FILE, FIND_ALL }FINDFILE_TYPE, *pFINDFILE_TYPE;

/// GKM:文件信息类型
typedef enum { FILEINFO_DIR, FILEINFO_FILE, FILEINFO_LINK,FILEINFO_OTHRER }FILEINFO_TYPE, *pFILEINFO_TYPE;

/// 文件拷贝状态
typedef enum { COPY_STOP,COPY_COPYING,COPY_FINISHED,COPY_FINISHED_ERROR}FILE_COPY_STATE;

/// GKM:文件信息结构定义
typedef struct _FILE_INFO
{
	char *pfilepath;
	char *pfilename;
	char *pfilefullname;
	FILEINFO_TYPE filetype;
	long long filesize;
	struct stat64 file_st;
}FILE_INFO,*pFILE_INFO;

typedef vector<FILE_INFO *>FILE_INFOS;


//////////////////////////
typedef struct _REPORT_STATUS REPORT_STATUS;
#define RET_SUCCESS                0                                     ///	操作成功
#define PARAMETER_ERROR            10030                                /// 输入参数错误
#define FILEMANAGER_MODULE_ERROR_CODE_BASE_VALUE						31000
#define ERROR_INPUT_PARMETER_INVALID									1
#define ERROR_FILE_NOT_EXIST											2
#define ERROR_READFILE_NOT_INITIALIZE									3
#define ERROR_FILE_ISEMPTY												4
#define ERROR_READFILE_READ_OVER										5
#define ERROR_READFILE_READ_ERROR										6
const int ERROR_FILEMANAGER_MODULE_INPUT_PARMETER_INVALID				= FILEMANAGER_MODULE_ERROR_CODE_BASE_VALUE + ERROR_INPUT_PARMETER_INVALID;
const int ERROR_FILEMANAGER_MODULE_FILE_NOT_EXIST						= FILEMANAGER_MODULE_ERROR_CODE_BASE_VALUE + ERROR_FILE_NOT_EXIST;
const int ERROR_FILEMANAGER_MODULE_READFILE_NOT_INITIALIZE				= FILEMANAGER_MODULE_ERROR_CODE_BASE_VALUE + ERROR_READFILE_NOT_INITIALIZE;
const int ERROR_FILEMANAGER_MODULE_FILE_ISEMPTY							= FILEMANAGER_MODULE_ERROR_CODE_BASE_VALUE + ERROR_FILE_ISEMPTY;
const int ERROR_FILEMANAGER_MODULE_READFILE_READ_OVER					= FILEMANAGER_MODULE_ERROR_CODE_BASE_VALUE + ERROR_READFILE_READ_OVER;
const int ERROR_FILEMANAGER_MODULE_READFILE_READ_ERROR					= FILEMANAGER_MODULE_ERROR_CODE_BASE_VALUE + ERROR_READFILE_READ_ERROR;
///////////////////////////

class CFileManager
{
public:
	CFileManager(void);
	~CFileManager(void);
	
//////////////////		
	/// 测试是否能ping通
	int PingIsOk(const char *pipaddress,int *pisok);

	/// 路径是否为目录
	int IsDir(const char *pdirpath,int *pisdir);

	/// 文件路径是否存在
	int IsExistPath(const char *ppath,int *pisexist);

	/// 创建目录
	int CreateDir(const char *pdirpath);

	/// 创建文件
	int CreateFile(const char *pfilepath);

	/// 删除目录
	int DeleteDirOrFile(const char *ppath);

	/// 获取路径下的所有文件及目录
	int GetFileFullNamesByPath(const char *ppath,char (*pfilefullnames)[BUFF_SIZE_50],int maxcount,int max_len,int *pfilefullname_count);

	/// 获取路径下的所有文件及目录
	int IsEmptyDir(const char *ppath,int *pisempty);

////////////

	/// GKM:根据指定路径检索文件
	///int FindFileByPath(const char *pfilepath, FILE_INFOS *pfileinfos, FINDFILE_TYPE findFileType, int isSubDir);
	
	/// GKM:获取可执行程序的路径
	int GetLocalPath(char *ppath,int maxlen);
	
	/// GKM:获取文件信息
	int GetFileInfo(const char *pfileFullName, FILE_INFO *pfileinfo);

	/// GKM:获取文件所在的路径
	int GetFilePath(const char *pfileFullName,int type,char *pfilePath,int maxlen_filepath);

	/// GKM:获取文件的名称
	int GetFileName(const char *pfileFullName,int type,char *pfileName,int maxlen_filename);

	/// GKM:在源字符串中去掉过滤字符串
	int GetFilterString(const char *strSrc, char *strFilter, char *strDest);

	/// @brief 获取文件大小
	int GetFileSize(const char *pfullpath,long long *pfilesize);

	/// @brief 获取文件大小
	int GetFileSizeS(const char *pfullpath,char *psize,int maxlen);
	
		/// 获取磁盘信息（磁盘大小，已用空间，可用空间，已用百分比）
	int GetDiskInfo_S(const char *pdiskdir,
					char *pdisksize,
					int maxlen_disksize,
					char *pdiskused,
					int maxlen_diskused,
					char *pdiskavail,
					int maxlen_diskavail,
					char *pdiskusedpercent,
					int maxlen_diskusedpercent
					);

	/// 获取最大单位数量
	int GetMaxUnitSize(char *pmaxunit,int maxlen_maxunit,long long maxsize);

	/// 文件是否存在
	bool IsExistFile(const char *pfullpath);

	/// 路径是否存在
	bool IsExistPath(const char *ppath);

	/// 纠正路径字符串为合法，针对路径中出现连续多个路径分隔符的情况,使多个变成一个
	int CorrectPathString(char *ppath,int maxlen,int type);

	/// 生成路径字符串
	int GeneratePathString(char *ppath,int type,int maxlen,const char *pathmsg,...);

	/// 在路径加上分隔符（windows "\\" linux "/"），如果有或多个连续分隔符，则处理成一个,
	/// flag = 0,从路径起始位置向后查找；flag = 1,从路径结束位置向前查找；flag = other,从路径两端向中间查找。
	int AddSeparator(char *ppath,int flag,int type);
	/// 在路径减去分隔符（windows "\\" linux "/"），如果有或多个连续分隔符，则全部减去,
	/// flag = 0,从路径起始位置向后查找；flag = 1,从路径结束位置向前查找；flag = other,从路径两端向中间查找。
	int SubtractSeparator(char *ppath,int flag,int type);
	
	/// 添加转义字符'\'，适应mysql插入语句中有路径的情况
	int ConrrectToHaveESCPathString(char *ppath,int maxlen,int type);

	/// 获取文件名后缀,最后一个点作为分隔符
	int GetPostFixInFileName(char *ppostfix,int maxlen,const char *pfilename);

	/// 获取文件名，不包含后缀，最后一个点作为分隔符
	int GetNameInFileName(char *pname,int maxlen,const char *pfilename);
	
	/// 清空语言函数
	///int EmptyLang();
	
	/// 字符替换函数 将字符串中的某字符全部替换成指定字符
	int ReplaceCharInString(char *pbuff,int max_len,char ch_from,char ch_to);
	
	/// 编码格式转换
	int code_convert(const char *pcharset_from,
						const char *pcharset_to,
						char *pbuff_in,
						size_t len_in,
						char *pbuff_out,
						size_t maxlen_out,
						size_t *plen_out);
						
	int ReplaceStr(char *pbuff_src,const char *pbuff_from,const char *pbuff_to);


    int DeleteCharA(char *pbuff_src,char ch);


	int testsplit();
	int split_parse(char *pbuffer,int buffer_len,char (*split_buffers)[20],int maxcount,int max_len,int *pbuffer_count,char *psplit_tmp,int split_tmp_len);

	int killmountlast(const char *pmountdest);
	
	int get_exe_path( char * buf, int count);

	int GetImbSM_time(const char *pipaddress,const char *pchain,const char *pprivate,char *presult,int maxlen);
	int SetImbSM_time(const char *pipaddress,const char *pchain,const char *pprivate,int diff_seconds);

public:
	//int DeleteFIle_M(const char *pfilefullpath);

	/// GKM:拷贝文件，创建拷贝线程，并生成hash
///	int CopyFile_M(const char *pdestfullpath,const char *psrcfullpath);
	/// GKM:拷贝文件，不创建拷贝线程，不生成hash（拷贝不需要验证hash的小文件）
///	int CopyFile_M1(const char *pdestfullpath,const char *psrcfullpath);
	/// GKM:停止文件拷贝
	///int StopCopyFile_M();

	/// GKM:移动文件
	///int MoveFile_M(const char *pdestfullpath,const char *psrcfullpath);

	/// 拷贝线程
	///int copy_procedure();

	/// 获取拷贝进度
	int GetCopyingState(FILE_COPY_STATE *pstate,
							long long *psize_total,
							long long *psize_elpased,
							long long *psize_remain,
							int *pcopyingbytespersecond);
							
	/// 是否正在拷贝（拷贝线程是否正在运行）
	int IsCopying(int *piscopying);	
	
	int StopCopy();	
	
	int ResetCopyState();

    /// gkm add 20141216
    int SetCopyRate(int copyrate_m);

    /// gkm add 20141216
    int GetCopyRate(int *pcopyrate_m);
	
#if 0	
	/// 获取hash值
	int GetHash(char *pbuff_hash,int *plen_hash,int max_len);
#endif	

#if 0
	/// 获取hash值
	int GetHash(char *pbuff_hash,int *plen_hash,int max_len);
#endif

	/// 获取拷贝速度（字节/秒）
	int GetCopyingSpeed(int *pcopyingbytespersecond);

	/// 获取已经拷贝的秒数
	int GetCopiedElapsedSeconds(int *pelapsedseconds);

	/// 根据文件大小获取拷贝所需要的时间
	int GetCopySeondsBySize(int *pcopiedseconds,long long filesize);

	void file_seek(const char *dir, int depth,FILE_INFOS *pfileinfos);

public:
	int InitReadFile(const char *pfilefullpath,long long &file_size);
	int ReadFile(unsigned char *pbuff,int max_len,int &len,long long &elapsed_size,int &isfinished);
	int UninitReadFile();
	//int GetErrorString(int errorcode,REPORT_STATUS *preportstatus);

public:
	int NewSpaceFileInfo(FILE_INFO **pfileinfo);
	int DeleteSpaceFileInfo(FILE_INFO **pfileinfo);
	int ClearSpaceFileInfo(FILE_INFO *pfileinfo);
	int DeleteSpaceFileInfos(FILE_INFOS *pfileinfos);	
	
private:
	char m_descfullpath[255];
	char m_sourcefullpath[255];
//	char m_buff_filehash[255];
//	int m_len_filehash;
	long long m_size_copyingfile;
	long long m_size_copiedelapsed;
	FILE_COPY_STATE m_copy_state;	/// 0 schedule 1 copying  2 finished  3 finished_error
	int m_bcopying;	
	int m_copy_control_stop;	
	time_t m_time_copystart;
	int m_isrunning;
    /// gkm add 20141216 start
    int m_wait_loop_counter;
    int m_min_loop_counter;
    int m_max_loop_counter;
    int m_copy_rate;
    int m_copy_rate_real;
    long long m_size_copiedelapsed_real;
    time_t m_time_elapsed_start_real;
    /// gkm add 20141216 end
	int m_thread_isexist;

	FILE *m_pf_read;
	char m_fullpath_read[255];
	long long m_totalsize_readfile;
	long long m_elapsed_readsize;
	int m_isfinished_read;
	int m_errorcode_read;

		
};
#endif
