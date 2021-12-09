// CustomAttack.cpp: implementation of the CCustomAttack class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomAttack.h"
#include "CommandManager.h"
#include "EffectManager.h"
#include "GameMain.h"
#include "ItemManager.h"
#include "Util.h"
#include "Map.h"
#include "MapManager.h"
#include "MasterSkillTree.h"
#include "Message.h"
#include "Notice.h"
#include "Party.h"
#include "ServerInfo.h"
#include "SkillManager.h"
#include "SocketManager.h"
#include "Viewport.h"

CCustomAttack gCustomAttack;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomAttack::CCustomAttack() // OK
{

}

CCustomAttack::~CCustomAttack() // OK
{

}

void CCustomAttack::ReadCustomAttackInfo(char* section,char* path) // OK
{
	this->m_CustomAttackTime[0] = GetPrivateProfileInt(section,"CustomAttackTime_AL0",0,path);
	this->m_CustomAttackTime[1] = GetPrivateProfileInt(section,"CustomAttackTime_AL1",0,path);
	this->m_CustomAttackTime[2] = GetPrivateProfileInt(section,"CustomAttackTime_AL2",0,path);
	this->m_CustomAttackTime[3] = GetPrivateProfileInt(section,"CustomAttackTime_AL3",0,path);

	this->m_CustomAttackDelay = GetPrivateProfileInt(section,"CustomAttackDelay",0,path);

	this->m_CustomAttackPotionDelay = GetPrivateProfileInt(section,"CustomAttackAutoPotionDelay",0,path);

	this->m_CustomAttackAutoBuff[0] = GetPrivateProfileInt(section,"CustomAttackAutoBuff_AL0",0,path);
	this->m_CustomAttackAutoBuff[1] = GetPrivateProfileInt(section,"CustomAttackAutoBuff_AL1",0,path);
	this->m_CustomAttackAutoBuff[2] = GetPrivateProfileInt(section,"CustomAttackAutoBuff_AL2",0,path);
	this->m_CustomAttackAutoBuff[3] = GetPrivateProfileInt(section,"CustomAttackAutoBuff_AL3",0,path);

	this->m_CustomAttackAutoBuffDelay = GetPrivateProfileInt(section,"CustomAttackAutoBuffDelay",0,path);

	this->m_CustomAttackAutoResume[0] = GetPrivateProfileInt(section,"CustomAttackAutoResume_AL0",0,path);
	this->m_CustomAttackAutoResume[1] = GetPrivateProfileInt(section,"CustomAttackAutoResume_AL1",0,path);
	this->m_CustomAttackAutoResume[2] = GetPrivateProfileInt(section,"CustomAttackAutoResume_AL2",0,path);
	this->m_CustomAttackAutoResume[3] = GetPrivateProfileInt(section,"CustomAttackAutoResume_AL3",0,path);
}

bool CCustomAttack::CommandCustomAttack(LPOBJ lpObj,char* arg) // OK
{
	if(lpObj->AttackCustom != 0)
	{
		this->OnAttackClose(lpObj);
		return 0;
	}

	if(lpObj->TradeDuel != 0)
	{
		return 0;
	}

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(791));
		return 0;
	}

	if (gMapManager.GetMapCustomAttack(lpObj->Map) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(36));
		return 0;
	}

	//if(CA_MAP_RANGE(lpObj->Map) != 0 || DS_MAP_RANGE(lpObj->Map) != 0 || BC_MAP_RANGE(lpObj->Map) != 0 || CC_MAP_RANGE(lpObj->Map) != 0 || IT_MAP_RANGE(lpObj->Map) != 0 || DA_MAP_RANGE(lpObj->Map) != 0 || DG_MAP_RANGE(lpObj->Map) != 0 || IG_MAP_RANGE(lpObj->Map) != 0)
	//{
	//	gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,this->m_CustomAttackText5);
	//	return;
	//}

	//Fix helper + custom attack
	if (lpObj->HelperTotalTime != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(763));
		return 0;
	}

	int SetSkill = gCommandManager.GetNumber(arg,0);

	int SetBuff = gCommandManager.GetNumber(arg,1);

	SetSkill = (SetSkill>0)?SetSkill:SKILL_NONE;

	int SkillNumber = SKILL_NONE;

	if(this->GetAttackSkill(lpObj,&SkillNumber,SetSkill) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(792));
		return 0;
	}

	if (lpObj->DBClass == DB_CLASS_HE && gSkillManager.GetSkill(lpObj,SKILL_INFINITY_ARROW) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(792));
		return 0;
	}

	if (lpObj->DBClass == DB_CLASS_HE)
	{
		CSkill* lpSkillElf = gSkillManager.GetSkill(lpObj,SKILL_INFINITY_ARROW);
		int SkillElf = lpSkillElf->m_index;
		this->SendSkillAttack(lpObj,lpObj->Index,SkillElf);
	}

	if(SetBuff == 1 && this->m_CustomAttackAutoBuff[lpObj->AccountLevel] == 1)
	{
		lpObj->AttackCustomAutoBuff = 1;
		lpObj->AttackCustomAutoBuffDelay = GetTickCount()+(DWORD)this->m_CustomAttackAutoBuffDelay;
	}

	lpObj->AttackCustom = 1;

	lpObj->AttackCustomSkill = SkillNumber;

	lpObj->AttackCustomDelay = GetTickCount();

	lpObj->AttackCustomZoneX = lpObj->X;

	lpObj->AttackCustomZoneY = lpObj->Y;

	lpObj->AttackCustomZoneMap = lpObj->Map;

	this->DGCustomAttackResumeSaveSend(lpObj->Index);

	lpObj->AttackCustomOffline = 0;

	lpObj->AttackCustomOfflineTime = 0;

	//lpObj->AttackCustomOfflineMoneyDelay = 0;

	lpObj->AttackCustomTime = this->m_CustomAttackTime[lpObj->AccountLevel]*600;

	gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(795));

	if (this->m_CustomAttackTime[lpObj->AccountLevel] > 0) 
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(793),this->m_CustomAttackTime[lpObj->AccountLevel]);
	}
	return 1;
}

