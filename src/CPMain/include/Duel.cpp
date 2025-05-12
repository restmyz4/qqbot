#include <format>
#include <thread>
#include "Duel.h"
#include "Helper.h"
#include <Windows.h>

std::vector<std::pair<uint64_t, uint64_t>> Duel::req;

Duel::Duel(uint64_t fromGroup, uint64_t fromQQ)
{
	exfromGroup = fromGroup;
	for (auto &value : req)
		if (value.first == fromGroup)
			newreq = false;
	this->fromQQ = fromQQ;
}

Duel::~Duel(void)
{
}

#define DEFINE_DUEL(op, ban, obj) \
	if (obj1 op obj2) \
	{ \
		*banObject = ban; \
		result.emplace_back(std::format("掷出了{0}点\n", obj1)); \
		result.emplace_back(std::format("掷出了{0}点\n", obj2)); \
		result.emplace_back(std::format("禁言{0}分钟!", obj)); \
		*bannedTime = obj * 60; \
		for (auto it = req.begin(); it != req.end(); it++) \
			if ((*it).first == exfromGroup) \
			{ \
				req.erase(it); \
				break; \
			} \
		return result; \
	}

std::vector<std::string> Duel::request(int *banObject, uint64_t *requestQQ, int *bannedTime)
{
	*banObject = -1;
	for (auto &value : req)
		if (value.first == exfromGroup)
			*requestQQ = value.second;
	*bannedTime = 0;
	std::vector<std::string> result;
	if (newreq)
	{
		std::thread([=]()
		{
			uint64_t curfromGroup = exfromGroup;
			Sleep(300000);
			for (auto it = req.begin(); it != req.end(); it++)
				if ((*it).first == curfromGroup)
				{
					req.erase(it);
					break;
				}
		}).detach();
		Duel::req.emplace_back(exfromGroup, *requestQQ);
		result.emplace_back("\n发起了决斗! 有效期5分钟.");
		return result;
	}
	if (*requestQQ == fromQQ)
	{
		*banObject = 0;
		*bannedTime = 300;
		result.emplace_back(std::format("\n由于你没有任何目标所以瞄准了自己的头, 禁言{0}分钟!", *bannedTime / 60));
		for (auto it = req.begin(); it != req.end(); it++)
			if ((*it).first == exfromGroup)
			{
				req.erase(it);
				break;
			}
		return result;
	}
	int obj1 = Helper::Random::randInt(1, 6);
	int obj2 = Helper::Random::randInt(1, 6);
	DEFINE_DUEL(< , 1, obj2);
	DEFINE_DUEL(> , 2, obj1);
	DEFINE_DUEL(== , 3, obj1);
	return result;
}