#include <sys/time.h>
#include <syscall.h>
#include <type.h>
#include "MarkJob.h"
#include "MarkJob_types.h"
#include "utility/FileManager_linux.h"
#include "log/MyLogger.h"



#define PARSE_KEY_FRAME_ONLY 1
extern bool g_bAresQuit;
extern MyLogger g_markjob_logwrite;


#define SCALE_PICTURE_TEST	0


void *markjob_thread(LPVOID pvoid)
{
	MarkJob *mark_proc = (MarkJob *)pvoid;

	mark_proc->markjob_procedure();

#ifndef _WIN32
	pthread_exit(0);
#endif

}


MarkJob::MarkJob()
{	
#ifdef _WIN32
	m_separator_type = 0;
#else
	m_separator_type = 1;
#endif	
	m_isrunning = 0;
	m_markjob_state = MARK_STATE_STOP;
	memset(&m_lasterror,0,sizeof(LAST_ERROR_REPORT));
	pthread_mutex_init(&m_mutex_markstate,NULL);

	m_pfunc_markcallback = NULL;
	m_puserdata = NULL;

}
MarkJob::~MarkJob()
{
}
int MarkJob::Initialize(int max_frame_count,char *ppath_mark,Rect check_rect)
{
	int ret = MARK_JOB_SUCCESS;
	char buff_temp[512] = {'\0'};

	ret = m_framebufferloop.InitFrameBuffer(max_frame_count);

	if( MARK_JOB_SUCCESS != ret )
	{
		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob initialize failed,frame count is invald,framecount:%d\n",max_frame_count);
		g_markjob_logwrite.PrintLog(MyLogger::FATAL,"%s",buff_temp);

		return ret;
	}

	m_markjob.max_frame_count = max_frame_count;

	m_pmarkengine = new MarkEngine(ppath_mark,check_rect);

	ret = m_pmarkengine->LoadFeatrue();
	if( !ret )
	{
		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob initialize failed,could not load featrue for mark,ret:%d\n",ret);
		g_markjob_logwrite.PrintLog(MyLogger::FATAL,"%s",buff_temp);

		return MARK_JOB_INITIALIZE_ERROR_LOAD_FEATURE;
	}

    //注册库中含有的所有可用的文件格式和编码器，这样当打开一个文件时，它们才能够自动选择相应的文件格式和编码器。
    av_register_all();
    av_log_set_level(AV_LOG_QUIET);///不显示打印信息

	return MARK_JOB_SUCCESS;
}
int MarkJob::UnInitialize()
{
	int ret = MARK_JOB_SUCCESS;

	m_framebufferloop.DestroyFrameBuffer();

    if(m_pmarkengine)
    {
        delete m_pmarkengine;
    }

	return ret;
}
int MarkJob::CreateMarkJob(MARK_JOB_ITEM *pmarkjob,pfunc_callback_mark pmarkcallback,void *puserdata)
{
	int ret = MARK_JOB_SUCCESS;
	char buff_temp[512] = {'\0'};

	if(NULL == pmarkjob)
	{
		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob create failed,job struct is null\n");
		g_markjob_logwrite.PrintLog(MyLogger::FATAL,"%s",buff_temp);

		return ret;
	}

#if 0
	if(pmarkjob->max_frame_count <= 0 )
	{
		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob create failed,frame count is invald,framecount:%d\n",pmarkjob->max_frame_count);
		g_markjob_logwrite.PrintLog(MyLogger::FATAL,"%s",buff_temp);

		return ret;
	}
#endif

	if(m_isrunning)
	{
		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob thread is running already\n");
		g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
		return ret;
	}


	int frames = 0;
	int seconds = 0;
	int width = 0;
	int height = 0;
	int framerate = 0;

	GetVideoInfo(pmarkjob->videopath,frames,seconds,width,height,framerate);
	if(pmarkjob->decode_start_pos>=seconds)
	{
		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"create markjob is failed,start pos is out range,error code:%d\n",MARK_JOB_CREATEJOB_ERROR_STARTPOS_OUTRANGE);
		g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
		printf("%s\n",buff_temp);
		return MARK_JOB_CREATEJOB_ERROR_STARTPOS_OUTRANGE;
	}



	pmarkjob->max_frame_count = m_markjob.max_frame_count;

	memset(&m_markjob,0,sizeof(m_markjob));
	memcpy(&m_markjob,pmarkjob,sizeof(m_markjob));
	m_pfunc_markcallback = pmarkcallback;
    m_puserdata = puserdata;
		
	memset(&m_markjob_result,0,sizeof(m_markjob_result));
	sprintf(m_markjob_result.markjob_id,"%s",pmarkjob->markjob_id);

	memset(buff_temp,0,sizeof(buff_temp));
	sprintf(buff_temp,"markjob create:parameters:\n");
	sprintf(buff_temp,"%s max frame count:%d\n",buff_temp,pmarkjob->max_frame_count);
	g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

	init_ffmpeg_decoder(&m_ffmpeg_decoder,&m_markjob);


	pthread_attr_t attr;
	unsigned long pthread_mark_proc = 0;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
	ret = pthread_create(&pthread_mark_proc, &attr, markjob_thread, this);
