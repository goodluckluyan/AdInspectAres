#include "./pub.h"
#include "TempletModule.h"
///#include "./systemconfig/SMS_SystemConfig.h"
///#include "sms_timeconvert.h"
//#if defined(_WIN32)
//#include "../FileManager.h"
//#else
//#include "../FileManager_linux.h"
//#endif
#include "log/MyLogger.h"


///extern SystemConfig g_systemconfig;

//#if SMS_MUTEX_USE
//extern pthread_mutex_t g_mutex_connected_database_counter;
//extern pthread_mutex_t g_mutex_disconnected_database_counter;
//extern pthread_mutex_t g_mutex_sql_execute_counter;
//#endif
extern long long g_connected_database_counter;
extern long long g_disconnected_database_counter;
extern long long g_sql_execute_counter;
extern int g_iscanconnected_database;

extern char g_database_ip[50];
extern char g_database_dbname[50];
extern char g_database_username[50];
extern char g_database_password[50];
extern char g_database_port[50];


extern MyLogger g_templet_logwrite;

#define DEBUG_DATABASE_CONNECTED_COUNTER 0
#define DEBUG_DATABASE_DISCONNECTED_COUNTER 0
#define DEBUG_DATABASE_SQL_EXECUTE_COUNTER 0



#define ENABLE_RECORD_SQL_TO_DBLOGFILE 1


typedef struct _COLUMN_VALUE_MAPA
{
	char column_name[BUFF_SIZE_50];
	char *pvalue_var;
}COLUMN_VALUE_MAPA;

PubFun::PubFun()
{

}

PubFun::~PubFun()
{
}
int PubFun::GetDataBaseInfoFromConfigFile(char *pdbname,int maxlen_dbname,
												char *pipaddress,int maxlen_ip,
												char *pusername,int maxlen_username,
												char *ppassword,int maxlen_password)
{
	int ret = RET_SUCCESS;

#if 0
	char sec_name[BUFF_SIZE_50];
	char configfilepath[BUFF_SIZE_255];
	ec_config diskinfo_cfg;

	if(NULL == pdbname ||
		0 >= maxlen_dbname ||
		NULL == pipaddress ||
		0 >= maxlen_ip ||
		NULL == pusername||
		0 >= maxlen_username ||
		NULL == ppassword ||
		0 >= maxlen_password)
	{
		return PARAMETER_ERROR;
	}

	memset(configfilepath,0,sizeof(configfilepath));
	sprintf(configfilepath,g_sms_configfile);

#if 0
	printf("GetDataBaseInfoFromConfigFile:%s\n",m_diskconfigfilefullpath);
#endif

	memset(sec_name,0,sizeof(sec_name));
	strcpy(sec_name,"db_info");


	memset(pdbname,0,maxlen_dbname);
	memset(pipaddress,0,maxlen_ip);
	memset(pusername,0,maxlen_username);
	memset(ppassword,0,maxlen_password);


	ret = diskinfo_cfg.readvalue(sec_name,
									"database",
									pdbname,
									configfilepath);
	if(RET_SUCCESS != ret)
	{
		printf("faile to read config file:%s\n",configfilepath);
		return ret;
	}


	ret = diskinfo_cfg.readvalue(sec_name,
									"ip",
									pipaddress,
									configfilepath);
	if(RET_SUCCESS != ret)
	{
		printf("faile to read config file:%s\n",configfilepath);
		return ret;
	}


	ret = diskinfo_cfg.readvalue(sec_name,
									"username",
									pusername,
									configfilepath);
	if(RET_SUCCESS != ret)
	{
		printf("faile to read config file:%s\n",configfilepath);
		return ret;
	}

	ret = diskinfo_cfg.readvalue(sec_name,
									"password",
									ppassword,
									configfilepath);

	if(RET_SUCCESS != ret)
	{
		printf("faile to read config file:%s\n",configfilepath);
		return ret;
	}
#endif

	//g_systemconfig.GetDataBaseInfo_FromCfg(pdbname,maxlen_dbname,
	//											pipaddress,maxlen_ip,
	//											pusername,maxlen_username,
	//											ppassword,maxlen_password);

#if 0
	memset(pipaddress,0,maxlen_ip);
	memset(pusername,0,maxlen_dbname);
	memset(ppassword,0,maxlen_password);
	memset(pdbname,0,maxlen_dbname);

	sprintf(pipaddress,"172.23.142.186");
	sprintf(pusername,"root");
	sprintf(ppassword,"123456");
	sprintf(pdbname,"templet_zhang");
#endif
	

	return ret;
}

