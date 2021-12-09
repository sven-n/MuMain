// CustomQuest.cpp: implementation of the CCustomQuest class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandManager.h"
#include "CustomNpcQuest.h"
#include "CustomRankUser.h"
#include "DSProtocol.h"
#include "ItemManager.h"
#include "GameMain.h"
#include "Log.h"
#include "MapManager.h"
#include "MemScript.h"
#include "Message.h"
#include "MonsterManager.h"
#include "Notice.h"
#include "NpcTalk.h"
#include "ObjectManager.h"
#include "Protocol.h"
#include "ServerInfo.h"
#include "SocketItemType.h"
#include "Util.h"


CCustomNpcQuest gCustomNpcQuest;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomNpcQuest::CCustomNpcQuest() // OK
{
	this->Init();
}

CCustomNpcQuest::~CCustomNpcQuest() // OK
{

}

void CCustomNpcQuest::Init() // OK
{
	this->m_CustomNpcQuestNpcTypeInfo.clear();
	this->m_CustomNpcQuestCharacterInfo.clear();
	this->m_CustomNpcQuestItemInfo.clear();
	this->m_CustomNpcQuestMonsterInfo.clear();
	this->m_CustomNpcQuestRewardInfo.clear();
	this->m_CustomNpcQuestRewardItemInfo.clear();
	this->m_CustomNpcQuestRewardBuffInfo.clear();
	this->m_CustomNpcQuestRewardExpInfo.clear();
}