//	ret = pthread_create(&pthread_mark_proc, NULL, markjob_thread, this);
	pthread_attr_destroy (&attr);
	if( ret )
	{

		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"thread:mark time procedure create failed:%d\n",ret);
		sprintf(buff_temp,"%s max frame count:%d\n",pmarkjob->max_frame_count);

		g_markjob_logwrite.PrintLog(MyLogger::FATAL,"%s",buff_temp);


		return ret;
	}

	int runningcounter = 0;
	while(1)
	{
		if(m_isrunning)
		{
			break;
		}

		runningcounter++;

		if(runningcounter > 100)
		{
			break;
		}
        usleep(100000);
	}

	memset(buff_temp,0,sizeof(buff_temp));
	sprintf(buff_temp,"markjob procedure startup timeout:%d,100\n",runningcounter);
	g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

	return ret;
}
int MarkJob::SetMarkControl(MARK_JOB_CONTROL markcontrol)
{
	int ret = MARK_JOB_SUCCESS;

	/// pausing--->running  stop
	/// running--->puaing  stop

	pthread_mutex_lock(&m_mutex_markstate);

	if ( MARK_CONTROL_STOP == markcontrol )
	{
		if((m_markjob_state == MARK_STATE_RUNNING) ||
			(m_markjob_state == MARK_STATE_PAUSING))
		{
			m_markjob_state = MARK_STATE_FINISHED_ABORT;

		}
		else
		{
			pthread_mutex_unlock(&m_mutex_markstate);

			return MARK_JOB_STOP_ERROR_NOTIN_RUNNING_PAUSING;

		}
	}
	else if ( MARK_CONTROL_START == markcontrol )
	{

		if((m_markjob_state == MARK_STATE_RUNNING) ||
			(m_markjob_state == MARK_STATE_PAUSING))
		{
			m_markjob_state = MARK_STATE_RUNNING;

		}
		else
		{
			pthread_mutex_unlock(&m_mutex_markstate);

			return MARK_JOB_START_ERROR_NOTIN_RUNNING_PAUSING;
		}
	}
	else
	{
		if((m_markjob_state == MARK_STATE_RUNNING) ||
			(m_markjob_state == MARK_STATE_PAUSING))
		{
			m_markjob_state = MARK_STATE_PAUSING;

		}
		else
		{
			pthread_mutex_unlock(&m_mutex_markstate);

			return MARK_JOB_PAUSE_ERROR_NOTIN_RUNNING_PAUSING;
		}
	}



	pthread_mutex_unlock(&m_mutex_markstate);


	return ret;
}
MARK_JOB_STATE MarkJob::GetMarkState()
{
	return m_markjob_state;
}
int MarkJob::RetsetMarkState()
{
	int ret = MARK_JOB_SUCCESS;

	pthread_mutex_lock(&m_mutex_markstate);

	if((m_markjob_state == MARK_STATE_RUNNING) ||
		(m_markjob_state == MARK_STATE_PAUSING))
	{
		pthread_mutex_unlock(&m_mutex_markstate);

		return MARK_JOB_RESET_STATE_ERROR_IN_RUNNING_PAUSING;
	}

	m_markjob_state = MARK_STATE_STOP;

	pthread_mutex_unlock(&m_mutex_markstate);

	return ret;
}
int MarkJob::markjob_procedure()
{
	int ret = MARK_JOB_SUCCESS;
	int ret_ffmpeg = 0;
	int ret_markengine = 0;
	bool bret = false;
	char buff_temp[512] = {'\0'};
	char time_start[50] = {'\0'};
	char time_current[50] = {'\0'};


    AVPacket InPack;

	struct SwsContext* pimg_convert_ctx_test = NULL; 
	AVPicture avpicture_convert_test;

	int frame_counter = 0;
	int frame_index = 0;
    int frame_len = 0;
    int ncomplete=0;
    int uncomplete_counter=0;
	int percent = 0;

	char filepath_mark[255] = {'\0'};

	AV_FRAME_INFO *pframeinfo = NULL;
	CFileManager filemanager;

	m_isrunning = 1;
	m_markjob_state = MARK_STATE_RUNNING;

#if 1
	int lwpid;
	lwpid=syscall(SYS_gettid);
	memset(buff_temp, 0, sizeof(buff_temp));
	sprintf(buff_temp, "thread:mark job procedure enter,lwpid:%d\n",lwpid);
	g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
	printf("%s\n",buff_temp);
#endif  

	m_framebufferloop.NewSpaceItem(&pframeinfo);

	m_framebufferloop.SetContol(BUFFER_CONTROL_WRITE);


	///job start
#if 0
	sleep(10);
#endif


#if SCALE_PICTURE_TEST
		m_scaletest_width_src = m_ffmpeg_decoder.video_width_src;
		m_scaletest_height_src = m_ffmpeg_decoder.video_height_src;

		m_pix_fmt_scaletest_src = m_ffmpeg_decoder.m_pix_fmt_src;

		//m_scaletest_width_dest = video_width;
		//m_scaletest_height_dest = video_height;
		m_scaletest_width_dest = 800;
		m_scaletest_height_dest = 600;

		//m_pix_fmt_scaletest_dest = AV_PIX_FMT_GRAY8;
		//m_pix_fmt_scaletest_dest = AV_PIX_FMT_YUV420P;
		//m_pix_fmt_scaletest_dest = AV_PIX_FMT_BGR24;
		m_pix_fmt_scaletest_dest = AV_PIX_FMT_RGB24;


		m_scaletest_flag_dest = SWS_FAST_BILINEAR;


		avpicture_alloc(&avpicture_convert_test, 
						m_pix_fmt_scaletest_dest,
						m_scaletest_width_dest,
						m_scaletest_height_dest);

		pimg_convert_ctx_test = sws_getContext(m_scaletest_width_src,
										m_scaletest_height_src,
										m_pix_fmt_scaletest_src,
										m_scaletest_width_dest,
										m_scaletest_height_dest,
										m_pix_fmt_scaletest_dest, 
										m_scaletest_flag_dest,
										NULL,
										NULL,
										NULL);

#endif


    int prepercent=0;
    while(1)
    {
		if(MARK_STATE_RUNNING == m_markjob_state )		
		{
            if(g_bAresQuit)
            {
                break;
            }

			ret_ffmpeg = av_read_frame(m_ffmpeg_decoder.pInputFormatContext, &InPack);

#if 0
			memset(buff_temp,0,sizeof(buff_temp));
			sprintf(buff_temp,"read frame,avpack,stream_index:%d,size:%d,flags:%d,duration:%d,pos:%d\n",
							InPack.size,
							InPack.stream_index,
							InPack.flags,
							InPack.duration,
							InPack.pos);
			//g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

			printf("%s\n",buff_temp);	
#endif

			if(ret_ffmpeg>=0)
			{
				frame_len = avcodec_decode_video2(m_ffmpeg_decoder.pInputCodecContext, m_ffmpeg_decoder.ptrOutFrame, &ncomplete, &InPack);

				if(ncomplete > 0)
				{
					frame_counter++;

					//if( (ncomplete > 0) && (ptrOutFrame->key_frame))
					if(!(frame_counter % m_ffmpeg_decoder.framerate))
					{
					
#if 0
						memset(buff_temp,0,sizeof(buff_temp));
						sprintf(buff_temp,"decode video2,frame_len:%d,ncomplete:%d,width:%d,height:%d,nb_samples:%d,format:%d,key_frame:%d,AV_NUM_DATA_POINTERS:%d\n"
										"pict_type:%d,sample_aspect_ratio.num:%d,sample_aspect_ratio.den:%d,coded_picture_number:%d,display_picture_number:%d,quality:%d\n"
										"flags:%d,color_range:%d,color_primaries:%d,color_trc:%d,colorspace:%d,chroma_location:%d,best_effort_timestamp:%d\n\n\n",
										frame_len,
										ncomplete,
										m_ffmpeg_decoder.ptrOutFrame->width,
										m_ffmpeg_decoder.ptrOutFrame->height,
										m_ffmpeg_decoder.ptrOutFrame->nb_samples,
										m_ffmpeg_decoder.ptrOutFrame->format,		//AVPixelFormat   AVSampleFormat
										m_ffmpeg_decoder.ptrOutFrame->key_frame,
										AV_NUM_DATA_POINTERS,
										m_ffmpeg_decoder.ptrOutFrame->pict_type,
										m_ffmpeg_decoder.ptrOutFrame->sample_aspect_ratio.num,
										m_ffmpeg_decoder.ptrOutFrame->sample_aspect_ratio.den,
										m_ffmpeg_decoder.ptrOutFrame->coded_picture_number,
										m_ffmpeg_decoder.ptrOutFrame->display_picture_number,
										m_ffmpeg_decoder.ptrOutFrame->quality,
										m_ffmpeg_decoder.ptrOutFrame->flags,
										m_ffmpeg_decoder.ptrOutFrame->color_range,
										m_ffmpeg_decoder.ptrOutFrame->color_primaries,
										m_ffmpeg_decoder.ptrOutFrame->color_trc,
										m_ffmpeg_decoder.ptrOutFrame->colorspace,
										m_ffmpeg_decoder.ptrOutFrame->chroma_location,
										m_ffmpeg_decoder.ptrOutFrame->best_effort_timestamp
										);
						//g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

						printf("%s\n",buff_temp);	
#endif


						//pInputCodecContext->width pInputCodecContext->height  pInputCodecContext->pix_fmt
						//if (pstream_info->dec_ctx->pix_fmt == AV_PIX_FMT_YUV420P) //如果是yuv420p的   
						//if (ptrOutFrame->format == AV_PIX_FMT_YUV420P) //如果是yuv420p的
						

						///转换
						if(m_ffmpeg_decoder.markjob.bisadjust_aspect)
						{
							/// 转换
							ret_ffmpeg = sws_scale(m_ffmpeg_decoder.pimg_convert_ctx, 
										m_ffmpeg_decoder.ptrOutFrame->data,
										m_ffmpeg_decoder.ptrOutFrame->linesize,
										0, 
										m_ffmpeg_decoder.video_height_src,
										m_ffmpeg_decoder.avpicture_convert.data,
										m_ffmpeg_decoder.avpicture_convert.linesize);

							if(ret_ffmpeg != m_ffmpeg_decoder.video_height_dest)
							{
								memset(buff_temp,0,sizeof(buff_temp));
								sprintf(buff_temp,"sws_scale failed 1,ret:%d\n",ret_ffmpeg);
								g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

 								printf("%s\n",buff_temp);

								return 1;
							}

							fill_frameinfo_by_avpicture(&m_ffmpeg_decoder.avpicture_convert,
												m_ffmpeg_decoder.video_width_dest,
												m_ffmpeg_decoder.video_height_dest,
												m_ffmpeg_decoder.m_pix_fmt_dest,
												frame_index,
												pframeinfo);

#if 0
							save_yuv(&m_ffmpeg_decoder.avpicture_convert,
										m_ffmpeg_decoder.m_pix_fmt_dest,
										m_ffmpeg_decoder.video_width_dest,
										m_ffmpeg_decoder.video_height_dest,
										frame_index+m_ffmpeg_decoder.markjob.decode_start_pos);
#endif


						}
						else
						{
							///不转换直接保存
							fill_frameinfo_by_avframe(m_ffmpeg_decoder.ptrOutFrame,frame_index,pframeinfo);
						}


#if SCALE_PICTURE_TEST
						ret_ffmpeg = sws_scale(pimg_convert_ctx_test, 
									m_ffmpeg_decoder.ptrOutFrame->data,//avpicture_convert.data,   ptrOutFrame->data,
									m_ffmpeg_decoder.ptrOutFrame->linesize,//avpicture_convert.linesize, ptrOutFrame->linesize
									0, 
									m_scaletest_height_src,
									avpicture_convert_test.data,
									avpicture_convert_test.linesize);

						if(ret_ffmpeg != m_scaletest_height_dest)
						{
							memset(buff_temp,0,sizeof(buff_temp));
							sprintf(buff_temp,"sws_scale failed 2,ret:%d\n",ret_ffmpeg);
							g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

 							printf("%s\n",buff_temp);

							return 1;
						}

						save_bmp(&avpicture_convert_test,
							m_scaletest_width_dest,
							m_scaletest_height_dest,
							frame_index+m_ffmpeg_decoder.markjob.decode_start_pos);
#endif


						///保存帧数据
						m_framebufferloop.WriteFrameData(pframeinfo);
						

						if(m_ffmpeg_decoder.markjob.bisfind_mark_flag)
						{

							///查找龙标
							char framename[50] = {'\0'};
							sprintf(framename,"%s_%d.bmp",m_markjob.markjob_id,pframeinfo->framenum);
						
							filemanager.GeneratePathString(filepath_mark,
															m_separator_type,
															sizeof(filepath_mark),
															m_ffmpeg_decoder.markjob.mark_store_path,
															framename,
															"args_end");
							ret_markengine = m_pmarkengine->FindLongbiao((char *)pframeinfo->data,
													pframeinfo->length,
													pframeinfo->width,
													pframeinfo->height,
													filepath_mark);

							if(ret_markengine>0)
							{
								///找到龙标
								m_markjob_result.bisfind = true;
								//m_markjob_result.find_pos = frame_index;
								m_markjob_result.find_pos = frame_index+m_ffmpeg_decoder.markjob.decode_start_pos;
								m_markjob_state = MARK_STATE_FINISHED;

								break;
							}
						}

						frame_index++;

#if 1

						percent = frame_index * 100 / (m_ffmpeg_decoder.duration_in_seconds - m_ffmpeg_decoder.markjob.decode_start_pos);
                        if(prepercent < percent)
                        {
                            g_markjob_logwrite.PrintLog(MyLogger::INFO,"markjob percent:%d\%",percent);
                            prepercent = percent;
                        }

#endif

					}
				}
				else
				{
					uncomplete_counter++;
				}


#if 0
				//判断是否是关键帧
#if PARSE_KEY_FRAME_ONLY
				if(ncomplete > 0 && m_ffmpeg_decoder.ptrOutFrame->key_frame)
#else
				if(ncomplete != 0)
#endif
				{
					//解码一帧成功
	     
		 //         InPack.pts = av_rescale_q(InPack.pts,m_ffmpeg_decoder.pInputCodecContext->time_base,m_ffmpeg_decoder.ptrOutFrame->time_base); 

					///填充帧数据
					SaveBmp(m_ffmpeg_decoder.pInputCodecContext,
							m_ffmpeg_decoderptrOutFrame,
							video_width,
							video_height,
							frame_index,
							pframeinfo);

				  ///保存帧数据
				  m_framebufferloop.WriteFrameData(pframeinfo);

				  frame_index++;

				  //printf("frame count:%d,height:%d,size:%d pictype:%d package.pts:%d out.pts:%d\n",nFrame,videoHeight,ptrOutFrame->linesize[0],ptrOutFrame->pict_type,InPack.pts,ptrOutFrame->pts);
				}// if(nComplete > 0/*&& ptrOutFrame->key_frame*/)
				else
				{
				   // printf("***uncomplete frame %d\n",ptrOutFrame->pict_type);
					uncomplete_counter++;
				}	
#endif

			}
			else
			{	///失败
				m_markjob_state = MARK_STATE_FINISHED_ERROR;
				break;
			}




		}
		else if(MARK_STATE_PAUSING == m_markjob_state)
		{
			///暂停状态
			sleep(1);
		}
		else
		{
			/// 其他状态(完成、错误、手动停止、停止)

			break;
		}



	}//while

#if !PARSE_KEY_FRAME_ONLY
    printf("uncomplete_counter %d\n",uncomplete_counter);
    for(int i=uncomplete_counter ;i>0;i--)
    {
        InPack.data=NULL;
		InPack.size=0;
        len = avcodec_decode_video2(pInputCodecContext, ptrOutFrame, &ncomplete, &InPack);
		if(ncomplete!=0)
		{
	   
			  frame_index++;
			  printf("frame count:%d,height:%d,size:%d pictype:%d  out.pts:%d\n",nFrame,videoHeight,len,ptrOutFrame->pict_type,ptrOutFrame->pts);
	
			  SaveBmp(pInputCodecContext,
						ptrOutFrame,
						video_width, 
						video_height,
						frame_index,
						pframeinfo);
		}
		else
		{
		  printf("second decodec failed\n");
		}

    }
#endif

	if( MARK_STATE_RUNNING == m_markjob_state)
	{
		m_markjob_state = MARK_STATE_FINISHED;
	}

#if SCALE_PICTURE_TEST
	avpicture_free(&avpicture_convert_test);
	sws_freeContext(pimg_convert_ctx_test);
#endif

	uninit_ffmpeg_decoder(&m_ffmpeg_decoder);

	m_isrunning = 0;

	m_framebufferloop.SetContol(BUFFER_CONTROL_IDLE);

	m_markjob_result.state = m_markjob_state;

	m_markjob_result.pframeloop = &m_framebufferloop;

	m_pfunc_markcallback(m_puserdata,&m_markjob_result);

#if 1
	memset(buff_temp, 0, sizeof(buff_temp));
	sprintf(buff_temp, "thread:mark job procedure exit,lwpid:%d\n",lwpid);
	g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
#endif  

	return ret;
}
int MarkJob::GetVideoInfo(const char *ppath_video,
						int &frames,
						int &seconds,
						int &width,
						int &height,
						int &framerate)
{
	int ret = MARK_JOB_SUCCESS;
	int ret_ffmpeg = 0;
	char buff_temp[1024] = {'\0'};
    struct AVFormatContext *pInputFormatContext = NULL;
    struct AVCodecContext *pInputCodecContext = NULL;
	struct AVStream *pvideostream = NULL;

    av_register_all();

    pInputFormatContext = avformat_alloc_context();
    if(NULL == pInputFormatContext)
    {
		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob alloc memory for avformat context failed\n");
		g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

 		printf("%s\n",buff_temp);
		return -1;
	}

    // 打开视频文件
	ret_ffmpeg = avformat_open_input(&pInputFormatContext, ppath_video, NULL, NULL);
    if(ret_ffmpeg)
    {
		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"open file failed,file:%s,ret_ffmpeg:%d\n",ppath_video,ret_ffmpeg);
		g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

 		printf("%s\n",buff_temp);
		return -1;
	}