int PubFun::FillSqlBufferByCondition(char* sql ,VEC_QUERY_VALUE sql_value,SQL_CONDITION_RULE sql_rule)
{
    int ret = RET_SUCCESS;
	char  temp_sql[BUFF_SIZE_2048];
	char  sub_sql[BUFF_SIZE_2048];
	memset(temp_sql, 0, BUFF_SIZE_2048);
	memset(sub_sql, 0, BUFF_SIZE_2048);
	int vec_first_position = 0;
	int last_condistion_site =  sql_value.size() - 1;
	for(int i = vec_first_position; sql_value.size() > i; i++)
	{
		if (i == vec_first_position)
		{

            if(strcmp(sql, "") == 0  && strcmp(sql_value[vec_first_position].table_name, "") != 0)
			{
				sprintf(sql, "select * from %s  where", sql_value[vec_first_position].table_name);

			}
            else
            {
                ret = PARAMETER_ERROR;
            }

		}

      //  if (sql_value[i].cloume_value2 != '\0' && sql_value[i].cloume_value2 != '\0')
        if(strcmp(sql_value[i].cloume_value1, "" ) != 0  &&  strcmp(sql_value[i].cloume_value2, "") != 0 )
        {
			sprintf(temp_sql, " (`%s` ", sql_value[i].cloume_name);
            strcat(sub_sql, temp_sql);
			sprintf(temp_sql, " %s", sql_value[i].cloume_condition1);
			strcat(sub_sql, temp_sql);
			sprintf(temp_sql, " \"%s\"", sql_value[i].cloume_value1);
			strcat(sub_sql, temp_sql);
			sprintf(temp_sql, " %s", sql_value[i].inside_relation);
			strcat(sub_sql, temp_sql);
			sprintf(temp_sql, " `%s`", sql_value[i].cloume_name);
			strcat(sub_sql, temp_sql);
			sprintf(temp_sql, " %s", sql_value[i].cloume_condition2);
			strcat(sub_sql, temp_sql);
			sprintf(temp_sql, " \"%s\" )", sql_value[i].cloume_value2);
			strcat(sub_sql, temp_sql);
			if(last_condistion_site != i)
			{
				sprintf(temp_sql, " %s", sql_value[i].outside_relation);
				strcat(sub_sql, temp_sql);
			}
		}
	}
    if (sql_rule.order_cloume[0] !=  '\0' && sql_rule.order != '\0')
	{
		sprintf(temp_sql, " order by `%s` %s ", sql_rule.order_cloume, sql_rule.order);
		strcat(sub_sql, temp_sql);
	}
    if(sql_rule.limit_max[0] != '\0' && sql_rule.limit_min[0] != '\0')
	{
		sprintf(temp_sql, " limit %s , %s ", sql_rule.limit_min, sql_rule.limit_max);
		strcat(sub_sql, temp_sql);
	}
	strcat(sql, sub_sql);	
	char temp_sql_test[2048] = {0};
	sprintf(temp_sql_test, "select * from log ");
	return RET_SUCCESS;

}

