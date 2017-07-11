#include "task_table.h"
#include <string.h>
///#include "zdb.h"

COLUMN_VALUE_MAP g_task_column_value_map[]=
{
	{"id",NULL},
	{"uuid",NULL},
	{"orig_fileName",NULL},
	{"fileName",NULL},
	{"filePath",NULL},
	{"fullFilePath",NULL},
	{"startDateTime",NULL},
	{"endDateTime",NULL},
	{"hall_id",NULL},
	{"frameRate",NULL},
	{"frequency",NULL},
    {"type",NULL},
	{"videoWidth",NULL},
	{"videoHeight",NULL},
	{"dstVideoWidth",NULL},
	{"dstVideoHeight",NULL},
	{"realDuration",NULL},
	{"featureFilePath",NULL},
	{"ad_order",NULL},
	{"show_type",NULL},
	{"description",NULL}
};

typedef struct _COLUMN_INFO
{
	char column_name[200];
	char *pcolumn_value;
	int type;
	int length;
}COLUMN_INFO;


TaskTable::TaskTable()
{

	CopyMapArray(m_task_column_value_map,g_task_column_value_map,sizeof(g_task_column_value_map)/sizeof(g_task_column_value_map[0]));

}
TaskTable::~TaskTable()
{

}
void TaskTable::ClearTableItemSpace(TASK_ITEM *task_item)
{
	if(NULL==task_item)
	{
		return;
	}
	if(NULL!=task_item->uuid)
	{
		memset(task_item->id,0,BUFF_SIZE_50);
		memset(task_item->uuid,0,BUFF_SIZE_50);
	}
	
	if(NULL!=task_item->orig_fileName)
	{
		memset(task_item->orig_fileName,0,BUFF_SIZE_255);
	}
	if(NULL!=task_item->fileName)
	{
		memset(task_item->fileName,0,BUFF_SIZE_255);
	}
	if(NULL!=task_item->filePath)
	{
		memset(task_item->filePath,0,BUFF_SIZE_255);
	}
	if(NULL!=task_item->startDateTime)
	{
		memset(task_item->startDateTime,0,BUFF_SIZE_50);
	}
	if(NULL!=task_item->endDateTime)
	{
		memset(task_item->endDateTime,0,BUFF_SIZE_50);
	}
	if(NULL!=task_item->hall_id)
	{
		memset(task_item->hall_id,0,BUFF_SIZE_50);
	}
	if(NULL!=task_item->frameRate)
	{
		memset(task_item->frameRate,0,BUFF_SIZE_50);
	}
	if(NULL!=task_item->frequency)
	{
		memset(task_item->frequency,0,BUFF_SIZE_50);
	}
	if(NULL!=task_item->type)
	{
		memset(task_item->type,0,BUFF_SIZE_50);
	}
	if(NULL!=task_item->description)
	{
		memset(task_item->description,0,BUFF_SIZE_50);
	}
	if(NULL!=task_item->fullFilePath)
	{
		memset(task_item->fullFilePath,0,BUFF_SIZE_255);
	}
	if(NULL!=task_item->videoHeight)
	{
		memset(task_item->videoHeight,0,BUFF_SIZE_50);
	}
	if(NULL!=task_item->videoWidth)
	{
		memset(task_item->videoWidth,0,BUFF_SIZE_50);
	}
	if(NULL!=task_item->dstVideoHeight)
	{
		memset(task_item->dstVideoHeight,0,BUFF_SIZE_50);
	}
	if(NULL!=task_item->dstVideoWidth)
	{
		memset(task_item->dstVideoWidth,0,BUFF_SIZE_50);
	}
	if(NULL!=task_item->realDuration)
	{
		memset(task_item->realDuration,0,BUFF_SIZE_50);
	}
	if(NULL!=task_item->featureFilePath)
	{
		memset(task_item->featureFilePath,0,BUFF_SIZE_255);
	}
	if(NULL!=task_item->ad_order)
	{
		memset(task_item->ad_order,0,BUFF_SIZE_50);
	}
	if(NULL!=task_item->show_type)
	{
		memset(task_item->show_type,0,BUFF_SIZE_50);
	}
}

