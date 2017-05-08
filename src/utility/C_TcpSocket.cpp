//filename:C_TcpSocket.cpp

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#ifdef WIN32
  #include   <windows.h>   
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h> 
	#include <errno.h>
#endif
//#include "log/C_LogManage.h"
#include "C_TcpSocket.h"
#include "C_ErrorDef.h"
const int SOCKET_SUCCEES                 =0;        //操作成功

using namespace std;

	//iTmpBufferLen：指定对象内临时缓存的长度，同时对m_TmpBuffer 分配空间。
C_TcpSocket::C_TcpSocket(int iTmpBufferLen)
{
  //套接字描述符
	m_iSocketId = -1; 
  //通信的超时时间
  m_iTimeout = 3;
  //临时缓存长度。
  m_iTmpBufferLen = iTmpBufferLen;
  //临时缓存数据长度。
  m_iTmpDataBufferLen = 0;//add by xiaozhengxiu 20130703
  //临时缓存地址。
  m_TmpBuffer = new char[iTmpBufferLen];
  memset(m_TmpBuffer,0,iTmpBufferLen);
  //交换数据区大小,等同m_iTmpDataBufferLen
  m_iTmpExBufferLen = iTmpBufferLen;
  //交换数据区
  m_TmpExBuffer = new char[m_iTmpExBufferLen];
  memset(m_TmpExBuffer,0,m_iTmpExBufferLen);

  m_iLogModuleNum = 22;
  m_iLogSubModuleNum = 0;
  memset(m_strIp,0,16);
  	  	
//windows
#ifdef WIN32
	WSADATA              wsd; 
	WSAStartup(MAKEWORD(2, 2), &wsd); 		
#endif	
}

C_TcpSocket::~C_TcpSocket()
{
//	if(SOCKET_NULL_FD != m_iSocketId)
	if(-1 != m_iSocketId)
	{
#ifdef WIN32
		closesocket(m_iSocketId); 
		WSACleanup();
#else
		close(m_iSocketId);
#endif
	}
	
	if (m_TmpBuffer)
	{
		delete []m_TmpBuffer;
		m_TmpBuffer = NULL;
	}
	if (m_TmpExBuffer)
	{
		delete []m_TmpExBuffer;
		m_TmpExBuffer = NULL;
	}
}

	//说明：建立tcp连接。
	//返回值：连接成功返回 0，连接失败 -1；
	//strIp：[in] ，usPort [in], iWait [in];
