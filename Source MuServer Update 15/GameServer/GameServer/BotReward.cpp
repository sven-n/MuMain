#include "stdafx.h"
#include "GameServer.h"
#include "Log.h"
#include "GameMain.h"
#include "User.h"
#include "DSProtocol.h"
#include "PG_Custom.h"

#include "BotReward.h"
#include "Skill.h"
#include "DevilSquare.h"
#include "BloodCastle.h"
#include "ChaosCastle.h"
#include "util.h"
#include "readscript.h"
#include "SetItemOption.h"
#include "Monster.h"
#include "NewsProtocol.h"
#include "ObjectManager.h"
#include "Notice.h"
#include "MuunSystem.h"

#if (BOT_REWARD ==1)

ObjBotReward BotReward;


void ObjBotReward::Read(char * FilePath)
{
	this->Enabled = FALSE;

	int Token;
	SMDFile = fopen(FilePath, "r");

	if ( SMDFile == NULL )
	{
		ErrorMessageBox("BotReward data load error %s", FilePath);
		return;
	}

	while ( true )
	{
		int iType = GetToken();
		
		if ( iType == 1 )
		{
			Token = GetToken();
			if ( strcmp("end", TokenString) == 0 )
			{
				break;
			}

			this->bot.Class = TokenNumber;

			Token = GetToken();
			strncpy(this->bot.Name,TokenString,sizeof(this->bot.Name));

			Token = GetToken();
			this->bot.Map = TokenNumber;

			Token = GetToken();
			this->bot.X = TokenNumber;

			Token = GetToken();
			this->bot.Y = TokenNumber;

			Token = GetToken();
			this->bot.Dir = TokenNumber;

			this->bot.Enabled = true;
		}
		
		GetToken();
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

				if(this->bot.Enabled == false)
				{	
					ErrorMessageBox("BotReward error: BotPet doesnt exist");
					return;
				}

				int Slot = TokenNumber;
				
				if(Slot < 0 || Slot > 8)
				{	
					ErrorMessageBox("BotReward error: Min Slot 0 ; Max Slot 8");
					return;
				}

				Token = GetToken();
				int iType = TokenNumber;

				Token = GetToken();
				int iIndex = TokenNumber;

				this->bot.body[Slot].num = GET_ITEM(iType,iIndex);

				Token = GetToken();
				this->bot.body[Slot].level = TokenNumber;

				Token = GetToken();
				this->bot.body[Slot].opt = TokenNumber;

				this->bot.body[Slot].Enabled = true;
			}
		}
		break;
	}

	this->Enabled = true;

	fclose(SMDFile);
}

void ObjBotReward::MakeBot()
{
	if(this->Enabled == true)
	{
		if(this->bot.Enabled == true)
		{
			int result = gObjAddSummon();

			if(result >= 0)
			{
				this->bot.index = result;
				gObj[result].PosNum = (WORD)-1;
				gObj[result].X = this->bot.X;
				gObj[result].Y = this->bot.Y;
				gObj[result].OldX = this->bot.X;
				gObj[result].OldY = this->bot.Y;
				gObj[result].TX = this->bot.X;
				gObj[result].TY = this->bot.Y;
				gObj[result].MTX = this->bot.X;
				gObj[result].MTY = this->bot.Y;
				gObj[result].Dir = this->bot.Dir;
				gObj[result].Map = this->bot.Map;
				gObj[result].Live = TRUE;
				gObj[result].PathCount = 0;
				gObj[result].IsBot = 9;
				gObj[result].BotPower = 0;
				gObjSetBots(result,this->bot.Class);

				gObj[result].ChangeUp = this->bot.Class & 0x07;	// Set Second Type of Character
				gObj[result].Class = this->bot.Class;
				gObj[result].Level = 400;
				gObj[result].Life = 1;
				gObj[result].MaxLife = 2;
				gObj[result].Mana = 1;
				gObj[result].MaxMana = 2;
				gObj[result].Experience = 0;
				gObj[result].DBClass = this->bot.Class;
				
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
				
				strncpy(gObj[result].Name,this->bot.Name,sizeof(gObj[result].Name));


				for(int i=0;i<9;i++)
				{
					if(this->bot.body[i].num >= 0 && this->bot.body[i].Enabled == true)
					{
							CItem item;
							item.m_Level = this->bot.body[i].level;
							item.m_Option1 = 0;
							item.m_Option2 = 1;
							item.m_Option3 = this->bot.body[i].opt;
							item.m_Durability = 255.0f;
							item.m_JewelOfHarmonyOption = 0;
							item.m_ItemOptionEx = 0;
							item.m_SocketOption[1] = 0;
							item.m_SocketOption[2] = 0;
							item.m_SocketOption[3] = 0;
							item.m_SocketOption[4] = 0;
							item.m_SocketOption[5] = 0;
							item.Convert(this->bot.body[i].num,item.m_Option1,item.m_Option2,item.m_Option3,item.m_NewOption,item.m_SetOption,item.m_JewelOfHarmonyOption,item.m_ItemOptionEx,item.m_SocketOption,item.m_SocketOptionBonus);
							

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

int ObjBotReward::GetBotIndex(int aIndex)
{
	if(this->bot.Enabled == true)
	{
		if(this->bot.index == aIndex)
		{
			return this->bot.index;
		}
	}
	return -1;
}

BOOL ObjBotReward::TradeOpen(int index, int nindex)
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
	{
		return 0;
	}

	GetPlayerExtraInfo(index,GSSend_GetBotReward);

	gNotice.NewMessageDevTeam(index,"%s Wait, i will check i have some reward for you!",this->bot.Name);
	return 1;
}

#endif