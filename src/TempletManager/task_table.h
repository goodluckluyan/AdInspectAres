#ifndef _TASK_TABLE_H
#define _TASK_TABLE_H

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



class TaskTable:public PubFun
{
public:
	TaskTable();
	~TaskTable();
	///int test();

	// 创建新表项 id不必填充
	int InsertTaskItem(TASK_ITEM *task_item);

	// 删除指定id的信息项
	int DeleteTaskItem(int id);
	// 删除指定uuid的信息项
	int DeleteTaskItemByUuid(const char * uuid);

	// 根据id号，获取到表中指定设备信息项
	int GetTaskItemById(TASK_ITEM *task_item,int id);

	// 根据uuid号，获取到表中指定设备信息项
	int GetTaskItemByUuid(TASK_ITEM *task_item,const char *puuid);

		// 任务信息
	int GetAllItems(TASKS *ptasks);

	/// 获取到表中的信息项集合，根据检索条件；
	int GetItemsByItem(TASKS *pitems,TASK_ITEM *pfilter);

	/// 获取有效期内的所有任务信息 
    int GetAllValidItems(std::string &inspecttm,TASKS *ptasks);

	
public:
	/// 结构体操作函数

	/// 清空结构体中各信息,置0
	void ClearTableItemSpace(TASK_ITEM *task_item);

	/// 为结构体中的指针变量申请内存空间，与DeleteItemSpace()函数成对使用
	void NewTableItemSpace(TASK_ITEM **task_item);

	/// 释放结构体中的指针变量的内存空间，与NewTableItemSpace()函数成对使用
	void DeleteItemSpace(TASK_ITEM **task_item);

	///释放容器内存空间
	void DeleteTaskItems(TASKS *ptaskitems);

//public:
//	/// 清空结构体中各信息,置0
//	void ClearPictureItemSpace(PICTUR_ITEM *item);
//
//	/// 为结构体中的指针变量申请内存空间，与DeleteItemSpace()函数成对使用
//	void NewPictureItemSpace(PICTUR_ITEM **item);
//
//	/// 释放结构体中的指针变量的内存空间，与NewTableItemSpace()函数成对使用
//	void DeletePictureItemSpace(PICTUR_ITEM **item);
//
//	///释放容器内存空间
//	void DeletePictureItems(PICTURE_LIST *items);
private:
	/// 数据库操作函数
	/// 填充sql查询语句，根据输入项 
	///tpye=0 insert sql
	///tpye=1 update sql
	///type=2 "amd"
	int FillSqlItem(char *sql,TASK_ITEM *task_item,int type);

	/// 映射结构体中指针变量到指针数组
	int MapItemVarToArray(TASK_ITEM *pusersitem);


private:
	COLUMN_VALUE_MAP m_task_column_value_map[21];

};



#endif
