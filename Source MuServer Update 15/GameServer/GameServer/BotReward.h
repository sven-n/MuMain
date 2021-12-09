#include "user.h"
#include "stdafx.h"

#if (BOT_REWARD ==1)

struct BotRewardBodyItems
{
	int num;
	int level;
	int opt;
	bool Enabled;
};

struct botRewardStruct
{
	int Class;
	char Name[11];
	BYTE Map;
	BYTE X;
	BYTE Y;
	BYTE Dir;
	bool Enabled;
	int index;

	BotRewardBodyItems body[9];
};

class ObjBotReward
{
public:
	bool Enabled;
	void Read(char * FilePath);
	void MakeBot();
	BOOL TradeOpen(int index,int nindex);
	botRewardStruct bot;
private:
	int GetBotIndex(int aIndex);
};

extern ObjBotReward BotReward;

#endif