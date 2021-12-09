// 
// -		New BotReset AOG-Team # (By Callejero)

#include "stdafx.h"
#include "BotReset.h"
#include "Monster.h"
#include "Protocol.h"
#include "Util.h"
#include "Notice.h"
#include "User.h"
#include "ItemManager.h"
#include "MuunSystem.h"
#include "NewsProtocol.h"
#include "ReadScript.h"
#include "ObjectManager.h"
#include "Trade.h"
#include "DSProtocol.h"
#include "GensSystem.h"
#include "ShopPointEx.h"
#include "PG_Custom.h"

BotReset gBotReset;

BotReset::BotReset()
{
	
}

BotReset::~BotReset()
{

}

void BotReset::Init()
{
	this->m_ItemLoaded = 0;
    ZeroMemory(this->Item_Req, sizeof(this->Item_Req));
}

void BotReset::LoadData(char * FilePath)
{
	if(this->Enabled == true)
	{
		for(int botNum=0;botNum<MAX_BOTR;botNum++)
		{
			if(this->BotLoad[botNum].Enabled == true)
			{
				int bIndex = this->BotLoad[botNum].index;

				if(gObjIsConnected(bIndex) == TRUE)
				{
					gObjDel(bIndex);
				}
			}
		}
	}
	memset(&this->BotLoad,0,sizeof(this->BotLoad));

	this->Enabled = FALSE;
	int Token;

	if (FilePath == NULL || !strcmp(FilePath, ""))
	{
		ErrorMessageBox("[BotReset] file load error - File Name Error");
		return ;
	}

	SMDFile = fopen(FilePath, "r");

	if ( SMDFile == NULL )
	{
		ErrorMessageBox("[BotReset] data load error %s", FilePath);
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
				if(BotNum < 0 || BotNum > MAX_BOTR-1)
				{
					ErrorMessageBox("BotReset error: Index 1: %d out of range!", BotNum);
					return;
				}

				Token = GetToken();
				this->BotLoad[BotNum].Class = TokenNumber;

				Token = GetToken();
				this->BotLoad[BotNum].Enabled = TokenNumber;

				Token = GetToken();
				this->BotLoad[BotNum].ColorName = TokenNumber;

				Token = GetToken();
				strncpy(this->BotLoad[BotNum].Name,TokenString,sizeof(this->BotLoad[BotNum].Name));

				Token = GetToken();
				this->BotLoad[BotNum].Map = TokenNumber;
				
				Token = GetToken();
				this->BotLoad[BotNum].X = TokenNumber;

				Token = GetToken();
				this->BotLoad[BotNum].Y = TokenNumber;

				Token = GetToken();
				this->BotLoad[BotNum].Dir = TokenNumber;
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
				if(BotNum < 0 || BotNum > MAX_BOTR-1)
				{
					LogAdd(LOG_BOT,"BotReset error: Index 2: %d doesnt exist", BotNum);
					return;
				}

				Token = GetToken();
				this->BotLoad[BotNum].Vip = TokenNumber;

				Token = GetToken();
				this->BotLoad[BotNum].ActiveGensFamily = TokenNumber;

				Token = GetToken();
				this->BotLoad[BotNum].ReqContribution = TokenNumber;

				Token = GetToken();
				this->BotLoad[BotNum].GensSystem = TokenNumber;
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
				if(BotNum < 0 || BotNum > MAX_BOTR-1)
				{
					LogAdd(LOG_BOT,"BotReset error: Index 3: %d doesnt exist", BotNum);
					return;
				}

				Token = GetToken();
				int Slot = TokenNumber;
				
				if(Slot < 0 || Slot > 8)
				{	
					ErrorMessageBox("BotReset error: Min Slot 0 ; Max Slot 8");
					return;
				}

				Token = GetToken();
				int iType = TokenNumber;

				Token = GetToken();
				int iIndex = TokenNumber;

				this->BotLoad[BotNum].Bot_Item[Slot].Type = GET_ITEM(iType,iIndex);

				Token = GetToken();
				this->BotLoad[BotNum].Bot_Item[Slot].level = TokenNumber;

				Token = GetToken();
				this->BotLoad[BotNum].Bot_Item[Slot].opt = TokenNumber;

				
				this->BotLoad[BotNum].Bot_Item[Slot].Enabled = true;
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
				if(BotNum < 0 || BotNum > MAX_BOTR-1)
				{
					LogAdd(LOG_BOT,"BotReset error: Index 4: %d doesnt exist", BotNum);
					return;
				}

				Token = GetToken();
				this->Item_Req[BotNum].ResetType = TokenNumber;
				
				Token = GetToken();
				this->Item_Req[BotNum].levels = TokenNumber;
	
				Token = GetToken();
				this->Item_Req[BotNum].reset = TokenNumber;
				
				Token = GetToken();
				this->Item_Req[BotNum].Zen = TokenNumber;
				
				Token = GetToken();
				this->Item_Req[BotNum].PCP = TokenNumber;

				Token = GetToken();
				this->Item_Req[BotNum].ItemReq = TokenNumber;

				Token = GetToken();
				this->Item_Req[BotNum].ItemType = TokenNumber;

				Token = GetToken();
				this->Item_Req[BotNum].ItemIndex = TokenNumber;

				this->Item_Req[BotNum].Num = GET_ITEM(this->Item_Req[BotNum].ItemType,this->Item_Req[BotNum].ItemIndex);

				Token = GetToken();
				this->Item_Req[BotNum].ItemMinLevel = TokenNumber;

				Token = GetToken();
				this->Item_Req[BotNum].ItemMaxLevel = TokenNumber;

				Token = GetToken();
				this->Item_Req[BotNum].ItemDur = TokenNumber;

				this->m_ItemLoaded++;
			}
		}
		iType = GetToken();
		iType = TokenNumber;
		if ( iType == 5 )	
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
				if(BotNum < 0 || BotNum > MAX_BOTR-1)
				{
					LogAdd(LOG_BOT,"BotReset error: Index 5: %d doesnt exist", BotNum);
					return;
				}
				Token = GetToken();
				this->Add_Point[BotNum].AddMasterP = TokenNumber;

				Token = GetToken();
				this->Add_Point[BotNum].MulMasterP = TokenNumber;
			}
		}
		iType = GetToken();
		iType = TokenNumber;
		if ( iType == 6 )	
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
				if(BotNum < 0 || BotNum > MAX_BOTR-1)
				{
					LogAdd(LOG_BOT,"BotReset error: Index 6: %d doesnt exist", BotNum);
					return;
				}

				Token = GetToken();
				this->Add_Item[BotNum].ItemReward	= TokenNumber;
				
				Token = GetToken();
				this->Add_Item[BotNum].ItemWinType = TokenNumber;
				
				Token = GetToken();
				this->Add_Item[BotNum].ItemWinIndex = TokenNumber;;
				
				Token = GetToken();
				this->Add_Item[BotNum].ItemWinLevel = TokenNumber;
				
				Token = GetToken();
				this->Add_Item[BotNum].ItemWinDur = TokenNumber;
				
				Token = GetToken();
				this->Add_Item[BotNum].ItemWinLuck = TokenNumber;
				
				Token = GetToken();
				this->Add_Item[BotNum].ItemWinSkill = TokenNumber;
				
				Token = GetToken();
				this->Add_Item[BotNum].ItemWinOpt = TokenNumber;
				
				Token = GetToken();
				this->Add_Item[BotNum].ItemWinExc = TokenNumber;
				
				Token = GetToken();
				this->Add_Item[BotNum].WCoinCWin = TokenNumber;	
				
				Token = GetToken();
				this->Add_Item[BotNum].WCoinPWin = TokenNumber;	
				
				Token = GetToken();
				this->Add_Item[BotNum].GoblinPointWin = TokenNumber;

			}
		}
		iType = GetToken();
		iType = TokenNumber;
		if ( iType == 7 )	
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
				if(BotNum < 0 || BotNum > MAX_BOTR-1)
				{
					LogAdd(LOG_BOT,"BotReset error: Index 7: %d doesnt exist", BotNum);
					return;
				}

				Token = GetToken();
				this->MarryReq[BotNum].MarryActive	= TokenNumber;

				Token = GetToken();
				this->MarryReq[BotNum].MarryMulLevelUp	= TokenNumber;

				Token = GetToken();
				this->MarryReq[BotNum].MarryMulMoney	= TokenNumber;
			}
		}			
		break;		
	} 
	
	int BotNum = 0;
	LPOBJ lpBot = &gObj[BotNum];
	
	if(this->MarryReq[BotNum].MarryActive == 1)
	{
		LogAdd(LOG_USER,"[%s] - Bonus Marriage Active",this->BotLoad[BotNum].Name);
	}	
	fclose(SMDFile);
}

