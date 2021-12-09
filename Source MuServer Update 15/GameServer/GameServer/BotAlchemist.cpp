#include "stdafx.h"
#include "GameServer.h"
#include "Log.h"
#include "GameMain.h"
#include "User.h"
#include "Protocol.h"
#include "BotAlchemist.h"
#include "Skill.h"
#include "DevilSquare.h"
#include "BloodCastle.h"
#include "ChaosCastle.h"
#include "util.h"
#include "readscript.h"
#include "SetItemOption.h"
#include "NewsProtocol.h"
#include "ObjectManager.h"
#include "Item.h"
#include "Monster.h"
#include "MuunSystem.h"
#include "Trade.h"
#include "DSProtocol.h"
#include "ServerInfo.h"
#include "Notice.h"
#include "Message.h"
#include "GensSystem.h"
#include "PG_Custom.h"

#if (BOT_ALCHEMIST == 1)

ObjBotAlchemist BotAlchemist;

void ObjBotAlchemist::Read(char * FilePath)
{
	
	if(this->Enabled == true)
	{
		for(int botNum=0;botNum<MAX_BOTALCHEMIST;botNum++)
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
	memset(&this->bot,0,sizeof(this->bot));
	this->Enabled = FALSE;
	int Token;

	if (FilePath == NULL || !strcmp(FilePath, ""))
	{
		ErrorMessageBox("[BotAlchemist] file load error - File Name Error");
		return ;
	}

	SMDFile = fopen(FilePath, "r");

	if ( SMDFile == NULL )
	{
		ErrorMessageBox("BotAlchemist data load error %s", FilePath);
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
				
				if(BotNum < 0 || BotNum > MAX_BOTALCHEMIST-1)
				{
					LogAdd(LOG_BOT,"BotAlchemist error: BotPetIndex:%d out of range!", BotNum);
					return;
				}

				Token = GetToken();
				this->bot[BotNum].Class = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].Enabled = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].ChangeColorName = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].Rate = TokenNumber;

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
				
				if(BotNum < 0 || BotNum > MAX_BOTALCHEMIST-1)
				{
					LogAdd(LOG_BOT,"BotAlchemist error: %d out of range!", BotNum);
					return;
				}

				Token = GetToken();
				this->bot[BotNum].OnlyVip = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].ActiveGensFamily = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].ContributionGens = TokenNumber;

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
				if(BotNum < 0 || BotNum > MAX_BOTALCHEMIST-1)
				{
					LogAdd(LOG_BOT,"BotAlchemist error: BotAlchemistIndex 2: %d doesnt exist", BotNum);
					return;
				}
				
				Token = GetToken();
				int Slot = TokenNumber;
				
				if(Slot < 0 || Slot > 8)
				{	
					LogAdd(LOG_BOT,"BotAlchemist error: Min Slot 0 ; Max Slot 8");
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
				if(BotNum < 0 || BotNum > MAX_BOTALCHEMIST-1)
				{
					LogAdd(LOG_BOT,"BotAlchemist error: BotAlchemistIndex 3:%d doesnt exist", BotNum);
					return;
				}
								
				Token = GetToken();
				this->bot[BotNum].AllowLevel = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].AllowOpt = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].AllowLuck = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].AllowSkill = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].AllowExc = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].AllowFFFF = TokenNumber;

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
				if(BotNum < 0 || BotNum > MAX_BOTALCHEMIST-1)
				{
					LogAdd(LOG_BOT,"BotAlchemist error: BotAlchemistIndex 4:%d doesnt exist", BotNum);
					return;
				}

				Token = GetToken();
				this->bot[BotNum].OnlySameType = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].OnlyLowerIndex = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].AcceptAncient = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].MaxLevel = TokenNumber;

				Token = GetToken();
				this->bot[BotNum].MaxExc = TokenNumber;
			}
		}
		break;
	}
	fclose(SMDFile);		
}

