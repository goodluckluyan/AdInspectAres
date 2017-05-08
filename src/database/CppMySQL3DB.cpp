//@file:CppMySQL3DB.cpp
//@brief: CppMySQL3DB：功能实现。
//@author:wangzhongping@oristartech.com
//dade:2012-07-12

#include "CppMySQL3DB.h"
#include "para/C_Para.h"
#include "C_constDef.h"
#include "C_ErrorDef.h"
#include "log/MyLogger.h"

extern MyLogger g_db_logwrite;
CppMySQL3DB::CppMySQL3DB()
{
	_db_ptr = NULL;
}

CppMySQL3DB::~CppMySQL3DB()
{
	if ( _db_ptr != NULL )
	{
		close();
	}
}

int CppMySQL3DB::open(const char* host, const char* user, const char* passwd, const char* db,
		unsigned int port /*= 0*/, unsigned long client_flag /*= 0*/)
{
	int ret = -1;

	_db_ptr = mysql_init(NULL);
	if( NULL == _db_ptr ) 
		goto EXT;
	
	//如果连接失败，返回NULL。对于成功的连接，返回值与第1个参数的值相同。
	if ( NULL == mysql_real_connect( _db_ptr, host, user, passwd, db,port, NULL, client_flag) )
		goto EXT;

	/* 设置数据库默认字符集 */ 	
	if ( mysql_set_character_set( _db_ptr, "utf8" )  != 0 )
	{
		fprintf ( stderr , "错误, %s/n" , mysql_error( _db_ptr) ) ;
	}

	//选择制定的数据库失败
	//0表示成功，非0值表示出现错误。
	if ( mysql_select_db( _db_ptr, db ) != 0 ) 
	{
		mysql_close(_db_ptr);
		_db_ptr = NULL;
		goto EXT;
	}

	ret = 0;
EXT:
	//初始化mysql结构失败
	if ( ret == -1 && _db_ptr != NULL )
	{
		//--zhangmiao begin:11/28/2012
		string strError;
		char buffer[BUFSIZE]="";
		sprintf( buffer , "mysql param: host(%s),user(%s),passwd(%s),DB(%s),port(%d),client_flag(%lu)\n" ,host ,user, passwd, db, port, client_flag );
		strError = errmsg();
		strError += buffer;
		DBLOG((int &)ERROR_OPEN_DATABASE,strError);
		//printf("%s\n",strError.c_str());
		//--zhangmiao end:11/28/2012

		mysql_close( _db_ptr );
		_db_ptr = NULL;
	}
	//--zhangmiao begin:11/28/2012
	else if ( ret == -1 && _db_ptr == NULL )
	{
		string strError;
		char buffer[BUFSIZE]="";
		sprintf( buffer , "mysql param: host(%s),user(%s),passwd(%s),DB(%s),port(%d),client_flag(%lu)\n" ,host ,user, passwd, db, port, client_flag );
		strError = errmsg();
		strError += buffer;
		DBLOG((int &)ERROR_OPEN_DATABASE,strError);
		//printf("%s\n",strError.c_str());
	}
	//--zhangmiao end:11/28/2012

	return ret;
}


void CppMySQL3DB::close()
{
	if ( _db_ptr != NULL )
	{
		mysql_close( _db_ptr );
		_db_ptr = NULL;
	}
}

MYSQL* CppMySQL3DB::getMysql()
{
	return _db_ptr;
}

/* 处理返回多行的查询，返回影响的行数 */
CppMySQLQuery& CppMySQL3DB::querySQL(const char *sql,int &iResult)
{
	//wzp 2012-09-11 modify;
/*	if ( !mysql_real_query( _db_ptr, sql, strlen(sql) ) )
	{
		_db_query._mysql_res = mysql_store_result( _db_ptr );
// 		_db_query._row =  mysql_fetch_row( _db_query._mysql_res );
// 		_db_query._row_count = mysql_num_rows( _db_query._mysql_res ); 
// 		//得到字段数量
// 		_db_query._field_count = mysql_num_fields( _db_query._mysql_res );
	}

	return _db_query;*/
	iResult = mysql_real_query( _db_ptr, sql, strlen(sql));
	if(iResult == 0)
	{
		//wzp add in 2012-9-24
		if(_db_query._mysql_res != NULL)
		{
			mysql_free_result(_db_query._mysql_res);
			_db_query._mysql_res = NULL;
		} 
		//wzp add end;
		_db_query._mysql_res = mysql_store_result( _db_ptr );
	}
	//--zhangmiao begin:11/28/2012
	else
	{
		string strError;
		char buffer[BUFSIZE]="";
		sprintf( buffer , "mysql param: sql=(%s)\n" , sql );
		strError = errmsg();
		strError += buffer;
		DBLOG((int &)ERROR_QUERY_TABLE,strError);
	}
	//--zhangmiao end:11/28/2012
	return _db_query;
}

/* 执行非返回结果查询 */
int CppMySQL3DB::execSQL(const char* sql)
{
	if( !mysql_real_query( _db_ptr, sql, strlen(sql) ) )
	{
		//得到受影响的行数
		return (int)mysql_affected_rows(_db_ptr) ;
	}
	else
	{
		//执行查询失败
		//--zhangmiao begin:11/28/2012
		string strError;
		char buffer[BUFSIZE]="";
		sprintf( buffer , "mysql param: sql=(%s)\n" , sql );
		strError = errmsg();
		strError += buffer;
		DBLOG( (int &)ERROR_EXEC_TABLE , strError);
		//--zhangmiao end:11/28/2012
		return -1;
	}
}

