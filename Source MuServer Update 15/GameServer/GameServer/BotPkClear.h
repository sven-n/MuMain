
#define MAX_BOTRESETPK 1

#include"User.h"


struct BOT_RESETPK
{
	int Type;
	int level;
	int opt;
	int Enabled;
};

struct BOTLOAD
{
	bool Enabled;
	int index;
	int Class;
	int OnlyVip;
	int ColorName;
	char Name[11];
	BYTE Map;
	BYTE X;
	BYTE Y;
	BYTE Dir;
	int ActiveGensFamily;
	int ReqContribution;
	int GensFamily;
	BOT_RESETPK Item_BLoad[9];
};

struct BOT_ITEM_REQ
{
	DWORD Zen;
	int PkLevels;

	int ItemReq;
	int iNum;
	int Type;
	int Index;
	int MinLevel;
	int MaxLevel;
	int Luck;
	int Skill;
	int Opt;
	int Exc;
};

class BotPkClear
{
public:
	bool Enabled;
	void Init();
	void LoadData(char* FilePath);
	int GetBotIndex(int aIndex);
	void MakeBot();
	BOOL TradeOpen(int index, int nindex);
	bool InventorySearchItem(LPOBJ lpObj, BYTE SearchItem);
	bool DeleteItem(LPOBJ lpObj, BYTE Item);
	bool CheckItemReq(LPOBJ lpObj);
	BYTE m_ItemLoad;	
	BOTLOAD Load[MAX_BOTRESETPK];
	BOT_ITEM_REQ Req[MAX_BOTRESETPK];
	
};

extern BotPkClear gBotPkClear;