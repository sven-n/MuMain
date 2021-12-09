#include "stdafx.h"
#include "PG_Custom.h"
#include "ItemManager.h"
#include "Util.h"
#include "MonsterSetBase.h"
#include "MapServerManager.h"
#include "ChaosCastle.h"
#include "Monster.h"
#include "Item.h"
#include "SocketItemType.h"
#include "ObjectManager.h"

#if (PG_CUSTOM == 1)

YolaCustom ObjYola;

void PHeadSubSetB(LPBYTE lpBuf, BYTE head, BYTE sub, int size) //OK BUSCAR SU FUNCION CORRESPONDIENTE
{
	lpBuf[0] =0xC1;	// Packets
	lpBuf[1] =size;
	lpBuf[2] =head;
	lpBuf[3] =sub;
}

BYTE gObjInventoryInsertItemPos(int aIndex, CItem item, int pos, BOOL RequestCheck) //OK
{

	if(OBJECT_RANGE(aIndex) == 0)
	{
		return -1;
	}

	/*if(pos < 0 || pos > INVENTORY_SIZE_CUSTOM(aIndex,false))
	{
		return -1;
	}*/

	LPOBJ lpObj;

	int useClass = -1;

	lpObj = &gObj[aIndex];

	if(lpObj->Inventory[pos].IsItem() == 1)
	{
		return -1;
	}

	if(item.IsItem() == 0)
	{
		return -1;
	}

	if(pos < 12)
	{
		if(lpObj->IsBot == 0)
		{
			useClass = item.IsClassBot(lpObj->Class,lpObj->ChangeUp);

			if(useClass == -1)
			{
				LogAdd(LOG_BLUE,"[Using Class Error] Error UseClass %s",useClass);
				return -1;
			}

		}
	}

	lpObj->Inventory[pos] = item;

	return pos;
}

void gObjTempInventoryItemBoxSet(BYTE * TempMap, int itempos, int xl, int yl, BYTE set_byte)
{
	int itemposx = (itempos - INVENTORY_WEAR_SIZE) % 8;
	int itemposy = (itempos - INVENTORY_WEAR_SIZE) / 8;

	for(int y = 0 ; y < yl ; y++)
	{
		for(int x = 0; x < xl; x++)
		{
			*(BYTE*)(TempMap + (itemposy +y) * 8 + (itemposx + x)) = set_byte;
		}
	}
}

BOOL SetPosMonsterSkyEvent(int aIndex, int PosTableNum)	//OK ?
{
	if ( (  (aIndex<0)? FALSE: (aIndex > MAX_OBJECT-1)? FALSE: TRUE ) == FALSE )
	{
			LogAdd(LOG_RED,"error : %s %d", __FILE__, __LINE__);
		return FALSE;
	}

	
	if ( (  (PosTableNum<0)? FALSE: (PosTableNum > MAX_OBJECT_MONSTER-1)? FALSE: TRUE ) == FALSE )
	{
		LogAdd(LOG_RED,"error : %s %d", __FILE__, __LINE__);
		return FALSE;
	}

	LPOBJ lpObj = &gObj[aIndex];
	lpObj->PosNum = PosTableNum;
	lpObj->X = gMonsterSetBase.m_Mp[PosTableNum].X;
	lpObj->Y = gMonsterSetBase.m_Mp[PosTableNum].Y;
	lpObj->Map = gMonsterSetBase.m_Mp[PosTableNum].Map;

	lpObj->TX = lpObj->X;
	lpObj->TY = lpObj->Y;
	lpObj->OldX = lpObj->X;
	lpObj->OldY = lpObj->Y;
	lpObj->Dir = gMonsterSetBase.m_Mp[PosTableNum].Dir;
	lpObj->StartX = lpObj->X;
	lpObj->StartY = lpObj->Y;

	if (lpObj->Class == 44 ||  
		lpObj->Class == 53 ||  
		lpObj->Class == 54 ||  
		lpObj->Class == 55 ||  
		lpObj->Class == 56 ||
		lpObj->Class == 492)
	{

	}
	else if (  lpObj->Class >= 78 &&  lpObj->Class <= 83)
	{

	}

	return true;
}

