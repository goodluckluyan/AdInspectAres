//@file:C_TcpTransport.h
//@brief:封装TCP通信程序
//@author: duheqing@oristartech.com
//date: 2012-6-5
#ifndef _H_TCPTRANSPORT_
#define _H_TCPTRANSPORT_

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <vector>
#include <string>
#include <string.h>
class TcpTransport
{
private:
	int fd;
	sockaddr_in addr;

public:
	TcpTransport();
	~TcpTransport();

	//success return 0, failed return -1;
	int TcpConnect(const char *ip, unsigned short port, int wait = 2);

	bool BeConnected()const;

	//return amount of send bytes
	int BlockSend(const char *buffer, int len);

	//return amount of recv bytes
	int SelectRecv(char *buffer, int len, timeval &timeOut);
	int SelectRecv(char *buffer, int len);//timeOut.tv_sec=0;timeOut.tv_usec=0;
	int BlockRecv(char *buffer, int len);

	void ReleaseConnect();
};

int GetThisAllIp(std::vector<std::string> &ipList);

#endif//_H_TCPTRANSPORT_
