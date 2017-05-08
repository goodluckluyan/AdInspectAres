///  author:huangkui

#ifndef _PUB_H_
#define _PUB_H_

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <mysql.h>

#include <iostream>
///#include "db_defs.h"
////#include "SMS_Ret.h"
#include "moduleDefine.h"

///#include "./SMS_Types.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
///#include "zdb.h"



///#include "ec_config.h"

using namespace std;


#define MAX_BUFF_LEN	255

#define CONDITION_AND   1
#define CONDITION_OR    0

typedef struct _SQL_CONDITION_VALUE
{
	char    table_name[BUFF_SIZE_20];
	char    cloume_name[BUFF_SIZE_20];
	char    cloume_condition1[BUFF_SIZE_20];
	char    cloume_value1[BUFF_SIZE_20];
	char    cloume_condition2[BUFF_SIZE_20];
	char    cloume_value2[BUFF_SIZE_20];
	///value1和value2之间的关系“or”、“and”
	char    inside_relation[BUFF_SIZE_20];
	///与下一个条件的关系“or”、“and”
	char    outside_relation[BUFF_SIZE_20];

}SQL_CONDITION_VALUE;

typedef struct _SQL_CONDITION_RULE
{
	///排列查询结果的字段  
	char	order_cloume[BUFF_SIZE_20];
	///查询结果的顺序"desc"或者"asc"
	char    order[BUFF_SIZE_20];
	///查询limite的范围从max到min的查询结果
	char	limit_max[BUFF_SIZE_20];
	char    limit_min[BUFF_SIZE_20];
}SQL_CONDITION_RULE;

typedef vector<SQL_CONDITION_VALUE>  VEC_QUERY_VALUE;

typedef struct _COLUMN_VALUE_MAP
{
	char column_name[BUFF_SIZE_50];
	char *pvalue_var;
}COLUMN_VALUE_MAP;

class PubFun
{
public:
	~PubFun();


	PubFun();

public:
	/// gkm add 20150130
	int GetTableState(const char *tablename,int *pstate,char *pstata_desc,int maxlen_desc);

protected:

public:
	int FillSqlBufferByCondition(char* sql ,VEC_QUERY_VALUE sql_value,SQL_CONDITION_RULE sql_rule);	
	int FillSqlBufferByStr(char *sql, char* str);
    int FillCondition(VEC_QUERY_VALUE query,char* tablename,
                      char* cloumename, char* condition1,
                      char* value1, char *conditon2,
                      char* value2, char* in_relation,
                      char* out_relation, char*  order_name,
                      char* order, char* limitemin, char* limitemax);
	
	int GetCountsBySql(char	*count, char *sql);

	/// 获取单条信息
	int GetOneRowBySql(char *row_buffer, char *sqlbuffer);

	/// gengkeming add 20150318
	/// 更新
	int UpdateColumnValue(int id,const char *ptable,const char *pcolumn,const char *pvalue);

	/// 复制映射的数组
	int CopyMapArray(COLUMN_VALUE_MAP column_map_dest[],COLUMN_VALUE_MAP column_map_src[],int column_count);

	///// mysql 数据库链接池
	//Connection_T ConnectionPool_getConnection_pool(ConnectionPool_T mysql_pool);
	//ResultSet_T Connection_executeQuery_pool(Connection_T con,const char *psql);
	//void Connection_close_pool(Connection_T con);
	//int ResultSet_next_pool(ResultSet_T rset);
	//const char *ResultSet_getString_pool(ResultSet_T rset,int column_index);
	//void assert_pool(void *pvoid);

public:
	///获取数据库信息从配置文件中
	int GetDataBaseInfoFromConfigFile(char *pdbname,int maxlen_dbname,
												char *pipaddress,int maxlen_ip,
												char *pusername,int maxlen_username,
												char *ppassword,int maxlen_password);


	int MysqlQuery(MYSQL **conn,const char *psql);

	MYSQL_RES *MysqlUseResult(MYSQL *conn);

	MYSQL_ROW MysqlFetchRow(MYSQL_RES *pres);

	int MysqlFreeResult(MYSQL_RES *pres);
	/// 连接数据库
	int Connect(MYSQL **conn);
	/// 断开数据库连接
	int DisConnected(MYSQL **conn);

	///zyh add
	//sprintf(pipaddress,"172.23.142.186");
	//sprintf(pusername,"root");
	//sprintf(ppassword,"123456");
	//sprintf(pdbname,"templet_zhang");
	char m_database_ip[50];
	char m_database_username[50];
	char m_database_password[50];
    char m_database_dbname[50];
    char m_database_port[50];
 

};

#endif