bool CCustomAttack::CommandCustomAttackOffline(LPOBJ lpObj,char* arg) // OK
{
	if(lpObj->AttackCustom == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(796));
		return 0;
	}

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(797));
		return 0;
	}

	if(CA_MAP_RANGE(lpObj->Map) != 0 || DS_MAP_RANGE(lpObj->Map) != 0 || BC_MAP_RANGE(lpObj->Map) != 0 || CC_MAP_RANGE(lpObj->Map) != 0 || IT_MAP_RANGE(lpObj->Map) != 0 || DA_MAP_RANGE(lpObj->Map) != 0 || DG_MAP_RANGE(lpObj->Map) != 0 || IG_MAP_RANGE(lpObj->Map) != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(797));
		return 0;
	}

	lpObj->Socket = INVALID_SOCKET;

	lpObj->AttackCustomOffline = 1;

	lpObj->AttackCustomOfflineTime = 0;

	#if(NEW_PROTOCOL_SYSTEM==0)
	closesocket(lpObj->PerSocketContext->Socket);
	#endif

	return 1;
}

bool CCustomAttack::GetAttackSkill(LPOBJ lpObj,int* SkillNumber,int SetSkill) // OK
{
	CSkill* lpSkill = 0;

	lpSkill = gSkillManager.GetSkill(lpObj,SetSkill);

	switch(lpObj->Class)
	{
		case CLASS_DW:
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_ICE_STORM):lpSkill);
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_EVIL_SPIRIT):lpSkill);
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_INFERNO):lpSkill);
			break;
		case CLASS_DK:
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_TWISTING_SLASH):lpSkill);
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_DEATH_STAB):lpSkill);
			break;
		case CLASS_FE:
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_FIVE_SHOT):lpSkill);
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_TRIPLE_SHOT):lpSkill);
			break;
		case CLASS_MG:
			if(lpObj->Inventory[0].m_Index >= GET_ITEM(5,0) && lpObj->Inventory[0].m_Index < GET_ITEM(6,0))
			{
				lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_LIGHTNING_STORM):lpSkill);
				lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_EVIL_SPIRIT):lpSkill);
			}
			else
			{
				lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_SWORD_SLASH):lpSkill);
				lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_POWER_SLASH):lpSkill);
			}
			break;
		case CLASS_DL:
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_FIRE_SCREAM):lpSkill);
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_BIRDS):lpSkill);
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_FIRE_BURST):lpSkill);
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_FORCE):lpSkill);
			break;
		case CLASS_SU:
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_RED_STORM):lpSkill);
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_CHAIN_LIGHTNING):lpSkill);
			break;
		case CLASS_RF:
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_DARK_SIDE):lpSkill);
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_DRAGON_LORE):lpSkill);
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_PHOENIX_SHOT):lpSkill);
			lpSkill = ((lpSkill==0)?gSkillManager.GetSkill(lpObj,SKILL_LARGE_RING_BLOWER):lpSkill);
			break;
	}

	if(lpSkill == 0)
	{
		return 0;
	}
	else
	{
		(*SkillNumber) = lpSkill->m_index;
		return 1;
	}
}

