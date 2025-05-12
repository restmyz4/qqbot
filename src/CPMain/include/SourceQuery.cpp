#include <memory>
#include <format>
#include <algorithm>
#include "SourceQuery.h"
#include "Helper.h"
#include "parsemsg.h"
#pragma comment(lib, "ws2_32.lib")

SourceQuery::SourceQuery(const char *ip, int port, const std::string &ipDraw)
{
	sin.sin_family = AF_INET;
	sin.sin_addr.S_un.S_addr = inet_addr(ip);
	sin.sin_port = htons(port);
	this->ipDraw = ipDraw;
}

SourceQuery::~SourceQuery(void)
{
	if (sock != INVALID_SOCKET)
		closesocket(sock);
	if (wsaStartup == NO_ERROR)
		WSACleanup();
}

std::string SourceQuery::calcDuration(float duration)
{
	int h = (int)duration / 3600;
	int m = ((int)duration % 3600) / 60;
	int s = (int)duration % 60;
	const std::string &h_str = h > 9 ? std::to_string(h) : std::format("0{0}", h);
	const std::string &m_str = m > 9 ? std::to_string(m) : std::format("0{0}", m);
	const std::string &s_str = s > 9 ? std::to_string(s) : std::format("0{0}", s);
	return std::format("{0}:{1}:{2}", h_str, m_str, s_str);
}

