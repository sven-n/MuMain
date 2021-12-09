#include "stdafx.h"
#include "GameServer.h"
#include "GameMain.h"
#include "User.h"
#include "BotStore.h"
#include "readscript.h"
#include "Util.h"
#include "ItemManager.h"
#include "PG_Custom.h"
#include "Monster.h"
#include "ObjectManager.h"
#include "MuunSystem.h"
#include "MemScript.h"

#if (BOT_STORE == 1)

ObjBotStore BotStore;

void ObjBotStore::Read(char * FilePath)
{
	if(this->Enabled == true)
	{
		for(int botNum=0;botNum<MAX_BOTSTORE;botNum++)
		{
			if(this->bot[botNum].Enabled == true)
			{
				int bIndex = this->bot[botNum].index;
				if(gObjIsConnected(bIndex) == TRUE)
				{
					gObjDel(bIndex);
				}
			}
		}
	}
	for(int i=0;i<MAX_BOTSTORE;i++)
	{
		this->bot[i].index = -1;
		this->bot[i].Enabled = false;
		this->bot[i].ItemCount=0;
		for(int j=0;j<9;j++)
			this->bot[i].body[j].num =-1;
	}

	this->Enabled = FALSE;
	int Token;
	SMDFile = fopen(FilePath, "r");

	if ( SMDFile == NULL )
	{
		ErrorMessageBox("BotStore data load error %s", FilePath);
		return;
	}

	while ( true )
	{
		int iType = GetToken();
		
		if ( iType == 1 )
		{
			while(true)
			{
				Token = GetToken();
				if ( strcmp("end", TokenString) == 0 )
				{
					break;
				}

				int BotNum = TokenNumber;
				if(BotNum < 0 || BotNum > MAX_BOTSTORE-1)
				{
					ErrorMessageBox("BotStore error: BotPetIndex:%d out of range!", BotNum);
					return;
				}

				Token = GetToken();
				this->bot[BotNum].Class = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].OnlyVip = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].UseVipMoney = TokenNumber;

				Token = GetToken();
				strncpy(this->bot[BotNum].Name,TokenString,sizeof(this->bot[BotNum].Name));

				Token = GetToken();
				strncpy(this->bot[BotNum].StoreName,TokenString,sizeof(this->bot[BotNum].StoreName));

				Token = GetToken();
				this->bot[BotNum].Map = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].X = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].Y = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].Dir = TokenNumber;

				this->bot[BotNum].Enabled = true;
			}
		}
		
		iType = GetToken();
		iType = TokenNumber;
		if ( iType == 2 )
		{
			while(true)
			{
				Token = GetToken();
				if ( strcmp("end", TokenString) == 0 )
				{
					break;
				}

				int BotNum = TokenNumber;
				if(BotNum < 0 || BotNum > MAX_BOTSTORE-1)
				{
					ErrorMessageBox("BotStore error: BotPetIndex:%d doesnt exist", BotNum);
					return;
				}
				if(this->bot[BotNum].Enabled == false)
				{	
					ErrorMessageBox("BotStore error: BotPetIndex:%d doesnt exist", BotNum);
					return;
				}
				Token = GetToken();
				int Slot = TokenNumber;
				
				if(Slot < 0 || Slot > 8)
				{	
					ErrorMessageBox("BotStore error: Min Slot 0 ; Max Slot 8");
					return;
				}

				Token = GetToken();
				int iType = TokenNumber;

				Token = GetToken();
				int iIndex = TokenNumber;

				this->bot[BotNum].body[Slot].num = GET_ITEM(iType,iIndex);

				Token = GetToken();
				this->bot[BotNum].body[Slot].level = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].body[Slot].opt = TokenNumber;

				this->bot[BotNum].body[Slot].Enabled = true;
			}
		}
		
		iType = GetToken();
		iType = TokenNumber;
		if ( iType == 3 )
		{
	
			CItem item;
			this->Enabled = TRUE;
			while(true)
			{
				Token = GetToken();
				if ( strcmp("end", TokenString) == 0 )
				{
					break;
				}

				int BotNum = TokenNumber;
				if(BotNum < 0 || BotNum > MAX_BOTSTORE-1)
				{
					ErrorMessageBox("BotStore error: BotPetIndex:%d doesnt exist", BotNum);
					return;
				}
				if(this->bot[BotNum].Enabled == false)
				{	
					ErrorMessageBox("BotStore error: BotPetIndex:%d doesnt exist", BotNum);
					return;
				}
				int Num = this->bot[BotNum].ItemCount;

				Token = GetToken();
				this->bot[BotNum].storeItem[Num].ValueZen = TokenNumber;

				Token = GetToken();
				int iType = TokenNumber;

				Token = GetToken();
				int iIndex = TokenNumber;

				this->bot[BotNum].storeItem[Num].num = GET_ITEM(iType,iIndex);

				Token = GetToken();
				this->bot[BotNum].storeItem[Num].Level = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].storeItem[Num].Opt = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].storeItem[Num].Luck = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].storeItem[Num].Skill = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].storeItem[Num].Dur = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].storeItem[Num].Exc = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].storeItem[Num].Anc = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].storeItem[Num].ValueSoul = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].storeItem[Num].ValueBless = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].storeItem[Num].ValueChaos = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].storeItem[Num].PCPoint = TokenNumber;

				for(int i=0;i<5;i++)
				{
				Token = GetToken();
				this->bot[BotNum].storeItem[Num].Sock[i] = TokenNumber;
				}
				this->bot[BotNum].ItemCount++;
			}
		}
		
		break;
	}
			
	LogAdd(LOG_BOT,"[BotStore] - %s file is Loaded",FilePath);
	fclose(SMDFile);
}

