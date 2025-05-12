#pragma once
#include <string>
#include <WinSock2.h>
#include "Helper.h"

namespace SourceLog
{
	static inline const std::string &token = "自己填token";

	class Sender
	{
	private:
		unsigned long long fromQQ;
		std::string nickname;
	public:
		Sender(uint64_t fromQQ, const std::string &nickname);
		~Sender(void);
		std::string send(const std::string &msg);
	};
	class Reciever
	{
	private:
		WSADATA wsaData;
		int wsaStartup = WSAStartup(WINSOCK_VERSION, &wsaData);
		SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		sockaddr_in sin = { 0 };
		bool isBind = false;
	public:
		Reciever(const char *ip, int port);
		~Reciever(void);
		bool bind(void);
		bool recv(uint64_t *fromGroup, std::string *msg);
	};
};