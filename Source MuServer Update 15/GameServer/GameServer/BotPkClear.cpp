// 
// -		New BotResetPk #Team [By Callejero]
// 
#include "stdafx.h"
#include "BotPkClear.h"
#include "Util.h"
#include "ReadScript.h"
#include "ItemManager.h"
#include "Monster.h"
#include "ObjectManager.h"
#include "NewsProtocol.h"
#include "MuunSystem.h"
#include "Notice.h"
#include "Trade.h"
#include "GensSystem.h"
#include "PG_Custom.h"

BotPkClear	gBotPkClear;

void BotPkClear::Init()
{
	this->m_ItemLoad = 0;
    ZeroMemory(this->Req, sizeof(this->Req));
}

void BotPkClear::LoadData(char* FilePath)
{
	if(this->Enabled == true)
	{
		for(int botNum=0;botNum<MAX_BOTRESETPK;botNum++)
		{
			if(this->Load[botNum].Enabled == true)
			{
				int bIndex = this->Load[botNum].index;
				if(gObjIsConnected(bIndex) == TRUE)
				{
					gObjDel(bIndex);
				}
			}
		}
	}

	memset(&this->Load,0,sizeof(this->Load));

	this->Enabled = FALSE;
	int Token;

	if (FilePath == NULL || !strcmp(FilePath, ""))
	{
		ErrorMessageBox("[BotPkClear] file load error - File Name Error");
		return ;
	}

	SMDFile = fopen(FilePath, "r");

	if ( SMDFile == NULL )
	{
		ErrorMessageBox("[BotPkClear] data load error %s", FilePath);
		return;
	}

	this->Init();

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
				if(BotNum < 0 || BotNum > MAX_BOTRESETPK-1)
				{
					ErrorMessageBox("BotPkClear error: Index 1: %d out of range!", BotNum);
					return;
				}

				LPOBJ lpObj = &gObj[BotNum];

				Token = GetToken();
				this->Load[BotNum].Class = TokenNumber;
				
				Token = GetToken();
				this->Load[BotNum].Enabled = TokenNumber;
				
				Token = GetToken();
				this->Load[BotNum].ColorName = TokenNumber;

				Token = GetToken();
				strncpy(this->Load[BotNum].Name,TokenString,sizeof(this->Load[BotNum].Name));

				Token = GetToken();
				this->Load[BotNum].Map = TokenNumber;
				
				Token = GetToken();
				this->Load[BotNum].X = TokenNumber;
				
				Token = GetToken();
				this->Load[BotNum].Y = TokenNumber;
				
				Token = GetToken();
				this->Load[BotNum].Dir = TokenNumber;
			}
		}
		iType = GetToken();
		iType = TokenNumber;
		if ( iType == 2 )	
		{
			this->Enabled = TRUE;

			while(true)
			{
				Token = GetToken();
				if ( strcmp("end", TokenString) == 0 )
				{
					break;
				}

				int BotNum = TokenNumber;
				if(BotNum < 0 || BotNum > MAX_BOTRESETPK-1)
				{
					LogAdd(LOG_BOT,"BotPkClear error: Index 3: %d doesnt exist", BotNum);
					return;
				}

				Token = GetToken();
				this->Load[BotNum].OnlyVip = TokenNumber;

				Token = GetToken();
				this->Load[BotNum].ActiveGensFamily = TokenNumber;

				Token = GetToken();
				this->Load[BotNum].ReqContribution = TokenNumber;

				Token = GetToken();
				this->Load[BotNum].GensFamily = TokenNumber;

				Token = GetToken();
				this->Req[BotNum].Zen = TokenNumber;

				Token = GetToken();
				this->Req[BotNum].PkLevels = TokenNumber;
			
			}
		}
		iType = GetToken();
		iType = TokenNumber;
		if ( iType == 3 )	
		{
			while(true)
			{
				Token = GetToken();
				if ( strcmp("end", TokenString) == 0 )
				{
					break;
				}

				int BotNum = TokenNumber;
				if(BotNum < 0 || BotNum > MAX_BOTRESETPK-1)
				{
					LogAdd(LOG_BOT,"BotPkClear error: Index 2: %d doesnt exist", BotNum);
					return;
				}

				Token = GetToken();
				int Slot = TokenNumber;
				
				if(Slot < 0 || Slot > 8)
				{	
					ErrorMessageBox("BotPkClear error: Min Slot 0 ; Max Slot 8");
					return;
				}

				Token = GetToken();
				int iType = TokenNumber;

				Token = GetToken();
				int iIndex = TokenNumber;

				this->Load[BotNum].Item_BLoad[Slot].Type = GET_ITEM(iType,iIndex);

				Token = GetToken();
				this->Load[BotNum].Item_BLoad[Slot].level = TokenNumber;

				Token = GetToken();
				this->Load[BotNum].Item_BLoad[Slot].opt = TokenNumber;

				
				this->Load[BotNum].Item_BLoad[Slot].Enabled = true;
			}
		}		
		iType = GetToken();
		iType = TokenNumber;
		if ( iType == 4 )	
		{
			this->Enabled = TRUE;

			while(true)
			{
				Token = GetToken();
				if ( strcmp("end", TokenString) == 0 )
				{
					break;
				}

				int BotNum = TokenNumber;
				if(BotNum < 0 || BotNum > MAX_BOTRESETPK-1)
				{
					LogAdd(LOG_BOT,"BotPkClear error: Index 4: %d doesnt exist", BotNum);
					return;
				}

				Token = GetToken();
				this->Req[BotNum].ItemReq = TokenNumber;

				Token = GetToken();
				this->Req[BotNum].Type = TokenNumber;

				Token = GetToken();
				this->Req[BotNum].Index = TokenNumber;
				
				this->Req[BotNum].iNum = GET_ITEM(this->Req[BotNum].Type,this->Req[BotNum].Index);

				Token = GetToken();
				this->Req[BotNum].MinLevel = TokenNumber;
				
				Token = GetToken();
				this->Req[BotNum].MaxLevel = TokenNumber;
				
				Token = GetToken();
				this->Req[BotNum].Luck = TokenNumber;
				
				Token = GetToken();
				this->Req[BotNum].Skill = TokenNumber;
				
				Token = GetToken();
				this->Req[BotNum].Opt = TokenNumber;
				
				Token = GetToken();
				this->Req[BotNum].Exc = TokenNumber;

				this->m_ItemLoad++;
			}
		}
		break;
	}

	fclose(SMDFile);
}