BOOL gObjIsNewClass(LPOBJ lpObj)	//SkyEvent
{
	int Class = lpObj->DBClass;

	if( Class == DB_GRAND_MASTER  || 
		Class == DB_BLADE_MASTER || 
		Class == DB_HIGH_ELF || 
		Class == DB_DUEL_MASTER ||			//MG
		Class == DB_DUEL_MASTER+1 ||		//MG
		Class == DB_LORD_EMPEROR ||			//DL
		Class == DB_LORD_EMPEROR+1 ||		//DL
		Class == DB_DIMENSION_MASTER ||		//SUM
		Class == DB_RAGEFIGHER_EVO ||		//RF
		Class == DB_RAGEFIGHER_EVO+1)		//RF
	{
		return 1;
	}else
	{
		return 0;
	}
}

BOOL DeleteItemByLevel (int iIndex, DWORD itemId, BYTE ItemLevel)	//OK
{
	if(OBJECT_RANGE(iIndex) == 0)
	{
		return false;
	}

	for ( BYTE ItemPos = INVENTORY_WEAR_SIZE ; ItemPos < INVENTORY_MAIN_SIZE; ItemPos ++ )
	{
		if ( gObj[iIndex].Inventory[ItemPos].m_Index == itemId )
		{
			if (  gObj[iIndex].Inventory[ItemPos].m_Level == ItemLevel )
			{
				gItemManager.InventoryDelItem(iIndex,ItemPos);
				gItemManager.GCItemDeleteSend(iIndex, ItemPos, 1);
				return 1;
			}
		}
	}
	return 0;
}

int gObjMonsterAdd(WORD Type, BYTE Map, BYTE X, BYTE Y)	//SkyEvent
{
	if ( gMonsterSetBase.m_count >= MAX_OBJECT )
	{
//		ErrorMessageBox("Monster attribute max over %s %d", __FILE__, __LINE__);
		return -1;
	}

	if (gMapServerManager.CheckMapServer(Map) == FALSE )
	{
		LogAdd(LOG_BLACK,"[MonsterAdd] No need to load monster for map %d [%d,%d,%d]",Map,Type,X,Y);
		return -1;
	}

	gMonsterSetBase.m_count++;
	gMonsterSetBase.m_Mp[gMonsterSetBase.m_count].Dis			= 30;
	gMonsterSetBase.m_Mp[gMonsterSetBase.m_count].Type			= Type;
	gMonsterSetBase.m_Mp[gMonsterSetBase.m_count].Map			= Map;
	gMonsterSetBase.m_Mp[gMonsterSetBase.m_count].TY			= X;
	gMonsterSetBase.m_Mp[gMonsterSetBase.m_count].TX			= Y;
	gMonsterSetBase.m_Mp[gMonsterSetBase.m_count].X				= X;
	gMonsterSetBase.m_Mp[gMonsterSetBase.m_count].Y				= Y;
	gMonsterSetBase.m_Mp[gMonsterSetBase.m_count].Value			= 1;	//TEST!!!
	gMonsterSetBase.m_Mp[gMonsterSetBase.m_count].Dir			= 3;

	return gMonsterSetBase.m_count;
}

int ItemGetNumberMake(int type, int index)	//TEST	
{
	int make;

	make = type*MAX_ITEM_TYPE + index;

	if ( make < 0 || make >= MAX_ITEM )
	{
		return -1;
	}

	if (ItemAttribute[make].Width < 1 ||  ItemAttribute[make].Height < 1)
	{
		return -1;
	}
	return make;
}

int gObjGetIndex(char* szId)	//OK BUSCAR SU FUNCION CORRESPONDIENTE
{
	for (int n=OBJECT_START_USER;n<MAX_OBJECT; n++)
	{
		if ( gObj[n].Connected >= OBJECT_ONLINE)
		{
			if ( gObj[n].Name[0] == *szId )
			{
				if ( strcmp(&gObj[n].Name[0] , szId) == 0)
				{
					return n;
				}
			}
		}
	}
	return -1;
}
/****************************************************************************************/
/****************************************************************************************/

struct PMSG_EQUIPMENTLIST
{
	PBMSG_HEAD h;	// C1:F3:13
	BYTE subcode;	// 3
	BYTE NumberH;	// 4
	BYTE NumberL;	// 5
	BYTE Equipment[18];	// 6 //CHAR_SET_SIZE

};