void CCustomNpcQuest::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->Init();

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
			gObj[n].CustomNpcQuest = -1;
			gObj[n].CustomNpcQuestMonsterIndex = -1;
			gObj[n].CustomNpcQuestMonsterQtd = -1;
			gObj[n].CustomNpcQuestFinished = -1;		
	}

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}
		
			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					QUESTNPC_TYPE_INFO info;

					info.Index = lpMemScript->GetNumber();

					info.MonsterClass = lpMemScript->GetAsNumber();

					info.Map = lpMemScript->GetAsNumber();

					info.X = lpMemScript->GetAsNumber();

					info.Y = lpMemScript->GetAsNumber();

					info.MaxQuest = lpMemScript->GetAsNumber();

					info.OpenNpc = lpMemScript->GetAsNumber();

					this->m_CustomNpcQuestNpcTypeInfo.push_back(info);
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_QUEST_CHARACTER info;

					info.Index = lpMemScript->GetNumber();

					info.Level = lpMemScript->GetAsNumber();

					info.Reset = lpMemScript->GetAsNumber();

					info.MReset = lpMemScript->GetAsNumber();

					info.Zen = lpMemScript->GetAsNumber();

					info.Coin1 = lpMemScript->GetAsNumber();

					info.Coin2 = lpMemScript->GetAsNumber();

					info.Coin3 = lpMemScript->GetAsNumber();

					info.VipLevel = lpMemScript->GetAsNumber();

					info.RankUser = lpMemScript->GetAsNumber();

					info.Kills = lpMemScript->GetAsNumber();

					this->m_CustomNpcQuestCharacterInfo.push_back(info);
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_QUEST_ITEM info;

					info.Index = lpMemScript->GetNumber();

					info.Category = lpMemScript->GetAsNumber();

					info.ItemIndex= lpMemScript->GetAsNumber();

					info.Level = lpMemScript->GetAsNumber();

					info.Luck = lpMemScript->GetAsNumber();

					info.Skill = lpMemScript->GetAsNumber();

					info.Quantity = lpMemScript->GetAsNumber();

					this->m_CustomNpcQuestItemInfo.push_back(info);
				}
				else if(section == 3)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_QUEST_MONSTER info;

					info.Index = lpMemScript->GetNumber();

					info.MonsterIndex = lpMemScript->GetAsNumber();

					info.Quantity = lpMemScript->GetAsNumber();

					this->m_CustomNpcQuestMonsterInfo.push_back(info);
				}
				else if(section == 4)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_QUEST_REWARD info;

					info.Index = lpMemScript->GetNumber();

					info.Value1 = lpMemScript->GetAsNumber();

					info.Value2 = lpMemScript->GetAsNumber();

					info.Value3 = lpMemScript->GetAsNumber();

					this->m_CustomNpcQuestRewardInfo.push_back(info);
				}
				else if(section == 5)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_QUEST_REWARD_ITEM info;

					info.Index = lpMemScript->GetNumber();

					info.ItemIndex = SafeGetItem(GET_ITEM(lpMemScript->GetAsNumber(),lpMemScript->GetAsNumber()));

					info.ItemLevel = lpMemScript->GetAsNumber();

					info.ItemDurability = lpMemScript->GetAsNumber();

					info.ItemOption1 = lpMemScript->GetAsNumber();

					info.ItemOption2 = lpMemScript->GetAsNumber();

					info.ItemOption3 = lpMemScript->GetAsNumber();

					info.ItemNewOption = lpMemScript->GetAsNumber();

					info.AncOption = lpMemScript->GetAsNumber();

					info.JOH	= lpMemScript->GetAsNumber();

					info.OpEx = lpMemScript->GetAsNumber();

					info.Socket1 = lpMemScript->GetAsNumber();

					info.Socket2 = lpMemScript->GetAsNumber();

					info.Socket3 = lpMemScript->GetAsNumber();

					info.Socket4 = lpMemScript->GetAsNumber();

					info.Socket5 = lpMemScript->GetAsNumber();

					info.Duration = lpMemScript->GetAsNumber();

					info.Class = lpMemScript->GetAsNumber();

					this->m_CustomNpcQuestRewardItemInfo.push_back(info);
				}
				else if(section == 6)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_QUEST_REWARD_BUFF info;

					info.Index = lpMemScript->GetNumber();

					info.EffectID = lpMemScript->GetAsNumber();

					info.Power1 = lpMemScript->GetAsNumber();

					info.Power2 = lpMemScript->GetAsNumber();

					info.Time = lpMemScript->GetAsNumber();

					this->m_CustomNpcQuestRewardBuffInfo.push_back(info);
				}
				else if(section == 7)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_QUEST_REWARD_EXP info;

					info.Index = lpMemScript->GetNumber();

					info.Experience = lpMemScript->GetAsNumber();

					this->m_CustomNpcQuestRewardExpInfo.push_back(info);
				}
				else
				{
					break;
				}
			}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool CCustomNpcQuest::CheckNpcQuest(LPOBJ lpObj,LPOBJ lpNpc) // OK
{
	#if (GAMESERVER_CLIENTE_UPDATE >= 13)

	if(gServerInfo.m_CustomNpcQuestSwitch == 0)
	{
		return 0;
	}

	for(std::vector<QUESTNPC_TYPE_INFO>::iterator it=this->m_CustomNpcQuestNpcTypeInfo.begin();it != this->m_CustomNpcQuestNpcTypeInfo.end();it++)
	{
		if(it->MonsterClass == lpNpc->Class && it->Map == lpNpc->Map && it->X == lpNpc->X && it->Y == lpNpc->Y)
		{
			if (lpObj->CustomNpcQuestFinished != -1 && lpObj->CustomNpcQuestFinished == it->Index)
			{
				return 0;
			}
			else
			{
				lpObj->CustomNpcQuestFinished = -1;
				this->DGCustomNpcQuestSend(lpObj->Index,it->Index,lpNpc->Index);
				return 1;
			}
		}
	}
#endif
	return 0;
}

bool CCustomNpcQuest::CheckNpcOpen(int quest) // OK
{
	for(std::vector<QUESTNPC_TYPE_INFO>::iterator it=this->m_CustomNpcQuestNpcTypeInfo.begin();it != this->m_CustomNpcQuestNpcTypeInfo.end();it++)
	{
		if(it->Index == quest)
		{
			if (it->OpenNpc != 0)
			{
				return 1;
			}
			else 
			{
				return 0;
			}
		}
	}
	return 0;
}

bool CCustomNpcQuest::CheckAll(int quest) // OK
{
	for(std::vector<CUSTOM_QUEST_CHARACTER>::iterator it=this->m_CustomNpcQuestCharacterInfo.begin();it != this->m_CustomNpcQuestCharacterInfo.end();it++)
	{
		if(it->Index == quest)
			return 1;
	}
	for(std::vector<CUSTOM_QUEST_ITEM>::iterator it=this->m_CustomNpcQuestItemInfo.begin();it != this->m_CustomNpcQuestItemInfo.end();it++)
	{
		if(it->Index == quest)
			return 1;
	}
	for(std::vector<CUSTOM_QUEST_MONSTER>::iterator it=this->m_CustomNpcQuestMonsterInfo.begin();it != this->m_CustomNpcQuestMonsterInfo.end();it++)
	{
		if(it->Index == quest)
			return 1;
	}
	return 0;
}