int C_TcpSocket::TcpConnect(const char *strIp, unsigned short usPort)
{
	struct sockaddr_in   servaddr;
	int                  flag = 0;
	int iResult = -1;

	if(NULL == strIp || 0 == usPort)
	{
		CreateLog(SOCKET_PARAR_ERR, iResult);
		return iResult;
	}

	if(-1 != m_iSocketId)
	{
		TcpDisConnect();
	}

//	m_iSocketId = socket(AF_INET, SOCK_STREAM, 0);//del by xiaozhengxiu 20130703
	int iTmpSocketId = socket(AF_INET, SOCK_STREAM, 0);//add by xiaozhengxiu 20130703
	
	if(flag < 0)    //初始化socket失败
	{
		perror("[socket error]:");
		CreateLog(CLIENT_SOCK_INI_ERR, iResult);
		return iResult;
		//return CLIENT_SOCK_INI_ERR;
	}

	memset (&servaddr, 0, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET;  
	servaddr.sin_port = htons(usPort);	        //设置服务器IP地址
	servaddr.sin_addr.s_addr = inet_addr(strIp);//设置服务器监听端口

//	flag = connect_nonb(m_iSocketId, (struct sockaddr *)&servaddr, sizeof(servaddr), m_iTimeout);//del by xiaozhengxiu 20130703
	flag = connect_nonb(iTmpSocketId, (struct sockaddr *)&servaddr, sizeof(servaddr), m_iTimeout);//add by xiaozhengxiu 20130703
	if(flag != 0)  //连接失败
	{
//add by xiaozhengxiu 20130703
#ifdef WIN32
		closesocket(iTmpSocketId); 
#else
		close(iTmpSocketId);
#endif
		m_iSocketId = -1;
//add by xiaozhengxiu 20130703 end		
		if(CLIENT_SOCK_CONNECT_TIMEOUT == flag)
		{
			CreateLog(CLIENT_SOCK_CONNECT_TIMEOUT, iResult);
			return iResult;
		}
		
		CreateLog(CLIENT_SOCK_CONNECT_ERR, iResult);
		return iResult;
	}
	strcpy(m_strIp, strIp);
	m_iSocketId = iTmpSocketId;//add by xiaozhengxiu 20130703
	return SOCKET_SUCCEES;
}

	//说明：一次发送。
	//返回值：以buffer开始 iLen 长度 的数据全部发送完成 其他情况返回错误码。
	//参数： 
	//buffer :[in]（*buffer）[out] 将要发送数据的缓冲区地址。
	//iLen: [in] 将要发送数据的长度。
	//iSendedLen:[out] 已经发送数据的长度。
int C_TcpSocket::OneSend(const char *buffer, int iLen, int &iSendedLen)
{
	int              sendnum = 0;
	int              flag = 0;
	fd_set           fSet;
  struct timeval   timeout;
	int iResult = -1;
	if((!buffer) || (iLen < 0)) 
	{
		CreateLog(CONN_WRITE_PARA_ERR, iResult);
		return iResult;
	} 
	
	if(m_iSocketId <= 0)
	{
		CreateLog(CONN_WRITE_CALL_ERR, iResult);
		return iResult;
	}
		
//del by xiaozhengxiu 20130703
	FD_ZERO(&fSet);
	FD_SET(m_iSocketId, &fSet);
    
    //设置时间限制
	if(m_iTimeout < 0)
		flag = select(m_iSocketId + 1, NULL, &fSet, NULL, NULL);
	else
	{
		timeout.tv_sec = m_iTimeout;
		timeout.tv_usec = 0;
		flag = select(m_iSocketId + 1, NULL, &fSet, NULL, &timeout);
	}

    //写失败
	if(flag < 0)
	{
		CreateLog(CONN_WRITE_SELECT_ERR, iResult);
		return iResult;						
	}
	//写超时		
	if(0 == flag)
	{	
		CreateLog(CONN_WRITE_TIMEOUT, iResult);
		return iResult;				
	}
//*///del by xiaozhengxiu 20130703 end
    
    //写数据
	iSendedLen = 0;
#ifdef WIN32
	if(-1 == (sendnum = send(m_iSocketId, buffer, iLen, 0)))
#else
	if(-1 == (sendnum = send(m_iSocketId, (void*)buffer, iLen, 0)))
#endif
	{
	    //写失败
		perror("send failed:");
		CreateLog(CONN_WRITE_SEND_FAIL, iResult);
		return iResult;	
		//return CONN_WRITE_SEND_FAIL;	
	}
	
	//printf("sendlen = %d\n",sendnum);
	
	iSendedLen = sendnum;
	
	return SOCKET_SUCCEES;
}
	//说明：发送指定长度的数据。
	//返回值：以buffer开始 iLen 长度 的数据全部发送完成 返回0，其他情况返回错误码。
	//参数： 
	//buffer :[in]（*buffer）[out] 将要发送数据的缓冲区地址。
	//iLen: [in] 将要发送数据的长度。
	//iSendedLen:[out] 已经发送数据的长度。返回值为0时 iLen == iSendedLen
int C_TcpSocket::Send(const char *buffer, int iLen, int &iSendedLen)
{
	int              sendnum = 0;
	int              flag = 0;
	fd_set           fSet;
  struct timeval   timeout;
  int sendlen = iLen;
  int i = 0;
	int iResult = -1;
	if((!buffer) || (iLen < 0)) 
	{
		CreateLog(CONN_WRITE_PARA_ERR, iResult);
		return iResult;	
     //   return CONN_WRITE_PARA_ERR;  
  }
	
	if(m_iSocketId <= 0)
	{
		CreateLog(CONN_WRITE_CALL_ERR, iResult);
		return iResult;
	}
		//return CONN_WRITE_CALL_ERR;
		
//del by xiaozhengxiu 20130703
	FD_ZERO(&fSet);
	FD_SET(m_iSocketId, &fSet);
    
	//	printf("tttttttttt\n");
    //设置时间限制
	if(m_iTimeout < 0)
		flag = select(m_iSocketId + 1, NULL, &fSet, NULL, NULL);
	else
	{
		timeout.tv_sec = m_iTimeout;
		timeout.tv_usec = 0;
		flag = select(m_iSocketId + 1, NULL, &fSet, NULL, &timeout);
	}
	//	printf("dddddddddddd\n");

    //写失败
	if(flag < 0)
	{
		//return CONN_WRITE_SELECT_ERR;	
		CreateLog(CONN_WRITE_SELECT_ERR, iResult);
		return iResult;
	}
	
	//写超时		
	if(0 == flag)
	{
		CreateLog(CONN_WRITE_TIMEOUT, iResult);
		return iResult;
	}			
		//return CONN_WRITE_TIMEOUT;
//*///del by xiaozhengxiu 20130703 end
    
	//	printf("qqqqqqqqqqqq\n");
    //写数据
	iSendedLen = 0;
	do
	{
	//	printf("222222222222\n");
#ifdef WIN32
		sendnum = send(m_iSocketId, buffer+iSendedLen, sendlen, 0);
#else
		sendnum = send(m_iSocketId, (void*)(buffer+iSendedLen), sendlen, 0);
#endif 
		i++;
		//printf("send count = %d\n",i);
		iSendedLen = iSendedLen + sendnum;
		//printf("sendlen = %d, onesendlen = \n",iSendedLen, sendnum);
		if (sendnum == sendlen)
			break;
		else if (sendnum >0 )
			sendlen = sendlen - sendnum;
		else
		{
			perror("send failed:");
			CreateLog(CONN_WRITE_SEND_FAIL, iResult);
			return iResult;
			//return CONN_WRITE_SEND_FAIL;	
		}		
	}while((sendlen > 0)&&(sendnum>0));

	if (iSendedLen == iLen)
		return SOCKET_SUCCEES;
	else
		return -1;
}

	//说明：接收由 iLen指定长度的数据。接收结果存入 buffer中。
	//返回值：成功接收到iLen长度的数据返回0，其他情况返回错误码。
	//        地址（buffer + iLen)的有效性由调用方保证。
	//参数：buffer :[in]（*buffer）[out] 将要接收数据的缓冲区地址。
	//      iLen:[in]:想要接收的数据长度。
	//      iRecvedLen: [out] 实际接收到的数据长度。
int C_TcpSocket::Recv(char *buffer, int iLen, int &iRecvedLen)
{
	int ret = -1;
	int tmpdatalen = 0;//临时缓存区允许接收的数据长度。
	int tmpbuflen = 0;//实际buffer大小
	int tmprecvlen = 0;//一次接收到数据长度
	//int i = 0;
	int tmplen = 0;

	iRecvedLen = 0;
  memset(buffer,0,iLen);
  tmplen = iLen;
	//printf("[INFO] Len=%d,TmpBufferLen=%d,TmpDataBufferLen=%d\n",iLen,m_iTmpBufferLen,m_iTmpDataBufferLen);
	do//while((ret = OneRecv(m_TmpExBuffer, tmpbuflen, tmprecvlen)) == 0)
	{
	  tmpdatalen = m_iTmpBufferLen-m_iTmpDataBufferLen;//接收缓存区允许再接收的数据长度
		//printf("[INFO] tmpdatalen :%d\n",tmpdatalen);
		tmplen = tmplen - tmprecvlen;
		if ((tmplen <= 0)||(tmpdatalen <= 0)) break;
		//printf("[INFO] tmplen :%d\n",tmplen);
		if (tmplen < tmpdatalen)
			tmpbuflen = tmplen;
		else
			tmpbuflen = tmpdatalen;
		//printf("[INFO] tmpbuflen :%d\n",tmpbuflen);
		ret = OneRecv(m_TmpBuffer+m_iTmpDataBufferLen, tmpbuflen, tmprecvlen);
		//printf("[INFO] Recv :%d\n",ret);			
		if (0 != ret) 
		{
			break;
//			return ret;
		}
		m_iTmpDataBufferLen = m_iTmpDataBufferLen + tmprecvlen;
		//printf("[INFO] m_iTmpDataBufferLen :%d\n",m_iTmpDataBufferLen);			
	}while((ret == 0)&&(tmpbuflen>0)&&(tmplen>0));

	if (m_iTmpDataBufferLen <= 0) return ret;
		
	//有接收到数据，以下没有区分接收部分数据还是接收到规定长度的数据
	
	//将完整的数据返回
	iRecvedLen = m_iTmpDataBufferLen;
	memcpy(buffer, m_TmpBuffer, m_iTmpDataBufferLen);
//	printf("[----m_TmpBuffer-----]:%d\n",m_iTmpDataBufferLen);
//	for(i = 0; i < m_iTmpDataBufferLen; i++)
//		printf("%02X ", m_TmpBuffer[i]);
//	printf("\n");

	//临时变量中没有剩余数据，将临时变量初始化
	memset(m_TmpBuffer, 0, m_iTmpBufferLen);
	m_iTmpDataBufferLen = 0;

	if (iRecvedLen == iLen)
		return SOCKET_SUCCEES;
	else
		return -1;
}

	// 说明：接收由 cTail 为结束标志符的数据。 接收数据后正向查找接收串检索结束标志（下同）
	// 地址:（buffer + iLen)的有效性由调用方保证。
	//返回值：接收成功返回0 其他情况返回错误码。
	//参数:buffer :[in]（*buffer）[out] 将要接收数据的缓冲区地址。
	//iLen:接收缓冲区的长度。
	//cTail:结束标志字符。
	//iRecvedLen: [out] 实际接收到的数据长度。
int C_TcpSocket::Recv(char *buffer, int iLen, char cTail, int &iRecvedLen)
{
	int ret = -1;
//	char tmpchar = {0};
	int tmpdatalen = 0;//临时缓存区允许接收的数据长度。
	int tmpbuflen = 0;//实际buffer大小
	int tmprecvlen = 0;//接收到数据长度
	int i = 0;
	int ifind = 0;
	int tmplen = 0;

	iRecvedLen = 0;

//	if (iLen < m_iTmpBufferLen) return SOCKET_PARAR_ERR;//接收缓存区太小//del by xiaozhengxiu 20130703,按最小缓冲区接收
/*
	//方法一：一个一个字符读取
	while(((ret = OneRecv(&tmpchar, 1, tmprecvlen)) == 0)&&(iLen > m_iTmpBufferLen))
	{
		if (0 != ret) return ret;
		
	  memcpy(m_TmpBuffer+m_iTmpBufferLen, &tmpchar, tmprecvlen);
  	m_iTmpBufferLen = m_iTmpBufferLen + tmprecvlen;
  	
  	if (0 == memcmp(&tmpchar,&cTail,1))break;
  }
	//将完整的数据返回
	iRecvedLen = m_iTmpBufferLen;
	memcpy(buffer, m_TmpBuffer, m_iTmpBufferLen);
	//清空临时缓冲区
	memset(m_TmpBuffer, 0, m_iTmpBufferLen);
	m_iTmpBufferLen = 0;
*/
	//方法二：一次读取
  tmplen = iLen;
  memset(buffer,0,iLen);
  memset(m_TmpExBuffer,0,m_iTmpExBufferLen);
  
	//20130722 by xiaozhengxiu add 
	//先查找m_TmpBuffer中保存的数据
	if (m_iTmpDataBufferLen>0)
	{
			for(i=0;i<m_iTmpDataBufferLen;i++)
			{
		  	if (0 == memcmp(&*(m_TmpBuffer+i),&cTail,1))
		  	{
					//将完整的数据返回
					iRecvedLen = i+1;
					memcpy(buffer, m_TmpBuffer, i+1);
					
					//修改剩余的数据存储到临时变量中
					m_iTmpDataBufferLen = m_iTmpDataBufferLen-i-1;
					memcpy(m_TmpExBuffer,m_TmpBuffer+i+1, m_iTmpDataBufferLen);
					memset(m_TmpBuffer, 0, m_iTmpBufferLen);
					memcpy(m_TmpBuffer,m_TmpExBuffer, m_iTmpDataBufferLen);
				//	printf("[----m_TmpBuffer-----]:%d\n",m_iTmpDataBufferLen);
				//	for(i = 0; i < m_iTmpDataBufferLen; i++)
				//		printf("%02X ", m_TmpBuffer[i]);
				//	printf("\n");
					
					return SOCKET_SUCCEES;
		  	}
			}
	}
	//20130722 by xiaozhengxiu add end
  
	do//while((ret = OneRecv(m_TmpExBuffer, tmpbuflen, tmprecvlen)) == 0)
	{
	  tmpdatalen = m_iTmpBufferLen-m_iTmpDataBufferLen;//接收缓存区允许再接收的数据长度
		tmplen = tmplen - tmprecvlen;
		if ((tmplen <= 0)||(tmpdatalen <= 0)) break;
		//printf("[INFO] tmplen :%d\n",tmplen);
		if (tmplen < tmpdatalen)
			tmpbuflen = tmplen;
		else
			tmpbuflen = tmpdatalen;
		//printf("[INFO] tmpbuflen :%d\n",tmpbuflen);
		ret = OneRecv(m_TmpExBuffer, tmpbuflen, tmprecvlen);
		if (0 != ret) 
		{
			printf("[INFO] Recv :%d\n",ret);
			return ret;//20130722 by xiaozhengxiu del 接收失败，但m_TmpBuffer有可能还有数据
		}
		for(i=0;i<tmpbuflen;i++)
		{
	  	if (0 == memcmp(&*(m_TmpExBuffer+i),&cTail,1))
	  	{
//				printf("[INFO] cTail :%c\n",cTail);			
	  		memcpy(m_TmpBuffer+m_iTmpDataBufferLen,m_TmpExBuffer,i+1);
		  	m_iTmpDataBufferLen = m_iTmpDataBufferLen + i + 1;
		  	ifind = 1;
				break;	  		
	  	}
		}
		if (ifind)
			break;
//		printf("[INFO] m_iTmpBufferLen :%d\n",m_iTmpBufferLen);			
	}while((ret == 0)&&(tmpbuflen>0));

	//将完整的数据返回
	iRecvedLen = m_iTmpDataBufferLen;
	memcpy(buffer, m_TmpBuffer, m_iTmpDataBufferLen);

	//将剩余的数据存储到临时变量中
	memset(m_TmpBuffer, 0, m_iTmpBufferLen);
	m_iTmpDataBufferLen = tmprecvlen-i-1;
	memcpy(m_TmpBuffer,m_TmpExBuffer+i+1, m_iTmpDataBufferLen);
	//printf("[----m_TmpBuffer-----]:%d\n",m_iTmpDataBufferLen);
	//for(i = 0; i < m_iTmpDataBufferLen; i++)
	//	printf("%02X ", m_TmpBuffer[i]);
//	printf("\n");

	ifind = 0;
	
	return SOCKET_SUCCEES;
}

	// 说明：接收由 strTail 为结束标志串的数据。 
	// 地址:（buffer + iLen)的有效性由调用方保证。
	//返回值：接收成功返回0 其他情况返回错误码。
	//参数:buffer :[in]（*buffer）[out] 将要接收数据的缓冲区地址。
	//iLen [in]:接收缓冲区的长度。
	//strTail [in]:结束标志串。 中间可以 '\0' 也可以不以 '\0' 结尾。 
	//iTailLen [in]: "strTail"的长度。
	//iRecvedLen: [out] 实际接收到的数据长度。	
int C_TcpSocket::Recv(char *buffer, int iLen, char* strTail, int iTailLen, int &iRecvedLen)
{
	int ret = 0;
//	char tmpchar = {0};
	int tmpdatalen = 0;//临时缓存区允许接收的数据长度。
	int tmpbuflen = 0;//实际buffer大小
	int tmprecvlen = 0;//接收到数据长度
	int i = 0;
	int ifind = 0;
	int tmplen = 0;
	
	iRecvedLen = 0;
	
//	if (iLen < m_iTmpBufferLen) return SOCKET_PARAR_ERR;//接收缓存区太小//del by xiaozhengxiu 20130703,按最小缓冲区接收

/*	//方法一：一个一个字符读取
	while(((ret == OneRecv(&tmpchar, 1, tmprecvlen)) != 0)&&(iLen > m_iTmpBufferLen))
	{
		if (0 != ret) return ret;
		
	  memcpy(m_TmpBuffer+m_iTmpBufferLen, &tmpchar, tmprecvlen);
  	m_iTmpBufferLen = m_iTmpBufferLen + tmprecvlen;
  	
  	if (i<iLen)
  	{
//	 		printf("tmpchar = %02x\n",tmpchar);
// 			printf("strail = %02x\n",strTail[i]);
  		if (0 == memcmp(&tmpchar,&strTail[i],1))
  			{
  				i++;
  				if (i == iTailLen) break;
  				else
  					continue;
  			}
  		else
  			i = 0;
  	}
  	else
  		break;  		
  }
	//将完整的数据返回
	iRecvedLen = m_iTmpBufferLen;
	memcpy(buffer, m_TmpBuffer, m_iTmpBufferLen);
	//清空临时缓冲区
	memset(m_TmpBuffer, 0, m_iTmpBufferLen);
	m_iTmpBufferLen = 0;
*/
	//方法二：一次读取
  tmplen = iLen;
  memset(buffer,0,iLen);
  memset(m_TmpExBuffer,0,m_iTmpExBufferLen);
  
	//20130722 by xiaozhengxiu add 
	//先查找m_TmpBuffer中保存的数据
	if (m_iTmpDataBufferLen>0)
	{
			for(i=0;i<m_iTmpDataBufferLen;i++)
			{
		  	if (0 == memcmp(&*(m_TmpBuffer+i),strTail,iTailLen))
		  	{
					//将完整的数据返回
					iRecvedLen = i+iTailLen;
					memcpy(buffer, m_TmpBuffer, i+iTailLen);
					
					//修改剩余的数据存储到临时变量中
					m_iTmpDataBufferLen = m_iTmpDataBufferLen-i-iTailLen;
					memcpy(m_TmpExBuffer,m_TmpBuffer+i+iTailLen, m_iTmpDataBufferLen);
					memset(m_TmpBuffer, 0, m_iTmpBufferLen);
					memcpy(m_TmpBuffer,m_TmpExBuffer, m_iTmpDataBufferLen);
				//	printf("[----m_TmpBuffer-----]:%d\n",m_iTmpDataBufferLen);
				//	for(i = 0; i < m_iTmpDataBufferLen; i++)
					//	printf("%02X ", m_TmpBuffer[i]);
					//printf("\n");
					
					return SOCKET_SUCCEES;
		  	}
			}
	}
	//20130722 by xiaozhengxiu add end
  
	do//while((ret = OneRecv(m_TmpExBuffer, tmpbuflen, tmprecvlen)) == 0)
	{
		//允许接收的数据长度
	  tmpdatalen = m_iTmpBufferLen-m_iTmpDataBufferLen;//接收缓存区允许再接收的数据长度
		tmplen = tmplen - tmprecvlen;
		if ((tmplen <= 0)||(tmpdatalen <= 0)) break;
		//printf("[INFO] tmplen :%d\n",tmplen);
		if (tmplen < tmpdatalen)
			tmpbuflen = tmplen;
		else
			tmpbuflen = tmpdatalen;
			
		ret = OneRecv(m_TmpExBuffer, tmpbuflen, tmprecvlen);
		if (0 != ret) 
		{
			printf("[INFO] Recv :%d\n",ret);			
			return ret;
		}
		for(i=0;i<tmpbuflen;i++)
		{
	  	if (0 == memcmp(&*(m_TmpExBuffer+i),strTail,iTailLen))
	  	{
//				printf("[INFO] cTail :%c\n",cTail);			
	  		memcpy(m_TmpBuffer+m_iTmpDataBufferLen,m_TmpExBuffer,i+iTailLen);
		  	m_iTmpDataBufferLen = m_iTmpDataBufferLen + i + iTailLen;
		  	ifind = 1;
				break;	  		
	  	}
		}
		
		if (ifind)
			break;
//		printf("[INFO] m_iTmpBufferLen :%d\n",m_iTmpBufferLen);			
	}while((ret == 0)&&(tmpbuflen>0));

	//将完整的数据返回
	iRecvedLen = m_iTmpDataBufferLen;
	memcpy(buffer, m_TmpBuffer, m_iTmpDataBufferLen);

	//将剩余的数据存储到临时变量中
	memset(m_TmpBuffer, 0, m_iTmpBufferLen);
	m_iTmpDataBufferLen = tmprecvlen-i-iTailLen;
	memcpy(m_TmpBuffer,m_TmpExBuffer+i+iTailLen, m_iTmpDataBufferLen);
	//printf("[----m_TmpBuffer-----]:%d\n",m_iTmpDataBufferLen);
	//for(i = 0; i < m_iTmpDataBufferLen; i++)
	//	printf("%02X ", m_TmpBuffer[i]);
	//printf("\n");

	ifind = 0;

	return SOCKET_SUCCEES;
}

	//说明：接收由 pTail->strTail 为多个结束标志串的数据。此函数负责检测接收串中
	//是否包含由 pTail->strTail 指定多个标志串，只要有任何一个检测到其中一个标志串，即为接收成功。
	//地址:（buffer + iLen)的有效性由调用方保证。
	//返回值：接收成功返回0 其他情况返回错误码。
	//参数:buffer :[in]（*buffer）[out] 将要接收数据的缓冲区地址。
	//iLen [in]:接收缓冲区的长度。
	//pTail [in]: TcpTail数组元素的首指针。
	//iTailSize: "pTail" 对应数组的元素个数。
	//iRecvedLen: [out] 实际接收到的数据长度。
	//iTailPos:结束标志串在“pTail”标识的数组中的位置。
int C_TcpSocket::Recv(char *buffer, int iLen, TcpTail* pTail, int iTailSize, int &iRecvedLen, int &iTailPos)
{
	int ret = 0;
	int tmpdatalen = 0;//临时缓存区允许接收的数据长度。
	int tmpbuflen = 0;//实际buffer大小
	int tmprecvlen = 0;//接收到数据长度
	int ExDatarecvlen = 0;//累计接收到数据长度
	int i = 0;
	int j = 0;
	int ifind = 0;
	int tmplen = 0;
	
	iRecvedLen = 0;
	iTailPos = 0;
  memset(buffer,0,iLen);
  memset(m_TmpExBuffer,0,m_iTmpExBufferLen);
  tmplen = iLen;
	
	//20130722 by xiaozhengxiu add 
	//先查找m_TmpBuffer中保存的数据
	if (m_iTmpDataBufferLen>0)
	{
		for(j=0;j<iTailSize;j++)
		{
			for(i=0;i<m_iTmpDataBufferLen;i++)
			{
		  	if (0 == memcmp(&*(m_TmpBuffer+i),pTail->strTail,pTail->iTailLen))
		  	{
					//将完整的数据返回
		  		memcpy(buffer,m_TmpBuffer,i+(pTail->iTailLen));
			  	m_iTmpDataBufferLen = m_iTmpDataBufferLen + i + (pTail->iTailLen);
					iTailPos = j;

					//修改剩余的数据存储到临时变量中 memset(m_TmpExBuffer, 0, tmpbuflen); 
					m_iTmpDataBufferLen = m_iTmpDataBufferLen-i-(pTail->iTailLen); 
					memcpy(m_TmpExBuffer,m_TmpBuffer+i+(pTail->iTailLen),m_iTmpDataBufferLen); 
					memset(m_TmpBuffer, 0, m_iTmpBufferLen); 
					memcpy(m_TmpBuffer,m_TmpExBuffer, m_iTmpDataBufferLen); 
					//printf("[----m_TmpBuffer-----]:%d\n",m_iTmpDataBufferLen); 
					//for(i = 0; i < m_iTmpDataBufferLen; i++) 
						//printf("%02X ", m_TmpBuffer[i]); 
					//printf("\n"); 
				} 
			} 
			pTail++; 
		} 
	} //20130722 by xiaozhengxiu add end
  
	do//while((ret = OneRecv(m_TmpExBuffer, tmpbuflen, tmprecvlen)) == 0)
	{
		//允许接收的数据长度
	  tmpdatalen = m_iTmpBufferLen-m_iTmpDataBufferLen-ExDatarecvlen;//接收缓存区允许再接收的数据长度
		tmplen = tmplen - tmprecvlen;
		if ((tmplen <= 0)||(tmpdatalen <= 0)) break;
		//printf("[INFO] tmplen :%d\n",tmplen);
		if (tmplen < tmpdatalen)
			tmpbuflen = tmplen;
		else
			tmpbuflen = tmpdatalen;
			
		ret = OneRecv(m_TmpExBuffer+ExDatarecvlen, tmpbuflen, tmprecvlen);
		//printf("[----m_TmpExBuffer-----]:%d\n",tmprecvlen);
		//for(i = 0; i < tmprecvlen; i++)
			//printf("%02X ", m_TmpExBuffer[ExDatarecvlen+i]);
		//printf("\n");
		if (0 != ret) 
		{
			printf("[INFO] Recv :%d\n",ret);			
			return ret;
		}
		
		for(j=0;j<iTailSize;j++)
		{
			for(i=0;i<tmpbuflen;i++)
			{
//				printf("i=%d\n",i);
		  	if (0 == memcmp(&*(m_TmpExBuffer+ExDatarecvlen+i),pTail->strTail,pTail->iTailLen))
		  	{
//					printf("[INFO] strTail :%s\n",pTail->strTail);
		  		memcpy(m_TmpBuffer+m_iTmpDataBufferLen,m_TmpExBuffer,ExDatarecvlen+i+pTail->iTailLen);
			  	m_iTmpDataBufferLen = m_iTmpDataBufferLen + ExDatarecvlen + i + pTail->iTailLen;
			  	ifind = 1;
					break;	  		
		  	}
//		  	printf("j=%d\n",j);
			}
			if (ifind)
				break;
			pTail++;
		}
		
		ExDatarecvlen = ExDatarecvlen + tmprecvlen;
		
		if (ifind)
			break;
//		printf("[INFO] m_iTmpBufferLen :%d\n",m_iTmpBufferLen);			
	}while((ret == 0)&&(tmpbuflen>0));

	//将完整的数据返回
	iRecvedLen = m_iTmpDataBufferLen;
	memcpy(buffer, m_TmpBuffer, m_iTmpDataBufferLen);
	iTailPos = j;

	//将剩余的数据存储到临时变量中
	memset(m_TmpBuffer, 0, m_iTmpBufferLen);
	m_iTmpDataBufferLen = tmprecvlen-i-(pTail->iTailLen);
	memcpy(m_TmpBuffer,m_TmpExBuffer+i+(pTail->iTailLen), m_iTmpDataBufferLen);
	//printf("[----m_TmpBuffer-----]:%d\n",m_iTmpDataBufferLen);
	//for(i = 0; i < m_iTmpDataBufferLen; i++)
	//	printf("%02X ", m_TmpBuffer[i]);
	//printf("\n");

	ifind = 0;

	return SOCKET_SUCCEES;
}

	//说明：关闭tcp连接。
	//返回值：断开连接成功返回 0，断开连接失败 -1；
	//strIp：
int C_TcpSocket::TcpDisConnect()
{
	if(-1 != m_iSocketId)
	{
#ifdef WIN32
		closesocket(m_iSocketId); 
#else
		close(m_iSocketId);
#endif

		m_iSocketId = -1;   
	}
//	printf("disconnect!\n");
	return SOCKET_SUCCEES;
}

	//设置通信类自身对应的模块，和子模块，以便发生错误时输出到指定的日志。
	//返回值：无
	//iModule:[in]模块编号 iSubModule:[in]子模块编号。
void C_TcpSocket::SetLog(int iModule, int iSubModule)
{
	m_iLogModuleNum    = iModule;
	m_iLogSubModuleNum = iSubModule;
	return ;
}

//==========================private===================================//

/****FUNCTION***************************************************
* DESCRIPTION : 建立与服务器端连接, 非阻塞模式connect函数
*       INPUT : sockfd      套皆口描述符                
*		        nsec        超时时间设置
*     RETURNS :  
*    CAUTIONS : nsec>0 connect 阻塞nsec时长，如果在此时间段内连接不成功返回
				nsec=0 connect 不阻塞，如果连接不成功立即返回
				nsec<0 connect 阻塞，作用为常规阻塞型connect
****************************************************************/
int C_TcpSocket::connect_nonb(int				sockfd, 
							   const struct sockaddr *saptr, 
							   SOKLEN                 salen, 
							   int                    nsec)
{
	int             flags = 0; 
	int             n = 0; 
	int iResult = -1;
#ifdef WIN32
	char            error = 0;
#else
	int             error = 0;
#endif
	SOKLEN          len = 0;
	fd_set          rset;
	fd_set          wset;
	struct timeval  tval;

#ifdef WIN32
	unsigned   long   ul   =   1;   
	flags = ioctlsocket(sockfd, FIONBIO, (unsigned long*)&ul);   
	if(SOCKET_ERROR == flags)
	{
		CreateLog(SOCKET_CALL_SYSAPI_ERR, iResult);
		return iResult;
	}
	//	return SOCKET_CALL_SYSAPI_ERR;   
#else
	flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);   //设置为非阻塞模式
#endif

	if((n = connect(sockfd, saptr, salen)) < 0)   //连接失败
	{	
#ifdef WIN32
		if(WSAEWOULDBLOCK != WSAGetLastError())	
#else
		printf("EINPROGRESS = %d  errno: %d\n",EINPROGRESS, errno);
		if(EINPROGRESS != errno)
#endif
		{
			CreateLog(CLIENT_SOCK_CONNECT_ERR, iResult);
			return iResult;
			//return CLIENT_SOCK_CONNECT_ERR;
		}
	}

	if(0 != n)
	{
		FD_ZERO(&rset);
		FD_SET(sockfd, &rset);
		wset = rset;
		tval.tv_sec = nsec;
		tval.tv_usec = 0;
		
        //设置连接超时限制
		if(0 == (n = select(sockfd+1, &rset, &wset, NULL, (nsec < 0)?NULL:&tval)))
		{
#ifdef WIN32
			closesocket(sockfd); 
#else
			close(sockfd);
			errno = ETIMEDOUT;
#endif
			//连接超时
			
			CreateLog(CLIENT_SOCK_CONNECT_TIMEOUT, iResult);
			return iResult;
			//return CLIENT_SOCK_CONNECT_TIMEOUT;
		}
        
        //读、写可用
		if(FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset))
		{
#ifndef WIN32
			len = sizeof(error);
			if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
			{
				CreateLog(CLIENT_SOCK_CONNECT_ERR, iResult);
				return iResult;
				//return CLIENT_SOCK_CONNECT_ERR;
			}
#endif
		}
	//	else
			//printf("select error: sockfd not set\n");
	}

