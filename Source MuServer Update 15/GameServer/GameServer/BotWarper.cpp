#include "stdafx.h"
#include "GameServer.h"
#include "Log.h"
#include "GameMain.h"
#include "User.h"
#include "Protocol.h"
#include "BotWarper.h"
#include "Skill.h"
#include "DevilSquare.h"
#include "BloodCastle.h"
#include "ChaosCastle.h"
#include "util.h"
#include "readscript.h"
#include "SetItemOption.h"
#include "Move.h"
#include "MapServerManager.h"
#include "Monster.h"
#include "NewsProtocol.h"
#include "ObjectManager.h"
#include "JSProtocol.h"
#include "Notice.h"
#include "MuunSystem.h"
#include "Shop.h"
#include "GensSystem.h"
#include "PG_Custom.h"

#if (BOT_WARPER ==1)

ObjBotWarper BotWarper;

void ObjBotWarper::Init() // OK
{
	char * FilePath;

	if(BotWarper.Enabled == 0)
	{
		Read(FilePath);
	}
}

void ObjBotWarper::Read(char * FilePath)
{
	if(this->Enabled == true)
	{
		for(int botNum=0;botNum<MAX_BOTWARPER;botNum++)
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
	for(int i=0;i<MAX_BOTWARPER;i++)
	{
		this->bot[i].index = -1;
		this->bot[i].Enabled = false;
		for(int j=0;j<9;j++)
			this->bot[i].body[j].num =-1;
	}

	this->Enabled = FALSE;
	int Token;
	SMDFile = fopen(FilePath, "r");

	if ( SMDFile == NULL )
	{
		ErrorMessageBox("Bot Warper data load error %s", FilePath);
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
				if(BotNum < 0 || BotNum > MAX_BOTWARPER-1)
				{
					ErrorMessageBox("BotWarper error: BotPetIndex:%d out of range!", BotNum);
					return;
				}

				Token = GetToken();
				this->bot[BotNum].Class = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].Enabled = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].ChangeColorName = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].MinLevel = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].MinReset = TokenNumber;

				Token = GetToken();
				strncpy(this->bot[BotNum].Name,TokenString,sizeof(this->bot[BotNum].Name));

				Token = GetToken();
				this->bot[BotNum].Map = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].X = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].Y = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].Dir = TokenNumber;

				this->Enabled = true;
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
				if(BotNum < 0 || BotNum > MAX_BOTWARPER-1)
				{
					ErrorMessageBox("BotWarper error: %d doesnt exist", BotNum);
					return;
				}

				Token = GetToken();
				this->bot[BotNum].OnlyVip = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].ActiveGensFamily = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].ReqContribution = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].GensFamily = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].Zen = TokenNumber;
				
				Token = GetToken();
				this->bot[BotNum].PCPoints = TokenNumber;
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
				if(BotNum < 0 || BotNum > MAX_BOTWARPER-1)
				{
					ErrorMessageBox("BotWarper error: BotPetIndex:%d doesnt exist", BotNum);
					return;
				}

				Token = GetToken();
				int Slot = TokenNumber;
				
				if(Slot < 0 || Slot > 8)
				{	
					ErrorMessageBox("BotWarper error: Min Slot 0 ; Max Slot 8");
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
		if ( iType == 4 )
		{
			while(true)
			{
				Token = GetToken();
				if ( strcmp("end", TokenString) == 0 )
				{
					break;
				}
				int BotNum = TokenNumber;
				if(BotNum < 0 || BotNum > MAX_BOTWARPER-1)
				{
					ErrorMessageBox("BotWarper error: BotPetIndex:%d doesnt exist", BotNum);
					return;
				}

				Token = GetToken();
				this->bot[BotNum].Warp_Map = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].Warp_X = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].Warp_Y = TokenNumber;
			}
		}
		break;
	}
	fclose(SMDFile);
}

