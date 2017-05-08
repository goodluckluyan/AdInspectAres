//@file:ec_config.h
//@brief: ec_config。
//ec_config：提供文件访问封装由C_Para调用 此类为外部引用。
//@引用人:wangzhongping@oristartech.com
//dade:2012-07-12


#ifndef _EC_CONFIG_H_
#define _EC_CONFIG_H_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <string.h>
using namespace std;

#define SEC_FLAG1		91		// '['	
#define	SEC_FLAG2		93		// ']'
#define COMM_FLAG		35		// '#'
#define	KEYVALUE_FLAG	61		// '='
#define BLANK_FLAG1		32		// ' '
#define BLANK_FLAG2		9		// '	'
#define BLANK_FLAG3		13		// 换行

//#define MAX_BUFF_LEN	255
#define MAX_BUFF_LEN	1024

typedef enum{CFG_SEC = 0,CFG_KEY = 1,CFG_COMMENT = 2,CFG_BLANK = 3,CFG_INVALID = 4}CFG_TYPE;

typedef struct _CFG_INFO
{
	string ssec;
	string skeyname;
	string skeyvalue;
	string scomment;
	string sblank;
	string sinvalid;
	CFG_TYPE cfg_type;
}CFG_INFO;

typedef struct _CFG_SECTION
{
	string ssec;
	vector<CFG_INFO>cfg_infos;
}CFG_SECTION;

typedef struct _CFG_FILE
{
	vector<CFG_INFO>cfg_infos;
	vector<CFG_SECTION>cfg_secs;
}CFG_FILE;



/// 文件结构定义：文件内容 = <注释> + <空白行> + <段信息1> + <段信息2> + ....
/// 段信息 = 关键值 + 注释

/// 注意:段名 与 关键值名 应该为字母数字或下划线的排列组合  后面可以有注释或空格
/// 字符'['与']'之间为段名
/// 可以有注释内容,以字符'#'作为注释起始，'#'之前可以有空格，但不能有其它内容(段名 和 关键值名除外)

class ec_config
{
public:
	ec_config();
	~ec_config();

public:

	/// 写信息
	int writevalue( const char *psec = NULL, const char *pkey = NULL, const char *pvalue = NULL, const char *ppath = NULL );
	/// 读信息
	int readvalue( const char *psec = NULL, const char *pkey = NULL, char *pvalue = NULL, const char *ppath = NULL);

private:
	/// 读取文件内容
	int readfinfo( const char *ppath, CFG_FILE &cfg_file, int &icntline );
	/// 是否是注释内容
	int iscomment( const char *pbuf );
	/// 是否是段内容
	int getsection( const char *pbuf, CFG_INFO &cfg_info );
	/// 是否是关键值
	int getkey( const char *pbuf, CFG_INFO &cfg_info );
	/// 是否是空白行
	int isblank( const char *pbuf );
	/// 去掉两端的空白字符（空格或制表符),返回字符串长度
	int trim( const char *pbuf,char *pdest,int ilenmax );

};

#endif