bool CCustomNpcQuest::CheckQuest(LPOBJ lpObj,int quest,int Npc, int qtd) // OK
{
	for(std::vector<QUESTNPC_TYPE_INFO>::iterator it=this->m_CustomNpcQuestNpcTypeInfo.begin();it != this->m_CustomNpcQuestNpcTypeInfo.end();it++)
	{
		if (it->Index <= 0)
		{
			continue;
		}

		if (it->Index != quest)
		{
			continue;
		}

		if (it->MaxQuest == 0)
		{
			return 1;
		}

		if (qtd >= it->MaxQuest)
		{
			return 0;
		}
	}
	return 1;
}

bool CCustomNpcQuest::CheckCharacter(LPOBJ lpObj,int quest,int Npc) // OK
{
	for(std::vector<CUSTOM_QUEST_CHARACTER>::iterator it=this->m_CustomNpcQuestCharacterInfo.begin();it != this->m_CustomNpcQuestCharacterInfo.end();it++)
	{

		if (it->Index <= 0)
		{
			continue;
		}

		if (it->Index != quest)
		{
			continue;
		}

		if (it->Level > 0 && lpObj->Level < it->Level)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(692),quest, it->Level);

			if (Npc >= 0)
			{
				GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(692),quest, it->Level);
			}
			return 0;
		}

		if (it->Reset > 0 && lpObj->Reset < it->Reset)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(693),quest, it->Reset);

			if (Npc >= 0)
			{
				GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(693),quest, it->Reset);
			}
			return 0;
		}

		if (it->MReset > 0 && lpObj->MasterReset < it->MReset)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(694),quest, it->MReset);

			if (Npc >= 0)
			{
				GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(694),quest, it->MReset);
			}
			return 0;
		}

		if (it->Zen > 0 && lpObj->Money < (DWORD)it->Zen)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(695),quest, it->Zen);

			if (Npc >= 0)
			{
				GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(695),quest, it->Zen);
			}
			return 0;
		}

		if (it->Coin1 > 0 && lpObj->Coin1 < it->Coin1)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(849),quest, it->Coin1);

			if (Npc >= 0)
			{
				GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(849),quest, it->Coin1);
			}
			return 0;
		}

		if (it->Coin2 > 0 && lpObj->Coin2 < it->Coin2)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(850),quest, it->Coin2);

			if (Npc >= 0)
			{
				GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(850),quest, it->Coin2);
			}
			return 0;
		}

		if (it->Coin3 > 0 && lpObj->Coin3 < it->Coin3)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(851),quest, it->Coin3);

			if (Npc >= 0)
			{
				GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(851),quest, it->Coin3);
			}
			return 0;
		}

		if (it->VipLevel >= 0 && lpObj->AccountLevel < it->VipLevel)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(697),quest);

			if (Npc >= 0)
			{
				GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(697),quest);
			}
			return 0;
		}

		if (it->RankUser >= 0 && gCustomRankUser.GetRankIndex(lpObj->Index) < it->RankUser)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(856),quest);

			if (Npc >= 0)
			{
				GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(856),quest);
			}
			return 0;
		}

		if (it->Kills > 0 && lpObj->Kills < it->Kills)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(857),quest, it->Kills);

			if (Npc >= 0)
			{
				GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(857),quest, it->Kills);
			}
			return 0;
		}
	}
	return 1;
}

