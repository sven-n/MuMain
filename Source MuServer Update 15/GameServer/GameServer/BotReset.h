// 
// -		New BotReset AOG-Team # (By Callejero)
// 

#include "User.h"
#include "Protocol.h"

#define MAX_BOTR 1
#define BOT_MAX_ITEM 5
#define BOT_MESSAGE 2


struct BotResetItems
{
	int Type;
	int level;
	int opt;
	int Enabled;
};

struct BOT_LOAD_CFG
{
	int Class;
	int Vip;
	int ActiveGensFamily;
	int ReqContribution;
	int GensSystem;
	bool Enabled;
	int ColorName;
	char Name[11];
	BYTE Map;
	BYTE X;
	BYTE Y;
	BYTE Dir;
	int index;
	BotResetItems Bot_Item[9];
	
};

struct BOT_REQ_ITEM
{
	int ResetType;
	short levels;
	int reset;
	int Zen;
	int PCP;
	int ItemReq;
	int Num;
	int ItemType;
	int ItemIndex;
	int ItemMinLevel;
	int ItemMaxLevel;
	int ItemDur;
	
};

struct BOT_ADD_POINT
{
	int AddMasterP;
	int MulMasterP;
};

struct BOT_ADD_ITEM
{
	WORD iNum;
	int ItemReward;
	int ItemWinType;
	int ItemWinIndex;
	int ItemWinLevel;
	int ItemWinDur;
	int ItemWinLuck;
	int ItemWinSkill;
	int ItemWinOpt;
	int ItemWinExc;
	int WCoinCWin;
	int WCoinPWin;
	int GoblinPointWin;
};

struct MarrySystem
{

	int MarryActive;
	int MarryMulLevelUp;
	int MarryMulMoney;
};

class BotReset
{
public:
	BotReset();
	~BotReset();
	bool Enabled;
	void Init();
	void LoadData(char * FilePath);
	void MakeBot();
	BOOL TradeOpen(int index,int nindex);
	bool InventorySearchItem(LPOBJ lpObj, BYTE SItemID);
	bool DeleteItem(LPOBJ lpObj, BYTE SItemID);
	bool CheckItemReq(LPOBJ lpObj);
	void AddResetItem(int iUser,BYTE SocketBonus,LPBYTE SocketOptions);
	void MessageTime();
	

	BOT_LOAD_CFG BotLoad[MAX_BOTR];
	BOT_REQ_ITEM Item_Req[BOT_MAX_ITEM];
	BOT_ADD_POINT Add_Point[MAX_BOTR];
	BOT_ADD_ITEM Add_Item[MAX_BOTR];
	MarrySystem MarryReq[MAX_BOTR];

private:
	int GetBotIndex(int aIndex);
	BYTE m_ItemLoaded;

};

extern BotReset gBotReset;