void TaskTable::NewTableItemSpace(TASK_ITEM **task_item)
{
	if(NULL!=*task_item)
	{
		return;
	}

	*task_item=(TASK_ITEM *)new TASK_ITEM;


	(*task_item)->id=new char[BUFF_SIZE_50];
	(*task_item)->uuid=new char[BUFF_SIZE_50];
	(*task_item)->orig_fileName=new char[BUFF_SIZE_255];
	(*task_item)->fileName=new char[BUFF_SIZE_255];
	(*task_item)->filePath=new char[BUFF_SIZE_255];
	(*task_item)->startDateTime=new char[BUFF_SIZE_50];
	(*task_item)->endDateTime=new char[BUFF_SIZE_50];
	(*task_item)->hall_id=new char[BUFF_SIZE_50];
	(*task_item)->frameRate=new char[BUFF_SIZE_50];
	(*task_item)->frequency=new char[BUFF_SIZE_50];
	(*task_item)->type=new char[BUFF_SIZE_50];
	(*task_item)->description=new char[BUFF_SIZE_50];
	(*task_item)->fullFilePath=new char[BUFF_SIZE_255];
	(*task_item)->videoHeight=new char[BUFF_SIZE_50];
	(*task_item)->videoWidth=new char[BUFF_SIZE_50];
	(*task_item)->dstVideoHeight=new char[BUFF_SIZE_50];
	(*task_item)->dstVideoWidth=new char[BUFF_SIZE_50];
	(*task_item)->realDuration=new char[BUFF_SIZE_50];
	(*task_item)->featureFilePath=new char[BUFF_SIZE_255];
	(*task_item)->ad_order=new char[BUFF_SIZE_50];
	(*task_item)->show_type=new char[BUFF_SIZE_50];
	ClearTableItemSpace(*task_item);
}

void TaskTable::DeleteItemSpace(TASK_ITEM **task_item)
{
	if(NULL==(*task_item))
	{
		return;
	}

	ClearTableItemSpace(*task_item);

	if(NULL!=(*task_item)->uuid)
	{
		delete [] (*task_item)->id;
		(*task_item)->id=NULL;
		delete [] (*task_item)->uuid;
		(*task_item)->uuid=NULL;
	}
	if(NULL!=(*task_item)->orig_fileName)
	{
		delete [] (*task_item)->orig_fileName;
		(*task_item)->orig_fileName=NULL;
	}
	if(NULL!=(*task_item)->fileName)
	{
		delete [] (*task_item)->fileName;
		(*task_item)->fileName=NULL;
	}
	if(NULL!=(*task_item)->filePath)
	{
		delete [] (*task_item)->filePath;
		(*task_item)->filePath=NULL;
	}
	if(NULL!=(*task_item)->startDateTime)
	{
		delete [] (*task_item)->startDateTime;
		(*task_item)->startDateTime=NULL;
	}
	if(NULL!=(*task_item)->endDateTime)
	{
		delete [] (*task_item)->endDateTime;
		(*task_item)->endDateTime=NULL;
	}
	if(NULL!=(*task_item)->hall_id)
	{
		delete [] (*task_item)->hall_id;
		(*task_item)->hall_id=NULL;
	}
	if(NULL!=(*task_item)->frameRate)
	{
		delete [] (*task_item)->frameRate;
		(*task_item)->frameRate=NULL;
	}
	if(NULL!=(*task_item)->frequency)
	{
		delete [] (*task_item)->frequency;
		(*task_item)->frequency=NULL;
	}
	if(NULL!=(*task_item)->type)
	{
		delete [] (*task_item)->type;
		(*task_item)->type=NULL;
	}
	if(NULL!=(*task_item)->description)
	{
		delete [] (*task_item)->description;
		(*task_item)->description=NULL;
	}
	if(NULL!=(*task_item)->fullFilePath)
	{
		delete [] (*task_item)->fullFilePath;
		(*task_item)->fullFilePath=NULL;
	}
	if(NULL!=(*task_item)->videoHeight)
	{
		delete [] (*task_item)->videoHeight;
		(*task_item)->videoHeight=NULL;
	}
	if(NULL!=(*task_item)->videoWidth)
	{
		delete [] (*task_item)->videoWidth;
		(*task_item)->videoWidth=NULL;
	}
	if(NULL!=(*task_item)->dstVideoHeight)
	{
		delete [] (*task_item)->dstVideoHeight;
		(*task_item)->dstVideoHeight=NULL;
	}
	if(NULL!=(*task_item)->dstVideoWidth)
	{
		delete [] (*task_item)->dstVideoWidth;
		(*task_item)->dstVideoWidth=NULL;
	}
	if(NULL!=(*task_item)->realDuration)
	{
		delete [] (*task_item)->realDuration;
		(*task_item)->realDuration=NULL;
	}
	if(NULL!=(*task_item)->featureFilePath)
	{
		delete [] (*task_item)->featureFilePath;
		(*task_item)->featureFilePath=NULL;
	}
	if(NULL!=(*task_item)->ad_order)
	{
		delete [] (*task_item)->ad_order;
		(*task_item)->ad_order=NULL;
	}
	if(NULL!=(*task_item)->show_type)
	{
		delete [] (*task_item)->show_type;
		(*task_item)->show_type=NULL;
	}
	if(NULL != *task_item)
	{
		delete *task_item;
		*task_item = NULL;
		
	}
}
void TaskTable::DeleteTaskItems(TASKS *ptaskitems)
{
	if(NULL == ptaskitems)
	{
		return;
	}

	if(ptaskitems->size() > 0)
	{
		for(int i = 0; i<ptaskitems->size(); i++)
		{
			TASK_ITEM *ptemp = (TASK_ITEM *)((*ptaskitems)[i]);
			DeleteItemSpace(&ptemp);
		}

		ptaskitems->clear();
	}
}