void ObjBotAlchemist::MakeBot()
{
	if(this->Enabled == true)
	{
		for(int botNum=0;botNum<MAX_BOTALCHEMIST;botNum++)
		{
			if(this->bot[botNum].Enabled == true)
			{
				CItem item;

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

BOOL ObjBotAlchemist::IsInTrade(int BotIndex)
{
	int bIndex = this->GetBotIndex(gObj[BotIndex].TargetNumber);
	if(bIndex != -1)
		return 1;
	else
		return 0;
		
}

int ObjBotAlchemist::GetBotIndex(int aIndex)
{
	for(int i=0;i<MAX_BOTALCHEMIST;i++)
	{
		if(this->bot[i].Enabled == true)
		{
			if(this->bot[i].index == aIndex)
			{
				return i;
			}
		}
	}
	return -1;
}

bool ObjBotAlchemist::AllowExc(BYTE BotNum, BYTE ExcOpt)
{				
	int dwExOpCount = getNumberOfExcOptions(ExcOpt);

	if(dwExOpCount < this->bot[BotNum].MaxExc)
	{
		return true;
	}
	return false;
}

BYTE ObjBotAlchemist::Alchemy(int aIndex,int BotNum)
{
	int fitem=-1;
	int sitem=-1;
	int count=0;

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return 0;
	}

	if((gObj[aIndex].TradeMoney - this->bot[BotNum].Zen) < 0)
	{
		gNotice.NewMessageDevTeam(gObj[aIndex].Index,"%s Insuficent Zen! %d ",this->bot[BotNum].Name,this->bot[BotNum].Zen);
		LogAdd(LOG_RED,"[%s] - (%s) Insuficent Zen",this->bot[BotNum].Name,gObj[aIndex].Name);
		goto Cancel;		
	}

	if((gObj[aIndex].PCPoint - this->bot[BotNum].PCPoints) < 0)
	{
		gNotice.NewMessageDevTeam(gObj[aIndex].Index,"%s Insuficent PCPoint! %d ",this->bot[BotNum].Name,this->bot[BotNum].PCPoints);
		LogAdd(LOG_RED,"[%s] - (%s) Insuficent PCPoint",this->bot[BotNum].Name,gObj[aIndex].Name);
		goto Cancel;		
	}

	for(int n = 0; n < TRADE_SIZE; n++)
	{
		if(gObj[aIndex].Trade[n].IsItem() == 1)
		{
			if((n >= 0 && n<=3) || (n >= 8 && n<=11) || (n >= 16 && n<=19) || (n >= 24 && n<=27))
			{
				fitem = n;
			}else
			{
				sitem = n;
			}
			count++;
		}
	}
	if(count != 2)
	{
		gNotice.NewMessageDevTeam(gObj[aIndex].Index,"%s You must put only 2 items!",this->bot[BotNum].Name);
		LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) Items Count Error (%d)",gObj[aIndex].Name,gObj[aIndex].Name,count);
		goto Cancel;
	}
	if((gObj[aIndex].Trade[fitem].m_Index >= 6144) || (gObj[aIndex].Trade[sitem].m_Index >= 6144))
	{
		gNotice.NewMessageDevTeam(gObj[aIndex].Index,"%s Only weapons and sets are allowed!!",this->bot[BotNum].Name);
		LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) Items Error",gObj[aIndex].Account,gObj[aIndex].Name);
		goto Cancel;
	}
	if(fitem == -1 || sitem == -1)
	{
		gNotice.NewMessageDevTeam(gObj[aIndex].Index,"%s Items Position Error!",this->bot[BotNum].Name);
		LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) Items Position Error",gObj[aIndex].Account,gObj[aIndex].Name);
		goto Cancel;
	}
	if(this->bot[BotNum].OnlySameType == 1)
	{
		int fType = gObj[aIndex].Trade[fitem].m_Index/512;
		int sType = gObj[aIndex].Trade[sitem].m_Index/512;
		if(fType != sType)
		{
			gNotice.NewMessageDevTeam(gObj[aIndex].Index,"%s You must put two items of same type!",this->bot[BotNum].Name);
			LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) Item Types Error",gObj[aIndex].Account,gObj[aIndex].Name);
			goto Cancel;
		}
	}
	if(this->bot[BotNum].OnlyLowerIndex == 1)
	{
		int fLevel = ItemAttribute[gObj[aIndex].Trade[fitem].m_Index].Level;
		int sLevel = ItemAttribute[gObj[aIndex].Trade[sitem].m_Index].Level;

		if(fLevel > sLevel)
		{
			if(fLevel > (sLevel + 10))
			{
				gNotice.NewMessageDevTeam(gObj[aIndex].Index,"%s Only lower items than source works with my alchemy!: %s",this->bot[BotNum].Name,gObj[aIndex].Name);
				LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) Item Destiny Lower Index Error",gObj[aIndex].Account,gObj[aIndex].Name);
				goto Cancel;
			}
		}
	}
	if(this->bot[BotNum].AcceptAncient == 0 && gObj[aIndex].Trade[fitem].m_SetOption > 0)
	{
		gNotice.NewMessageDevTeam(gObj[aIndex].Index,"%s I cant work with Ancient Items!: %s",this->bot[BotNum].Name,gObj[aIndex].Name);
		LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) Ancient Item Error",gObj[aIndex].Account,gObj[aIndex].Name);
		goto Cancel;
	}
	if(this->bot[BotNum].AllowFFFF == 0)
	{
		if(gObj[aIndex].Trade[fitem].GetNumber() >= 0xFFFFFFF0 || gObj[aIndex].Trade[sitem].GetNumber() >= 0xFFFFFFF0)
		{
			gNotice.NewMessageDevTeam(gObj[aIndex].Index,"%s You cant transmutate those items!: %s",this->bot[BotNum].Name,gObj[aIndex].Name);
			LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) FFFF Item Error",gObj[aIndex].Account,gObj[aIndex].Name);
			goto Cancel;
		}
	}
	srand(static_cast<int>(time(NULL)));
	int random = rand()%100;
	bool failed = false;

	if(random > this->bot[BotNum].Rate )
	{
		failed = true;
	}
	if(this->bot[BotNum].AllowLuck == 1)
	{
		if(gObj[aIndex].Trade[fitem].m_Option2 == 0 && gObj[aIndex].Trade[sitem].m_Option2 == 1)
		{
			if(!failed)
				gObj[aIndex].Trade[fitem].m_Option2 = 1;
			gObj[aIndex].Trade[sitem].m_Option2 = 0;
		}
	}
	if(this->bot[BotNum].AllowLevel == 1)
	{
		int fLevel = gObj[aIndex].Trade[fitem].m_Level;
		if(fLevel < this->bot[BotNum].MaxLevel)
		{
			int sLevel = gObj[aIndex].Trade[sitem].m_Level;
			if ((fLevel + sLevel) > this->bot[BotNum].MaxLevel)
			{
				sLevel -= (this->bot[BotNum].MaxLevel - fLevel);
				if(!failed)
					fLevel = this->bot[BotNum].MaxLevel;
			}else
			{
				if(!failed)
					fLevel += sLevel;
				sLevel = 0;
			}
			gObj[aIndex].Trade[fitem].m_Level = fLevel;
			gObj[aIndex].Trade[sitem].m_Level = sLevel;
		}
	}
	if(this->bot[BotNum].AllowSkill == 1)
	{
		if(gObj[aIndex].Trade[fitem].m_Index < 3584)
		{
			if(gObj[aIndex].Trade[fitem].m_Option1 == 0 && gObj[aIndex].Trade[sitem].m_Option1 == 1)
			{
				if(!failed)
					gObj[aIndex].Trade[fitem].m_Option1 = 1;
				gObj[aIndex].Trade[sitem].m_Option1 = 0;
			}
		}
	}
	if(this->bot[BotNum].AllowOpt == 1)
	{
		int fOpt = gObj[aIndex].Trade[fitem].m_Option3;
		if(fOpt < 7)
		{
			int sOpt = gObj[aIndex].Trade[sitem].m_Option3;
			if ((fOpt + sOpt) > 7)
			{
				sOpt -= (7 - fOpt);
				if(!failed)
					fOpt = 7;
			}else
			{
				if(!failed)
					fOpt += sOpt;
				sOpt = 0;
			}
			gObj[aIndex].Trade[fitem].m_Option3 = fOpt;
			gObj[aIndex].Trade[sitem].m_Option3 = sOpt;
		}
	}
	if(this->bot[BotNum].AllowExc == 1)
	{
		if(gObj[aIndex].Trade[fitem].m_NewOption != 63)
		{
			if(gObj[aIndex].Trade[sitem].m_NewOption != 0)
			{		
				if(this->AllowExc(BotNum,gObj[aIndex].Trade[fitem].m_NewOption) == true)
				{
					BYTE fLife		= gObj[aIndex].Trade[fitem].IsExtLifeAdd();
					BYTE fMana		= gObj[aIndex].Trade[fitem].IsExtManaAdd();
					BYTE fDmg		= gObj[aIndex].Trade[fitem].IsExtDamageMinus();
					BYTE fReflect	= gObj[aIndex].Trade[fitem].IsExtDamageReflect();
					BYTE fDef		= gObj[aIndex].Trade[fitem].IsExtDefenseSuccessfull();
					BYTE fMoney		= gObj[aIndex].Trade[fitem].IsExtMonsterMoney();
					BYTE sLife		= gObj[aIndex].Trade[sitem].IsExtLifeAdd();
					BYTE sMana		= gObj[aIndex].Trade[sitem].IsExtManaAdd();
					BYTE sDmg		= gObj[aIndex].Trade[sitem].IsExtDamageMinus();
					BYTE sReflect	= gObj[aIndex].Trade[sitem].IsExtDamageReflect();
					BYTE sDef		= gObj[aIndex].Trade[sitem].IsExtDefenseSuccessfull();
					BYTE sMoney		= gObj[aIndex].Trade[sitem].IsExtMonsterMoney();

					if(fLife == 0 && sLife != 0)
					{
						if(!failed)
							gObj[aIndex].Trade[fitem].m_NewOption |= 0x20;
						gObj[aIndex].Trade[sitem].m_NewOption &= ~0x20;
					}
					if(this->AllowExc(BotNum,gObj[aIndex].Trade[fitem].m_NewOption) == true)
					{
						if(fMana == 0 && sMana != 0)
						{
							if(!failed)
								gObj[aIndex].Trade[fitem].m_NewOption |= 0x10;
							gObj[aIndex].Trade[sitem].m_NewOption &= ~0x10;
						}
					}
					if(this->AllowExc(BotNum,gObj[aIndex].Trade[fitem].m_NewOption) == true)
					{
						if(fDmg == 0 && sDmg != 0)
						{
							if(!failed)
								gObj[aIndex].Trade[fitem].m_NewOption |= 8;
							gObj[aIndex].Trade[sitem].m_NewOption &= ~8;
						}
					}
					if(this->AllowExc(BotNum,gObj[aIndex].Trade[fitem].m_NewOption) == true)
					{
						if(fReflect == 0 && sReflect != 0)
						{
							if(!failed)
								gObj[aIndex].Trade[fitem].m_NewOption |= 4;
							gObj[aIndex].Trade[sitem].m_NewOption &= ~4;
						}
					}
					if(this->AllowExc(BotNum,gObj[aIndex].Trade[fitem].m_NewOption) == true)
					{
						if(fDef == 0 && sDef != 0)
						{
							if(!failed)
								gObj[aIndex].Trade[fitem].m_NewOption |= 2;
							gObj[aIndex].Trade[sitem].m_NewOption &= ~2;
						}
					}
					if(this->AllowExc(BotNum,gObj[aIndex].Trade[fitem].m_NewOption) == true)
					{
						if(fMoney == 0 && sMoney != 0)
						{
							if(!failed)
							{
								gObj[aIndex].Trade[fitem].m_NewOption |= 1;
							gObj[aIndex].Trade[sitem].m_NewOption &= ~1;
							}
						}
					}
				}
			}
		}
	}
	int a = gObjInventoryInsertItem(aIndex,gObj[aIndex].Trade[fitem]);
	int b = gObjInventoryInsertItem(aIndex,gObj[aIndex].Trade[sitem]);

	if(a == 255 || b == 255)
	{
		gNotice.NewMessageDevTeam(gObj[aIndex].Index,"%s Insuficent space in inventory!: %s",this->bot[BotNum].Name,gObj[aIndex].Name);
		LogAdd(LOG_RED,"[BotAlchemist] Account: %s - Name: %s Doesnt have space on inventory",gObj[aIndex].Account,gObj[aIndex].Name);
		goto Cancel;
	}
	if(failed)
	{
		gNotice.NewMessageDevTeam(gObj[aIndex].Index,"%s Transmutation failed!: %s",this->bot[BotNum].Name,gObj[aIndex].Name);
		LogAdd(LOG_RED,"[BotAlchemist] - [Bot: %d] Name: %s Alchemy Failed",this->bot[BotNum].Name,gObj[aIndex].Name);
	}
	else
	{ 
		gNotice.NewMessageDevTeam(gObj[aIndex].Index,"%s Transmutation success!: %s",this->bot[BotNum].Name,gObj[aIndex].Name);
		LogAdd(LOG_BOT,"[BotAlchemist] - [Bot: %s] Name: %s Alchemy Success",this->bot[BotNum].Name,gObj[aIndex].Name);
	}

	gObj[aIndex].Money -= this->bot[BotNum].Zen;

	if(this->bot[BotNum].PCPoints > 0)
	{
		gObj[aIndex].PCPoint -= this->bot[BotNum].PCPoints;
		gNotice.NewNoticeSend(gObj[aIndex].Index,0,0,0,0,0,"%s total PCPoint: %d",this->bot[BotNum].Name,gObj[aIndex].PCPoint);
	}

	gObjInventoryCommit(aIndex);
	gObjectManager.CharacterMakePreviewCharSet(aIndex);
	GDCharacterInfoSaveSend(aIndex); 
	
	gItemManager.GCItemEquipmentSend(aIndex);
	
	lpObj->TargetNumber = -1;
	lpObj->Interface.use = 0;
	lpObj->Interface.state = 0;
	lpObj->TradeOk = 0;
	lpObj->TradeMoney = 0;
	GCMoneySend(aIndex,gObj[aIndex].Money);
	gTrade.GCTradeResultSend(aIndex,1);
	gItemManager.GCItemListSend(aIndex);

	for(int n = 0; n < TRADE_SIZE; n++)
	{
		gObj[aIndex].Trade[n].Clear();
	}
	gObjNotifyUpdateUnionV1(&gObj[aIndex]);
	gObjNotifyUpdateUnionV2(&gObj[aIndex]);
	return 1;
