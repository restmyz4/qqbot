#pragma once
#include <string>
#include <vector>
#include <WinSock2.h>

class SourceQuery
{
private:
	const char *A2S_INFO =
		"\xFF\xFF\xFF\xFF\x54"
		"\x53\x6F\x75\x72\x63"
		"\x65\x20\x45\x6E\x67"
		"\x69\x6E\x65\x20\x51"
		"\x75\x65\x72\x79\x00";
	const char *A2S_PLAYER_CHALLENGE = "\xFF\xFF\xFF\xFF\x55\xFF\xFF\xFF\xFF";
	WSADATA wsaData;
	int wsaStartup = WSAStartup(WINSOCK_VERSION, &wsaData);
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sockaddr_in sin = { 0 };
	std::string ipDraw;
	const std::string &err = "\n❌服务器信息获取失败!";
	std::string calcDuration(float duration);
public:
	SourceQuery(const char *ip, int port, const std::string &ipDraw = "");
	~SourceQuery(void);
	std::string query(int sendTimeout = 1000, int recvTimeout = 1000,
		int maxRetry = 3, const char *errMsg = "", const std::vector<int> &valueIndex = {});
};