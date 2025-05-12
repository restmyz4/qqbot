#include <format>
#include <thread>
#include "TCPPing.h"
#include "Helper.h"

TCPPing::TCPPing(const char *ip, int port)
{
	sin.sin_family = AF_INET;
	sin.sin_addr.S_un.S_addr = inet_addr(ip);
	sin.sin_port = htons(port);
}

TCPPing::~TCPPing(void)
{
	if (wsaStartup == NO_ERROR)
		WSACleanup();
}

std::string TCPPing::ping(void)
{
	if (wsaStartup != NO_ERROR)
		return "WSAStartup failed";
	int fail = 0;
	clock_t all[4] = { -1, -1, -1, -1 };
	clock_t average = 0;
	clock_t min = 0;
	clock_t max = 0;
	for (int i = 0; i < 4; i++)
	{
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET)
		{
			fail++;
			continue;
		}
		bool ok = false;
		const clock_t start = clock();
		std::thread([&]()
		{
			if (Helper::Socket::success(0, connect(sock, (SOCKADDR *)&sin, sizeof(sin))))
			{
				ok = true;
				all[i] = clock() - start;
			}
		}).detach();
		for (int i = 0; !ok && i < 1000; i++)
			Sleep(1);
		if (!ok)
			fail++;
		closesocket(sock);
	}
	if (all[0] > -1)
		min = all[0];
	for (int i = 0; i < 4; i++)
	{
		if (all[i] > -1 && min > all[i])
			min = all[i];
		if (max < all[i])
			max = all[i];
		if (all[i] > -1)
			average += all[i];
	}
	average /= 4;
	return std::format("\n四次延时: {0}, {1}, {2}, {3}\n数据包: 已发送 = 4, 已接收 = {4}, 丢失: = {5} ({6}% 丢失)\n最短 = {7}ms, 最长 = {8}ms, 平均 = {9}ms",
		make(all[0]), make(all[1]), make(all[2]), make(all[3]), 4 - fail, fail, 25 * fail, min, max, average);
}

std::string TCPPing::make(clock_t ping)
{
	if (ping == -1)
		return "没有回应";
	else
		return std::to_string(ping) + "ms";
}