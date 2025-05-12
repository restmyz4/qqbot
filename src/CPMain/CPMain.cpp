#include <format>
#include <fstream>
#include <filesystem>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")
#include "CPMain.h"
#include "Helper.h"
#include "SourceQuery.h"
#include "SourceRcon.h"
#include "SourceLog.h"
#include "TCPPing.h"
#include "RequestSong.h"
#include "Duel.h"

void MiraiCP::enrollPlugin(void)
{
	MiraiCP::enrollPlugin0(new CPMain());
}

void CPMain::onEnable(void)
{
	Event::processor.registerEvent<BotOnlineEvent>([this](BotOnlineEvent e)
	{ onBotOnline(e); });
	Event::processor.registerEvent<GroupMessageEvent>([this](GroupMessageEvent e)
	{ onGroupMessage(e); });
}

void CPMain::onDisable(void)
{
}

void CPMain::onBotOnline(BotOnlineEvent &e)
{
	std::thread([=]()
	{
		SourceLog::Reciever slr("0.0.0.0", 48927);
		if (!slr.bind())
			return;
		QQID fromGroup;
		std::string msg;
		for (;;)
			if (slr.recv(&fromGroup, &msg))
				Group(fromGroup, e.bot.id).sendMessage(msg);
	}).detach();
}

