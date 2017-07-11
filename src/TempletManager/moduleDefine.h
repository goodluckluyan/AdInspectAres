#ifndef _MODULE_DEFINE_H_
#define _MODULE_DEFINE_H_

#include <stdio.h>
#include <vector>
#include <string>
#include <unistd.h>
using namespace std;
#if defined(_WIN32)
#define sleep(x) Sleep(1000*x)
#define msleep(x) Sleep(x)
#else
#define msleep(x) usleep(1000*x)
#endif
#define	snprintf			_snprintf
#if !defined(_WIN32)
#ifndef itoa
#define itoa(a,b,c) sprintf(b,"%d",a)
#endif
#endif

#if defined(_WIN32)
#ifndef atoll
#define atoll(a) _atoi64(a)
#endif
#endif

#define PRINT_TEST 0

#define BUFF_SIZE_11	11
#define BUFF_SIZE_20    20
#define BUFF_SIZE_36    36
#define BUFF_SIZE_50	50
#define BUFF_SIZE_128	128
#define BUFF_SIZE_255	255
#define BUFF_SIZE_512	512
#define BUFF_SIZE_1024	1024
#define BUFF_SIZE_2048  2048
#define BUFF_SIZE_4096	4096
#define FEATURE_SIZE 1024    ///特征地址分配大小根据特征数量*500

#define RET_SUCCESS                 0           ///	操作成功
#define PARAMETER_ERROR             41200001    /// 输入参数错误
#define FRAMERATE_ZERO				41200002	///帧率为零
#define NO_VIDEO_STREAM				41200003	///找不到视频流///Can't find video stream
#define NO_OPEN_MP4_FILE			41200004	///Can't open mp4 file.
#define CODEC_PARAMETER_ERROR		41200005	///Can't find suitable codec parameters
#define NO_DECODE					41200006	///can't decode.
#define DECODE_ERR					41200007	///Decode err.
#define TEMPLETYPE_ERR			    41200008	///type input error.

///数据库操作
#define DB_SUCCESS								0	///	数据库操作成功
#define DB_PARAMETER_ERROR						41200101	/// 输入参数错误
#define DB_COMMAND_QUERY_ERROR					41200102	///	发送数据库查询命令失败
#define DB_CONNECTED_ERROR						41200103	///	连接数据库失败



typedef struct _PICTUR_ITEM{
    int picture_order;              ////bmp图片的序号
    unsigned int length;            ////地址长度
    char *addr;                     ///特征首地址
    char *picturePath;              ///bmp图片文件路径，目前图片路径和特征路径一致,/home/zyh/mp4File/xiaogou/dianqi001
    char *pictureFullFilePath;      ///全图片路径，例如/home/zyh/mp4File/feature001/0.bmp
    char *write_file;               ///test use,例如：/home/zyh/mp4File/xiaogou/dianqi001/outbuffer
    int quantity;                   ///quantity  一张图片特征点数量
}PICTUR_ITEM;
typedef vector<PICTUR_ITEM *>PICTURE_LIST;
///typedef vector<PICTURE_LIST *> TEMPLET_LIST;

typedef struct _TEMPLET_ITEM{
	
	char *uuid;				///订单号 
	char *ad_fileName;		///广告名称
	int picture_quantity;	///图片数量
	char *dstVideoWidth;    ///输出视频宽度   //必填输入值
	char *dstVideoHeight;   ///输出视频高度   //必填输入值 
	int ad_order;			///广告序号
	char *show_type;        ///映前广告pre_ad和贴片广告cinema_ad
    char *featrue_type;     ///特征类型


	
	PICTURE_LIST picture_list;

}TEMPLET_ITEM;

typedef vector<TEMPLET_ITEM *> TEMPLET_LIST;

typedef struct _TASK_ITEM{
	char *id; 
    char *uuid;             ///订单号 (订单号唯一)///必填输入值
    char *orig_fileName;    ////起始文件名
    char *fileName;         ////实际文件名(必须为英文),广告名称,例如：20161028-xiaogoudianqi-15s.mp4 /////必填输入值
    char *filePath;         ////文件路径,例如：/home/zyh/mp4File //必填输入值
    char *fullFilePath;     ///文件全路径，例如/home/zyh/mp4File/20161028-xiaogoudianqi-15s.mp4,内部获取
    char *startDateTime;    ////开始时间，例如2017-04-19 08:30:15  //必填输入值
    char *endDateTime;      ////结束时间,例如2017-04-30 08:30:15    //必填输入值
    char *hall_id;          ////厅号   //必填输入值
    char *frameRate;        ////帧率，每秒多少帧,内部获取
    char *frequency;        ////每秒取多少帧   //必填输入值
    char *type;             /////sift or surf   //必填输入值
    char *videoWidth;       ////videoWidth  内部获取，例如：1920
    char *videoHeight;      ////videoHeight 内部获取，例如：1080
    char *dstVideoWidth;    ///dstVideoWidth   //必填输入值
    char *dstVideoHeight;   ///dstVideoHeight   //必填输入值
	char *realDuration;     ///持续时间，内部获取
	char *featureFilePath;  ///特征文件路径,与图片路径一致 /////必填输入值
    char *ad_order;         ///广告序号
    char *show_type;        ///映前广告pre_ad和贴片广告cinema_ad
    char *description;      ////描述  //可选输入值

}TASK_ITEM;

typedef vector<TASK_ITEM *>TASKS;

typedef struct _FEATURE_ITEM{
	char *id;
    char *uuid;             ////广告订单号,不唯一，例如：feature001
    char *ad_fileName;		////广告文件名，，BUFF_SIZE_255
    char *bmp_fileName;		///bmp图片文件名，，BUFF_SIZE_255
    char *fileName;			////特征文件名，，BUFF_SIZE_255
    char *filePath;			////特征文件路径，例如/home/zyh/mp4File/feature001，，BUFF_SIZE_255
    char *fullFilePath;		/// 特征文件全路径，路径+文件名,例如/home/zyh/mp4File/feature001/1.txt
    char *bmp_fullFilePath;	////bmp图片全路径，用起始路径+bmp图片名例如：/home/zyh/mp4File/feature001/1.bmp
    char *picture_order;    ///order  bmp图片的序号
	char *quantity;         ///quantity  一张图片特征点数量
    char *bmp_quantity;     ///一个mp4文件生成的图片数量，BUFF_SIZE_50

}FEATURE_ITEM;

typedef vector<FEATURE_ITEM *>FEATURES;

#endif