std::string SourceQuery::query(int sendTimeout, int recvTimeout,
	int maxRetry, const char *errMsg, const std::vector<int> &valueIndex)
{
	if (wsaStartup != NO_ERROR || sock == INVALID_SOCKET)
		return errMsg == "" ? "WSA or SOCKET startup failed" : errMsg;
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&sendTimeout, sizeof(int));
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&recvTimeout, sizeof(int));
	int len = sizeof(sin);
	int bufferSize = 1024;
	std::unique_ptr<char> buffer(new char[bufferSize] { 0 });
	for (int i = 0; i <= maxRetry; i++)
	{
		if (Helper::Socket::success(2, sendto(sock, A2S_INFO, (int)strlen(A2S_INFO) + 1, 0, (SOCKADDR *)&sin, sizeof(sin))))
			if (Helper::Socket::success(3, recvfrom(sock, buffer.get(), bufferSize, 0, (SOCKADDR *)&sin, &len)))
				break;
		if (maxRetry == 0 || i == maxRetry - 1)
			return errMsg == "" ? err : errMsg;
	}
	BufferReader br(buffer.get(), bufferSize);
	br.read<int32_t>();
	uint8_t header = br.read<uint8_t>();
	if (header != 'I')
	{
		const size_t packetSize = strlen(A2S_INFO) + 1 + 4 + 1;
		std::unique_ptr<char> A2S_INFO_CHALLENGE(new char[packetSize] { 0 });
		strcpy(A2S_INFO_CHALLENGE.get(), A2S_INFO);
		for (int i = 0; i < 4; i++)
			A2S_INFO_CHALLENGE.get()[strlen(A2S_INFO) + 1 + i] = br.read<uint8_t>();
		for (int i = 0; i <= maxRetry; i++)
		{
			if (Helper::Socket::success(2, sendto(sock, A2S_INFO_CHALLENGE.get(), (int)packetSize, 0, (SOCKADDR *)&sin, sizeof(sin))))
				if (Helper::Socket::success(3, recvfrom(sock, buffer.get(), bufferSize, 0, (SOCKADDR *)&sin, &len)))
					break;
			if (maxRetry == 0 || i == maxRetry - 1)
				return errMsg == "" ? err : errMsg;
		}
		br = BufferReader(buffer.get(), bufferSize);
		br.read<int32_t>();
		br.read<uint8_t>();
	}
	std::string result;
	br.read<uint8_t>();
	const std::string &name = br.readString();
	const std::string &map = br.readString();
	const std::string &folder = br.readString();
	const std::string &game = br.readString();
	br.read<int16_t>();
	uint8_t players = br.read<uint8_t>();
	const uint8_t maxPlayers = br.read<uint8_t>();
	const uint8_t bots = br.read<uint8_t>();
	br.read<uint8_t>();
	const uint8_t environment = br.read<uint8_t>();
	const uint8_t visibility = br.read<uint8_t>();
	const uint8_t vac = br.read<uint8_t>();
	if (valueIndex.size() > 0)
	{
		for (size_t i = 0; i < valueIndex.size(); i++)
		{
			switch (valueIndex[i])
			{
				case 0:
				{
					result += name + '\0';
					break;
				}
				case 1:
				{
					result += map + '\0';
					break;
				}
				case 2:
				{
					result += folder + '\0';
					break;
				}
				case 3:
				{
					result += game + '\0';
					break;
				}
				case 4:
				{
					result += std::to_string(players) + '\0';
					break;
				}
				case 5:
				{
					result += std::to_string(maxPlayers) + '\0';
					break;
				}
				case 6:
				{
					result += std::to_string(bots) + '\0';
					break;
				}
				case 7:
				{
					result += std::to_string(environment) + '\0';
					break;
				}
				case 8:
				{
					result += std::to_string(visibility) + '\0';
					break;
				}
				case 9:
				{
					result += std::to_string(vac) + '\0';
					break;
				}
			}
		}
		return result;
	}
	result = std::format("\n名称: {0}\n地址: {1}:{2}\n地图: {3}\n游戏: {4} - {5}\n人数: {6}/{7} ({8} Bot)\n平台: {9}\n密码: {10}\nVAC: {11}\n——————————————\n",
		name, (ipDraw == "" ? inet_ntoa(sin.sin_addr) : ipDraw), ntohs(sin.sin_port), map, folder, game, players,
		maxPlayers, bots, (environment == 'l' ? "Linux" : environment == 'w' ? "Windows" : "macOS"), (visibility == 0 ? "无" : "有"),
		(vac == 1 ? "安全" : "不安全"));
	char A2S_PLAYER[10] = { 0 };
	for (int i = 0; i <= maxRetry; i++)
	{
		if (Helper::Socket::success(2, sendto(sock, A2S_PLAYER_CHALLENGE, (int)strlen(A2S_PLAYER_CHALLENGE), 0, (SOCKADDR *)&sin, sizeof(sin))))
			if (Helper::Socket::success(3, recvfrom(sock, A2S_PLAYER, sizeof(A2S_PLAYER), 0, (SOCKADDR *)&sin, &len)))
				break;
		if (maxRetry == 0 || i == maxRetry - 1)
			goto Failed;
	}
	br = BufferReader(A2S_PLAYER, sizeof(A2S_PLAYER));
	br.read<int32_t>();
	header = br.read<uint8_t>();
	if (header != 'A')
		goto Failed;
	A2S_PLAYER[4] = A2S_PLAYER_CHALLENGE[4];
	bufferSize = 4096;
	buffer.reset(new char[bufferSize] { 0 });
	for (int i = 0; i <= maxRetry; i++)
	{
		if (Helper::Socket::success(2, sendto(sock, A2S_PLAYER, sizeof(A2S_PLAYER), 0, (SOCKADDR *)&sin, sizeof(sin))))
			if (Helper::Socket::success(3, recvfrom(sock, buffer.get(), bufferSize, 0, (SOCKADDR *)&sin, &len)))
				break;
		if (maxRetry == 0 || i == maxRetry - 1)
			goto Failed;
	}
	br = BufferReader(buffer.get(), bufferSize);
	br.read<int32_t>();
	header = br.read<uint8_t>();
	if (header != 'D')
		goto Failed;
	players = br.read<uint8_t>();
	if (players)
	{
		std::vector<std::tuple<std::string, int32_t, float>> playerTuple(players);
		for (int i = 0; i < players; i++)
		{
			br.read<uint8_t>();
			const std::string &name = br.readString();
			const int32_t score = br.read<int32_t>();
			const float duration = br.read<float>();
			playerTuple.emplace_back(name, score, duration);
		}
		std::sort(playerTuple.begin(), playerTuple.end(),
			[=](const std::tuple<std::string, int32_t, float> &begin,
				const std::tuple<std::string, int32_t, float> &end)
		{
			return std::get<1>(begin) > std::get<1>(end);
		});
		bool valid = false;
		for (int i = 0; i < playerTuple.size(); i++)
			if (!(std::get<0>(playerTuple[i]).length() == 0 && std::get<1>(playerTuple[i]) == 0))
			{
				if (!valid)
				{
					result += "名称:\t得分:\t时长:\n";
					valid = true;
				}
				result += std::get<0>(playerTuple[i]) + "\t" + std::to_string(std::get<1>(playerTuple[i])) + "\t" + calcDuration((std::get<2>(playerTuple[i]))) + "\n";
			}
		if (!valid)
			result += "暂无玩家在线...";
	}
	else
		result += "暂无玩家在线...";
	Helper::String::fixUtf8(Helper::String::rstrip(result, '\n'), &result);
	return result;
Failed:
	result += "玩家信息获取失败!";
	Helper::String::fixUtf8(Helper::String::rstrip(result, '\n'), &result);
	return result;
}