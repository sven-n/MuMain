// DarkSpirit.cpp: implementation of the CDarkSpirit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DarkSpirit.h"
#include "Attack.h"
#include "BattleSoccerManager.h"
#include "CastleSiege.h"
#include "Crywolf.h"
#include "DefaultClassInfo.h"
#include "Duel.h"
#include "EffectManager.h"
#include "GensSystem.h"
#include "Guild.h"
#include "ItemManager.h"
#include "Kalima.h"
#include "Map.h"
#include "MapManager.h"
#include "MasterSkillTree.h"
#include "Monster.h"
#include "ObjectManager.h"
#include "ServerInfo.h"
#include "SkillManager.h"
#include "Util.h"
#include "Viewport.h"

CDarkSpirit gDarkSpirit[MAX_OBJECT];
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDarkSpirit::CDarkSpirit() // OK
{
	this->Init();
}

CDarkSpirit::~CDarkSpirit() // OK
{

}

void CDarkSpirit::Init() // OK
{
	this->m_AttackDamageMin = 0;
	this->m_AttackDamageMax = 0;
	this->m_AttackSpeed = 0;
	this->m_AttackSuccessRate = 0;
	this->m_MasterIndex = -1;
	this->m_TargetIndex = -1;
	this->m_LastAttackTime = 0;
	this->m_Item = 0;
}

void CDarkSpirit::MainProc() // OK
{
	if(gObjIsConnectedGP(this->m_MasterIndex) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[this->m_MasterIndex];

	if(lpObj->Class != CLASS_DL)
	{
		return;
	}

	if(this->m_Item == 0)
	{
		return;
	}

	if(this->m_Item->m_Durability == 0)
	{
		return;
	}

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) != 0)
	{
		return;
	}

	if(this->m_LastAttackTime > GetTickCount())
	{
		return;
	}

	this->m_LastAttackTime = (GetTickCount()+1500)-(this->m_AttackSpeed*10);

	switch(this->m_ActionMode)
	{
		case DARK_SPIRIT_MODE_NORMAL:
			this->ModeNormal();
			break;
		case DARK_SPIRIT_MODE_ATTACK_RANDOM:
			this->ModeAttackRandom();
			break;
		case DARK_SPIRIT_MODE_ATTACK_WITH_MASTER:
			this->ModeAttackWithMaster();
			break;
		case DARK_SPIRIT_MODE_ATTACK_TARGET:
			this->ModeAttakTarget();
			break;
	}
}

void CDarkSpirit::ModeNormal() // OK
{

}

void CDarkSpirit::ModeAttackRandom() // OK
{
	LPOBJ lpObj = &gObj[this->m_MasterIndex];
	int IndexTable[MAX_VIEWPORT];
	int IndexCount = 0;

	for(int n=0;n < MAX_VIEWPORT;n++)
	{
		if(lpObj->VpPlayer2[n].state == VIEWPORT_NONE)
		{
			continue;
		}

		int index = lpObj->VpPlayer2[n].index;

		if(gSkillManager.CheckSkillTarget(lpObj,index,-1,lpObj->VpPlayer2[n].type) == 0)
		{
			continue;
		}

		if(gObjCalcDistance(lpObj,&gObj[index]) >= 5)
		{
			continue;
		}

		IndexTable[IndexCount++] = index;
	}

	if(IndexCount > 0)
	{
		if((GetLargeRand()%100) < gServerInfo.m_DarkSpiritRangeAttackRate)
		{
			this->RangeAttack(lpObj->Index,IndexTable[GetLargeRand()%IndexCount]);
		}
		else
		{
			this->SendAttackMsg(lpObj->Index,IndexTable[GetLargeRand()%IndexCount],1,0);
		}
	}
}

void CDarkSpirit::ModeAttackWithMaster() // OK
{
	LPOBJ lpObj = &gObj[this->m_MasterIndex];

	if(OBJECT_RANGE(this->m_TargetIndex) == 0)
	{
		return;
	}

	LPOBJ lpTarget = &gObj[this->m_TargetIndex];

	if(lpTarget->Live == 0 || lpTarget->State != OBJECT_PLAYING)
	{
		this->ResetTarget(lpTarget->Index);
		return;
	}

	if(lpObj->Map != lpTarget->Map)
	{
		this->ResetTarget(lpTarget->Index);
		return;
	}

	if(gObjCalcDistance(lpObj,lpTarget) >= 7)
	{
		return;
	}

	if((GetLargeRand()%100) < gServerInfo.m_DarkSpiritRangeAttackRate)
	{
		this->RangeAttack(lpObj->Index,lpTarget->Index);
	}
	else
	{
		this->SendAttackMsg(lpObj->Index,lpTarget->Index,1,0);
	}
}