#ifdef WIN32
  ul = 0;   
	flags = ioctlsocket(sockfd, FIONBIO, (unsigned long*)&ul);   
	if(SOCKET_ERROR == flags)
	{
		CreateLog(SOCKET_CALL_SYSAPI_ERR, iResult);
		return iResult;
		//return SOCKET_CALL_SYSAPI_ERR;
	}   
#else
	fcntl(sockfd, F_SETFL, flags);   //恢复套接口原始设置
#endif	

	if(error)
	{
#ifdef WIN32
		closesocket(sockfd); 
#else
		close(sockfd);
#endif	
		errno = error;
		CreateLog(SOCKET_CALL_SYSAPI_ERR, iResult);
		return iResult;
		//return CLIENT_SOCK_CONNECT_ERR;
	}
	return SOCKET_SUCCEES;
}

	//说明：一次接收。接收结果存入 buffer中。
	//返回值：成功接收到数据返回0，其他情况返回错误码。
	//        地址（buffer + iLen)的有效性由调用方保证。
	//参数：buffer :[in]（*buffer）[out] 将要接收数据的缓冲区地址。
	//      iLen:[in]:想要接收的数据长度。
	//      iRecvedLen: [out] 实际接收到的数据长度。
int C_TcpSocket::OneRecv(char *buffer, int iLen, int &iRecvedLen)
{
	int iResult = -1;
	int              readnum = 0;
	int              flag = 0;
	fd_set           fSet;
  struct timeval   timeout;

	if((!buffer) || (iLen < 0))
	{ 
		CreateLog(CONN_READ_PARA_ERR, iResult);
		return iResult;
        //return CONN_READ_PARA_ERR; 
	} 
	
	if(m_iSocketId <= 0)
	{
		CreateLog(CONN_READ_CALL_ERR, iResult);
		return iResult;
		//return CONN_READ_CALL_ERR;
	}

	FD_ZERO(&fSet);
	FD_SET(m_iSocketId, &fSet);
    
    //设置时间限制
	if(m_iTimeout < 0)
		flag = select(m_iSocketId + 1, &fSet, NULL, NULL, NULL);
	else
	{
		timeout.tv_sec = m_iTimeout;
		timeout.tv_usec = 0;
		flag = select(m_iSocketId + 1, &fSet, NULL, NULL, &timeout);
	}

    //读失败
	if(flag < 0)	
	{	
		CreateLog(CONN_READ_SELECT_ERR, iResult);
		return iResult;				
		//return CONN_READ_SELECT_ERR;
	}	
	//读超时		
	if(0 == flag)	
	{	
		CreateLog(CONN_READ_TIMEOUT, iResult);
		return iResult;			
		//return CONN_READ_TIMEOUT;
	}
    
    //读数据
#ifdef WIN32
	readnum = recv(m_iSocketId, buffer, iLen, 0);
#else
	readnum = recv(m_iSocketId, (void*)buffer, iLen, 0);
#endif
	if(-1 == readnum)
	{
	    //读失败
		iRecvedLen = 0;
		CreateLog(CONN_READ_RECV_FAIL, iResult);
		return iResult;
		//return CONN_READ_RECV_FAIL;
	}
	
	if(0 == readnum )  //对方连接关闭
	{	
		iRecvedLen = 0;
		CreateLog(CONN_PEER_TERMINATOR, iResult);
		return iResult;
		//return CONN_PEER_TERMINATOR;
	}	

	iRecvedLen = readnum;

	return SOCKET_SUCCEES;
}

