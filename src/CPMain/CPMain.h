#pragma once
#include <MiraiCP.hpp>

using namespace MiraiCP;

class CPMain : public CPPPlugin
{
public:
	CPMain() : CPPPlugin(PluginConfig("MiraiCP", "MiraiCP", "2.11.0", "MiraiCP", "MiraiCP")) {}
	void onEnable(void) override;
	void onDisable(void) override;
	void onBotOnline(BotOnlineEvent &e);
	void onGroupMessage(GroupMessageEvent &e);
};

void MiraiCP::enrollPlugin(void);