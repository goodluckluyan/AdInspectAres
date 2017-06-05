#ifndef _TEMPLETMODULE_H
#define _TEMPLETMODULE_H

#include "task_table.h"
#include "feature_table.h"
#include "moduleDefine.h"
//#include "./pub.h"
extern "C"{
#ifdef __cplusplus
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif
}
/////
extern "C"
{
#include <libavutil/avstring.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>

}

/// 类的超前声明
class TaskTable;
class FeatureTable;
class VideoCompareModule;
///@end

class TempletManager
{
public:
	TempletManager();
	~TempletManager();
    ///db_ip :输入ip地址；
	///db_username :输入用户名
	///db_password :输入密码
	///db_dbname:输入数据库名
	///port :输入端口，目前没有用到置0
    int InitDatabase(const char *db_ip, const char *db_username,
					   const char *db_password,
					   const char *db_dbname,
					   unsigned int port);

	///创建任务模板,将新的模板插入到数据库中，需要在外部申请和释放task_item内存
	int CreateTaskTemplet(TASK_ITEM *task_item);

	///获取任务表中在有效期内的所有模板信息,获取后要调用DeleteTemplet_list释放内存
    int GetAllTemplets(std::string &inspecttm,TEMPLET_LIST &templet_list);

	///释放Templet_list容器内存
	int DeleteTemplet_list(TEMPLET_LIST *pTemplet_list);

	///释放Templet_list容器内存
	int DeleteTempletByUuid(char *uuid);
public:

	/// 清空pictureItem结构体中各信息
	void ClearPictureItemSpace(PICTUR_ITEM *pictureItem,int quantity);

	/// 为pictureItem结构体中的指针变量申请内存空间，与DeletePictureSpace()函数成对使用
	void NewPictureItemSpace(PICTUR_ITEM **pictureItem, int quantity);

	/// 释放pictureItem结构体中的指针变量的内存空间，与NewPictureItemSpace()函数成对使用
	void DeletePictureSpace(PICTUR_ITEM **pictureItem,int quantity);

public:
		///获取任务表中所有模板信息
	int GetAllTaskTemplets(TASKS &ptasks);
	///释放任务模块容器内存
	int DeleteTaskItems(TASKS *ptaskitems);

	///获取特征表中所有特征信息
	int GetAllFeatures(FEATURES &pfeatures);
	///释放任务模块容器内存
	int DeleteAllFeaturesSpace(FEATURES *pfeatures);

	/***************************************************************/
	/*发送特征值容器，返回特征值容器内容                           */
    /*先调用CreateTaskTemplet创建新的任务模板                      */
    /*然后通过SetCall获取所有特征值                                */
    /*最后需要调用DeleteAllFeaturesSpace释放m_pFreatureItems内存   */
    /***************************************************************/
	//int SetCall(FEATURES &pfeatures);
	
	void SaveBmp(AVCodecContext *CodecContex, AVFrame *Picture, int width, int height,int dst_width,int dst_height,int num,char *bmp_path);

public:
	TaskTable *m_taskTable;
	FeatureTable *m_featureTable;

	///TEMPLET_LIST m_templet_list;
	///FEATURES m_freatureItems;
	///VideoCompareModule *videoCompareModule;
	
	///PICTURE_LIST m_pictureList;
	//PubFun *m_pubFun;
///private:
};

#endif