int BotPkClear::GetBotIndex(int aIndex)
{
	for(int i=0; i<MAX_BOTRESETPK; i++)
	{
		if(this->Load[i].Enabled == true)
		{
			if(this->Load[i].index == aIndex)
				return i;
		}
	}
	return -1;
}

void BotPkClear::MakeBot()
{
	if(this->Enabled == true)
	{	
		for(int botNum=0; botNum<MAX_BOTRESETPK; botNum++)
		{
			if(this->Load[botNum].Enabled == true)	
			{
				int result = gObjAddSummon(); 

				if(result >= 0)
				{
					this->Load[botNum].index = result;

					gObj[result].PosNum = (WORD)-1;
					gObj[result].X = this->Load[botNum].X;
					gObj[result].Y = this->Load[botNum].Y;
					gObj[result].OldX = this->Load[botNum].X;
					gObj[result].OldY = this->Load[botNum].Y;
					gObj[result].TX = this->Load[botNum].X;
					gObj[result].TY = this->Load[botNum].Y;
					gObj[result].MTX = this->Load[botNum].X;
					gObj[result].MTY = this->Load[botNum].Y;
					gObj[result].Dir = this->Load[botNum].Dir;
					gObj[result].Map = this->Load[botNum].Map;
					gObj[result].Live = TRUE;	
					gObj[result].PathCount = 0; 
					gObj[result].IsBot = 3;
					gObj[result].BotPower = 0;
					gObjSetBots(result,this->Load[botNum].Class);
					gObj[result].ChangeUp = this->Load[botNum].Class;
					gObj[result].Class = this->Load[botNum].Class;
					gObj[result].Level = 400;
					gObj[result].Life = 1;
					gObj[result].MaxLife = 2;
					gObj[result].Mana = 1;
					gObj[result].MaxMana = 2;
					gObj[result].Experience = 0;
					gObj[result].DBClass = this->Load[botNum].Class;
					gObj[result].PKLevel = this->Load[botNum].ColorName;

					if(this->Load[botNum].ActiveGensFamily == 1)
					{
						gObj[result].GensFamily = this->Load[botNum].GensFamily;
					}

					gObj[result].Inventory = new CItem[INVENTORY_SIZE];					
					gObj[result].Inventory1 = new CItem[INVENTORY_SIZE];
					gObj[result].InventoryMap1 = new BYTE[INVENTORY_SIZE];
					gObj[result].InventoryMap = new BYTE[INVENTORY_SIZE];
					
					for (int i=0;i<INVENTORY_SIZE;i++)
					{
						gObj[result].Inventory[i].Clear();
						gObj[result].Inventory1[i].Clear();
					}

					memset(&gObj[result].InventoryMap[0], (BYTE)-1, INVENTORY_SIZE);
					memset(&gObj[result].InventoryMap1[0], (BYTE)-1, INVENTORY_SIZE);
					strncpy(gObj[result].Name,this->Load[botNum].Name,sizeof(gObj[result].Name));

					for(int i=0;i<9;i++)
					{
						if(this->Load[botNum].Item_BLoad[i].Type >= 0 && this->Load[botNum].Item_BLoad[i].Enabled == true)
						{
							CItem item;
							item.m_Level = this->Load[botNum].Item_BLoad[i].level;
							item.m_Option1 = 0;	
							item.m_Option2 = 1;	
							item.m_Option3 = this->Load[botNum].Item_BLoad[i].opt;	
							item.m_Durability = 255.0f;
							item.m_JewelOfHarmonyOption = 0;
							item.m_ItemOptionEx = 0;
							item.m_SocketOption[1] = 0;
							item.m_SocketOption[2] = 0;
							item.m_SocketOption[3] = 0;
							item.m_SocketOption[4] = 0;
							item.m_SocketOption[5] = 0;
							item.Convert(this->Load[botNum].Item_BLoad[i].Type,item.m_Option1,item.m_Option2,item.m_Option3,item.m_NewOption,item.m_SetOption,item.m_JewelOfHarmonyOption,item.m_ItemOptionEx,item.m_SocketOption,item.m_SocketOptionBonus);
							gObj[result].Inventory[i].m_Option1 = item.m_Option1;
							gObj[result].Inventory[i].m_Option2 = item.m_Option2;
							gObj[result].Inventory[i].m_Option3 = item.m_Option3;
							gObj[result].Inventory[i].m_JewelOfHarmonyOption = item.m_JewelOfHarmonyOption;
							gObj[result].Inventory[i].m_ItemOptionEx = item.m_ItemOptionEx;

							item.m_Number = 0;
							gObjInventoryInsertItemPos(gObj[result].Index,item,i,0);
						}
					}
					
					gObj[result].Inventory1 = gObj[result].Inventory;
					gObj[result].InventoryMap1 = gObj[result].InventoryMap;
					
					gObjectManager.CharacterMakePreviewCharSet(result);

					LPOBJ lpObj = &gObj[result];

					gObj[result].AttackType = 0;
					gObj[result].BotSkillAttack = 0;					
					gObj[result].Attribute = 100;
					gObj[result].TargetNumber = (WORD)-1;
					gObj[result].ActionState.Emotion = 0;
					gObj[result].ActionState.Attack = 0;
					gObj[result].ActionState.EmotionCount = 0;
					gObj[result].PathCount = 0;
					gObj[result].BotPower = 0;
					gObj[result].BotDefense = this->Load[botNum].OnlyVip;
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

					GCSummonLifeSend(gObj[result].SummonIndex,gObj[result].MaxLife,gObj[result].Life);				
				}
			}
		}
	}
}

