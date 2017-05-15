#include "FrameBufferLoop.h"

///error num
#define FRAME_BUFFER_SUCCESS							0
#define FRAME_BUFFER_INIT_ERROR_NOT_IN_IDLE				1
#define FRAME_BUFFER_DESTROY_ERROR_NOT_IN_IDLE			2
#define FRAME_BUFFER_SETCONTROL_ERROR_NOT_IN_IDLE		3
#define FRAME_BUFFER_WRITE_ERROR_NOT_IN_WRITE			4
#define FRAME_BUFFER_READ_ERROR_NOT_IN_READ				5
#define FRAME_BUFFER_WRITE_ERROR_NOT_INIT				6
#define FRAME_BUFFER_WRITE_ERROR_DATA_OVER_LENGTH		7
#define FRAME_BUFFER_READ_ERROR_NOFRAME_IN_BUFFER		8
#define FRAME_BUFFER_INIT_FRAMECOUNT_INVALID			9
#define FRAME_BUFFER_READ_ERROR_INDEX_INVALID			10
#define FRAME_BUFFER_GET_LASTERROR_PARAMETER_IS_NULL	11


FrameBufferLoop::FrameBufferLoop()
{
	m_control_type = BUFFER_CONTROL_IDLE;

	m_buffer_size = 0;
	m_counter_write = 0;
	m_counter_frame = 0;
	memset(&m_lasterror,0,sizeof(LAST_ERROR_REPORT));

	pthread_mutex_init(&m_mutex_control,NULL);
}

FrameBufferLoop::~FrameBufferLoop()
{
	DestroyFrameBuffer();
}
int FrameBufferLoop::InitFrameBuffer(int buffercount)
{
	int ret = FRAME_BUFFER_SUCCESS;
	int framecount = 0;

	if(buffercount < 0 ||
		buffercount > BUFFER_MAX_SIZE)
	{
		return FRAME_BUFFER_INIT_FRAMECOUNT_INVALID;
	}

	pthread_mutex_lock(&m_mutex_control);

	framecount = buffercount;

	if( m_control_type != BUFFER_CONTROL_IDLE )
	{
		pthread_mutex_unlock(&m_mutex_control);
		return FRAME_BUFFER_INIT_ERROR_NOT_IN_IDLE;
	}

	if(framecount>BUFFER_MAX_SIZE)
	{
		framecount = BUFFER_MAX_SIZE;
	}	
	
	if(framecount == m_framelist.size())
	{
		ZeroSpaceItems(&m_framelist);
		pthread_mutex_unlock(&m_mutex_control);
		return ret;
	}
	else if(framecount > m_framelist.size())
	{

	}
	else
	{
	}
	
	DeleteSpaceItems(&m_framelist);

	for(int i = 0; i < framecount; i++)
	{
		AV_FRAME_INFO *pframeitem = NULL;
	
		NewSpaceItem(&pframeitem);

		m_framelist.push_back(pframeitem);

	}

	m_buffer_size = m_framelist.size();

	pthread_mutex_unlock(&m_mutex_control);

	return ret;
}
int FrameBufferLoop::DestroyFrameBuffer()
{
	int ret = FRAME_BUFFER_SUCCESS;
	pthread_mutex_lock(&m_mutex_control);

	if( m_control_type != BUFFER_CONTROL_IDLE )
	{
		pthread_mutex_unlock(&m_mutex_control);
		return FRAME_BUFFER_DESTROY_ERROR_NOT_IN_IDLE;
	}

	DeleteSpaceItems(&m_framelist);

	pthread_mutex_unlock(&m_mutex_control);
	return ret;
}
int FrameBufferLoop::ResetFrameBuffer()
{
	int ret = FRAME_BUFFER_SUCCESS;

	pthread_mutex_lock(&m_mutex_control);

	if( m_control_type != BUFFER_CONTROL_IDLE )
	{
		pthread_mutex_unlock(&m_mutex_control);
		return FRAME_BUFFER_DESTROY_ERROR_NOT_IN_IDLE;
	}

	m_counter_write = 0;
	m_counter_frame = 0;

	ZeroSpaceItems(&m_framelist);

	pthread_mutex_unlock(&m_mutex_control);
	return ret;
}
bool FrameBufferLoop::IsIdle()
{
	bool bret = false;
	pthread_mutex_lock(&m_mutex_control);

	if( BUFFER_CONTROL_IDLE == m_control_type)
	{
		pthread_mutex_unlock(&m_mutex_control);
		bret =  true;
	}

	pthread_mutex_unlock(&m_mutex_control);

	return bret;
}
int FrameBufferLoop::SetContol(BUFFER_CONTROL_TYPE controltype, bool isalwaysok)
{
	int ret = FRAME_BUFFER_SUCCESS;
	pthread_mutex_lock(&m_mutex_control);

	if(isalwaysok)
	{
		m_control_type = controltype;
		pthread_mutex_unlock(&m_mutex_control);
		return ret;
	}

	if(controltype == BUFFER_CONTROL_IDLE)
	{
		m_control_type = controltype;
	}
	else
	{
		if(m_control_type != BUFFER_CONTROL_IDLE)
		{
			pthread_mutex_unlock(&m_mutex_control);
			return  FRAME_BUFFER_SETCONTROL_ERROR_NOT_IN_IDLE;
		}
		else
		{
			m_control_type = controltype;
		}
	}

	pthread_mutex_unlock(&m_mutex_control);

	return ret;
}
int FrameBufferLoop::WriteFrameData(AV_FRAME_INFO *pframeinfo)
{
	int ret = FRAME_BUFFER_SUCCESS;
	int frameindex = 0;

	if(m_control_type != BUFFER_CONTROL_WRITE)
	{
		return  FRAME_BUFFER_WRITE_ERROR_NOT_IN_WRITE;
	}

	if(m_buffer_size == 0)
	{
		return FRAME_BUFFER_WRITE_ERROR_NOT_INIT; 
	}

	if(pframeinfo->length>=MAX_VIDEO_FRAME_LEN)
	{
		return FRAME_BUFFER_WRITE_ERROR_DATA_OVER_LENGTH;
	}

	pthread_mutex_lock(&m_mutex_control);

	frameindex = m_counter_write%m_buffer_size;

	////写数据
	AV_FRAME_INFO *ptemp = (AV_FRAME_INFO *)(m_framelist[frameindex]);

	///长度
	ptemp->length = pframeinfo->length;

	///类型
	ptemp->type = pframeinfo->type;

	///帧编号
	ptemp->framenum = pframeinfo->framenum;

	///通道号
	//ptemp->channelnum = channelnum;

	///宽
	ptemp->width = pframeinfo->width;

	///高
	ptemp->height = pframeinfo->height;

	///日期
	//memcpy(ptemp->record_time,precord_time,strlen(precord_time));

	///数据
	memcpy(ptemp->data,pframeinfo->data,pframeinfo->length);

	////
	m_counter_write++;
	if(m_counter_write>m_buffer_size)
	{
		m_counter_frame = m_buffer_size;
	}
	else
	{
		m_counter_frame++;
	}

#if 0
	printf("write buffer index:%d,frame counter:%d,write counter:%d\n",frameindex,m_counter_frame,m_counter_write);
#endif

	pthread_mutex_unlock(&m_mutex_control);

	return ret;
}