bool CCustomNpcQuest::CheckItem(LPOBJ lpObj,int quest,int Npc) // OK
{
	int ItemOK;
	int Qtd;

	for(std::vector<CUSTOM_QUEST_ITEM>::iterator it=this->m_CustomNpcQuestItemInfo.begin();it != this->m_CustomNpcQuestItemInfo.end();it++)
	{
		if (it->Index <= 0)
		{
			continue;
		}

		if (it->Index != quest)
		{
			continue;
		}

		ItemOK	= 0;
		Qtd		= 0;

		if (it->Category >= 0 && it->ItemIndex >= 0)
		{
			for( int x = 12; x < INVENTORY_MAIN_SIZE; x++ )
			{
				if( lpObj->Inventory[x].IsItem() == TRUE )
				{
					if( lpObj->Inventory[x].m_Index == GET_ITEM(it->Category,it->ItemIndex))
					{
						ItemOK	= 0;
						if (it->Level >= 0)
						{
							if(lpObj->Inventory[x].m_Level == it->Level)
							{
								ItemOK++;
							}
						}
						else 
						{
							ItemOK++;
						}

						if (it->Luck >= 0)
						{
							if (lpObj->Inventory[x].m_Option2 == it->Luck)
							{
								ItemOK++;
							}
						}
						else 
						{
							ItemOK++;
						}

						if (it->Skill >= 0 )
						{
							if (lpObj->Inventory[x].m_Option1 == it->Skill)
							{
								ItemOK++;
							}
						}
						else 
						{
							ItemOK++;
						}

						if (ItemOK >= 3)
						{
							Qtd++;
						}
					}
				}
			}

			if (Qtd < it->Quantity)
			{
				char text[100]; 
				char level[10];
				wsprintf(level,"+%d", it->Level);

				wsprintf(text,"%d %s %s %s %s",it->Quantity,gItemManager.GetItemName(GET_ITEM(it->Category,it->ItemIndex)),(it->Level >= 0) ? level : "",(it->Luck >= 0) ? (it->Luck == 1) ? "+Luck" : "-No Luck" : "",(it->Skill >= 0) ? (it->Skill == 1) ? "+Skill" : "-No Skill" : "");
				gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(698),quest,text);

				if (Npc >= 0)
				{
					GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(698),quest,text);
				}
				return 0;
			}
		}

	}
	return 1;
}

bool CCustomNpcQuest::CheckMonster(LPOBJ lpObj,int quest,int Npc,int Count) // OK
{
	if (lpObj->CustomNpcQuest != -1 && quest != lpObj->CustomNpcQuest)
	{
		if (lpObj->CustomNpcQuestMonsterQtd >= 0)
		{
			this->DGCustomNpcQuestMonsterSaveSend(lpObj->Index,lpObj->CustomNpcQuest,lpObj->CustomNpcQuestMonsterQtd);
		}

		lpObj->CustomNpcQuestMonsterIndex	= -1;
		lpObj->CustomNpcQuestMonsterQtd		= -1;
	}

	lpObj->CustomNpcQuest = quest;

	for(std::vector<CUSTOM_QUEST_MONSTER>::iterator it=this->m_CustomNpcQuestMonsterInfo.begin();it != this->m_CustomNpcQuestMonsterInfo.end();it++)
	{
		if (it->Index <= 0)
		{
			continue;
		}

		if (it->Index != quest)
		{
			continue;
		}

		if (it->MonsterIndex >= 0 && lpObj->CustomNpcQuestMonsterQtd != 0)
		{
			if (lpObj->CustomNpcQuestMonsterIndex < 0)
			{
				lpObj->CustomNpcQuestMonsterIndex	= it->MonsterIndex;
				if (Count != 99999)
				{
					lpObj->CustomNpcQuestMonsterQtd		= Count;
				}
				else
				{
					lpObj->CustomNpcQuestMonsterQtd		= it->Quantity;
				}
			}

			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(699),quest, it->Quantity, gMonsterManager.GetMonsterName(it->MonsterIndex),lpObj->CustomNpcQuestMonsterQtd);

			if (Npc >= 0)
			{
				GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(699),quest, it->Quantity, gMonsterManager.GetMonsterName(it->MonsterIndex),lpObj->CustomNpcQuestMonsterQtd);
			}
			return 0;
		}

	}
	return 1;
}

bool CCustomNpcQuest::CheckItemRewardInventorySpace(LPOBJ lpObj,int quest,int Npc) // OK
{
	this->m_LastPosX = 0;
	this->m_LastPosY = 0;

	for(std::vector<CUSTOM_QUEST_REWARD_ITEM>::iterator it=this->m_CustomNpcQuestRewardItemInfo.begin();it != this->m_CustomNpcQuestRewardItemInfo.end();it++)
	{
		if (it->Index <= 0)
		{
			continue;
		}

		if (it->Index != quest)
		{
			continue;
		}

		if (it->Class != -1 && it->Class != lpObj->Class)
		{
			continue;
		}

		if (this->CheckItemInventorySpace(lpObj,it->ItemIndex) == 0)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(852),quest);

			if (Npc >= 0)
			{
				GCChatTargetNewSend(lpObj,Npc,gMessage.GetMessage(852),quest);
			}
			return 0;
		}
	}
	return 1;
}