#if 1
	/// av文件信息
	memset(buff_temp,0,sizeof(buff_temp));
	sprintf(buff_temp,"avformat context,duration:%d,nb_streams:%d,bitrate:%d,filename:%s\n",
		pInputFormatContext->duration/1000000,
		pInputFormatContext->nb_streams,
		pInputFormatContext->bit_rate/1000,
		pInputFormatContext->filename
		);
	g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);


	//get_metadata(pffmpeg_decoder->pInputFormatContext);

	printf("%s\n",buff_temp);
#endif


    // 取出文件流信息 填充AVFormatContext中的音视频流结构
	ret_ffmpeg = avformat_find_stream_info(pInputFormatContext,NULL);

    if(ret_ffmpeg<0)
	{
		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob can't find suitable codec parameters,file:%s,ret_ffmpeg:%d\n",ppath_video,ret_ffmpeg);
		g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

 		printf("%s\n",buff_temp);
		return -1;
	}

    //print media info
    av_dump_format(pInputFormatContext, 0, ppath_video, false);

    //仅仅处理视频流
    //只简单处理我们发现的第一个视频流
    //  寻找第一个视频流
    int videoIndex = -1;
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
		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob can't find video stream,file:%s\n",ppath_video);
		g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

 		printf("%s\n",buff_temp);
		return -1;
    }

    // 得到视频流编码上下文的指针
	pvideostream = pInputFormatContext->streams[videoIndex];

	frames = pvideostream->nb_frames;
	seconds = pvideostream->duration / pvideostream->time_base.den;

	memset(buff_temp,0,sizeof(buff_temp));
	sprintf(buff_temp,"avstream,stream index:%d,time_base.num:%d,time_base.den:%d,start_time:%d,duration:%d,nb_frames:%d\n"
					"sample_aspect_ratio.num:%d,sample_aspect_ratio.den:%d,avg_frame_rate.num:%d,avg_frame_rate.den:%d\n"
					"r_frame_rate.num:%d,r_frame_rate.den:%d,display_aspect_ratio.num:%d,display_aspect_ratio.den:%d\n",
					pvideostream->index,
					pvideostream->time_base.num,
					pvideostream->time_base.den,
					pvideostream->start_time,
					pvideostream->duration,
					pvideostream->nb_frames,
					pvideostream->sample_aspect_ratio.num,
					pvideostream->sample_aspect_ratio.den,
					pvideostream->avg_frame_rate.num,
					pvideostream->avg_frame_rate.den,
					pvideostream->r_frame_rate.num,
					pvideostream->r_frame_rate.den,
					pvideostream->display_aspect_ratio.num,
					pvideostream->display_aspect_ratio.den);
	g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

	printf("%s\n",buff_temp);

    pInputCodecContext = pInputFormatContext->streams[videoIndex]->codec;

	width = pInputCodecContext->width;
    height = pInputCodecContext->height;

	framerate = pInputCodecContext->framerate.num;

	memset(buff_temp,0,sizeof(buff_temp));
	sprintf(buff_temp,"codec context,video index:%d,width:%d,height:%d,time_base.num:%d,time_base.den:%d,framerate.num:%d,framerate.den:%d,pix_fmt:%d,frame_rate:%d\n",
					videoIndex,
					pInputCodecContext->width,
					pInputCodecContext->height,
					pInputCodecContext->time_base.num,
					pInputCodecContext->time_base.den,
					pInputCodecContext->framerate.num,
					pInputCodecContext->framerate.den,
					pInputCodecContext->pix_fmt,
					framerate);
	g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

	printf("%s\n",buff_temp);


    avformat_close_input(&pInputFormatContext);
    avformat_free_context(pInputFormatContext);

	return ret;
}
int MarkJob::get_metadata(const AVFormatContext *pformat_ctx)
{
	int ret = MARK_JOB_SUCCESS;
    //MetaData------------------------------------------------------------  
    //从AVDictionary获得  
    //需要用到AVDictionaryEntry对象  
    //CString author,copyright,description;  
	char meta_key[255] = {'\0'};
	char meta_value[255] = {'\0'};
	char buff_temp[1024] = {'\0'};

    AVDictionaryEntry *pmeta = NULL;

#if 0
    //不用一个一个找出来  
    pmeta=av_dict_get(pformat_ctx->metadata,"author",pmeta,0); 
    printf("作者：%s",pmeta->value); 
    pmeta=av_dict_get(pformat_ctx->metadata,"copyright",pmeta,0); 
    printf("版权：%s",pmeta->value); 
    pmeta=av_dict_get(pformat_ctx->metadata,"description",pmeta,0); 
	printf("描述：%s",pmeta->value);
#endif

    //使用循环读出  
    //(需要读取的数据，字段名称，前一条字段（循环时使用），参数)  
	
	memset(buff_temp, 0, sizeof(buff_temp));
    while(pmeta=av_dict_get(pformat_ctx->metadata,"",pmeta,AV_DICT_IGNORE_SUFFIX))
	{  
		sprintf(buff_temp, "%s key:%s,value:%s\n",buff_temp,pmeta->key,pmeta->value);
    }  
	g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

	return ret;
}