void CPMain::onGroupMessage(GroupMessageEvent &e)
{
	if (SingleMessage::messageType[e.message.front().type()] != "plainText")
		return;
	if (e.message.toMiraiCode() == "喝茶")
	{
		switch (atoi(Helper::Time::getLocalTime("%H")))
		{
			case 12:
			case 18:
			case 19:
			{
				e.group.sendMessage(e.sender.at(), "\n吃饭时间不准喝茶");
				break;
			}
			default:
			{
				const int bannedTime = Helper::Random::randInt(60, 300);
				e.group.sendMessage(e.sender.at(), std::format("\n已服用红茶, 昏睡 {0} 分钟", bannedTime / 60));
				if (e.sender.permission == 0)
				{
					try
					{
						e.sender.mute(bannedTime);
					}
					catch (BotException)
					{
					}
					break;
				}
			}
		}
		return;
	}
	if (e.message.toMiraiCode() == "精致喝茶" ||
		e.message.toMiraiCode() == "精緻喝茶")
	{
		switch (atoi(Helper::Time::getLocalTime("%H")))
		{
			case 12:
			case 18:
			case 19:
			{
				e.group.sendMessage(e.sender.at(),
					"\n吃饭时间不准喝茶");
				break;
			}
			default:
			{
				const int bannedTime = Helper::Random::randInt(300, 1200);
				e.group.sendMessage(e.sender.at(), std::format("\n已服用红茶, 昏睡 {0} 分钟", bannedTime / 60));
				if (e.sender.permission == 0)
				{
					try
					{
						e.sender.mute(bannedTime);
					}
					catch (BotException)
					{
					}
					break;
				}
			}
		}
		return;
	}
	if (e.message.toMiraiCode() == "精致睡眠" ||
		e.message.toMiraiCode() == "精緻睡眠")
	{
		const int hour = atoi(Helper::Time::getLocalTime("%H"));
		if (hour > 8 && hour < 22)
			e.group.sendMessage(e.sender.at(), "\n还没到睡觉时间, 不准用");
		else
		{
			e.group.sendMessage(e.sender.at(), "\n已使用精致睡眠, 祝您睡眠愉快");
			if (e.sender.permission == 0)
			{
				try
				{
					e.sender.mute(28800);
				}
				catch (BotException)
				{
				}
			}
		}
		return;
	}
	if (e.message.toMiraiCode() == "粗制睡眠" ||
		e.message.toMiraiCode() == "粗製睡眠")
	{
		e.group.sendMessage(e.sender.at(), "\n已使用粗制睡眠, 祝您睡眠不愉快");
		if (e.sender.permission == 0)
		{
			try
			{
				e.sender.mute(7200);
			}
			catch (BotException)
			{
			}
		}
		return;
	}
	if (e.message.toMiraiCode().starts_with("!tcping"))
	{
		std::vector<std::string> msgSplit = Helper::String::split(e.message.toMiraiCode(), ' ');
		if (msgSplit.size() < 2)
			return;
		msgSplit = Helper::String::split(msgSplit[1], ':');
		if (msgSplit.size() < 2)
			return;
		if (!Helper::Socket::isVaildIPPort(msgSplit[0], &msgSplit[0], std::stoi(msgSplit[1])))
		{
			e.group.sendMessage(e.sender.at(), Helper::Socket::feedback(2));
			return;
		}
		e.group.sendMessage(e.sender.at(), TCPPing(msgSplit[0].c_str(), std::stoi(msgSplit[1])).ping());
		return;
	}
	if (e.message.toMiraiCode().starts_with("点歌") ||
		e.message.toMiraiCode().starts_with("點歌"))
	{
		const std::vector<std::string> &msgSplit = Helper::String::split(e.message.toMiraiCode(), ' ');
		if (msgSplit.size() < 2)
			return;
		const std::vector<std::string> &result = RequestSong(e.message.toMiraiCode().substr(msgSplit[0].length() + 1)).request();
		if (result.size() == 7)
			e.group.sendMessage(MusicShare(result[0], result[1], result[2], result[3], result[4], result[5], result[6]));
		else
		{
			const std::vector<std::string> &result = RequestSong(e.message.toMiraiCode().substr(msgSplit[0].length() + 1)).
				request();
			if (result.size() == 7)
				e.group.sendMessage(MusicShare(result[0], result[1], result[2], result[3], result[4], result[5], result[6]));
			else
				e.group.sendMessage(result[0]);
		}
		return;
	}
	if (e.message.toMiraiCode().starts_with("!bet"))
	{
		const std::vector<std::string> &msgSplit = Helper::String::split(e.message.toMiraiCode(), ' ');
		if (msgSplit.size() < 2)
			return;
		const int bullets = std::stoi(msgSplit[1]);
		if (bullets < 1 || bullets > 5)
			e.group.sendMessage(e.sender.at(), "\n数值必须为大于0且小于6的整数!");
		else
		{
			const std::string &numsOfBulletMsg = std::format("\n放入了{0}颗子弹!", bullets);
			if (bullets >= Helper::Random::randInt(1, 6))
			{
				e.group.sendMessage(e.sender.at(), numsOfBulletMsg, "\n你输了并被爆头了! 禁言5分钟!");
				if (e.sender.permission == 0)
				{
					try
					{
						e.sender.mute(300);
					}
					catch (BotException)
					{
					}
				}
			}
			else
				e.group.sendMessage(e.sender.at(), numsOfBulletMsg, "\n你赢了!");
		}
		return;
	}
	if (e.message.toMiraiCode() == "决斗" ||
		e.message.toMiraiCode() == "決鬥")
	{
		int banObject;
		QQID requestQQ = e.sender.id();
		int bannedTime;
		const std::vector<std::string> &msg = Duel(e.group.id(), e.sender.id()).request(&banObject, &requestQQ, &bannedTime);
		switch (banObject)
		{
			case -1:
			{
				e.group.sendMessage(e.sender.at(), msg[0]);
				break;
			}
			case 0:
			{
				e.group.sendMessage(e.sender.at(), msg[0]);
				if (e.sender.permission == 0)
				{
					try
					{
						e.sender.mute(bannedTime);
					}
					catch (BotException)
					{
					}
				}
				break;
			}
			case 1:
			{
				e.group.sendMessage(e.sender.at(), msg[0], At(requestQQ), msg[1], e.sender.at(), msg[2]);
				if (e.sender.permission == 0)
				{
					try
					{
						e.sender.mute(bannedTime);
					}
					catch (BotException)
					{
					}
				}
				break;
			}
			case 2:
			{
				e.group.sendMessage(e.sender.at(), msg[0], At(requestQQ), msg[1], At(requestQQ), msg[2]);
				if (Member(requestQQ, e.group.id(), e.bot.id).getPermission() == 0)
				{
					try
					{
						Member(requestQQ, e.group.id(), e.bot.id).mute(bannedTime);
					}
					catch (BotException)
					{
					}
				}
				break;
			}
			case 3:
			{
				e.group.sendMessage(e.sender.at(), msg[0], At(requestQQ), msg[1], msg[2]);
				try
				{
					if (e.sender.permission == 0)
						e.sender.mute(bannedTime);
					if (Member(requestQQ, e.group.id(), e.bot.id).getPermission() == 0)
						Member(requestQQ, e.group.id(), e.bot.id).mute(bannedTime);
				}
				catch (BotException)
				{
				}
				break;
			}
		}
		return;
	}
	if (e.message.toMiraiCode() == "运势" ||
		e.message.toMiraiCode() == "運勢")
	{
		const char *date = Helper::Time::getLocalTime("%Y-%m-%d");
		bool canRead = true;
		bool newDay = false;
		const std::string &cfgPath = std::format("{0}/yunshi.cfg", Helper::Path::basePath());
		if (!Helper::Path::Exists(cfgPath))
		{
			Helper::Path::Throw(cfgPath);
			return;
		}
		std::ifstream ifs(cfgPath);
		while (!ifs.eof())
		{
			std::string line;
			ifs >> line;
			if (canRead)
			{
				if (line != date)
				{
					ifs.close();
					std::ofstream ofs(cfgPath, std::ios::trunc);
					ofs << date << '\n';
					ofs.close();
					newDay = true;
					break;
				}
				else
					canRead = false;
			}
			if (line == std::to_string(e.sender.id()))
			{
				e.group.sendMessage(e.sender.at(), "\n❌你今日已经查过运势了!");
				ifs.close();
				return;
			}
		}
		if (!newDay)
			ifs.close();
		std::ofstream ofs(cfgPath, std::ios::app);
		ofs << std::to_string(e.sender.id()) << '\n';
		ofs.close();
		const std::string &cfgPath2 = std::format("{0}/yunshi.json", Helper::Path::basePath());
		if (!Helper::Path::Exists(cfgPath2))
		{
			Helper::Path::Throw(cfgPath2);
			return;
		}
		try
		{
			std::ifstream ifs(cfgPath2);
			nlohmann::json json = nlohmann::json::parse(ifs);
			ifs.close();
			int id = Helper::Random::randInt(0, (int)json.size());
			const std::string &title = json[id]["title"];
			const std::string &content = json[id]["content"][Helper::Random::randInt(0, (int)json[id]["content"].size())];
			e.group.sendMessage(e.group.uploadImg(Helper::Path::basePath(1) +
				"/yunshi/yunshi.png"),
				"\n", e.sender.at(), std::format("\n运势: {0}\n评语：{1}", title, content));
		}
		catch (nlohmann::json::exception)
		{
			e.group.sendMessage("运势模块异常！");
		}
		return;
	}
	if (e.message.toMiraiCode().starts_with("!v"))
	{
		const std::vector<std::string> &msgSplit = Helper::String::split(e.message.toMiraiCode(), ' ');
		if (msgSplit.size() < 2)
			return;
		const std::string &cfgPath = std::format("{0}/ChatSounds.cfg", Helper::Path::basePath());
		if (!Helper::Path::Exists(cfgPath))
		{
			Helper::Path::Throw(cfgPath);
			return;
		}
		std::ifstream ifs(cfgPath);
		while (!ifs.eof())
		{
			std::string line;
			ifs >> line;
			if (line == msgSplit[1])
			{
				e.group.sendVoice(Helper::Path::basePath(2) + '/' + line + ".amr");
				ifs.close();
				return;
			}
		}
		e.group.sendMessage(e.sender.at(), "\n❌未找到该音频!");
		ifs.close();
		return;
	}
	if (e.message.toMiraiCode() == "thetime")
	{
		e.group.sendMessage(Helper::Time::getLocalTime("%Y.%m.%d - %H:%M:%S"));
		return;
	}
	if (e.message.toMiraiCode() == "娶群友")
	{
		const char *date = Helper::Time::getLocalTime("%Y-%m-%d");
		bool canRead = true;
		bool newDay = false;
		const std::string &cfgPath = std::format("{0}/jrlp.cfg", Helper::Path::basePath());
		if (!Helper::Path::Exists(cfgPath))
		{
			Helper::Path::Throw(cfgPath);
			return;
		}
		std::ifstream ifs(cfgPath);
		while (!ifs.eof())
		{
			std::string line;
			ifs >> line;
			if (canRead)
			{
				if (line != date)
				{
					ifs.close();
					std::ofstream ofs(cfgPath, std::ios::trunc);
					ofs << date << '\n';
					ofs.close();
					newDay = true;
					break;
				}
				else
					canRead = false;
			}
			if (line == std::to_string(e.sender.id()))
			{
				e.group.sendMessage(e.sender.at(), "\n❌你已经娶过了, 一天之内不可再娶!");
				ifs.close();
				return;
			}
		}
		if (!newDay)
			ifs.close();
		std::ofstream ofs(cfgPath, std::ios::app);
		ofs << std::to_string(e.sender.id()) << '\n';
		ofs.close();
		const std::vector<QQID> &memberList = e.group.getMemberList();
		const Member &member = e.group.getMember(memberList[Helper::Random::randInt(0, (int)memberList.size())]);
		const std::string &image = std::format("{0}/{1}.jpg", Helper::Path::basePath(1), std::format("{0}.jpg", Helper::Random::randInt(0, 1000)));
		if (SUCCEEDED(URLDownloadToFileA(NULL, member.avatarUrl().c_str(), image.c_str(), 0, NULL)))
		{
			e.group.sendMessage(e.sender.at(), "\n你娶到了", e.group.uploadImg(image), member.nickOrNameCard());
			DeleteFileA(image.c_str());
		}
		return;
	}
	if (e.message.toMiraiCode().starts_with("!sv"))
	{
		std::vector<std::string> msgSplit = Helper::String::split(e.message.toMiraiCode(), ' ');
		if (msgSplit.size() < 2)
			return;
		msgSplit = Helper::String::split(msgSplit[1], ':');
		if (msgSplit.size() < 2)
			return;
		if (!Helper::Socket::isVaildIPPort(msgSplit[0], &msgSplit[0], std::stoi(msgSplit[1])))
		{
			e.group.sendMessage(e.sender.at(), Helper::Socket::feedback(2));
			return;
		}
		std::vector<ForwardedNode> nodes;
		nodes.emplace_back(ForwardedNode(e.bot.id, e.bot.nick(), MessageChain(e.sender.at(),
			PlainText(SourceQuery(msgSplit[0].c_str(), std::stoi(msgSplit[1])).query())), (int)time(0)));
		ForwardedMessage(nodes).sendTo(&e.group);
		return;
	}
	if (e.message.toMiraiCode() == "allsv")
	{
		const std::string &dbPath = std::format("{0}/servers.db3", Helper::Path::basePath());
		sqlite3 *db = nullptr;
		if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK)
		{
			Helper::Path::Throw(dbPath);
			return;
		}
		const std::vector<std::string> &msgSplit = Helper::String::split(e.message.toMiraiCode(), ' ');
		bool allsvAbort = false;
		int total_online_servers = 0;
		int total_players = 0;
		int total_maxplayers = 0;
		int total_bots = 0;
		std::string statusResult;
		const std::vector<std::string> &name = Helper::SQLite::getServerValueFromGroupVector(db, "name", std::to_string(e.group.id()));
		const std::vector<std::string> &addr = Helper::SQLite::getServerValueFromGroupVector(db, "addr", std::to_string(e.group.id()));
		for (int i = 0; i < name.size() && i < addr.size(); i++)
		{
			std::vector<std::string> line2Split = Helper::String::split(addr[i], ':');
			const std::string line2Split0_orig = line2Split[0];
			if (!Helper::Socket::isVaildIPPort(line2Split[0], &line2Split[0], std::stoi(line2Split[1])))
				allsvAbort = true;
			if (allsvAbort)
				allsvAbort = false;
			const char *errMsg = "未响应";
			const std::string &name2 = Helper::SQLite::getServerValueFromGroup(db, "name2", name[i], std::to_string(e.group.id()), false);
			if (!allsvAbort)
			{
				const std::vector<int> &valueIndex = { 4, 5, 6, 1 };
				const std::string &result = SourceQuery(line2Split[0].c_str(), std::stoi(line2Split[1])).query(100, 100, 0, errMsg, valueIndex);
				if (!allsvAbort && result != errMsg)
				{
					const std::vector<std::string> &resultSplit = Helper::String::split(result, '\0');
					if (resultSplit.size() == valueIndex.size())
					{
						if (name2.length() > 0)
							statusResult += std::format("名称：{0}\n别名：{1}\n地址：{2}\n人数：{3}/{4} ({5} Bot)\n地图：{6}\n\n",
								name[i], name2, line2Split0_orig + ':' + line2Split[1], resultSplit[0], resultSplit[1], resultSplit[2], resultSplit[3]);
						else
							statusResult += std::format("名称：{0}\n地址：{1}\n人数：{2}/{3} ({4} Bot)\n地图：{5}\n\n",
								name[i], line2Split0_orig + ':' + line2Split[1], resultSplit[0], resultSplit[1], resultSplit[2], resultSplit[3]);
						total_online_servers++;
						total_players += std::stoi(resultSplit[0]);
						total_maxplayers += std::stoi(resultSplit[1]);
						total_bots += std::stoi(resultSplit[2]);
					}
				}
				else
				{
					if (name2.length() > 0)
						statusResult += std::format("名称：{0}\n别名：{1}\n地址：{2}\n状态：{3}\n\n",
							name[i], name2, line2Split0_orig + ':' + line2Split[1], result);
					else
						statusResult += std::format("名称：{0}\n地址：{1}\n状态：{2}\n\n",
							name[i], line2Split0_orig + ':' + line2Split[1], result);
				}
			}
			if (allsvAbort)
			{
				if (name2.length() > 0)
					statusResult += std::format("名称：{0}\n别名：{1}\n状态：{2}\n\n",
						name[i], name2, errMsg);
				else
					statusResult += std::format("名称：{0}\n状态：{1}\n\n",
						name[i], errMsg);
			}
		}
		if (statusResult.length() > 0)
		{
			std::vector<ForwardedNode> nodes;
			nodes.emplace_back(ForwardedNode(e.bot.id, e.bot.nick(), MessageChain(e.sender.at(),
				PlainText(std::format("\n在线服务器: {0}/{1}\n在线人数: {2}/{3} ({4} Bot)\n\n",
					total_online_servers, name.size() == addr.size() ? name.size() : addr.size(), total_players, total_maxplayers, total_bots)),
				PlainText(Helper::String::rstrip(statusResult, '\n'))), (int)time(0)));
			ForwardedMessage(nodes).sendTo(&e.group);
		}
		else
			e.group.sendMessage(e.sender.at(), "\n❌暂无可用服务器!");
		sqlite3_close(db);
		return;
	}
	if (e.message.toMiraiCode().starts_with("传话") ||
		e.message.toMiraiCode().starts_with("傳話"))
	{
		const std::vector<std::string> &msgSplit = Helper::String::split(e.message.toMiraiCode(), ' ');
		if (msgSplit.size() < 2)
			return;
		const size_t headerLength = msgSplit[0].length() + msgSplit[1].length();
		if (e.message.toMiraiCode().length() - headerLength < 1)
			return;
		const std::string &dbPath = std::format("{0}/servers.db3", Helper::Path::basePath());
		sqlite3 *db = nullptr;
		if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK)
		{
			Helper::Path::Throw(dbPath);
			return;
		}
		bool send = false;
		bool sendAlt = false;
		if (Helper::SQLite::verifyValueFromGroup(db, "name", msgSplit[1], std::to_string(e.group.id())))
			send = true;
		else if (Helper::SQLite::verifyValueFromGroup(db, "name2", msgSplit[1], std::to_string(e.group.id())))
			sendAlt = true;
		if (send || sendAlt)
		{
			if (Helper::SQLite::getServerValueFromGroup(db, "sendmsg", msgSplit[1], std::to_string(e.group.id()), sendAlt ? true : false) == "" ||
				(!Helper::SQLite::verifySendMsgFromGroup(db, msgSplit[1], std::to_string(e.group.id()), sendAlt ? true : false)))
			{
				e.group.sendMessage(e.sender.at(), "\n❌传话目标在本群受限!");
				sqlite3_close(db);
				return;
			}
			std::vector<std::string> line2Split = Helper::String::split(Helper::SQLite::getServerValueFromGroup(db, "addr",
				msgSplit[1], std::to_string(e.group.id()), sendAlt ? true : false),
				':');
			if (!Helper::Socket::isVaildIPPort(line2Split[0], &line2Split[0], std::stoi(line2Split[1])))
				e.group.sendMessage(e.sender.at(), Helper::Socket::feedback(2));
			SourceRcon sr(line2Split[0].c_str(), std::stoi(line2Split[1]),
				Helper::SQLite::getServerValueFromGroup(db, "rcon", msgSplit[1],
					std::to_string(e.group.id()), sendAlt ? true : false),
				SourceLog::Sender(e.sender.id(),
					e.sender.nickOrNameCard())
				.send(e.message.toMiraiCode().substr(headerLength + 1)));
			if (sr.rcon(true) != sr.geterr(1))
				e.group.sendMessage(e.sender.at(), std::format("\n{0}", e.message.toMiraiCode()));
			else
				e.group.sendMessage(e.sender.at(), std::format("\n{0}", "❌"));
		}
		sqlite3_close(db);
		return;
	}
	if (e.message.toMiraiCode().length() > 0)
	{
		const std::string &dbPath = std::format("{0}/servers.db3", Helper::Path::basePath());
		sqlite3 *db = nullptr;
		if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK)
		{
			Helper::Path::Throw(dbPath);
			return;
		}
		const std::vector<std::string> &msgSplit = Helper::String::split(e.message.toMiraiCode(), ' ');
		bool query = false;
		bool queryAlt = false;
		bool rcon = false;
		bool rconAlt = false;
		if (Helper::SQLite::verifyValueFromGroup(db, "name", e.message.toMiraiCode(), std::to_string(e.group.id())))
			query = true;
		else if (Helper::SQLite::verifyValueFromGroup(db, "name2", e.message.toMiraiCode(), std::to_string(e.group.id())))
			queryAlt = true;
		if (msgSplit[0] == "!rcon" && msgSplit.size() > 2)
		{
			if (Helper::SQLite::verifyValueFromGroup(db, "name", msgSplit[1], std::to_string(e.group.id())))
			{
				if (Helper::SQLite::getServerValueFromGroup(db, "rcon", msgSplit[1], std::to_string(e.group.id())) != "")
					rcon = true;
			}
			else if (Helper::SQLite::verifyValueFromGroup(db, "name2", msgSplit[1], std::to_string(e.group.id())))
				if (Helper::SQLite::getServerValueFromGroup(db, "rcon", msgSplit[1], std::to_string(e.group.id()), true) != "")
					rconAlt = true;
		}
		if (!query && !queryAlt && !rcon && !rconAlt)
		{
			sqlite3_close(db);
			return;
		}
		std::vector<std::string> line2Split = Helper::String::split(Helper::SQLite::getServerValueFromGroup(
			db, "addr", query || queryAlt ? e.message.toMiraiCode() : msgSplit[1],
			std::to_string(e.group.id()), queryAlt || rconAlt ? true : false), ':');
		const std::string line2Split0_orig = line2Split[0];
		if (!Helper::Socket::isVaildIPPort(line2Split[0], &line2Split[0], std::stoi(line2Split[1])))
			e.group.sendMessage(e.sender.at(), Helper::Socket::feedback(2));
		else if (query || queryAlt)
		{
			std::vector<ForwardedNode> nodes;
			nodes.emplace_back(ForwardedNode(e.bot.id, e.bot.nick(), MessageChain(e.sender.at(), PlainText(SourceQuery(line2Split[0].c_str(),
				std::stoi(line2Split[1]), line2Split0_orig)
				.query())), (int)time(0)));
			ForwardedMessage(nodes).sendTo(&e.group);
		}
		else if (rcon || rconAlt)
		{
			if (Helper::SQLite::verifyAdminFromGroup(db, std::to_string(e.sender.id()), msgSplit[1], std::to_string(e.group.id())))
			{
				const std::string &command = e.message.toMiraiCode().substr(msgSplit[0].length() + msgSplit[1].length() + 2);
				SourceRcon sr(line2Split[0].c_str(), std::stoi(line2Split[1]),
					Helper::SQLite::getServerValueFromGroup(db, "rcon",
						query ? e.message.toMiraiCode() : msgSplit[1], std::to_string(e.group.id()), rconAlt ? true : false), command);
				std::vector<ForwardedNode> nodes;
				if (Helper::SQLite::getServerValueFromGroup(db, "source",
					query ? e.message.toMiraiCode() : msgSplit[1], std::to_string(e.group.id()), rconAlt ? true : false) == "true")
					nodes.emplace_back(ForwardedNode(e.bot.id, e.bot.nick(), MessageChain(e.sender.at(), PlainText(sr.rcon(true))), (int)time(0)));
				else if (Helper::SQLite::getServerValueFromGroup(db, "source",
					query ? e.message.toMiraiCode() : msgSplit[1], std::to_string(e.group.id()), rconAlt ? true : false) == "false")
					nodes.emplace_back(ForwardedNode(e.bot.id, e.bot.nick(), MessageChain(e.sender.at(), PlainText(sr.rcon(false))), (int)time(0)));
				ForwardedMessage(nodes).sendTo(&e.group);
			}
			else
				e.group.sendMessage(e.sender.at(), "\n❌拒绝访问!");
		}
		sqlite3_close(db);
		return;
	}
}