bool CCustomNpcQuest::CheckItemInventorySpace(LPOBJ lpObj,int index) // OK
{
	ITEM_INFO ItemInfo;

	if(gItemManager.GetInfo(index,&ItemInfo) == 0)
	{
		return 0;
	}

	int MaxY = (gItemManager.GetInventoryMaxValue(lpObj)-INVENTORY_WEAR_SIZE)/8;

	for(int y=0;y < MaxY;y++)
	{
		for(int x=0;x < 8;x++)
		{
			if(lpObj->InventoryMap[((y*8)+x)] == 0xFF)
			{
				if(y < this->m_LastPosY && x < this->m_LastPosX)
				{
					continue;
				}
				if(gItemManager.InventoryRectCheck(lpObj->Index,x,y,ItemInfo.Width,ItemInfo.Height) != 0xFF)
				{
					this->m_LastPosX = (x+ItemInfo.Width);
					this->m_LastPosY = (y+ItemInfo.Height);
					return 1;
				}
			}
		}
	}

	return 0;
}

void CCustomNpcQuest::RemoveItem(LPOBJ lpObj,int quest) // OK
{
	int ItemOK;
	int QtdRemove;

	for(std::vector<CUSTOM_QUEST_ITEM>::iterator it=this->m_CustomNpcQuestItemInfo.begin();it != this->m_CustomNpcQuestItemInfo.end();it++)
	{
		if (it->Index <= 0)
		{
			continue;
		}

		if (it->Index != quest)
		{
			continue;
		}

		if (it->Quantity > 0)
		{
			if (it->Category >= 0 && it->ItemIndex >= 0)
			{
				QtdRemove	= 0;

				for( int x = 12; x < INVENTORY_MAIN_SIZE; x++ )
				{
					if( lpObj->Inventory[x].IsItem() == TRUE )
					{
						if( lpObj->Inventory[x].m_Index == GET_ITEM(it->Category,it->ItemIndex))
						{
							ItemOK	= 0;

							if (it->Level >= 0)
							{
								if(lpObj->Inventory[x].m_Level == it->Level)
								{
									ItemOK++;
								}
							}
							else 
							{
								ItemOK++;
							}

							if (it->Luck >= 0)
							{
								if (lpObj->Inventory[x].m_Option2 == it->Luck)
								{
									ItemOK++;
								}
							}
							else 
							{
								ItemOK++;
							}

							if (it->Skill >= 0 )
							{
								if (lpObj->Inventory[x].m_Option1 == it->Skill)
								{
									ItemOK++;
								}
							}
							else 
							{
								ItemOK++;
							}

							if (ItemOK >= 3)
							{
								gItemManager.InventoryDelItem(lpObj->Index,x);
								gItemManager.GCItemDeleteSend(lpObj->Index,x,1);

								QtdRemove++;

								if (QtdRemove >= it->Quantity)
								{
									break;
								}
							}
						}
					}
				}
			}
		}
	}
}

void CCustomNpcQuest::RemoveMoney(LPOBJ lpObj,int quest) // OK
{
	for(std::vector<CUSTOM_QUEST_CHARACTER>::iterator it=this->m_CustomNpcQuestCharacterInfo.begin();it != this->m_CustomNpcQuestCharacterInfo.end();it++)
	{
		if (it->Index <= 0)
		{
			continue;
		}

		if (it->Index != quest)
		{
			continue;
		}

		if (it->Zen > 0)
		{
			lpObj->Money -= (DWORD)it->Zen;

			GCMoneySend(lpObj->Index,lpObj->Money);
		}		

		if(it->Coin1 > 0 || it->Coin2 > 0 || it->Coin3 > 0 )
		{
			GDSetCoinSend(lpObj->Index, -(it->Coin1), -(it->Coin2), -(it->Coin3),"CustomQuest");
		}
	}
}