int C_TcpSocket::CreateLog(int iLogNum, int &iReturnErrorNum)
{
		string str;
		switch(iLogNum)
		{
			case SOCKET_WIN_ENVIRONMENT_ERROR:
			{
				str = "Windows环境错误";
			}
			break;
			
			case SOCKET_NULL_FD:
			{
				str = "空的socket文件描述符号";
			}
			break;
			
			case SOCKET_CALL_SYSAPI_ERR:
			{
				str = "调用系统API错误";
			}
			break;
			
			case SERVER_SOCK_INI_ERR:
			{
				str = "server端socket初始化错误";
			}
			break;
			
			case SERVER_SOCK_BIND_ERR:
			{
				str = "server端bind错误";
			}
			break;
			
			case SERVER_SOCK_LISTEN_ERR:
			{
				str = "server端listen错误";
			}
			break;
			
			case SERVER_SOCK_ACCEPT_ERR:
			{
				str = "server端accept错误";
			}
			break;	
			
			case CLIENT_SOCK_INI_ERR:
			{
				str = "client端socket初始化错误";
			}
			break;	
			
			case CLIENT_SOCK_BIND_ERR:
			{
				str = "client端bind错误";
			}
			break;																					

			case CLIENT_SOCK_CONNECT_ERR:
			{
				str = "client端connect错误";
			}
			break;	
			
			case CLIENT_SOCK_CONNECT_TIMEOUT:
			{
				str = "client端connect超时";
			}
			break;	
			
			case CONN_READ_PARA_ERR:
			{
				str = "连接读操作参数输入错误";
			}
			break;
			
			case CONN_READ_CALL_ERR:
			{
				str = "连接读操作调用错误";
			}
			break;
			
			case CONN_READ_SELECT_ERR:
			{
				str = "连接读操作select错误";
			}
			break;
				
			case CONN_READ_TIMEOUT:
			{
				str = "连接读操作超时";
			}
			break;	
			
			case CONN_READ_RECV_FAIL:
			{
				str = "连接读操作recv错误";
			}
			break;	
			
			case CONN_WRITE_PARA_ERR:
			{
				str = "连接写操作参数输入错误";
			}
			break;	
			
			case CONN_WRITE_CALL_ERR:
			{
				str = "连接写操作调用错误";
			}
			break;
			
			case CONN_WRITE_SELECT_ERR:
			{
				str = "连接写操作elect错误";
			}
			break;	
			
			case CONN_WRITE_TIMEOUT:
			{
				str = "连接写操作超时";
			}
			break;	
			
			case CONN_WRITE_SEND_FAIL:
			{
				str = "连接写操作send错误";
			}
			break;	
			
			case CONN_PEER_TERMINATOR:
			{
				str = "连接对端关闭";
			}
			break;	
			
			default:
			{
				str = "未知的错误";
			}																	
		}
		str = m_strIp + str;
//		C_LogManage *pLogManage = C_LogManage::GetInstance();
//		iReturnErrorNum = pLogManage->CreateLogNumber(3,m_iLogModuleNum,m_iLogSubModuleNum,iLogNum);
//		pLogManage->WriteLog(iReturnErrorNum,str);
		m_strError = str;
		return  0;
}

void C_TcpSocket::GetErrorInfo(std::string &strError)
{
	strError = m_strError;
}