void CDarkSpirit::ModeAttakTarget() // OK
{
	LPOBJ lpObj = &gObj[this->m_MasterIndex];

	if(OBJECT_RANGE(this->m_TargetIndex) == 0)
	{
		return;
	}

	LPOBJ lpTarget = &gObj[this->m_TargetIndex];

	if(lpTarget->Live == 0 || lpTarget->State != OBJECT_PLAYING)
	{
		this->ResetTarget(lpTarget->Index);
		return;
	}

	if(lpObj->Map != lpTarget->Map)
	{
		this->ResetTarget(lpTarget->Index);
		return;
	}

	if(gObjCalcDistance(lpObj,lpTarget) >= 7)
	{
		return;
	}

	if((GetLargeRand()%100) < gServerInfo.m_DarkSpiritRangeAttackRate)
	{
		this->RangeAttack(lpObj->Index,lpTarget->Index);
	}
	else
	{
		this->SendAttackMsg(lpObj->Index,lpTarget->Index,1,0);
	}
}

void CDarkSpirit::RangeAttack(int aIndex,int bIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(OBJECT_RANGE(bIndex) == 0)
	{
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	this->SendAttackMsg(aIndex,bIndex,1,1);

	int count = 0;

	for(int n=0;n < MAX_VIEWPORT;n++)
	{
		if(lpObj->VpPlayer2[n].state == VIEWPORT_NONE)
		{
			continue;
		}

		if(lpObj->VpPlayer2[n].index == bIndex)
		{
			continue;
		}

		int index = lpObj->VpPlayer2[n].index;

		if(gSkillManager.CheckSkillTarget(lpObj,index,bIndex,lpObj->VpPlayer2[n].type) == 0)
		{
			continue;
		}

		if(gObjCalcDistance(lpTarget,&gObj[index]) >= 4)
		{
			continue;
		}

		this->SendAttackMsg(aIndex,index,0,1);

		if(CHECK_SKILL_ATTACK_COUNT(count) == 0)
		{
			break;
		}
	}
}

void CDarkSpirit::SendAttackMsg(int aIndex,int bIndex,bool send,BYTE action) // OK
{
	if(send != 0)
	{
		PMSG_DARK_SPIRIT_ATTACK_SEND pMsg;

		pMsg.header.set(0xA8,sizeof(pMsg));

		pMsg.type = 0;

		pMsg.action = action;

		pMsg.index[0] = SET_NUMBERHB(aIndex);

		pMsg.index[1] = SET_NUMBERLB(aIndex);

		pMsg.target[0] = SET_NUMBERHB(bIndex);

		pMsg.target[1] = SET_NUMBERLB(bIndex);

		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

		MsgSendV2(&gObj[aIndex],(BYTE*)&pMsg,pMsg.header.size);
	}

	gObjAddAttackProcMsgSendDelay(&gObj[aIndex],51,bIndex,600,send,action);
}

void CDarkSpirit::Set(int aIndex,CItem* lpItem) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Class != CLASS_DL)
	{
		return;
	}

	bool change = 0;

	if(OBJECT_RANGE(this->m_MasterIndex) == 0)
	{
		change = 1;
	}

	this->Init();

	if(lpItem->m_Index != GET_ITEM(13,5)) // Dark Spirit
	{
		return;
	}

	if(lpItem->m_IsValidItem == 0)
	{
		return;
	}

	this->m_MasterIndex = aIndex;

	this->m_Item = lpItem;

	this->m_AttackDamageMin = gServerInfo.m_DarkSpiritAttackDamageMinConstA+(lpItem->m_PetItemLevel*gServerInfo.m_DarkSpiritAttackDamageMinConstB)+((lpObj->Leadership+lpObj->AddLeadership)/gServerInfo.m_DarkSpiritAttackDamageMinConstC);

	this->m_AttackDamageMax = gServerInfo.m_DarkSpiritAttackDamageMaxConstA+(lpItem->m_PetItemLevel*gServerInfo.m_DarkSpiritAttackDamageMaxConstB)+((lpObj->Leadership+lpObj->AddLeadership)/gServerInfo.m_DarkSpiritAttackDamageMaxConstC);

	this->m_AttackSpeed = gServerInfo.m_DarkSpiritAttackSpeedConstA+((lpItem->m_PetItemLevel*gServerInfo.m_DarkSpiritAttackSpeedConstB)/gServerInfo.m_DarkSpiritAttackSpeedConstC)+((lpObj->Leadership+lpObj->AddLeadership)/gServerInfo.m_DarkSpiritAttackSpeedConstD);

	this->m_AttackSuccessRate = gServerInfo.m_DarkSpiritAttackSuccessRateConstA+((lpItem->m_PetItemLevel*gServerInfo.m_DarkSpiritAttackSuccessRateConstB)/gServerInfo.m_DarkSpiritAttackSuccessRateConstC);

	if(change != 0)
	{
		this->SetMode(DARK_SPIRIT_MODE_NORMAL,-1);
	}
}