int PubFun::FillSqlBufferByStr(char *sql, char* str)
{
	int ret = RET_SUCCESS;

    if(str != NULL &&  sql != NULL)
    {
        strcat(sql, str);
    }

	return ret;
}
int PubFun::FillCondition(VEC_QUERY_VALUE query,char* tablename, char* cloumename, char* condition1, char* value1, char *conditon2, char* value2, char* in_relation, char* out_relation
												, char*  order_name, char* order, char* limitemin, char* limitemax)
{
	int ret = RET_SUCCESS;	
	SQL_CONDITION_VALUE str_value;
	SQL_CONDITION_RULE str_rule;
	memset(&str_value, 0, sizeof(SQL_CONDITION_RULE));
	memset(&str_rule, 0, sizeof(SQL_CONDITION_VALUE));
	sprintf(str_value.table_name, tablename);
	sprintf(str_value.cloume_name, cloumename);
	sprintf(str_value.cloume_condition1, condition1);
	sprintf(str_value.cloume_value1, value1);
	sprintf(str_value.cloume_condition2, conditon2);
	sprintf(str_value.cloume_value2, value2);
	sprintf(str_value.inside_relation, in_relation);
	sprintf(str_value.outside_relation, out_relation);

	if(strcmp(order_name, "") != 0 || strcmp(order, "") != 0)
	{
		sprintf(str_rule.order_cloume, order_name);
		sprintf(str_rule.order, order);
	}

	if(strcmp(limitemin, "") != 0 || strcmp(limitemax, "") != 0)
	{
		sprintf(str_rule.limit_min, limitemin);
		sprintf(str_rule.limit_max, limitemax);
	}
		
	query.push_back(str_value);
	return ret;
}
int PubFun::MysqlQuery(MYSQL **conn,const char *psql)
{
	int ret = DB_SUCCESS;
	int istry_ok = 0;
	int retry_counter = 30;
	int index = 0;
	int sql_len = 0;
	char *prompt_buff_1 = NULL;
	char prompt_buff[BUFF_SIZE_2048]={'\0'};
	///sms_timeconvert timeconvert;
	///CFileManager filemanager;

	sql_len = strlen(psql);

	if(sql_len>(BUFF_SIZE_2048-512))
	{
		prompt_buff_1 = new char[sql_len+512];
		memset(prompt_buff_1,0,sql_len+512);
	}
	//else
	//{
	//	memset(prompt_buff,0,sizeof(prompt_buff));
	//}

	if(!g_iscanconnected_database)
	{
		return DB_CONNECTED_ERROR;
	}

#if ENABLE_RECORD_SQL_TO_DBLOGFILE
	//if(strcmp(g_database_logfile_fullpath,""))
	//{
		g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",psql);
	//}
#endif

#if DEBUG_DATABASE_SQL_EXECUTE_COUNTER

	if(sql_len>(BUFF_SIZE_2048-512))
	{
		///pthread_mutex_lock(&g_mutex_sql_execute_counter);
		g_sql_execute_counter++;
		sprintf(prompt_buff_1,"query sql:%s-->%s,total counter:%lld,sql:%s\n",g_database_ip,g_database_dbname,g_sql_execute_counter,psql);
		///pthread_mutex_unlock(&g_mutex_sql_execute_counter);
		g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff_1);

	}
	else
	{
		///pthread_mutex_lock(&g_mutex_sql_execute_counter);
		g_sql_execute_counter++;
		sprintf(prompt_buff,"query sql:%s-->%s,total counter:%lld,sql:%s\n",g_database_ip,g_database_dbname,g_sql_execute_counter,psql);
		///pthread_mutex_unlock(&g_mutex_sql_execute_counter);
		g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff);

	}

