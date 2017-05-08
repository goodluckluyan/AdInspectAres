#ifndef _FRAME_BUFFER_LOOP_H_
#define _FRAME_BUFFER_LOOP_H_

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <pthread.h>


using namespace std;

#define BUFFER_MAX_SIZE		900

#define MAX_VIDEO_FRAME_LEN		2097152//1048576=1M 2097152=2M
#define MAX_TIME_LEN		20

///

typedef struct _AV_FRAME_INFO
{
	int length;///帧大小
	int type;//彩色/灰度
	//int channelnum;
	int width;	
	int height; 
	int framenum;	/// MARK_JOB_ITEM::time_base + MARK_JOB_ITEM::decode_pos + frameindex
	//unsigned char *record_time;
	unsigned char *data;

}AV_FRAME_INFO;

typedef enum{BUFFER_CONTROL_IDLE = 0,			
				BUFFER_CONTROL_WRITE,			
				BUFFER_CONTROL_READ,
			}BUFFER_CONTROL_TYPE;

typedef std::vector<AV_FRAME_INFO *>AV_FRAME_INFOS;

////错误返回定义
struct _LAST_ERROR_REPORT
{
	char status[20];
	char level[20];
	char module_name[20];
	char error_code[20];
	char message[255];
	char message_la[255];
};

typedef struct _LAST_ERROR_REPORT LAST_ERROR_REPORT;

class FrameBufferLoop
{
public:
	FrameBufferLoop();
	~FrameBufferLoop();

public:
	/// 初始化缓冲
	int InitFrameBuffer(int buffercount);

	/// 销毁缓冲
	int DestroyFrameBuffer();

	/// 复位缓冲 清空缓冲 读写索引复位
	int ResetFrameBuffer();
	
	/// 是否空闲
	bool IsIdle();
	
	/// 设置控制权限（读写之前获取控制权-->WRITE/READ，读写之后交出控制权-->IDLE）
	int SetContol(BUFFER_CONTROL_TYPE controltype, bool isalwaysok=0);	
	
	/// 向缓冲中写入数据 
	int WriteFrameData(	AV_FRAME_INFO *pframeinfo);

	/// 从缓冲中读出数据   (注:需要为参数pframeitem申请空间,以便进行值拷贝)
	int ReadFrameData(AV_FRAME_INFO *pframeitem);

	/// 从缓冲中读出数据 	
	int ReadFrameData(int index,AV_FRAME_INFO **pframeitem);

	/// 获取缓冲中的帧数量
	int GetFrameCount();

	/// 获取最近的错误信息
	int GetLastError(LAST_ERROR_REPORT *preportstatus);


public:
	/// 申请内存空间，为单项帧信息
	void NewSpaceItem(AV_FRAME_INFO **pframeitem);
	/// 释放内存空间，为单项配置信息
	void DeleteSpaceItem(AV_FRAME_INFO **pframeitem);
	/// 清空内存空间，为单项配置信息
	void ZeroSpaceItem(AV_FRAME_INFO *pframeitem);
	/// 释放内存空间，为配置信息
	void DeleteSpaceItems(AV_FRAME_INFOS *pframeitems);
	/// 清空内存空间，为所有配置信息
	void ZeroSpaceItems(AV_FRAME_INFOS *pframeitems);

private:
	int m_counter_write;
	int m_buffer_size;
	int m_counter_frame;
	LAST_ERROR_REPORT m_lasterror;

	AV_FRAME_INFOS m_framelist;
	BUFFER_CONTROL_TYPE m_control_type;
	pthread_mutex_t m_mutex_control;


};

#endif