void CDarkSpirit::SetMode(eDarkSpiritMode mode,int aIndex) // OK
{
	if(OBJECT_RANGE(this->m_MasterIndex) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[this->m_MasterIndex];

	if(lpObj->Class != CLASS_DL)
	{
		return;
	}

	if(mode == this->m_ActionMode && mode != DARK_SPIRIT_MODE_ATTACK_TARGET)
	{
		return;
	}

	if(mode == DARK_SPIRIT_MODE_ATTACK_TARGET)
	{
		if(OBJECT_RANGE(aIndex) == 0)
		{
			return;
		}

		this->m_TargetIndex = aIndex;
	}

	this->m_ActionMode = mode;

	PMSG_DARK_SPIRIT_MODE_SEND pMsg;

	pMsg.header.set(0xA7,sizeof(pMsg));

	pMsg.type = 0;

	pMsg.mode = mode;

	pMsg.index[0] = SET_NUMBERHB(aIndex);

	pMsg.index[1] = SET_NUMBERLB(aIndex);

	DataSend(this->m_MasterIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CDarkSpirit::SetTarget(int aIndex) // OK
{
	if(OBJECT_RANGE(this->m_MasterIndex) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[this->m_MasterIndex];

	if(lpObj->Class != CLASS_DL)
	{
		return;
	}

	this->m_TargetIndex = aIndex;
}

void CDarkSpirit::ResetTarget(int aIndex) // OK
{
	if(OBJECT_RANGE(this->m_MasterIndex) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[this->m_MasterIndex];

	if(lpObj->Class != CLASS_DL)
	{
		return;
	}

	if(this->m_TargetIndex == aIndex)
	{
		this->m_TargetIndex = -1;

		if(this->m_ActionMode == DARK_SPIRIT_MODE_ATTACK_TARGET)
		{
			this->SetMode(DARK_SPIRIT_MODE_NORMAL,-1);
		}
	}
}

void CDarkSpirit::ChangeCommand(int command,int aIndex) // OK
{
	switch(command)
	{
		case DARK_SPIRIT_MODE_NORMAL:
			this->SetMode(DARK_SPIRIT_MODE_NORMAL,aIndex);
			break;
		case DARK_SPIRIT_MODE_ATTACK_RANDOM:
			this->SetMode(DARK_SPIRIT_MODE_ATTACK_RANDOM,aIndex);
			break;
		case DARK_SPIRIT_MODE_ATTACK_WITH_MASTER:
			this->SetMode(DARK_SPIRIT_MODE_ATTACK_WITH_MASTER,aIndex);
			break;
		case DARK_SPIRIT_MODE_ATTACK_TARGET:
			this->SetMode(DARK_SPIRIT_MODE_ATTACK_TARGET,aIndex);
			break;
		default:
			this->SetMode(DARK_SPIRIT_MODE_NORMAL,aIndex);
			break;
	}
}

bool CDarkSpirit::Attack(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,bool send,BYTE flag,BYTE action) // OK
{
	#pragma region ATTACK_CHECK

	if(lpObj->Index == lpTarget->Index)
	{
		return 0;
	}

	if(lpObj->Type == OBJECT_USER && gObjIsConnectedGP(lpObj->Index) == 0)
	{
		return 0;
	}

	if(lpTarget->Type == OBJECT_USER && gObjIsConnectedGP(lpTarget->Index) == 0)
	{
		return 0;
	}

	if(lpObj->Map != lpTarget->Map || lpObj->Teleport == 2)
	{
		return 0;
	}

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) != 0 || gMap[lpTarget->Map].CheckAttr(lpTarget->X,lpTarget->Y,1) != 0)
	{
		return 0;
	}

	if(lpTarget->Type == OBJECT_USER && lpObj->DisablePvp == 1)
	{
		return 0;
	}

	if(lpTarget->Type == OBJECT_USER && lpTarget->DisablePvp == 1)
	{
		return 0;
	}

	#if(GAMESERVER_UPDATE>=402)

	if(gDuel.GetDuelArenaBySpectator(lpObj->Index) != 0 || gDuel.GetDuelArenaBySpectator(lpTarget->Index) != 0)
	{
		return 0;
	}

	#endif

	#if(GAMESERVER_TYPE==1)

	if(gCastleSiege.GetCastleState() != CASTLESIEGE_STATE_STARTSIEGE)
	{
		if(lpTarget->Type == OBJECT_MONSTER && lpTarget->Map == MAP_CASTLE_SIEGE && (lpTarget->Class == 277 || lpTarget->Class == 283 || lpTarget->Class == 288))
		{
			return 0;
		}
	}

	if(gCrywolf.GetCrywolfState() == CRYWOLF_STATE_READY || gCrywolf.GetCrywolfState() == CRYWOLF_STATE_END)
	{
		if(lpTarget->Type == OBJECT_MONSTER && lpTarget->Map == MAP_CRYWOLF)
		{
			return 0;
		}
	}

	#endif

	if(lpObj->GuildNumber > 0 && lpObj->Guild != 0 && lpObj->Guild->WarState != 0)
	{
		if(lpObj->Guild->WarType == 1 && GetBattleSoccerGoalMove(0) == 0)
		{
			return 1;
		}

		if(lpObj->Guild->WarType == 0 && lpTarget->Type == OBJECT_MONSTER)
		{
			return 0;
		}
	}

	if(lpTarget->Type == OBJECT_NPC || lpTarget->Live == 0 || lpTarget->State != OBJECT_PLAYING || lpTarget->Teleport != 0)
	{
		return 0;
	}

	if(lpTarget->Type == OBJECT_MONSTER)
	{
		if(KALIMA_ATTRIBUTE_RANGE(lpTarget->Attribute) != 0)
		{
			return 0;
		}

		if((lpTarget->Class >= 100 && lpTarget->Class <= 110) || lpTarget->Class == 523) // Trap
		{
			return 0;
		}

		if(lpTarget->Class == 221 || lpTarget->Class == 222) // Siege
		{
			return 0;
		}
	}

	if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_MONSTER)
	{
		if(OBJECT_RANGE(lpObj->SummonIndex) != 0)
		{
			if(lpObj->SummonIndex == lpTarget->Index)
			{
				return 0;
			}
		}
	}

	int SummonIndex = lpObj->Index;

	if(lpObj->Type == OBJECT_MONSTER && OBJECT_RANGE(lpObj->SummonIndex) != 0)
	{
		SummonIndex = lpObj->SummonIndex;
	}

	int SummonTargetIndex = lpTarget->Index;

	if(lpTarget->Type == OBJECT_MONSTER && OBJECT_RANGE(lpTarget->SummonIndex) != 0)
	{
		SummonTargetIndex = lpTarget->SummonIndex;
	}

	if(gAttack.CheckPlayerTarget(&gObj[SummonIndex],&gObj[SummonTargetIndex]) == 0)
	{
		return 0;
	}

	#pragma endregion

	#pragma region ATTACK_RETURN

	int skill = ((lpSkill==0)?SKILL_NONE:lpSkill->m_skill);

	if(lpObj->Type == OBJECT_USER)
	{
		lpObj->HPAutoRecuperationTime = GetTickCount();
		lpObj->MPAutoRecuperationTime = GetTickCount();
		lpObj->BPAutoRecuperationTime = GetTickCount();
		lpObj->SDAutoRecuperationTime = GetTickCount();
	}

	if(lpTarget->Type == OBJECT_USER)
	{
		lpTarget->HPAutoRecuperationTime = GetTickCount();
		lpTarget->MPAutoRecuperationTime = GetTickCount();
		lpTarget->BPAutoRecuperationTime = GetTickCount();
		lpTarget->SDAutoRecuperationTime = GetTickCount();
	}

	if(OBJECT_RANGE(lpObj->SummonIndex) != 0)
	{
		gObjSummonSetEnemy(lpObj,lpTarget->Index);
	}

	if(lpObj->Type == OBJECT_USER)
	{
		gDarkSpirit[lpObj->Index].SetTarget(lpTarget->Index);
	}

	bool duel = gDuel.CheckDuel(lpObj,lpTarget);

	if(lpObj->Type == OBJECT_USER && duel != 0)
	{
		lpObj->DuelTickCount = GetTickCount();
	}

	if(lpTarget->Type == OBJECT_USER && duel != 0)
	{
		lpTarget->DuelTickCount = GetTickCount();
	}

	gEffectManager.DelEffect(lpObj,EFFECT_INVISIBILITY);

	if(gEffectManager.CheckEffect(lpTarget,EFFECT_ORDER_OF_PROTECTION) != 0 && lpSkill == 0)
	{
		gAttack.MissSend(lpObj,lpTarget,lpSkill,send,0);
		return 1;
	}

	if(gEffectManager.CheckEffect(lpTarget,EFFECT_ORDER_OF_PROTECTION) != 0 && lpSkill != 0)
	{
		gAttack.MissSend(lpObj,lpTarget,lpSkill,send,0);
		return 1;
	}

	if(gEffectManager.CheckEffect(lpTarget,EFFECT_PHYSI_DAMAGE_IMMUNITY) != 0 && lpSkill == 0)
	{
		gAttack.MissSend(lpObj,lpTarget,lpSkill,send,0);
		return 1;
	}

	if(gEffectManager.CheckEffect(lpTarget,EFFECT_MAGIC_DAMAGE_IMMUNITY) != 0 && lpSkill != 0)
	{
		gAttack.MissSend(lpObj,lpTarget,lpSkill,send,0);
		return 1;
	}

	if(lpTarget->Type == OBJECT_MONSTER)
	{
		if(lpTarget->Class == 200 && lpSkill == 0)
		{
			gObjMonsterStateProc(lpTarget,6,lpObj->Index,0);
			gAttack.MissSend(lpObj,lpTarget,lpSkill,send,0);
			return 1;
		}

		if(lpTarget->Class == 200 && lpSkill != 0)
		{
			gObjMonsterStateProc(lpTarget,7,lpObj->Index,0);
			gAttack.MissSend(lpObj,lpTarget,lpSkill,send,0);
			return 1;
		}

		if(gEffectManager.CheckEffect(lpTarget,EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY) != 0 && skill == SKILL_NONE)
		{
			gAttack.MissSend(lpObj,lpTarget,lpSkill,send,0);
			return 1;
		}

		if(gEffectManager.CheckEffect(lpTarget,EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY) != 0 && skill != SKILL_NONE)
		{
			gAttack.MissSend(lpObj,lpTarget,lpSkill,send,0);
			return 1;
		}

		if(lpTarget->MonsterSkillElementOption.CheckImmuneTime() != 0)
		{
			if(lpTarget->MonsterSkillElementOption.m_SkillElementImmuneNumber == skill)
			{
				gAttack.MissSend(lpObj,lpTarget,lpSkill,send,0);
				return 1;
			}
		}
	}

	#pragma endregion

	#pragma region DAMAGE_CALC

	flag = 0;

	BYTE miss = 0;

	WORD effect = 0;

	if((lpObj->Type != OBJECT_USER || lpTarget->Type != OBJECT_USER) && this->MissCheck(lpObj,lpTarget,lpSkill,send,&miss) == 0)
	{
		return 1;
	}

	if((lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER) && gAttack.MissCheckPvP(lpObj,lpTarget,lpSkill,send,0,&miss) == 0)
	{
		return 1;
	}

	int defense = this->GetTargetDefense(lpObj,lpTarget,&effect);

	int damage = this->GetAttackDamage(lpObj,lpTarget,lpSkill,&effect,defense);

	if(miss != 0)
	{
		damage = (damage*30)/100;
	}

	if(action != 0)
	{
		damage = (damage*60)/100;
	}

	for(int n=0;n < MAX_DAMAGE_REDUCTION;n++)
	{
		damage -= (damage*lpTarget->DamageReduction[n])/100;
	}

	if(lpTarget->EffectOption.AddDamageReduction > 0)
	{
		damage -= (damage*lpTarget->EffectOption.AddDamageReduction)/100;
	}

	if((GetTickCount()-lpTarget->ShieldDamageReductionTime) < ((DWORD)(gServerInfo.m_DefenseTimeConstA*1000)))
	{
		damage -= (damage*lpTarget->ShieldDamageReduction)/100;
	}

	gAttack.WingSprite(0,lpTarget,&damage);

	gAttack.HelperSprite(0,lpTarget,&damage);

	int MinDamage = (lpObj->Level+lpObj->MasterLevel)/10;

	MinDamage = ((MinDamage<1)?1:MinDamage);

	damage = ((damage<MinDamage)?MinDamage:damage);

	this->DarkSpiritSprite(lpObj,damage);

	gAttack.DamageSprite(lpTarget,damage);

	#if(GAMESERVER_UPDATE>=602)

	if((GetLargeRand()%100) < gMasterSkillTree.GetMasterSkillValue(lpObj,MASTER_SKILL_ADD_DARK_SPIRIT_DOUBLE_DAMAGE_RATE))
	{
		effect |= 0x40;
		damage += damage;
	}

	#endif

	#pragma endregion

	#pragma region DAMAGE_CONFIG

	if(lpObj->Type == OBJECT_USER)
	{
		if(lpTarget->Type == OBJECT_USER)
		{
			damage = (damage*gServerInfo.m_GeneralDamageRatePvP)/100;

			damage = (damage*gServerInfo.m_DamageRatePvP[lpObj->Class])/100;

			damage = (damage*gServerInfo.m_DamageRateTo[lpObj->Class][lpTarget->Class])/100;

			if(duel != 0)
			{
				damage = (damage*gServerInfo.m_DuelDamageRate)/100;
			}
			else if(gGensSystem.CheckGens(lpObj,lpTarget) != 0)
			{
				damage = (damage*gServerInfo.m_GensDamageRate)/100;
			}
			else if(CA_MAP_RANGE(lpObj->Map) != 0 && CA_MAP_RANGE(lpTarget->Map) != 0)
			{
				damage = (damage*gServerInfo.m_CustomArenaDamageRate)/100;
			}
			else if(CC_MAP_RANGE(lpObj->Map) != 0 && CC_MAP_RANGE(lpTarget->Map) != 0)
			{
				damage = (damage*gServerInfo.m_ChaosCastleDamageRate)/100;
			}
			else if(IT_MAP_RANGE(lpObj->Map) != 0 && IT_MAP_RANGE(lpTarget->Map) != 0)
			{
				damage = (damage*gServerInfo.m_IllusionTempleDamageRate)/100;
			}
			#if(GAMESERVER_TYPE==1)
			else if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE && lpObj->Map == MAP_CASTLE_SIEGE && lpTarget->Map == MAP_CASTLE_SIEGE)
			{
				if(lpObj->CsJoinSide == 0 || lpTarget->CsJoinSide == 0 || lpObj->CsJoinSide != lpTarget->CsJoinSide)
				{
					damage = (damage*gServerInfo.m_CastleSiegeDamageRate1)/100;
				}
				else
				{
					damage = (damage*gServerInfo.m_CastleSiegeDamageRate2)/100;
				}
			}
			#endif
		}
		else
		{
			damage = (damage*gServerInfo.m_GeneralDamageRatePvM)/100;

			damage = (damage*gServerInfo.m_DamageRatePvM[lpObj->Class])/100;
		}
	}

	#pragma endregion

	#pragma region DAMAGE_FINISH

	if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_MONSTER)
	{
		if(lpTarget->Class == 277 || lpTarget->Class == 283) // Castle Gate,Guardian Statue
		{
			if(gEffectManager.CheckEffect(lpObj,EFFECT_BLESS_POTION) != 0)
			{
				damage += (damage*20)/100;
			}

			damage = (damage*gServerInfo.m_CastleSiegeDamageRate3)/100;

			lpObj->AccumulatedDamage = ((lpObj->AccumulatedDamage>100)?0:(lpObj->AccumulatedDamage+damage));
		}
	}

	damage = ((damage<0)?0:damage);

	#pragma endregion

	#pragma region DAMAGE_APPLY

	int ShieldDamage = 0;

	if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
	{
		ShieldDamage = gAttack.GetShieldDamage(lpObj,lpTarget,damage);

		if(lpTarget->Life < (damage-ShieldDamage))
		{
			lpTarget->Life = 0;
		}
		else
		{
			lpTarget->Life -= damage-ShieldDamage;
		}

		if(lpTarget->Shield < ShieldDamage)
		{
			lpTarget->Shield = 0;
		}
		else
		{
			lpTarget->Shield -= ShieldDamage;
		}
	}
	else
	{
		if(lpTarget->Life < damage)
		{
			lpTarget->Life = 0;
		}
		else
		{
			lpTarget->Life -= damage;
		}
	}

	if(lpTarget->Type == OBJECT_MONSTER)
	{
		lpTarget->LastAttackerID = lpObj->Index;

		gObjAddMsgSendDelay(lpTarget,0,lpObj->Index,100,0);

		if(lpTarget->CurrentAI != 0)
		{
			lpTarget->Agro.IncAgro(lpObj->Index,(damage/50));
		}
	}

	#pragma endregion

	#pragma region CHECK_SELF_DEFENSE

	if(damage > 0)
	{
		if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER && lpObj->Index != lpTarget->Index)
		{
			bool CheckSelfDefense = 1;

			if(effect == 4)
			{
				CheckSelfDefense = 0;
			}

			if(gDuel.CheckDuel(lpObj,lpTarget) != 0)
			{
				CheckSelfDefense = 0;
			}

			if(gObjGetRelationShip(lpObj,lpTarget) == 2)
			{
				CheckSelfDefense = 0;
			}

			if(gObjTargetGuildWarCheck(lpObj,lpTarget) != 0)
			{
				CheckSelfDefense = 0;
			}

			if(gGensSystem.CheckGens(lpObj,lpTarget) != 0)
			{
				CheckSelfDefense = 0;
			}

			#if(GAMESERVER_TYPE==1)

			if(lpObj->Map == MAP_CASTLE_SIEGE && gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE && lpObj->CsJoinSide != 0)
			{
				CheckSelfDefense = 0;
			}

			#endif

			if(CA_MAP_RANGE(lpObj->Map) != 0 && CA_MAP_RANGE(lpTarget->Map) != 0)
			{
				CheckSelfDefense = 0;
			}

			if(CC_MAP_RANGE(lpObj->Map) != 0 && CC_MAP_RANGE(lpTarget->Map) != 0)
			{
				CheckSelfDefense = 0;
			}

			if(IT_MAP_RANGE(lpObj->Map) != 0 && IT_MAP_RANGE(lpTarget->Map) != 0)
			{
				CheckSelfDefense = 0;
			}

			if(lpObj->PvP != 0 && lpTarget->PvP != 0)
			{
				CheckSelfDefense = 0;
			}

			if(lpObj->KillAll != 0 && lpTarget->KillAll != 0)
			{
				CheckSelfDefense = 0;
			}

			if(gMapManager.GetMapNonOutlaw(lpObj->Map) != 0)
			{
				CheckSelfDefense = 0;
			}

			if(CheckSelfDefense != 0)
			{
				gObjCheckSelfDefense(lpObj,lpTarget->Index);
			}
		}

		gAttack.ArmorDurabilityDown(lpObj,lpTarget);
	}

	lpObj->Rest = 0;

	#pragma endregion

	#pragma region ATTACK_FINISH

	if(damage > 0)
	{
		gEffectManager.DelEffect(lpTarget,EFFECT_SLEEP);

		if(effect != 4 && lpTarget->Type == OBJECT_USER)
		{
			if((GetLargeRand()%100) < lpTarget->FullDamageReflectRate)
			{
				gObjAddMsgSendDelay(lpTarget,10,lpObj->Index,10,damage);
			}
			else if((lpTarget->DamageReflect+lpTarget->EffectOption.AddDamageReflect) > 0)
			{
				gObjAddMsgSendDelay(lpTarget,10,lpObj->Index,10,((damage*(lpTarget->DamageReflect+lpTarget->EffectOption.AddDamageReflect))/100));
			}

			if((GetLargeRand()%100) < lpTarget->DefensiveFullHPRestoreRate)
			{
				lpTarget->Life = lpTarget->MaxLife+lpTarget->AddLife;
				GCLifeSend(lpTarget->Index,0xFF,(int)lpTarget->Life,lpTarget->Shield);
			}

			if((GetLargeRand()%100) < lpTarget->DefensiveFullSDRestoreRate)
			{
				lpTarget->Shield = lpTarget->MaxShield+lpTarget->AddShield;
				GCLifeSend(lpTarget->Index,0xFF,(int)lpTarget->Life,lpTarget->Shield);
			}

			if((GetLargeRand()%100) < lpTarget->DefensiveFullMPRestoreRate)
			{
				lpTarget->Mana = lpTarget->MaxMana+lpTarget->AddMana;
				GCManaSend(lpTarget->Index,0xFF,(int)lpTarget->Mana,lpTarget->BP);
			}

			if((GetLargeRand()%100) < lpTarget->DefensiveFullBPRestoreRate)
			{
				lpTarget->BP = lpTarget->MaxBP+lpTarget->AddBP;
				GCManaSend(lpTarget->Index,0xFF,(int)lpTarget->Mana,lpTarget->BP);
			}

			if(lpTarget->Inventory[8].IsItem() == 0 || (lpTarget->Inventory[8].m_Index != GET_ITEM(13,2) && lpTarget->Inventory[8].m_Index != GET_ITEM(13,3) && lpTarget->Inventory[8].m_Index != GET_ITEM(13,4) && lpTarget->Inventory[8].m_Index != GET_ITEM(13,37)))
			{
				if((GetLargeRand()%100) < gServerInfo.m_DamageStuckRate[lpTarget->Class])
				{
					flag = 1;
				}
			}
		}

		gObjectManager.CharacterLifeCheck(lpObj,lpTarget,(damage-ShieldDamage),0,flag,effect,((lpSkill==0)?0:lpSkill->m_index),ShieldDamage);
	}
	else
	{
		GCDamageSend(lpObj->Index,lpTarget->Index,0,0,effect,0);
	}

	if(lpObj->Type == OBJECT_USER && lpObj->Life <= 0 && lpObj->CheckLifeTime <= 0)
	{
		lpObj->AttackObj = lpTarget;
		lpObj->AttackerKilled = ((lpTarget->Type==OBJECT_USER)?1:0);
		lpObj->CheckLifeTime = 3;
	}

	#pragma endregion

	return 1;
}