int FrameBufferLoop::ReadFrameData(AV_FRAME_INFO *pframeitem)
{
	int ret = FRAME_BUFFER_SUCCESS;
	int frameindex = 0;

	if(m_control_type != BUFFER_CONTROL_READ)
	{
		return  FRAME_BUFFER_READ_ERROR_NOT_IN_READ;
	}

	pthread_mutex_lock(&m_mutex_control);

	if(m_counter_frame<=0)
	{
		pthread_mutex_unlock(&m_mutex_control);
		return FRAME_BUFFER_READ_ERROR_NOFRAME_IN_BUFFER;
	}


	if(m_counter_write>m_buffer_size)
	{
		frameindex = (((m_counter_write%m_buffer_size) + (m_buffer_size - m_counter_frame)))%m_buffer_size;
	}
	else
	{
		frameindex = m_counter_write - m_counter_frame;
	}

	////读数据
	AV_FRAME_INFO *ptemp = (AV_FRAME_INFO *)(m_framelist[frameindex]);

	pframeitem->length = ptemp->length;
	pframeitem->type = ptemp->type;
	//pframeitem->channelnum = ptemp->channelnum;
	pframeitem->width = ptemp->width;
	pframeitem->height = ptemp->height;

	//memcpy(pframeitem->record_time,ptemp->record_time,20);
	memcpy(pframeitem->data,ptemp->data,pframeitem->length);

	////
	
	m_counter_frame--;

	if(m_counter_frame == 0)
	{
		/// 可以在此复位写入的总数,避免重读数据
		m_counter_write = 0;
	}

#if 0
	printf("read buffer index:%d,frame counter:%d,write counter:%d\n",frameindex,m_counter_frame,m_counter_write);
#endif

	pthread_mutex_unlock(&m_mutex_control);

	return ret;
}
int FrameBufferLoop::ReadFrameData(int index,AV_FRAME_INFO **pframeitem)
{
	int ret = FRAME_BUFFER_SUCCESS;
	int frameindex = 0;

	if(m_control_type != BUFFER_CONTROL_READ)
	{
		return  FRAME_BUFFER_READ_ERROR_NOT_IN_READ;
	}

	pthread_mutex_lock(&m_mutex_control);

	if(m_counter_frame<=0)
	{
		pthread_mutex_unlock(&m_mutex_control);
		return FRAME_BUFFER_READ_ERROR_NOFRAME_IN_BUFFER;
	}

	if( (index<0) ||
		(index>=m_counter_frame))
	{
		pthread_mutex_unlock(&m_mutex_control);
		return FRAME_BUFFER_READ_ERROR_INDEX_INVALID;
	}

	if(m_counter_write>m_buffer_size)
	{
		frameindex = (((m_counter_write%m_buffer_size) + index))%m_buffer_size;
	}
	else
	{
		frameindex = index;
	}

	////读数据
	AV_FRAME_INFO *ptemp = (AV_FRAME_INFO *)(m_framelist[frameindex]);

	*pframeitem = ptemp;

#if 0
	printf("read buffer index:%d,frame counter:%d,write counter:%d,startpos:%d,pos:%d\n",
					index,
					m_counter_frame,
					m_counter_write,
					m_counter_write%m_buffer_size,
					frameindex);
#endif

	pthread_mutex_unlock(&m_mutex_control);

	return ret;
}
int FrameBufferLoop::GetFrameCount()
{
	int counter = 0;

	pthread_mutex_lock(&m_mutex_control);

	counter = m_counter_frame;

	pthread_mutex_unlock(&m_mutex_control);

	return counter;

}
void FrameBufferLoop::ZeroSpaceItem(AV_FRAME_INFO *pframeitem)
{
	if(NULL == pframeitem)
	{
		return;
	}

	if(NULL != pframeitem->data)
	{
		memset(pframeitem->data,0,MAX_VIDEO_FRAME_LEN);
	}

	pframeitem->type = 0;
	pframeitem->length = 0;
	pframeitem->framenum = 0;
	pframeitem->width = 0;
	pframeitem->height = 0;

}

