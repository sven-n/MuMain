#include "user.h"
#include "stdafx.h"

#if (BOT_STORE == 1)

#define MAX_BOTSTORE	50

struct BotStoreBodyItems
{
	int num;
	int level;
	int opt;
	bool Enabled;
};

struct BotStoreItems
{
	int ValueZen;
	int ValueSoul;
	int ValueBless;
	int ValueChaos;
	int PCPoint;
	WORD num;
	short Level;
	BYTE Opt;
	BYTE Luck;
	BYTE Skill;
	float Dur;
	BYTE Exc;
	BYTE Anc;
	BYTE Sock[5];
};

struct botStoreStruct
{
	int index;
	int Class;
	int OnlyVip;
	int UseVipMoney;
	char Name[11];
	char StoreName[11];
	BYTE Map;
	BYTE X;
	BYTE Y;
	BYTE Dir;
	bool Enabled;
	BotStoreBodyItems body[9];
	BotStoreItems storeItem[INVENTORY_SIZE-MAIN_INVENTORY_NORMAL_SIZE];
	BYTE ItemCount;
};

class ObjBotStore
{
public:
	bool Enabled;
	void Read(char * FilePath);
	void MakeBot();
	void AddItem(int bIndex,int botNum);
	BYTE CheckSpace(LPOBJ lpObj, int type, BYTE * TempMap);
	botStoreStruct bot[MAX_BOTSTORE];
};
extern ObjBotStore BotStore;

#endif