void CDarkSpirit::DarkSpiritSprite(LPOBJ lpObj,int damage) // OK
{
	CItem* lpItem = &lpObj->Inventory[1];

	lpItem->m_DurabilitySmall += (damage*2)/100;

	int MaxDurSmall = (800*gServerInfo.m_PetDurabilityRate)/100;

	MaxDurSmall = (MaxDurSmall*lpObj->PetDurabilityRate)/100;

	if(lpItem->m_DurabilitySmall >= MaxDurSmall)
	{
		lpItem->m_Durability = (((--lpItem->m_Durability)<1)?0:lpItem->m_Durability);
		lpItem->m_DurabilitySmall = 0;

		if(lpItem->CheckDurabilityState() != 0)
		{
			gObjectManager.CharacterCalcAttribute(lpObj->Index);
		}

		gItemManager.GCItemDurSend(lpObj->Index,1,(BYTE)lpItem->m_Durability,0);

		GCPetItemInfoSend(lpObj->Index,0,0,1,lpItem->m_PetItemLevel,lpItem->m_PetItemExp,(BYTE)lpItem->m_Durability);
	}
}

bool CDarkSpirit::MissCheck(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int send,BYTE* miss) // OK
{
	if(this->m_AttackSuccessRate < lpTarget->DefenseSuccessRate)
	{
		(*miss) = 1;

		if((GetLargeRand()%100) >= 5)
		{
			gAttack.MissSend(lpObj,lpTarget,lpSkill,send,0);
			return 0;
		}
	}
	else
	{
		(*miss) = 0;

		if((GetLargeRand()%((this->m_AttackSuccessRate==0)?1:this->m_AttackSuccessRate)) < lpTarget->DefenseSuccessRate)
		{
			gAttack.MissSend(lpObj,lpTarget,lpSkill,send,0);
			return 0;
		}
	}

	return 1;
}