int MarkJob::log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           tag,
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}
int MarkJob::init_ffmpeg_decoder(FFMPEG_DECODER *pffmpeg_decoder,MARK_JOB_ITEM *pmarkjob)
{
	int ret = MARK_JOB_SUCCESS;
	int ret_ffmpeg = 0;
	bool bret = false;
	char buff_temp[512] = {'\0'};

	memset(&pffmpeg_decoder->markjob,0,sizeof(MARK_JOB_ITEM));
	memcpy(&pffmpeg_decoder->markjob,pmarkjob,sizeof(MARK_JOB_ITEM));

	pffmpeg_decoder->video_width_dest = pffmpeg_decoder->markjob.decode_width;
	pffmpeg_decoder->video_height_dest = pffmpeg_decoder->markjob.decode_height;

    pffmpeg_decoder->pInputFormatContext = avformat_alloc_context();
    if(NULL == pffmpeg_decoder->pInputFormatContext)
    {
		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob alloc memory for avformat context failed\n");
		g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

 		printf("%s\n",buff_temp);
		return -1;
	}

    // 打开视频文件
	ret_ffmpeg = avformat_open_input(&pffmpeg_decoder->pInputFormatContext, pffmpeg_decoder->markjob.videopath, NULL, NULL);
    if(ret_ffmpeg)
    {
		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob open file failed,file:%s,ret_ffmpeg:%d\n",pffmpeg_decoder->markjob.videopath,ret_ffmpeg);
		g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

 		printf("%s\n",buff_temp);
		return -1;
	}

#if 1
	/// av文件信息
	memset(buff_temp,0,sizeof(buff_temp));
	sprintf(buff_temp,"avformat context,duration:%d,nb_streams:%d,bitrate:%d,filename:%s\n",
		pffmpeg_decoder->pInputFormatContext->duration/1000000,
		pffmpeg_decoder->pInputFormatContext->nb_streams,
		pffmpeg_decoder->pInputFormatContext->bit_rate/1000,
		pffmpeg_decoder->pInputFormatContext->filename
		);
	g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);


	get_metadata(pffmpeg_decoder->pInputFormatContext);

	printf("%s\n",buff_temp);