int BotReset::GetBotIndex(int aIndex)
{
	for(int i=0; i<MAX_BOTR; i++)
	{
		if(this->BotLoad[i].Enabled == true)
		{
			if(this->BotLoad[i].index == aIndex)
				return i;
		}
	}
	return -1;
}

void BotReset::MakeBot()
{
	if(this->Enabled == true)
	{	
		for(int botNum=0; botNum<MAX_BOTR; botNum++)
		{
			if(this->BotLoad[botNum].Enabled == true)	
			{
				int result = gObjAddSummon(); 

				if(result >= 0)
				{
					this->BotLoad[botNum].index = result;

					gObj[result].PosNum = (WORD)-1;
					gObj[result].X = this->BotLoad[botNum].X;
					gObj[result].Y = this->BotLoad[botNum].Y;
					gObj[result].OldX = this->BotLoad[botNum].X;
					gObj[result].OldY = this->BotLoad[botNum].Y;
					gObj[result].TX = this->BotLoad[botNum].X;
					gObj[result].TY = this->BotLoad[botNum].Y;
					gObj[result].MTX = this->BotLoad[botNum].X;
					gObj[result].MTY = this->BotLoad[botNum].Y;
					gObj[result].Dir = this->BotLoad[botNum].Dir;
					gObj[result].Map = this->BotLoad[botNum].Map;
					gObj[result].Live = TRUE;	
					gObj[result].PathCount = 0; 
					gObj[result].IsBot = 3;
					gObj[result].BotPower = 0;
					gObjSetBots(result,this->BotLoad[botNum].Class);
					gObj[result].ChangeUp = this->BotLoad[botNum].Class;
					gObj[result].Class = this->BotLoad[botNum].Class;
					gObj[result].Level = 400;
					gObj[result].Life = 1;
					gObj[result].MaxLife = 2;
					gObj[result].Mana = 1;
					gObj[result].MaxMana = 2;
					gObj[result].Experience = 0;
					gObj[result].DBClass = this->BotLoad[botNum].Class;
					gObj[result].PKLevel = this->BotLoad[botNum].ColorName;
					gObj[result].GensFamily = this->BotLoad[botNum].GensSystem;
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
					strncpy(gObj[result].Name,this->BotLoad[botNum].Name,sizeof(gObj[result].Name));

					for(int i=0;i<9;i++)
					{
						if(this->BotLoad[botNum].Bot_Item[i].Type >= 0 && this->BotLoad[botNum].Bot_Item[i].Enabled == true)
						{
							CItem item;
							item.m_Level = this->BotLoad[botNum].Bot_Item[i].level;
							item.m_Option1 = 0;	
							item.m_Option2 = 1;	
							item.m_Option3 = this->BotLoad[botNum].Bot_Item[i].opt;	
							item.m_Durability = 255.0f;
							item.m_JewelOfHarmonyOption = 0;
							item.m_ItemOptionEx = 0;
							item.m_SocketOption[1] = 0;
							item.m_SocketOption[2] = 0;
							item.m_SocketOption[3] = 0;
							item.m_SocketOption[4] = 0;
							item.m_SocketOption[5] = 0;
							item.Convert(this->BotLoad[botNum].Bot_Item[i].Type,item.m_Option1,item.m_Option2,item.m_Option3,item.m_NewOption,item.m_SetOption,item.m_JewelOfHarmonyOption,item.m_ItemOptionEx,item.m_SocketOption,item.m_SocketOptionBonus);
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

					gObj[result].AttackType = 0;
					gObj[result].BotSkillAttack = 0;					
					gObj[result].Attribute = 100;
					gObj[result].TargetNumber = (WORD)-1;
					gObj[result].ActionState.Emotion = 0;
					gObj[result].ActionState.Attack = 0;
					gObj[result].ActionState.EmotionCount = 0;
					gObj[result].PathCount = 0;
					gObj[result].BotPower = 0;
					gObj[result].BotDefense = this->BotLoad[botNum].Vip;
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

BOOL BotReset::TradeOpen(int index, int nindex)
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

	if(this->BotLoad[number].Vip != 0 && lpObj->AccountLevel == 0)
	{
		gNotice.NewMessageDevTeam(lpObj->Index,"%s Im Only work for Account Vips!",this->BotLoad[number].Name);
		LogAdd(LOG_RED,"[%s] Account: %s Free [OnlyVip!]",this->BotLoad[number].Name,lpObj->Account);
		goto Cancel;
	}
	if(this->Item_Req[number].levels > gObj[index].Level  )
	{
		gNotice.NewMessageDevTeam(lpObj->Index,"%s You need more MinLevel: %d",this->BotLoad[number].Name,this->Item_Req[number].levels);
		LogAdd(LOG_RED,"[%s] CharName: %s Insuficent Level: %d ",this->BotLoad[number].Name,lpObj->Name,lpObj->Level);
		goto Cancel;
	}
	if(this->Item_Req[number].reset < gObj[index].Reset)
	{
		gNotice.NewMessageDevTeam(lpObj->Index,"%s you can not use my system reset MaxReset: %d",this->BotLoad[number].Name,this->Item_Req[number].reset);	
		LogAdd(LOG_RED,"[%s] CharName: %s Excceded MaxReset: %d ",this->BotLoad[number].Name,lpObj->Name,lpObj->Reset);
		goto Cancel;
	}
	if(this->Item_Req[number].Zen > gObj[index].Money )
	{
		gNotice.NewMessageDevTeam(lpObj->Index,"%s You need more money: %d",this->BotLoad[number].Name,this->Item_Req[number].Zen);
		LogAdd(LOG_RED,"[%s] CharName: %s Insuficent Money: %d ",this->BotLoad[number].Name,lpObj->Name,lpObj->Money);
		goto Cancel;
	}
	if(this->Item_Req[number].PCP > gObj[index].PCPoint )
	{
		gNotice.NewMessageDevTeam(lpObj->Index,"%s You need more PCPoint: %d",this->BotLoad[number].Name,this->Item_Req[number].PCP);
		LogAdd(LOG_RED,"[%s] CharName: %s Insuficent PCPoint: %d ",this->BotLoad[number].Name,lpObj->Name,lpObj->PCPoint);
		goto Cancel;
	}	
	if(this->BotLoad[number].ActiveGensFamily == 1)
	{
		if(this->BotLoad[number].GensSystem != GENS_FAMILY_VARNERT && gObj[index].GensFamily == GENS_FAMILY_NONE )
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s You Need be GensFamily Vanert",this->BotLoad[number].Name);
			return 1;
		}
		if (this->BotLoad[number].GensSystem != GENS_FAMILY_DUPRIAN && gObj[index].GensFamily == GENS_FAMILY_NONE)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s You Need be GensFamily Duprian",this->BotLoad[number].Name);
			return 1;
		}
		if (this->BotLoad[number].GensSystem == GENS_FAMILY_VARNERT && gObj[index].GensFamily == GENS_FAMILY_DUPRIAN)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s You Need be GensFamily Duprian",this->BotLoad[number].Name);
			return 1;
		}
		if (this->BotLoad[number].GensSystem == GENS_FAMILY_DUPRIAN && gObj[index].GensFamily == GENS_FAMILY_VARNERT)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s You Need be GensFamily Vanert",this->BotLoad[number].Name);
			return 1;
		}
		if(this->BotLoad[number].ReqContribution > gObj[index].GensContribution)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s Insuficent GensContribution : %d",this->BotLoad[number].Name,this->BotLoad[number].ReqContribution);
			LogAdd(LOG_RED,"[%s] Account: %s - Insuficent GensContribution : %d",this->BotLoad[number].Name,gObj[index].Account,this->BotLoad[number].ReqContribution);
			return 1;
		}
	}
	if(this->Item_Req[number].ItemReq != 0 ) 
	{
		if( !this->CheckItemReq(lpObj) )
		{
			return false;
		}
		
		for( int i = 0; i < this->m_ItemLoaded; i++ )
		{
			this->DeleteItem(lpObj, i);
		}
		
		if(this->Add_Item[number].ItemReward != 0)
		{
			this->AddResetItem(lpObj->Index,0,0);
		}
		
	}
	else if(this->Add_Item[number].ItemReward != 0  || this->Item_Req[number].ItemReq == 0)
	{
		for( int i = 0; i < this->m_ItemLoaded; i++ )
		{
			this->DeleteItem(lpObj, i);
		}

		this->AddResetItem(lpObj->Index,0,0);
	}

	if(this->Item_Req[number].ResetType == 0)
	{
		lpObj->Level = 1;
		
		lpObj->Reset += 1;
	
		lpObj->PCPoint -= this->Item_Req[number].PCP;

		lpObj->Money -= this->Item_Req[number].Zen;

		GDResetInfoSaveSend(lpObj->Index,0,0,0);

		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"New Level: %d",lpObj->Level);
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"New Reset: %d",lpObj->Reset);
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"New Reset: %s ",lpObj->Name);
		if(this->Item_Req[number].PCP != 0){ gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"-%d PCPoint: %d",this->Item_Req[number].PCP,
			lpObj->PCPoint); }

		LogAdd(LOG_BOT,"[BotReset] CharName: %s successful Reset",lpObj->Name);

	}
	if(this->Item_Req[number].ResetType == 1)
	{
		lpObj->Level = 1;

		lpObj->MasterLevel = 1;
		
		lpObj->MasterReset += 1;

		lpObj->MasterPoint = this->Add_Point[number].AddMasterP;
	
		if(this->Add_Point[number].AddMasterP == 0)
		{
			lpObj->MasterPoint = this->Add_Point[number].MulMasterP*lpObj->MasterReset;
		}

		lpObj->PCPoint -= this->Item_Req[number].PCP;

		lpObj->Money -= this->Item_Req[number].Zen;

		GDMasterResetInfoSaveSend(lpObj->Index,0,0,0);

		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"New MasterLevel: %d",lpObj->MasterLevel);
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"New MasterReset: %d",lpObj->MasterReset);
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"New MasterPoint: %d",lpObj->MasterPoint);
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"New MasterReset: %s ",lpObj->Name);
		if(this->Item_Req[number].PCP != 0){ gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"-%d PCPoint: %d",this->Item_Req[number].PCP,
			lpObj->PCPoint); }

		gObjectManager.CharacterGameCloseSet(lpObj->Index,1);

		LogAdd(LOG_BOT,"[BotReset] CharName: %s successful MasterReset",lpObj->Name);
	}

	if(this->MarryReq[number].MarryActive == 1)
	{
		if(lpObj->MarryCharacterInfo.Married != 0 )
		{
			lpObj->LevelUpPoint += this->MarryReq[number].MarryMulLevelUp*(lpObj->Reset);

			lpObj->Money += this->MarryReq[number].MarryMulMoney*(lpObj->Reset);

			gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"Obtained %d UpPoint and %d Zen by Marriage",lpObj->LevelUpPoint,
				lpObj->Money);

			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"Congratulation! %s Obtained",lpObj->Name);
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"%d UpPoint and Zen %d by Bonus Marriage!",lpObj->LevelUpPoint,lpObj->Money);
	
		}
	}
	
	GCMoneySend(lpObj->Index,lpObj->Money);
	
	gObjectManager.CharacterCalcAttribute(lpObj->Index);

	GCNewCharacterInfoSend(lpObj);

	GDCharacterInfoSaveSend(lpObj->Index);

	switch(lpObj->Class)
		{
			case CLASS_DW:
				gObjMoveGate(lpObj->Index,17);
				break;
			case CLASS_DK:
				gObjMoveGate(lpObj->Index,17);
				break;
			case CLASS_FE:
				gObjMoveGate(lpObj->Index,27);
				break;
			case CLASS_MG:
				gObjMoveGate(lpObj->Index,17);
				break;
			case CLASS_DL:
				gObjMoveGate(lpObj->Index,17);
				break;
			case CLASS_SU:
				gObjMoveGate(lpObj->Index,267);
				break;
			case CLASS_RF:
				gObjMoveGate(lpObj->Index,17);
				break;
		}