int TaskTable::FillSqlItem(char *sql,TASK_ITEM *task_item,int type)
{
	int ret = DB_SUCCESS;
	char pbuff_columns[BUFF_SIZE_4096]={'\0'};
	char pbuff_values[BUFF_SIZE_4096]={'\0'};
	int column_index = 0;
	int column_count = 0;

	memset(pbuff_columns,0,sizeof(pbuff_columns));
	memset(pbuff_values,0,sizeof(pbuff_values));

	column_count = sizeof(m_task_column_value_map) / sizeof(m_task_column_value_map[0]);
	MapItemVarToArray(task_item);
	/// insert sql
	if( 0 == type )
	{
		for(column_index = 1; column_index < column_count; column_index++)
		{
			if(strcmp(m_task_column_value_map[column_index].pvalue_var,""))
			{		
				if(column_index != 1 && strcmp(pbuff_values,"") )
				{
					strcat(pbuff_columns,",");
					strcat(pbuff_values,",");
				}	


				strcat(pbuff_columns,m_task_column_value_map[column_index].column_name);
				strcat(pbuff_values,"'");
				strcat(pbuff_values,m_task_column_value_map[column_index].pvalue_var);
				strcat(pbuff_values,"'");
			}


		}
		sprintf(sql,"INSERT INTO tasks(%s) "
			"VALUES(%s)",
			pbuff_columns,
			pbuff_values
			);
	}

	/// update sql
	if( 1 == type )
	{

		for(column_index = 1; column_index < column_count; column_index++)
		{
			if(strcmp(m_task_column_value_map[column_index].pvalue_var,""))
			{
				if(column_index != 1 && strcmp(pbuff_values,"") )
				{
					strcat(pbuff_values,",");
				}	

				strcat(pbuff_values,m_task_column_value_map[column_index].column_name);
				strcat(pbuff_values,"='");
				strcat(pbuff_values,m_task_column_value_map[column_index].pvalue_var);
				strcat(pbuff_values,"'");
			}


		}

		/// 更新记录
		sprintf(sql,"update tasks set %s "
			"where id = '%s'",
			pbuff_values,
			task_item->id);

	}

	if( 2 == type )
	{

		for(column_index = 0;column_index < column_count; column_index++)
		{
			if(strcmp(m_task_column_value_map[column_index].pvalue_var,""))
			{
				if(column_index != 0 && strcmp(pbuff_values,"") )
				{
					strcat(pbuff_values," and ");
				}	

				strcat(pbuff_values,m_task_column_value_map[column_index].column_name);
				strcat(pbuff_values,"='");
				strcat(pbuff_values,m_task_column_value_map[column_index].pvalue_var);
				strcat(pbuff_values,"'");
			}
		}

		if(strcmp(pbuff_values,""))
		{
			sprintf(sql,"select * from tasks where %s order by id asc",pbuff_values);
		}
		else
		{
			sprintf(sql,"select * from tasks order by id asc");
		}


	}

	return ret;
}

