#pragma once
#include <string>
#include <vector>

class RequestSong
{
private:
	std::string song;
	nlohmann::json json;
	std::string err[3] = { "❌HTTP错误!", "❌未找到该歌曲!", "❌配置文件读取失败!" };
public:
	RequestSong(const std::string &song);
	~RequestSong(void);
	std::vector<std::string> request(void);
};