#endif


    // 取出文件流信息 填充AVFormatContext中的音视频流结构
	ret_ffmpeg = avformat_find_stream_info(pffmpeg_decoder->pInputFormatContext,NULL);

    if(ret_ffmpeg<0)
	{
		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob can't find suitable codec parameters,file:%s,ret_ffmpeg:%d\n",pffmpeg_decoder->markjob.videopath,ret_ffmpeg);
		g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

 		printf("%s\n",buff_temp);
		return -1;
	}

    //print media info
    av_dump_format(pffmpeg_decoder->pInputFormatContext, 0, pffmpeg_decoder->markjob.videopath, false);

    //仅仅处理视频流
    //只简单处理我们发现的第一个视频流
    //  寻找第一个视频流
    int videoIndex = -1;
    for(int i=0; i<pffmpeg_decoder->pInputFormatContext->nb_streams; i++)
    {
        if(pffmpeg_decoder->pInputFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoIndex = i;
            break;
        }
    }


    if(-1 == videoIndex)
    {
		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob can't find video stream,file:%s\n",pffmpeg_decoder->markjob.videopath);
		g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

 		printf("%s\n",buff_temp);
		return -1;
    }

    // 得到视频流编码上下文的指针
	pffmpeg_decoder->pvideostream =pffmpeg_decoder->pInputFormatContext->streams[videoIndex];

	pffmpeg_decoder->duration = pffmpeg_decoder->pvideostream->nb_frames;
	pffmpeg_decoder->duration_in_seconds = pffmpeg_decoder->pvideostream->duration / pffmpeg_decoder->pvideostream->time_base.den;

	memset(buff_temp,0,sizeof(buff_temp));
	sprintf(buff_temp,"avstream,stream index:%d,time_base.num:%d,time_base.den:%d,start_time:%d,duration:%d,nb_frames:%d\n"
					"sample_aspect_ratio.num:%d,sample_aspect_ratio.den:%d,avg_frame_rate.num:%d,avg_frame_rate.den:%d\n"
					"r_frame_rate.num:%d,r_frame_rate.den:%d,display_aspect_ratio.num:%d,display_aspect_ratio.den:%d\n",
					pffmpeg_decoder->pvideostream->index,
					pffmpeg_decoder->pvideostream->time_base.num,
					pffmpeg_decoder->pvideostream->time_base.den,
					pffmpeg_decoder->pvideostream->start_time,
					pffmpeg_decoder->pvideostream->duration,
					pffmpeg_decoder->pvideostream->nb_frames,
					pffmpeg_decoder->pvideostream->sample_aspect_ratio.num,
					pffmpeg_decoder->pvideostream->sample_aspect_ratio.den,
					pffmpeg_decoder->pvideostream->avg_frame_rate.num,
					pffmpeg_decoder->pvideostream->avg_frame_rate.den,
					pffmpeg_decoder->pvideostream->r_frame_rate.num,
					pffmpeg_decoder->pvideostream->r_frame_rate.den,
					pffmpeg_decoder->pvideostream->display_aspect_ratio.num,
					pffmpeg_decoder->pvideostream->display_aspect_ratio.den);
	g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

	printf("%s\n",buff_temp);

    pffmpeg_decoder->pInputCodecContext = pffmpeg_decoder->pInputFormatContext->streams[videoIndex]->codec;

	pffmpeg_decoder->video_width_src = pffmpeg_decoder->pInputCodecContext->width;
    pffmpeg_decoder->video_height_src = pffmpeg_decoder->pInputCodecContext->height;

	pffmpeg_decoder->m_pix_fmt_src = pffmpeg_decoder->pInputCodecContext->pix_fmt;
	pffmpeg_decoder->m_pix_fmt_dest = pffmpeg_decoder->pInputCodecContext->pix_fmt;
	pffmpeg_decoder->flag_decode = SWS_BICUBIC;

	//pffmpeg_decoder->framerate = pffmpeg_decoder->pInputCodecContext->time_base.den / pffmpeg_decoder->pInputCodecContext->time_base.num;
	pffmpeg_decoder->framerate = pffmpeg_decoder->pInputCodecContext->framerate.num;

	memset(buff_temp,0,sizeof(buff_temp));
	sprintf(buff_temp,"codec context,video index:%d,width:%d,height:%d,time_base.num:%d,time_base.den:%d,framerate.num:%d,framerate.den:%d,pix_fmt:%d,frame_rate:%d\n",
					videoIndex,
					pffmpeg_decoder->pInputCodecContext->width,
					pffmpeg_decoder->pInputCodecContext->height,
					pffmpeg_decoder->pInputCodecContext->time_base.num,
					pffmpeg_decoder->pInputCodecContext->time_base.den,
					pffmpeg_decoder->pInputCodecContext->framerate.num,
					pffmpeg_decoder->pInputCodecContext->framerate.den,
					pffmpeg_decoder->pInputCodecContext->pix_fmt,
					pffmpeg_decoder->framerate);
	g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

	printf("%s\n",buff_temp);

    //  寻找视频流的解码器
    pffmpeg_decoder->pInputCodec = avcodec_find_decoder(pffmpeg_decoder->pInputCodecContext->codec_id);

    if(NULL == pffmpeg_decoder->pInputCodec)
    {
  		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob can't decode,file:%s\n",pffmpeg_decoder->markjob.videopath);
		g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
       
		return -1;
    }

 

    // 通知解码器我们能够处理截断的bit流，bit流帧边界可以在包中
    //视频流中的数据是被分割放入包中的。因为每个视频帧的数据的大小是可变的，
    //那么两帧之间的边界就不一定刚好是包的边界。这里，我们告知解码器我们可以处理bit流。