bool CCustomAttack::GetTargetMonster(LPOBJ lpObj,int SkillNumber,int* MonsterIndex) // OK
{
	int NearestDistance = 100;

	for(int n=0;n < MAX_VIEWPORT;n++)
	{
		if(lpObj->VpPlayer2[n].state == VIEWPORT_NONE || OBJECT_RANGE(lpObj->VpPlayer2[n].index) == 0 || lpObj->VpPlayer2[n].type != OBJECT_MONSTER)
		{
			continue;
		}

		if(gSkillManager.CheckSkillTarget(lpObj,lpObj->VpPlayer2[n].index,-1,lpObj->VpPlayer2[n].type) == 0)
		{
			continue;
		}

		if(gObjCalcDistance(lpObj,&gObj[lpObj->VpPlayer2[n].index]) >= NearestDistance)
		{
			continue;
		}

		if(gSkillManager.CheckSkillRange(SkillNumber,lpObj->X,lpObj->Y,gObj[lpObj->VpPlayer2[n].index].X,gObj[lpObj->VpPlayer2[n].index].Y) != 0)
		{
			(*MonsterIndex) = lpObj->VpPlayer2[n].index;
			NearestDistance = gObjCalcDistance(lpObj,&gObj[lpObj->VpPlayer2[n].index]);
			continue;
		}

		if(gSkillManager.CheckSkillRadio(SkillNumber,lpObj->X,lpObj->Y,gObj[lpObj->VpPlayer2[n].index].X,gObj[lpObj->VpPlayer2[n].index].Y) != 0)
		{
			(*MonsterIndex) = lpObj->VpPlayer2[n].index;
			NearestDistance = gObjCalcDistance(lpObj,&gObj[lpObj->VpPlayer2[n].index]);
			continue;
		}
	}

	return ((NearestDistance==100)?0:1);
}

void CCustomAttack::OnAttackClose(LPOBJ lpObj) // OK
{
	if(lpObj->AttackCustom != 0)
	{
		lpObj->AttackCustom = 0;
		lpObj->AttackCustomSkill = 0;
		lpObj->AttackCustomDelay = 0;
		lpObj->AttackCustomZoneX = 0;
		lpObj->AttackCustomZoneY = 0;
		lpObj->AttackCustomZoneMap = 0;
		lpObj->AttackCustomAutoBuff = 0;
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(790));

		this->DGCustomAttackResumeSaveSend(lpObj->Index);
	}

	if(lpObj->AttackCustomOffline == 1)
	{
		lpObj->AttackCustomOffline = 2;
		lpObj->AttackCustomOfflineTime = 5;
	}
}

void CCustomAttack::OnAttackSecondProc(LPOBJ lpObj) // OK
{
	if(lpObj->AttackCustomOffline != 0)
	{
		if(lpObj->AttackCustomOffline == 2)
		{
			if((--lpObj->AttackCustomOfflineTime) == 0)
			{
				gObjDel(lpObj->Index);
				lpObj->AttackCustomOffline = 0;
				lpObj->AttackCustomOfflineTime = 0;
				//lpObj->AttackCustomOfflineMoneyDelay = 0;
			}
		}

		lpObj->CheckSumTime = GetTickCount();
		lpObj->ConnectTickCount = GetTickCount();
		lpObj->OnlineRewardCoin1 = ((gServerInfo.m_OnlineRewardOfflineSystems==0)?GetTickCount():lpObj->OnlineRewardCoin1);
		lpObj->OnlineRewardCoin2 = ((gServerInfo.m_OnlineRewardOfflineSystems==0)?GetTickCount():lpObj->OnlineRewardCoin2);
		lpObj->OnlineRewardCoin3 = ((gServerInfo.m_OnlineRewardOfflineSystems==0)?GetTickCount():lpObj->OnlineRewardCoin3);
	}

	if (lpObj->AttackCustom == 1 && gMapManager.GetMapCustomAttack(lpObj->Map) == 0)
	{
		this->OnAttackClose(lpObj);
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(36));
	}

	if(lpObj->AttackCustom == 1 && lpObj->TradeDuel != 0)
	{
		this->OnAttackClose(lpObj);
	}
}

void CCustomAttack::OnAttackAlreadyConnected(LPOBJ lpObj) // OK
{
	if(lpObj->AttackCustomOffline != 0)
	{
		gObjDel(lpObj->Index);
		lpObj->AttackCustomOffline = 0;
		lpObj->AttackCustomOfflineTime = 0;
		//lpObj->AttackCustomOfflineMoneyDelay = 0;
	}
}

