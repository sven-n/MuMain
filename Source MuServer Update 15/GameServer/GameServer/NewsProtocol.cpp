#include "stdafx.h"
#include "NewsProtocol.h"
#include "ItemManager.h"
#include "Util.h"
#include "MonsterSetBase.h"
#include "MapServerManager.h"
#include "ChaosCastle.h"
#include "Monster.h"
#include "Item.h"
#include "ObjectManager.h"
#include "Notice.h"
#include "PG_Custom.h"

void PHeadSubSetW(LPBYTE lpBuf, BYTE head, BYTE sub,  int size)
{
	lpBuf[0]=0xC2;	// packet header
	lpBuf[1]=SET_NUMBERHW(size);
	lpBuf[2]=SET_NUMBERLW(size);
	lpBuf[3]=head;
	lpBuf[4]=sub;
}
BYTE gObjTempInventoryRectCheck(BYTE * TempMap, int sx, int sy, int width, int height)
{
	int x,y,blank = 0;
	int S6E2;

	if(sx + width > 8)
	{
		return -1;
	}
	if(sy + height > 8)
	{
		return -2;
	}

	if(S6E2 == 1)
	{
		if(sy < 8 && sy+height > 8 )
		{
			return -1;
		}

		if(sy > 7 && sy < 12 && sy+height > 12)
		{
			return -1;
		}

		if(sy > 11 && sy < 16 && sy+height > 16)
		{
			return -1;
		}

		if(sy > 16 && sy < 20 && sy+height > 20)
		{
			return -1;
		}

		if(sy > 20 && sy < 24 && sy+height > 24)
		{
			return -1;
		}

		if(sy > 24 && sy < 28 && sy+height > 28)
		{
			return -1;
		}
	}

	for(y = 0; y < height; y++)
	{
		for(x = 0; x < width; x++)
		{
			if(*(BYTE*)(TempMap + (sy+y)*8 + (sx+x))!= 255)
			{
				blank += 1;
				return -1;
			}
		}
	}
	if(blank == 0)
	{
		return sx+sy*8+INVENTORY_WEAR_SIZE;
	}
	return -1;
}

/****************************************************************************************/

struct PMSG_EQUIPMENTLIST
{
	PBMSG_HEAD h;	// C1:F3:13
	BYTE subcode;	// 3
	BYTE NumberH;	// 4
	BYTE NumberL;	// 5
	BYTE Equipment[18];	// 6 //CHAR_SET_SIZE

};

BYTE BoxExcOptions(int maxOptions)
{
	BYTE NumberOfOptions = 0;
	BYTE RetOption = 0, TempOption = 0, deadlock = 15;
	BYTE ExcOrgArr[6];

	if (maxOptions > 0)
	{
		if (maxOptions > 6)
			NumberOfOptions=6;
		else
			NumberOfOptions=rand()%maxOptions + 1;

		//Exc Options IDs
		ExcOrgArr[0]=8;
		ExcOrgArr[1]=16;
		ExcOrgArr[2]=2;
		ExcOrgArr[3]=4;
		ExcOrgArr[4]=32;
		ExcOrgArr[5]=1;

		if (NumberOfOptions >= 6)
		{
			RetOption = ExcOrgArr[0]+ExcOrgArr[1]+ExcOrgArr[2]+ExcOrgArr[3]+ExcOrgArr[4]+ExcOrgArr[5];
			return RetOption;
		}

		if (NumberOfOptions == 5)
		{
			TempOption = ExcOrgArr[rand()%6];

			RetOption = ExcOrgArr[0]+ExcOrgArr[1]+ExcOrgArr[2]+ExcOrgArr[3]+ExcOrgArr[4]+ExcOrgArr[5]-TempOption;
			return RetOption;
		}

		if (NumberOfOptions == 4)
		{
			RetOption = ExcOrgArr[0]+ExcOrgArr[1]+ExcOrgArr[2]+ExcOrgArr[3]+ExcOrgArr[4]+ExcOrgArr[5];
			while(true)
			{		
				TempOption = ExcOrgArr[rand()%6];

				if ( (RetOption & TempOption) == TempOption )
				{
					RetOption -= TempOption;
					NumberOfOptions += 1;
				}

				deadlock -= 1;
				if ( NumberOfOptions == 6 || deadlock == 0 )
					break;
			}
			return RetOption;
		}

		if (NumberOfOptions == 3)
		{
			RetOption = 0;
			while(true)
			{		
				TempOption = ExcOrgArr[rand()%6];

				if ( (RetOption & TempOption) != TempOption )
				{
					RetOption += TempOption;
					NumberOfOptions -= 1;
				}

				deadlock -= 1;
				if ( NumberOfOptions == 0 || deadlock == 0 )
					break;
			}
			return RetOption;
		}

		if (NumberOfOptions == 2)
		{
			RetOption = 0;
			while(true)
			{		
				TempOption = ExcOrgArr[rand()%6];

				if ( (RetOption & TempOption) != TempOption )
				{
					RetOption += TempOption;
					NumberOfOptions -= 1;
				}

				deadlock -= 1;
				if ( NumberOfOptions == 0 || deadlock == 0 )
					break;
			}
			return RetOption;
		}

		if (NumberOfOptions == 1)
		{
			RetOption = ExcOrgArr[rand()%6];
			return RetOption;
		}
	}
	return RetOption;
}