BOOL BotPkClear::TradeOpen(int index, int nindex)
{
	if ( gObjIsConnectedGP(index) == FALSE )
	{
		LogAdd(LOG_BLUE, "Error-L3 [%s][%d]", __FILE__, __LINE__);
		return 0;
	}
	if(gObjIsConnected(nindex) == 0)
	{
		return 0;
	}
	int number = this->GetBotIndex(nindex);

	if(number == -1)
	{
		return 0;
	}

	LPOBJ lpObj = &gObj[index];
	LPOBJ lpBot = &gObj[nindex];

	if(this->Load[number].OnlyVip != 0 && lpObj->AccountLevel == 0)
	{
		gNotice.NewMessageDevTeam(lpObj->Index,"%s Im Only work for Account Vips",this->Load[number].Name);
		LogAdd(LOG_RED,"[%s] Account: %s Free [OnlyVip!]",this->Load[number].Name,lpObj->Account);
		goto Cancel;
	}
	if(this->Load[number].ActiveGensFamily == 1)
	{
		if(this->Load[number].GensFamily != GENS_FAMILY_VARNERT && gObj[index].GensFamily == GENS_FAMILY_NONE )
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s You Need be GensFamily Vanert",this->Load[number].Name);
			return 1;
		}
		if (this->Load[number].GensFamily != GENS_FAMILY_DUPRIAN && gObj[index].GensFamily == GENS_FAMILY_NONE)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s You Need be GensFamily Duprian",this->Load[number].Name);
			return 1;
		}
		if (this->Load[number].GensFamily == GENS_FAMILY_VARNERT && gObj[index].GensFamily == GENS_FAMILY_DUPRIAN)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s You Need be GensFamily Duprian",this->Load[number].Name);
			return 1;
		}
		if (this->Load[number].GensFamily == GENS_FAMILY_DUPRIAN && gObj[index].GensFamily == GENS_FAMILY_VARNERT)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s You Need be GensFamily Vanert",this->Load[number].Name);
			return 1;
		}
		if(this->Load[number].ReqContribution > gObj[index].GensContribution)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s Insuficent GensContribution : %d",this->Load[number].Name,this->Load[number].ReqContribution);
			LogAdd(LOG_RED,"[%s] Account: %s - Insuficent GensContribution : %d",this->Load[number].Name,gObj[index].Account,this->Load[number].ReqContribution);
			return 1;
		}
	}
	if(this->Req[number].Zen > lpObj->Money)
	{
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"%s You need more Money: %d",this->Load[number].Name,this->Req[number].Zen);
		LogAdd(LOG_RED,"[%s] Account: %s CharName: %s Insuficent Money",this->Load[number].Name,lpObj->Account,lpObj->Name);
		goto Cancel;
	}
	if(this->Req[number].PkLevels > lpObj->PKLevel)
	{
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"%s you do not have to reset the level pk",this->Load[number].Name,this->Req[number].PkLevels);
		LogAdd(LOG_RED,"[%s] CharName: %s, do not have level pk suficient ",this->Load[number].Name,lpObj->Name);
		goto Cancel;
	}

	if(this->Req[number].ItemReq != 0)
	{
		if(!this->CheckItemReq(lpObj))
		{
			return false;
		}
	}

	for( int i = 0; i < this->m_ItemLoad; i++ )
	{
		this->DeleteItem(lpObj, i);
	}

	lpObj->PKLevel = 1;

	GCPKLevelSend(lpObj->Index,lpObj->PKLevel);

	gNotice.NewMessageDevTeam(lpObj->Index,"%s Reset level pk success",this->Load[number].Name);