Cancel:
	gObjTradeCancel(index);
	gTrade.GCTradeResultSend(index,3);
	
}

bool BotReset::InventorySearchItem(LPOBJ lpObj, BYTE SItemID)
{
    WORD ItemID = this->Item_Req[SItemID].Num;
    
    for( int i = INVENTORY_WEAR_SIZE; i < MAIN_INVENTORY_NORMAL_SIZE; i++ )
    {
        if( !lpObj->Inventory[i].IsItem() || lpObj->Inventory[i].m_Index != ItemID )
        {
            continue;
        }    
        if( lpObj->Inventory[i].m_Level < this->Item_Req[SItemID].ItemMinLevel
            || lpObj->Inventory[i].m_Level > this->Item_Req[SItemID].ItemMaxLevel )
        {
            continue;
        }      
        if( lpObj->Inventory[i].m_Durability < (float)this->Item_Req[SItemID].ItemDur )
        {
            continue;
        }
        return true;
    }

    return false;
}

bool BotReset::DeleteItem(LPOBJ lpObj, BYTE SItemID)
{
	 WORD ItemID = this->Item_Req[SItemID].Num;

	for( int i = INVENTORY_WEAR_SIZE; i < MAIN_INVENTORY_NORMAL_SIZE; i++ )
    {
        if( !lpObj->Inventory[i].IsItem() || lpObj->Inventory[i].m_Index != ItemID )
        {
            continue;
        }       
        if( lpObj->Inventory[i].m_Level < this->Item_Req[SItemID].ItemMinLevel
            || lpObj->Inventory[i].m_Level > this->Item_Req[SItemID].ItemMaxLevel )
        {
            continue;
        }      
        if( lpObj->Inventory[i].m_Durability < (float)this->Item_Req[SItemID].ItemDur )
        {
            continue;
        }
		gItemManager.InventoryDelItem(lpObj->Index,i);
		gItemManager.GCItemDeleteSend(lpObj->Index,i,1);
		return true;
    }   
    return false;
}

