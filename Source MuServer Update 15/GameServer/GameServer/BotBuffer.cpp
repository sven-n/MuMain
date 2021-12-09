#include "stdafx.h"
#include "BotBuffer.h"
#include "Monster.h"
#include "Protocol.h"
#include "Util.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "User.h"
#include "ItemManager.h"
#include "Map.h"
#include "ScheduleManager.h"
#include "ObjectManager.h"
#include "DSProtocol.h"
#include "SkillManager.h"
#include "MuunSystem.h"
#include "GensSystem.h"
#include "PcPoint.h"

#if (BOT_BUFFER == 1)

ObjBotBuffer ObjBotBuff;

CMemScript objScript;

void ObjBotBuffer::Read(char * FilePath)
{
	if(this->Enabled == 1)
	{
		for(int botNum=0;botNum<MAX_BOTBUFFER;botNum++)
		{
			if(this->bot[botNum].Enabled == 1)
			{
				int bIndex = this->bot[botNum].index;
				if(gObjIsConnected(bIndex) == 1)
				{
					gObjDel(bIndex);
				}
			}
		}
	}
	for(int i=0;i<MAX_BOTBUFFER;i++)
	{
		this->bot[i].index = -1;
		this->bot[i].SkillCount = 0;
		this->bot[i].MaxLevel = ObjBotBuff.Max_Normal_Level;
		memset(&this->bot[i].skill,0,sizeof(this->bot[i].skill));
		this->bot[i].Enabled = 0;
		for(int j=0;j<9;j++)
			this->bot[i].body[j].num =-1;
	}

		this->Enabled = 0;

	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,FilePath);
		return;
	}

	if(lpMemScript->SetBuffer(FilePath) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
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
				if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

				int BotNum = lpMemScript->GetNumber();
				if(BotNum < 0 || BotNum > MAX_BOTBUFFER-1)
				{
					ErrorMessageBox("BotBuffer error 1: BotBufferIndex: %d out of range!", BotNum);
					return;
				}

				this->bot[BotNum].Class = lpMemScript->GetAsNumber();
				this->bot[BotNum].Enabled = lpMemScript->GetAsNumber();
				this->bot[BotNum].ChangeColorName = lpMemScript->GetAsNumber();
				this->bot[BotNum].Zen = lpMemScript->GetAsNumber();
				this->bot[BotNum].PCPoints = lpMemScript->GetAsNumber();
				this->bot[BotNum].MaxLevel = lpMemScript->GetAsNumber();
				strncpy_s(this->bot[BotNum].Name,lpMemScript->GetAsString(),sizeof(this->bot[BotNum].Name));
				this->bot[BotNum].Map = lpMemScript->GetAsNumber();
				this->bot[BotNum].X = lpMemScript->GetAsNumber();
				this->bot[BotNum].Y = lpMemScript->GetAsNumber();
				this->bot[BotNum].Dir = lpMemScript->GetAsNumber();

				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

				int BotNum = lpMemScript->GetNumber();
				if(BotNum < 0 || BotNum > MAX_BOTBUFFER-1)
				{
					ErrorMessageBox("BotBuffer error 2: BotBufferIndex: %d out of range!", BotNum);
					return;
				}

				this->bot[BotNum].OnlyVip = lpMemScript->GetAsNumber();
				this->bot[BotNum].ActiveGensFamily = lpMemScript->GetAsNumber();
				this->bot[BotNum].ContributionGens = lpMemScript->GetAsNumber();
				this->bot[BotNum].GensFamily = lpMemScript->GetAsNumber();

				}

				else if(section == 3)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

				int BotNum = lpMemScript->GetNumber();
				if(BotNum < 0 || BotNum > MAX_BOTBUFFER-1)
				{
					LogAdd(LOG_RED,"BotBuffer error 3: BotBufferIndex: %d doesnt exist", BotNum);
					return;
				}

				int Slot = lpMemScript->GetAsNumber();
				
				if(Slot < 0 || Slot > 8)
				{	
					ErrorMessageBox("BotBuffer error: Min Slot 0 ; Max Slot 8");
					return;
				}

				int iType = lpMemScript->GetAsNumber();
				int iIndex = lpMemScript->GetAsNumber();

				this->bot[BotNum].body[Slot].num = GET_ITEM(iType,iIndex);
				this->bot[BotNum].body[Slot].level = lpMemScript->GetAsNumber();
				this->bot[BotNum].body[Slot].opt = lpMemScript->GetAsNumber();
				this->bot[BotNum].body[Slot].Enabled = 1;

				}

				else if(section == 4)
				{
					this->Enabled = 1;
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

				int BotNum = lpMemScript->GetNumber();
				if(BotNum < 0 || BotNum > MAX_BOTBUFFER-1)
				{
					LogAdd(LOG_RED,"BotBuffer error 4: BotBufferIndex: %d doesnt exist", BotNum);
					return;
				}

				if(this->bot[BotNum].SkillCount < 0 || this->bot[BotNum].SkillCount > MAX_BOTBUFFERSKILLS-1)
				{
					LogAdd(LOG_RED,"BotBuffer error: Skill Buff out of range (MAX BUFF PER BOT = %d)!", this->bot[BotNum].SkillCount);
					return;
				}

				this->bot[BotNum].skill[this->bot[BotNum].SkillCount].skill = lpMemScript->GetAsNumber();
				this->bot[BotNum].skill[this->bot[BotNum].SkillCount].time = lpMemScript->GetAsNumber();
				this->bot[BotNum].skill[this->bot[BotNum].SkillCount].power = lpMemScript->GetAsNumber();
				this->bot[BotNum].SkillCount++;

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

int ObjBotBuffer::GetBotIndex(int aIndex)
{
	for(int i=0; i<MAX_BOTBUFFER; i++)
	{
		if(this->bot[i].Enabled == 1)
		{
			if(this->bot[i].index == aIndex)
				return i;
		}
	}
	return -1;
}

void ObjBotBuffer::MakeBot()
{

	if(this->Enabled == 1)
	{
	
		for(int botNum=0; botNum<MAX_BOTBUFFER; botNum++)
		{
			if(this->bot[botNum].Enabled == 1)	
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
					gObj[result].Live = 1;	
					gObj[result].PathCount = 0; 
					gObj[result].IsBot = 2;
					gObj[result].BotPower = 0;

					gObjSetBots(result,this->bot[botNum].Class);

					gObj[result].ChangeUp = this->bot[botNum].Class;
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

					#if (GAMESERVER_UPDATE >= 801)
						gObj[result].MuunInventory = new CItem[MUUN_INVENTORY_SIZE];
						gObj[result].MuunInventoryMap = new BYTE[MUUN_INVENTORY_SIZE];

						for (int i=0;i<MUUN_INVENTORY_SIZE;i++)
						{
							gObj[result].MuunInventory[i].Clear();
						}

						memset(&gObj[result].MuunInventoryMap[0], (BYTE)-1, MUUN_INVENTORY_SIZE); //novo
					#endif
					
					strcpy_s(gObj[result].Name,this->bot[botNum].Name);
					
					for(int i=0;i<9;i++)
					{
						if(this->bot[botNum].body[i].num >= 0 && this->bot[botNum].body[i].Enabled == 1)
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
					

					GCSummonLifeSend(gObj[result].SummonIndex,(int)gObj[result].MaxLife,(int)gObj[result].Life);
					
					GCActionSend(&gObj[result],ACTION_SALUTE1,result,result);
					
				}
			}
		}
	}
}


