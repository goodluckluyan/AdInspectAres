#include "TempletModule.h"
#include "utility/FileManager_linux.h"
#include "feature_table.h"
#include "VideoCompareModule.h"

#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <memory.h>
#include <ctype.h>
#include <vector>
#include "log/MyLogger.h"

extern MyLogger g_templet_logwrite;
extern char buff_temp[512];

////
char g_database_ip[50];
char g_database_dbname[50];
char g_database_username[50];
char g_database_password[50];
char g_database_port[50];
/////

typedef struct _BITMAPFILEHEADER
{
    unsigned short bfType;
    unsigned int  bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
}__attribute__((packed)) BITMAPFILEHEADER;

typedef struct _BITMAPINFOHEADER
{
    u_int32_t biSize;
    u_int32_t biWidth;
    u_int32_t biHeight;
    u_int16_t biPlanes;
    u_int16_t biBitCount;
    u_int32_t biCompression;
    u_int32_t biSizeImage;
    u_int32_t biXPelsPerMeter;
    u_int32_t biYPelsPerMeter;
    u_int32_t biClrUsed;
    u_int32_t biClrImportant;
}__attribute__((packet)) BITMAPINFOHEADER;

TempletManager::TempletManager()
{
	m_taskTable = NULL;
	m_taskTable = new TaskTable();

	m_featureTable =NULL;
	m_featureTable = new FeatureTable();

	//videoCompareModule =NULL;
	//videoCompareModule = new VideoCompareModule();
}

TempletManager::~TempletManager()
{
	delete m_taskTable;
	m_taskTable = NULL;

	delete m_featureTable;
	m_featureTable =NULL;

	///DeleteAllFeaturesSpace(&m_freatureItems);
}

int TempletManager::InitDatabase(const char *db_ip, const char *db_username,
					   const char *db_password,
					   const char *db_dbname,
					   unsigned int port)
{
	int iRet = RET_SUCCESS;

	memset(g_database_ip,0,BUFF_SIZE_50);
	memset(g_database_username,0,BUFF_SIZE_50);
	memset(g_database_password,0,BUFF_SIZE_50);
	memset(g_database_dbname,0,BUFF_SIZE_50);

	memcpy(g_database_ip,db_ip,BUFF_SIZE_50);
	memcpy(g_database_username,db_username,BUFF_SIZE_50);
	memcpy(g_database_password,db_password,BUFF_SIZE_50);
	memcpy(g_database_dbname,db_dbname,BUFF_SIZE_50);
	//sprintf(m_pubFun->m_database_ip,"%s",db_ip);
	//sprintf(m_pubFun->m_database_username,"%s",db_username);
	//sprintf(m_pubFun->m_database_password,"%s",db_password);
	//sprintf(m_pubFun->m_database_dbname,"%s",db_dbname);

	printf("m_pubFun->m_database_ip :%s\n",g_database_ip);
	return iRet;
}
int TempletManager::GetAllTaskTemplets(TASKS &ptasks)
{
	int iRet = RET_SUCCESS;

	iRet = m_taskTable->GetAllItems(&ptasks);

	return iRet;
}

int TempletManager::DeleteTaskItems(TASKS *ptaskitems)
{
	int iRet = RET_SUCCESS;
	///TaskTable taskTable;
	for(TASKS::iterator it = ptaskitems->begin(); it != ptaskitems->end(); it++)
	{
		TASK_ITEM *pItem = (TASK_ITEM *)(*it);
		m_taskTable->DeleteItemSpace(&pItem);
	}
	ptaskitems->clear();

	return iRet;
}

