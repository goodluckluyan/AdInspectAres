#include "feature_table.h"
#include <string.h>
///#include "zdb.h"

COLUMN_VALUE_MAP g_feature_column_value_map[]=
{
	{"id",NULL},
	{"uuid",NULL},
	{"ad_fileName",NULL},
	{"bmp_fileName",NULL},
	{"fileName",NULL},
	{"filePath",NULL},
	{"fullFilePath",NULL},
	{"bmp_fullFilePath",NULL},
	{"picture_order",NULL},
	{"quantity",NULL},
	{"bmp_quantity",NULL}
};

typedef struct _COLUMN_INFO
{
	char column_name[200];
	char *pcolumn_value;
	int type;
	int length;
}COLUMN_INFO;


FeatureTable::FeatureTable()
{

	CopyMapArray(m_feature_column_value_map,g_feature_column_value_map,sizeof(g_feature_column_value_map)/sizeof(g_feature_column_value_map[0]));

}
FeatureTable::~FeatureTable()
{

}
void FeatureTable::ClearTableItemSpace(FEATURE_ITEM *feature_item)
{
	if(NULL==feature_item)
	{
		return;
	}
	if(NULL!=feature_item->uuid)
	{
		memset(feature_item->id,0,BUFF_SIZE_50);
		memset(feature_item->uuid,0,BUFF_SIZE_50);
	}
	
	if(NULL!=feature_item->ad_fileName)
	{
		memset(feature_item->ad_fileName,0,BUFF_SIZE_255);
	}
	if(NULL!=feature_item->bmp_fileName)
	{
		memset(feature_item->bmp_fileName,0,BUFF_SIZE_255);
	}
	if(NULL!=feature_item->fileName)
	{
		memset(feature_item->fileName,0,BUFF_SIZE_255);
	}
	if(NULL!=feature_item->filePath)
	{
		memset(feature_item->filePath,0,BUFF_SIZE_255);
	}
	
	if(NULL!=feature_item->fullFilePath)
	{
		memset(feature_item->fullFilePath,0,BUFF_SIZE_255);
	}
	if(NULL!=feature_item->bmp_fullFilePath)
	{
		memset(feature_item->bmp_fullFilePath,0,BUFF_SIZE_255);
	}
	if(NULL!=feature_item->picture_order)
	{
		memset(feature_item->picture_order,0,BUFF_SIZE_50);
	}
	if(NULL!=feature_item->quantity)
	{
		memset(feature_item->quantity,0,BUFF_SIZE_50);
	}
	if(NULL!=feature_item->bmp_quantity)
	{
		memset(feature_item->bmp_quantity,0,BUFF_SIZE_50);
	}

}

void FeatureTable::NewTableItemSpace(FEATURE_ITEM **feature_item)
{
	if(NULL!=*feature_item)
	{
		return;
	}
	*feature_item=(FEATURE_ITEM *)new FEATURE_ITEM;
	(*feature_item)->id=new char[BUFF_SIZE_50];
	(*feature_item)->uuid=new char[BUFF_SIZE_50];
	(*feature_item)->ad_fileName=new char[BUFF_SIZE_255];
	(*feature_item)->bmp_fileName=new char[BUFF_SIZE_255];
	(*feature_item)->fileName=new char[BUFF_SIZE_255];
	(*feature_item)->filePath=new char[BUFF_SIZE_255];
	(*feature_item)->fullFilePath=new char[BUFF_SIZE_255];
	(*feature_item)->bmp_fullFilePath=new char[BUFF_SIZE_255];
	(*feature_item)->picture_order=new char[BUFF_SIZE_50];
	(*feature_item)->quantity=new char[BUFF_SIZE_50];
	(*feature_item)->bmp_quantity=new char[BUFF_SIZE_50];

	ClearTableItemSpace(*feature_item);
}