void GCEquipmentSend(int aIndex)
{
	PMSG_EQUIPMENTLIST pMsg;

	PHeadSubSetB((LPBYTE)&pMsg, 0xF3, 0x13, sizeof(pMsg));
	pMsg.NumberH = SET_NUMBERHB(aIndex);
	pMsg.NumberL = SET_NUMBERLB(aIndex);
	gObjectManager.CharacterMakePreviewCharSet(aIndex);
	LPOBJ lpObj = &gObj[aIndex];
	memcpy(pMsg.Equipment, lpObj->CharSet, sizeof(lpObj->CharSet));

	MsgSendV2(lpObj, (LPBYTE)&pMsg, pMsg.h.size);
}
void gObjTradeCancel(int aIndex)
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		//if(ReadConfig.TradeLog == TRUE)
			LogAdd(LOG_BLUE,"Error : index[%d] error %s %d",aIndex,__FILE__,__LINE__);
		//LogAdd("Error : index error %s %d",__FILE__,__LINE__);
		return;
	}
	if(gObj[aIndex].Type != OBJECT_USER || gObj[aIndex].Connected != OBJECT_ONLINE)
	{
		//if(ReadConfig.TradeLog == TRUE)
		//	TRADE_LOG.Output(lMsg.Get(535),gObj[aIndex].AccountID,gObj[aIndex].Name);
		//LogAdd(lMsg.Get(535),gObj[aIndex].AccountID,gObj[aIndex].Name);
		return;
	}
	if(gObj[aIndex].Interface.use != 1)
	{
		//if(ReadConfig.TradeLog == TRUE)
		//	TRADE_LOG.Output(lMsg.Get(536),gObj[aIndex].AccountID,gObj[aIndex].Name);
		//LogAdd(lMsg.Get(536),gObj[aIndex].AccountID,gObj[aIndex].Name);
		return;
	}
	if(gObj[aIndex].Interface.type != 1)
	{
		//if(ReadConfig.TradeLog == TRUE)
		//	TRADE_LOG.Output(lMsg.Get(537),gObj[aIndex].AccountID,gObj[aIndex].Name);
		//LogAdd(lMsg.Get(537),gObj[aIndex].AccountID,gObj[aIndex].Name);
		return;
	}

	gObjInventoryRollback(aIndex);	//OK
	gObj[aIndex].TargetNumber = -1;	//OK
	gObj[aIndex].Interface.use = 0;	//OK
	gObjCharTradeClear(&gObj[aIndex]);	//OK
	GCMoneySend(aIndex,gObj[aIndex].Money);	//NO VISTO
	gItemManager.GCItemListSend(aIndex);	//OK
	GCEquipmentSend(aIndex);	//OK
	//GCMagicListMultiSend(&gObj[aIndex],0);
	//gObjUseSkill.SkillChangeUse(aIndex);

	if(gObj[aIndex].GuildNumber > 0)
	{
	//	GCGuildViewportNowPaint(aIndex,gObj[aIndex].GuildName,0,0);
		gObjNotifyUpdateUnionV1(&gObj[aIndex]);
		gObjNotifyUpdateUnionV2(&gObj[aIndex]);
	}
}

void gObjCharTradeClear(LPOBJ lpObj)
{
	if ( lpObj->Type != OBJECT_USER && lpObj->Type != OBJECT_USER )
	{
		return;
	}

	memset(lpObj->TradeMap, 0xFF , 32); //TRADE_BOX_SIZE

	for ( int i=0 ; i< 32 ; i++)
	{
		lpObj->Trade[i].Clear();
	}

	lpObj->TradeMoney = 0;
	lpObj->TradeOk = false;
}

int getNumberOfExcOptions(int checksum)
{
	int optionscount=0;
	int ExcOrgArr[6];

	ExcOrgArr[0]=1;
	ExcOrgArr[1]=2;
	ExcOrgArr[2]=4;
	ExcOrgArr[3]=8;
	ExcOrgArr[4]=16;
	ExcOrgArr[5]=32;

	if (checksum>0)
	{
		for (int i=0;i<6;i++)
		{
			int and_val=checksum & ExcOrgArr[i];
			if (and_val != 0)
				optionscount+=1;
		}
	}

	return optionscount;
}

DWORD CItem::GetNumber()
{
	__try
	{
		if (this->m_Index < 0)
		{
			return 0;
		}
		if (this->m_IsValidItem == false)
		{
			return 0;
		}

		return this->m_Number;
	}
	__except( EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() )
	{
		return 0;
	}
}