BYTE ObjBotBuffer::gObjInventoryInsertItemPos(int aIndex, CItem item, int pos, BOOL RequestCheck) //OK
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

	int useClass = 0;

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
			useClass = item.IsClassBot((char)lpObj->Class,lpObj->ChangeUp);

			if(useClass == 0)
			{
				LogAdd(LOG_BLUE,"[Using Class Error] Error UseClass %d",useClass);
				return -1;
			}

		}
	}

	lpObj->Inventory[pos] = item;

	return pos;
}

BOOL ObjBotBuffer::TradeOpen(int index, int nindex)
{
	if ( gObjIsConnectedGP(index) == 0 )
	{
		LogAdd(LOG_BLUE, "error-L3 [%s][%d]", __FILE__, __LINE__);
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

	if(lpObj->Level > this->bot[number].MaxLevel)
	{
		gNotice.NewMessageDevTeam(lpObj->Index,gMessage.GetMessage(521),this->bot[number].Name);	
		return 1;
	}
	if(this->bot[number].OnlyVip != 0 && lpObj->AccountLevel == 0)
	{
		gNotice.NewMessageDevTeam(gObj[nindex].Index,gMessage.GetMessage(522),this->bot[number].Name);
		return 1;
	}
	if((DWORD)this->bot[number].Zen > gObj[index].Money)
	{
		gNotice.NewMessageDevTeam(gObj[index].Index,gMessage.GetMessage(523),this->bot[number].Name,this->bot[number].Zen);
		return 1;		
	}
	if(this->bot[number].PCPoints > gObj[index].PCPoint)
	{
		gNotice.NewMessageDevTeam(gObj[index].Index,gMessage.GetMessage(524),this->bot[number].Name,this->bot[number].PCPoints);
		return 1;		
	}
	if(this->bot[number].ActiveGensFamily == 1)
	{
		if(this->bot[number].GensFamily != GENS_FAMILY_VARNERT && gObj[index].GensFamily == GENS_FAMILY_NONE )
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,gMessage.GetMessage(525),this->bot[number].Name);
			return 1;
		}
		if (this->bot[number].GensFamily != GENS_FAMILY_DUPRIAN && gObj[index].GensFamily == GENS_FAMILY_NONE)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,gMessage.GetMessage(526),this->bot[number].Name);
			return 1;
		}
		if (this->bot[number].GensFamily == GENS_FAMILY_VARNERT && gObj[index].GensFamily == GENS_FAMILY_DUPRIAN)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,gMessage.GetMessage(527),this->bot[number].Name);
			return 1;
		}
		if (this->bot[number].GensFamily == GENS_FAMILY_DUPRIAN && gObj[index].GensFamily == GENS_FAMILY_VARNERT)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,gMessage.GetMessage(528),this->bot[number].Name);
			return 1;
		}
		if(this->bot[number].ContributionGens > gObj[index].GensContribution)
		{
			gNotice.NewMessageDevTeam(gObj[index].Index,gMessage.GetMessage(529),this->bot[number].Name,this->bot[number].ContributionGens);
			//LogAdd(LOG_RED,"[%s] Account: %s - Insuficent GensContribution : %d",this->bot[number].Name,gObj[index].Account,this->bot[number].ContributionGens);
			return 1;
		}
	}

	for(int x = 0; x < this->bot[number].SkillCount; x ++)
	{
		switch(this->bot[number].skill[x].skill)
		{
		case SKILL_MANA_SHIELD:
			{	
				if(lpObj->Class == CLASS_DW )
				{
					GCActionSend(lpBot,ACTION_RESPECT1,lpBot->Index,nindex);
					
					gSkillManager.GCSkillAttackSend(lpObj,SKILL_GREATER_MANA,index,1);
				}
				else{gSkillManager.GCSkillAttackSend(lpBot,SKILL_GREATER_MANA,index,1);}

				GCManaSend(index,0xFF,(int)lpObj->Mana,lpObj->BP);
				
				gEffectManager.AddEffect(lpObj,1,EFFECT_MANA_SHIELD,this->bot[number].skill[x].time*60,this->bot[number].skill[x].power,0,0,0);
				
				LogAdd(LOG_BLACK,"[BotBuffer] CharName: %s - ADD_SKILL_MANA SecondSkillTime: %d",lpObj->Name,this->bot[number].skill[x].time);
			}
			break;
		case SKILL_GREATER_LIFE:
			{	
				if(lpObj->Class == CLASS_DK )
				{
					GCActionSend(lpBot,ACTION_SALUTE1,lpBot->Index,nindex);
					
					gSkillManager.GCSkillAttackSend(lpObj,SKILL_GREATER_LIFE,index,1);
				}
				else{gSkillManager.GCSkillAttackSend(lpBot,SKILL_GREATER_LIFE,index,1);}
				
				GCLifeSend(lpObj->Index,0xFF,(int)lpObj->Life,lpObj->Shield);	
				
				gEffectManager.AddEffect(lpObj,1,EFFECT_GREATER_LIFE,this->bot[number].skill[x].time*60,this->bot[number].skill[x].power,0,0,0);
				
				LogAdd(LOG_BLACK,"[BotBuffer] CharName: %s - ADD_GREATER_LIFE SecondSkillTime: %d",lpObj->Name,this->bot[number].skill[x].time);
			}	
			break;
		case SKILL_GREATER_DEFENSE:
			{
				if(lpObj->Class == CLASS_FE )
				{
					GCActionSend(lpBot,ACTION_RESPECT1,lpBot->Index,nindex);
					
					gSkillManager.GCSkillAttackSend(lpObj,SKILL_GREATER_DEFENSE,index,1);

				}
				else{gSkillManager.GCSkillAttackSend(lpBot,SKILL_GREATER_DEFENSE,index,1);}
				
				gObj[index].Defense = gObj[index].Level / 3 + 50;
				
				gEffectManager.AddEffect(lpObj,1,EFFECT_GREATER_DEFENSE,this->bot[number].skill[x].time*60,this->bot[number].skill[x].power,0,0,0);
				
				LogAdd(LOG_BLACK,"[BotBuffer] CharName: %s -ADD_GREATER_DEFENSE SecondSkillTime: %d",lpObj->Name,this->bot[number].skill[x].time);
			}
			break;
		case SKILL_GREATER_DAMAGE:
			{
				if(lpObj->Class == CLASS_FE )
				{
					GCActionSend(lpBot,ACTION_RESPECT1,lpBot->Index,nindex);
					
					gSkillManager.GCSkillAttackSend(lpObj,SKILL_GREATER_DAMAGE,index,1);
				}
				else{gSkillManager.GCSkillAttackSend(lpBot,SKILL_GREATER_DAMAGE,index,1);}
					
				gObj[index].Defense = gObj[index].Level / 3 + 45;
				
				gEffectManager.AddEffect(lpObj,1,EFFECT_GREATER_DAMAGE,this->bot[number].skill[x].time*60,this->bot[number].skill[x].power,0,0,0);
				
				LogAdd(LOG_BLACK,"[BotBuffer] CharName: %s - ADD_GREATER_DAMAGE SecondSkillTime: %d",lpObj->Name,this->bot[number].skill[x].time);
			}
			break;
		case SKILL_GREATER_CRITICAL_DAMAGE:
			{
				if(lpObj->Class == CLASS_DL )
				{
					GCActionSend(lpBot,ACTION_RESPECT1,lpBot->Index,nindex);
					
					gSkillManager.GCSkillAttackSend(lpObj,SKILL_GREATER_CRITICAL_DAMAGE,index,1);
				}
				else{gSkillManager.GCSkillAttackSend(lpBot,SKILL_GREATER_CRITICAL_DAMAGE,index,1);}
				
				gObj[index].CriticalDamageRate = gObj[index].Level / 3 + 45;	
				
				gEffectManager.AddEffect(lpObj,1,EFFECT_GREATER_CRITICAL_DAMAGE,this->bot[number].skill[x].time*60,this->bot[number].skill[x].power,0,0,0);				
				
				LogAdd(LOG_BLACK,"[BotBuffer] CharName: %s - ADD_CRITICAL_DAMAGE SecondSkillTime: %d",lpObj->Name,this->bot[number].skill[x].time);
			}
			break;
		case SKILL_DAMAGE_REFLECT:
			{
				if(lpObj->Class == CLASS_SU )
				{
					GCActionSend(lpBot,ACTION_RESPECT1,lpBot->Index,nindex);
					gSkillManager.GCSkillAttackSend(lpObj,SKILL_DAMAGE_REFLECT,index,1);
					
				}
				else{gSkillManager.GCSkillAttackSend(lpBot,SKILL_DAMAGE_REFLECT,index,1);}
				
				gObj[index].DamageReflect = gObj[index].Level / 3 + 45;
				
				gEffectManager.AddEffect(lpObj,1,EFFECT_DAMAGE_REFLECT,this->bot[number].skill[x].time*60,this->bot[number].skill[x].power,0,0,0);
				
				LogAdd(LOG_BLACK,"[BotBuffer] CharName: %s - ADD_DAMAGE_REFLECT SecondSkillTime: %d",lpObj->Name,this->bot[number].skill[x].time);
			}
			break;
		case EFFECT_MAGIC_CIRCLE:
			{
				GCActionSend(lpBot,ACTION_RESPECT1,lpBot->Index,nindex);

				gEffectManager.AddEffect(lpObj,0,EFFECT_MAGIC_CIRCLE,this->bot[number].skill[x].time*60,this->bot[number].skill[x].power,0,0,0);
				
				LogAdd(LOG_BLACK,"[BotBuffer] CharName: %s -ADD_MAGIC_CIRCLE SecondSkillTime: %d",lpObj->Name,this->bot[number].skill[x].time);
			}
			break;
		case EFFECT_GREATER_IGNORE_DEFENSE_RATE:
			{
				GCActionSend(lpBot,ACTION_RAGEBUFF_1,lpBot->Index,nindex);
				
				gEffectManager.AddEffect(lpObj,1,EFFECT_GREATER_IGNORE_DEFENSE_RATE,this->bot[number].skill[x].time*60,this->bot[number].skill[x].power,0,0,0);
				
				LogAdd(LOG_BLACK,"[BotBuffer] CharName: %s - ADD_IGNORE_DEFENSE_RATE SecondSkillTime: %d",lpObj->Name,this->bot[number].skill[x].time);
			}
			break;
		case EFFECT_FITNESS:
			{
				GCActionSend(lpBot,ACTION_RAGEBUFF_1,lpBot->Index,nindex);

				gEffectManager.AddEffect(lpObj,1,EFFECT_FITNESS,this->bot[number].skill[x].time*60,this->bot[number].skill[x].power,0,0,0);
				
				LogAdd(LOG_BLACK,"[BotBuffer] CharName: %s - ADD_FITNESS SecondSkillTime: %d",lpObj->Name,this->bot[number].skill[x].time);
			}
			break;
		}

		gNotice.NewNoticeSend(lpObj->Index,0,0,0,0,0,gMessage.GetMessage(530),this->bot[number].Name,this->bot[number].skill[x].time);
	}
		
	if(this->bot[number].Zen > 0)
	{
		gObj[index].Money -= this->bot[number].Zen;
	    GCMoneySend(lpObj->Index,lpObj->Money);
		gNotice.NewNoticeSend(gObj[index].Index,0,0,0,0,0,gMessage.GetMessage(531),this->bot[number].Name,this->bot[number].Zen);
	}
	
	if(this->bot[number].PCPoints > 0)
	{
		gPcPoint.GDPcPointSubPointSaveSend(lpObj->Index,this->bot[number].PCPoints);
		gPcPoint.GDPcPointPointSend(lpObj->Index);
		gNotice.NewNoticeSend(gObj[index].Index,0,0,0,0,0,gMessage.GetMessage(532),this->bot[number].Name,gObj[index].PCPoint);
	}

	gNotice.NewMessageDevTeam(lpObj->Index,gMessage.GetMessage(533),this->bot[number].Name);	
	return 1;
}

#endif
