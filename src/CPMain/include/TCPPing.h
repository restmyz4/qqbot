#pragma once
#include <string>
#include <WinSock2.h>

class TCPPing
{
private:
	WSADATA wsaData;
	int wsaStartup = WSAStartup(WINSOCK_VERSION, &wsaData);
	sockaddr_in sin{ 0 };
public:
	TCPPing(const char *ip, int port);
	~TCPPing(void);
	std::string ping(void);
	std::string make(clock_t ping);
};