#endif


	for(int index = 0; index < retry_counter; index++)
	{
		if(NULL == *conn)
		{
			if(sql_len>(BUFF_SIZE_2048-512))
			{
				memset(prompt_buff_1,0,sql_len+512);
				sprintf(prompt_buff_1,"error:\n");
				sprintf(prompt_buff_1,"%s database ip:%s\n",prompt_buff_1,g_database_ip);
				sprintf(prompt_buff_1,"%s database name:%s\n",prompt_buff_1,g_database_dbname);
				sprintf(prompt_buff_1,"%s database username:%s\n",prompt_buff_1,g_database_username);
				sprintf(prompt_buff_1,"%s database password:%s\n",prompt_buff_1,g_database_password);
				sprintf(prompt_buff_1,"%s conn is null,query database error:%u: %s,trycounter:%d/%d\n",prompt_buff_1,mysql_errno(*conn), mysql_error(*conn),index,retry_counter);
				sprintf(prompt_buff_1,"%s sql:%s\n",prompt_buff_1,psql);

				printf("111111 prompt_buff_1:%s\n",prompt_buff_1);

				g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff_1);

			}
			else
			{
				memset(prompt_buff,0,sizeof(prompt_buff));
				sprintf(prompt_buff,"error:\n");
				sprintf(prompt_buff,"%s database ip:%s\n",prompt_buff,g_database_ip);
				sprintf(prompt_buff,"%s database name:%s\n",prompt_buff,g_database_dbname);
				sprintf(prompt_buff,"%s database username:%s\n",prompt_buff,g_database_username);
				sprintf(prompt_buff,"%s database password:%s\n",prompt_buff,g_database_password);
				sprintf(prompt_buff,"%s conn is null,query database error:%u: %s,trycounter:%d/%d\n",prompt_buff,mysql_errno(*conn), mysql_error(*conn),index,retry_counter);
				sprintf(prompt_buff,"%s sql:%s\n",prompt_buff,psql);

				printf("2222222 prompt_buff_1:%s\n",prompt_buff_1);

				g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff);


			}



			DisConnected(conn);
			sleep(1);
			Connect(conn);

			if(NULL == *conn)
			{
				sleep(1);
				continue;
			}

		}


		if (mysql_query(*conn, psql))
		{
			if(sql_len>(BUFF_SIZE_2048-512))
			{
				memset(prompt_buff_1,0,sql_len+512);
				sprintf(prompt_buff_1,"error:\n");
				sprintf(prompt_buff_1,"%s database ip:%s\n",prompt_buff_1,g_database_ip);
				sprintf(prompt_buff_1,"%s database name:%s\n",prompt_buff_1,g_database_dbname);
				sprintf(prompt_buff_1,"%s database username:%s\n",prompt_buff_1,g_database_username);
				sprintf(prompt_buff_1,"%s database password:%s\n",prompt_buff_1,g_database_password);
				sprintf(prompt_buff_1,"%s query database error:%u: %s,trycounter:%d/%d\n",prompt_buff_1,mysql_errno(*conn), mysql_error(*conn),index,retry_counter);
				sprintf(prompt_buff_1,"%s sql:%s\n",prompt_buff_1,psql);
				
				printf("%s\n",prompt_buff);
				
				g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff_1);
			}
			else
			{
				memset(prompt_buff,0,sizeof(prompt_buff));
				sprintf(prompt_buff,"error:\n");
				sprintf(prompt_buff,"%s database ip:%s\n",prompt_buff,g_database_ip);
				sprintf(prompt_buff,"%s database name:%s\n",prompt_buff,g_database_dbname);
				sprintf(prompt_buff,"%s database username:%s\n",prompt_buff,g_database_username);
				sprintf(prompt_buff,"%s database password:%s\n",prompt_buff,g_database_password);
				sprintf(prompt_buff,"%s query database error:%u: %s,trycounter:%d/%d\n",prompt_buff,mysql_errno(*conn), mysql_error(*conn),index,retry_counter);
				sprintf(prompt_buff,"%s sql:%s\n",prompt_buff,psql);
				
				printf("%s\n",prompt_buff);

				g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff);
			}


			DisConnected(conn);
			sleep(1);
			
			ret = Connect(conn);
			if(DB_SUCCESS != ret)
			{
				istry_ok = 0;
				return ret;
			}
			
			//return DB_COMMAND_QUERY_ERROR;

		}
		else
		{
			istry_ok = 1;
			break;
		}
	}

	if(NULL != prompt_buff_1)
	{
		delete [] prompt_buff_1;
		prompt_buff_1 = NULL;
	}

	if(!istry_ok)
	{
		return DB_COMMAND_QUERY_ERROR;
	}

	return ret;
}

MYSQL_RES *PubFun::MysqlUseResult(MYSQL *conn)
{
	int ret = DB_SUCCESS;
	MYSQL_RES *pres = NULL;
	pres = mysql_use_result(conn);
	return pres;
}

MYSQL_ROW PubFun::MysqlFetchRow(MYSQL_RES *pres)
{
	return mysql_fetch_row(pres);
}

int PubFun::MysqlFreeResult(MYSQL_RES *pres)
{
	int ret = DB_SUCCESS;
	mysql_free_result(pres);
	return ret;
}