int CDarkSpirit::GetTargetDefense(LPOBJ lpObj,LPOBJ lpTarget,WORD* effect) // OK
{
	int defense = lpTarget->Defense;

	defense += lpTarget->EffectOption.AddDefense;
	defense -= lpTarget->EffectOption.SubDefense;

	if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
	{
		defense += lpTarget->DefensePvP;
	}

	if(lpTarget->MonsterSkillElementOption.CheckDefenseTime() != 0)
	{
		defense += lpTarget->MonsterSkillElementOption.m_SkillElementDefense;
	}

	if(lpTarget->EffectOption.MulDefense > 0)
	{
		defense += (defense*lpTarget->EffectOption.MulDefense)/100;
	}

	if(lpTarget->EffectOption.DivDefense > 0)
	{
		defense -= (defense*lpTarget->EffectOption.DivDefense)/100;
	}

	defense = (defense*50)/100;

	#if(GAMESERVER_UPDATE>=602)

	if((GetLargeRand()%100) < gMasterSkillTree.GetMasterSkillValue(lpObj,MASTER_SKILL_ADD_DARK_SPIRIT_IGNORE_DEFENSE_RATE))
	{
		(*effect) = 1;

		defense = 0;
	}

	#endif

	defense = ((defense<0)?0:defense);

	return defense;
}

