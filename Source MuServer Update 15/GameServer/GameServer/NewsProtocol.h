/*********************/
/*  CUSTOM_PROTOCOL	 */
/*********************/

#include "User.h"

BYTE gObjTempInventoryRectCheck(BYTE * TempMap, int sx, int sy, int width, int height);
void PHeadSubSetW(LPBYTE lpBuf, BYTE head, BYTE sub, int size);

BYTE BoxExcOptions(int maxOptions);
BYTE gObjInventoryInsertItem(int aIndex, CItem item);
unsigned char gObjTempInventoryInsertItem(LPOBJ lpObj, int itemType, BYTE * TempMap);
void GCMagicAttackNumberSend(LPOBJ lpObj, WORD MagicNumber, int usernumber,  unsigned char skillsuccess);

BYTE GETHEIGHT(int aIndex);
//unsigned char gObjShopBuyInventoryInsertItem(LPOBJ lpObj, int type, int index, int level, int iSerial, int iDur);
unsigned char gObjOnlyInventoryRectCheck(int aIndex, int sx, int sy, int width, int height);
unsigned char gObjShopBuyInventoryInsertItem(int aIndex, class CItem item);
void ItemByteConvert(unsigned char* buf, CItem item);
void GCInventoryItemOneSend(int aIndex, int pos);
BYTE gObjInventoryInsertItemBot(LPOBJ lpObj, int type, int index, int level, int iSerial, int iDur, int ItemSlot1, int ItemSlot2, int ItemSlot3, int ItemSlot4, int ItemSlot5);
void MakeNoticeMsg(void * lpNotice, BYTE btType, char * szNoticeMsg);