int TempletManager::GetAllFeatures(FEATURES &pfeatures)
{
	int iRet = RET_SUCCESS;

	iRet = m_featureTable->GetAllItems(&pfeatures);

	return iRet;
}
int TempletManager::DeleteAllFeaturesSpace(FEATURES *pfeatures)
{
	int iRet = RET_SUCCESS;

	m_featureTable->DeleteAllItemsSpace(pfeatures);

	return iRet;
}
int TempletManager::CreateTaskTemplet(TASK_ITEM *task_item)
{
	int iRet = RET_SUCCESS;
	
	struct AVFormatContext *pInputFormatContext = avformat_alloc_context();//NULL;
    struct AVCodecContext *pInputCodecContext = NULL;
    struct  AVCodec *pInputCodec = NULL;


	//注册库中含有的所有可用的文件格式和编码器，这样当打开一个文件时，它们才能够自动选择相应的文件格式和编码器。
    av_register_all();
	av_log_set_level(AV_LOG_QUIET);///不显示打印信息

	sprintf(task_item->fullFilePath,"%s/%s",task_item->filePath,task_item->fileName);
	 // 打开视频文件
	if((iRet=avformat_open_input(&pInputFormatContext, task_item->fullFilePath, NULL, NULL))!=0)
    {
		printf("Can't open mp4 file.\n");
		///////////////记录日志////////////////
		sprintf(buff_temp,"Error,can't open mp4 file. fullFilePath: %s\n",task_item->fullFilePath);
		g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
		//////////////////////////////////////
        return -1;
    }
	
    // 取出文件流信息
    if(avformat_find_stream_info(pInputFormatContext,NULL)<0)
    {
		printf("Can't find suitable codec parameters.\n");

		///////////////记录日志///////////////////
		memset(buff_temp, 0, sizeof(buff_temp));
		sprintf(buff_temp,"Error,can't find suitable codec parameters. fullFilePath: %s\n",task_item->fullFilePath);
		g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
		/////////////////////////////////////////

        return -1;
    }

	 //print media info
    av_dump_format(pInputFormatContext, 0, task_item->fullFilePath, false);

	//仅仅处理视频流
    //只简单处理我们发现的第一个视频流
    //  寻找第一个视频流
    int videoIndex  =-1;
    for(int i=0; i<pInputFormatContext->nb_streams; i++)
    {
        if(pInputFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoIndex = i;
            break;
        }
    }

    if(-1 == videoIndex)
    {
		printf("Can't find video stream !\n");

		///////////////记录日志//////////////
		sprintf(buff_temp,"Error,can't find video stream ! fullFilePath: %s\n",task_item->fullFilePath);
		g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
		////////////////////////////////////
        return -1;
    }

    // 得到视频流编码上下文的指针  ///pInputCodecContext 可以获取分辨率例如1920*1080,帧率等
    pInputCodecContext = pInputFormatContext->streams[videoIndex]->codec;

    //  寻找视频流的解码器
    pInputCodec = avcodec_find_decoder(pInputCodecContext->codec_id);

    if(NULL == pInputCodec)
    {
		printf("can't decode\n");
		///////////////记录日志 ////////
		sprintf(buff_temp,"Error,can't decode. fullFilePath: %s\n",task_item->fullFilePath);
		g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
		//////////////////////////////
        return -1;
    }
	
    //打开解码器
    if(avcodec_open2(pInputCodecContext, pInputCodec,NULL) != 0)
    {
		printf("decode error\n");

		///////////////记录日志/////////////////
		sprintf(buff_temp,"Error,decode error. fullFilePath: %s\n",task_item->fullFilePath);
		g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
		///////////////////////////////////////
        return -1;
    }
	///bmp图片保存路径为输入值featureFilePath，例如：/home/zyh/mp4File/feature001
	char bmp_path[BUFF_SIZE_255];
	memset(bmp_path,0,BUFF_SIZE_255);
	sprintf(bmp_path,"%s",task_item->featureFilePath);
	////持续时间
	int realDuration = pInputFormatContext->duration /1000000;
	///分辨率
	int videoWidth = pInputCodecContext->width;
    int videoHeight = pInputCodecContext->height;
	///帧率
    /// float frameRate = (float)avRation.den/avRation.num;
    AVRational avRate = pInputFormatContext->streams[videoIndex]->avg_frame_rate;
    int frameRate = avRate.num/avRate.den;
    if(frameRate == 0)
    {
        printf("error,frameRate is 0.\n");
        return -1;
    }

    //int frameRate = pInputCodecContext->framerate.num;
	sprintf(task_item->videoWidth,"%d",pInputCodecContext->width);
	sprintf(task_item->videoHeight,"%d",pInputCodecContext->height);
	sprintf(task_item->realDuration,"%d",realDuration);
	sprintf(task_item->frameRate,"%d",frameRate);
	
#if 0
	///////////////记录日志//////////////
	sprintf(buff_temp,"videoWidth:%s, videoHeight:%s, framerate:%s,realDuration:%s, bmp_path:%s\n",task_item->videoWidth,task_item->videoHeight,
		task_item->frameRate,task_item->realDuration,bmp_path);
	g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
	//////////////////////////////
#endif
	/// 插入任务表	
	m_taskTable->InsertTaskItem(task_item);
	///////////////记录日志////////////
	sprintf(buff_temp,"InsertTaskItem suceed!!! Add Name:%s\n",task_item->fileName);
	g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
	//////////////////////////////////

    AVPacket InPack;
    int len = 0;
    AVFrame *ptrOutFrame=NULL;
    ptrOutFrame=av_frame_alloc();
    int nComplete=0;

	int nFrame = 0;
    AVRational avRation = pInputCodecContext->time_base;

    //  加入这句话来纠正某些编码器产生的帧速错误
    /*if(pInputCodecContext->frame_rate>1000 && pInputCodecContext->frame_rate_base==1)
         pInputCodecContext->frame_rate_base=1000;*/

    //av_seek_frame(pInputFormatContext,0,0,AVSEEK_FLAG_FRAME);

	//int dstwidth = 1920;
 //   int dsthight = 1080;

    AVPicture PictureRGB;//RGB图片
    avpicture_alloc(&PictureRGB, AV_PIX_FMT_BGR24, videoWidth, videoHeight);   // 确认所需缓冲区大小并且分配缓冲区空间

    //get  swscontext
    static struct SwsContext *img_convert_ctx;
	int dst_videoWidth=0;
	int dst_videoHeight=0;
	dst_videoWidth =atoi(task_item->dstVideoWidth);
	dst_videoHeight =atoi(task_item->dstVideoHeight);
//    img_convert_ctx = sws_getContext(videoWidth, videoHeight,	pInputCodecContext->pix_fmt, dst_videoWidth, dst_videoHeight,
//                                     AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);

	int cnt_frameRate=0;
	int cnt_frequency = atoi(task_item->frequency);
#if 0
	///////////////记录日志
	sprintf(buff_temp,"frequency:%s, cnt_frequency:%d, dst_videoWidth:%d, dst_videoHeight:%d\n",
		task_item->frequency,cnt_frequency,dst_videoWidth,dst_videoHeight);
	g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
	////////////////////
#endif
	int count_feature =0;
	///FEATURES *freatureItems;
	////保存图片，生成特征文件
	VideoCompareModule videoCompareModule;
	int featureNumber=0;
	int frequency=0;
	int duration=0;
	int fram_coun=0;
	char bmp_fullPath[255];
	char feature_fullPath[255];
	memset(bmp_fullPath,0,255);
	memset(feature_fullPath,0,255);

    while((av_read_frame(pInputFormatContext, &InPack) >= 0))
    {
        len = avcodec_decode_video2(pInputCodecContext, ptrOutFrame, &nComplete, &InPack);
//判断是否是关键帧
#if 0
        
        if(nComplete > 0&& ptrOutFrame->key_frame)
        {
            ///解码一帧成功
            SaveBmp(pInputCodecContext, ptrOutFrame, videoWidth, videoHeight,nFrame,task_item,bmp_path);
            nFrame++;

          ///  std::cout<<nFrame<<".bmp find faces:"<<len<<std::endl;
			printf("%d.bmp save succeed!!!\n",nFrame);
        }// if(nComplete > 0/*&& ptrOutFrame->key_frame*/)
#endif
#if 1
		//time_t testTime;
		//time(&testTime);
		//printf("testTime:%d\n",testTime);        
		 if(nComplete > 0)
        { 
			////frameRate
			if((cnt_frameRate % frameRate) < cnt_frequency ) 
			{
				if(nFrame< realDuration)
				{
					///解码一帧成功，保存图片
					SaveBmp(pInputCodecContext, ptrOutFrame, videoWidth, videoHeight,dst_videoWidth,dst_videoHeight ,nFrame,bmp_path);
					printf("%d.bmp save succeed!!!\n",nFrame);
					/////////////记录日志/////////////
					sprintf(buff_temp,"%d.bmp save succeed!!!\n",nFrame);
					g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
					////@end////////////////////////

					////bmp图片全路径  featureFilePath+当前帧号.bmp
					sprintf(bmp_fullPath,"%s/%d.bmp",task_item->featureFilePath,nFrame);
					////特征全路径featureFilePath+当前帧号.txt
					sprintf(feature_fullPath,"%s/%d.txt",task_item->featureFilePath,nFrame);
					///////////////记录日志////////////////
					sprintf(buff_temp,"bmp_fullPath:%s, feature_fullPath:%s\n",bmp_fullPath,feature_fullPath);
					g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
					///////////////////////////////////////

				   ///增加生成特征文件,调用xiao函数
					iRet=videoCompareModule.ExportFeature(bmp_fullPath, feature_fullPath,featureNumber);
					//if(iRet !=0)
					//{
					//	printf("ExportFeature failed!!!\n");
					//	return -1;
					//}

					/////////////////记录日志////////////////
					//sprintf(buff_temp,"ExportFeature::featureNumber:%d\n",featureNumber);
					//g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
					/////////////////////////////////////////

					//////插入记录项,将每张bmp图片信息 插入特征表
					FEATURE_ITEM *item = NULL;
					/// 申请的记录项内存空间，与DeleteSpaceItem()函数成对使用
					m_featureTable->NewTableItemSpace(&item);
					sprintf(item->id,"");
					sprintf(item->uuid,"%s",task_item->uuid);  ////广告uuid
					sprintf(item->ad_fileName,"%s",task_item->fileName);  ///广告文件名
					sprintf(item->bmp_fileName,"%d.bmp",nFrame);   ///bmp图片文件名
					if(featureNumber == 0)
					{
						sprintf(item->fileName,"");       ///特征文件名
						sprintf(item->filePath,""); ///特征文件路径
						sprintf(item->fullFilePath,"");  ////特征全路径
					}
					else
					{
						sprintf(item->fileName,"%d.txt",nFrame);       ///特征文件名
						sprintf(item->filePath,"%s",task_item->featureFilePath); ///特征文件路径
						sprintf(item->fullFilePath,"%s",feature_fullPath);  ////特征全路径
					}
				
					sprintf(item->bmp_fullFilePath,"%s",bmp_fullPath);   ///图片全路径
					sprintf(item->picture_order,"%d",nFrame);    ///图片序号
					sprintf(item->quantity,"%d",featureNumber);  ///特征数量
					frequency =atoi(task_item->frequency);
					duration = atoi(task_item->realDuration);
					sprintf(item->bmp_quantity,"%d",frequency * duration);   ///图片数量
					m_featureTable->InsertItem(item);  

					////释放申请的内存空间，与NewUserItemTbl()函数成对使用
					m_featureTable->DeleteItemSpace(&item);
				
					nFrame++;
				}
			}
			cnt_frameRate++;
			///printf("cnt_frameRate:%d, frameRate:%d\n",cnt_frameRate,frameRate);
			/////////////记录日志/////////////
			sprintf(buff_temp,"cnt_frameRate:%d, frameRate:%d, frameRate*realDuration:%d\n",cnt_frameRate,frameRate,frameRate*realDuration);
			g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
			////@end////////////////////////

		}
#endif
    }//end while

	//////********此函数未实现功能
	//iRet = SetCall(m_pFreatureItems);
	//////释放特征容器内存
	//m_featureTable->DeleteAllItemsSpace(freatureItems);

    avpicture_free(&PictureRGB);
	printf("save frame number:\n",nFrame);
    avcodec_close(pInputCodecContext);
    avformat_close_input(&pInputFormatContext);
    avformat_free_context(pInputFormatContext);
    av_frame_free(&ptrOutFrame);
	
	return iRet;
}