Cancel:
	gObjTradeCancel(aIndex);
	gTrade.GCTradeResultSend(aIndex,3);
	
	return 0;
}

void ObjBotAlchemist::TradeOk(int aIndex)
{
	int MixNum=-1;
	int MixItem=-1;

	int number = this->GetBotIndex(gObj[aIndex].TargetNumber);

	if(number == -1)
	{	
		gObj[aIndex].Interface.state = 0;
		gObj[aIndex].Interface.type = 0;
		gObj[aIndex].TargetShopNumber = OBJECT_BOTS;
		gObj[aIndex].Transaction = 0;
		gObj[aIndex].TradeOkTime = GetTickCount();
		gObj[aIndex].TradeMoney = 1;
		gObj[aIndex].TradeOk = 1;

		return;
	}
	this->Alchemy(aIndex,number);
}

BOOL ObjBotAlchemist::TradeOpen(int index, int nindex)
{
	if(gObjIsConnectedGP(index) == 0)
	{
		return 0;
	}

	if(gObjIsConnectedGP(nindex) == 0)
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
		gNotice.NewMessageDevTeam(lpObj->Index,"%s Im Only work for Account Vips!",gObj[nindex].Name);
		LogAdd(LOG_RED,"[%s] AccountLevel Free ( Account: %s Char: %s )",this->bot[number].Name,lpObj->Account,lpObj->Name);
		return 1;
	}

	if(this->bot[number].PCPoints > gObj[index].PCPoint)
	{
		gNotice.NewMessageDevTeam(gObj[index].Index,"%s Insuficent PCPoint : %d",this->bot[number].Name,this->bot[number].PCPoints);
		LogAdd(LOG_RED,"[%s] Account: %s - Insuficent PCPoint : %d",this->bot[number].Name,gObj[index].Account,this->bot[number].PCPoints);
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
		if(this->bot[number].ContributionGens > gObj[index].GensContribution)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,"%s Insuficent GensContribution : %d",this->bot[number].Name,this->bot[number].ContributionGens);
			LogAdd(LOG_RED,"[%s] Account: %s - Insuficent GensContribution : %d",this->bot[number].Name,gObj[index].Account,this->bot[number].ContributionGens);
			return 1;
		}
	}

	if(this->Enabled == TRUE)
	{
		if ( lpObj->Interface.use > 0 )
		{
			return 0;
		}else
		{
			for(int n = 0; n < TRADE_SIZE; n++)
			{
				lpObj->Trade[n].Clear();
			}
			memset(lpObj->TradeMap, (BYTE)-1, TRADE_SIZE );
	
			gObjInventoryTransaction(lpObj->Index);
			gTrade.GCTradeResponseSendBOT(true, lpObj->Index, lpBot->Name, 400, 0);
			gTrade.GCTradeOkButtonSend(lpObj->Index, 1);
			lpObj->Interface.state = 1;
			lpObj->Interface.use = 1;
			lpObj->Interface.type = 1;
			lpObj->TradeMoney = 0;
			lpObj->TargetNumber = lpBot->Index;
			lpObj->Transaction = 1;
			
			gNotice.NewMessageDevTeam(lpObj->Index,"%s I'm Ready: %s",gObj[nindex].Name,gObj[index].Name);
			LogAdd(LOG_BOT,"[Alchemist] [BotName: %s] CharName: %s OPEN",gObj[nindex].Name,gObj[index].Name);
			
			if(this->bot[number].Zen > 0)
			{
				gNotice.NewMessageDevTeam(lpObj->Index,"%s I Need: %d Zen!",gObj[nindex].Name,this->bot[number].Zen);
			}

			if(this->bot[number].PCPoints > 0)
			{
				gNotice.NewMessageDevTeam(lpObj->Index,"%s I Need: %d PCPoint!",gObj[nindex].Name,this->bot[number].PCPoints);
			}
			
		}
	}
	return 1;
}

void ObjBotAlchemist::TradeCancel(int aIndex)
{
	gObjTradeCancel(aIndex);
	gTrade.GCTradeResultSend(aIndex,3);

}

#endif