LPSTR CItem::GetName()
{
	return &ItemAttribute[this->m_Serial].Name[0];
}

struct SDHP_ITEMCREATE
{
	PBMSG_HEAD h;	// C1:XX
	BYTE x;	// 3
	BYTE y;	// 4
	BYTE MapNumber;	// 5 [235:Mu_2nd_Aniv], [236:CashShop]
	short Type;	// 6
	BYTE Level;	// 8
	BYTE Dur;	// 9
	BYTE Op1;	// A
	BYTE Op2;	// B
	BYTE Op3;	// C
	BYTE NewOption;	// D
	int aIndex;	// 10
	short lootindex;	// 14
	BYTE SetOption;	// 16
	BYTE c;
	BYTE SocketBonus;
	BYTE SocketOptions[5];
};
#include "SocketItemType.h"

/**************************************************************************************/

BYTE gObjTempInventoryInsertItem(LPOBJ lpObj, int itemType, BYTE * TempMap)
{
	int w,h,iwidth,iheight;
	BYTE blank = 0;

	
	iwidth=ItemAttribute[itemType].Width;
	iheight=ItemAttribute[itemType].Height;

	for(h = 0; h < 8; h++)
	{
		for(w = 0; w < 8; w++)
		{
			if(*(BYTE*)(TempMap + h * 8 + w) == 255)
			{
				blank = gObjTempInventoryRectCheck(TempMap,w,h,iwidth,iheight);

				if(blank == 254)
				{
					goto GOTO_EndFunc;
				}
				if(blank != 255)
				{
					gObjTempInventoryItemBoxSet(TempMap,blank,iwidth,iheight,itemType);
					return blank;
				}
			}
		}
	}
GOTO_EndFunc:
	return -1;
}

BYTE GETHEIGHT(int aIndex)
{
	int S6E2;

	if(OBJMAX_RANGE(aIndex) == false)
		return BASIC_INV_HEIGHT;

	if (aIndex < OBJECT_START_USER || aIndex > MAX_OBJECT)
		return BASIC_INV_HEIGHT;

	if(S6E2 == 1)
	{
		if(gObj[aIndex].ExInventory >= 1 && gObj[aIndex].ExInventory <= MAX_EXTENDED_INV)
			return BASIC_INV_HEIGHT + (EXTENDED_INV_HEIGHT*gObj[aIndex].ExInventory); 
	}
	return BASIC_INV_HEIGHT;
}

BYTE gObjShopBuyInventoryInsertItem(int aIndex, CItem item)
{
	int w,h,iwidth,iheight;
	BYTE blank = 0;

	if(item.GetSize(iwidth,iheight)==0)
	{
		//LogAdd(lMsg.Get(527),__FILE__,__LINE__);
		return -1;
	}

	for(h = 0; h < 8; h++)
	{
		for( w = 0; w < 8; w++)
		{
			if(*(BYTE*)(gObj[aIndex].InventoryMap+h*8+w) == 255)
			{
				blank = gObjOnlyInventoryRectCheck(aIndex,w,h,iwidth,iheight);

				if(blank == 254)
				{
					goto NEXT1;
				}

				if(blank != 255)
				{
					gObj[aIndex].Inventory[blank] = item;

					gObj[aIndex].Inventory[blank].m_Number = 0;
					
					gObjInventoryItemSet(aIndex,blank,gObj[aIndex].Inventory[blank].m_Serial);
					return blank;
				}
			}
		}
	}
NEXT1:
	if(IS_EXTENDED_INV(aIndex,1) == TRUE)
	{	
		for(h = 8; h < 12; h++)
		{
			for( w = 0; w < 8; w++)
			{
				if(*(BYTE*)(gObj[aIndex].InventoryMap+h*8+w) == 255)
				{
					blank = gObjOnlyInventoryRectCheck(aIndex,w,h,iwidth,iheight);

					if(blank == 254)
					{
						goto NEXT2;
					}

					if(blank != 255)
					{
						gObj[aIndex].Inventory[blank] = item;

						gObj[aIndex].Inventory[blank].m_Number = 0;
						
						gObjInventoryItemSet(aIndex,blank,gObj[aIndex].Inventory[blank].m_Index);
						return blank;
					}
				}
			}
		}
	}
NEXT2:
	if(IS_EXTENDED_INV(aIndex,2) == TRUE)
	{	
		for(h = 12; h < 16; h++)
		{
			for( w = 0; w < 8; w++)
			{
				if(*(BYTE*)(gObj[aIndex].InventoryMap+h*8+w) == 255)
				{
					blank = gObjOnlyInventoryRectCheck(aIndex,w,h,iwidth,iheight);

					if(blank == 254)
					{
						goto GOTO_EndFunc;
					}

					if(blank != 255)
					{
						gObj[aIndex].Inventory[blank] = item;

						gObj[aIndex].Inventory[blank].m_Number = 0;
						
						gObjInventoryItemSet(aIndex,blank,gObj[aIndex].Inventory[blank].m_Index);
						return blank;
					}
				}
			}
		}
	}
GOTO_EndFunc:
	return -1;
}

