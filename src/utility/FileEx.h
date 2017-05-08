/*******************************************************************************
* 版权所有 (C) 2008
* 
* 文件名称： FileEx.h
* 文件标识： 
* 内容摘要： 文件操作辅助类。
* 其它说明： 本类提供的操作在linux下依然有效。
* 当前版本： V1.0
* 作    者： 周锋
* 完成日期： 2008-01-04
*******************************************************************************/
#ifndef _FILE_EX_75894328849318493216789054320573409
#define _FILE_EX_75894328849318493216789054320573409

#include <vector>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _WINDOWS_
#include <unistd.h>
#include <ftw.h>
#include <sys/vfs.h>
#endif

#define LPCSTR const char *
#define UINT unsigned int
class CFileEx
{
public:
	//获取文件分隔符（对于windows返回'\\'，linux返回'/'）
	static char Separator();

	//获取当前目录(返回值不包括末尾的“\\”或“/”)
	static std::string GetCurDirectory();

	//获取可执行程序所在目录(返回值不包括末尾的“\\”或“/”)
	static std::string GetExeDirectory();

	//使用标准文件分隔符
	static std::string StdSeparator(const char * lpszPath);

	//设置当前工作目录
	static bool SetCurDirectory(const char * lpszFolder);

	//创建指定的多级文件目录
	static bool CreateFolder(const char * lpszFolder);

	//为创建指定的文件创建必要的文件目录
	static bool CreateFolderForFile(const char * lpszFile);

	//获取指定目录下的所有文件（不包括目录）
	static void GetSubFiles(const char * lpszFolder, std::vector<std::string> &vecFile);

	//获取指定目录下的所有目录（不包括文件）
	static void GetSubFolders(const char * lpszFolder, std::vector<std::string> &vecFolder);

	//根据全路径获取文件名
	static std::string Path2FileName(const char *lpszPath);

	//根据全路径获取路径名
	static std::string File2PathName(const char *lpszPath);

	//获取磁盘剩余空间
	static UINT GetFreeDiskSpace(LPCSTR lpszPath);

	//删除指定文件夹（包括文件夹里面的所有内容）
	static bool DelFolder(LPCSTR lpszFolder);

	//删除指定的文件
	static bool DelFile(LPCSTR lpszPath);

private:
	CFileEx(void){}
	~CFileEx(void){}
};

#endif