/* 测试mysql服务器是否存活 */
int CppMySQL3DB::ping()
{
	if( mysql_ping(_db_ptr) == 0 )
		return 0;
	else 
		return -1; 
}

/* 关闭mysql 服务器 */
int CppMySQL3DB::shutDown()
{
	if( mysql_shutdown(_db_ptr,SHUTDOWN_DEFAULT) == 0 )
		return 0;
	else 
		return -1;
}

/* 主要功能:重新启动mysql 服务器 */
int CppMySQL3DB::reboot()
{
	if(!mysql_reload(_db_ptr))
		return 0;
	else
		return -1;
}

/*
* 说明:事务支持InnoDB or BDB表类型
*/
/* 主要功能:开始事务 */
int CppMySQL3DB::startTransaction()
{
	if(!mysql_real_query(_db_ptr, "START TRANSACTION" ,
		(unsigned long)strlen("START TRANSACTION") ))
	{
		return 0;
	}
	else
	{
		//执行查询失败
		//--zhangmiao begin:11/28/2012
		string strError;
		char buffer[BUFSIZE]="";
		sprintf( buffer , "mysql param: START TRANSACTION\n" );
		strError = errmsg();
		strError += buffer;
		DBLOG( (int &)ERROR_START_TRANSACTION , strError);
		//--zhangmiao end:11/28/2012
		return -1;
	}
}

/* 主要功能:提交事务 */
int CppMySQL3DB::commit()
{
	if(!mysql_real_query( _db_ptr, "COMMIT",
		(unsigned long)strlen("COMMIT") ) )
	{
		return 0;
	}
	else
	{
		//执行查询失败
		//--zhangmiao begin:11/28/2012
		string strError;
		char buffer[BUFSIZE]="";
		sprintf( buffer , "mysql param:\"COMMIT\" \n" );
		strError = errmsg();
		strError += buffer;
		DBLOG( (int &)ERROR_COMMIT_TRANSACTION , strError);
		//--zhangmiao end:11/28/2012
		return -1;
	}
}

/* 主要功能:回滚事务 */
int CppMySQL3DB::rollback()
{
	if(!mysql_real_query(_db_ptr, "ROLLBACK",
		(unsigned long)strlen("ROLLBACK") ) )
		return 0;
	else
	{
		//执行查询失败
		//--zhangmiao begin:11/28/2012
		string strError;
		char buffer[BUFSIZE]="";
		sprintf( buffer , "mysql param:\"ROLLBACK\" \n" );
		strError = errmsg();
		strError += buffer;
		DBLOG( (int &)ERROR_ROLL_BACK_TRANSACTION , strError);
		//--zhangmiao end:11/28/2012
		return -1;
	}
}

/* 得到客户信息 */
const char * CppMySQL3DB::getClientInfo()
{
	return mysql_get_client_info();
}

/* 主要功能:得到客户版本信息 */
const unsigned long  CppMySQL3DB::getClientVersion()
{
	return mysql_get_client_version();
}

/* 主要功能:得到主机信息 */
const char * CppMySQL3DB::getHostInfo()
{
	return mysql_get_host_info(_db_ptr);
}

/* 主要功能:得到服务器信息 */
const char * CppMySQL3DB::getServerInfo()
{
	return mysql_get_server_info( _db_ptr ); 

}
/*主要功能:得到服务器版本信息*/
const unsigned long  CppMySQL3DB::getServerVersion()
{
	return mysql_get_server_version(_db_ptr);

}

/*主要功能:得到 当前连接的默认字符集*/
const char *  CppMySQL3DB::getCharacterSetName()
{
	return mysql_character_set_name(_db_ptr);

}

/* 得到系统时间 */
const char * CppMySQL3DB::getSysTime()
{
	//return ExecQueryGetSingValue("select now()");
	return NULL;

}

/* 建立新数据库 */
int CppMySQL3DB::createDB(const char* name)
{
	char temp[1024];

	sprintf(temp, "CREATE DATABASE %s", name);

	if(!mysql_real_query( _db_ptr, temp, strlen(temp)) )
		return 0;
	
	else
		//执行查询失败
		return -1;
}

/* 删除制定的数据库*/
int CppMySQL3DB::dropDB(const char*  name)
{
	char temp[1024];
	
	sprintf(temp, "DROP DATABASE %s", name);

	if(!mysql_real_query( _db_ptr, temp, strlen(temp)) )
		return 0;
	else
		//执行查询失败
		return -1;
}

bool CppMySQL3DB::tableExists(const char* table)
{
	return false;
}

u_int CppMySQL3DB::lastRowId()
{
	return 0;
}

#ifdef WRITE_DBELOG
int MySqlDB_WriteErrorLog( int& errorCode ,const std::string& strError )
{
    int ret = 0;
	//int errorCode = 0;
    //ret = C_LogManage::GetInstance()->WriteLog( iLevel,  iModule, iSubModule, errorCode, strError);
    g_db_logwrite.PrintLog(MyLogger::INFO,strError.c_str());
	return ret;
}
#endif // WRITE_DBELOG