int CDarkSpirit::GetAttackDamage(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,WORD* effect,int TargetDefense) // OK
{
	CItem* lpItem = &lpObj->Inventory[0];

	int DamageMin = this->m_AttackDamageMin;
	int DamageMax = this->m_AttackDamageMax;

	if(lpItem->IsItem() != 0 && lpItem->m_IsValidItem != 0 && lpItem->m_MagicDamageRate != 0 && (lpItem->m_Index >= GET_ITEM(2,0) && lpItem->m_Index < GET_ITEM(3,0)))
	{
		int rise = (int)(((lpItem->m_MagicDamageRate/2)+(lpItem->m_Level*2))*lpItem->m_CurrentDurabilityState);
		DamageMin += (DamageMin*rise)/100;
		#if(GAMESERVER_UPDATE>=602)
		DamageMin += gMasterSkillTree.GetMasterSkillValue(lpObj,MASTER_SKILL_ADD_SCEPTER_PET_DAMAGE);
		#endif
		DamageMax += (DamageMax*rise)/100;
		#if(GAMESERVER_UPDATE>=602)
		DamageMax += gMasterSkillTree.GetMasterSkillValue(lpObj,MASTER_SKILL_ADD_SCEPTER_PET_DAMAGE);
		#endif
	}

	int range = (DamageMax-DamageMin);

	range = ((range<1)?1:range);

	int damage = DamageMin+(GetLargeRand()%range);

	#if(GAMESERVER_UPDATE>=602)
	if((GetLargeRand()%100) < (gServerInfo.m_DarkSpiritCriticalDamageRate+gMasterSkillTree.GetMasterSkillValue(lpObj,MASTER_SKILL_ADD_DARK_SPIRIT_CRITICAL_DAMAGE_RATE)))
	#else
	if((GetLargeRand()%100) < (gServerInfo.m_DarkSpiritCriticalDamageRate))
	#endif
	{
		(*effect) = 3;

		damage = DamageMax;
	}

	#if(GAMESERVER_UPDATE>=602)
	if((GetLargeRand()%100) < (gServerInfo.m_DarkSpiritExcellentDamageRate+gMasterSkillTree.GetMasterSkillValue(lpObj,MASTER_SKILL_ADD_DARK_SPIRIT_EXCELLENT_DAMAGE_RATE)))
	#else
	if((GetLargeRand()%100) < (gServerInfo.m_DarkSpiritExcellentDamageRate))
	#endif
	{
		(*effect) = 2;

		damage = (DamageMax*120)/100;
	}

	#if(GAMESERVER_UPDATE>=602)

	damage += gMasterSkillTree.GetMasterSkillValue(lpObj,MASTER_SKILL_ADD_DARK_SPIRIT_DAMAGE);

	#endif

	damage -= TargetDefense;

	damage = ((damage<0)?0:damage);

	return damage;
}