//    if(pInputCodec->capabilities & CODEC_CAP_TRUNCATED)
//    {
//        pInputCodecContext->flags|=CODEC_FLAG_TRUNCATED;
//        pInputCodecContext->flags|=CODEC_CAP_DELAY;		
//    }


    //打开解码器
	ret_ffmpeg = avcodec_open2(pffmpeg_decoder->pInputCodecContext, pffmpeg_decoder->pInputCodec,NULL);

    if(ret_ffmpeg)
	{

  		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob open decode error,file:%s,ret_ffmpeg:%d\n",pffmpeg_decoder->markjob.videopath,ret_ffmpeg);
		g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
 
        return -1;
    }


    pffmpeg_decoder->ptrOutFrame=av_frame_alloc();
	if(NULL == pffmpeg_decoder->ptrOutFrame)
	{
		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob alloc memory for avframe failed\n");
		g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);

        return -1;
	}


    //  加入这句话来纠正某些编码器产生的帧速错误
    /*if(pInputCodecContext->frame_rate>1000 && pInputCodecContext->frame_rate_base==1)
         pInputCodecContext->frame_rate_base=1000;*/

#if 1
	int time_base = 0;
	time_base = pffmpeg_decoder->markjob.decode_start_pos * pffmpeg_decoder->pvideostream->time_base.den;
    ret_ffmpeg = av_seek_frame(pffmpeg_decoder->pInputFormatContext,0,time_base,AVSEEK_FLAG_FRAME);//AV_TIME_BASE
    if(ret_ffmpeg<0)
	{
  		memset(buff_temp,0,sizeof(buff_temp));
		sprintf(buff_temp,"markjob av_seek_frame error,file:%s,ret_ffmpeg:%d\n",pffmpeg_decoder->markjob.videopath,ret_ffmpeg);
		g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
 
        return -1;
    }

  	memset(buff_temp,0,sizeof(buff_temp));
	sprintf(buff_temp,"av_seek_frame time_base:%d,pffmpeg_decoder->markjob.decode_start_pos:%d,ret_ffmpeg:%d",
				pffmpeg_decoder->markjob.time_base,
				pffmpeg_decoder->markjob.decode_start_pos,
				ret_ffmpeg);
	g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
	printf("%s\n",buff_temp);