BOOL CItem::GetSize(int & w, int & h)
{
	w=ItemAttribute[this->m_Index].Width  ;
	h=ItemAttribute[this->m_Index].Height  ;

	return 1;
}
void gObjInventoryItemBoxSet(int aIndex, int itempos, int xl, int yl, BYTE set_byte)
{
	int S6E2 =1;
	int InventoryRows = 12;
	if(S6E2 == 1)
		InventoryRows = 28;
			
	int itemposx = (itempos - INVENTORY_WEAR_SIZE)%8;
	int itemposy = (itempos - INVENTORY_WEAR_SIZE)/8;

	int xx,yy;

	for(int y = 0; y < yl; y ++)
	{
		yy = itemposy + y;

		for(int x = 0; x < xl; x++)
		{
			xx = itemposx + x;

			if((ExtentCheck(xx,yy,8,InventoryRows)==1) 
#if (PACK_EDITION>=3)
				|| (gObj[aIndex].IsBot >= 1)
#endif
				)
			{
				*(BYTE*)(gObj[aIndex].InventoryMap + (itemposy + y)*8+(itemposx + x)) = set_byte;
			}
			else
			{
			//	LogAdd("error : %s %d",__FILE__,__LINE__);
				return;
			}
		}
	}
}

void gObjInventoryItemSet(int aIndex, int itempos, BYTE set_byte)
{
	int width;
	int height;
	if(itempos < INVENTORY_WEAR_SIZE)
	{
		return;
	}
	else if(itempos > (INVENTORY_MAIN_SIZE-1))
	{
		return;
	}

	if(gObj[aIndex].Inventory[itempos].GetSize((int&)width,(int &)height)==0)
	{
//		LogAdd(lMsg.Get(527),__FILE__,__LINE__);
		return;
	}

	gObjInventoryItemBoxSet(aIndex,itempos,width,height,set_byte);
}

BOOL gItemSerialCheck=1;
BOOL gItemZeroSerialCheck;

int gObjCheckSerial0ItemList(class CItem* lpItem)
{
	if(gItemSerialCheck == 0)
	{
		return false;
	}
	if(gItemZeroSerialCheck == 0)
	{
		return false;
	}

	if(lpItem->m_Index == GET_ITEM(19,13)
		|| lpItem->m_Index == GET_ITEM(19,14)
		|| lpItem->m_Index == GET_ITEM(19,16)
		|| lpItem->m_Index == GET_ITEM(19,22)
		|| lpItem->m_Index == GET_ITEM(19,15)
		|| lpItem->m_Index == GET_ITEM(19,30)
		|| lpItem->m_Index == GET_ITEM(19,31)
		|| lpItem->m_Index == GET_ITEM(19,31))
	{
		if(lpItem->GetNumber() == 0)
		{
			return true;
		}
	}
	return false;
}

BOOL IS_EXTENDED_INV(int aIndex, BYTE INVNUM)
{
	int S6E2 = 1;
	if(OBJECT_RANGE(aIndex) == false)
		return FALSE;

	if (aIndex < OBJECT_START_USER || aIndex > MAX_OBJECT)
		return FALSE;

	if(S6E2 == 1)
	{
		if(gObj[aIndex].ExInventory >= 1 && INVNUM == 1)
			return TRUE;
		else if(gObj[aIndex].ExInventory == 2 && INVNUM == 2)
			return TRUE;
		else if(gObj[aIndex].ExInventory == 3 && INVNUM == 3)
			return TRUE;
		else if(gObj[aIndex].ExInventory == 4 && INVNUM == 4)
			return TRUE;
	}
	return FALSE;
}