void CCustomAttack::OnAttackMonsterAndMsgProc(LPOBJ lpObj) // OK
{
	CSkill* lpSkill;

	if(lpObj->AttackCustom == 0)
	{
		return;
	}

	//if(this->CheckRequireMoney(lpObj) == 0)
	//{
	//	this->OnAttackClose(lpObj);
	//	return;
	//}

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) != 0)
	{
		this->OnAttackClose(lpObj);
		return;
	}

	if((lpSkill=gSkillManager.GetSkill(lpObj,lpObj->AttackCustomSkill)) == 0)
	{
		this->OnAttackClose(lpObj);
		return;
	}

	if (lpObj->AttackCustomTime > 0) 
	{
		if((--lpObj->AttackCustomTime) == 0)
		{
			gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(794));
			this->OnAttackClose(lpObj);
			return;
		}
	}

	if(gViewport.CheckViewportObjectPosition(lpObj->Index,lpObj->AttackCustomZoneMap,lpObj->AttackCustomZoneX,lpObj->AttackCustomZoneY,5) == 0)
	{
		this->OnAttackClose(lpObj);
		return;
	}

	if((GetTickCount()-((DWORD)lpObj->PotionTime)) >= (DWORD)this->m_CustomAttackPotionDelay && ((lpObj->Life*100)/(lpObj->MaxLife+lpObj->AddLife)) < 50)
	{
		PMSG_ITEM_USE_RECV pMsg;

		pMsg.header.set(0x26,sizeof(pMsg));

		pMsg.SourceSlot = 0xFF;

		pMsg.SourceSlot = ((pMsg.SourceSlot==0xFF)?gItemManager.GetInventoryItemSlot(lpObj,GET_ITEM(14,3),-1):pMsg.SourceSlot);

		pMsg.SourceSlot = ((pMsg.SourceSlot==0xFF)?gItemManager.GetInventoryItemSlot(lpObj,GET_ITEM(14,2),-1):pMsg.SourceSlot);

		pMsg.SourceSlot = ((pMsg.SourceSlot==0xFF)?gItemManager.GetInventoryItemSlot(lpObj,GET_ITEM(14,1),-1):pMsg.SourceSlot);

		pMsg.TargetSlot = 0xFF;

		pMsg.type = 0;

		if(INVENTORY_FULL_RANGE(pMsg.SourceSlot) != 0){gItemManager.CGItemUseRecv(&pMsg,lpObj->Index);}
	}

	if((GetTickCount()-((DWORD)lpObj->PotionTime)) >= (DWORD)this->m_CustomAttackPotionDelay && ((lpObj->Mana*100)/(lpObj->MaxMana+lpObj->AddMana)) < 50)
	{
		PMSG_ITEM_USE_RECV pMsg;

		pMsg.header.set(0x26,sizeof(pMsg));

		pMsg.SourceSlot = 0xFF;

		pMsg.SourceSlot = ((pMsg.SourceSlot==0xFF)?gItemManager.GetInventoryItemSlot(lpObj,GET_ITEM(14,6),-1):pMsg.SourceSlot);

		pMsg.SourceSlot = ((pMsg.SourceSlot==0xFF)?gItemManager.GetInventoryItemSlot(lpObj,GET_ITEM(14,5),-1):pMsg.SourceSlot);

		pMsg.SourceSlot = ((pMsg.SourceSlot==0xFF)?gItemManager.GetInventoryItemSlot(lpObj,GET_ITEM(14,4),-1):pMsg.SourceSlot);

		pMsg.TargetSlot = 0xFF;

		pMsg.type = 0;

		if(INVENTORY_FULL_RANGE(pMsg.SourceSlot) != 0){gItemManager.CGItemUseRecv(&pMsg,lpObj->Index);}
	}

	if (lpObj->AttackCustomAutoBuff == 1)
	{
		if ((GetTickCount()-((DWORD)lpObj->AttackCustomAutoBuffDelay)) > (DWORD)this->m_CustomAttackAutoBuffDelay)
		{
			CSkill* lpSkillBuff;

			if (lpObj->Class == CLASS_FE)
			{

				lpSkillBuff = gSkillManager.GetSkill(lpObj,SKILL_INFINITY_ARROW);
				if (lpSkillBuff != 0)
				{
					if(gEffectManager.CheckEffect(lpObj,EFFECT_INFINITY_ARROW) == 0 && gEffectManager.CheckEffect(lpObj,EFFECT_INFINITY_ARROW_IMPROVED) == 0)
					{
						this->SendSkillAttack(lpObj,lpObj->Index,lpSkillBuff->m_index); //Buff Your self
					}
				}

				lpSkillBuff = gSkillManager.GetSkill(lpObj,SKILL_GREATER_DEFENSE);
				if (lpSkillBuff != 0)
				{
					this->SendSkillAttack(lpObj,lpObj->Index,lpSkillBuff->m_index); //Buff Your self

					this->GetTargetParty(lpObj,lpSkillBuff->m_index);
				}

				lpSkillBuff = gSkillManager.GetSkill(lpObj,SKILL_GREATER_DAMAGE);
				if (lpSkillBuff != 0)
				{
					this->SendSkillAttack(lpObj,lpObj->Index,lpSkillBuff->m_index); //Buff Your self

					this->GetTargetParty(lpObj,lpSkillBuff->m_index);

				}

				lpSkillBuff = gSkillManager.GetSkill(lpObj,SKILL_HEAL);
				if (lpSkillBuff != 0)
				{
					gSkillManager.SkillHeal(lpObj->Index,lpObj->Index,lpSkillBuff);
				}
			}

			if (lpObj->Class == CLASS_DW)
			{
				lpSkillBuff = gSkillManager.GetSkill(lpObj,SKILL_MAGIC_CIRCLE);
				if (gEffectManager.GetEffect(lpObj,EFFECT_MAGIC_CIRCLE) == 0 && lpSkillBuff != 0)
				{
					this->SendSkillAttack(lpObj,lpObj->Index,lpSkillBuff->m_index); //Buff Your self
				}
				
				lpSkillBuff = gSkillManager.GetSkill(lpObj,SKILL_MANA_SHIELD);
				if (lpSkillBuff != 0)
				{
					if(gEffectManager.GetEffect(lpObj,EFFECT_MANA_SHIELD) == 0)
					{
						this->SendSkillAttack(lpObj,lpObj->Index,lpSkillBuff->m_index); //Buff Your self
					}

					this->GetTargetParty(lpObj,lpSkillBuff->m_index);
				}
			}

			if (lpObj->Class == CLASS_DL)
			{
				lpSkillBuff = gSkillManager.GetSkill(lpObj,SKILL_GREATER_CRITICAL_DAMAGE);
				if (gEffectManager.GetEffect(lpObj,EFFECT_GREATER_CRITICAL_DAMAGE) == 0 && lpSkillBuff != 0)
				{
					this->SendSkillAttack(lpObj,lpObj->Index,lpSkillBuff->m_index); //Buff Your self
				}
			}

			if (lpObj->Class == CLASS_DK)
			{
				lpSkillBuff = gSkillManager.GetSkill(lpObj,SKILL_GREATER_LIFE);
				if (gEffectManager.GetEffect(lpObj,EFFECT_GREATER_LIFE) == 0 && lpSkillBuff != 0)
				{
					this->SendSkillAttack(lpObj,lpObj->Index,lpSkillBuff->m_index); //Buff Your self
				}
			}

			if (lpObj->Class == CLASS_SU)
			{
				lpSkillBuff = gSkillManager.GetSkill(lpObj,SKILL_DAMAGE_REFLECT);
				if (gEffectManager.GetEffect(lpObj,EFFECT_DAMAGE_REFLECT) == 0 && lpSkillBuff != 0)
				{
					this->SendSkillAttack(lpObj,lpObj->Index,lpSkillBuff->m_index); //Buff Your self
				}

				lpSkillBuff = gSkillManager.GetSkill(lpObj,SKILL_SWORD_POWER);
				if (gEffectManager.GetEffect(lpObj,SKILL_SWORD_POWER) == 0 && lpSkillBuff != 0)
				{
					this->SendSkillAttack(lpObj,lpObj->Index,lpSkillBuff->m_index); //Buff Your self
				}
			}

			if (lpObj->Class == CLASS_RF)
			{
				lpSkillBuff = gSkillManager.GetSkill(lpObj,SKILL_GREATER_IGNORE_DEFENSE_RATE);
				if (gEffectManager.GetEffect(lpObj,EFFECT_GREATER_IGNORE_DEFENSE_RATE) == 0 && lpSkillBuff != 0)
				{
					this->SendSkillAttack(lpObj,lpObj->Index,lpSkillBuff->m_index); //Buff Your self
				}

				lpSkillBuff = gSkillManager.GetSkill(lpObj,SKILL_FITNESS);
				if (gEffectManager.GetEffect(lpObj,EFFECT_FITNESS) == 0 && lpSkillBuff != 0)
				{
					this->SendSkillAttack(lpObj,lpObj->Index,lpSkillBuff->m_index); //Buff Your self
				}

				lpSkillBuff = gSkillManager.GetSkill(lpObj,SKILL_GREATER_DEFENSE_SUCCESS_RATE);
				if (gEffectManager.GetEffect(lpObj,EFFECT_GREATER_DEFENSE_SUCCESS_RATE) == 0 && lpSkillBuff != 0)
				{
					this->SendSkillAttack(lpObj,lpObj->Index,lpSkillBuff->m_index); //Buff Your self
				}
			}

			lpObj->AttackCustomAutoBuffDelay = GetTickCount();
		}

	}

	if(lpObj->Mana < gSkillManager.GetSkillMana(lpSkill->m_index))
	{
		return;
	}

	if((GetTickCount()-((DWORD)lpObj->AttackCustomDelay)) >= (((((DWORD)lpObj->PhysiSpeed)*5)>1500)?0:(1500-(((DWORD)lpObj->PhysiSpeed)*5))))
	{
		if ((GetTickCount()-((DWORD)lpObj->AttackCustomDelay)) < (DWORD)this->m_CustomAttackDelay)
		{
			return;
		}

		//this->ItemGet(lpObj);

		int MonsterIndex = -1;

		lpObj->AttackCustomDelay = GetTickCount();

		if(this->GetTargetMonster(lpObj,lpSkill->m_index,&MonsterIndex) != 0)
		{
			switch(lpSkill->m_skill)
			{
				case SKILL_EVIL_SPIRIT:
					this->SendMultiSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_INFERNO:
					this->SendMultiSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_ICE_STORM:
					this->SendDurationSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_TWISTING_SLASH:
					this->SendDurationSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_DEATH_STAB:
					this->SendMultiSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_POWER_SLASH:
					gSkillManager.GCSkillAttackSend(lpObj,lpSkill->m_index,MonsterIndex,1);
					this->SendDurationSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_FORCE:
					this->SendSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_FIRE_BURST:
					this->SendMultiSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_FIRE_SCREAM:
					this->SendMultiSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_CHAIN_LIGHTNING:
					this->SendSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_RED_STORM:
					this->SendDurationSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_FIVE_SHOT:
					this->SendDurationSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_TRIPLE_SHOT:
					this->SendMultiSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_SWORD_SLASH:
					this->SendDurationSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_LIGHTNING_STORM:
					this->SendDurationSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_BIRDS:
					gSkillManager.GCSkillAttackSend(lpObj,lpSkill->m_index,MonsterIndex,1);
					this->SendDurationSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_LARGE_RING_BLOWER:
					this->SendSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_DRAGON_LORE:
					this->SendDurationSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_PHOENIX_SHOT:
					this->SendSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
				case SKILL_DARK_SIDE:
					this->SendRFSkillAttack(lpObj,MonsterIndex,lpSkill->m_index);
					break;
			}
		}
	}
}

