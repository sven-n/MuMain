

#if (BOT_ALCHEMIST == 1)

#define MAX_BOTALCHEMIST	30

struct BotAlchemistBodyItems
{
	int num;
	int level;
	int opt;
	int IsMuun;
	bool Enabled;
};

struct botAlchemistStruct
{
	int index;
	BYTE Class;
	int OnlyVip;
	int Zen;
	int PCPoints;
	int ContributionGens;
	int ActiveGensFamily;
	BOOL OnlySameType;
	BOOL OnlyLowerIndex;
	BOOL AcceptAncient;
	BYTE MaxLevel;
	BYTE MaxExc;
	BYTE Rate;
	char Name[11];
	BYTE Map;
	BYTE X;
	BYTE Y;
	BYTE Dir;
	int GensFamily;
	bool Enabled;
	int ChangeColorName;
	BotAlchemistBodyItems body[9];
	//-
	BOOL AllowLevel;
	BOOL AllowOpt;
	BOOL AllowLuck;
	BOOL AllowSkill;
	BOOL AllowExc;
	BOOL AllowFFFF;
};

class ObjBotAlchemist
{
public:
	void Read(char * FilePath);
	void MakeBot();
	int GetBotIndex(int aIndex);
	BOOL IsInTrade(int BotIndex);
	BOOL TradeOpen(int index, int nindex);
	void TradeOk(int aIndex);
	void TradeCancel(int aIndex);

private:
	bool Enabled;
	bool AllowExc(BYTE BotNum, BYTE ExcOpt);
	BYTE Alchemy(int aIndex,int BotNum);
	botAlchemistStruct bot[MAX_BOTALCHEMIST];
};
extern ObjBotAlchemist BotAlchemist;

#endif