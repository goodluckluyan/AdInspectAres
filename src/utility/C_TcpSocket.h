//filename:C_TcpSocket.h
#ifndef TCP_SOCKET_H_
#define TCP_SOCKET_H_
#include <string>
#ifndef WIN32 
	#include <sys/socket.h> 
#endif

#ifdef WIN32
	#define    SOKLEN    int
#else
	#define    SOKLEN    socklen_t
#endif

typedef struct TcpTail
{
	char* strTail;
	int iTailLen;
}TcpTail;

class C_TcpSocket
{
public:
	//iTmpBufferLen：指定对象内临时缓存的长度，同时对m_TmpBuffer 分配空间。
	C_TcpSocket(int iTmpBufferLen);
	~C_TcpSocket();
	
	//说明：建立tcp连接。
	//返回值：连接成功返回 0，连接失败 -1；
	//strIp：[in] ，usPort [in], iWait [in];
	int TcpConnect(const char *strIp, unsigned short usPort);
	
	
	void GetErrorInfo(std::string &strError);
	//说明：返回当前的连接状态。
	//返回值：已经连接返回 1 未连接返回 0；
	bool BeConnected()const
	{
		return (m_iSocketId == -1) ? false : true;
	}
	
	//说明：一次发送。
	//返回值：以buffer开始 iLen 长度 的数据全部发送完成 其他情况返回错误码。
	//参数： 
	//buffer :[in]（*buffer）[out] 将要发送数据的缓冲区地址。
	//iLen: [in] 将要发送数据的长度。
	//iSendedLen:[out] 已经发送数据的长度。
	int OneSend(const char *buffer, int iLen, int &iSendedLen);

	//说明：发送指定长度的数据。
	//返回值：以buffer开始 iLen 长度 的数据全部发送完成 返回0，其他情况返回错误码。
	//参数： 
	//buffer :[in]（*buffer）[out] 将要发送数据的缓冲区地址。
	//iLen: [in] 将要发送数据的长度。
	//iSendedLen:[out] 已经发送数据的长度。返回值为0时 iLen == iSendedLen
	int Send(const char *buffer, int iLen, int &iSendedLen);
	
	//说明：一次接收。接收结果存入 buffer中。
	//返回值：成功接收到数据返回0，其他情况返回错误码。
	//        地址（buffer + iLen)的有效性由调用方保证。
	//参数：buffer :[in]（*buffer）[out] 将要接收数据的缓冲区地址。
	//      iLen:[in]:想要接收的数据长度。
	//      iRecvedLen: [out] 实际接收到的数据长度。
	int OneRecv(char *buffer, int iLen, int &iRecvedLen);

	//说明：接收由 iLen指定长度的数据。接收结果存入 buffer中。
	//返回值：成功接收到iLen长度的数据返回0，其他情况返回错误码。
	//        地址（buffer + iLen)的有效性由调用方保证。
	//参数：buffer :[in]（*buffer）[out] 将要接收数据的缓冲区地址。
	//      iLen:[in]:想要接收的数据长度。
	//      iRecvedLen: [out] 实际接收到的数据长度。
	int Recv(char *buffer, int iLen, int &iRecvedLen);
	
	// 说明：接收由 cTail 为结束标志符的数据。 接收数据后正向查找接收串检索结束标志（下同）
	// 地址:（buffer + iLen)的有效性由调用方保证。
	//返回值：接收成功返回0 其他情况返回错误码。
	//参数:buffer :[in]（*buffer）[out] 将要接收数据的缓冲区地址。
	//iLen:接收缓冲区的长度。
	//cTail:结束标志字符。
	//iRecvedLen: [out] 实际接收到的数据长度。
	int Recv(char *buffer, int iLen, char cTail, int &iRecvedLen);
	
	// 说明：接收由 strTail 为结束标志串的数据。 
	// 地址:（buffer + iLen)的有效性由调用方保证。
	//返回值：接收成功返回0 其他情况返回错误码。
	//参数:buffer :[in]（*buffer）[out] 将要接收数据的缓冲区地址。
	//iLen [in]:接收缓冲区的长度。
	//strTail [in]:结束标志串。 中间可以 '\0' 也可以不以 '\0' 结尾。 
	//iTailLen [in]: "strTail"的长度。
	//iRecvedLen: [out] 实际接收到的数据长度。	
	int Recv(char *buffer, int iLen, char* strTail, int iTailLen, int &iRecvedLen);
	
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
	int Recv(char *buffer, int iLen, TcpTail* pTail, int iTailSize, int &iRecvedLen, int &iTailPos);
	
	//说明：关闭tcp连接。
	//返回值：断开连接成功返回 0，断开连接失败 -1；
	//strIp：
	int TcpDisConnect();
	
	//设置通信类自身对应的模块，和子模块，以便发生错误时输出到指定的日志。
	//返回值：无
	//iModule:[in]模块编号 iSubModule:[in]子模块编号。
	void SetLog(int iModule, int iSubModule);
	
	int GetTimeout()
	{
		return m_iTimeout;
	}
	void SetTimeout(int iTimeout)
	{
		m_iTimeout = iTimeout;
	}
	
private:
  //套接字描述符
	int m_iSocketId; 
  //通信的超时时间
  int m_iTimeout;
  //临时缓存长度。
  int m_iTmpBufferLen;
  //临时缓存数据长度。
  int m_iTmpDataBufferLen;//add by xiaozhengxiu 20130703
  //临时缓存地址。
  char * m_TmpBuffer;
  //交换数据区数据长度,整个buffer长度等同m_iTmpDataBufferLen
  int m_iTmpExBufferLen;
  //交换数据区，大小为m_iTmpBufferLen
  char * m_TmpExBuffer;
  //一次发送最大数据长度
  int m_iOneSendMaxLen;
  char m_strIp[16];
	
//	bool m_isConn;//连接状态
/****FUNCTION***************************************************
* DESCRIPTION : 建立与服务器端连接, 非阻塞模式connect函数
*       INPUT : sockfd      套皆口描述符                
*		        nsec        超时时间设置
*     RETURNS : 参见错误码文件socket_err.h 
*    CAUTIONS : time_out>0 connect 阻塞time_out时长，如果在此时间段内连接不成功返回
				time_out=0 connect 不阻塞，如果连接不成功立即返回
				time_out<0 connect 阻塞，作用为常规阻塞型connect
****************************************************************/
	int connect_nonb(int                    sockfd, 
							   const struct sockaddr *saptr, 
							   SOKLEN                 salen, 
							   int                    nsec);
							   
	//wzp on 2013-09-22 add
	int CreateLog(int iLogNum, int &iReturnErrorNum);	
	int m_iLogModuleNum;
	int m_iLogSubModuleNum;
	std::string m_strError;			   
};
#endif