int PubFun::Connect(MYSQL **conn)
{
	int ret = DB_SUCCESS;
#if 0
	char server[BUFF_SIZE_255];
	char username[BUFF_SIZE_255];
	char password[BUFF_SIZE_255];
	char dbname[BUFF_SIZE_255];	
#endif
	char prompt_buff[BUFF_SIZE_1024]={'\0'};
////	sms_timeconvert timeconvert;
	int istry_ok = 0;
	int retry_counter = 30;
	int index = 0;

	if(!g_iscanconnected_database)
	{
		return DB_CONNECTED_ERROR;
	}

#if 0
	for(index = 0; index < retry_counter; index++)
	{
		ret = GetDataBaseInfoFromConfigFile(dbname,
									sizeof(dbname),
									server,
									sizeof(server),
									username,
									sizeof(username),
									password,
									sizeof(password));
		if(RET_SUCCESS != ret)
		{
			memset(prompt_buff,0,sizeof(prompt_buff));
			sprintf(prompt_buff,"error:\n");
			sprintf(prompt_buff,"%s database ip:%s\n",prompt_buff,server);
			sprintf(prompt_buff,"%s database name:%s\n",prompt_buff,dbname);
			sprintf(prompt_buff,"%s database username:%s\n",prompt_buff,username);
			sprintf(prompt_buff,"%s database password:%s\n",prompt_buff,password);
			sprintf(prompt_buff,"%s read database error,from config file:trycounter:%d/%d\n", prompt_buff,index,retry_counter);
			g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff);

			msleep(1000);
			//return DB_READ_CONFIG_FILE_ERROR;
		}
		else
		{
			istry_ok = 1;
			break;
		}
	}
	if(!istry_ok)
	{
		ret = DB_READ_CONFIG_FILE_ERROR;
	}
#endif

	istry_ok = 0;

	/// 初始化连接对象
	for(index = 0; index < retry_counter; index++)
	{

		*conn = mysql_init(NULL);

		if( NULL == *conn)
		{
			memset(prompt_buff,0,sizeof(prompt_buff));
			sprintf(prompt_buff,"error:\n");
			sprintf(prompt_buff,"%s database ip:%s\n",prompt_buff,g_database_ip);
			sprintf(prompt_buff,"%s database name:%s\n",prompt_buff,g_database_dbname);
			sprintf(prompt_buff,"%s database username:%s\n",prompt_buff,g_database_username);
			sprintf(prompt_buff,"%s database password:%s\n",prompt_buff,g_database_password);
			sprintf(prompt_buff,"%s init connect database error:%u:%s,trycounter:%d/%d\n", prompt_buff,mysql_errno(*conn), mysql_error(*conn),index,retry_counter);
			DisConnected(conn);
			g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff);
			msleep(1000);
			continue;
			//return DB_INIT_ERROR;
		}

		/// 连接数据库
		if (!mysql_real_connect(*conn, 
								g_database_ip,
								g_database_username,
								g_database_password,
								g_database_dbname, 
								0, 
								NULL, 
								0)) 
		{

			memset(prompt_buff,0,sizeof(prompt_buff));
			sprintf(prompt_buff,"error:\n");
			sprintf(prompt_buff,"%s database ip:%s\n",prompt_buff,g_database_ip);
			sprintf(prompt_buff,"%s database name:%s\n",prompt_buff,g_database_dbname);
			sprintf(prompt_buff,"%s database username:%s\n",prompt_buff,g_database_username);
			sprintf(prompt_buff,"%s database password:%s\n",prompt_buff,g_database_password);
			sprintf(prompt_buff,"%s connect database error:%u: %s,trycounter:%d/%d\n", prompt_buff,mysql_errno(*conn), mysql_error(*conn),index,retry_counter);
			DisConnected(conn);
			g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff);
			msleep(1000);
			//return DB_CONNECTED_ERROR;

#if DEBUG_DATABASE_CONNECTED_COUNTER
			memset(prompt_buff,0,sizeof(prompt_buff));
			///pthread_mutex_lock(&g_mutex_connected_database_counter);
			g_connected_database_counter++;
			sprintf(prompt_buff,"connected database ip:%s-->%s,trycounter:%d/%d,total counter:%lld\n",g_database_ip,g_database_dbname,index,retry_counter,g_connected_database_counter);

			g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff);

			///pthread_mutex_unlock(&g_mutex_connected_database_counter);
#endif
		}
		else
		{
			if ( mysql_set_character_set( *conn, "utf8" )  != 0 )
			{
				memset(prompt_buff,0,sizeof(prompt_buff));
				sprintf(prompt_buff,"error:\n");
				sprintf(prompt_buff,"%s database ip:%s\n",prompt_buff,g_database_ip);
				sprintf(prompt_buff,"%s database name:%s\n",prompt_buff,g_database_dbname);
				sprintf(prompt_buff,"%s database username:%s\n",prompt_buff,g_database_username);
				sprintf(prompt_buff,"%s database password:%s\n",prompt_buff,g_database_password);
				sprintf(prompt_buff,"%s failed to set character set to utf8:%u: %s\n", prompt_buff,mysql_errno(*conn), mysql_error(*conn));
				DisConnected(conn);
				g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff);
				//fprintf ( error_detail , "错误, %s/n" , mysql_error( *conn) ) ;
			}

