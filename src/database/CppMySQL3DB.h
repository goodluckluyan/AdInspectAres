//@file:CppMySQL3DB.h
//@brief: 包含类CppMySQL3DB。此类为外部引用。
//封装了数据库的各种访问操作。
//@引用者:wangzhongping@oristartech.com
//dade:2012-07-12
#ifndef _CPP_MYSQL_3DB
#define _CPP_MYSQL_3DB
#include <string>
#include "CppMySQLQuery.h"

#define BUFSIZE 1024

class CppMySQL3DB
{
public:

    CppMySQL3DB();

    virtual ~CppMySQL3DB();
		//链接数据库。
    int open(const char* host, const char* user, const char* passwd, const char* db,
		unsigned int port = 0, unsigned long client_flag = 0);
		int openTMS();
		//关闭数据库。
    void close();

	/* 返回句柄 */
	MYSQL* getMysql();

	/* 处理返回多行的查询，返回影响的行数 */
	//返回引用是因为在CppMySQLQuery的赋值构造函数中要把成员变量_mysql_res置为空
	CppMySQLQuery& querySQL(const char *sql,int &iResult);
	/* 执行非返回结果查询 */
	int execSQL(const char* sql);
	/* 测试mysql服务器是否存活 */
	int ping();
	/* 关闭mysql 服务器 */
	int shutDown();
	/* 主要功能:重新启动mysql 服务器 */
	int reboot();
	/*
	* 说明:事务支持InnoDB or BDB表类型
    */
	/* 主要功能:开始事务 */
	int startTransaction();
	/* 主要功能:提交事务 */
	int commit();
	/* 主要功能:回滚事务 */
	int rollback();
	/* 得到客户信息 */
	const char * getClientInfo();
	/* 主要功能:得到客户版本信息 */
	const unsigned long  getClientVersion();
	/* 主要功能:得到主机信息 */
	const char * getHostInfo();
	/* 主要功能:得到服务器信息 */
	const char * getServerInfo();
	/*主要功能:得到服务器版本信息*/
	const unsigned long  getServerVersion();
	/*主要功能:得到 当前连接的默认字符集*/
	const char *  getCharacterSetName();
	/* 得到系统时间 */
	const char * getSysTime();
	/* 建立新数据库 */
	int createDB(const char* name);
	/* 删除制定的数据库*/
	int dropDB(const char* name);

	bool tableExists(const char* table);

    u_int lastRowId();

    void setBusyTimeout(int nMillisecs){};

	
	std::string errmsg()
	{
		std::string sError;
		char buffer[BUFSIZE]="";
		unsigned int errcode = mysql_errno(_db_ptr);
		const char* pstr = mysql_error(_db_ptr);
		if ( errcode == 0 && pstr!=NULL && pstr[0] == '\0' )
		{
			return sError;
		}
		sprintf( buffer , "mysql DB error msg:errno(%u) \n errormsg:(%s)\n" , errcode , pstr );
		sError = buffer;
		return sError;
	}

private:

    CppMySQL3DB(const CppMySQL3DB& db);
    CppMySQL3DB& operator=(const CppMySQL3DB& db);

    void checkDB();

private:
	/* msyql 连接句柄 */
	MYSQL* _db_ptr;
	CppMySQLQuery _db_query;
};
#endif //_CPP_MYSQL_3DB

#define WRITE_DBELOG
#if defined(WRITE_DBELOG) 
#ifndef TMS20_LOG
#include "log/MyLogger.h"
#endif
#endif

#ifdef WRITE_DBELOG
#define DBLOG( errorCode ,str) MySqlDB_WriteErrorLog(errorCode,str);
int MySqlDB_WriteErrorLog( int& errorCode ,const std::string& strError );
#else
#define DBLOG( errCode ,str ) 
#endif // WRITE_DBELOG