void CCustomNpcQuest::AddRewardCoin(LPOBJ lpObj,int quest) // OK
{
	for(std::vector<CUSTOM_QUEST_REWARD>::iterator it=this->m_CustomNpcQuestRewardInfo.begin();it != this->m_CustomNpcQuestRewardInfo.end();it++)
	{
		if (it->Index <= 0)
		{
			continue;
		}

		if (it->Index != quest)
		{
			continue;
		}

		GDSetCoinSend(lpObj->Index, (it->Value1 < 0)?0:it->Value1, (it->Value2 < 0)?0:it->Value2, (it->Value3 < 0)?0:it->Value3,"CustomQuest");
	}
}

void CCustomNpcQuest::AddRewardItem(LPOBJ lpObj,int quest) // OK
{
	for(std::vector<CUSTOM_QUEST_REWARD_ITEM>::iterator it=this->m_CustomNpcQuestRewardItemInfo.begin();it != this->m_CustomNpcQuestRewardItemInfo.end();it++)
	{
		if (it->Index <= 0)
		{
			continue;
		}

		if (it->Index != quest)
		{
			continue;
		}

		if (it->Class != -1 && it->Class != lpObj->Class)
		{
			continue;
		}

		BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

		if (gSocketItemType.CheckSocketItemType(it->ItemIndex) == 1)
		{
			int qtd = gSocketItemType.GetSocketItemMaxSocket(it->ItemIndex);

			ItemSocketOption[0] = (BYTE)((qtd > 0)?((it->Socket1 != 255)?it->Socket1:255):255);
			ItemSocketOption[1] = (BYTE)((qtd > 1)?((it->Socket2 != 255)?it->Socket2:255):255);
			ItemSocketOption[2] = (BYTE)((qtd > 2)?((it->Socket3 != 255)?it->Socket3:255):255);
			ItemSocketOption[3] = (BYTE)((qtd > 3)?((it->Socket4 != 255)?it->Socket4:255):255);
			ItemSocketOption[4] = (BYTE)((qtd > 4)?((it->Socket5 != 255)?it->Socket5:255):255);
			//this->m_Item[slot].m_SocketOptionBonus = gSocketItemOption.GetSocketItemBonusOption(this->m_Item);
		}

#if(GAMESERVER_UPDATE>=603)
		GDCreateItemSend(lpObj->Index,0xEB,0,0,it->ItemIndex,it->ItemLevel,0,it->ItemOption1,it->ItemOption2,it->ItemOption3,-1,it->ItemNewOption,it->AncOption,it->JOH,it->OpEx,ItemSocketOption,0xFF,((it->Duration>0)?((DWORD)time(0)+it->Duration):0));
#else
		GDCreateItemSend(lpObj->Index,0xEB,0,0,it->ItemIndex,it->ItemLevel,0,it->ItemOption1,it->ItemOption2,it->ItemOption3,-1,it->ItemNewOption,it->AncOption,it->JOH,it->OpEx,ItemSocketOption,0xFF,0);
#endif
	}
}

void CCustomNpcQuest::AddRewardBuff(LPOBJ lpObj,int quest) // OK
{
	for(std::vector<CUSTOM_QUEST_REWARD_BUFF>::iterator it=this->m_CustomNpcQuestRewardBuffInfo.begin();it != this->m_CustomNpcQuestRewardBuffInfo.end();it++)
	{
		if (it->Index <= 0)
		{
			continue;
		}

		if (it->Index != quest)
		{
			continue;
		}

		EFFECT_INFO* lpInfo = gEffectManager.GetInfo(it->EffectID);

		if(lpInfo == 0)
		{
			continue;
		}

		gEffectManager.AddEffect(lpObj,1,it->EffectID,(lpInfo->Type==2)?(int)(time(0)+(it->Time*60)):it->Time*60,it->Power1,it->Power2,0,0);

	}
}

void CCustomNpcQuest::AddRewardExperience(LPOBJ lpObj,int quest) // OK
{
	for(std::vector<CUSTOM_QUEST_REWARD_EXP>::iterator it=this->m_CustomNpcQuestRewardExpInfo.begin();it != this->m_CustomNpcQuestRewardExpInfo.end();it++)
	{
		if (it->Index <= 0)
		{
			continue;
		}

		if (it->Index != quest)
		{
			continue;
		}

		if(gEffectManager.CheckEffect(lpObj,EFFECT_SEAL_OF_SUSTENANCE1) != 0 || gEffectManager.CheckEffect(lpObj,EFFECT_SEAL_OF_SUSTENANCE2) != 0)
		{
			return;
		}

		if(gObjectManager.CharacterLevelUp(lpObj,it->Experience,gServerInfo.m_MaxLevelUp,EXPERIENCE_COMMON) != 0)
		{
			return;
		}

		GCRewardExperienceSend(lpObj->Index,it->Experience);
	}
}


