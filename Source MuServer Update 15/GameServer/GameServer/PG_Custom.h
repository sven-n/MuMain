/* PG_CUSTOM */

#include "User.h"


extern BOOL gItemSerialCheck;
extern BOOL gItemZeroSerialCheck;

class YolaCustom
{
public:
	int PCShop_MonsterDieAddPoints;	// meter que se configure de algun lado!
	int g_RenaCount;	// GOLDEN ARCHER :D
};

extern YolaCustom ObjYola;

int getNumberOfExcOptions(int checksum);
BOOL IS_EXTENDED_INV(int aIndex, BYTE INVNUM);
BYTE gObjInventoryRectCheck(int aIndex, int sx, int sy, int width, int height);
BOOL ExtentCheck(int x, int y, int w, int h);
BYTE gObjInventoryInsertItem(int aIndex, CItem item);
int getNumberOfExcOptions(int checksum);
void gObjTradeCancel(int aIndex);
void gObjCharTradeClear(LPOBJ lpObj);
void GCEquipmentSend(int aIndex);
void gObjInventoryItemBoxSet(int aIndex, int itempos, int xl, int yl, BYTE set_byte);
void PHeadSubSetB(LPBYTE lpBuf, BYTE head, BYTE sub, int size);
void gObjInventoryItemSet(int aIndex, int itempos, BYTE set_byte);
void GCMagicAttackNumberSend(LPOBJ lpObj, WORD MagicNumber, int usernumber,  unsigned char skillsuccess);
void gObjTempInventoryItemBoxSet(BYTE * TempMap, int itempos, int xl, int yl, BYTE set_byte);
BOOL SetPosMonsterSkyEvent(int aIndex, int PosTableNum);
BOOL gObjIsNewClass(LPOBJ lpObj);

BOOL DeleteItemByLevel( int iIndex, DWORD itemId, BYTE ItemLevel);
int gObjGetIndex(char* szId);
int gObjMonsterAdd(WORD Type, BYTE Map, BYTE X, BYTE Y);
int ItemGetNumberMake(int type, int index);

void ItemSerialCreateSend(int aIndex, BYTE MapNumber, BYTE x, BYTE y,int type, BYTE level, BYTE dur, BYTE Op1, BYTE Op2, BYTE Op3,int LootIndex, BYTE NewOption, BYTE SetOption,BYTE SocketBonus,LPBYTE SocketOptions);
int ItemGetDurability(int index, int itemLevel, int ExcellentItem, int SetItem);

int gObjCheckSerial0ItemList(class CItem* lpItem);

unsigned char gObjInventoryInsertItemPos(int aIndex, class CItem item, int pos, int RequestCheck);