//@file:C_TcpTransport.cpp
//@brief:封装TCP通信程序
//@author: duheqing@oristartech.com
//date: 2012-6-5

#include "C_TcpTransport.h"

TcpTransport::TcpTransport():fd(-1)
{}

TcpTransport::~TcpTransport()
{
	if(fd != -1)
	{
		close(fd);
		fd = -1;
	}
}

int TcpTransport::TcpConnect(const char *ip, unsigned short port, int wait)
{
	if(ip == NULL || port == 0)
		return -1;

	if(fd != -1)
	{
		close(fd);
		fd = -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	int iTmp = socket(AF_INET, SOCK_STREAM, 0);

	timeval timeo;
	timeo.tv_sec = wait;
	timeo.tv_usec = 0;
	setsockopt(iTmp, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));

	int result = connect(iTmp, (sockaddr*)&addr, sizeof(sockaddr));
	if(result < 0)
	{
		close(iTmp);
		fd = -1;
	}
	else
	{
		fd = iTmp;
	}
	return result;
}

bool TcpTransport::BeConnected()const
{
	return (fd == -1) ? false : true;
}

int TcpTransport::BlockSend(const char *buffer, int len)
{
	if(buffer == NULL)
		return -1;
	if(fd == -1)
		return -1;

	int result = send(fd, buffer, len, MSG_NOSIGNAL);
	if(result < 0)
	{
		close(fd);
		fd = -1;
	}
	return result;
}

int TcpTransport::SelectRecv(char *buffer, int len, timeval &timeOut)
{
	if(buffer == NULL)
		return -1;
	if(fd == -1)
		return -1;

	fd_set fdSet;
	FD_ZERO(&fdSet);
	FD_SET(fd, &fdSet);

	int result = select(fd+1, &fdSet, NULL, NULL, &timeOut);
	if(result < 0)
	{
		close(fd);
		fd = -1;
		return result;
	}
	if(result == 0)
		return 0;

	result = recv(fd, buffer, len, 0);
	//wzp modify on 2013-5-19
	//if(result < 0)
	if(result <= 0)
	//end.
	{
		close(fd);
		fd = -1;
	}
	return result;
}

int TcpTransport::SelectRecv(char *buffer, int len)
{
	timeval timeOut;
	timeOut.tv_sec = 0;
	timeOut.tv_usec = 0;
	return SelectRecv(buffer, len, timeOut);
}

int TcpTransport::BlockRecv(char *buffer, int len)
{
	int result = recv(fd, buffer, len, 0);
	if(result < 0)
	{
		close(fd);
		fd = -1;
	}
	return result;
}

void TcpTransport::ReleaseConnect()
{
	close(fd);
	fd = -1;
}

int GetThisAllIp(std::vector<std::string> &ipList)
{
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		return 0;

	struct ifreq ifq[16];
	struct ifconf ifc;

	ifc.ifc_len = sizeof(ifq);
	ifc.ifc_buf = (caddr_t)ifq;

	if(ioctl(fd, SIOCGIFCONF, (char *)&ifc))
		return 0;
	int num = ifc.ifc_len / sizeof(struct ifreq);

	if(ioctl(fd, SIOCGIFADDR, (char *)&ifq[num-1]))
		return 0;
	close(fd);

	for(int i=0; i<num; i++)
	{
		char *tmpIp = inet_ntoa(((struct sockaddr_in*)(&ifq[i].ifr_addr))->sin_addr);
		if(strcmp(tmpIp, "127.0.0.1"))
			ipList.push_back(tmpIp);
	}
	return 0;
}