void CCustomAttack::SendSkillAttack(LPOBJ lpObj,int aIndex,int SkillNumber) // OK
{
	PMSG_SKILL_ATTACK_RECV pMsg;

	pMsg.header.set(0x19,sizeof(pMsg));

	#if(GAMESERVER_UPDATE>=701)

	pMsg.skillH = SET_NUMBERHB(SkillNumber);

	pMsg.skillL = SET_NUMBERLB(SkillNumber);

	pMsg.indexH = SET_NUMBERHB(aIndex);

	pMsg.indexL = SET_NUMBERLB(aIndex);

	#else

	pMsg.skill[0] = SET_NUMBERHB(SkillNumber);

	pMsg.skill[1] = SET_NUMBERLB(SkillNumber);

	pMsg.index[0] = SET_NUMBERHB(aIndex);

	pMsg.index[1] = SET_NUMBERLB(aIndex);

	#endif

	pMsg.dis = 0;

	gSkillManager.CGSkillAttackRecv(&pMsg,lpObj->Index);
}

void CCustomAttack::SendMultiSkillAttack(LPOBJ lpObj,int aIndex,int SkillNumber) // OK
{
	this->SendDurationSkillAttack(lpObj,aIndex,SkillNumber);

	BYTE send[256];

	PMSG_MULTI_SKILL_ATTACK_RECV pMsg;

	int size = sizeof(pMsg);

	#if(GAMESERVER_UPDATE>=701)

	pMsg.skillH = SET_NUMBERHB(SkillNumber);

	pMsg.skillL = SET_NUMBERLB(SkillNumber);

	#else

	pMsg.skill[0] = SET_NUMBERHB(SkillNumber);

	pMsg.skill[1] = SET_NUMBERLB(SkillNumber);

	#endif

	pMsg.x = (BYTE)lpObj->X;

	pMsg.y = (BYTE)lpObj->Y;

	pMsg.serial = 0;

	pMsg.count = 0;

	PMSG_MULTI_SKILL_ATTACK info;

	for(int n=0;n < MAX_VIEWPORT;n++)
	{
		if(lpObj->VpPlayer2[n].state == VIEWPORT_NONE || OBJECT_RANGE(lpObj->VpPlayer2[n].index) == 0 || lpObj->VpPlayer2[n].type != OBJECT_MONSTER)
		{
			continue;
		}

		int index = lpObj->VpPlayer2[n].index;

		if(gSkillManager.CheckSkillTarget(lpObj,index,aIndex,lpObj->VpPlayer2[n].type) == 0)
		{
			continue;
		}

		if(gSkillManager.CheckSkillRadio(SkillNumber,lpObj->X,lpObj->Y,gObj[index].X,gObj[index].Y) == 0)
		{
			continue;
		}

		#if(GAMESERVER_UPDATE>=701)

		info.indexH = SET_NUMBERHB(index);

		info.indexL = SET_NUMBERLB(index);

		#else

		info.index[0] = SET_NUMBERHB(index);

		info.index[1] = SET_NUMBERLB(index);

		#endif

		info.MagicKey = 0;

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		if(CHECK_SKILL_ATTACK_COUNT(pMsg.count) == 0)
		{
			break;
		}
	}

	memcpy(send,&pMsg,sizeof(pMsg));

	gSkillManager.CGMultiSkillAttackRecv((PMSG_MULTI_SKILL_ATTACK_RECV*)send,lpObj->Index,0);
}