BYTE gObjOnlyInventoryRectCheck(int aIndex, int sx, int sy, int width, int height)
{
	int x,y;
	int blank = 0;
	int InventoryRows = 12;
	int S6E2;
	InventoryRows =	GETHEIGHT(aIndex);

	if(sx + width > 8)
	{
		return -1;
	}

	if(sy + height > InventoryRows)
	{
		return -2;
	}

	if(S6E2 == 1 && height > 1)
	{
		if(sy < 8 && sy+height > 8 )
		{
			return -1;
		}

		if(sy > 7 && sy < 12 && sy+height > 12)
		{
			return -1;
		}

		if(sy > 11 && sy < 16 && sy+height > 16)
		{
			return -1;
		}

		if(sy > 16 && sy < 20 && sy+height > 20)
		{
			return -1;
		}

		if(sy > 20 && sy < 24 && sy+height > 24)
		{
			return -1;
		}
	}

	int xx,yy;
	for(y = 0; y < height; y ++)
	{

		yy = sy+y;

		for(x = 0; x < width; x++)
		{
			xx = sx + x;

			if(ExtentCheck(xx,yy,8,InventoryRows)==1)
			{
				if(*(BYTE*)(gObj[aIndex].InventoryMap+(sy+y)*8+(sx+x)) != 255)
				{
					blank += 1;
					return -1;
				}
			}
			else
			{
				LogAdd(LOG_BOT,"error : %s %d",__FILE__,__LINE__);
				return -1;
			}
		}
	}

	if(blank == 0)
	{
		return sx+sy*8+12;
	}
	return  -1;
}


void ItemByteConvert(unsigned char* buf, CItem item)
{
	int n=0;

	memset(buf, 0, 7+5);
	buf[n] = item.m_Index & 0xFF;
	n++;
	buf[n] = 0;
	buf[n] |= item.m_Level * 8;
	buf[n] |= item.m_Option1 * 128;
	buf[n] |= item.m_Option2 * 4;
	buf[n] |= item.m_Option3 & 3;
	n++;
	buf[n] = item.m_Durability;
	n++;
	buf[n] = 0;
	buf[n] |= ((item.m_Index & 0x100) >> 1);

	if ( item.m_Option3 > 3 )
	{
		buf[n] |= 0x40; // item +16 option
	}

	buf[n] |= item.m_NewOption;
	n++;
	buf[n] = item.m_SetOption;
	n++;
	buf[n] |= (item.m_Index & 0x1E00 ) >> 5;

	BYTE btItemEffectFor380 = 0;

#if(ENABLE_CSHOP == 0)
	btItemEffectFor380 = (item.m_ItemOptionEx & 0x80 ) >> 4;
#else
	BYTE btItemType=0;
	btItemType |= (item.m_Type & 0x1E00 ) >> 5;

	BYTE tmp = item.m_ItemOptionEx & 2;
	if((tmp == 2) && ((btItemType+2) == item.m_ItemOptionEx))
		btItemEffectFor380 = 2;
	else
		btItemEffectFor380 = (item.m_ItemOptionEx & 0x80 ) >> 4;
#endif

	buf[n] |= btItemEffectFor380;
	n++;
	buf[n] = item.m_JewelOfHarmonyOption;

	n++;
	buf[n] = item.m_SocketOption[0]-1;
	n++;
	buf[n] = item.m_SocketOption[1]-1;
	n++;
	buf[n] = item.m_SocketOption[2]-1;
	n++;
	buf[n] = item.m_SocketOption[3]-1;
	n++;
	buf[n] = item.m_SocketOption[4]-1;		
}