#if DEBUG_DATABASE_CONNECTED_COUNTER
			memset(prompt_buff,0,sizeof(prompt_buff));
			pthread_mutex_lock(&g_mutex_connected_database_counter);

			g_connected_database_counter++;
			sprintf(prompt_buff,"connected database ip:%s-->%s,trycounter:%d/%d,total counter:%lld\n",g_database_ip,g_database_dbname,index,retry_counter,g_connected_database_counter);
		
			pthread_mutex_unlock(&g_mutex_connected_database_counter);

			g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff);

#endif
			istry_ok = 1;
			break;
		}

	}

	if(!istry_ok)
	{
		ret = DB_CONNECTED_ERROR;
	}
	
	return ret;
}

int PubFun::DisConnected(MYSQL **conn)
{
	int ret = DB_SUCCESS;
	char prompt_buff[BUFF_SIZE_1024]={'\0'};
	///sms_timeconvert timeconvert;

	if(NULL != *conn)
	{
		/// 断开数据库连接
		mysql_close(*conn);
		*conn = NULL;
#if 0
		msleep(100);
#endif	

#if DEBUG_DATABASE_DISCONNECTED_COUNTER
		memset(prompt_buff,0,sizeof(prompt_buff));
		///pthread_mutex_lock(&g_mutex_disconnected_database_counter);
		g_disconnected_database_counter++;
		sprintf(prompt_buff,"disconnected database ip:%s-->%s,total counter:%lld\n",g_database_ip,g_database_dbname,g_disconnected_database_counter);
		///pthread_mutex_unlock(&g_mutex_disconnected_database_counter);
		g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff);

#endif		
	}


	return ret;
}


int PubFun::GetCountsBySql(char	*count, char *sqlbuffer)
{
    int ret=DB_SUCCESS;
    char sql[BUFF_SIZE_2048]={'\0'};
    int column_count = 0;
    int column_index = 0;
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;


   
    /// 连接数据库
    ret = Connect(&conn);
    if( DB_SUCCESS != ret)
    {
        return ret;
    }

    memset(sql,0,sizeof(sql));
    strcat(sql, sqlbuffer);

    /// 发送查询命令
	if (MysqlQuery(&conn,sql))
	{
		DisConnected(&conn);
		return DB_COMMAND_QUERY_ERROR;
	}

	pres = MysqlUseResult(conn);

	while((row = MysqlFetchRow(pres)) != NULL)
	{		
        //strcpy(count, (char *)row[0]);
        column_count++;
	}
    itoa(column_count, count, 10);
    /// 释放结果
    MysqlFreeResult(pres);

    /// 断开数据库连接
    ret = DisConnected(&conn);
    return ret;
}
int PubFun::GetTableState(const char *tablename,int *pstate,char *pstata_desc,int maxlen_desc)
{
	int ret = RET_SUCCESS;
	char sql[BUFF_SIZE_2048]={'\0'};
	int column_count = 0;
	int column_index = 0;
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;

	if( NULL == tablename ||
		(!strcmp(tablename,"")) ||
		NULL == pstate ||
		NULL == pstata_desc ||
		maxlen_desc <= 0)
	{
		return PARAMETER_ERROR;
	}

	*pstate = 1;

	/// 连接数据库
	ret = Connect(&conn);
	if( RET_SUCCESS != ret)
	{
		return ret;
	}

	memset(sql,0,sizeof(sql));
	sprintf(sql,"check table %s fast quick",tablename);

	/// 发送查询命令
	if (MysqlQuery(&conn,sql))
	{
        
        DisConnected(&conn);
		return DB_COMMAND_QUERY_ERROR;
	}

	/// 获取结果
	pres = MysqlUseResult(conn);
	
	/// 获取结果中各信息项
	char table_column[BUFF_SIZE_50];
	char op_column[BUFF_SIZE_50];
	char msg_type[BUFF_SIZE_50];
	char msg_text[BUFF_SIZE_50];

	memset(table_column,0,sizeof(table_column));
	memset(op_column,0,sizeof(op_column));
	memset(msg_type,0,sizeof(msg_type));
	memset(msg_text,0,sizeof(msg_text));

	while ((row = MysqlFetchRow(pres)) != NULL)
	{		

		strcpy(table_column,(char *)row[0]);
		strcpy(op_column,(char *)row[1]);
		strcpy(msg_type,(char *)row[2]);
		strcpy(msg_text,(char *)row[3]);

		break;

	}

	memset(pstata_desc,0,maxlen_desc);
	sprintf(pstata_desc,"%s:%s:%s:%s",table_column,op_column,msg_type,msg_text);

	if((!strcmp(msg_text,"OK")) ||
		(!strcmp(msg_text,"Table is already up to date")))
	{
		*pstate = 0;
		/// 释放结果
		MysqlFreeResult(pres);

		/// 断开数据库连接
		ret = DisConnected(&conn);

		return RET_SUCCESS;
	}
	
	/// 释放结果
	MysqlFreeResult(pres);
	
	
	/// 断开数据库连接
	ret = DisConnected(&conn);


	return DB_COMMAND_QUERY_ERROR;
}




