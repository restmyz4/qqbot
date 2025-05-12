#include <WinSock2.h>
#include <format>
#include "SourceLog.h"
#pragma comment(lib, "ws2_32.lib")

namespace SourceLog
{
	Sender::Sender(uint64_t fromQQ, const std::string &nickname)
	{
		this->fromQQ = fromQQ;
		this->nickname = nickname;
	}

	Sender::~Sender(void)
	{
	}

	std::string Sender::send(const std::string &msg)
	{
		return std::format("sm_ch_srv {0}{1}<{2}>: {3}", token, nickname, fromQQ, msg);
	}

	Reciever::Reciever(const char *ip, int port)
	{
		sin.sin_family = AF_INET;
		sin.sin_addr.S_un.S_addr = inet_addr(ip);
		sin.sin_port = htons(port);
	}

	Reciever::~Reciever(void)
	{
		if (sock != INVALID_SOCKET)
			closesocket(sock);
		if (wsaStartup == NO_ERROR)
			WSACleanup();
	}

	bool Reciever::bind(void)
	{
		if (wsaStartup != NO_ERROR || sock == INVALID_SOCKET)
			return false;
		if (!Helper::Socket::success(1, ::bind(sock, (SOCKADDR *)&sin, sizeof(sin))))
			return false;
		isBind = true;
		return true;
	}

	bool Reciever::recv(uint64_t *fromGroup, std::string *msg)
	{
		if (!isBind)
			return false;
		*fromGroup = 0;
		*msg = "";
		char data[1024] = { 0 };
		int len = sizeof(sin);
		if (!Helper::Socket::success(3, recvfrom(sock, data, sizeof(data), 0, (SOCKADDR *)&sin, &len)))
			return false;
		const std::vector<std::string> &dataSplit = Helper::String::split(data, ' ');
		if (dataSplit[0] == token)
		{
			*fromGroup = std::stoull(dataSplit[1]);
			size_t headerLength = token.length() + dataSplit[1].length();
			if (strlen(data) - headerLength > 1)
				*msg = Helper::String::lstrip(std::string(data).substr(headerLength + 1), ' ');
			return true;
		}
		return false;
	}
}