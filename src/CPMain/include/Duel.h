#pragma once
#include <string>
#include <vector>

class Duel
{
private:
	bool newreq = true;
	static std::vector<std::pair<uint64_t, uint64_t>> req;
	uint64_t exfromGroup;
	uint64_t fromQQ;
public:
	Duel(uint64_t fromGroup, uint64_t fromQQ);
	~Duel(void);
	std::vector<std::string> request(int *banObject, uint64_t *requestQQ, int *bannedTime);
};