Cancel:
	gObjTradeCancel(index);
	gTrade.GCTradeResultSend(index,3);
	
}

bool BotPkClear::InventorySearchItem(LPOBJ lpObj, BYTE SearchItem)
{
    WORD ItemID = this->Req[SearchItem].iNum;
    
    for( int i = INVENTORY_WEAR_SIZE; i < MAIN_INVENTORY_NORMAL_SIZE; i++ )
    {
        if( !lpObj->Inventory[i].IsItem() || lpObj->Inventory[i].m_Index != ItemID )
        {
            continue;
        }    
        if( lpObj->Inventory[i].m_Level < this->Req[SearchItem].MinLevel
            || lpObj->Inventory[i].m_Level > this->Req[SearchItem].MaxLevel )
        {
            continue;
        }      
        if( lpObj->Inventory[i].m_Option2 < (float)this->Req[SearchItem].Luck )
        {
            continue;
        }
		if( lpObj->Inventory[i].m_Option1 < (float)this->Req[SearchItem].Skill )
        {
            continue;
        }
		if( lpObj->Inventory[i].m_Option3 < (float)this->Req[SearchItem].Opt )
        {
            continue;
        }
		if( lpObj->Inventory[i].m_NewOption < (float)this->Req[SearchItem].Exc )
        {
            continue;
        }
        return true;
    }

    return false;
}

bool BotPkClear::DeleteItem(LPOBJ lpObj, BYTE Item)
{
	 WORD ItemID = this->Req[Item].iNum;

	for( int i = INVENTORY_WEAR_SIZE; i < MAIN_INVENTORY_NORMAL_SIZE; i++ )
    {
        if( !lpObj->Inventory[i].IsItem() || lpObj->Inventory[i].m_Index != ItemID )
        {
            continue;
        }       
        if( lpObj->Inventory[i].m_Level < this->Req[Item].MinLevel
            || lpObj->Inventory[i].m_Level > this->Req[Item].MaxLevel )
        {
            continue;
        }      
        if( lpObj->Inventory[i].m_Option2 < (float)this->Req[Item].Luck )
        {
            continue;
        }
		if( lpObj->Inventory[i].m_Option1 < (float)this->Req[Item].Skill )
        {
            continue;
        }
		if( lpObj->Inventory[i].m_Option3 < (float)this->Req[Item].Opt )
        {
            continue;
        }
		if( lpObj->Inventory[i].m_NewOption < (float)this->Req[Item].Exc )
        {
            continue;
        }
		gItemManager.InventoryDelItem(lpObj->Index,i);
		gItemManager.GCItemDeleteSend(lpObj->Index,i,1);
		return true;
    }   
    return false;
}

bool BotPkClear::CheckItemReq(LPOBJ lpObj)
{
	int BotNum = 0;
	LPOBJ lpBot = &gObj[BotNum];

	if( !this->InventorySearchItem(lpObj, 0) )
	{
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"You have not required items for ResetPk: %s",
			ItemAttribute[GET_ITEM(this->Req[BotNum].Type,this->Req[BotNum].Index)].Name);
		
        return false;
    }
     return true;
}