struct PMSG_INVENTORYITEMMODIFY
{
	PBMSG_HEAD h;	// C1:F3:14
	BYTE subcode;	// 3
	BYTE Pos;	// 4
	BYTE ItemInfo[MAX_ITEM_INFO];	// 5

};


void GCInventoryItemOneSend(int aIndex, int pos)
{
	if(OBJMAX_RANGE(aIndex) == 0)
	{
		return;
	}

	if ( !gObj[aIndex].Inventory[pos].IsItem())
		return;

	PMSG_INVENTORYITEMMODIFY pMsg;

	PHeadSubSetB((LPBYTE)&pMsg, 0xF3, 0x14, sizeof(pMsg));
	pMsg.Pos = pos;
	ItemByteConvert(pMsg.ItemInfo, gObj[aIndex].Inventory[pos]);

	DataSend(aIndex, (UCHAR *)&pMsg, pMsg.h.size);
}




BYTE gObjInventoryInsertItemBot(LPOBJ lpObj, int type, int index, int level, int iSerial, int iDur, int ItemSlot1, int ItemSlot2, int ItemSlot3, int ItemSlot4, int ItemSlot5)
{
	CItem item;
	int w,h,iwidth,iheight;
	BYTE blank = 0;

	int item_type = ItemGetNumberMake(type,index);
	item.m_Level = level;
	item.m_Durability = iDur;
	item.m_SocketOption[0] = ItemSlot1;
	item.m_SocketOption[1] = ItemSlot2;
	item.m_SocketOption[2] = ItemSlot3;
	item.m_SocketOption[3] = ItemSlot4;
	item.m_SocketOption[4] = ItemSlot5;

	item.Convert(item_type,0,0,0,0,0,0,0,0,CURRENT_DB_VERSION);

	if(item.GetSize((int &)iwidth,(int &)iheight)==0)
	{
		//LogAdd(lMsg.Get(527),__FILE__,__LINE__);
		return -1;
	}

	for(h = 0; h < 8; h++)
	{
		for(w = 0; w < 8; w++)
		{
			if(*(BYTE*)(lpObj->InventoryMap+h*8+w) == 255)
			{
				blank = gObjInventoryRectCheck(lpObj->Index,w,h,iwidth,iheight);

				if(blank == 254)
				{
					goto NEXT1;
				}

				if(blank != 255)
				{
					lpObj->Inventory[blank] = item;
					lpObj->Inventory[blank].m_Serial = iSerial;

					gObjInventoryItemSet(lpObj->Index,blank,lpObj->Inventory[blank].m_Index);
					return blank;
				}
			}
		}
	}
NEXT1:
	if(IS_EXTENDED_INV(lpObj->Index,1) == TRUE)
	{
		for(h = 8; h < 12; h++)
		{
			for(w = 0; w < 8; w++)
			{
				if(*(BYTE*)(lpObj->InventoryMap+h*8+w) == 255)
				{
					blank = gObjInventoryRectCheck(lpObj->Index,w,h,iwidth,iheight);

					if(blank == 254)
					{
						goto NEXT2;
					}

					if(blank != 255)
					{
						lpObj->Inventory[blank] = item;
						lpObj->Inventory[blank].m_Serial = iSerial;

						gObjInventoryItemSet(lpObj->Index,blank,lpObj->Inventory[blank].m_Index);
						return blank;
					}
				}
			}
		}
	}
NEXT2:
	if(IS_EXTENDED_INV(lpObj->Index,2) == TRUE)
	{
		for(h = 12; h < 16; h++)
		{
			for(w = 0; w < 8; w++)
			{
				if(*(BYTE*)(lpObj->InventoryMap+h*8+w) == 255)
				{
					blank = gObjInventoryRectCheck(lpObj->Index,w,h,iwidth,iheight);

					if(blank == 254)
					{
						goto GOTO_EndFunc;
					}

					if(blank != 255)
					{
						lpObj->Inventory[blank] = item;
						lpObj->Inventory[blank].m_Serial = iSerial;

						gObjInventoryItemSet(lpObj->Index,blank,lpObj->Inventory[blank].m_Index);
						return blank;
					}
				}
			}
		}
	}
GOTO_EndFunc:
	return -1;
}