void FeatureTable::DeleteItemSpace(FEATURE_ITEM **feature_item)
{
	if(NULL==(*feature_item))
	{
		return;
	}

	ClearTableItemSpace(*feature_item);

	if(NULL!=(*feature_item)->uuid)
	{
		delete [] (*feature_item)->id;
		(*feature_item)->id=NULL;
		delete [] (*feature_item)->uuid;
		(*feature_item)->uuid=NULL;
	}
	if(NULL!=(*feature_item)->ad_fileName)
	{
		delete [] (*feature_item)->ad_fileName;
		(*feature_item)->ad_fileName=NULL;
	}
	if(NULL!=(*feature_item)->bmp_fileName)
	{
		delete [] (*feature_item)->bmp_fileName;
		(*feature_item)->bmp_fileName=NULL;
	}
	if(NULL!=(*feature_item)->fileName)
	{
		delete [] (*feature_item)->fileName;
		(*feature_item)->fileName=NULL;
	}
	if(NULL!=(*feature_item)->filePath)
	{
		delete [] (*feature_item)->filePath;
		(*feature_item)->filePath=NULL;
	}
	if(NULL!=(*feature_item)->fullFilePath)
	{
		delete [] (*feature_item)->fullFilePath;
		(*feature_item)->fullFilePath=NULL;
	}
	
	if(NULL!=(*feature_item)->bmp_fullFilePath)
	{
		delete [] (*feature_item)->bmp_fullFilePath;
		(*feature_item)->bmp_fullFilePath=NULL;
	}
	if(NULL!=(*feature_item)->picture_order)
	{
		delete [] (*feature_item)->picture_order;
		(*feature_item)->picture_order=NULL;
	}
	if(NULL!=(*feature_item)->quantity)
	{
		delete [] (*feature_item)->quantity;
		(*feature_item)->quantity=NULL;
	}
	if(NULL!=(*feature_item)->bmp_quantity)
	{
		delete [] (*feature_item)->bmp_quantity;
		(*feature_item)->bmp_quantity=NULL;
	}
	if(NULL != *feature_item)
	{
		delete *feature_item;
		*feature_item = NULL;
		
	}

}
void FeatureTable::DeleteAllItemsSpace(FEATURES *ptaskitems)
{
	if(NULL == ptaskitems)
	{
		return;
	}

	if(ptaskitems->size() > 0)
	{
		for(int i = 0; i<ptaskitems->size(); i++)
		{
			FEATURE_ITEM *ptemp = (FEATURE_ITEM *)((*ptaskitems)[i]);
			DeleteItemSpace(&ptemp);
		}

		ptaskitems->clear();
	}
}
int FeatureTable::MapItemVarToArray(FEATURE_ITEM *feature_item)
{
	int ret = DB_SUCCESS;

	if(NULL != feature_item)
	{
		m_feature_column_value_map[0].pvalue_var = feature_item->id;
		m_feature_column_value_map[1].pvalue_var = feature_item->uuid;
		m_feature_column_value_map[2].pvalue_var = feature_item->ad_fileName;
		m_feature_column_value_map[3].pvalue_var = feature_item->bmp_fileName;
		m_feature_column_value_map[4].pvalue_var = feature_item->fileName;
		m_feature_column_value_map[5].pvalue_var = feature_item->filePath;
		m_feature_column_value_map[6].pvalue_var = feature_item->fullFilePath;
		m_feature_column_value_map[7].pvalue_var = feature_item->bmp_fullFilePath;
		m_feature_column_value_map[8].pvalue_var = feature_item->picture_order;
		m_feature_column_value_map[9].pvalue_var = feature_item->quantity;
		m_feature_column_value_map[10].pvalue_var = feature_item->bmp_quantity;
	}

	return ret;
}

int FeatureTable::FillSqlItem(char *sql,FEATURE_ITEM *feature_item,int type)
{
	int ret = DB_SUCCESS;
	char pbuff_columns[BUFF_SIZE_4096]={'\0'};
	char pbuff_values[BUFF_SIZE_4096]={'\0'};
	int column_index = 0;
	int column_count = 0;

	memset(pbuff_columns,0,sizeof(pbuff_columns));
	memset(pbuff_values,0,sizeof(pbuff_values));

	column_count = sizeof(m_feature_column_value_map) / sizeof(m_feature_column_value_map[0]);
	MapItemVarToArray(feature_item);
	/// insert sql
	if( 0 == type )
	{
		for(column_index = 1; column_index < column_count; column_index++)
		{
			if(strcmp(m_feature_column_value_map[column_index].pvalue_var,""))
			{		
				if(column_index != 1 && strcmp(pbuff_values,"") )
				{
					strcat(pbuff_columns,",");
					strcat(pbuff_values,",");
				}	


				strcat(pbuff_columns,m_feature_column_value_map[column_index].column_name);
				strcat(pbuff_values,"'");
				strcat(pbuff_values,m_feature_column_value_map[column_index].pvalue_var);
				strcat(pbuff_values,"'");
			}


		}
		sprintf(sql,"INSERT INTO features(%s) "
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
			if(strcmp(m_feature_column_value_map[column_index].pvalue_var,""))
			{
				if(column_index != 1 && strcmp(pbuff_values,"") )
				{
					strcat(pbuff_values,",");
				}	

				strcat(pbuff_values,m_feature_column_value_map[column_index].column_name);
				strcat(pbuff_values,"='");
				strcat(pbuff_values,m_feature_column_value_map[column_index].pvalue_var);
				strcat(pbuff_values,"'");
			}


		}

		/// 更新记录
		sprintf(sql,"update features set %s "
			"where id = '%s'",
			pbuff_values,
			feature_item->id);

	}

	if( 2 == type )
	{

		for(column_index = 0;column_index < column_count; column_index++)
		{
			if(strcmp(m_feature_column_value_map[column_index].pvalue_var,""))
			{
				if(column_index != 0 && strcmp(pbuff_values,"") )
				{
					strcat(pbuff_values," and ");
				}	

				strcat(pbuff_values,m_feature_column_value_map[column_index].column_name);
				strcat(pbuff_values,"='");
				strcat(pbuff_values,m_feature_column_value_map[column_index].pvalue_var);
				strcat(pbuff_values,"'");
			}
		}

		if(strcmp(pbuff_values,""))
		{
			sprintf(sql,"select * from features where %s order by id asc",pbuff_values);
		}
		else
		{
			sprintf(sql,"select * from features order by id asc");
		}


	}

	return ret;
}

