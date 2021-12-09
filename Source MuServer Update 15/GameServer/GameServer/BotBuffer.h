
#include "User.h"

#if(BOT_BUFFER == 1)

#define MAX_BOTBUFFER	30
#define MAX_BOTBUFFERSKILLS	5


struct BotBufferBodyItems
{
	int num;
	int level;
	int opt;
	int Enabled;
};

struct botBufferSkill
{
	int BotIndex;
	WORD skill;
	int time;
	int power;
	
};

struct botBufferStruct
{
	int Class;
	char Name[11];
	BYTE Map;
	BYTE X;
	BYTE Y;
	BYTE Dir;
	int OnlyVip;
	int ActiveGensFamily;
	int ContributionGens;
	int GensFamily;
	int Enabled;
	int Zen;
	int PCPoints;
	int ChangeColorName;
	int index;
	WORD MaxLevel;
	BotBufferBodyItems body[9];
	botBufferSkill skill[MAX_BOTBUFFERSKILLS];
	BYTE SkillCount;
};

class ObjBotBuffer
{
public:
	int Enabled;
	void Read(char * path);
	void MakeBot();
	BOOL TradeOpen(int index,int nindex);
	botBufferStruct bot[MAX_BOTBUFFER];
	short Max_Normal_Level;
	BYTE gObjInventoryInsertItemPos(int aIndex, CItem item, int pos, BOOL RequestCheck);

private:
	int GetBotIndex(int aIndex);
};

extern  ObjBotBuffer ObjBotBuff;

#endif