#endif


	if(pffmpeg_decoder->markjob.bisadjust_aspect)
	{
		avpicture_alloc(&pffmpeg_decoder->avpicture_convert, 
						pffmpeg_decoder->m_pix_fmt_dest,
						pffmpeg_decoder->video_width_dest,
						pffmpeg_decoder->video_height_dest);

		pffmpeg_decoder->pimg_convert_ctx = sws_getContext(pffmpeg_decoder->video_width_src,
										pffmpeg_decoder->video_height_src,
										pffmpeg_decoder->m_pix_fmt_src,
										pffmpeg_decoder->video_width_dest,
										pffmpeg_decoder->video_height_dest,
										pffmpeg_decoder->m_pix_fmt_dest,
										pffmpeg_decoder->flag_decode,
										NULL,
										NULL,
										NULL);

	}

	return ret;
}
int MarkJob::uninit_ffmpeg_decoder(FFMPEG_DECODER *pffmpeg_decoder)
{
	int ret = MARK_JOB_SUCCESS;

	if(pffmpeg_decoder->markjob.bisadjust_aspect)
	{
		avpicture_free(&pffmpeg_decoder->avpicture_convert);
		sws_freeContext(pffmpeg_decoder->pimg_convert_ctx);
	}

    avcodec_close(pffmpeg_decoder->pInputCodecContext);
    avformat_close_input(&pffmpeg_decoder->pInputFormatContext);
    avformat_free_context(pffmpeg_decoder->pInputFormatContext);
    av_frame_free(&pffmpeg_decoder->ptrOutFrame);

	return ret;
}
int MarkJob::convertfileformat(const char *pfilepath_src,const char *pfilepath_desc)
{
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    int ret, i;
	char in_filename[255] = {'\0'};
	char out_filename[255] = {'\0'};

    if ( NULL == pfilepath_src ||
		NULL == pfilepath_desc) 
	{
        printf("usage: test input output\n"
               "API example program to remux a media file with libavformat and libavcodec.\n"
               "The output format is guessed according to the file extension.\n");
        return 1;
    }

    memset(in_filename,0,sizeof(in_filename));
	memset(out_filename,0,sizeof(out_filename));

	strcpy(in_filename,pfilepath_src);
	strcpy(out_filename,pfilepath_desc);

    av_register_all();

    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        fprintf(stderr, "Could not open input file '%s'", in_filename);
        goto end;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        goto end;
    }

    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) {
        fprintf(stderr, "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    ofmt = ofmt_ctx->oformat;

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
        if (!out_stream) {
            fprintf(stderr, "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy context from input to output stream codec context\n");
            goto end;
        }
        out_stream->codec->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    av_dump_format(ofmt_ctx, 0, out_filename, 1);

    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "Could not open output file '%s'", out_filename);
            goto end;
        }
    }

    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        goto end;
    }

    while (1) {
        AVStream *in_stream, *out_stream;

        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;

        in_stream  = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];

        log_packet(ifmt_ctx, &pkt, "in");

        /* copy packet */
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        log_packet(ofmt_ctx, &pkt, "out");

        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
            fprintf(stderr, "Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }

    av_write_trailer(ofmt_ctx);
end:

    avformat_close_input(&ifmt_ctx);

    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

    if (ret < 0 && ret != AVERROR_EOF) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }

    return 0;
}
int MarkJob::fill_frameinfo_by_avframe(AVFrame *pavframe,int frameindex,AV_FRAME_INFO *pframeinfo)
{
	int ret = MARK_JOB_SUCCESS;
	int i = 0;
	int j = 0;
	int k = 0;
	char buff_temp[255] = {'\0'};

	//if (pavframe->format == AV_PIX_FMT_YUV420P) //如果是yuv420p的
	//{
#if 0
		memset(buff_temp,0,sizeof(buff_temp));
		//sprintf(buff_temp,"save yuv420,frame type is AV_PIX_FMT_YUV420P\n");
		sprintf(buff_temp,"save yuv420,frame type is %d\n",pavframe->format);
		//g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
		printf("%s\n",buff_temp);
#endif


		for(i = 0; i <pavframe->height; i++)
		{
			memcpy(pframeinfo->data+pavframe->width*i,
					pavframe->data[0]+pavframe->linesize[0]*i,
					pavframe->width);
		}

		for(j = 0; j < pavframe->height/2; j++)
		{                 
			memcpy(pframeinfo->data+pavframe->width*i+pavframe->width/2*j,
				pavframe->data[1]+pavframe->linesize[1]*j,
					pavframe->width/2);
		}
		for(k =0; k < pavframe->height/2; k++)
		{
			memcpy(pframeinfo->data+pavframe->width*i+pavframe->width/2*j+pavframe->width/2*k,
						pavframe->data[2]+pavframe->linesize[2]*k,
						pavframe->width/2);
		}
				
		pframeinfo->type = 0;
		pframeinfo->width = pavframe->width;
		pframeinfo->height = pavframe->height;
		pframeinfo->length = pavframe->width * pavframe->height * 3 /2;
		pframeinfo->framenum = m_markjob.time_base + m_markjob.decode_start_pos + frameindex;
		
	//}

						
	return ret;
}
int MarkJob::fill_frameinfo_by_avpicture(AVPicture *pavpicture,
							int width,
							int height,
							AVPixelFormat pix_fmt,
							int frameindex,
							AV_FRAME_INFO *pframeinfo)
{
	int ret = MARK_JOB_SUCCESS;
	int i = 0;
	int j = 0;
	int k = 0;
	char buff_temp[255] = {'\0'};

	//if (pix_fmt == AV_PIX_FMT_YUV420P) //如果是yuv420p的
	//{
#if 0
		memset(buff_temp,0,sizeof(buff_temp));
		//sprintf(buff_temp,"save yuv420,frame type is AV_PIX_FMT_YUV420P\n");
		sprintf(buff_temp,"save yuv420,frame type is %d\n",pix_fmt);
		//g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
		printf("%s\n",buff_temp);
#endif


		for(i = 0; i <height; i++)
		{
			memcpy(pframeinfo->data+width*i,
					pavpicture->data[0]+pavpicture->linesize[0]*i,
					width);
		}

		for(j = 0; j < height/2; j++)
		{                 
			memcpy(pframeinfo->data+width*i+width/2*j,
				pavpicture->data[1]+pavpicture->linesize[1]*j,
					width/2);
		}
		for(k =0; k < height/2; k++)
		{
			memcpy(pframeinfo->data+width*i+width/2*j+width/2*k,
						pavpicture->data[2]+pavpicture->linesize[2]*k,
						width/2);
		}
				
		pframeinfo->type = 0;
		pframeinfo->width = width;
		pframeinfo->height = height;
		pframeinfo->length = width * height * 3 /2;
		pframeinfo->framenum = m_markjob.time_base + m_markjob.decode_start_pos + frameindex;
		
	//}

						
	return ret;
}
int  MarkJob::save_bmp(AVPicture *pavpicture,int width,int height,int framenum)
{
	int ret = MARK_JOB_SUCCESS;
	int ret_ffmpeg = 0;
	char buff_temp[255] = {'\0'};

    int lineBytes = pavpicture->linesize[0];

    BITMAPFILEHEADER btfileHeader;
    btfileHeader.bfType = 0x4d42;//mb
    btfileHeader.bfSize = lineBytes*height;
    btfileHeader.bfReserved1 = 0;
    btfileHeader.bfReserved2 = 0;
    btfileHeader.bfOffBits = sizeof(BITMAPFILEHEADER);

    BITMAPINFOHEADER bitmapinfoheader;
    bitmapinfoheader.biSize = 40;
    bitmapinfoheader.biWidth = width;
    bitmapinfoheader.biHeight = height;
    bitmapinfoheader.biPlanes = 1;
    bitmapinfoheader.biBitCount = 24;
    bitmapinfoheader.biCompression = 0;
    bitmapinfoheader.biSizeImage = lineBytes*height;
    bitmapinfoheader.biXPelsPerMeter = 0;
    bitmapinfoheader.biYPelsPerMeter = 0;
    bitmapinfoheader.biClrUsed = 0;
    bitmapinfoheader.biClrImportant = 0;


    char filepath[1024]={0};
    sprintf(filepath,"/smsstore/restore/%06d.bmp" , framenum);
    FILE *pDestFile = fopen(filepath, "wb");
    fwrite(&btfileHeader, sizeof(BITMAPFILEHEADER), 1, pDestFile);
    fwrite(&bitmapinfoheader, sizeof(BITMAPINFODEADER), 1, pDestFile);
    int k=0;
    for(k=height-1; k>=0; k--)
    {
         fwrite(pavpicture->data[0]+k*lineBytes, lineBytes, 1, pDestFile);
	}
    fclose(pDestFile);
	pDestFile = NULL;


	return ret;
}
int  MarkJob::save_yuv(AVPicture *pavpicture,AVPixelFormat pix_fmt,int width,int height,int framenum)
{
	int ret = MARK_JOB_SUCCESS;
	int i = 0;
	int j = 0;
	int k = 0;	int ret_ffmpeg = 0;
	char buff_temp[255] = {'\0'};

    int lineBytes = pavpicture->linesize[0];

    char filepath[1024]={0};
    sprintf(filepath,"/smsstore/restore/%06d.yuv" , framenum);



//////////////


	//if (pix_fmt == AV_PIX_FMT_YUV420P) //如果是yuv420p的
	//{
#if 0
		memset(buff_temp,0,sizeof(buff_temp));
		//sprintf(buff_temp,"save yuv420,frame type is AV_PIX_FMT_YUV420P\n");
		sprintf(buff_temp,"save yuv420,frame type is %d\n",pix_fmt);
		//g_markjob_logwrite.PrintLog(MyLogger::INFO,"%s",buff_temp);
		printf("%s\n",buff_temp);
#endif

    FILE *pDestFile = fopen(filepath, "wb");

	for(i = 0; i <height; i++)
	{
		fwrite(pavpicture->data[0]+pavpicture->linesize[0]*i, 1,width,pDestFile);

	}

	for(j = 0; j < height/2; j++)
	{                 
		fwrite(pavpicture->data[1]+pavpicture->linesize[1]*j,1,width/2,pDestFile);
	}


	for(k =0; k < height/2; k++)
	{
		fwrite(pavpicture->data[2]+pavpicture->linesize[2]*k, 1, width/2,pDestFile);
	}
				

    fclose(pDestFile);
	pDestFile = NULL;

	return ret;
}
int MarkJob::GetLastError(LAST_ERROR_REPORT *preportstatus)
{
	int ret = MARK_JOB_SUCCESS;

	if(NULL == preportstatus)
	{
		return MARK_JOB_GET_LASTERROR_PARAMETER_IS_NULL;
	}

	memset(preportstatus,0,sizeof(LAST_ERROR_REPORT));

	memcpy(preportstatus,&m_lasterror,sizeof(LAST_ERROR_REPORT));

	return ret;
}
