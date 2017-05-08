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
const int SOCKET_SUCCEES                 =0;        //�����ɹ�

using namespace std;

	//iTmpBufferLen��ָ����������ʱ����ĳ��ȣ�ͬʱ��m_TmpBuffer ����ռ䡣
C_TcpSocket::C_TcpSocket(int iTmpBufferLen)
{
  //�׽���������
	m_iSocketId = -1; 
  //ͨ�ŵĳ�ʱʱ��
  m_iTimeout = 3;
  //��ʱ���泤�ȡ�
  m_iTmpBufferLen = iTmpBufferLen;
  //��ʱ�������ݳ��ȡ�
  m_iTmpDataBufferLen = 0;//add by xiaozhengxiu 20130703
  //��ʱ�����ַ��
  m_TmpBuffer = new char[iTmpBufferLen];
  memset(m_TmpBuffer,0,iTmpBufferLen);
  //������������С,��ͬm_iTmpDataBufferLen
  m_iTmpExBufferLen = iTmpBufferLen;
  //����������
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

	//˵��������tcp���ӡ�
	//����ֵ�����ӳɹ����� 0������ʧ�� -1��
	//strIp��[in] ��usPort [in], iWait [in];
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
	
	if(flag < 0)    //��ʼ��socketʧ��
	{
		perror("[socket error]:");
		CreateLog(CLIENT_SOCK_INI_ERR, iResult);
		return iResult;
		//return CLIENT_SOCK_INI_ERR;
	}

	memset (&servaddr, 0, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET;  
	servaddr.sin_port = htons(usPort);	        //���÷�����IP��ַ
	servaddr.sin_addr.s_addr = inet_addr(strIp);//���÷����������˿�

//	flag = connect_nonb(m_iSocketId, (struct sockaddr *)&servaddr, sizeof(servaddr), m_iTimeout);//del by xiaozhengxiu 20130703
	flag = connect_nonb(iTmpSocketId, (struct sockaddr *)&servaddr, sizeof(servaddr), m_iTimeout);//add by xiaozhengxiu 20130703
	if(flag != 0)  //����ʧ��
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

	//˵����һ�η��͡�
	//����ֵ����buffer��ʼ iLen ���� ������ȫ��������� ����������ش����롣
	//������ 
	//buffer :[in]��*buffer��[out] ��Ҫ�������ݵĻ�������ַ��
	//iLen: [in] ��Ҫ�������ݵĳ��ȡ�
	//iSendedLen:[out] �Ѿ��������ݵĳ��ȡ�
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
    
    //����ʱ������
	if(m_iTimeout < 0)
		flag = select(m_iSocketId + 1, NULL, &fSet, NULL, NULL);
	else
	{
		timeout.tv_sec = m_iTimeout;
		timeout.tv_usec = 0;
		flag = select(m_iSocketId + 1, NULL, &fSet, NULL, &timeout);
	}

    //дʧ��
	if(flag < 0)
	{
		CreateLog(CONN_WRITE_SELECT_ERR, iResult);
		return iResult;						
	}
	//д��ʱ		
	if(0 == flag)
	{	
		CreateLog(CONN_WRITE_TIMEOUT, iResult);
		return iResult;				
	}
//*///del by xiaozhengxiu 20130703 end
    
    //д����
	iSendedLen = 0;
#ifdef WIN32
	if(-1 == (sendnum = send(m_iSocketId, buffer, iLen, 0)))
#else
	if(-1 == (sendnum = send(m_iSocketId, (void*)buffer, iLen, 0)))
#endif
	{
	    //дʧ��
		perror("send failed:");
		CreateLog(CONN_WRITE_SEND_FAIL, iResult);
		return iResult;	
		//return CONN_WRITE_SEND_FAIL;	
	}
	
	//printf("sendlen = %d\n",sendnum);
	
	iSendedLen = sendnum;
	
	return SOCKET_SUCCEES;
}
	//˵��������ָ�����ȵ����ݡ�
	//����ֵ����buffer��ʼ iLen ���� ������ȫ��������� ����0������������ش����롣
	//������ 
	//buffer :[in]��*buffer��[out] ��Ҫ�������ݵĻ�������ַ��
	//iLen: [in] ��Ҫ�������ݵĳ��ȡ�
	//iSendedLen:[out] �Ѿ��������ݵĳ��ȡ�����ֵΪ0ʱ iLen == iSendedLen
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
    //����ʱ������
	if(m_iTimeout < 0)
		flag = select(m_iSocketId + 1, NULL, &fSet, NULL, NULL);
	else
	{
		timeout.tv_sec = m_iTimeout;
		timeout.tv_usec = 0;
		flag = select(m_iSocketId + 1, NULL, &fSet, NULL, &timeout);
	}
	//	printf("dddddddddddd\n");

    //дʧ��
	if(flag < 0)
	{
		//return CONN_WRITE_SELECT_ERR;	
		CreateLog(CONN_WRITE_SELECT_ERR, iResult);
		return iResult;
	}
	
	//д��ʱ		
	if(0 == flag)
	{
		CreateLog(CONN_WRITE_TIMEOUT, iResult);
		return iResult;
	}			
		//return CONN_WRITE_TIMEOUT;
//*///del by xiaozhengxiu 20130703 end
    
	//	printf("qqqqqqqqqqqq\n");
    //д����
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

	//˵���������� iLenָ�����ȵ����ݡ����ս������ buffer�С�
	//����ֵ���ɹ����յ�iLen���ȵ����ݷ���0������������ش����롣
	//        ��ַ��buffer + iLen)����Ч���ɵ��÷���֤��
	//������buffer :[in]��*buffer��[out] ��Ҫ�������ݵĻ�������ַ��
	//      iLen:[in]:��Ҫ���յ����ݳ��ȡ�
	//      iRecvedLen: [out] ʵ�ʽ��յ������ݳ��ȡ�
int C_TcpSocket::Recv(char *buffer, int iLen, int &iRecvedLen)
{
	int ret = -1;
	int tmpdatalen = 0;//��ʱ������������յ����ݳ��ȡ�
	int tmpbuflen = 0;//ʵ��buffer��С
	int tmprecvlen = 0;//һ�ν��յ����ݳ���
	//int i = 0;
	int tmplen = 0;

	iRecvedLen = 0;
  memset(buffer,0,iLen);
  tmplen = iLen;
	//printf("[INFO] Len=%d,TmpBufferLen=%d,TmpDataBufferLen=%d\n",iLen,m_iTmpBufferLen,m_iTmpDataBufferLen);
	do//while((ret = OneRecv(m_TmpExBuffer, tmpbuflen, tmprecvlen)) == 0)
	{
	  tmpdatalen = m_iTmpBufferLen-m_iTmpDataBufferLen;//���ջ����������ٽ��յ����ݳ���
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
		
	//�н��յ����ݣ�����û�����ֽ��ղ������ݻ��ǽ��յ��涨���ȵ�����
	
	//�����������ݷ���
	iRecvedLen = m_iTmpDataBufferLen;
	memcpy(buffer, m_TmpBuffer, m_iTmpDataBufferLen);
//	printf("[----m_TmpBuffer-----]:%d\n",m_iTmpDataBufferLen);
//	for(i = 0; i < m_iTmpDataBufferLen; i++)
//		printf("%02X ", m_TmpBuffer[i]);
//	printf("\n");

	//��ʱ������û��ʣ�����ݣ�����ʱ������ʼ��
	memset(m_TmpBuffer, 0, m_iTmpBufferLen);
	m_iTmpDataBufferLen = 0;

	if (iRecvedLen == iLen)
		return SOCKET_SUCCEES;
	else
		return -1;
}

	// ˵���������� cTail Ϊ������־�������ݡ� �������ݺ�������ҽ��մ�����������־����ͬ��
	// ��ַ:��buffer + iLen)����Ч���ɵ��÷���֤��
	//����ֵ�����ճɹ�����0 ����������ش����롣
	//����:buffer :[in]��*buffer��[out] ��Ҫ�������ݵĻ�������ַ��
	//iLen:���ջ������ĳ��ȡ�
	//cTail:������־�ַ���
	//iRecvedLen: [out] ʵ�ʽ��յ������ݳ��ȡ�
int C_TcpSocket::Recv(char *buffer, int iLen, char cTail, int &iRecvedLen)
{
	int ret = -1;
//	char tmpchar = {0};
	int tmpdatalen = 0;//��ʱ������������յ����ݳ��ȡ�
	int tmpbuflen = 0;//ʵ��buffer��С
	int tmprecvlen = 0;//���յ����ݳ���
	int i = 0;
	int ifind = 0;
	int tmplen = 0;

	iRecvedLen = 0;

//	if (iLen < m_iTmpBufferLen) return SOCKET_PARAR_ERR;//���ջ�����̫С//del by xiaozhengxiu 20130703,����С����������
/*
	//����һ��һ��һ���ַ���ȡ
	while(((ret = OneRecv(&tmpchar, 1, tmprecvlen)) == 0)&&(iLen > m_iTmpBufferLen))
	{
		if (0 != ret) return ret;
		
	  memcpy(m_TmpBuffer+m_iTmpBufferLen, &tmpchar, tmprecvlen);
  	m_iTmpBufferLen = m_iTmpBufferLen + tmprecvlen;
  	
  	if (0 == memcmp(&tmpchar,&cTail,1))break;
  }
	//�����������ݷ���
	iRecvedLen = m_iTmpBufferLen;
	memcpy(buffer, m_TmpBuffer, m_iTmpBufferLen);
	//�����ʱ������
	memset(m_TmpBuffer, 0, m_iTmpBufferLen);
	m_iTmpBufferLen = 0;
*/
	//��������һ�ζ�ȡ
  tmplen = iLen;
  memset(buffer,0,iLen);
  memset(m_TmpExBuffer,0,m_iTmpExBufferLen);
  
	//20130722 by xiaozhengxiu add 
	//�Ȳ���m_TmpBuffer�б��������
	if (m_iTmpDataBufferLen>0)
	{
			for(i=0;i<m_iTmpDataBufferLen;i++)
			{
		  	if (0 == memcmp(&*(m_TmpBuffer+i),&cTail,1))
		  	{
					//�����������ݷ���
					iRecvedLen = i+1;
					memcpy(buffer, m_TmpBuffer, i+1);
					
					//�޸�ʣ������ݴ洢����ʱ������
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
	  tmpdatalen = m_iTmpBufferLen-m_iTmpDataBufferLen;//���ջ����������ٽ��յ����ݳ���
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
			return ret;//20130722 by xiaozhengxiu del ����ʧ�ܣ���m_TmpBuffer�п��ܻ�������
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

	//�����������ݷ���
	iRecvedLen = m_iTmpDataBufferLen;
	memcpy(buffer, m_TmpBuffer, m_iTmpDataBufferLen);

	//��ʣ������ݴ洢����ʱ������
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

	// ˵���������� strTail Ϊ������־�������ݡ� 
	// ��ַ:��buffer + iLen)����Ч���ɵ��÷���֤��
	//����ֵ�����ճɹ�����0 ����������ش����롣
	//����:buffer :[in]��*buffer��[out] ��Ҫ�������ݵĻ�������ַ��
	//iLen [in]:���ջ������ĳ��ȡ�
	//strTail [in]:������־���� �м���� '\0' Ҳ���Բ��� '\0' ��β�� 
	//iTailLen [in]: "strTail"�ĳ��ȡ�
	//iRecvedLen: [out] ʵ�ʽ��յ������ݳ��ȡ�	
int C_TcpSocket::Recv(char *buffer, int iLen, char* strTail, int iTailLen, int &iRecvedLen)
{
	int ret = 0;
//	char tmpchar = {0};
	int tmpdatalen = 0;//��ʱ������������յ����ݳ��ȡ�
	int tmpbuflen = 0;//ʵ��buffer��С
	int tmprecvlen = 0;//���յ����ݳ���
	int i = 0;
	int ifind = 0;
	int tmplen = 0;
	
	iRecvedLen = 0;
	
//	if (iLen < m_iTmpBufferLen) return SOCKET_PARAR_ERR;//���ջ�����̫С//del by xiaozhengxiu 20130703,����С����������

/*	//����һ��һ��һ���ַ���ȡ
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
	//�����������ݷ���
	iRecvedLen = m_iTmpBufferLen;
	memcpy(buffer, m_TmpBuffer, m_iTmpBufferLen);
	//�����ʱ������
	memset(m_TmpBuffer, 0, m_iTmpBufferLen);
	m_iTmpBufferLen = 0;
*/
	//��������һ�ζ�ȡ
  tmplen = iLen;
  memset(buffer,0,iLen);
  memset(m_TmpExBuffer,0,m_iTmpExBufferLen);
  
	//20130722 by xiaozhengxiu add 
	//�Ȳ���m_TmpBuffer�б��������
	if (m_iTmpDataBufferLen>0)
	{
			for(i=0;i<m_iTmpDataBufferLen;i++)
			{
		  	if (0 == memcmp(&*(m_TmpBuffer+i),strTail,iTailLen))
		  	{
					//�����������ݷ���
					iRecvedLen = i+iTailLen;
					memcpy(buffer, m_TmpBuffer, i+iTailLen);
					
					//�޸�ʣ������ݴ洢����ʱ������
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
		//������յ����ݳ���
	  tmpdatalen = m_iTmpBufferLen-m_iTmpDataBufferLen;//���ջ����������ٽ��յ����ݳ���
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

	//�����������ݷ���
	iRecvedLen = m_iTmpDataBufferLen;
	memcpy(buffer, m_TmpBuffer, m_iTmpDataBufferLen);

	//��ʣ������ݴ洢����ʱ������
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

	//˵���������� pTail->strTail Ϊ���������־�������ݡ��˺�����������մ���
	//�Ƿ������ pTail->strTail ָ�������־����ֻҪ���κ�һ����⵽����һ����־������Ϊ���ճɹ���
	//��ַ:��buffer + iLen)����Ч���ɵ��÷���֤��
	//����ֵ�����ճɹ�����0 ����������ش����롣
	//����:buffer :[in]��*buffer��[out] ��Ҫ�������ݵĻ�������ַ��
	//iLen [in]:���ջ������ĳ��ȡ�
	//pTail [in]: TcpTail����Ԫ�ص���ָ�롣
	//iTailSize: "pTail" ��Ӧ�����Ԫ�ظ�����
	//iRecvedLen: [out] ʵ�ʽ��յ������ݳ��ȡ�
	//iTailPos:������־���ڡ�pTail����ʶ�������е�λ�á�
int C_TcpSocket::Recv(char *buffer, int iLen, TcpTail* pTail, int iTailSize, int &iRecvedLen, int &iTailPos)
{
	int ret = 0;
	int tmpdatalen = 0;//��ʱ������������յ����ݳ��ȡ�
	int tmpbuflen = 0;//ʵ��buffer��С
	int tmprecvlen = 0;//���յ����ݳ���
	int ExDatarecvlen = 0;//�ۼƽ��յ����ݳ���
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
	//�Ȳ���m_TmpBuffer�б��������
	if (m_iTmpDataBufferLen>0)
	{
		for(j=0;j<iTailSize;j++)
		{
			for(i=0;i<m_iTmpDataBufferLen;i++)
			{
		  	if (0 == memcmp(&*(m_TmpBuffer+i),pTail->strTail,pTail->iTailLen))
		  	{
					//�����������ݷ���
		  		memcpy(buffer,m_TmpBuffer,i+(pTail->iTailLen));
			  	m_iTmpDataBufferLen = m_iTmpDataBufferLen + i + (pTail->iTailLen);
					iTailPos = j;

					//�޸�ʣ������ݴ洢����ʱ������ memset(m_TmpExBuffer, 0, tmpbuflen); 
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
		//������յ����ݳ���
	  tmpdatalen = m_iTmpBufferLen-m_iTmpDataBufferLen-ExDatarecvlen;//���ջ����������ٽ��յ����ݳ���
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

	//�����������ݷ���
	iRecvedLen = m_iTmpDataBufferLen;
	memcpy(buffer, m_TmpBuffer, m_iTmpDataBufferLen);
	iTailPos = j;

	//��ʣ������ݴ洢����ʱ������
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

	//˵�����ر�tcp���ӡ�
	//����ֵ���Ͽ����ӳɹ����� 0���Ͽ�����ʧ�� -1��
	//strIp��
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

	//����ͨ���������Ӧ��ģ�飬����ģ�飬�Ա㷢������ʱ�����ָ������־��
	//����ֵ����
	//iModule:[in]ģ���� iSubModule:[in]��ģ���š�
void C_TcpSocket::SetLog(int iModule, int iSubModule)
{
	m_iLogModuleNum    = iModule;
	m_iLogSubModuleNum = iSubModule;
	return ;
}

//==========================private===================================//

/****FUNCTION***************************************************
* DESCRIPTION : �����������������, ������ģʽconnect����
*       INPUT : sockfd      �׽Կ�������                
*		        nsec        ��ʱʱ������
*     RETURNS :  
*    CAUTIONS : nsec>0 connect ����nsecʱ��������ڴ�ʱ��������Ӳ��ɹ�����
				nsec=0 connect ��������������Ӳ��ɹ���������
				nsec<0 connect ����������Ϊ����������connect
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
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);   //����Ϊ������ģʽ
#endif

	if((n = connect(sockfd, saptr, salen)) < 0)   //����ʧ��
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
		
        //�������ӳ�ʱ����
		if(0 == (n = select(sockfd+1, &rset, &wset, NULL, (nsec < 0)?NULL:&tval)))
		{
#ifdef WIN32
			closesocket(sockfd); 
#else
			close(sockfd);
			errno = ETIMEDOUT;
#endif
			//���ӳ�ʱ
			
			CreateLog(CLIENT_SOCK_CONNECT_TIMEOUT, iResult);
			return iResult;
			//return CLIENT_SOCK_CONNECT_TIMEOUT;
		}
        
        //����д����
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
	fcntl(sockfd, F_SETFL, flags);   //�ָ��׽ӿ�ԭʼ����
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

	//˵����һ�ν��ա����ս������ buffer�С�
	//����ֵ���ɹ����յ����ݷ���0������������ش����롣
	//        ��ַ��buffer + iLen)����Ч���ɵ��÷���֤��
	//������buffer :[in]��*buffer��[out] ��Ҫ�������ݵĻ�������ַ��
	//      iLen:[in]:��Ҫ���յ����ݳ��ȡ�
	//      iRecvedLen: [out] ʵ�ʽ��յ������ݳ��ȡ�
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
    
    //����ʱ������
	if(m_iTimeout < 0)
		flag = select(m_iSocketId + 1, &fSet, NULL, NULL, NULL);
	else
	{
		timeout.tv_sec = m_iTimeout;
		timeout.tv_usec = 0;
		flag = select(m_iSocketId + 1, &fSet, NULL, NULL, &timeout);
	}

    //��ʧ��
	if(flag < 0)	
	{	
		CreateLog(CONN_READ_SELECT_ERR, iResult);
		return iResult;				
		//return CONN_READ_SELECT_ERR;
	}	
	//����ʱ		
	if(0 == flag)	
	{	
		CreateLog(CONN_READ_TIMEOUT, iResult);
		return iResult;			
		//return CONN_READ_TIMEOUT;
	}
    
    //������
#ifdef WIN32
	readnum = recv(m_iSocketId, buffer, iLen, 0);
#else
	readnum = recv(m_iSocketId, (void*)buffer, iLen, 0);
#endif
	if(-1 == readnum)
	{
	    //��ʧ��
		iRecvedLen = 0;
		CreateLog(CONN_READ_RECV_FAIL, iResult);
		return iResult;
		//return CONN_READ_RECV_FAIL;
	}
	
	if(0 == readnum )  //�Է����ӹر�
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
				str = "Windows��������";
			}
			break;
			
			case SOCKET_NULL_FD:
			{
				str = "�յ�socket�ļ���������";
			}
			break;
			
			case SOCKET_CALL_SYSAPI_ERR:
			{
				str = "����ϵͳAPI����";
			}
			break;
			
			case SERVER_SOCK_INI_ERR:
			{
				str = "server��socket��ʼ������";
			}
			break;
			
			case SERVER_SOCK_BIND_ERR:
			{
				str = "server��bind����";
			}
			break;
			
			case SERVER_SOCK_LISTEN_ERR:
			{
				str = "server��listen����";
			}
			break;
			
			case SERVER_SOCK_ACCEPT_ERR:
			{
				str = "server��accept����";
			}
			break;	
			
			case CLIENT_SOCK_INI_ERR:
			{
				str = "client��socket��ʼ������";
			}
			break;	
			
			case CLIENT_SOCK_BIND_ERR:
			{
				str = "client��bind����";
			}
			break;																					

			case CLIENT_SOCK_CONNECT_ERR:
			{
				str = "client��connect����";
			}
			break;	
			
			case CLIENT_SOCK_CONNECT_TIMEOUT:
			{
				str = "client��connect��ʱ";
			}
			break;	
			
			case CONN_READ_PARA_ERR:
			{
				str = "���Ӷ����������������";
			}
			break;
			
			case CONN_READ_CALL_ERR:
			{
				str = "���Ӷ��������ô���";
			}
			break;
			
			case CONN_READ_SELECT_ERR:
			{
				str = "���Ӷ�����select����";
			}
			break;
				
			case CONN_READ_TIMEOUT:
			{
				str = "���Ӷ�������ʱ";
			}
			break;	
			
			case CONN_READ_RECV_FAIL:
			{
				str = "���Ӷ�����recv����";
			}
			break;	
			
			case CONN_WRITE_PARA_ERR:
			{
				str = "����д���������������";
			}
			break;	
			
			case CONN_WRITE_CALL_ERR:
			{
				str = "����д�������ô���";
			}
			break;
			
			case CONN_WRITE_SELECT_ERR:
			{
				str = "����д����elect����";
			}
			break;	
			
			case CONN_WRITE_TIMEOUT:
			{
				str = "����д������ʱ";
			}
			break;	
			
			case CONN_WRITE_SEND_FAIL:
			{
				str = "����д����send����";
			}
			break;	
			
			case CONN_PEER_TERMINATOR:
			{
				str = "���ӶԶ˹ر�";
			}
			break;	
			
			default:
			{
				str = "δ֪�Ĵ���";
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
