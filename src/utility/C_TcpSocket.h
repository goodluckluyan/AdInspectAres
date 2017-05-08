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
	//iTmpBufferLen��ָ����������ʱ����ĳ��ȣ�ͬʱ��m_TmpBuffer ����ռ䡣
	C_TcpSocket(int iTmpBufferLen);
	~C_TcpSocket();
	
	//˵��������tcp���ӡ�
	//����ֵ�����ӳɹ����� 0������ʧ�� -1��
	//strIp��[in] ��usPort [in], iWait [in];
	int TcpConnect(const char *strIp, unsigned short usPort);
	
	
	void GetErrorInfo(std::string &strError);
	//˵�������ص�ǰ������״̬��
	//����ֵ���Ѿ����ӷ��� 1 δ���ӷ��� 0��
	bool BeConnected()const
	{
		return (m_iSocketId == -1) ? false : true;
	}
	
	//˵����һ�η��͡�
	//����ֵ����buffer��ʼ iLen ���� ������ȫ��������� ����������ش����롣
	//������ 
	//buffer :[in]��*buffer��[out] ��Ҫ�������ݵĻ�������ַ��
	//iLen: [in] ��Ҫ�������ݵĳ��ȡ�
	//iSendedLen:[out] �Ѿ��������ݵĳ��ȡ�
	int OneSend(const char *buffer, int iLen, int &iSendedLen);

	//˵��������ָ�����ȵ����ݡ�
	//����ֵ����buffer��ʼ iLen ���� ������ȫ��������� ����0������������ش����롣
	//������ 
	//buffer :[in]��*buffer��[out] ��Ҫ�������ݵĻ�������ַ��
	//iLen: [in] ��Ҫ�������ݵĳ��ȡ�
	//iSendedLen:[out] �Ѿ��������ݵĳ��ȡ�����ֵΪ0ʱ iLen == iSendedLen
	int Send(const char *buffer, int iLen, int &iSendedLen);
	
	//˵����һ�ν��ա����ս������ buffer�С�
	//����ֵ���ɹ����յ����ݷ���0������������ش����롣
	//        ��ַ��buffer + iLen)����Ч���ɵ��÷���֤��
	//������buffer :[in]��*buffer��[out] ��Ҫ�������ݵĻ�������ַ��
	//      iLen:[in]:��Ҫ���յ����ݳ��ȡ�
	//      iRecvedLen: [out] ʵ�ʽ��յ������ݳ��ȡ�
	int OneRecv(char *buffer, int iLen, int &iRecvedLen);

	//˵���������� iLenָ�����ȵ����ݡ����ս������ buffer�С�
	//����ֵ���ɹ����յ�iLen���ȵ����ݷ���0������������ش����롣
	//        ��ַ��buffer + iLen)����Ч���ɵ��÷���֤��
	//������buffer :[in]��*buffer��[out] ��Ҫ�������ݵĻ�������ַ��
	//      iLen:[in]:��Ҫ���յ����ݳ��ȡ�
	//      iRecvedLen: [out] ʵ�ʽ��յ������ݳ��ȡ�
	int Recv(char *buffer, int iLen, int &iRecvedLen);
	
	// ˵���������� cTail Ϊ������־�������ݡ� �������ݺ�������ҽ��մ�����������־����ͬ��
	// ��ַ:��buffer + iLen)����Ч���ɵ��÷���֤��
	//����ֵ�����ճɹ�����0 ����������ش����롣
	//����:buffer :[in]��*buffer��[out] ��Ҫ�������ݵĻ�������ַ��
	//iLen:���ջ������ĳ��ȡ�
	//cTail:������־�ַ���
	//iRecvedLen: [out] ʵ�ʽ��յ������ݳ��ȡ�
	int Recv(char *buffer, int iLen, char cTail, int &iRecvedLen);
	
	// ˵���������� strTail Ϊ������־�������ݡ� 
	// ��ַ:��buffer + iLen)����Ч���ɵ��÷���֤��
	//����ֵ�����ճɹ�����0 ����������ش����롣
	//����:buffer :[in]��*buffer��[out] ��Ҫ�������ݵĻ�������ַ��
	//iLen [in]:���ջ������ĳ��ȡ�
	//strTail [in]:������־���� �м���� '\0' Ҳ���Բ��� '\0' ��β�� 
	//iTailLen [in]: "strTail"�ĳ��ȡ�
	//iRecvedLen: [out] ʵ�ʽ��յ������ݳ��ȡ�	
	int Recv(char *buffer, int iLen, char* strTail, int iTailLen, int &iRecvedLen);
	
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
	int Recv(char *buffer, int iLen, TcpTail* pTail, int iTailSize, int &iRecvedLen, int &iTailPos);
	
	//˵�����ر�tcp���ӡ�
	//����ֵ���Ͽ����ӳɹ����� 0���Ͽ�����ʧ�� -1��
	//strIp��
	int TcpDisConnect();
	
	//����ͨ���������Ӧ��ģ�飬����ģ�飬�Ա㷢������ʱ�����ָ������־��
	//����ֵ����
	//iModule:[in]ģ���� iSubModule:[in]��ģ���š�
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
  //�׽���������
	int m_iSocketId; 
  //ͨ�ŵĳ�ʱʱ��
  int m_iTimeout;
  //��ʱ���泤�ȡ�
  int m_iTmpBufferLen;
  //��ʱ�������ݳ��ȡ�
  int m_iTmpDataBufferLen;//add by xiaozhengxiu 20130703
  //��ʱ�����ַ��
  char * m_TmpBuffer;
  //�������������ݳ���,����buffer���ȵ�ͬm_iTmpDataBufferLen
  int m_iTmpExBufferLen;
  //��������������СΪm_iTmpBufferLen
  char * m_TmpExBuffer;
  //һ�η���������ݳ���
  int m_iOneSendMaxLen;
  char m_strIp[16];
	
//	bool m_isConn;//����״̬
/****FUNCTION***************************************************
* DESCRIPTION : �����������������, ������ģʽconnect����
*       INPUT : sockfd      �׽Կ�������                
*		        nsec        ��ʱʱ������
*     RETURNS : �μ��������ļ�socket_err.h 
*    CAUTIONS : time_out>0 connect ����time_outʱ��������ڴ�ʱ��������Ӳ��ɹ�����
				time_out=0 connect ��������������Ӳ��ɹ���������
				time_out<0 connect ����������Ϊ����������connect
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