int PubFun::GetOneRowBySql(char	*row_buffer, char *sqlbuffer)
{
    int ret=DB_SUCCESS;
    char sql[BUFF_SIZE_2048]={'\0'};
    int column_count = 0;
    int column_index = 0;
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;


   
    /// 连接数据库
    ret = Connect(&conn);
    if( DB_SUCCESS != ret)
    {
        return ret;
    }

    memset(sql,0,sizeof(sql));
    strcat(sql, sqlbuffer);

    /// 发送查询命令
	if (MysqlQuery(&conn,sql))
	{
		DisConnected(&conn);
		return DB_COMMAND_QUERY_ERROR;
	}

	pres = MysqlUseResult(conn);

	while((row = MysqlFetchRow(pres)) != NULL)
	{		
        strcpy(row_buffer, (char *)row[0]);
        
	}
    /// 释放结果
    MysqlFreeResult(pres);

    /// 断开数据库连接
    ret = DisConnected(&conn);
	
    return ret;
}
int PubFun::UpdateColumnValue(int id,const char *ptable,const char *pcolumn,const char *pvalue)
{
	int ret=DB_SUCCESS;
	char sql[BUFF_SIZE_2048]={'\0'};
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;

	if( id <=0 ||
		NULL == ptable ||
		(!strcmp(ptable,"")) ||
		NULL == pcolumn ||
		(!strcmp(pcolumn,"")) ||
		NULL == pvalue ||
		(!strcmp(pvalue,"")) )
	{
		return PARAMETER_ERROR;
	}

	/// 连接数据库
	ret = Connect(&conn);
	if( DB_SUCCESS != ret)
	{
		return ret;
	}


	memset(sql,0,sizeof(sql));

	sprintf(sql,"update %s set `%s`='%s' where `id` = '%d'",
				ptable,
				pcolumn,
				pvalue,
				id);


	/// 发送查询命令
	if (MysqlQuery(&conn,sql)) 
	{
		
		DisConnected(&conn);
		return DB_COMMAND_QUERY_ERROR;
	}

	/// 断开数据库连接
	ret = DisConnected(&conn);

	return ret;
}
int PubFun::CopyMapArray(COLUMN_VALUE_MAP column_map_dest[],COLUMN_VALUE_MAP column_map_src[],int column_count)
{
	int ret=0;

#if 0
	printf("column_count:%d\n",column_count);
#endif

	for(int i = 0;i<column_count;i++)
	{
		memset(column_map_dest[i].column_name,0,sizeof(column_map_dest[i].column_name));
		strcpy(column_map_dest[i].column_name,column_map_src[i].column_name);
		column_map_dest[i].pvalue_var = column_map_src[i].pvalue_var;
#if 0
		printf("column_map_dest[%d]:%s\n",i,column_map_dest[i].column_name);
#endif
	}


	return ret;
}
#if 0
/// mysql 数据库链接池
Connection_T PubFun::ConnectionPool_getConnection_pool(ConnectionPool_T mysql_pool)
{
    Connection_T conn = NULL;
	char prompt_buff[BUFF_SIZE_1024]={'\0'};
	sms_timeconvert timeconvert;
    int i=0;

    conn = ConnectionPool_getConnection(mysql_pool);

    while(NULL == conn)
    {
        conn = ConnectionPool_getConnection(mysql_pool);

		printf("======================================================\n");
		memset(prompt_buff,0,sizeof(prompt_buff));
		sprintf(prompt_buff,"error:\n");
		sprintf(prompt_buff,"%s database ip:%s\n",prompt_buff,g_database_ip);
		sprintf(prompt_buff,"%s database name:%s\n",prompt_buff,g_database_dbname);
		sprintf(prompt_buff,"%s database username:%s\n",prompt_buff,g_database_username);
		sprintf(prompt_buff,"%s database password:%s\n",prompt_buff,g_database_password);
		sprintf(prompt_buff,"%s ConnectionPool_getConnection retry counter:%d,con:%08X\n", prompt_buff,i++,conn);
		g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff);

		printf("======================================================\n");
		sleep(1);
    }
    return conn;
}
ResultSet_T PubFun::Connection_executeQuery_pool(Connection_T con,const char *psql)
{
    ResultSet_T prset = NULL;
	char prompt_buff[BUFF_SIZE_2048]={'\0'};
	int retry_counter=0;
	int sql_len = 0;
	char *prompt_buff_1= NULL;
	sms_timeconvert timeconvert;
	CFileManager filemanager;

	sql_len = strlen(psql);

	if(sql_len>(BUFF_SIZE_2048-512))
	{
		prompt_buff_1 = new char[sql_len+512];
		memset(prompt_buff_1,0,sql_len+512);
	}
	//else
	//{
	//	memset(prompt_buff,0,sizeof(prompt_buff));
	//}


#if ENABLE_RECORD_SQL_TO_DBLOGFILE

	//if(strcmp(g_database_logfile_fullpath,""))
	//{
		g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",psql);
	//}
#endif


	prset = Connection_executeQuery(con,psql);

    while(NULL == prset)
    {
        prset = Connection_executeQuery(con,psql);

		printf("---------------------------\n");


		if(sql_len>(BUFF_SIZE_2048-512))
		{
			memset(prompt_buff_1,0,sql_len+512);
			sprintf(prompt_buff_1,"error:\n");
			sprintf(prompt_buff_1,"%s database ip:%s\n",prompt_buff_1,g_database_ip);
			sprintf(prompt_buff_1,"%s database name:%s\n",prompt_buff_1,g_database_dbname);
			sprintf(prompt_buff_1,"%s database username:%s\n",prompt_buff_1,g_database_username);
			sprintf(prompt_buff_1,"%s database password:%s\n",prompt_buff_1,g_database_password);
			sprintf(prompt_buff_1,"%s Connection_executeQuery retry counter:%d,rset:%08X\n", prompt_buff_1,retry_counter++,prset);
			g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff_1);
		}
		else
		{
			memset(prompt_buff,0,sizeof(prompt_buff));
			sprintf(prompt_buff,"error:\n");
			sprintf(prompt_buff,"%s database ip:%s\n",prompt_buff,g_database_ip);
			sprintf(prompt_buff,"%s database name:%s\n",prompt_buff,g_database_dbname);
			sprintf(prompt_buff,"%s database username:%s\n",prompt_buff,g_database_username);
			sprintf(prompt_buff,"%s database password:%s\n",prompt_buff,g_database_password);
			sprintf(prompt_buff,"%s Connection_executeQuery retry counter:%d,rset:%08X\n", prompt_buff,retry_counter++,prset);
			g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",prompt_buff);
		}

		printf("---------------------------\n");
        sleep(1);
    }

	if(NULL != prompt_buff_1)
	{
		delete [] prompt_buff_1;
		prompt_buff_1 = NULL;
	}

    return prset;
}
void PubFun::Connection_close_pool(Connection_T con)
{
    if(NULL != con)
    {
        Connection_close(con);
    }
}
int PubFun::ResultSet_next_pool(ResultSet_T rset)
{
	return ResultSet_next(rset);
}
const char *PubFun::ResultSet_getString_pool(ResultSet_T rset,int column_index)
{
	return ResultSet_getString(rset,column_index);
}
void PubFun::assert_pool(void *pvoid)
{
	assert(pvoid);
}
#endif
