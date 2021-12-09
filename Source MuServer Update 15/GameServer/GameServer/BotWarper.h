#include "user.h"
#include "stdafx.h"

#if (BOT_WARPER==1)

#define MAX_BOTWARPER		10

struct BotWarperBodyItems
{
	int num;
	BYTE level;
	BYTE opt;
	bool Enabled;
};

struct botWarperStruct
{
	int Class;
	BYTE OnlyVip;
	int Zen;
	int PCPoints;
	int MinLevel;
	int MinReset;
//	int PcPoint;
	char Name[11];
	BYTE Map;
	BYTE X;
	BYTE Y;
	BYTE Dir;
	int ActiveGensFamily;
	int ReqContribution;
	int GensFamily;
	bool Enabled;
	int ChangeColorName;
	int index;
	BotWarperBodyItems body[9];

	BYTE Warp_Map;
	BYTE Warp_X;
	BYTE Warp_Y;
};

class ObjBotWarper
{
public:
	bool Enabled;
	void Init();
	void Read(char * FilePath);
	void MakeBot();
	BOOL TradeOpen(int index,int nindex);
	botWarperStruct bot[MAX_BOTWARPER];
private:
	int GetBotIndex(int aIndex);
};
extern ObjBotWarper BotWarper;

#endif