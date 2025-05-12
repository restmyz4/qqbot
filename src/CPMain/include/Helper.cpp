#include <random>
#include <io.h>
#include <format>
#include <direct.h>
#include <sstream>
#include <regex>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include "Helper.h"
#include "utf8.h"
#pragma comment(lib, "sqlite3.lib")

namespace Helper
{
	namespace Path
	{
		std::string basePath(const int mode) noexcept
		{
			char basePath[MAX_PATH];
			getcwd(basePath, MAX_PATH);
			switch (mode)
			{
				case 0:
					return std::format("{0}/data/tech.eritquearcus.miraiCP/configs", basePath);
				case 1:
					return std::format("{0}/data/image", basePath);
				case 2:
					return std::format("{0}/data/record", basePath);
			}
			return "";
		}

		bool Exists(const std::string &path) noexcept
		{
			return access(path.c_str(), 0) == 0;
		}

		void Throw(const std::string &path) noexcept
		{
			printf("%s input failed", path.c_str());
		}
	}

	namespace String
	{
		std::vector<std::string> split(const std::string &str, char delim)
		{
			std::istringstream iss(str);
			std::string token;
			std::vector<std::string> result;
			while (std::getline(iss, token, delim))
				result.emplace_back(token);
			return result;
		}

		std::string lstrip(const std::string &str, char substr)
		{
			return str.substr(str.find_first_not_of(substr));
		}

		std::string rstrip(const std::string &str, char substr)
		{
			return str.substr(0, str.find_last_not_of(substr) + 1);
		}

		bool contains(const std::string &str, const std::string &substr) noexcept
		{
			return str.find(substr) != std::string::npos;
		}

		bool contains(const std::vector<std::string> &str, const std::string &substr) noexcept
		{
			return count(str.begin(), str.end(), substr);
		}

		void fixUtf8(const std::string &str, std::string *strOut)
		{
			std::string temp;
			utf8::replace_invalid(str.begin(), str.end(), std::back_inserter(temp));
			*strOut = temp;
		}
	}

	namespace Time
	{
		const char *getLocalTime(const char *format) noexcept
		{
			time_t t = time(0);
			static char result[32];
			strftime(result, sizeof(result), format, localtime(&t));
			return result;
		}
	}

	namespace Random
	{
		inline std::random_device rd;

		int randInt(int min, int max) noexcept
		{
			return min + rd() % max;
		}
	}

	namespace Socket
	{
		bool success(int action, int code) noexcept
		{
			switch (action)
			{
				case 0:
				case 1:
				case 2:
					return code != SOCKET_ERROR;
				case 3:
					return code != NULL && code != SOCKET_ERROR;
			}
			return false;
		}

		bool isVaildIP(const std::string &ip, std::string *ipOut) noexcept
		{
			std::regex re("^((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]|2[0-4]\\d|[01]?\\d\\d?)$");
			std::smatch match;
			if (!std::regex_search(ip.begin(), ip.end(), match, re))
				if (!resolveDomain(ipOut))
					return false;
			return true;
		}

		bool isVaildIP(const std::string &ip) noexcept
		{
			std::regex re("^((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]|2[0-4]\\d|[01]?\\d\\d?)$");
			std::smatch match;
			if (!std::regex_search(ip.begin(), ip.end(), match, re))
					return false;
			return true;
		}

		bool isVaildPort(int port) noexcept
		{
			return port >= 0 && port <= USHRT_MAX;
		}

		bool isVaildIPPort(const std::string &ip, std::string *ipOut, int port) noexcept
		{
			if (isVaildIP(ip, ipOut))
				if (isVaildPort(port))
					return true;
			return false;
		}

		bool isVaildIPPort(const std::string &ip, int port) noexcept
		{
			if (isVaildIP(ip))
				if (isVaildPort(port))
					return true;
			return false;
		}

		bool resolveDomain(std::string *domain) noexcept
		{
			WSADATA wsaData;
			if (WSAStartup(WINSOCK_VERSION, &wsaData) != NO_ERROR)
				return false;
			hostent *host = gethostbyname(domain->c_str());
			if (!host)
			{
				WSACleanup();
				return false;
			}
			else if (host->h_addr_list[0])
				*domain = inet_ntoa(*(in_addr *)host->h_addr_list[0]);
			WSACleanup();
			return true;
		}

		std::string feedback(int mode) noexcept
		{
			const char *emoji = "⚠";
			const char *ip = "IP/域名";
			const char *port = "端口";
			const char *reason = "解析失败!";
			switch (mode)
			{
				case 0:
					return std::format("\n{0}{1}{2}", emoji, ip, reason);
				case 1:
					return std::format("\n{0}{1}{2}", emoji, port, reason);
				case 2:
					return std::format("\n{0}{1}/{2}{3}", emoji, ip, port, reason);
			}
			return "";
		}
	}

	namespace SQLite
	{
		std::string getTable(sqlite3 *db, const char *sql)
		{
			int result;
			char *errMsg = nullptr;
			static char **dbResult;
			int row, col;
			result = sqlite3_get_table(db, sql, &dbResult, &row, &col, &errMsg);
			if (result != SQLITE_OK)
				return "";
			return dbResult[col];
		}

		std::vector<std::string> getTableVector(sqlite3 *db, const char *sql)
		{
			int result;
			char *errMsg = nullptr;
			char **dbResult;
			int row, col;
			result = sqlite3_get_table(db, sql, &dbResult, &row, &col, &errMsg);
			if (result != SQLITE_OK)
				return std::vector<std::string>{""};
			std::vector<std::string> dbResultVector;
			for (int i = 1; i < row + 1; i++)
				dbResultVector.emplace_back(dbResult[i]);
			return dbResultVector;
		}

		bool verifyValueFromGroup(sqlite3 *db, const std::string &data, const std::string &name, const std::string &group)
		{
			return (bool)std::stoi(getTable(db, std::format("select count(1) from tb_groups where sn_id in \
			(select sn_id from tb_servers where {0} = '{1}') and group_id = '{2}';", data, name, group).c_str()));
		}

		bool verifyAdminFromGroup(sqlite3 *db, const std::string &admin, const std::string &name, const std::string &group)
		{
			return (bool)std::stoi(getTable(db, std::format("select count(1) from tb_admins where sn_id in \
			(select sn_id from tb_groups where group_id = '{0}') and admin_id = '{1}' and (server_name = '' \
			or server_name like '%{2};%');", group, admin, name).c_str()));
			return true;
		}

		bool verifySendMsgFromGroup(sqlite3 *db, const std::string &name, const std::string &group, bool alt)
		{
			return (bool)std::stoi(getTable(db, std::format("select count(1) from tb_groups where sn_id in \
			(select sn_id from tb_servers where number in (select sendmsg from tb_servers where {0} = '{1}')) \
			and group_id = '{2}';", alt ? "name2" : "name", name, group).c_str()));
		}

		std::string getServerValueFromGroup(sqlite3 *db, const std::string &data, const std::string &name, const std::string &group, bool alt)
		{
			return getTable(db, std::format("select {0} from tb_servers where sn_id in \
			(select sn_id from tb_groups where group_id = '{1}') and {2} = '{3}'; ",
			data, group, alt ? "name2" : "name", name).c_str());
		}

		std::vector<std::string> getServerValueFromGroupVector(sqlite3 *db, const std::string &data, const std::string &group)
		{
			return getTableVector(db, std::format("select {0} from tb_servers where sn_id in \
			(select sn_id from tb_groups where group_id = '{1}');", data, group).c_str());
		}
	}
}