bool BotReset::CheckItemReq(LPOBJ lpObj)
{
	int BotNum = 0;

	LPOBJ lpBot = &gObj[BotNum];

	if( !this->InventorySearchItem(lpObj, 0) )
	{
		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,"You have not required items for Reset: %s",
			ItemAttribute[GET_ITEM(this->Item_Req[BotNum].ItemType,this->Item_Req[BotNum].ItemIndex)].Name);
		
        return false;
    }
     return true;
}

void BotReset::AddResetItem(int iUser,BYTE SocketBonus,LPBYTE SocketOptions)	
{
	int BotNum = 0;
	LPOBJ lpBot = &gObj[BotNum];

	if ( OBJMAX_RANGE(iUser) == FALSE )
	{
		return;
	}
		
	GDCreateItemSend(gObj[iUser].Index,0xEB,gObj[iUser].X,gObj[iUser].Y,GET_ITEM(this->Add_Item[BotNum].ItemWinType,
		this->Add_Item[BotNum].ItemWinIndex),this->Add_Item[BotNum].ItemWinLevel,this->Add_Item[BotNum].ItemWinDur,
		this->Add_Item[BotNum].ItemWinSkill,
		this->Add_Item[BotNum].ItemWinLuck,
		this->Add_Item[BotNum].ItemWinOpt,
		-1,
		this->Add_Item[BotNum].ItemWinExc,
		0,0,
		SocketBonus,0,0xFE,0);
	
	g_ShopPointEx.SendNotify(gObj[iUser].Index,this->Add_Item[BotNum].WCoinCWin,this->Add_Item[BotNum].WCoinPWin,
		this->Add_Item[BotNum].GoblinPointWin,0);

}