void ObjBotWarper::MakeBot()
{
	if(this->Enabled == true)
	{
		for(int botNum=0;botNum<MAX_BOTWARPER;botNum++)
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
					gObj[result].IsBot = 2;
					gObj[result].BotPower = 0;
					gObjSetBots(result,this->bot[botNum].Class);

					gObj[result].ChangeUp = this->bot[botNum].Class & 0x07;
					gObj[result].Class = this->bot[botNum].Class;
					gObj[result].Level = 400;
					gObj[result].Life = 1;
					gObj[result].MaxLife = 2;
					gObj[result].Mana = 1;
					gObj[result].MaxMana = 2;
					gObj[result].Experience = 0;
					gObj[result].DBClass = this->bot[botNum].Class;
					gObj[result].PKLevel = this->bot[botNum].ChangeColorName;
					gObj[result].GensFamily = this->bot[botNum].GensFamily;
					
					gObj[result].Inventory = new CItem[MAIN_INVENTORY_NORMAL_SIZE];					
					gObj[result].Inventory1 = new CItem[MAIN_INVENTORY_NORMAL_SIZE];
					gObj[result].InventoryMap1 = new BYTE[MAIN_INVENTORY_NORMAL_SIZE];
					gObj[result].InventoryMap = new BYTE[MAIN_INVENTORY_NORMAL_SIZE];
					
					for (int i=0;i<MAIN_INVENTORY_NORMAL_SIZE;i++)
					{
						gObj[result].Inventory[i].Clear();
						gObj[result].Inventory1[i].Clear();
					
					}
										
					memset(&gObj[result].InventoryMap[0], (BYTE)-1, MAIN_INVENTORY_NORMAL_SIZE);
					memset(&gObj[result].InventoryMap1[0], (BYTE)-1, MAIN_INVENTORY_NORMAL_SIZE);
					
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
					
					GCSummonLifeSend(gObj[result].SummonIndex,gObj[result].MaxLife,gObj[result].Life);

				}
			}
		}
	}
}

int ObjBotWarper::GetBotIndex(int aIndex)
{
	for(int i=0;i<MAX_BOTWARPER;i++)
	{
		if(this->bot[i].Enabled == true)
		{
			if(this->bot[i].index == aIndex)
				return i;
		}
	}
	return -1;
}