// 创建新表项 id不必填充
int FeatureTable::InsertItem(FEATURE_ITEM *feature_item)
{
	int ret=DB_SUCCESS;
	char sql[BUFF_SIZE_4096];
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;

	if( NULL == feature_item)
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

	ret=FillSqlItem(sql,feature_item,0);

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
			strcpy(m_feature_column_value_map[0].pvalue_var,(char *)row[0]);
		}
	}

	MysqlFreeResult(pres);
	/// 断开数据库连接
	ret = DisConnected(&conn);
	return ret;
}

// 删除指定id的信息项
int FeatureTable::DeleteItemById(int id)
{
	int ret;
	char sql[BUFF_SIZE_255]={'\0'};
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;

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
	sprintf(sql,"delete from features where id = '%d'",id);

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

int FeatureTable::DeleteItemByUuid(const char * uuid)
{
	int ret;
	char sql[BUFF_SIZE_255]={'\0'};
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;

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
	sprintf(sql,"delete from features where uuid = '%s'", uuid);

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
int FeatureTable::GetItemById(FEATURE_ITEM *feature_item,int id)
{
	int ret=DB_SUCCESS;
	char sql[BUFF_SIZE_255]={'\0'};

	if(0 > id ||
		NULL == feature_item)
	{
		return DB_PARAMETER_ERROR;
	}

	
	int column_count=0;
	int column_index=0;
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;

	ClearTableItemSpace(feature_item);

	/// 连接数据库
	ret = Connect(&conn);
	if( DB_SUCCESS != ret)
	{
		return ret;
	}

	memset(sql,0,sizeof(sql));
	sprintf(sql,"select * from features where id = '%d'",id);


	/// 发送查询命令
	if (MysqlQuery(&conn,sql))
	{

		DisConnected(&conn);
		return DB_COMMAND_QUERY_ERROR;
	}

	/// 获取结果
	pres = MysqlUseResult(conn);

	/// 获取结果中各信息项
	column_count=sizeof(m_feature_column_value_map)/sizeof(m_feature_column_value_map[0]);
	while ((row = MysqlFetchRow(pres)) != NULL)
	{
		MapItemVarToArray(feature_item);

		for(column_index = 0;column_index < column_count;column_index++ )
		{
			if(NULL != row[column_index])
			{
				strcpy(m_feature_column_value_map[column_index].pvalue_var,(char *)row[column_index]);
			}
		}

	}

	/// 释放结果
	MysqlFreeResult(pres);

	/// 断开数据库连接
	ret = DisConnected(&conn);

	return ret;
}


int FeatureTable::GetItemByUuid(FEATURES *pfeatures,const char *puuid)
{
	int ret=DB_SUCCESS;
	char sql[BUFF_SIZE_255]={'\0'};
	int column_count=0;
	int column_index=0;
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;

	///if( (NULL == puuid) || (!strcmp(puuid,"")) || (NULL == pfeatures))
	if( (NULL == puuid) || (!strcmp(puuid,"")))
	{
		return DB_PARAMETER_ERROR;
	}	
	/// 清空原有的记录项
	if(pfeatures->size() > 0)
	{
		pfeatures->clear();
	}
	///ClearTableItemSpace(feature_item);

	/// 连接数据库
	ret = Connect(&conn);
	if( DB_SUCCESS != ret)
	{
		return ret;
	}

	memset(sql,0,sizeof(sql));
	sprintf(sql,"select * from features where `uuid` = '%s'",puuid);


	/// 发送查询命令
	if (MysqlQuery(&conn,sql))
	{

		DisConnected(&conn);
		return DB_COMMAND_QUERY_ERROR;
	}

	/// 获取结果
	pres = MysqlUseResult(conn);

	/// 获取结果中各信息项
	///ClearTableItemSpace(feature_item);

	column_count=sizeof(m_feature_column_value_map)/sizeof(m_feature_column_value_map[0]);
	
	while ((row = MysqlFetchRow(pres)) != NULL)
	{
		/////add
		FEATURE_ITEM *feature_item = NULL;
		NewTableItemSpace(&feature_item);

		//////

		MapItemVarToArray(feature_item);

		for(column_index = 0;column_index < column_count;column_index++ )
		{
			if(NULL != row[column_index])
			{
				strcpy(m_feature_column_value_map[column_index].pvalue_var,(char *)row[column_index]);
			}
		}
		pfeatures->push_back(feature_item);
	}

	/// 释放结果
	MysqlFreeResult(pres);

	/// 断开数据库连接
	ret = DisConnected(&conn);
	
	return ret;
}
// 获取所有设备信息
int FeatureTable::GetAllItems(FEATURES *pfeatures)
{
	int ret=DB_SUCCESS;
	char sql[BUFF_SIZE_255];
	int column_count=0;
	int column_index=0;
	MYSQL *conn = NULL;			
	MYSQL_RES *pres = NULL;
	MYSQL_ROW row;

	/// 清空原有的记录项
	if(pfeatures->size() > 0)
	{
		pfeatures->clear();
	}

	/// 连接数据库
	ret = Connect(&conn);
	if( DB_SUCCESS != ret)
	{
		return ret;
	}

	memset(sql,0,sizeof(sql));
	///sprintf(sql,"select * from features where `check_state` = \"finished\" or `check_state` = \"finished_abort\" or `check_state` = \"finished_error\"  order by id asc");
	sprintf(sql,"select * from features order by id asc");
	/// 发送查询命令
	if (MysqlQuery(&conn,sql)) 
	{

		DisConnected(&conn);
		return DB_COMMAND_QUERY_ERROR;
	}

	/// 获取结果
	pres = MysqlUseResult(conn);

	column_count = sizeof(m_feature_column_value_map) / sizeof(m_feature_column_value_map[0]);

	///vector<COLUMN_INFO>columninfos;
	/// 获取结果中各信息项
	while ((row = MysqlFetchRow(pres)) != NULL)
	{

		FEATURE_ITEM *cplitem=NULL;

		NewTableItemSpace(&cplitem);
		MapItemVarToArray(cplitem);

		for(column_index=0;column_index<column_count;column_index++)
		{
			if(NULL!=row[column_index])
			{
				strcpy(m_feature_column_value_map[column_index].pvalue_var,(char *)row[column_index]);
			}
		}

		pfeatures->push_back(cplitem);

	}	
	/// 释放结果
	MysqlFreeResult(pres);			

	/// 断开数据库连接
	ret = DisConnected(&conn);
	return ret;
}

//int FeatureTable::test()
//{
//
//	int ret=DB_SUCCESS;
//	char sql[BUFF_SIZE_4096];
//	int column_count=0;
//	int column_index=0;
//	MYSQL *conn = NULL;			
//	MYSQL_RES *pres = NULL;
//	MYSQL_ROW row;
//
//	/// 连接数据库
//	ret = Connect(&conn);
//	if( DB_SUCCESS != ret)
//	{
//		return ret;
//	}
//
//	memset(sql,0,sizeof(sql));
//
//	//	sprintf(sql,"select COLUMN_NAME from information_schema.COLUMNS  where table_name = 'features'");
//	sprintf(sql,"select id from features");
//
//	/// 发送查询命令
//	if (MysqlQuery(&conn,sql)) 
//	{
//
//		DisConnected(&conn);
//		return DB_COMMAND_QUERY_ERROR;
//	}
//
//
//	/// 获取结果
//	pres = MysqlUseResult(conn);
//
//	column_count = sizeof(m_feature_column_value_map) / sizeof(m_feature_column_value_map[0]);
//
//	vector<COLUMN_INFO>columninfos;
//	/// 获取结果中各信息项
//
//#if 0
//	MYSQL_FIELD *field = NULL;
//
//	while ((field = MysqlFetchRow(pres)) != NULL)
//	{
//
//		COLUMN_INFO column_info;
//
//
//		strcpy(column_info.column_name,(char *)field->name);
//		column_info.length = field->length;
//		column_info.type = field->type;
//
//		columninfos.push_back(column_info);
//
//	}	
//#endif
//	/// 释放结果
//	MysqlFreeResult(pres);			
//
//	/// 断开数据库连接
//	ret = DisConnected(&conn);
//	return ret;
//
//}
