#include <format>
#include "httplib.h"
#include "json.hpp"
#include "RequestSong.h"
#include "URLEncoding.h"
#include "Helper.h"

RequestSong::RequestSong(const std::string &song)
{
	this->song = song;
}

RequestSong::~RequestSong(void)
{
}

std::vector<std::string> RequestSong::request(void)
{
	const std::string &cfgPath = std::format("{0}/request_music.json", Helper::Path::basePath());
	if (!Helper::Path::Exists(cfgPath))
	{
		Helper::Path::Throw(cfgPath);
		return std::vector<std::string>{ err[2] };
	}
	try
	{
		std::ifstream ifs(cfgPath);
		json = nlohmann::json::parse(ifs);
		ifs.close();
	}
	catch (nlohmann::json::exception)
	{
		return std::vector<std::string>{ err[2] };
	}
	httplib::Client cli("http://music.163.com");
	if (json["netease"]["proxy_enable"] && Helper::Socket::isVaildIPPort(json["netease"]["ip"], json["netease"]["port"]))
		cli.set_proxy(json["netease"]["ip"], json["netease"]["port"]);
	auto res = cli.Get(std::format("/api/search/get/web?csrf_token=hlpretag=&hlposttag=&s={0}&type=1&offset=0&total=true&limit=1",
		UrlEncoding().toUrl(song)));
	if (res->status != 200)
		return std::vector<std::string>{ err[0] };
	try
	{
		nlohmann::json json = nlohmann::json::parse(res->body);
		std::string title = json["result"]["songs"][0]["name"];
		std::string author = json["result"]["songs"][0]["artists"][0]["name"];
		const int id = json["result"]["songs"][0]["id"];
		if (id <= 0)
			return std::vector<std::string>{ err[1] };
		res = cli.Get(std::format("/api/song/detail/?id={0}&ids=%5B{1}%5D", id, id));
		if (res->status != 200)
			return std::vector<std::string>{ err[0] };
		std::string pic = "";
		try
		{
			json = nlohmann::json::parse(res->body);
			pic = json["songs"][0]["album"]["blurPicUrl"];
		}
		catch (nlohmann::json::exception)
		{
			return std::vector<std::string>{ err[1] };
		}
		if (title.size() > 0)
		{
			title.erase(std::remove(title.begin(), title.end(), ':'), title.end());
			title.erase(std::remove(title.begin(), title.end(), ','), title.end());
			title.erase(std::remove(title.begin(), title.end(), '['), title.end());
			title.erase(std::remove(title.begin(), title.end(), ']'), title.end());
			title.erase(std::remove(title.begin(), title.end(), '/'), title.end());
		}
		if (author.size() > 0)
		{
			author.erase(std::remove(author.begin(), author.end(), ':'), author.end());
			author.erase(std::remove(author.begin(), author.end(), ','), author.end());
			author.erase(std::remove(author.begin(), author.end(), '['), author.end());
			author.erase(std::remove(author.begin(), author.end(), ']'), author.end());
			author.erase(std::remove(author.begin(), author.end(), '/'), author.end());
		}
		return std::vector<std::string>{"NeteaseCloudMusic", title, author, std::format("https://music.163.com/song?id={0}", id),
			pic, std::format("http://music.163.com/song/media/outer/url?id={0}.mp3", id), "网易云音乐分享"};
	}
	catch (nlohmann::json::exception)
	{
		return std::vector<std::string>{ err[1] };
	}
}