void FrameBufferLoop::NewSpaceItem(AV_FRAME_INFO **pframeitem)
{
	if(NULL != *pframeitem)
	{
		return;
	}

	*pframeitem = (AV_FRAME_INFO *)new AV_FRAME_INFO;

	(*pframeitem)->data = new unsigned char[MAX_VIDEO_FRAME_LEN];

	ZeroSpaceItem(*pframeitem);
}
void FrameBufferLoop::DeleteSpaceItem(AV_FRAME_INFO **pframeitem)
{
	if(NULL == (*pframeitem))
	{
		return;
	}

	ZeroSpaceItem(*pframeitem);

	if(NULL != (*pframeitem)->data)
	{
		delete [] (*pframeitem)->data;
		(*pframeitem)->data = NULL;
	}

	if(NULL != (*pframeitem))
	{
		delete (*pframeitem);
		(*pframeitem) = NULL;
	}


}
void FrameBufferLoop::DeleteSpaceItems(AV_FRAME_INFOS *pframeitems)
{
	if(NULL == pframeitems)
	{
		return;
	}
	
	if(pframeitems->size() > 0)
	{
		for(int i = 0; i<pframeitems->size(); i++)
		{
			AV_FRAME_INFO *ptemp = (AV_FRAME_INFO *)((*pframeitems)[i]);
			DeleteSpaceItem(&ptemp);
		}

		pframeitems->clear();
	}

}

void FrameBufferLoop::ZeroSpaceItems(AV_FRAME_INFOS *pframeitems)
{
	if(NULL == pframeitems)
	{
		return;
	}
	
	if(pframeitems->size() > 0)
	{
		for(int i = 0; i<pframeitems->size(); i++)
		{
			AV_FRAME_INFO *ptemp = (AV_FRAME_INFO *)((*pframeitems)[i]);
			ZeroSpaceItem(ptemp);
		}

	}

}
int FrameBufferLoop::GetLastError(LAST_ERROR_REPORT *preportstatus)
{
	int ret = FRAME_BUFFER_SUCCESS;

	if(NULL == preportstatus)
	{
		return FRAME_BUFFER_GET_LASTERROR_PARAMETER_IS_NULL;
	}

	memset(preportstatus,0,sizeof(LAST_ERROR_REPORT));

	memcpy(preportstatus,&m_lasterror,sizeof(LAST_ERROR_REPORT));

	return ret;
}