// 创建新表项 id不必填充
int TaskTable::InsertTaskItem(TASK_ITEM *task_item)
{
	int ret=DB_SUCCESS;
	char sql[BUFF_SIZE_4096];
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;

	if( NULL == task_item)
	{
		
		///////////////记录日志////////////
//		sprintf(buff_temp,"InsertTaskItem create task_item space  was failure!!!\n");
//		g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
		return DB_PARAMETER_ERROR;
	}

	///sprintf(task_item->fullFilePath,"%s/%s",task_item->filePath,task_item->fileName);
	/// 连接数据库
	ret = Connect(&conn);
	if( DB_SUCCESS != ret)
	{
		return ret;
	}

	memset(sql,0,sizeof(sql));

	ret=FillSqlItem(sql,task_item,0);

	///printf("FillSqlItem succeed!sql:%s\n",sql);

	/// 发送查询命令
	if (MysqlQuery(&conn,sql)) 
	{

		DisConnected(&conn);
		return DB_COMMAND_QUERY_ERROR;
	}


	memset(sql,0,sizeof(sql));
	sprintf(sql,"SELECT LAST_INSERT_ID()");

	if (MysqlQuery(&conn,sql))
	{

		DisConnected(&conn);
		return DB_COMMAND_QUERY_ERROR;
	}

	/// 获取结果
	pres = MysqlUseResult(conn);
	/// 获取结果
	if((row = MysqlFetchRow(pres)) != NULL)
	{
		if(NULL != row[0])
		{
			strcpy(m_task_column_value_map[0].pvalue_var,(char *)row[0]);
		}
	}

	MysqlFreeResult(pres);
	/// 断开数据库连接
	ret = DisConnected(&conn);

	return ret;
}