BOOL ObjBotWarper::TradeOpen(int index, int nindex)
{
	if ( gObjIsConnectedGP(index) == FALSE )
	{
		LogAdd(LOG_BOT, "error-L3 [%s][%d]", __FILE__, __LINE__);
		return 0;
	}
	if(gObjIsConnected(nindex) == 0)
	{
		return 0;
	}

	int number = this->GetBotIndex(nindex);
	if(number == -1)
		return 0;

	LPOBJ lpObj = &gObj[index];
	LPOBJ lpBot = &gObj[nindex];

	if(this->bot[number].OnlyVip != 0 && lpObj->AccountLevel == 0)
	{
		gNotice.NewMessageDevTeam(gObj[index].Index,"%s Im only work for VIPs!: %s",this->bot[number].Name,gObj[index].Name);
		LogAdd(LOG_BOT,"[Warning] Not Open [Bot: %s] Name: %s  AccountLevel Free",this->bot[number].Name,gObj[index].Account);
		return 1;
	}
	if(gObj[index].Level < this->bot[number].MinLevel)
	{
		gNotice.NewMessageDevTeam(gObj[index].Index,"%s You Need to be level %d at least",this->bot[number].Name,this->bot[number].MinLevel);
		return 1;
	}	
	if(gObj[index].Reset < this->bot[number].MinReset)
	{
		gNotice.NewMessageDevTeam(gObj[index].Index,"%s You Need to have %d resets at least",this->bot[number].Name,this->bot[number].MinReset);
		return 1;
	}
	if(this->bot[number].ActiveGensFamily == 1)
	{
		if(this->bot[number].GensFamily != GENS_FAMILY_VARNERT && gObj[index].GensFamily == GENS_FAMILY_NONE )
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s You Need be GensFamily Vanert",this->bot[number].Name);
			return 1;
		}
		if (this->bot[number].GensFamily != GENS_FAMILY_DUPRIAN && gObj[index].GensFamily == GENS_FAMILY_NONE)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s You Need be GensFamily Duprian",this->bot[number].Name);
			return 1;
		}
		if (this->bot[number].GensFamily == GENS_FAMILY_VARNERT && gObj[index].GensFamily == GENS_FAMILY_DUPRIAN)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s You Need be GensFamily Duprian",this->bot[number].Name);
			return 1;
		}
		if (this->bot[number].GensFamily == GENS_FAMILY_DUPRIAN && gObj[index].GensFamily == GENS_FAMILY_VARNERT)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s You Need be GensFamily Vanert",this->bot[number].Name);
			return 1;
		}
		if(this->bot[number].ReqContribution > gObj[index].GensContribution)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s Insuficent GensContribution : %d",this->bot[number].Name,this->bot[number].ReqContribution);
			LogAdd(LOG_RED,"[%s] Account: %s - Insuficent GensContribution : %d",this->bot[number].Name,gObj[index].Account,this->bot[number].ReqContribution);
			return 1;
		}
	}
	if(this->bot[number].Zen > gObj[index].Money)
	{
		gNotice.NewMessageDevTeam(gObj[index].Index,"%s Insuficent Money: %d",this->bot[number].Name,this->bot[number].Zen);
		return 1;		
	}
	if(this->bot[number].PCPoints > gObj[index].PCPoint)
	{
		gNotice.NewMessageDevTeam(gObj[index].Index,"%s Insuficent PCPoint: %d",this->bot[number].Name,this->bot[number].PCPoints);
		return 1;		
	}

	int m_check1 = gMove.CheckMainToMove(lpObj);
	int m_check2 = gMove.CheckEquipmentToMove(lpObj,lpObj->Map);
	int m_check3 = gMove.CheckInterfaceToMove(lpObj);

	if(m_check1 == false || m_check2 == false || m_check3 == false)
	{
		gNotice.NewMessageDevTeam(gObj[index].Index,"%s I cant warp you there!",this->bot[number].Name);		
	}

	int addx;
	int addy;
	int count = 50;
	int Find = 0;

	while(count--)
	{
		addx = this->bot[number].Warp_X + rand()%9 - 4;
		addy = this->bot[number].Warp_Y + rand()%9 - 4;

		if(gObjCheckTeleportArea(this->bot[number].Warp_Map,addx,addy))
		{
			Find = 1;
			break;
		}
	}
	if(Find == 0)
	{
		addx = this->bot[number].Warp_X;
		addy = this->bot[number].Warp_Y;
	}

	gNotice.NewMessageDevTeam(gObj[index].Index,"%s I will warp you!!",this->bot[number].Name);

	if(this->bot[number].Zen > 0)
	{
		gObj[index].Money -= this->bot[number].Zen;
		GCMoneySend(index, gObj[index].Money);
	}

	if(this->bot[number].PCPoints > 0)
	{
		gObj[index].PCPoint -= this->bot[number].PCPoints;
		gNotice.NewNoticeSend(gObj[index].Index,0,0,0,0,0,"%s PCPoint: %d",this->bot[number].Name,gObj[index].PCPoint);
	}

	if(lpObj->Map != this->bot[number].Warp_Map)
	{
		short sSvrCode = gMapServerManager.CheckMapServerMove(lpObj->Index,this->bot[number].Warp_Map,lpObj->NextServerCode);

		if(sSvrCode != ServerCode)
		{
			if(sSvrCode == -1)
			{
				LogAdd(LOG_BOT,"[MapServerMng] Map Server Move Fail : CheckMoveMapSvr() == -1 [%s][%s] (%d)",lpObj->Account,lpObj->Name,lpObj->Index);
				return 1;
			}

			if(lpObj->PShopOpen == 1)
			{
				LogAdd(LOG_BOT,"[MapServerMng] Map Server Move Fail : PShopOpen == 1 [%s][%s] (%d)",lpObj->Account,lpObj->Name,lpObj->Index);
				return 1;
			}

			GJMapServerMoveSend(lpObj->Index,sSvrCode,this->bot[number].Warp_Map,addx,addy);
			LogAdd(LOG_BOT,"[MapServerMng] Request to Move Map Server : (%d) - [%s][%s] (%d)",sSvrCode,lpObj->Account,lpObj->Name,lpObj->Index);
			return 1;
		}
	}
	lpObj->SkillSummonPartyTime = 3;
	lpObj->SkillSummonPartyMap = this->bot[number].Warp_Map;
	lpObj->SkillSummonPartyX = addx;
	lpObj->SkillSummonPartyY = addy;
	return 1;
}

#endif