#ifndef _MARKJOB_H_
#define _MARKJOB_H_

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <vector>
#include "Bitmap_define.h"
#include "FrameBufferLoop.h"
#include "MarkEngine.h"

#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
extern "C"{
#ifdef __cplusplus
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif
}
#ifdef __cplusplus
extern "C"
{
#endif
	
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

#include <libavutil/avstring.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>

#ifdef __cplusplus
}
#endif

using namespace std;

typedef enum {MARK_STATE_STOP = 0,
				MARK_STATE_RUNNING,
				MARK_STATE_PAUSING,
				MARK_STATE_FINISHED,
				MARK_STATE_FINISHED_ABORT,
				MARK_STATE_FINISHED_ERROR
}MARK_JOB_STATE;

typedef enum {MARK_CONTROL_STOP = 0,
				MARK_CONTROL_START,
				MARK_JOB_PAUSE	///保留
}MARK_JOB_CONTROL;


typedef struct _MARK_JOB_RESULT
{
	/// 任务id
	char markjob_id[50];
	/// 当前任务结束位置
	int find_pos;
	/// 是否找到龙标
	bool bisfind;
	/// 比对任务的状态
	MARK_JOB_STATE state;
	//// 循环缓冲帧对象指针
	FrameBufferLoop *pframeloop;
}MARK_JOB_RESULT;

/// 龙标图片保存文件名
//(MARK_JOB_ITEM::markjob_id)_(AV_FRAME_INFO::framenum).bmp

typedef struct _MARK_JOB_ITEM
{
	/// 1.任务id
	char markjob_id[50];
	/// 最大帧数量
	int max_frame_count;
	/// 2.mp4路径
	char videopath[255];
	/// 3.一天的时间基准值
	int time_base;
	/// 4.开始解码的时间点：秒值
	int decode_start_pos;
	/// 5.解码帧率：1秒解n帧
	int decode_rate;
	/// 6.龙标保存路径(找到龙标之后保存图片)
	char mark_store_path[255];
	/// 7.转码后图片分辨率调整
	bool bisadjust_aspect;
	/// 8.解码图片宽度
	int decode_width;
	/// 9.解码图片高度
	int decode_height;
	/// 10.是否查找龙标（默认为查找）
	bool bisfind_mark_flag;
	/// 11.彩色/灰度格式设置
	bool bisgray;

}MARK_JOB_ITEM;


typedef std::vector<MARK_JOB_ITEM *>MARK_JOB_ITEMS;


typedef struct _LAST_ERROR_REPORT LAST_ERROR_REPORT;


typedef int (*pfunc_callback_mark)(void *userdata,void *pmarkjobresult);


/////

typedef struct _FFMPEG_DECODER
{
    struct AVFormatContext *pInputFormatContext;
    struct AVCodecContext *pInputCodecContext;
	struct AVStream *pvideostream;
    struct AVCodec *pInputCodec;

    AVFrame *ptrOutFrame;
    struct SwsContext* pimg_convert_ctx; 
 	AVPicture avpicture_convert;

	MARK_JOB_ITEM markjob;

	int duration;
	int duration_in_seconds;

	AVPixelFormat m_pix_fmt_src;
	int video_width_src;
	int video_height_src;	
	int framerate;
	int flag_decode;

	AVPixelFormat m_pix_fmt_dest;
	int video_width_dest;
	int video_height_dest;	

}FFMPEG_DECODER;

////error code