BYTE gObjTempPShopRectCheck(BYTE * TempMap, int sx, int sy, int width, int height)
{
	int x,y,blank = 0;

	if(sx + width > 12)
	{
		return -1;
	}
	if(sy + height > 8)
	{
		return -2;
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

BYTE ObjBotStore::CheckSpace(LPOBJ lpObj, int type, BYTE * TempMap)
{
	int w,h,iwidth,iheight;
	BYTE blank = 0;

	iwidth=ItemAttribute[type].Width;
	iheight=ItemAttribute[type].Height;

	for(h = 0; h < 4; h++)
	{
		for(w = 0; w < 8; w++)
		{
			if(*(BYTE*)(TempMap + h * 8 + w) == 255)
			{
				blank = gObjTempPShopRectCheck(TempMap,w,h,iwidth,iheight);

				if(blank == 254)
				{
					goto GOTO_EndFunc;
				}
				if(blank != 255)
				{
					gObjTempInventoryItemBoxSet(TempMap,blank,iwidth,iheight,type);
					return blank;
				}
			}
		}
	}
GOTO_EndFunc:
	return -1;
}

void ObjBotStore::AddItem(int bIndex,int botNum)
{
	for(int i=0;i<this->bot[botNum].ItemCount;i++)
	{ 
		//int blank = this->CheckSpace(&gObj[bIndex],this->bot[botNum].storeItem[i].num,&gObj[bIndex].InventoryMap[MAIN_INVENTORY_SIZE(bIndex,false)]);
		int blank = this->CheckSpace(&gObj[bIndex],this->bot[botNum].storeItem[i].num,&gObj[bIndex].InventoryMap[MAIN_INVENTORY_NORMAL_SIZE]);
		
		if(blank != 255)
		{
			blank += MAIN_INVENTORY_NORMAL_SIZE - INVENTORY_WEAR_SIZE -1;

			CItem item;
			item.m_Level		   = this->bot[botNum].storeItem[i].Level;
			item.m_Option1		   = this->bot[botNum].storeItem[i].Skill;
			item.m_Option2		   = this->bot[botNum].storeItem[i].Luck;
			item.m_Option3		   = this->bot[botNum].storeItem[i].Opt;
			item.m_Durability	   = this->bot[botNum].storeItem[i].Dur;
			item.m_JewelOfHarmonyOption = 0;
			item.m_NewOption	   = this->bot[botNum].storeItem[i].Exc;
			item.m_ItemOptionEx	   = 0;
			item.m_SetOption	   = this->bot[botNum].storeItem[i].Anc;
			item.m_SocketOption[0] = this->bot[botNum].storeItem[i].Sock[0];	
			item.m_SocketOption[1] = this->bot[botNum].storeItem[i].Sock[1];
			item.m_SocketOption[2] = this->bot[botNum].storeItem[i].Sock[2];
			item.m_SocketOption[3] = this->bot[botNum].storeItem[i].Sock[3];
			item.m_SocketOption[4] = this->bot[botNum].storeItem[i].Sock[4];
			item.m_PShopValue	   = this->bot[botNum].storeItem[i].ValueZen;	//OK
			//item.m_PShopJoSValue   = this->bot[botNum].storeItem[i].ValueSoul;	//OK
			//item.m_PShopJoBValue   = this->bot[botNum].storeItem[i].ValueBless;	//OK
			//item.m_PShopJoCValue   = this->bot[botNum].storeItem[i].ValueChaos;
			//item.m_PCPoint		   = this->bot[botNum].storeItem[i].PCPoint;	//OK

			item.Convert(this->bot[botNum].storeItem[i].num,item.m_Option1,item.m_Option2,item.m_Option3,item.m_NewOption,item.m_SetOption,item.m_JewelOfHarmonyOption,item.m_ItemOptionEx,item.m_SocketOption,item.m_SocketOptionBonus);

			gObj[bIndex].Inventory[blank].m_Option1 = item.m_Option1;
			gObj[bIndex].Inventory[blank].m_Option2 = item.m_Option2;
			gObj[bIndex].Inventory[blank].m_Option3 = item.m_Option3;
			gObj[bIndex].Inventory[blank].m_JewelOfHarmonyOption = item.m_JewelOfHarmonyOption;
			gObj[bIndex].Inventory[blank].m_ItemOptionEx = item.m_ItemOptionEx;

			item.m_Number = 0;

			gObj[bIndex].Inventory[blank] = item;
		}
	}	
}

void ObjBotStore::MakeBot()
{
	if(this->Enabled == true)
	{
		for(int botNum=0;botNum<MAX_BOTSTORE;botNum++)
		{
			if(this->bot[botNum].Enabled == true)
			{
				int result = gObjAddSummon();

				if(result >= 0)
				{
					this->bot[botNum].index = result;
					gObj[result].PosNum = (WORD)-1;
					gObj[result].X = this->bot[botNum].X;
					gObj[result].Y = this->bot[botNum].Y;
					gObj[result].OldX = this->bot[botNum].X;
					gObj[result].OldY = this->bot[botNum].Y;
					gObj[result].TX = this->bot[botNum].X;
					gObj[result].TY = this->bot[botNum].Y;
					gObj[result].MTX = this->bot[botNum].X;
					gObj[result].MTY = this->bot[botNum].Y;
					gObj[result].Dir = this->bot[botNum].Dir;
					gObj[result].Map = this->bot[botNum].Map;
					gObj[result].Live = TRUE;
					gObj[result].PathCount = 0;
					gObj[result].IsBot = 4;
					gObjSetBots(result,this->bot[botNum].Class);

					gObj[result].ChangeUp = this->bot[botNum].Class & 0x07;	// Set Second Type of Character
					gObj[result].Class = this->bot[botNum].Class;
					gObj[result].Level = 400;
					gObj[result].Life = 1;
					gObj[result].MaxLife = 2;
					gObj[result].Mana = 1;
					gObj[result].MaxMana = 2;
					gObj[result].Experience = 0;
					gObj[result].DBClass = this->bot[botNum].Class;
					gObj[result].PKLevel = 3;
					
					gObj[result].Inventory = new CItem[MAIN_INVENTORY_NORMAL_SIZE + (MAX_EXTENDED_INV*EXTENDED_INV_SIZE) + PSHOP_SIZE];					
					gObj[result].Inventory1 = new CItem[MAIN_INVENTORY_NORMAL_SIZE + (MAX_EXTENDED_INV*EXTENDED_INV_SIZE) + PSHOP_SIZE];
					gObj[result].InventoryMap1 = new BYTE[MAIN_INVENTORY_NORMAL_SIZE + (MAX_EXTENDED_INV*EXTENDED_INV_SIZE) + PSHOP_SIZE];
					gObj[result].InventoryMap = new BYTE[MAIN_INVENTORY_NORMAL_SIZE + (MAX_EXTENDED_INV*EXTENDED_INV_SIZE) + PSHOP_SIZE];
					
					//gObj[result].MuunInventory = new CItem[MAIN_INVENTORY_NORMAL_SIZE + (MAX_EXTENDED_INV*EXTENDED_INV_SIZE) + PSHOP_SIZE];
					//gObj[result].MuunInventoryMap = new BYTE[MAIN_INVENTORY_NORMAL_SIZE + (MAX_EXTENDED_INV*EXTENDED_INV_SIZE) + PSHOP_SIZE];

					for (int i=0;i<(MAIN_INVENTORY_NORMAL_SIZE + (MAX_EXTENDED_INV*EXTENDED_INV_SIZE) + PSHOP_SIZE);i++)
					{
						gObj[result].Inventory[i].Clear();
						gObj[result].Inventory1[i].Clear();
						//gObj[result].MuunInventory[i].Clear();

					}

					memset(&gObj[result].InventoryMap[0], (BYTE)-1, (MAIN_INVENTORY_NORMAL_SIZE + (MAX_EXTENDED_INV*EXTENDED_INV_SIZE) + PSHOP_SIZE));	//108+64+64
					memset(&gObj[result].InventoryMap1[0], (BYTE)-1, (MAIN_INVENTORY_NORMAL_SIZE + (MAX_EXTENDED_INV*EXTENDED_INV_SIZE) + PSHOP_SIZE));	//108+64+64
					//memset(&gObj[result].MuunInventoryMap[0], (BYTE)-1, (MUUN_INVENTORY_SIZE));

					strncpy(gObj[result].Name,this->bot[botNum].Name,sizeof(gObj[result].Name));


					for(int i=0;i<9;i++)
					{
						if(this->bot[botNum].body[i].num >= 0 && this->bot[botNum].body[i].Enabled == true)
						{
							CItem item;
							item.m_Level = this->bot[botNum].body[i].level;
							item.m_Option1 = 0;
							item.m_Option2 = 1;
							item.m_Option3 = this->bot[botNum].body[i].opt;
							item.m_Durability = 255.0f;
							item.m_JewelOfHarmonyOption = 0;
							item.m_ItemOptionEx = 0;
							item.m_SocketOption[1] = 0;
							item.m_SocketOption[2] = 0;
							item.m_SocketOption[3] = 0;
							item.m_SocketOption[4] = 0;
							item.m_SocketOption[5] = 0;
							item.Convert(this->bot[botNum].body[i].num,item.m_Option1,item.m_Option2,item.m_Option3,item.m_NewOption,item.m_SetOption,item.m_JewelOfHarmonyOption,item.m_ItemOptionEx,item.m_SocketOption,item.m_SocketOptionBonus);

							gObj[result].Inventory[i].m_Option1 = item.m_Option1;
							gObj[result].Inventory[i].m_Option2 = item.m_Option2;
							gObj[result].Inventory[i].m_Option3 = item.m_Option3;
							gObj[result].Inventory[i].m_JewelOfHarmonyOption = item.m_JewelOfHarmonyOption;
							gObj[result].Inventory[i].m_ItemOptionEx = item.m_ItemOptionEx;

							item.m_Number = 0;
							
							gObjInventoryInsertItemPos(gObj[result].Index,item,i,0);
						}
					}	
					this->AddItem(result,botNum);
					
					gObj[result].Inventory1 = gObj[result].Inventory;
					gObj[result].InventoryMap1 = gObj[result].InventoryMap;

					//gObj[result].MuunInventory = gObj[result].MuunInventory;
					//gObj[result].MuunInventoryMap = gObj[result].MuunInventoryMap;
					//
					gObjectManager.CharacterMakePreviewCharSet(result);

					gObj[result].AttackType = 0;
					gObj[result].BotSkillAttack = 0;					

					gObj[result].Attribute = 100;
					gObj[result].TargetNumber = (WORD)-1;
					gObj[result].ActionState.Emotion = 0;
					gObj[result].ActionState.Attack = 0;
					gObj[result].ActionState.EmotionCount = 0;
					gObj[result].PathCount = 0;
					gObj[result].BotPower = this->bot[botNum].UseVipMoney;
					gObj[result].BotDefense = this->bot[botNum].OnlyVip;
					gObj[result].BotLife = 1;
					gObj[result].BotMaxLife = 1;
					
					gObj[result].BotLvlUpDefense = 1;
					gObj[result].BotLvlUpPower = 1;
					gObj[result].BotLvlUpLife = 1;
					gObj[result].BotLvlUpMana = 1;
					gObj[result].BotLvlUpExp = 1;
					gObj[result].BotLvlUpMaxLevel = 1;

					gObj[result].MoveRange = 1;
					gObj[result].BotFollowMe = 0;
					gObj[result].NextExperience = gLevelExperience[gObj[result].Level];

					GCSummonLifeSend(gObj[result].SummonIndex,gObj[result].MaxLife,gObj[result].Life);	//OK
					
					gObj[result].PShopOpen = true;
					memcpy(gObj[result].PShopText, this->bot[botNum].StoreName, sizeof(gObj[result].PShopText));

					ChatSend(&gObj[result],"NEW SELLER!");
					GCActionSend(&gObj[result],ACTION_SALUTE1,result,result);
				}
			}
		}
	}
}

#endif