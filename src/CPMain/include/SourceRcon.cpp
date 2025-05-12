#include <regex>
#include "SourceRcon.h"
#include "Helper.h"
#pragma comment(lib, "ws2_32.lib")

SourceRcon::SourceRcon(const char *ip, int port, const std::string &password, const std::string &command)
{
	sin.sin_family = AF_INET;
	sin.sin_addr.S_un.S_addr = inet_addr(ip);
	sin.sin_port = htons(port);
	this->password = Helper::String::rstrip(password, '\n');
	this->command = Helper::String::rstrip(command, '\n');
}

SourceRcon::~SourceRcon(void)
{
	if (sock != INVALID_SOCKET)
		closesocket(sock);
	if (wsaStartup == NO_ERROR)
		WSACleanup();
}

std::string SourceRcon::rcon(bool source)
{
	if (wsaStartup != NO_ERROR)
		return "\nWSAStartup failed";
	if (Helper::String::contains(command, "rcon_password"))
		return err[0];
	return source ? sourceRcon() : goldsourceRcon();
}

std::string SourceRcon::sourceRcon(void)
{
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		return '\n' + "SOCKET failed";
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&sendTimeout, sizeof(int));
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&recvTimeout, sizeof(int));
	if (!Helper::Socket::success(0, connect(sock, (SOCKADDR *)&sin, sizeof(sin))))
		return err[1];
	for (int i = 0; i < 2; i++)
	{
		const int packetSize = (int)(13 + (i == 0 ? password.length() : command.length()));
		std::unique_ptr<char> packet(new char[packetSize] { 0 });
		packet.get()[0] = (char)((i == 0 ? password.length() : command.length()) + 9);
		packet.get()[4] = i + 1;
		packet.get()[8] = i == 0 ? SourceServer::AUTH : SourceServer::EXECCOMMAND;
		for (int j = 0; j < (i == 0 ? password.length() : command.length()); j++)
			packet.get()[12 + j] = i == 0 ? password[j] : command[j];
		if (!Helper::Socket::success(2, send(sock, packet.get(), packetSize, 0)))
			return err[1];
	}
	char buffer[4096] = { 0 };
	for (int i = 0; i < 3; i++)
		if (!Helper::Socket::success(3, recv(sock, buffer, sizeof(buffer), 0)))
			return err[1];
	std::string result;
	result += '\n';
	for (int i = 12; buffer[i] != '\x00'; i++)
		result += buffer[i];
	Helper::String::fixUtf8(Helper::String::rstrip(result, '\n'), &result);
	return result;
}

std::string SourceRcon::goldsourceRcon(void)
{
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
		return '\n' + "SOCKET failed";
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&sendTimeout, sizeof(int));
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&recvTimeout, sizeof(int));
	int sinSize = sizeof(sin);
	std::unique_ptr<char> packet(new char[32] { 0 });
	if (Helper::Socket::success(2, sendto(sock, GoldSourceServer.AUTH, (int)strlen(GoldSourceServer.AUTH) + 1, 0, (SOCKADDR *)&sin, sizeof(sin))))
		if (Helper::Socket::success(3, recvfrom(sock, packet.get(), 32, 0, (SOCKADDR *)&sin, &sinSize)))
		{
			std::regex re("([0-9]+)");
			std::smatch match;
			std::string challenge;
			const std::string &temp = packet.get();
			if (std::regex_search(temp.begin(), temp.end(), match, re))
				challenge = match[1].str();
			packet.reset(new char[strlen(GoldSourceServer.EXECCOMMAND) + challenge.size() + 1 + password.length() + 1 + command.length() + 1]);
			sprintf(packet.get(), "%s%s%c%s%c%s", GoldSourceServer.EXECCOMMAND, challenge.c_str(), '\x20', password.c_str(), '\x20', command.c_str());
			char buffer[4096] = { 0 };
			if (Helper::Socket::success(2, sendto(sock, packet.get(), (int)strlen(packet.get()) + 1, 0, (SOCKADDR *)&sin, sizeof(sin))))
				if (Helper::Socket::success(3, recvfrom(sock, buffer, sizeof(buffer), 0, (SOCKADDR *)&sin, &sinSize)))
				{
					std::string result;
					result += '\n';
					for (int i = 5; buffer[i] != '\x00'; i++)
						result += buffer[i];
					Helper::String::fixUtf8(Helper::String::rstrip(result, '\n'), &result);
					return result;
				}
		}
	return err[1];
}

const std::string &SourceRcon::geterr(int index)
{
	return err[index];
}