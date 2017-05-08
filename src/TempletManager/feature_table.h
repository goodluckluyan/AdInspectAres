#ifndef _FEATURE_TABLE_H
#define _FEATURE_TABLE_H

#include <stdio.h>
#include <vector>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <mysql.h>

#include "moduleDefine.h"
#include "pub.h"
using namespace std;



class FeatureTable:public PubFun
{
public:
	FeatureTable();
	~FeatureTable();
	///int test();

	// 创建新表项 id不必填充
	int InsertItem(FEATURE_ITEM *feature_item);

	// 删除指定id的信息项
	int DeleteItemById(int id);
	// 删除指定uuid的信息项
	int DeleteItemByUuid(const char * uuid);

	// 根据id号，获取到表中指定设备信息项
	int GetItemById(FEATURE_ITEM *feature_item,int id);

	// 根据uuid号，获取到表中指定设备信息项
	int GetItemByUuid(FEATURES *pfeatures,const char *puuid);

		// 所有设备信息
	int GetAllItems(FEATURES *pfeatures);

	
public:
	/// 结构体操作函数

	/// 清空结构体中各信息
	void ClearTableItemSpace(FEATURE_ITEM *feature_item);

	/// 为结构体中的指针变量申请内存空间，与DeleteItemSpace()函数成对使用
	void NewTableItemSpace(FEATURE_ITEM **feature_item);

	/// 释放结构体中的指针变量的内存空间，与NewTableItemSpace()函数成对使用
	void DeleteItemSpace(FEATURE_ITEM **feature_item);

	///释放容器
	void DeleteAllItemsSpace(FEATURES *ptaskitems);
private:
	/// 数据库操作函数
	/// 填充sql查询语句，根据输入项 
	///tpye=0 insert sql
	///tpye=1 update sql
	///type=2 "amd"
	int FillSqlItem(char *sql,FEATURE_ITEM *feature_item,int type);

	/// 映射结构体中指针变量到指针数组
	int MapItemVarToArray(FEATURE_ITEM *pusersitem);


private:
	COLUMN_VALUE_MAP m_feature_column_value_map[11];

};



#endif