void CCustomAttack::SendDurationSkillAttack(LPOBJ lpObj,int aIndex,int SkillNumber) // OK
{
	PMSG_DURATION_SKILL_ATTACK_RECV pMsg;

	pMsg.header.set(0x1E,sizeof(pMsg));

	#if(GAMESERVER_UPDATE>=701)

	pMsg.skillH = SET_NUMBERHB(SkillNumber);

	pMsg.skillL = SET_NUMBERLB(SkillNumber);

	#else

	pMsg.skill[0] = SET_NUMBERHB(SkillNumber);

	pMsg.skill[1] = SET_NUMBERLB(SkillNumber);

	#endif

	pMsg.x = (BYTE)gObj[aIndex].X;

	pMsg.y = (BYTE)gObj[aIndex].Y;

	pMsg.dir = (gSkillManager.GetSkillAngle(gObj[aIndex].X,gObj[aIndex].Y,lpObj->X,lpObj->Y)*255)/360;

	pMsg.dis = 0;

	pMsg.angle = (gSkillManager.GetSkillAngle(lpObj->X,lpObj->Y,gObj[aIndex].X,gObj[aIndex].Y)*255)/360;

	#if(GAMESERVER_UPDATE>=803)

	pMsg.indexH = SET_NUMBERHB(aIndex);

	pMsg.indexL = SET_NUMBERLB(aIndex);

	#else

	pMsg.index[0] = SET_NUMBERHB(aIndex);

	pMsg.index[1] = SET_NUMBERLB(aIndex);

	#endif

	pMsg.MagicKey = 0;

	gSkillManager.CGDurationSkillAttackRecv(&pMsg,lpObj->Index);
}