#define MARK_JOB_SUCCESS	0
#define MARK_JOB_GET_LASTERROR_PARAMETER_IS_NULL		1		///获取最近错误信息错误，输入参数为空
#define MARK_JOB_RESET_STATE_ERROR_IN_RUNNING_PAUSING	2		///运行或暂停状态下，重置状态错误，需要先停止
#define MARK_JOB_STOP_ERROR_NOTIN_RUNNING_PAUSING		3		///停止失败，没有任务运行或暂停，需要有任务运行
#define MARK_JOB_START_ERROR_NOTIN_RUNNING_PAUSING		4		///开始失败，没有任务运行或暂停，需要有任务运行
#define MARK_JOB_PAUSE_ERROR_NOTIN_RUNNING_PAUSING		5		///暂停失败，没有任务运行或暂停，需要有任务运行
#define MARK_JOB_INITIALIZE_ERROR_LOAD_FEATURE			6		///初始化失败，加载龙标特征标志
#define MARK_JOB_CREATEJOB_ERROR_ASPECT_IS_OVERFLOW		7		///创建查找任务失败，分辨率超出范围
#define MARK_JOB_CREATEJOB_ERROR_STARTPOS_OUTRANGE		8		///创建查找任务失败，开始位置超出范围
////error code end

class MarkJob
{
public:
	MarkJob();
	virtual ~MarkJob();

	/// 初始化模块
    int Initialize(int max_frame_count,char *ppath_mark,Rect check_rect,
                   float rela_threshold,int abs_threshold);

	/// 卸载模块
	int UnInitialize();

	/// 创建比对龙标任务
	int CreateMarkJob(MARK_JOB_ITEM *pmarkjob,pfunc_callback_mark pmarkcallback,void *puserdata);

	/// 设置比对龙标状态
	int SetMarkControl(MARK_JOB_CONTROL markcontrol);

	/// 获取比对龙标状态
	MARK_JOB_STATE GetMarkState();

	/// 重置龙标的检测状态
	int RetsetMarkState();

	/// 获取最近的错误信息
	int GetLastError(LAST_ERROR_REPORT *preportstatus);

	/// 比对龙标处理线程
	int markjob_procedure();

	/// 获取video的信息
	int GetVideoInfo(const char *ppath_video,
						int &frames,
						int &seconds,
						int &width,
						int &height,
						int &framerate);
	

private:

	/// 获取原数据
	int get_metadata(const AVFormatContext *pformat_ctx);

	/// 初始化ffmpeg解码器
	int init_ffmpeg_decoder(FFMPEG_DECODER *pffmpeg_decoder,MARK_JOB_ITEM *pmarkjob);


	/// 销毁ffmpeg解码器
	int uninit_ffmpeg_decoder(FFMPEG_DECODER *pffmpeg_decoder);

	/// 根据avframe填充frameinfo
	int fill_frameinfo_by_avframe(AVFrame *pavframe,int frameindex,AV_FRAME_INFO *pframeinfo);


	/// 根据avpicture填充frameinfo
	int fill_frameinfo_by_avpicture(AVPicture *pavpicture,
							int width,
							int height,
							AVPixelFormat pix_fmt,
							int frameindex,
							AV_FRAME_INFO *pframeinfo);

	////save bmp
	int save_bmp(AVPicture *pavpicture,int width,int height,int framenum);

	////save yuv
	int save_yuv(AVPicture *pavpicture,AVPixelFormat pix_fmt,int width,int height,int framenum);


	///
	int convertfileformat(const char *pfilepath_src,const char *pfilepath_desc);
	int log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag);
	///

protected:


private:
	int m_separator_type;
	FrameBufferLoop m_framebufferloop;
	MarkEngine *m_pmarkengine;

	MARK_JOB_ITEM m_markjob;

	int m_isrunning;
	MARK_JOB_STATE m_markjob_state;
	LAST_ERROR_REPORT m_lasterror;
	pthread_mutex_t m_mutex_markstate;
	MARK_JOB_RESULT m_markjob_result;


	AVPixelFormat m_pix_fmt_scaletest_src;
	int m_scaletest_width_src;
	int m_scaletest_height_src;

	AVPixelFormat m_pix_fmt_scaletest_dest;
	int m_scaletest_width_dest;
	int m_scaletest_height_dest;
	int m_scaletest_flag_dest;

	FFMPEG_DECODER m_ffmpeg_decoder;

    static pfunc_callback_mark m_pfunc_markcallback;
    static void *m_puserdata;

};


#endif