void TempletManager::SaveBmp(AVCodecContext *CodecContex, AVFrame *Picture, int width, int height,
	int dst_width,int dst_height,int num,char *bmp_path)
{
	/////////////////记录日志
	//sprintf(buff_temp,"SaveBmp::width:%d, height:%d, dst_width:%d, dst_height:%d\n",width,height,dst_width,dst_height);
	//g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
	////////////////////////
    AVPicture PictureRGB;//RGB图片

    static struct SwsContext *img_convert_ctx;
    img_convert_ctx = sws_getContext(width, height,	CodecContex->pix_fmt, dst_width, dst_height,
                                     AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
    // 确认所需缓冲区大小并且分配缓冲区空间
    avpicture_alloc(&PictureRGB, AV_PIX_FMT_BGR24, dst_width, dst_height);
    sws_scale(img_convert_ctx, Picture->data, Picture->linesize, 0, height, PictureRGB.data, PictureRGB.linesize);

    int lineBytes = PictureRGB.linesize[0];
    int i=0;
	
	////// 创建文件夹 /////
	char buff_temp[255];
	memset(buff_temp,0,sizeof(buff_temp));
	///sprintf(buff_temp,"/home/zyh/mp4File/%s/",task_item->uuid);
	sprintf(buff_temp,"/%s/",bmp_path);
	CFileManager filemanager;
	filemanager.CreateDir(buff_temp);
	////@end///////

	//////
    char fileName[1024]={0};
    ///char * bmpSavePath = "/home/luyan/%d.bmp";
    time_t ltime;
    time(&ltime);
    //sprintf(fileName,bmpSavePath , ltime);////////////////////////////////////////////???????????????????????????
    sprintf(fileName,"%s%d.bmp" , buff_temp,num);

    FILE *pDestFile = fopen(fileName, "wb");
    BITMAPFILEHEADER btfileHeader;
    btfileHeader.bfType = 0x4d42;
    ///btfileHeader.bfSize = lineBytes*height;dst_height
	btfileHeader.bfSize = lineBytes*dst_height;
    btfileHeader.bfReserved1 = 0;
    btfileHeader.bfReserved2 = 0;
    btfileHeader.bfOffBits = sizeof(BITMAPFILEHEADER);

    BITMAPINFOHEADER bitmapinfoheader;
    bitmapinfoheader.biSize = 40;
   /// bitmapinfoheader.biWidth = width;
	bitmapinfoheader.biWidth = dst_width;
    ///bitmapinfoheader.biHeight = height;
	bitmapinfoheader.biHeight = dst_height;
    bitmapinfoheader.biPlanes = 1;
    bitmapinfoheader.biBitCount = 24;
    bitmapinfoheader.biCompression = 0;
   /// bitmapinfoheader.biSizeImage = lineBytes*height
	bitmapinfoheader.biSizeImage = lineBytes*dst_height;
    bitmapinfoheader.biXPelsPerMeter = 0;
    bitmapinfoheader.biYPelsPerMeter = 0;
    bitmapinfoheader.biClrUsed = 0;
    bitmapinfoheader.biClrImportant = 0;

    fwrite(&btfileHeader, sizeof(BITMAPFILEHEADER), 1, pDestFile);
    fwrite(&bitmapinfoheader, sizeof(BITMAPINFOHEADER), 1, pDestFile);
    ///for(i=height-1; i>=0; i--
	for(i=dst_height-1; i>=0; i--)
    {
        fwrite(PictureRGB.data[0]+i*lineBytes, lineBytes, 1, pDestFile);
    }

    fclose(pDestFile);
    avpicture_free(&PictureRGB);
}

//int TempletManager::SetCall(FEATURES &pfeatures)
//{
//	int iRet = RET_SUCCESS;
//
//	//for(FEATURES::iterator it = m_freatureItems.begin(); it != m_freatureItems.end(); it++)
//	//{
//	//	pfeatures.push_back(*it);
//	//}
//	
//	return iRet;
//}
void TempletManager::ClearPictureItemSpace(PICTUR_ITEM *pictureItem,int quantity)
{
	if(NULL==pictureItem)
	{
		return;
	}
	
	if(NULL!=pictureItem->picturePath)
	{
		memset(pictureItem->picturePath,0,BUFF_SIZE_255);
	}
	if(NULL!=pictureItem->pictureFullFilePath)
	{
		memset(pictureItem->pictureFullFilePath,0,BUFF_SIZE_255);
	}
	if(NULL!=pictureItem->addr)
	{
		memset(pictureItem->addr,0,quantity * FEATURE_SIZE);
	}
	pictureItem->length = 0;
	pictureItem->picture_order = 0;
	pictureItem->quantity =0;

}

void TempletManager::NewPictureItemSpace(PICTUR_ITEM **pictureItem,int quantity)
{
	if(NULL!=(*pictureItem))
	{
		return;
	}
	(*pictureItem)=(PICTUR_ITEM *)new PICTUR_ITEM;
	(*pictureItem)->addr=new char[quantity * FEATURE_SIZE];
	(*pictureItem)->picturePath=new char[BUFF_SIZE_255];
	(*pictureItem)->pictureFullFilePath=new char[BUFF_SIZE_255];
	(*pictureItem)->write_file=new char[BUFF_SIZE_255];

	ClearPictureItemSpace((*pictureItem),quantity);
}

void TempletManager::DeletePictureSpace(PICTUR_ITEM **pictureItem,int quantity)
{
	if(NULL==(*pictureItem))
	{
		return;
	}
	ClearPictureItemSpace((*pictureItem),quantity);
	if(NULL!=(*pictureItem)->addr)
	{
		delete [] (*pictureItem)->addr;
		(*pictureItem)->addr=NULL;
	}
	if(NULL!=(*pictureItem)->picturePath)
	{
		delete [] (*pictureItem)->picturePath;
		(*pictureItem)->picturePath=NULL;
	}
	if(NULL!=(*pictureItem)->pictureFullFilePath)
	{
		delete [] (*pictureItem)->pictureFullFilePath;
		(*pictureItem)->pictureFullFilePath=NULL;
	}
	if(NULL!=(*pictureItem)->write_file)
	{
		delete [] (*pictureItem)->write_file;
		(*pictureItem)->write_file=NULL;
	}
	
}


int TempletManager::GetAllTemplets(TEMPLET_LIST &templet_list)
{
	int iRet=RET_SUCCESS;
	
	VideoCompareModule videoCompareModule;
	
	FEATURES feature_items;  ///特征list
	FeatureTable featureObject; ///特征表对象
	int real_length=0; ///实际长度
	int quantity=0;
	/// 获取所有任务记录项
	TaskTable my; ///创建任务表类对象
	TASKS tast_items;

	///iRet = my.GetAllItems(&tast_items);
	iRet = my.GetAllValidItems(&tast_items); ///获取有效期内的所有任务
	if( DB_SUCCESS != iRet)
	{
		printf("GetAllItems failed! iRet=%d\n",iRet);
	}
	int i=0; ///打印task表用
	int duration=0;
	int frequency =0; ///每秒取的帧数

	if(tast_items.size() == 0)
	{
		printf("No task item can be get!!!  size=%d\n",tast_items.size() );
	}
	else
	{
		for(TASKS::iterator itTask = tast_items.begin(); itTask != tast_items.end(); itTask++)
		{
			TASK_ITEM *pTaskItem = (TASK_ITEM *)(*itTask);
#if 1
			/// 打印结果
		
			printf("****************TASK TABLE  i=%d *********\n",i);
			printf("id:%s,		uuid:%s\n",pTaskItem->id,pTaskItem->uuid);
			///printf("orig_fileName:%s\n",pTaskItem->orig_fileName);
			///printf("fileName:%s\n",pTaskItem->fileName);
			///printf("filePath:%s\n",pTaskItem->filePath);
			printf("fullFilePath:%s\n",pTaskItem->fullFilePath);
			printf("startDateTime:%s --- endDateTime:%s\n",pTaskItem->startDateTime,pTaskItem->endDateTime);
			printf("hall_id:%s,		ad_order:%s\n",pTaskItem->hall_id,pTaskItem->ad_order);
			printf("frameRate:%s,		frequency:%s\n",pTaskItem->frameRate,pTaskItem->frequency);
			printf("videoWidth * videoHeight: %s*%s\n",pTaskItem->videoWidth,pTaskItem->videoHeight);
			printf("dstVideoWidth * dstVideoHeight: %s*%s\n",pTaskItem->dstVideoWidth,pTaskItem->dstVideoHeight);
			printf("type:%s,		realDuration:%s\n",pTaskItem->realDuration,pTaskItem->type);
			///printf("description:%s\n",pTaskItem->description);

			printf("-----------------------------------------------\n");
			i++;
#endif
			TEMPLET_ITEM *templet_item = NULL;

			templet_item =(TEMPLET_ITEM *)new TEMPLET_ITEM;

			templet_item->uuid = new char[BUFF_SIZE_50];
			memset(templet_item->uuid,0,BUFF_SIZE_50);
			sprintf(templet_item->uuid,"%s",pTaskItem->uuid);

			templet_item->ad_fileName = new char[BUFF_SIZE_255];
			memset(templet_item->ad_fileName,0,BUFF_SIZE_255);
            //sprintf(templet_item->ad_fileName,"%s",pTaskItem->fileName);
            sprintf(templet_item->ad_fileName,"%s",pTaskItem->orig_fileName);

			templet_item->dstVideoWidth = new char[BUFF_SIZE_255];
			memset(templet_item->dstVideoWidth,0,BUFF_SIZE_255);
			sprintf(templet_item->dstVideoWidth,"%s",pTaskItem->dstVideoWidth);

			templet_item->dstVideoHeight = new char[BUFF_SIZE_255];
			memset(templet_item->dstVideoHeight,0,BUFF_SIZE_255);
			sprintf(templet_item->dstVideoHeight,"%s",pTaskItem->dstVideoHeight);

			duration = atoi(pTaskItem->realDuration);
			frequency = atoi(pTaskItem->frequency);
			templet_item->picture_quantity = duration * frequency;
			///sprintf(templet_item->ad_order,"",pTaskItem->ad_order);
			templet_item->ad_order = atoi(pTaskItem->ad_order);
#if 1
			featureObject.GetItemByUuid(&feature_items,pTaskItem->uuid);
			if( DB_SUCCESS != iRet)
			{
				printf("GetAllFeatures failed! iRet=%d\n",iRet);
			}

			for(FEATURES::iterator it = feature_items.begin(); it != feature_items.end(); it++)
			{
				FEATURE_ITEM *pFeatureItem = (FEATURE_ITEM *)(*it);
#if 1
				/// 打印结果
				printf("	****************FEATURE TABLE [%s]***********\n",pFeatureItem->picture_order);
				printf("	id:%s,		uuid:%s\n",pFeatureItem->id,pFeatureItem->uuid);
				printf("	ad_fileName:%s,		bmp_fileName:%s\n",pFeatureItem->ad_fileName,pFeatureItem->bmp_fileName);
				printf("	fileName:%s,		filePath:%s\n",pFeatureItem->fileName,pFeatureItem->filePath);
				///printf("	fullFilePath:%s\n",pFeatureItem->fullFilePath);
				printf("	bmp_fullFilePath:%s,		picture_order:%s\n",pFeatureItem->bmp_fullFilePath,pFeatureItem->picture_order);
				printf("	quantity:%s,		bmp_quantity:%s\n",pFeatureItem->quantity,pFeatureItem->bmp_quantity);
				///printf("-----------------------------------------------\n");
			
#endif
			quantity = atoi(pFeatureItem->quantity);
			if(quantity !=0)
			{
				PICTUR_ITEM *pPictureItem = NULL;

				NewPictureItemSpace(&pPictureItem,quantity);
				sprintf(pPictureItem->pictureFullFilePath,"%s",pFeatureItem->bmp_fullFilePath);
				sprintf(pPictureItem->picturePath,"%s",pFeatureItem->filePath);
				sprintf(pPictureItem->write_file,"%s/outbuffer",pFeatureItem->filePath);
				pPictureItem->picture_order = atoi(pFeatureItem->picture_order);
				/// add quantity
				pPictureItem->quantity = atoi(pFeatureItem->quantity);

				///调用xiao的库
				videoCompareModule.ImportFeature(pFeatureItem->fullFilePath,pPictureItem->quantity,pPictureItem->addr,pPictureItem->length);
				
				templet_item->picture_list.push_back(pPictureItem);
				///////////////记录日志
				sprintf(buff_temp,"ImportFeature:length= %d, picture_order=%d, pictureFullFilePath:%s,add_length:%d,write_file :%s\n",
					pPictureItem->length,pPictureItem->picture_order,pPictureItem->pictureFullFilePath,
					pPictureItem->quantity * FEATURE_SIZE,pPictureItem->write_file);
				g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
				////////////////////
			}
			
		}///end feature
		
		templet_list.push_back(templet_item);
		/// 释放空间
		DeleteAllFeaturesSpace(&feature_items);
		
	#endif
		} ///task end;
	}
	//////测试保存内存/////
#if 0
	///TEMPLET_LIST templet_list;
	char open_fileName[255];
	FILE *pffeature_data = NULL;
	///templet_object.GetAllTemplets(templet_list);
	///printf("******************* GetAllTemplets **************\n");
	
	for(TEMPLET_LIST::iterator it_templets = templet_list.begin(); it_templets != templet_list.end(); it_templets++)
	{
		printf("*******************out templet **************\n");
		///PICTUR_ITEM *picture_item = (PICTUR_ITEM *)(*it_picture);
		printf("uuid:%s\n",(*it_templets)->uuid);
		printf("ad_fileName:%s\n",(*it_templets)->ad_fileName);
	
		for(PICTURE_LIST::iterator it_picture = (*it_templets)->picture_list.begin(); it_picture != (*it_templets)->picture_list.end(); it_picture++)
		{
			printf("	******************* out picture_order:%d **************\n",(*it_picture)->picture_order);
			printf("	pictureFullFilePath:%s\n",(*it_picture)->pictureFullFilePath);
			//printf("picturePath :%s\n",(*it_picture)->picturePath);
			///printf("picture_order :%s\n",(*it_picture)->picture_order);
			//printf("write_file :%s\n",(*it_picture)->write_file);
			sprintf(open_fileName,"%s/%d.txt",(*it_picture)->write_file,(*it_picture)->picture_order);
			printf("	open_fileName :%s\n",open_fileName);

			///先判断文件夹是否存在
			if(access((*it_picture)->write_file,0)!=0)
			{
					mkdir((*it_picture)->write_file,0755);
					///////////////记录日志///////////////
				sprintf(buff_temp,"create path  %s succeed!!!\n",open_fileName);
				g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
				/////////////////////////////////////////
			}
			pffeature_data = fopen(open_fileName,"wb");
			fwrite((*it_picture)->addr,1,(*it_picture)->length,pffeature_data);
			
			///////////////记录日志///////////////////
			sprintf(buff_temp,"write %s succeed!!! length :%d \n",open_fileName,(*it_picture)->length);
			g_templet_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
			/////////////////////////////////////////
			
		}

	///printf("*******************out end**************\n");
	}
	fclose(pffeature_data);
	pffeature_data = NULL;


#endif

	
#if 1
	/// 释放task表空间
	for(TASKS::iterator itTask = tast_items.begin(); itTask != tast_items.end(); itTask++)
	{
		TASK_ITEM *pTaskItem = (TASK_ITEM *)(*itTask);

		my.DeleteItemSpace(&pTaskItem);
	}
	tast_items.clear();
#if 0
	///释放templet_items空间
	DeleteTemplet_list(&templet_list);
#endif
#endif


	return iRet;
}
int TempletManager::DeleteTemplet_list(TEMPLET_LIST *pTemplet_list)
{
	int iRet = RET_SUCCESS;

	if(pTemplet_list !=NULL)
	{
	for(TEMPLET_LIST ::iterator itTemplet = pTemplet_list->begin(); itTemplet != pTemplet_list->end(); itTemplet++)
	{
		///printfd_fileNa22222222222222222,me=%s\n",(*itTemplet)->ad_fileName);
		///printfuid=%s\n22222222222222222,",(*itTemplet)->uuid);

		for(PICTURE_LIST ::iterator itPicture = (*itTemplet)->picture_list.begin(); itPicture != (*itTemplet)->picture_list.end(); itPicture++)
		{
		///	printquantity=22222222222222222,%d\n",(*itPicture)->quantity);
		///	printpicture_o22222222222222222,rder=%d\n",(*itPicture)->picture_order);
		///	printpicture_o22222222222222222,rder=%s\n",(*itPicture)->pictureFullFilePath);
			DeletePictureSpace(&*itPicture,(*itPicture)->quantity);
		}
		(*itTemplet)->picture_list.clear();

			if(NULL!=(*itTemplet)->ad_fileName)
			{
				delete [] (*itTemplet)->ad_fileName;
				(*itTemplet)->ad_fileName=NULL;
			}
			if(NULL!=(*itTemplet)->uuid)
			{
				delete [] (*itTemplet)->uuid;
				(*itTemplet)->uuid=NULL;
			}
		}
	}
	pTemplet_list->clear();


	return iRet;
}