BYTE gObjInventoryRectCheck(int aIndex, int sx, int sy, int width, int height)
{
	int x,y;
	int blank = 0;
	int S6E2 = 1;

	int InventoryRows = 12;
	if(S6E2 == 1)
		InventoryRows = 28;

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

		if(sy > 24 && sy < 28 && sy+height > 28)
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

BOOL ExtentCheck(int x, int y, int w, int h)
{
	if ( (x >=0) && (x<w) && (y >=0) && (y<h) )
	{
		return 1;
	}
	return 0;
}

BYTE gObjInventoryInsertItem(int aIndex, CItem item)
{
	int w,h,iwidth,iheight;
	BYTE blank = 0;

	if(item.GetSize((int&)iwidth,(int&)iheight)==0)
	{
	//	LogAdd(lMsg.Get(527),__FILE__,__LINE__);
		return -1;
	}

	for(h = 0; h < 8; h++)
	{
		for( w = 0; w < 8; w++)
		{
			if(*(BYTE*)(gObj[aIndex].InventoryMap+h*8+w) == 255)
			{
				blank = gObjInventoryRectCheck(aIndex,w,h,iwidth,iheight);

				if(blank == 254)
				{
					goto NEXT1;
				}

				if(blank != 255)
				{
					if(gObjCheckSerial0ItemList(&item)!=0)
					{
				//		MsgOutput(aIndex,lMsg.Get(3354));
						return -1;
					}


					if(gObjInventorySearchSerialNumber(&gObj[aIndex],item.m_Number) == 0)
					{
						return -1;
					}

					gObj[aIndex].Inventory[blank] = item;

					gObjInventoryItemSet(aIndex,blank,gObj[aIndex].Inventory[blank].m_Index);
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
					blank = gObjInventoryRectCheck(aIndex,w,h,iwidth,iheight);

					if(blank == 254)
					{
						goto NEXT2;
					}

					if(blank != 255)
					{
						if(gObjCheckSerial0ItemList(&item)!=0)
						{
						//	MsgOutput(aIndex,lMsg.Get(3354));
							return -1;
						}


						if(gObjInventorySearchSerialNumber(&gObj[aIndex],item.m_Number) == 0)
						{
							return -1;
						}

						gObj[aIndex].Inventory[blank] = item;

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
					blank = gObjInventoryRectCheck(aIndex,w,h,iwidth,iheight);

					if(blank == 254)
					{
						goto GOTO_EndFunc;
					}

					if(blank != 255)
					{
						if(gObjCheckSerial0ItemList(&item)!=0)
						{
					//		MsgOutput(aIndex,lMsg.Get(3354));
							return -1;
						}


						if(gObjInventorySearchSerialNumber(&gObj[aIndex],item.m_Number) == 0)
						{
							return -1;
						}

						gObj[aIndex].Inventory[blank] = item;

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

void ItemSerialCreateSend(int aIndex, BYTE MapNumber, BYTE x, BYTE y, int type, BYTE level, BYTE dur, BYTE Op1, BYTE Op2, BYTE Op3, int LootIndex, BYTE NewOption, BYTE SetOption,BYTE SocketBonus,LPBYTE SocketOptions)
{
	SDHP_ITEMCREATE  pMsg;

	pMsg.h.type = 0xC1;
	pMsg.h.head = 0x52;
	pMsg.h.size = sizeof(pMsg);
	pMsg.MapNumber = MapNumber;
	pMsg.x = x;
	pMsg.y = y;
	pMsg.Type = type;
	pMsg.Level = level;

	if (dur == 0 )
	{
		dur = ItemGetDurability(type, level, NewOption, SetOption);
	}

	pMsg.Dur = dur;
	pMsg.Op1 = Op1;
	pMsg.Op2 = Op2;
	pMsg.Op3 = Op3;
	pMsg.NewOption = NewOption;
	pMsg.aIndex = aIndex;
	pMsg.lootindex = LootIndex;
	pMsg.SetOption = SetOption;

/**********/
		if( SocketOptions != NULL )
	{
		pMsg.SocketBonus = SocketBonus;

		for(int i = 0; i < 5; i++)
		{
			pMsg.SocketOptions[i] = SocketOptions[i];
		}
	}//GetSocketItemMaxSocket
	else if( gSocketItemType.CheckSocketItemType(type) )
	{
		int SocketCount = gSocketItemType.GetSocketItemMaxSocket(type);
		pMsg.SocketBonus = (BYTE)-1;

		LogAdd(LOG_BOT,"[SocketItem] Generate Socket Item ( Socket Slot : %d ) - %s, [%d,%d,%d,%d], Serial : %u, ItemCode : %d, MapNumber : %d(%d/%d)",
			SocketCount,ItemAttribute[type].Name,level,Op1,Op2,Op3,type,MapNumber,x,y);

		for(int i = 0;i < MAX_SOCKET_OPTION;i++)
		{
			if( i < SocketCount )
			{
				pMsg.SocketOptions[i] = (BYTE)-2;
			}
			else
			{
				pMsg.SocketOptions[i] = (BYTE)-1;
			}
		}
	}
	else
	{
		pMsg.SocketBonus = (BYTE)-1;
		for(int i = 0; i < 5; i++)	{
			pMsg.SocketOptions[i] = (BYTE)-1;
		}
	}

/**********/
	if ( OBJECT_RANGE(aIndex) != FALSE  && gObj[aIndex].Type != OBJECT_USER )
	{
		BYTE NewOption[MAX_EXC_OPTION];

		for ( int i=0;i<MAX_EXC_OPTION;i++)
		{
			NewOption[i] = 0;
		}

		if ( (pMsg.NewOption & 0x20) != 0 )
		{
			NewOption[0] = TRUE;
		}
		if ( (pMsg.NewOption & 0x10) != 0 )
		{
			NewOption[1] = TRUE;
		}

		if ( (pMsg.NewOption & 0x8) != 0 )
		{
			NewOption[2] = TRUE;
		}
		if ( (pMsg.NewOption & 0x4) != 0 )
		{
			NewOption[3] = TRUE;
		}

		if ( (pMsg.NewOption & 0x2) != 0 )
		{
			NewOption[4] = TRUE;
		}
		if ( (pMsg.NewOption & 0x1) != 0 )
		{
			NewOption[5] = TRUE;
		}

		LogAdd(LOG_EVENT,"Monster Item Drop Request [%s] [%d][%d][%d] : [%s][%d][%d][%d][%d] Ex:[%d,%d,%d,%d,%d,%d,%d] Set:[%d]",
			gObj[aIndex].Name, MapNumber, pMsg.x, pMsg.y, ItemAttribute[pMsg.Type].Name,
			pMsg.Level, pMsg.Op1,pMsg.Op2, pMsg.Op3, NewOption[0], NewOption[1], NewOption[2],
			NewOption[3], NewOption[4],	NewOption[5], NewOption[6], pMsg.SetOption);
	}

//	cDBSMng.Send( (char*)&pMsg, pMsg.h.size);
}
/**************************************************************************************/






int ItemGetDurability(int index, int itemLevel, int ExcellentItem, int SetItem)
{
	if ( index < 0 || index >= MAX_ITEM_TYPE )
	{
		return 0;
	}

	if ( index == GET_ITEM(14,21) && itemLevel == 3 )	// Mark Lord
	{
		itemLevel=0;
	}
	
	//Kundun Symbol, Piece of Paper, Lucky Peny, Blue Ball Invitation Part
	if ( (index == GET_ITEM(14,29)) || (index == GET_ITEM(14,100)) || (index == GET_ITEM(14,101)) || (index == GET_ITEM(14,110))  || (index == GET_ITEM(14,153)) || (index == GET_ITEM(14,154)) || (index == GET_ITEM(14,155)) || (index == GET_ITEM(14,156)))
	{
			return 1;
	}

	int dur=0;

	if ( itemLevel < 5)
	{
		dur= ItemAttribute[index].Durability + itemLevel;
	}
	else if ( itemLevel >= 5 )
	{
		if ( itemLevel == 10 )
		{
			dur=ItemAttribute[index].Durability + itemLevel*2-3;
		}
		else if (itemLevel == 11 )
		{
			dur=ItemAttribute[index].Durability + itemLevel*2-1;
		}
		else if (itemLevel == 12 )
		{
			dur=ItemAttribute[index].Durability + itemLevel*2+2;
		}
		else if (itemLevel == 13 )
		{
			dur=ItemAttribute[index].Durability + itemLevel*2+6;
		}
		else if (itemLevel == 14 )
		{
			dur=ItemAttribute[index].Durability + itemLevel*2+11;
		}
		else if (itemLevel == 15 )
		{
			dur=ItemAttribute[index].Durability + itemLevel*2+17;
		}
		else
		{
			dur=ItemAttribute[index].Durability + itemLevel*2-4;
		}
	}
	
	if ( (index < GET_ITEM(12,3) || index > GET_ITEM(12,6) ) &&
		  index != GET_ITEM(0,19) &&
		  index != GET_ITEM(4,18) &&
		  index != GET_ITEM(5,10) &&
		  index != GET_ITEM(2,13) &&
		  index != GET_ITEM(12,49) &&
		  index != GET_ITEM(13,30) )
	{
		if ( SetItem != 0 )
			dur +=20;
		else if ( ExcellentItem != 0 && index != GET_ITEM(12,50) && (index < GET_ITEM(12,36) || index > GET_ITEM(12,43) ) ) // Third Wings Fix Durability *Prevent duple if items
			dur +=15;
	}

	if ( dur > 255 )
		dur = 255;

	return dur;
}
#endif