void CCustomAttack::SendRFSkillAttack(LPOBJ lpObj,int aIndex,int SkillNumber) // OK
{
	PMSG_SKILL_DARK_SIDE_RECV Msg;

	Msg.skill[0] = SET_NUMBERHB(SkillNumber);

	Msg.skill[1] = SET_NUMBERLB(SkillNumber);

	Msg.index[0] = SET_NUMBERHB(aIndex);

	Msg.index[1] = SET_NUMBERLB(aIndex);

	gSkillManager.CGSkillDarkSideRecv(&Msg,lpObj->Index);

	PMSG_RAGE_FIGHTER_SKILL_ATTACK_RECV pMsg;

	pMsg.header.set(0x19,sizeof(pMsg));

	#if(GAMESERVER_UPDATE>=701)

	pMsg.skillH = SET_NUMBERHB(SkillNumber);

	pMsg.skillL = SET_NUMBERLB(SkillNumber);

	pMsg.indexH = SET_NUMBERHB(aIndex);

	pMsg.indexL = SET_NUMBERLB(aIndex);

	#else

	pMsg.skill[0] = SET_NUMBERHB(SkillNumber);

	pMsg.skill[1] = SET_NUMBERLB(SkillNumber);

	pMsg.index[0] = SET_NUMBERHB(aIndex);

	pMsg.index[1] = SET_NUMBERLB(aIndex);

	#endif

	pMsg.dis = 0;

	//gSkillManager.CGSkillAttackRecv(&pMsg,lpObj->Index);
	gSkillManager.CGRageFighterSkillAttackRecv(&pMsg,lpObj->Index);
	this->SendDurationSkillAttack(lpObj,aIndex,SkillNumber);
}

