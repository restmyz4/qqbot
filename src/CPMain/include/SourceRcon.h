#pragma once
#include <string>
#include <WinSock2.h>

class SourceRcon
{
private:
	enum SourceServer : int
	{
		AUTH = 3,
		EXECCOMMAND = 2
	};
	const struct
	{
		const char *AUTH =
			"\xFF\xFF\xFF\xFF\x63"
			"\x68\x61\x6C\x6C\x65"
			"\x6E\x67\x65\x20\x72"
			"\x63\x6F\x6E\x10\x00";
		const char *EXECCOMMAND = "\xFF\xFF\xFF\xFF\x72\x63\x6F\x6E\x20";
	} GoldSourceServer;
	WSAData wsaData;
	int wsaStartup = WSAStartup(WINSOCK_VERSION, &wsaData);
	SOCKET sock = INVALID_SOCKET;
	int sendTimeout = 1000;
	int recvTimeout = 1000;
	sockaddr_in sin = { 0 };
	std::string password;
	std::string command;
	std::string sourceRcon();
	std::string goldsourceRcon();
	std::string err[2] = {"\n❌RCON命令受限!", "\n❌RCON执行失败!"};
public:
	SourceRcon(const char *ip, int port, const std::string &password, const std::string &command);
	~SourceRcon(void);
	std::string rcon(bool source);
	const std::string &geterr(int index);
};