void CDarkSpirit::GetPreviewAttackDamage(LPOBJ lpObj,DWORD* DamageMin,DWORD* DamageMax,DWORD* AttackSpeed,DWORD* AttackSuccessRate) // OK
{
	CItem* lpItem = &lpObj->Inventory[0];

	(*DamageMin) = this->m_AttackDamageMin;
	(*DamageMax) = this->m_AttackDamageMax;

	if(lpItem->IsItem() != 0 && lpItem->m_IsValidItem != 0 && lpItem->m_MagicDamageRate != 0 && (lpItem->m_Index >= GET_ITEM(2,0) && lpItem->m_Index < GET_ITEM(3,0)))
	{
		int rise = (int)(((lpItem->m_MagicDamageRate/2)+(lpItem->m_Level*2))*lpItem->m_CurrentDurabilityState);
		(*DamageMin) += ((*DamageMin)*rise)/100;
		#if(GAMESERVER_UPDATE>=602)
		(*DamageMin) += gMasterSkillTree.GetMasterSkillValue(lpObj,MASTER_SKILL_ADD_SCEPTER_PET_DAMAGE);
		#endif
		(*DamageMax) += ((*DamageMax)*rise)/100;
		#if(GAMESERVER_UPDATE>=602)
		(*DamageMax) += gMasterSkillTree.GetMasterSkillValue(lpObj,MASTER_SKILL_ADD_SCEPTER_PET_DAMAGE);
		#endif
	}

	#if(GAMESERVER_UPDATE>=602)

	(*DamageMin) += gMasterSkillTree.GetMasterSkillValue(lpObj,MASTER_SKILL_ADD_DARK_SPIRIT_DAMAGE);
	(*DamageMin) += gMasterSkillTree.GetMasterSkillValue(lpObj,MASTER_SKILL_ADD_DARK_SPIRIT_DAMAGE);

	#endif

	(*AttackSpeed) = this->m_AttackSpeed;
	(*AttackSuccessRate) = this->m_AttackSuccessRate;
}