// 删除指定id的信息项
int TaskTable::DeleteTaskItem(int id)
{
	int ret;
	char sql[BUFF_SIZE_255]={'\0'};
	MYSQL *conn = NULL;			
	///MYSQL_RES *pres = NULL;
///	MYSQL_ROW row;

	if( 0 > id )
	{
		return DB_PARAMETER_ERROR;
	}
	/// 连接数据库
	ret = Connect(&conn);
	if( DB_SUCCESS != ret)
	{
		return ret;
	}

	memset(sql,0,sizeof(sql));

	/// 删除指定id记录
	sprintf(sql,"delete from tasks where id = '%d'",id);

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

int TaskTable::DeleteTaskItemByUuid(const char * uuid)
{
	int ret;
	char sql[BUFF_SIZE_255]={'\0'};
	MYSQL *conn = NULL;			
	///MYSQL_RES *pres = NULL;
	///MYSQL_ROW row;

	if( 0 == uuid )
	{
		return DB_PARAMETER_ERROR;
	}
	/// 连接数据库
	ret = Connect(&conn);
	if( DB_SUCCESS != ret)
	{
		return ret;
	}

	memset(sql,0,sizeof(sql));

	/// 删除指定id记录
	sprintf(sql,"delete from tasks where uuid = '%s'", uuid);

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

// 根据id号，获取到表中指定设备信息项
int TaskTable::GetTaskItemById(TASK_ITEM *task_item,int id)
{
	int ret=DB_SUCCESS;
	char sql[BUFF_SIZE_255]={'\0'};

	if(0 > id ||
		NULL == task_item)
	{
		return DB_PARAMETER_ERROR;
	}

	
	int column_count=0;
	int column_index=0;
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;

	ClearTableItemSpace(task_item);

	/// 连接数据库
	ret = Connect(&conn);
	if( DB_SUCCESS != ret)
	{
		return ret;
	}

	memset(sql,0,sizeof(sql));
	sprintf(sql,"select * from tasks where id = '%d'",id);


	/// 发送查询命令
	if (MysqlQuery(&conn,sql))
	{

		DisConnected(&conn);
		return DB_COMMAND_QUERY_ERROR;
	}

	/// 获取结果
	pres = MysqlUseResult(conn);

	/// 获取结果中各信息项
	column_count=sizeof(m_task_column_value_map)/sizeof(m_task_column_value_map[0]);
	while ((row = MysqlFetchRow(pres)) != NULL)
	{
		MapItemVarToArray(task_item);

		for(column_index = 0;column_index < column_count;column_index++ )
		{
			if(NULL != row[column_index])
			{
				strcpy(m_task_column_value_map[column_index].pvalue_var,(char *)row[column_index]);
			}
		}

	}

	/// 释放结果
	MysqlFreeResult(pres);

	/// 断开数据库连接
	ret = DisConnected(&conn);

	return ret;
}


int TaskTable::GetTaskItemByUuid(TASK_ITEM *task_item,const char *puuid)
{
	int ret=DB_SUCCESS;
	char sql[BUFF_SIZE_255]={'\0'};
	int column_count=0;
	int column_index=0;
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;

	if( (NULL == puuid) ||
		(!strcmp(puuid,"")) ||
		(NULL == task_item))
	{
		return DB_PARAMETER_ERROR;
	}	

	ClearTableItemSpace(task_item);

	/// 连接数据库
	ret = Connect(&conn);
	if( DB_SUCCESS != ret)
	{
		return ret;
	}

	memset(sql,0,sizeof(sql));
	sprintf(sql,"select * from tasks where `uuid` = '%s'",puuid);


	/// 发送查询命令
	if (MysqlQuery(&conn,sql))
	{

		DisConnected(&conn);
		return DB_COMMAND_QUERY_ERROR;
	}

	/// 获取结果
	pres = MysqlUseResult(conn);

	/// 获取结果中各信息项
	ClearTableItemSpace(task_item);

	column_count=sizeof(m_task_column_value_map)/sizeof(m_task_column_value_map[0]);
	while ((row = MysqlFetchRow(pres)) != NULL)
	{
		MapItemVarToArray(task_item);

		for(column_index = 0;column_index < column_count;column_index++ )
		{
			if(NULL != row[column_index])
			{
				strcpy(m_task_column_value_map[column_index].pvalue_var,(char *)row[column_index]);
			}
		}

	}

	/// 释放结果
	MysqlFreeResult(pres);

	/// 断开数据库连接
	ret = DisConnected(&conn);
	
	return ret;
}
// 获取所有设备信息
int TaskTable::GetAllItems(TASKS *ptasks)
{
	int ret=DB_SUCCESS;
	char sql[BUFF_SIZE_255];
	int column_count=0;
	int column_index=0;
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;

	/// 清空原有的记录项
	if(ptasks->size() > 0)
	{
		ptasks->clear();
	}

	/// 连接数据库
	ret = Connect(&conn);
	if( DB_SUCCESS != ret)
	{
		return ret;
	}

	memset(sql,0,sizeof(sql));
	///sprintf(sql,"select * from tasks where `check_state` = \"finished\" or `check_state` = \"finished_abort\" or `check_state` = \"finished_error\"  order by id asc");
	sprintf(sql,"select * from tasks order by id asc");
	/// 发送查询命令
	if (MysqlQuery(&conn,sql)) 
	{

		DisConnected(&conn);
		return DB_COMMAND_QUERY_ERROR;
	}

	/// 获取结果
	pres = MysqlUseResult(conn);

	column_count = sizeof(m_task_column_value_map) / sizeof(m_task_column_value_map[0]);

	///vector<COLUMN_INFO>columninfos;
	/// 获取结果中各信息项
	while ((row = MysqlFetchRow(pres)) != NULL)
	{

		TASK_ITEM *cplitem=NULL;

		NewTableItemSpace(&cplitem);
		MapItemVarToArray(cplitem);

		for(column_index=0;column_index<column_count;column_index++)
		{
			if(NULL!=row[column_index])
			{
				strcpy(m_task_column_value_map[column_index].pvalue_var,(char *)row[column_index]);
			}
		}

		ptasks->push_back(cplitem);

	}	
	/// 释放结果
	MysqlFreeResult(pres);			

	/// 断开数据库连接
	ret = DisConnected(&conn);
	return ret;
}

int TaskTable::GetItemsByItem(TASKS *pitems,TASK_ITEM *pfilter)
{
	int ret = DB_SUCCESS;
	char sql[BUFF_SIZE_2048]={'\0'};
	int column_count = 0;
	int column_index = 0;
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;

	/// 清空原有的记录项
	if(pitems->size() > 0)
	{
		pitems->clear();
	}

	/// 连接数据库
	ret = Connect(&conn);
	if( DB_SUCCESS != ret)
	{
		return ret;
	}

	memset(sql,0,sizeof(sql));

	ret = FillSqlItem(sql,pfilter,2);

	/// 发送查询命令
	if (MysqlQuery(&conn,sql)) 
	{
		
		DisConnected(&conn);
		return DB_COMMAND_QUERY_ERROR;
	}

	
	/// 获取结果
	pres = MysqlUseResult(conn);
	
	/// 获取结果中各信息项
	column_count = sizeof(m_task_column_value_map) / sizeof(m_task_column_value_map[0]);


	while ((row = MysqlFetchRow(pres)) != NULL)
	{
		TASK_ITEM *pItem = NULL;

		NewTableItemSpace(&pItem);

		MapItemVarToArray(pItem);

		for(column_index = 0;column_index < column_count;column_index++ )
		{
			if(NULL != row[column_index])
			{
				strcpy(m_task_column_value_map[column_index].pvalue_var,(char *)row[column_index]);
			}
		}

		pitems->push_back(pItem);

	}

	/// 释放结果
	MysqlFreeResult(pres);
	
			

	/// 断开数据库连接
	ret = DisConnected(&conn);


	return ret;
}

int TaskTable::GetAllValidItems(std::string &inspecttm,TASKS *ptasks)
{
	int ret=DB_SUCCESS;
	char sql[BUFF_SIZE_255];
	int column_count=0;
	int column_index=0;
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;

	/// 清空原有的记录项
	if(ptasks->size() > 0)
	{
		ptasks->clear();
	}

	/// 连接数据库
	ret = Connect(&conn);
	if( DB_SUCCESS != ret)
	{
		return ret;
	}

	memset(sql,0,sizeof(sql));
	///sprintf(sql,"select * from tasks where `check_state` = \"finished\" or `check_state` = \"finished_abort\" or `check_state` = \"finished_error\"  order by id asc");
	///sprintf(sql,"select * from tasks order by id asc");

    sprintf(sql,"select * from tasks where endDateTime>=\'%s\' and \'%s\'>= startDateTime  "
                "order by id asc;",inspecttm.c_str(),inspecttm.c_str());
	/// 发送查询命令
	if (MysqlQuery(&conn,sql)) 
	{

		DisConnected(&conn);
		return DB_COMMAND_QUERY_ERROR;
	}

	/// 获取结果
	pres = MysqlUseResult(conn);

	column_count = sizeof(m_task_column_value_map) / sizeof(m_task_column_value_map[0]);

	///vector<COLUMN_INFO>columninfos;
	/// 获取结果中各信息项
	while ((row = MysqlFetchRow(pres)) != NULL)
	{

		TASK_ITEM *cplitem=NULL;

		NewTableItemSpace(&cplitem);
		MapItemVarToArray(cplitem);

		for(column_index=0;column_index<column_count;column_index++)
		{
			if(NULL!=row[column_index])
			{
				strcpy(m_task_column_value_map[column_index].pvalue_var,(char *)row[column_index]);
			}
		}

		ptasks->push_back(cplitem);

	}	
	/// 释放结果
	MysqlFreeResult(pres);			

	/// 断开数据库连接
	ret = DisConnected(&conn);
	return ret;
}
int TaskTable::MapItemVarToArray(TASK_ITEM *task_item)
{
	int ret = DB_SUCCESS;

	if(NULL != task_item)
	{
		m_task_column_value_map[0].pvalue_var = task_item->id;
		m_task_column_value_map[1].pvalue_var = task_item->uuid;
		m_task_column_value_map[2].pvalue_var = task_item->orig_fileName;
		m_task_column_value_map[3].pvalue_var = task_item->fileName;
		m_task_column_value_map[4].pvalue_var = task_item->filePath;
		m_task_column_value_map[5].pvalue_var = task_item->fullFilePath;
		m_task_column_value_map[6].pvalue_var = task_item->startDateTime;
		m_task_column_value_map[7].pvalue_var = task_item->endDateTime;
		m_task_column_value_map[8].pvalue_var = task_item->hall_id;
		m_task_column_value_map[9].pvalue_var = task_item->frameRate;
		m_task_column_value_map[10].pvalue_var = task_item->frequency;
		m_task_column_value_map[11].pvalue_var = task_item->type;
		m_task_column_value_map[12].pvalue_var = task_item->videoWidth;
		m_task_column_value_map[13].pvalue_var = task_item->videoHeight;
		m_task_column_value_map[14].pvalue_var = task_item->dstVideoWidth;
		m_task_column_value_map[15].pvalue_var = task_item->dstVideoHeight;
		m_task_column_value_map[16].pvalue_var = task_item->realDuration;
		m_task_column_value_map[17].pvalue_var = task_item->featureFilePath;
        m_task_column_value_map[18].pvalue_var = task_item->ad_order;
		m_task_column_value_map[19].pvalue_var = task_item->show_type;
		m_task_column_value_map[20].pvalue_var = task_item->description;
		
	}

	return ret;
}