void CCustomNpcQuest::DGCustomNpcQuestSend(int aIndex,int Quest, int NpcIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_CUSTOMNPCQUEST_SEND pMsg;

	pMsg.header.set(0xF7,0x00,sizeof(pMsg));

	pMsg.index = aIndex;

	pMsg.indexnpc = NpcIndex;

	pMsg.quest = Quest;

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg)); 
}

void CCustomNpcQuest::DGCustomNpcQuestRecv(SDHP_CUSTOMNPCQUEST_RECV* lpMsg) // OK
{
#if GAMESERVER_CLIENTE_UPDATE >= 13
	
	if(gServerInfo.m_CustomNpcQuestSwitch == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(gObjIsConnectedGP(lpMsg->index) == 0)
	{
		return;
	}

	if(lpObj->Interface.use != 0)
	{
		return;
	}

	if(lpObj->TradeDuel != 0)
	{
		return;
	}

	int quest = lpMsg->quest;

	if (this->CheckQuest(lpObj,quest,lpMsg->indexnpc,lpMsg->questcount) == 0)
	{
		if (this->CheckNpcOpen(quest) != 1)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(860));

			if (lpMsg->indexnpc >= 0)
			{
				GCChatTargetNewSend(lpObj,lpMsg->indexnpc,gMessage.GetMessage(860));
			}
		}
		else 
		{
			lpObj->CustomNpcQuestFinished = quest;

			PMSG_NPC_TALK_RECV pMsg;

			pMsg.header.set(0x30,sizeof(pMsg));

			pMsg.index[0] = SET_NUMBERHB(lpMsg->indexnpc); 

			pMsg.index[1] = SET_NUMBERLB(lpMsg->indexnpc);

			gNpcTalk.CGNpcTalkRecv(&pMsg,lpObj->Index);
		}
		return;
	}

	if(this->CheckAll(quest) == 0)
	{
		return;
	}

	if (this->CheckCharacter(lpObj,quest,lpMsg->indexnpc) == 0)
	{
		return;
	}

	if (this->CheckItem(lpObj,quest,lpMsg->indexnpc) == 0)
	{
		return;
	}

	if (lpMsg->monstercount != 0)
	{
		if (this->CheckMonster(lpObj,quest,lpMsg->indexnpc,lpMsg->monstercount) == 0)
		{
			return;
		}
	}

	if(this->CheckItemRewardInventorySpace(lpObj,quest,lpMsg->indexnpc) == 0)
	{
		return;
	}

	lpObj->CustomNpcQuest = -1;
	lpObj->CustomNpcQuestMonsterIndex	= -1;
	lpObj->CustomNpcQuestMonsterQtd	= -1;

	SDHP_CUSTOMNPCQUEST_SAVE_SEND pMsg;

	pMsg.header.set(0xF7,0x01,sizeof(pMsg));

	pMsg.index = lpObj->Index;

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.quest = quest;

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg)); 

	this->RemoveItem(lpObj,quest);
	this->RemoveMoney(lpObj,quest);
	this->AddRewardCoin(lpObj,quest);
	this->AddRewardItem(lpObj,quest);
	this->AddRewardBuff(lpObj,quest);
	this->AddRewardExperience(lpObj,quest);

	//Log
	LogAdd(LOG_EVENT,"[CustomNpcQuest][%s][%s] - (NpcQuest %d: Completed)",lpObj->Account,lpObj->Name,lpObj->CustomQuest);

	GCFireworksSend(lpObj,lpObj->X,lpObj->Y);
#endif
}

void CCustomNpcQuest::DGCustomNpcQuestMonsterSaveSend(int aIndex,int Quest, int Count) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_CUSTOMNPCQUESTMONSTERSAVE_SEND pMsg;

	pMsg.header.set(0xF7,0x02,sizeof(pMsg));

	pMsg.index = aIndex;

	pMsg.monsterqtd = Count;

	pMsg.quest = Quest;

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg)); 
}