void CCustomAttack::GetTargetParty(LPOBJ lpObj,int SkillNumber) // OK
{
	int NearestDistance = 100;

	for(int n=0;n < MAX_VIEWPORT;n++)
	{
		NearestDistance = 100;

		if(lpObj->VpPlayer2[n].state == VIEWPORT_NONE || OBJECT_RANGE(lpObj->VpPlayer2[n].index) == 0 || (lpObj->VpPlayer2[n].type != OBJECT_USER && OBJECT_RANGE(gObj[lpObj->VpPlayer2[n].index].SummonIndex) == 0))
		{
			continue;
		}

		if(gObjCalcDistance(lpObj,&gObj[lpObj->VpPlayer2[n].index]) >= NearestDistance)
		{
			continue;
		}

		if(gParty.IsParty(lpObj->PartyNumber) == 0) //
		{
			continue;
		}

		if(gParty.IsParty(lpObj->PartyNumber) == 1 && lpObj->PartyNumber != gObj[lpObj->VpPlayer2[n].index].PartyNumber) //
		{
			continue;
		}

		if(gSkillManager.CheckSkillRange(SkillNumber,lpObj->X,lpObj->Y,gObj[lpObj->VpPlayer2[n].index].X,gObj[lpObj->VpPlayer2[n].index].Y) != 0)
		{
			this->SendSkillAttack(lpObj,lpObj->VpPlayer2[n].index,SkillNumber); //Buff party
			continue;
		}

		if(gSkillManager.CheckSkillRadio(SkillNumber,lpObj->X,lpObj->Y,gObj[lpObj->VpPlayer2[n].index].X,gObj[lpObj->VpPlayer2[n].index].Y) != 0)
		{
			this->SendSkillAttack(lpObj,lpObj->VpPlayer2[n].index,SkillNumber); //Buff party
			continue;
		}
	}
	return;
}

void CCustomAttack::DGCustomAttackResumeSend(int aIndex) // OK
{
#if GAMESERVER_CLIENTE_UPDATE >= 11

	LPOBJ lpObj = &gObj[aIndex];

	if (this->m_CustomAttackAutoResume[lpObj->AccountLevel] == 0)
	{
		return;
	}

	SDHP_CARESUME_SEND pMsg;

	pMsg.header.set(0xF5,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));

#endif
}

void CCustomAttack::DGCustomAttackResumeRecv(SDHP_CARESUME_RECV* lpMsg) // OK
{
#if GAMESERVER_CLIENTE_UPDATE >= 11

	LPOBJ lpObj = &gObj[lpMsg->index];

	if(gObjIsConnectedGP(lpMsg->index) == 0)
	{
		return;
	}

	if(strcmp(lpMsg->name,lpObj->Name) != 0)
	{
		return;
	}

	lpObj->AttackCustom = lpMsg->active;

	lpObj->AttackCustomSkill = lpMsg->skill;

	lpObj->AttackCustomDelay = GetTickCount();

	lpObj->AttackCustomZoneX = lpMsg->posx;

	lpObj->AttackCustomZoneY = lpMsg->posy;

	lpObj->AttackCustomZoneMap = lpMsg->map;

	lpObj->AttackCustomAutoBuff = lpMsg->autobuff;

	lpObj->DisablePvp = lpMsg->offpvp;

	lpObj->AutoResetEnable = lpMsg->autoreset;

	lpObj->AutoResetStats[0] = lpMsg->autoaddstr;

	lpObj->AutoResetStats[1] = lpMsg->autoaddagi;

	lpObj->AutoResetStats[2] = lpMsg->autoaddvit;

	lpObj->AutoResetStats[3] = lpMsg->autoaddene;

	lpObj->AutoResetStats[4] = lpMsg->autoaddcmd;
#endif
}

void CCustomAttack::DGCustomAttackResumeSaveSend(int Index)
{
#if GAMESERVER_CLIENTE_UPDATE >= 11

	LPOBJ lpObj = &gObj[Index];

	if (this->m_CustomAttackAutoResume[lpObj->AccountLevel] == 0)
	{
		return;
	}

	SDHP_CARESUME_SAVE_SEND pMsg;

	pMsg.header.set(0xF6,sizeof(pMsg));

	pMsg.index = lpObj->Index;

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.active = lpObj->AttackCustom;

	pMsg.skill = lpObj->AttackCustomSkill;

	pMsg.posx = lpObj->AttackCustomZoneX;

	pMsg.posy = lpObj->AttackCustomZoneY;

	pMsg.map = lpObj->AttackCustomZoneMap;

	pMsg.autobuff = lpObj->AttackCustomAutoBuff;

	pMsg.offpvp = lpObj->DisablePvp;

	pMsg.autoreset = lpObj->AutoResetEnable;

	pMsg.autoaddstr = lpObj->AutoResetStats[0];

	pMsg.autoaddagi = lpObj->AutoResetStats[1];

	pMsg.autoaddvit = lpObj->AutoResetStats[2];

	pMsg.autoaddene = lpObj->AutoResetStats[3];

	pMsg.autoaddcmd = lpObj->AutoResetStats[4];

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg)); 
#endif
}
