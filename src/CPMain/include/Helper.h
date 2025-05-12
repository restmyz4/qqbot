#pragma once
#include <string>
#include <vector>
#include "sqlite3.h"

namespace Helper
{
	namespace Path
	{
		// mode: 0 = default, 1 = image, 2 = voice
		std::string basePath(int mode = 0) noexcept;
		bool Exists(const std::string &path) noexcept;
		void Throw(const std::string &path) noexcept;
	}

	namespace String
	{
		std::vector<std::string> split(const std::string &str, char delim);
		std::string lstrip(const std::string &str, char substr);
		std::string rstrip(const std::string &str, char substr);
		bool contains(const std::string &str, const std::string &substr) noexcept;
		bool contains(const std::vector<std::string> &str, const std::string &substr) noexcept;
		void fixUtf8(const std::string &str, std::string *strOut);
	}

	namespace Time
	{
		const char *getLocalTime(const char *format) noexcept;
	}

	namespace Random
	{
		// [min, max)
		int randInt(int min, int max) noexcept;
	}

	namespace Socket
	{
		// action: 0 = connect, 1 = bind, 2 = send/sendto, 3 = recv/recvfrom
		bool success(int action, int code) noexcept;
		bool isVaildIP(const std::string &ip, std::string *ipOut) noexcept;
		bool isVaildIP(const std::string &ip) noexcept;
		bool isVaildPort(int port) noexcept;
		bool isVaildIPPort(const std::string &ip, std::string *ipOut, int port) noexcept;
		bool isVaildIPPort(const std::string &ip, int port) noexcept;
		bool resolveDomain(std::string *domain) noexcept;
		std::string feedback(int mode) noexcept;
	}

	namespace SQLite
	{
		std::string getTable(sqlite3 *db, const char *sql);
		std::vector<std::string> getTableVector(sqlite3 *db, const char *sql);
		bool verifyValueFromGroup(sqlite3 *db, const std::string &data, const std::string &name, const std::string &group);
		bool verifyAdminFromGroup(sqlite3 *db, const std::string &admin, const std::string &name, const std::string &group);
		bool verifySendMsgFromGroup(sqlite3 *db, const std::string &name, const std::string &group, bool alt = false);
		std::string getServerValueFromGroup(sqlite3 *db, const std::string &data, const std::string &name, const std::string &group, bool alt = false);
		std::vector<std::string> getServerValueFromGroupVector(sqlite3 *db, const std::string &data, const std::string &group);
	}
}