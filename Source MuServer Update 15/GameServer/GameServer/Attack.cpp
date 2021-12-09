// Attack.cpp: implementation of the CAttack class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Attack.h"
#include "BattleSoccerManager.h"
#include "CastleSiege.h"
#include "ChaosCastle.h"
#include "Crywolf.h"
#include "CustomArena.h"
#include "CustomWing.h"
#include "DarkSpirit.h"
#include "Duel.h"
#include "EventKillAll.h"
#include "EventRunAndCatch.h"
#include "EventGvG.h"
#include "EventTvT.h"
#include "EffectManager.h"
#include "GensSystem.h"
#include "Guild.h"
#include "IllusionTemple.h"
#include "Kalima.h"
#include "Map.h"
#include "MapManager.h"
#include "MasterSkillTree.h"
#include "Message.h"
#include "Monster.h"
#include "MuunSystem.h"
#include "Notice.h"
#include "ObjectManager.h"
#include "Party.h"
#include "PentagramSystem.h"
#include "ReiDoMU.h"
#include "ServerInfo.h"
#include "SkillDamage.h"
#include "SkillManager.h"
#include "Util.h"

CAttack gAttack;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAttack::CAttack() // OK
{

}

CAttack::~CAttack() // OK
{

}

bool CAttack::Attack(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,bool send,BYTE flag,int damage,int count,bool combo) // OK
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

	if (lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
	{
		if(lpObj->DisablePvp == 1 || lpTarget->DisablePvp == 1)
		{
			if (gObjCheckEventJoin(lpTarget))
			{
				if (lpObj->DisablePvp == 1)
				{
					gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(772));
					lpObj->DisablePvp = 0;
				}
				if (lpTarget->DisablePvp == 1)
				{
					gNotice.GCNoticeSend(lpTarget->Index,1,0,0,0,0,0,gMessage.GetMessage(772));
					lpTarget->DisablePvp = 0;
				}
			}
		}

		#if(GAMESERVER_UPDATE <= 401)
			if (gDuel.CheckDuel(lpObj,lpTarget) == 1)
			{
					if (lpObj->DisablePvp == 1)
					{
						gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(772));
						lpObj->DisablePvp = 0;
					}
					if (lpTarget->DisablePvp == 1)
					{
						gNotice.GCNoticeSend(lpTarget->Index,1,0,0,0,0,0,gMessage.GetMessage(772));
						lpTarget->DisablePvp = 0;
					}
			}
		#endif

		if(lpObj->DisablePvp == 1)
		{
			return 0;
		}

		if(lpTarget->DisablePvp == 1)
		{
			return 0;
		}
	}

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) != 0 || gMap[lpTarget->Map].CheckAttr(lpTarget->X,lpTarget->Y,1) != 0)
	{
		return 0;
	}

	#if(GAMESERVER_UPDATE>=402)

	if(gDuel.GetDuelArenaBySpectator(lpObj->Index) != 0 || gDuel.GetDuelArenaBySpectator(lpTarget->Index) != 0)
	{
		return 0;
	}

	#endif

	//Event Run And Cacth - Pega Pega
	if (lpTarget->Type == OBJECT_USER && gEventRunAndCatch.m_EventRunAndCatchActive==1)
	{
		gEventRunAndCatch.EventRunAndCatchHit(lpObj->Index,lpTarget->Index);
	}

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

	if(lpTarget->Type == OBJECT_BOTS)
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

	if(lpTarget->SummonIndex == lpObj->Index) //help command fix
	{
		return 0;
	}

	if(lpTarget->Index == lpObj->SummonIndex) //help command fix
	{
		return 0;
	}

	if (OBJECT_RANGE(lpObj->SummonIndex) != 0)
	{
		if(lpTarget->SummonIndex == lpObj->SummonIndex) //help command fix
		{
			return 0;
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

	if(this->CheckPlayerTarget(&gObj[SummonIndex],&gObj[SummonTargetIndex]) == 0)
	{
		return 0;
	}

	#pragma endregion

	#pragma region ATTACK_RETURN

	int skill = ((lpSkill==0)?SKILL_NONE:lpSkill->m_skill);

	if(damage == 0 && skill != SKILL_PLASMA_STORM && this->DecreaseArrow(lpObj) == 0)
	{
		return 0;
	}

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
		this->MissSend(lpObj,lpTarget,lpSkill,send,count);
		return 1;
	}

	if(gEffectManager.CheckEffect(lpTarget,EFFECT_ORDER_OF_PROTECTION) != 0 && lpSkill != 0)
	{
		this->MissSend(lpObj,lpTarget,lpSkill,send,count);
		return 1;
	}

	if(gEffectManager.CheckEffect(lpTarget,EFFECT_PHYSI_DAMAGE_IMMUNITY) != 0 && lpSkill == 0)
	{
		this->MissSend(lpObj,lpTarget,lpSkill,send,count);
		return 1;
	}

	if(gEffectManager.CheckEffect(lpTarget,EFFECT_MAGIC_DAMAGE_IMMUNITY) != 0 && lpSkill != 0)
	{
		this->MissSend(lpObj,lpTarget,lpSkill,send,count);
		return 1;
	}

	if(lpTarget->Type == OBJECT_MONSTER)
	{
		if(lpTarget->Class == 200 && lpSkill == 0)
		{
			gObjMonsterStateProc(lpTarget,6,lpObj->Index,0);
			this->MissSend(lpObj,lpTarget,lpSkill,send,count);
			return 1;
		}

		if(lpTarget->Class == 200 && lpSkill != 0)
		{
			gObjMonsterStateProc(lpTarget,7,lpObj->Index,0);
			this->MissSend(lpObj,lpTarget,lpSkill,send,count);
			return 1;
		}

		if(gEffectManager.CheckEffect(lpTarget,EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY) != 0 && skill == SKILL_NONE)
		{
			this->MissSend(lpObj,lpTarget,lpSkill,send,count);
			return 1;
		}

		if(gEffectManager.CheckEffect(lpTarget,EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY) != 0 && skill != SKILL_NONE)
		{
			this->MissSend(lpObj,lpTarget,lpSkill,send,count);
			return 1;
		}

		if(lpTarget->MonsterSkillElementOption.CheckImmuneTime() != 0)
		{
			if(lpTarget->MonsterSkillElementOption.m_SkillElementImmuneNumber == skill)
			{
				this->MissSend(lpObj,lpTarget,lpSkill,send,count);
				return 1;
			}
		}
	}

	#pragma endregion

	#pragma region DAMAGE_CALC

	flag = 0;

	BYTE miss = 0;

	WORD effect = 0;

	if(damage == 0)
	{
		if((lpObj->Type != OBJECT_USER || lpTarget->Type != OBJECT_USER) && this->MissCheck(lpObj,lpTarget,lpSkill,send,count,&miss) == 0)
		{
			#if(GAMESERVER_UPDATE>=701)
			this->AttackElemental(lpObj,lpTarget,lpSkill,send,flag,damage,count,combo);
			#endif
			return 1;
		}

		if((lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER) && this->MissCheckPvP(lpObj,lpTarget,lpSkill,send,count,&miss) == 0)
		{
			#if(GAMESERVER_UPDATE>=701)
			this->AttackElemental(lpObj,lpTarget,lpSkill,send,flag,damage,count,combo);
			#endif
			return 1;
		}

		int defense = this->GetTargetDefense(lpObj,lpTarget,&effect);

		if(skill == SKILL_PLASMA_STORM)
		{
			damage = this->GetAttackDamageFenrir(lpObj,lpTarget,lpSkill,&effect,defense);
		}
		else if(lpObj->Class == CLASS_SU && (skill == SKILL_SAHAMUTT || skill == SKILL_NEIL || skill == SKILL_GHOST_PHANTOM))
		{
			damage = this->GetAttackDamageCursed(lpObj,lpTarget,lpSkill,&effect,defense);
		}
		else if((lpObj->Class == CLASS_DW || lpObj->Class == CLASS_MG || lpObj->Class == CLASS_SU) && skill != SKILL_NONE && skill != SKILL_FALLING_SLASH && skill != SKILL_LUNGE && skill != SKILL_UPPERCUT && skill != SKILL_CYCLONE && skill != SKILL_SLASH && skill != SKILL_TWISTING_SLASH && skill != SKILL_IMPALE && skill != SKILL_FIRE_SLASH && skill != SKILL_POWER_SLASH && skill != SKILL_SPIRAL_SLASH && skill != SKILL_SWORD_SLASH && skill != SKILL_BLOOD_STORM)
		{
			damage = this->GetAttackDamageWizard(lpObj,lpTarget,lpSkill,&effect,defense);
		}
		else
		{
			damage = this->GetAttackDamage(lpObj,lpTarget,lpSkill,&effect,defense);
		}

		if(damage > 0)
		{
			this->WeaponDurabilityDown(lpObj,lpTarget);
		}

		if(miss != 0)
		{
			damage = (damage*30)/100;
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

		this->WingSprite(lpObj,lpTarget,&damage);

		this->HelperSprite(lpObj,lpTarget,&damage);

		int MinDamage = (lpObj->Level+lpObj->MasterLevel)/10;

		MinDamage = ((MinDamage<1)?1:MinDamage);

		damage = ((damage<MinDamage)?MinDamage:damage);

		this->DamageSprite(lpTarget,damage);

		#if(GAMESERVER_UPDATE>=803)

		gMuunSystem.MuunSprite(lpTarget,damage);

		#endif

		if(skill == SKILL_FALLING_SLASH || skill == SKILL_LUNGE || skill == SKILL_UPPERCUT || skill == SKILL_CYCLONE || skill == SKILL_SLASH || skill == SKILL_TWISTING_SLASH || skill == SKILL_RAGEFUL_BLOW || skill == SKILL_DEATH_STAB || skill == SKILL_CRESCENT_MOON_SLASH || skill == SKILL_STAR_FALL || skill == SKILL_IMPALE || skill == SKILL_FIRE_BREATH || skill == SKILL_ICE_ARROW || skill == SKILL_PENETRATION || skill == SKILL_FIRE_SLASH || skill == SKILL_POWER_SLASH || skill == SKILL_SPIRAL_SLASH || skill == SKILL_FROZEN_STAB || skill == SKILL_SWORD_SLASH || skill == SKILL_CHARGE || skill == SKILL_BLOOD_STORM)
		{
			if(skill != SKILL_IMPALE || lpObj->Inventory[8].m_Index == GET_ITEM(13,2) || lpObj->Inventory[8].m_Index == GET_ITEM(13,3) || lpObj->Inventory[8].m_Index == GET_ITEM(13,37))
			{
				if(lpObj->Class == CLASS_DK)
				{
					damage = (damage*lpObj->DKDamageMultiplierRate)/100;
				}
				else
				{
					damage = (damage*200)/100;
				}
			}
		}
		else if(skill == SKILL_FORCE || skill == SKILL_FIRE_BURST || skill == SKILL_EARTHQUAKE || skill == SKILL_ELECTRIC_SPARK || skill == SKILL_FIRE_BLAST || skill == SKILL_FIRE_SCREAM || skill == SKILL_BIRDS)
		{
			damage = (damage*lpObj->DLDamageMultiplierRate)/100;
		}
		else if(skill == SKILL_PLASMA_STORM)
		{
			damage = (damage*(200+(((lpObj->Level>300)?((lpObj->Level-300)+lpObj->MasterLevel):0)/5)))/100;
		}
		else if(skill == SKILL_CHAIN_LIGHTNING)
		{
			damage = (damage*((count==2)?70:((count==3)?50:100)))/100;
		}
		else if(skill == SKILL_RAKLION_SELUPAN1)
		{
			damage = (damage*200)/100;
		}
		else if(skill == SKILL_RAKLION_SELUPAN2)
		{
			damage = (damage*220)/100;
		}
		else if(skill == SKILL_RAKLION_SELUPAN3)
		{
			damage = (damage*230)/100;
		}
		else if(skill == SKILL_RAKLION_SELUPAN4)
		{
			damage = (damage*250)/100;
		}
		else if(skill == SKILL_LARGE_RING_BLOWER || skill == SKILL_UPPER_BEAST || skill == SKILL_PHOENIX_SHOT)
		{
			damage = (damage*lpObj->RFDamageMultiplierRate[0])/100;
		}
		else if(skill == SKILL_CHAIN_DRIVER)
		{
			damage = (damage+((lpObj->Vitality+lpObj->AddVitality)/gServerInfo.m_RFDamageMultiplierConstB));
			damage = (damage*lpObj->RFDamageMultiplierRate[0])/100;
		}
		else if(skill == SKILL_DARK_SIDE)
		{
			damage = (damage+(((lpObj->Dexterity+lpObj->AddDexterity)/gServerInfo.m_RFDamageMultiplierConstA)+((lpObj->Energy+lpObj->AddEnergy)/gServerInfo.m_RFDamageMultiplierConstC)));
			damage = (damage*lpObj->RFDamageMultiplierRate[2])/100;
		}
		else if(skill == SKILL_DRAGON_LORE)
		{
			damage = (damage+((lpObj->Energy+lpObj->AddEnergy)/gServerInfo.m_RFDamageMultiplierConstC));
			damage = (damage*lpObj->RFDamageMultiplierRate[1])/100;
		}
		else if(skill == SKILL_DRAGON_SLAYER)
		{
			damage = (damage*lpObj->RFDamageMultiplierRate[1])/100;
			damage = ((lpTarget->Type==OBJECT_USER)?((damage+100)*3):damage);
		}
		else if(skill == SKILL_CHARGE)
		{
			damage = (damage*(150+lpObj->RFDamageMultiplierRate[0]))/100;
		}

		//Custom Skill Damage System
		damage = (damage*gSkillDamage.GetSkillDamage(lpObj,skill,lpTarget->Type))/100;

		if(count > 0 && (skill == SKILL_UPPER_BEAST || skill == SKILL_DARK_SIDE))
		{
			effect |= (((count%2)==0)?0x20:0x10);
		}

		if(count > 0 && (skill == SKILL_LARGE_RING_BLOWER || skill == SKILL_CHAIN_DRIVER || skill == SKILL_DRAGON_LORE || skill == SKILL_PHOENIX_SHOT))
		{
			effect |= (((count%4)==0)?0x20:0x10);
		}

		if((GetLargeRand()%100) < ((lpObj->DoubleDamageRate)-lpTarget->ResistDoubleDamageRate))
		{
			effect |= 0x40;
			damage += damage;
		}

		if((GetLargeRand()%100) < lpObj->TripleDamageRate) 
		{
			effect |= 0x100;
			damage += damage+damage;
		}

		if(combo != 0)
		{
			effect |= 0x80;
			damage += damage;
			damage += (((lpObj->Strength+lpObj->AddStrength)+(lpObj->Dexterity+lpObj->AddDexterity)+(lpObj->Energy+lpObj->AddEnergy))/gServerInfo.m_ComboDamageConstA)*gServerInfo.m_ComboDamageConstB;
			#if(GAMESERVER_UPDATE>=602)
			damage += (damage*gMasterSkillTree.GetMasterSkillValue(lpObj,MASTER_SKILL_ADD_COMBO_DAMAGE))/100;
			#endif
			skill = SKILL_COMBO;
		}
	}
	else
	{
		if(skill != SKILL_EXPLOSION)
		{
			effect = 0x04;
			damage = (damage*((lpObj->Type==OBJECT_USER&&lpTarget->Type==OBJECT_USER)?gServerInfo.m_ReflectDamageRatePvP:gServerInfo.m_ReflectDamageRatePvM))/100;
		}
	}

	#pragma endregion

	#pragma region DAMAGE_CONFIG

	if(lpObj->Type == OBJECT_USER)
	{
		if(lpTarget->Type == OBJECT_USER)
		{
			damage = (damage*gServerInfo.m_GeneralDamageRatePvP)/100;

			damage = (damage*gServerInfo.m_DamageRatePvP[lpObj->Class])/100;

			damage = (damage*gServerInfo.m_DamageRateTo[lpObj->Class][lpTarget->Class])/100;

			if(gDuel.CheckDuel(lpObj,lpTarget) != 0)
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

	#pragma region APPLY_EFFECT

	if(lpObj->Type == OBJECT_USER && effect != 4)
	{
		if((GetLargeRand()%100) < lpObj->OffensiveFullHPRestoreRate)
		{
			lpObj->Life = lpObj->MaxLife+lpObj->AddLife;
			GCLifeSend(lpObj->Index,0xFF,(int)lpObj->Life,lpObj->Shield);
		}

		if((GetLargeRand()%100) < lpObj->OffensiveFullSDRestoreRate)
		{
			lpObj->Shield = lpObj->MaxShield+lpObj->AddShield;
			GCLifeSend(lpObj->Index,0xFF,(int)lpObj->Life,lpObj->Shield);
		}

		if((GetLargeRand()%100) < lpObj->OffensiveFullMPRestoreRate)
		{
			lpObj->Mana = lpObj->MaxMana+lpObj->AddMana;
			GCManaSend(lpObj->Index,0xFF,(int)lpObj->Mana,lpObj->BP);
		}

		if((GetLargeRand()%100) < lpObj->OffensiveFullBPRestoreRate)
		{
			lpObj->BP = lpObj->MaxBP+lpObj->AddBP;
			GCManaSend(lpObj->Index,0xFF,(int)lpObj->Mana,lpObj->BP);
		}

		#if(GAMESERVER_UPDATE>=602)

		if(lpObj->Inventory[0].m_Index >= GET_ITEM(2,0) && lpObj->Inventory[0].m_Index < GET_ITEM(3,0))
		{
			if((GetLargeRand()%100) < (gMasterSkillTree.GetMasterSkillValue(lpObj,MASTER_SKILL_ADD_MACE_MASTERY)-lpTarget->ResistStunRate))
			{
				if(gEffectManager.CheckEffect(lpTarget,EFFECT_IRON_DEFENSE) == 0 && gEffectManager.CheckEffect(lpTarget,EFFECT_IRON_DEFENSE_IMPROVED) == 0)
				{
					gEffectManager.AddEffect(lpTarget,0,EFFECT_STERN,2,0,0,0,0);
				}
			}
		}

		if(lpObj->Inventory[1].m_Index >= GET_ITEM(2,0) && lpObj->Inventory[1].m_Index < GET_ITEM(3,0))
		{
			if((GetLargeRand()%100) < (gMasterSkillTree.GetMasterSkillValue(lpObj,MASTER_SKILL_ADD_MACE_MASTERY)-lpTarget->ResistStunRate))
			{
				if(gEffectManager.CheckEffect(lpTarget,EFFECT_IRON_DEFENSE) == 0 && gEffectManager.CheckEffect(lpTarget,EFFECT_IRON_DEFENSE_IMPROVED) == 0)
				{
					gEffectManager.AddEffect(lpTarget,0,EFFECT_STERN,2,0,0,0,0);
				}
			}
		}

		if(gEffectManager.CheckEffect(lpObj,EFFECT_BLOOD_HOWLING) != 0 || gEffectManager.CheckEffect(lpObj,EFFECT_BLOOD_HOWLING_IMPROVED) != 0)
		{
			int rate = gServerInfo.m_BloodHowlingConstA;

			rate += gMasterSkillTree.GetMasterSkillValue(lpObj,MASTER_SKILL_ADD_BLOOD_HOWLING_IMPROVED);

			if((GetLargeRand()%100) < rate)
			{
				if(gEffectManager.CheckEffect(lpTarget,EFFECT_IRON_DEFENSE) == 0 && gEffectManager.CheckEffect(lpTarget,EFFECT_IRON_DEFENSE_IMPROVED) == 0)
				{
					int damage = gServerInfo.m_BloodHowlingConstB;

					gEffectManager.AddEffect(lpTarget,0,EFFECT_DEATH_STAB_ENHANCED,10,lpObj->Index,1,SET_NUMBERHW(damage),SET_NUMBERLW(damage));
				}
			}
		}

		#endif
	}

	if(lpSkill != 0 && count <= 1)
	{
		if(this->ApplySkillEffect(lpObj,lpTarget,lpSkill,damage) == 0)
		{
			if(send != 0)
			{
				gSkillManager.GCSkillAttackSend(lpObj,lpSkill->m_index,lpTarget->Index,0);
			}
		}
		else
		{
			if(send != 0)
			{
				gSkillManager.GCSkillAttackSend(lpObj,lpSkill->m_index,lpTarget->Index,1);
			}
		}
	}

	#pragma endregion

	#pragma region DAMAGE_APPLY

	int ShieldDamage = 0;

	if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
	{
		ShieldDamage = this->GetShieldDamage(lpObj,lpTarget,damage);

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

			if (gTvTEvent.CheckPlayerTarget(lpObj) && gTvTEvent.CheckPlayerTarget(lpTarget))
			{
				CheckSelfDefense = 0;
			}

			if (gGvGEvent.CheckPlayerTarget(lpObj) && gGvGEvent.CheckPlayerTarget(lpTarget))
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

		this->ArmorDurabilityDown(lpObj,lpTarget);
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

		}
		else if(effect == 4 && lpTarget->Type == OBJECT_USER)
		{
			if(lpTarget->Inventory[8].IsItem() == 0 || (lpTarget->Inventory[8].m_Index != GET_ITEM(13,2) && lpTarget->Inventory[8].m_Index != GET_ITEM(13,3) && lpTarget->Inventory[8].m_Index != GET_ITEM(13,4) && lpTarget->Inventory[8].m_Index != GET_ITEM(13,37)))
			{
				if((GetLargeRand()%100) < gServerInfo.m_DamageStuckRate[lpTarget->Class])
				{
					flag = 1;
				}
			}
		}

		gObjectManager.CharacterLifeCheck(lpObj,lpTarget,(damage-ShieldDamage),0,flag,effect,((lpSkill==0)?0:lpSkill->m_index),ShieldDamage);

		#if(GAMESERVER_UPDATE>=701)

		this->AttackElemental(lpObj,lpTarget,lpSkill,send,flag,damage,count,combo);

		#endif
	}
	else
	{
		GCDamageSend(lpObj->Index,lpTarget->Index,0,0,effect,0);

		#if(GAMESERVER_UPDATE>=701)

		this->AttackElemental(lpObj,lpTarget,lpSkill,send,flag,damage,count,combo);

		#endif
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

bool CAttack::AttackElemental(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,bool send,BYTE flag,int damage,int count,bool combo) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	#pragma region ATTACK_CHECK

	if(lpObj->ElementalAttribute == 0)
	{
		return 0;
	}

	if(lpObj->Type == OBJECT_USER && lpSkill == 0)
	{
		return 0;
	}

	if(lpObj->Type == OBJECT_USER && (lpObj->Inventory[236].IsItem() == 0 || lpObj->Inventory[236].IsPentagramItem() == 0 || lpObj->Inventory[236].m_IsValidItem == 0))
	{
		return 0;
	}

	#pragma endregion

	#pragma region DAMAGE_CALC

	flag = 0;

	BYTE miss = 0;

	WORD effect = lpObj->ElementalAttribute;

	if(this->MissCheckElemental(lpObj,lpTarget,lpSkill,send,count,&miss) == 0)
	{
		return 1;
	}

	int defense = this->GetTargetElementalDefense(lpObj,lpTarget,&effect);

	damage = this->GetAttackDamageElemental(lpObj,lpTarget,lpSkill,&effect,damage,defense);

	if(miss != 0)
	{
		damage = (damage*30)/100;
	}

	if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
	{
		damage -= (damage*lpTarget->PentagramJewelOption.AddElementalDamageReductionPvP)/100;
	}

	if(lpObj->Type != OBJECT_USER && lpTarget->Type == OBJECT_USER)
	{
		damage -= (damage*lpTarget->PentagramJewelOption.AddElementalDamageReductionPvM)/100;
	}

	int MinDamage = (lpObj->Level+lpObj->MasterLevel)/10;

	MinDamage = ((MinDamage<1)?1:MinDamage);

	damage = ((damage<MinDamage)?MinDamage:damage);

	#pragma endregion

	#pragma region DAMAGE_CONFIG

	if(lpObj->Type == OBJECT_USER)
	{
		if(lpTarget->Type == OBJECT_USER)
		{
			damage = (damage*gServerInfo.m_GeneralElementalDamageRatePvP)/100;

			damage = (damage*gServerInfo.m_ElementalDamageRatePvP[lpObj->Class])/100;

			damage = (damage*gServerInfo.m_ElementalDamageRateTo[lpObj->Class][lpTarget->Class])/100;

			if(gDuel.CheckDuel(lpObj,lpTarget) != 0)
			{
				damage = (damage*gServerInfo.m_DuelElementalDamageRate)/100;
			}
			else if(gGensSystem.CheckGens(lpObj,lpTarget) != 0)
			{
				damage = (damage*gServerInfo.m_GensElementalDamageRate)/100;
			}
			else if(CA_MAP_RANGE(lpObj->Map) != 0 && CA_MAP_RANGE(lpTarget->Map) != 0)
			{
				damage = (damage*gServerInfo.m_CustomArenaElementalDamageRate)/100;
			}
			else if(CC_MAP_RANGE(lpObj->Map) != 0 && CC_MAP_RANGE(lpTarget->Map) != 0)
			{
				damage = (damage*gServerInfo.m_ChaosCastleElementalDamageRate)/100;
			}
			else if(IT_MAP_RANGE(lpObj->Map) != 0 && IT_MAP_RANGE(lpTarget->Map) != 0)
			{
				damage = (damage*gServerInfo.m_IllusionTempleElementalDamageRate)/100;
			}
			#if(GAMESERVER_TYPE==1)
			else if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE && lpObj->Map == MAP_CASTLE_SIEGE && lpTarget->Map == MAP_CASTLE_SIEGE)
			{
				if(lpObj->CsJoinSide == 0 || lpTarget->CsJoinSide == 0 || lpObj->CsJoinSide != lpTarget->CsJoinSide)
				{
					damage = (damage*gServerInfo.m_CastleSiegeElementalDamageRate1)/100;
				}
				else
				{
					damage = (damage*gServerInfo.m_CastleSiegeElementalDamageRate2)/100;
				}
			}
			#endif
		}
		else
		{
			damage = (damage*gServerInfo.m_GeneralElementalDamageRatePvM)/100;

			damage = (damage*gServerInfo.m_ElementalDamageRatePvM[lpObj->Class])/100;
		}
	}

	#pragma endregion

	#pragma region APPLY_EFFECT

	if(lpObj->Type == OBJECT_USER)
	{
		if((GetLargeRand()%100) < lpObj->PentagramJewelOption.AddElementalSlowRate)
		{
			gEffectManager.AddEffect(lpTarget,0,EFFECT_PENTAGRAM_JEWEL_SLOW,20,0,0,0,0);
		}

		if((GetLargeRand()%100) < lpObj->PentagramJewelOption.AddElementalDebuffRate)
		{
			switch(lpObj->ElementalAttribute)
			{
				case ELEMENTAL_ATTRIBUTE_FIRE:
					gEffectManager.AddEffect(lpTarget,0,EFFECT_PENTAGRAM_JEWEL_HALF_SD,5,0,0,0,0);
					break;
				case ELEMENTAL_ATTRIBUTE_WATER:
					gEffectManager.AddEffect(lpTarget,0,EFFECT_PENTAGRAM_JEWEL_HALF_MP,5,0,0,0,0);
					break;
				case ELEMENTAL_ATTRIBUTE_EARTH:
					gEffectManager.AddEffect(lpTarget,0,EFFECT_PENTAGRAM_JEWEL_HALF_SPEED,5,0,0,0,0);
					break;
				case ELEMENTAL_ATTRIBUTE_WIND:
					gEffectManager.AddEffect(lpTarget,0,EFFECT_PENTAGRAM_JEWEL_HALF_HP,5,0,0,0,0);
					break;
				case ELEMENTAL_ATTRIBUTE_DARK:
					gEffectManager.AddEffect(lpTarget,0,EFFECT_PENTAGRAM_JEWEL_STUN,5,0,0,0,0);
					break;
			}
		}
	}

	#pragma endregion

	#pragma region DAMAGE_APPLY

	int ShieldDamage = 0;

	if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
	{
		ShieldDamage = this->GetShieldDamage(lpObj,lpTarget,damage);

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

		GCLifeSend(lpTarget->Index,0xFF,(int)lpTarget->Life,lpTarget->Shield);
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

	#pragma endregion

	#pragma region ATTACK_FINISH

	if(damage > 0)
	{
		gObjectManager.CharacterLifeCheck(lpObj,lpTarget,(damage-ShieldDamage),4,flag,effect,((lpSkill==0)?0:lpSkill->m_index),ShieldDamage);
	}
	else
	{
		GCElementalDamageSend(lpObj->Index,lpTarget->Index,(BYTE)effect,0);
	}

	#pragma endregion

	return 1;

	#else

	return 0;

	#endif
}

bool CAttack::DecreaseArrow(LPOBJ lpObj) // OK
{
	if(lpObj->Type != OBJECT_USER || lpObj->Class != CLASS_FE)
	{
		return 1;
	}

	if(gEffectManager.CheckEffect(lpObj,EFFECT_INFINITY_ARROW) != 0 || gEffectManager.CheckEffect(lpObj,EFFECT_INFINITY_ARROW_IMPROVED) != 0)
	{
		return 1;
	}

	if(lpObj->Inventory[0].m_Index >= GET_ITEM(4,0) && lpObj->Inventory[0].m_Index < GET_ITEM(5,0) && lpObj->Inventory[0].m_Index != GET_ITEM(4,15) && lpObj->Inventory[0].m_Slot == 0)
	{
		if(lpObj->Inventory[1].m_Index != GET_ITEM(4,7) || lpObj->Inventory[1].m_Durability < 1)
		{
			return 0;
		}
		else
		{
			gItemManager.DecreaseItemDur(lpObj,1,1);
		}
	}

	if(lpObj->Inventory[1].m_Index >= GET_ITEM(4,0) && lpObj->Inventory[1].m_Index < GET_ITEM(5,0) && lpObj->Inventory[1].m_Index != GET_ITEM(4,7) && lpObj->Inventory[1].m_Slot == 1)
	{
		if(lpObj->Inventory[0].m_Index != GET_ITEM(4,15) || lpObj->Inventory[0].m_Durability < 1)
		{
			return 0;
		}
		else
		{
			gItemManager.DecreaseItemDur(lpObj,0,1);
		}
	}

	return 1;
}

void CAttack::WingSprite(LPOBJ lpObj,LPOBJ lpTarget,int* damage) // OK
{
	if(lpObj != 0 && lpObj->Type == OBJECT_USER)
	{
		CItem* lpItem = &lpObj->Inventory[7];

		if(lpItem->IsItem() != 0 && lpItem->m_Durability > 0)
		{
			if(lpObj->Class == CLASS_DW || lpObj->Class == CLASS_FE || lpObj->Class == CLASS_SU)
			{
				lpObj->Life -= 1;
			}
			else
			{
				lpObj->Life -= 3;
			}

			GCLifeSend(lpObj->Index,0xFF,(int)lpObj->Life,lpObj->Shield);

			if((lpItem->m_Index >= GET_ITEM(12,0) && lpItem->m_Index <= GET_ITEM(12,2)) || lpItem->m_Index == GET_ITEM(12,41)) // 1st wing
			{
				(*damage) = ((*damage)*(112+(lpItem->m_Level*2)))/100;
			}
			else if((lpItem->m_Index >= GET_ITEM(12,3) && lpItem->m_Index <= GET_ITEM(12,6)) || lpItem->m_Index == GET_ITEM(12,42)) // 2sd wing
			{
				(*damage) = ((*damage)*(132+(lpItem->m_Level*1)))/100;
			}
			else if((lpItem->m_Index >= GET_ITEM(12,36) && lpItem->m_Index <= GET_ITEM(12,40)) || lpItem->m_Index == GET_ITEM(12,43) || lpItem->m_Index == GET_ITEM(12,50)) // 3rd wing
			{
				(*damage) = ((*damage)*(139+(lpItem->m_Level*2)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(12,49)) // Cloak of Fighter
			{
				(*damage) = ((*damage)*(120+(lpItem->m_Level*2)))/100;
			}
			else if(lpItem->m_Index >= GET_ITEM(12,130) && lpItem->m_Index <= GET_ITEM(12,135)) // Mini Wings
			{
				(*damage) = ((*damage)*(112+(lpItem->m_Level*2)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(12,262)) // Cloak of Death
			{
				(*damage) = ((*damage)*(121+(lpItem->m_Level*1)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(12,263)) // Wings of Chaos
			{
				(*damage) = ((*damage)*(133+(lpItem->m_Level*1)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(12,264)) // Wings of Magic
			{
				(*damage) = ((*damage)*(135+(lpItem->m_Level*1)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(12,265)) // Wings of Life
			{
				(*damage) = ((*damage)*(135+(lpItem->m_Level*1)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(12,266)) // Wings of Conqueror
			{
				(*damage) = ((*damage)*(171+(lpItem->m_Level*0)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(12,267)) // Wings of Angel and Devil
			{
				(*damage) = ((*damage)*(160+(lpItem->m_Level*1)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(13,30)) // Cloak of Lord
			{
				(*damage) = ((*damage)*(120+(lpItem->m_Level*2)))/100;
			}
			else if(gCustomWing.CheckCustomWingByItem(lpItem->m_Index) != 0)
			{
				(*damage) = ((*damage)*gCustomWing.GetCustomWingIncDamage(lpItem->m_Index,lpItem->m_Level))/100;
			}
		}
	}

	if(lpTarget != 0 && lpTarget->Type == OBJECT_USER)
	{
		CItem* lpItem = &lpTarget->Inventory[7];

		if(lpItem->IsItem() != 0 && lpItem->m_Durability > 0)
		{
			if((lpItem->m_Index >= GET_ITEM(12,0) && lpItem->m_Index <= GET_ITEM(12,2)) || lpItem->m_Index == GET_ITEM(12,41)) // 1st wing
			{
				(*damage) = ((*damage)*(88-(lpItem->m_Level*2)))/100;
			}
			else if((lpItem->m_Index >= GET_ITEM(12,3) && lpItem->m_Index <= GET_ITEM(12,6)) || lpItem->m_Index == GET_ITEM(12,42)) // 2sd wing
			{
				(*damage) = ((*damage)*(75-(lpItem->m_Level*2)))/100;
			}
			else if((lpItem->m_Index >= GET_ITEM(12,36) && lpItem->m_Index <= GET_ITEM(12,39)) || lpItem->m_Index == GET_ITEM(12,43) || lpItem->m_Index == GET_ITEM(12,50)) // 3rd wing
			{
				(*damage) = ((*damage)*(61-(lpItem->m_Level*2)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(12,40)) // Mantle of Monarch
			{
				(*damage) = ((*damage)*(76-(lpItem->m_Level*2)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(12,49)) // Cloak of Fighter
			{
				(*damage) = ((*damage)*(90-(lpItem->m_Level*2)))/100;
			}
			else if(lpItem->m_Index >= GET_ITEM(12,130) && lpItem->m_Index <= GET_ITEM(12,135)) // Mini Wings
			{
				(*damage) = ((*damage)*(88-(lpItem->m_Level*2)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(12,262)) // Cloak of Death
			{
				(*damage) = ((*damage)*(87-(lpItem->m_Level*2)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(12,263)) // Wings of Chaos
			{
				(*damage) = ((*damage)*(70-(lpItem->m_Level*2)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(12,264)) // Wings of Magic
			{
				(*damage) = ((*damage)*(71-(lpItem->m_Level*2)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(12,265)) // Wings of Life
			{
				(*damage) = ((*damage)*(71-(lpItem->m_Level*2)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(12,266)) // Wings of Conqueror
			{
				(*damage) = ((*damage)*(29-(lpItem->m_Level*0)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(12,267)) // Wings of Angel and Devil
			{
				(*damage) = ((*damage)*(40-(lpItem->m_Level*1)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(13,30)) // Cloak of Lord
			{
				(*damage) = ((*damage)*(90-(lpItem->m_Level*1)))/100;
			}
			else if(gCustomWing.CheckCustomWingByItem(lpItem->m_Index) != 0)
			{
				(*damage) = ((*damage)*gCustomWing.GetCustomWingDecDamage(lpItem->m_Index,lpItem->m_Level))/100;
			}
		}
	}
}

void CAttack::HelperSprite(LPOBJ lpObj,LPOBJ lpTarget,int* damage) // OK
{
	if(lpObj != 0 && lpObj->Type == OBJECT_USER)
	{
		CItem* lpItem = &lpObj->Inventory[8];

		if(lpItem->IsItem() != 0 && lpItem->m_Durability > 0)
		{
			if(lpItem->m_Index == GET_ITEM(13,1)) // Satan
			{
				lpObj->Life -= 3;

				GCLifeSend(lpObj->Index,0xFF,(int)lpObj->Life,lpObj->Shield);

				(*damage) = ((*damage)*(100+gServerInfo.m_SatanIncDamageConstA))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(13,3)) // Dinorant
			{
				lpObj->Life -= 1;

				GCLifeSend(lpObj->Index,0xFF,(int)lpObj->Life,lpObj->Shield);

				(*damage) = ((*damage)*(100+gServerInfo.m_DinorantIncDamageConstA))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(13,37)) // Fenrir
			{
				if((lpItem->m_NewOption & 1) != 0)
				{
					(*damage) = ((*damage)*(100+gServerInfo.m_BlackFenrirIncDamageConstA))/100;
				}
			}
			else if(lpItem->m_Index == GET_ITEM(13,64)) // Demon
			{
				lpObj->Life -= 4;

				GCLifeSend(lpObj->Index,0xFF,(int)lpObj->Life,lpObj->Shield);

				(*damage) = ((*damage)*(100+gServerInfo.m_DemonIncDamageConstA))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(13,123)) // Skeleton
			{
				lpObj->Life -= 2;

				GCLifeSend(lpObj->Index,0xFF,(int)lpObj->Life,lpObj->Shield);

				(*damage) = ((*damage)*(100+gServerInfo.m_SkeletonIncDamageConstA))/100;
			}
		}
	}

	if(lpTarget != 0 && lpTarget->Type == OBJECT_USER)
	{
		CItem* lpItem = &lpTarget->Inventory[8];

		if(lpItem->IsItem() != 0 && lpItem->m_Durability > 0)
		{
			if(lpItem->m_Index == GET_ITEM(13,0)) // Angel
			{
				(*damage) = ((*damage)*(100-gServerInfo.m_AngelDecDamageConstA))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(13,3)) // Dinorant
			{
				(*damage) = ((*damage)*(100-gServerInfo.m_DinorantDecDamageConstA-(((lpItem->m_NewOption & 1)==0)?0:gServerInfo.m_DinorantDecDamageConstB)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(13,4)) // Dark Horse
			{
				(*damage) = ((*damage)*(100-((gServerInfo.m_DarkHorseDecDamageConstA+lpItem->m_PetItemLevel)/gServerInfo.m_DarkHorseDecDamageConstB)))/100;
			}
			else if(lpItem->m_Index == GET_ITEM(13,37)) // Fenrir
			{
				if((lpItem->m_NewOption & 2) != 0)
				{
					(*damage) = ((*damage)*(100-gServerInfo.m_BlueFenrirDecDamageConstA))/100;
				}
			}
			else if(lpItem->m_Index == GET_ITEM(13,65)) // Maria
			{
				(*damage) = ((*damage)*(100-gServerInfo.m_MariaDecDamageConstA))/100;
			}
		}
	}
}

void CAttack::DamageSprite(LPOBJ lpObj,int damage) // OK
{
	if(lpObj->Type != OBJECT_USER)
	{
		return;
	}

	CItem* lpItem = &lpObj->Inventory[8];

	if(lpItem->IsItem() == 0 || lpItem->m_IsPeriodicItem != 0)
	{
		return;
	}

	float DurabilityValue = (1.0f/gServerInfo.m_GuardianDurabilityRate)*100;

	DurabilityValue = (DurabilityValue/lpObj->GuardianDurabilityRate)*100;

	if(lpItem->m_Index == GET_ITEM(13,0)) // Angel
	{
		lpItem->m_Durability -= (damage*(3.0f*DurabilityValue))/100;
	}
	else if(lpItem->m_Index == GET_ITEM(13,1)) // Satan
	{
		lpItem->m_Durability -= (damage*(2.0f*DurabilityValue))/100;
	}
	else if(lpItem->m_Index == GET_ITEM(13,2)) // Uniria
	{
		lpItem->m_Durability -= (damage*(1.0f*DurabilityValue))/100;
	}
	else if(lpItem->m_Index == GET_ITEM(13,3)) // Dinorant
	{
		lpItem->m_Durability -= (damage*(1.0f*DurabilityValue))/100;
	}
	else if(lpItem->m_Index == GET_ITEM(13,4)) // Dark Horse
	{
		if(this->DarkHorseSprite(lpObj,damage) == 0)
		{
			return;
		}
	}
	else if(lpItem->m_Index == GET_ITEM(13,37)) // Fenrir
	{
		if(this->FenrirSprite(lpObj,damage) == 0)
		{
			return;
		}
	}
	else if(lpItem->m_Index == GET_ITEM(13,64)) // Demon
	{
		lpItem->m_Durability -= (damage*(1.0f*DurabilityValue))/100;
	}
	else if(lpItem->m_Index == GET_ITEM(13,65)) // Maria
	{
		lpItem->m_Durability -= (damage*(2.0f*DurabilityValue))/100;
	}
	else if(lpItem->m_Index == GET_ITEM(13,67)) // Rudolf
	{
		lpItem->m_Durability -= (damage*(1.0f*DurabilityValue))/100;
	}
	else if(lpItem->m_Index == GET_ITEM(13,80)) // Panda
	{
		lpItem->m_Durability -= (damage*(1.0f*DurabilityValue))/100;
	}
	else if(lpItem->m_Index == GET_ITEM(13,106)) // Unicorn
	{
		lpItem->m_Durability -= (damage*(1.0f*DurabilityValue))/100;
	}
	else if(lpItem->m_Index == GET_ITEM(13,123)) // Skeleton
	{
		lpItem->m_Durability -= (damage*(1.0f*DurabilityValue))/100;
	}
	else
	{
		return;
	}

	gItemManager.GCItemDurSend(lpObj->Index,8,(BYTE)lpItem->m_Durability,0);

	if(lpItem->m_Durability < 1)
	{
		if(lpObj->Map == MAP_ICARUS && lpObj->Inventory[7].IsItem() == 0 && (lpItem->m_Index == GET_ITEM(13,3) || lpItem->m_Index == GET_ITEM(13,37)))
		{
			gObjMoveGate(lpObj->Index,22);
		}

		gItemManager.InventoryDelItem(lpObj->Index,8);

		gItemManager.GCItemDeleteSend(lpObj->Index,8,0);

		gObjectManager.CharacterMakePreviewCharSet(lpObj->Index);

		gItemManager.GCItemChangeSend(lpObj->Index,8);
	}
}

bool CAttack::DarkHorseSprite(LPOBJ lpObj,int damage) // OK
{
	CItem* lpItem = &lpObj->Inventory[8];

	if(lpItem->m_Durability < 1)
	{
		return 0;
	}

	lpItem->m_DurabilitySmall += ((damage*2)/100)+1;

	int MaxSmallDur = (1500*gServerInfo.m_PetDurabilityRate)/100;

	MaxSmallDur = (MaxSmallDur*lpObj->PetDurabilityRate)/100;

	if(lpItem->m_DurabilitySmall > MaxSmallDur)
	{
		lpItem->m_Durability = (((--lpItem->m_Durability)<1)?0:lpItem->m_Durability);
		lpItem->m_DurabilitySmall = 0;

		if(lpItem->CheckDurabilityState() != 0)
		{
			gObjectManager.CharacterCalcAttribute(lpObj->Index);
		}

		gItemManager.GCItemDurSend(lpObj->Index,8,(BYTE)lpItem->m_Durability,0);

		GCPetItemInfoSend(lpObj->Index,1,0,8,lpItem->m_PetItemLevel,lpItem->m_PetItemExp,(BYTE)lpItem->m_Durability);
	}

	return 0;
}

bool CAttack::FenrirSprite(LPOBJ lpObj,int damage) // OK
{
	CItem* lpItem = &lpObj->Inventory[8];

	if(lpItem->m_Durability < 1)
	{
		return 0;
	}

	lpItem->m_DurabilitySmall += ((damage*2)/100)+1;

	int MaxSmallDur = (200*gServerInfo.m_GuardianDurabilityRate)/100;

	MaxSmallDur = (MaxSmallDur*lpObj->GuardianDurabilityRate)/100;

	if(lpItem->m_DurabilitySmall > MaxSmallDur)
	{
		lpItem->m_Durability = (((--lpItem->m_Durability)<1)?0:lpItem->m_Durability);
		lpItem->m_DurabilitySmall = 0;

		if(lpItem->CheckDurabilityState() != 0)
		{
			gObjectManager.CharacterCalcAttribute(lpObj->Index);
		}

		gItemManager.GCItemDurSend(lpObj->Index,8,(BYTE)lpItem->m_Durability,0);
	}

	return 1;
}

void CAttack::WeaponDurabilityDown(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if(lpObj->Type != OBJECT_USER)
	{
		return;
	}

	if(gEffectManager.GetEffect(lpObj,EFFECT_TALISMAN_OF_PROTECTION) != 0)
	{
		return;
	}

	for(int n=0;n < 2;n++)
	{
		if(lpObj->Inventory[n].IsItem() != 0)
		{
			bool result = 0;

			switch((lpObj->Inventory[n].m_Index/MAX_ITEM_TYPE))
			{
				case 0:
					result = lpObj->Inventory[n].WeaponDurabilityDown(lpObj->Index,lpTarget->Defense,0);
					break;
				case 1:
					result = lpObj->Inventory[n].WeaponDurabilityDown(lpObj->Index,lpTarget->Defense,0);
					break;
				case 2:
					result = lpObj->Inventory[n].WeaponDurabilityDown(lpObj->Index,lpTarget->Defense,0);
					break;
				case 3:
					result = lpObj->Inventory[n].WeaponDurabilityDown(lpObj->Index,lpTarget->Defense,0);
					break;
				case 4:
					result = lpObj->Inventory[n].WeaponDurabilityDown(lpObj->Index,lpTarget->Defense,1);
					break;
				case 5:
					result = lpObj->Inventory[n].WeaponDurabilityDown(lpObj->Index,lpTarget->Defense,((lpObj->Inventory[n].m_Slot==0)?2:3));
					break;
			}

			if(result != 0)
			{
				gItemManager.GCItemDurSend(lpObj->Index,n,(BYTE)lpObj->Inventory[n].m_Durability,0);
			}
		}
	}
}

void CAttack::ArmorDurabilityDown(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if(lpTarget->Type != OBJECT_USER)
	{
		return;
	}

	if(gEffectManager.GetEffect(lpTarget,EFFECT_TALISMAN_OF_PROTECTION) != 0)
	{
		return;
	}

	int slot = 1+(GetLargeRand()%6);

	if(lpTarget->Inventory[slot].IsItem() != 0)
	{
		if(slot != 1 || (lpTarget->Inventory[slot].m_Index >= GET_ITEM(6,0) && lpTarget->Inventory[slot].m_Index < GET_ITEM(7,0)))
		{
			if(lpTarget->Inventory[slot].ArmorDurabilityDown(lpTarget->Index,lpObj->PhysiDamageMin) != 0)
			{
				gItemManager.GCItemDurSend(lpTarget->Index,slot,(BYTE)lpTarget->Inventory[slot].m_Durability,0);
			}
		}
	}
}

bool CAttack::CheckPlayerTarget(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if(lpObj->Type != OBJECT_USER || lpTarget->Type != OBJECT_USER)
	{
		return 1;
	}

	if(lpTarget->Authority == 32)
	{
		return 0;
	}

	if(lpObj->PvP != 0 && lpTarget->PvP != 0)
	{
		return 1;
	}

	if(lpObj->HelperDelayTime != 0 && gParty.IsMember(lpObj->PartyNumber,lpTarget->Index) != 0)
	{
		return 0;
	}

	if(gServerInfo.m_PartyDisablePK != 0 && gParty.IsMember(lpObj->PartyNumber,lpTarget->Index) != 0)
	{
		return 0;
	}

	if(lpObj->HelperDelayTime != 0 && gParty.IsMember(lpObj->PartyNumber,lpTarget->Index) != 0)
	{
		return 0;
	}

	if(gObjGetRelationShip(lpObj,lpTarget) == 2 && gMapManager.GetMapNonPK(lpTarget->Map) == 0)
	{
		return 1;
	}

	if(lpObj->Guild != 0 && lpTarget->Guild != 0)
	{
		if(lpObj->Guild->WarState != 0 && lpTarget->Guild->WarState != 0)
		{
			if(lpObj->Guild->Number == lpTarget->Guild->Number)
			{
				return 0;
			}
		}
	}

	if(gObjTargetGuildWarCheck(lpObj,lpTarget) == 0)
	{
		if(lpTarget->Guild != 0 && lpTarget->Guild->WarState != 0)
		{
			if(lpTarget->Guild->WarType == 1 && lpTarget->Map != MAP_ARENA && gMapManager.GetMapNonPK(lpTarget->Map) == 0)
			{
				return 1;
			}

			if(CA_MAP_RANGE(lpTarget->Map) == 0 && CC_MAP_RANGE(lpTarget->Map) == 0 && IT_MAP_RANGE(lpTarget->Map) == 0 && gDuel.CheckDuel(lpObj,lpTarget) == 0 && gGensSystem.CheckGens(lpObj,lpTarget) == 0)
			{
				return 0;
			}
		}
	}

	if(gDuel.CheckDuel(lpObj,lpTarget) == 0 && OBJECT_RANGE(lpTarget->DuelUser) != 0)
	{
		return 0;
	}

	if(gMapManager.GetMapGensBattle(lpObj->Map) != 0 && gMapManager.GetMapGensBattle(lpTarget->Map) != 0)
	{
		return ((gMapManager.GetMapNonPK(lpTarget->Map)==0)?gGensSystem.CheckGens(lpObj,lpTarget):0);
	}

	if(lpObj->KillAll == 1 && lpTarget->KillAll == 1 && gEventKillAll.CheckPlayerTarget(lpObj,lpTarget) == 0)
	{
		return 0;
	}

	if(CA_MAP_RANGE(lpObj->Map) != 0 && CA_MAP_RANGE(lpTarget->Map) != 0)
	{
		return ((gCustomArena.CheckPlayerTarget(lpObj,lpTarget)==0)?0:1);
	}

	if(DS_MAP_RANGE(lpObj->Map) != 0 && DS_MAP_RANGE(lpTarget->Map) != 0)
	{
		return 0;
	}

	if(BC_MAP_RANGE(lpObj->Map) != 0 && BC_MAP_RANGE(lpTarget->Map) != 0)
	{
		return 0;
	}

	if(CC_MAP_RANGE(lpObj->Map) != 0 && CC_MAP_RANGE(lpTarget->Map) != 0)
	{
		return ((gChaosCastle.GetState(GET_CC_LEVEL(lpObj->Map))==CC_STATE_START)?1:0);
	}

	#if(GAMESERVER_TYPE==1)

	if(lpObj->Map == MAP_CASTLE_SIEGE && lpTarget->Map == MAP_CASTLE_SIEGE)
	{
		if(gCastleSiege.GetCastleState() == CASTLESIEGE_STATE_STARTSIEGE)
		{
			if(lpObj->CsJoinSide != 0 && lpTarget->CsJoinSide != 0)
			{
				if(gServerInfo.m_CastleSiegeDamageRate2 == 0 && lpObj->CsJoinSide == lpTarget->CsJoinSide)
				{
					return 0;
				}
				else
				{
					return 1;
				}
			}
		}
	}

	#endif

	if(lpObj->Map == MAP_KANTURU3 && lpTarget->Map == MAP_KANTURU3)
	{
		return 0;
	}

	if(IT_MAP_RANGE(lpObj->Map) != 0 && IT_MAP_RANGE(lpTarget->Map) != 0)
	{
		return gIllusionTemple.CheckPlayerTarget(lpObj,lpTarget);
	}

	if(lpObj->Map == MAP_RAKLION2 && lpTarget->Map == MAP_RAKLION2)
	{
		return 0;
	}

	if(DG_MAP_RANGE(lpObj->Map) != 0 && DG_MAP_RANGE(lpTarget->Map) != 0)
	{
		return 0;
	}

	if(IG_MAP_RANGE(lpObj->Map) != 0 && IG_MAP_RANGE(lpTarget->Map) != 0)
	{
		return 0;
	}

	if(lpObj->Level <= 5 || lpTarget->Level <= 5)
	{
		return 0;
	}

	if(gMapManager.GetMapNonPK(lpTarget->Map) != 0)
	{
		return 0;
	}
	if (gTvTEvent.CheckPlayerJoined(lpObj,lpObj) && !gTvTEvent.CheckPlayerJoined(lpTarget,lpTarget))
	{
		return 0;
	}

	if (gTvTEvent.CheckPlayerJoined(lpTarget,lpTarget) && !gTvTEvent.CheckPlayerJoined(lpObj,lpObj))
	{
		return 0;
	}

	if (gTvTEvent.CheckSelfTeam(lpObj, lpTarget))
	{
		return 0;
	}

	if (gGvGEvent.CheckStandTarget(lpObj) && gGvGEvent.CheckStandTarget(lpTarget))
	{
		return 0;
	}

	if (gGvGEvent.CheckPlayerJoined(lpObj,lpObj) && !gGvGEvent.CheckPlayerJoined(lpTarget,lpTarget))
	{
		return 0;
	}

	if (gGvGEvent.CheckPlayerJoined(lpTarget,lpTarget) && !gGvGEvent.CheckPlayerJoined(lpObj,lpObj))
	{
		return 0;
	}

	if (gGvGEvent.CheckSelfTeam(lpObj, lpTarget))
	{
		return 0;
	}
	return 1;
}

void CAttack::MissSend(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int send,int count) // OK
{
	WORD effect = 0;

	if(count > 0 && lpSkill != 0 && (lpSkill->m_skill == SKILL_UPPER_BEAST || lpSkill->m_skill == SKILL_DARK_SIDE))
	{
		effect |= (((count%2)==0)?0x20:0x10);
	}

	if(count > 0 && lpSkill != 0 && (lpSkill->m_skill == SKILL_LARGE_RING_BLOWER || lpSkill->m_skill == SKILL_CHAIN_DRIVER || lpSkill->m_skill == SKILL_DRAGON_LORE || lpSkill->m_skill == SKILL_PHOENIX_SHOT))
	{
		effect |= (((count%4)==0)?0x20:0x10);
	}

	GCDamageSend(lpObj->Index,lpTarget->Index,0,0,effect,0);

	if(send != 0 && lpSkill != 0)
	{
		gSkillManager.GCSkillAttackSend(lpObj,lpSkill->m_index,lpTarget->Index,0);
	}
}

bool CAttack::MissCheck(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int send,int count,BYTE* miss) // OK
{
	int AttackSuccessRate = lpObj->AttackSuccessRate;

	AttackSuccessRate += lpObj->EffectOption.AddAttackSuccessRate;

	AttackSuccessRate += (AttackSuccessRate*lpObj->EffectOption.MulAttackSuccessRate)/100;

	AttackSuccessRate -= (AttackSuccessRate*lpObj->EffectOption.DivAttackSuccessRate)/100;

	AttackSuccessRate = ((AttackSuccessRate<0)?0:AttackSuccessRate);

	int DefenseSuccessRate = lpTarget->DefenseSuccessRate;

	DefenseSuccessRate += lpTarget->EffectOption.AddDefenseSuccessRate;

	DefenseSuccessRate += (DefenseSuccessRate*lpTarget->EffectOption.MulDefenseSuccessRate)/100;

	DefenseSuccessRate -= (DefenseSuccessRate*lpTarget->EffectOption.DivDefenseSuccessRate)/100;

	DefenseSuccessRate = ((DefenseSuccessRate<0)?0:DefenseSuccessRate);

	if(AttackSuccessRate < DefenseSuccessRate)
	{
		(*miss) = 1;

		if((GetLargeRand()%100) >= 5)
		{
			this->MissSend(lpObj,lpTarget,lpSkill,send,count);
			return 0;
		}
	}
	else
	{
		(*miss) = 0;

		if((GetLargeRand()%((AttackSuccessRate==0)?1:AttackSuccessRate)) < DefenseSuccessRate)
		{
			this->MissSend(lpObj,lpTarget,lpSkill,send,count);
			return 0;
		}
	}

	return 1;
}

bool CAttack::MissCheckPvP(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int send,int count,BYTE* miss) // OK
{
	(*miss) = 0;

	int AttackSuccessRate = (int)(((100*(((lpObj->AttackSuccessRatePvP*10000.0f)/(lpObj->AttackSuccessRatePvP+lpTarget->DefenseSuccessRatePvP))/10000.0f))*gServerInfo.m_ShieldGaugeAttackRate)*(((lpObj->Level*10000.0f)/(lpObj->Level+lpTarget->Level))/10000.0f));

	if((lpTarget->Level-lpObj->Level) >= 100)
	{
		AttackSuccessRate -= 5;
	}
	else if((lpTarget->Level-lpObj->Level) >= 200)
	{
		AttackSuccessRate -= 10;
	}
	else if((lpTarget->Level-lpObj->Level) >= 300)
	{
		AttackSuccessRate -= 15;
	}

	if((GetLargeRand()%100) > AttackSuccessRate)
	{
		this->MissSend(lpObj,lpTarget,lpSkill,send,count);

		if(gServerInfo.m_ShieldGaugeAttackComboMiss != 0 && lpObj->ComboSkill.m_index >= 0)
		{
			lpObj->ComboSkill.Init();
		}

		return 0;
	}

	return 1;
}

bool CAttack::MissCheckElemental(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int send,int count,BYTE* miss) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int ElementalAttackSuccessRate = lpObj->ElementalAttackSuccessRate;

	if(lpObj->Type == OBJECT_USER)
	{
		ElementalAttackSuccessRate += (ElementalAttackSuccessRate*lpObj->PentagramOption.MulElementalAttackSuccessRate)/100;

		ElementalAttackSuccessRate += (ElementalAttackSuccessRate*lpObj->PentagramJewelOption.MulElementalAttackSuccessRate)/100;

		ElementalAttackSuccessRate = ((ElementalAttackSuccessRate<0)?0:ElementalAttackSuccessRate);
	}

	int ElementalDefenseSuccessRate = lpTarget->ElementalDefenseSuccessRate;

	if(lpTarget->Type == OBJECT_USER)
	{
		ElementalDefenseSuccessRate += (ElementalDefenseSuccessRate*lpTarget->PentagramOption.MulElementalDefenseSuccessRate)/100;

		ElementalDefenseSuccessRate += (ElementalDefenseSuccessRate*lpTarget->PentagramJewelOption.MulElementalDefenseSuccessRate)/100;

		ElementalDefenseSuccessRate = ((ElementalDefenseSuccessRate<0)?0:ElementalDefenseSuccessRate);
	}

	if(ElementalAttackSuccessRate < ElementalDefenseSuccessRate)
	{
		(*miss) = 1;

		if((GetLargeRand()%100) >= 5)
		{
			GCElementalDamageSend(lpObj->Index,lpTarget->Index,lpObj->ElementalAttribute,0);
			return 0;
		}
	}
	else
	{
		(*miss) = 0;

		if((GetLargeRand()%((ElementalAttackSuccessRate==0)?1:ElementalAttackSuccessRate)) < ElementalDefenseSuccessRate)
		{
			GCElementalDamageSend(lpObj->Index,lpTarget->Index,lpObj->ElementalAttribute,0);
			return 0;
		}
	}

	return 1;

	#else

	return 0;

	#endif
}

bool CAttack::ApplySkillEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage) // OK
{
	if(lpTarget->Type != OBJECT_USER && ((lpTarget->Class >= 204 && lpTarget->Class <= 209) || (lpTarget->Class >= 215 && lpTarget->Class <= 219) || lpTarget->Class == 277 || lpTarget->Class == 278 || lpTarget->Class == 283 || lpTarget->Class == 288))
	{
		return 0;
	}

	if(lpTarget->Type == OBJECT_USER && (gEffectManager.CheckEffect(lpTarget,EFFECT_IRON_DEFENSE) != 0 || gEffectManager.CheckEffect(lpTarget,EFFECT_IRON_DEFENSE_IMPROVED) != 0))
	{
		return 0;
	}

	if(gSkillManager.GetSkillType(lpSkill->m_index) != -1 && gObjCheckResistance(lpTarget,gSkillManager.GetSkillType(lpSkill->m_index)) != 0)
	{
		return 0;
	}

	switch(lpSkill->m_skill)
	{
		case SKILL_POISON:
			gEffectManager.AddEffect(lpTarget,0,gSkillManager.GetSkillEffect(lpSkill->m_index),20,lpObj->Index,2,3,0);
			break;
		case SKILL_METEORITE:
			gSkillManager.ApplyMeteoriteEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_LIGHTNING:
			if (gServerInfo.m_DisableLightningEffect == 0)	gObjAddMsgSendDelay(lpTarget,2,lpObj->Index,150,0);
			break;
		case SKILL_ICE:
			gEffectManager.AddEffect(lpTarget,0,gSkillManager.GetSkillEffect(lpSkill->m_index),10,0,0,0,0);
			break;
		case SKILL_FALLING_SLASH:
			if (gServerInfo.m_DisableFallingEffect == 0)	gObjAddMsgSendDelay(lpTarget,2,lpObj->Index,150,0);
			break;
		case SKILL_LUNGE:
			if (gServerInfo.m_DisableLungEffect == 0)		gObjAddMsgSendDelay(lpTarget,2,lpObj->Index,150,0);
			break;
		case SKILL_UPPERCUT:
			if (gServerInfo.m_DisableUpperCutEffect == 0)	gObjAddMsgSendDelay(lpTarget,2,lpObj->Index,150,0);
			break;
		case SKILL_CYCLONE:
			if (gServerInfo.m_DisableCycloneEffect == 0) 	gObjAddMsgSendDelay(lpTarget,2,lpObj->Index,150,0); 
			break;
		case SKILL_SLASH:
			if (gServerInfo.m_DisableTwistEffect == 0)		gObjAddMsgSendDelay(lpTarget,2,lpObj->Index,150,0);
			break;
		case SKILL_DECAY:
			gEffectManager.AddEffect(lpTarget,0,gSkillManager.GetSkillEffect(lpSkill->m_index),10,lpObj->Index,2,3,0);
			break;
		case SKILL_ICE_STORM:
			gSkillManager.ApplyIceStormEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_ICE_ARROW:
			gEffectManager.AddEffect(lpTarget,0,gSkillManager.GetSkillEffect(lpSkill->m_index),7,0,0,0,0);
			break;
		case SKILL_TWISTING_SLASH:
			gSkillManager.ApplyTwistingSlashEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_RAGEFUL_BLOW:
			gSkillManager.ApplyRagefulBlowEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_DEATH_STAB:
			gSkillManager.ApplyDeathStabEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_FIRE_SLASH:
			gSkillManager.ApplyFireSlashEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_FIRE_BURST:
			gSkillManager.ApplyFireBurstEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_PLASMA_STORM:
			gSkillManager.ApplyPlasmaStormEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_FIRE_SCREAM:
			gSkillManager.ApplyFireScreamEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_EARTHQUAKE:
			gSkillManager.ApplyEarthquakeEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_DRAIN_LIFE:
			gSkillManager.ApplyDrainLifeEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_SAHAMUTT:
			gSkillManager.ApplySahamuttEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_NEIL:
			gSkillManager.ApplyNeilEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_GHOST_PHANTOM:
			gSkillManager.ApplyGhostPhantomEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_RED_STORM:
			gEffectManager.AddEffect(lpTarget,0,gSkillManager.GetSkillEffect(lpSkill->m_index),1,0,0,0,0);
			break;
		case SKILL_FROZEN_STAB:
			gSkillManager.ApplyFrozenStabEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_FIVE_SHOT:
			gSkillManager.ApplyFiveShotEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_SWORD_SLASH:
			gSkillManager.ApplySwordSlashEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_LIGHTNING_STORM:
			gEffectManager.AddEffect(lpTarget,0,gSkillManager.GetSkillEffect(lpSkill->m_index),1,0,0,0,0);
			break;
		case SKILL_LARGE_RING_BLOWER:
			gSkillManager.ApplyLargeRingBlowerEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_UPPER_BEAST:
			gSkillManager.ApplyUpperBeastEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_CHAIN_DRIVER:
			gSkillManager.ApplyChainDriverEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_DRAGON_LORE:
			gSkillManager.ApplyDragonLoreEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_DRAGON_SLAYER:
			gSkillManager.ApplyDragonSlayerEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_PHOENIX_SHOT:
			gSkillManager.ApplyPhoenixShotEffect(lpObj,lpTarget,lpSkill,damage);
			break;
		case SKILL_POISON_ARROW:
			gEffectManager.AddEffect(lpTarget,0,gSkillManager.GetSkillEffect(lpSkill->m_index),10,lpObj->Index,2,3,0);
			break;
		case SKILL_EARTH_PRISON:
			gSkillManager.ApplyEarthPrisonEffect(lpObj,lpTarget,lpSkill,damage);
			break;
	}

	return 1;
}

int CAttack::GetTargetDefense(LPOBJ lpObj,LPOBJ lpTarget,WORD* effect) // OK
{
	int defense = lpTarget->Defense;

	defense += lpTarget->EffectOption.AddDefense;
	defense -= lpTarget->EffectOption.SubDefense;

	gSkillManager.SkillSwordPowerGetDefense(lpObj->Index,&defense);

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

	if(lpTarget->Type == OBJECT_USER)
	{
		defense = (defense*50)/100;
	}

	if((GetLargeRand()%100) < ((lpObj->IgnoreDefenseRate+lpObj->EffectOption.AddIgnoreDefenseRate)-lpTarget->ResistIgnoreDefenseRate))
	{
		(*effect) = 1;

		defense = 0;
	}

	defense = ((defense<0)?0:defense);

	return defense;
}

int CAttack::GetTargetElementalDefense(LPOBJ lpObj,LPOBJ lpTarget,WORD* effect) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int defense = lpTarget->ElementalDefense;

	if(lpTarget->Type == OBJECT_USER)
	{
		defense += (((lpTarget->Inventory[236].IsItem()==0)?0:lpTarget->Inventory[236].m_Defense)*lpTarget->PentagramOption.MulPentagramDefense)/100;

		defense += (lpTarget->Defense*lpTarget->PentagramOption.AddElementalDefenseTransferRate)/100;

		defense += lpTarget->PentagramJewelOption.AddElementalDefense;

		if(lpObj->Type == OBJECT_USER)
		{
			defense += lpTarget->PentagramJewelOption.AddElementalDefensePvP;

			if(lpObj->Class == CLASS_DW || lpObj->Class == CLASS_FE || lpObj->Class == CLASS_MG || lpObj->Class == CLASS_SU)
			{
				defense += lpTarget->PentagramJewelOption.AddElementalDefenseRange;
			}
			else
			{
				defense += lpTarget->PentagramJewelOption.AddElementalDefenseMelee;
			}
		}
		else
		{
			defense += lpTarget->PentagramJewelOption.AddElementalDefensePvM;
		}
	}

	gPentagramSystem.GetPentagramRelationshipDefense(lpTarget,lpObj,&defense);

	defense = ((defense<0)?0:defense);

	return defense;

	#else

	return 0;

	#endif
}

int CAttack::GetAttackDamage(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,WORD* effect,int TargetDefense) // OK
{
	CItem* Right = &lpObj->Inventory[0];
	CItem* Left = &lpObj->Inventory[1];

	int damage = 0;
	int DamageMin = 0;
	int DamageMax = 0;
	int SkillDamageMin = 0;
	int SkillDamageMax = 0;
	bool DualHandWeapon = 0;

	if(lpObj->Type == OBJECT_MONSTER || lpObj->Type == OBJECT_NPC)
	{
		DamageMin = lpObj->PhysiDamageMin;
		DamageMax = lpObj->PhysiDamageMax;

		DamageMin += lpObj->EffectOption.AddPhysiDamage;
		DamageMax += lpObj->EffectOption.AddPhysiDamage;

		DamageMin += lpObj->EffectOption.AddMinPhysiDamage;
		DamageMax += lpObj->EffectOption.AddMaxPhysiDamage;

		DamageMin += (DamageMin*lpObj->EffectOption.MulPhysiDamage)/100;
		DamageMax += (DamageMax*lpObj->EffectOption.MulPhysiDamage)/100;

		DamageMin -= (DamageMin*lpObj->EffectOption.DivPhysiDamage)/100;
		DamageMax -= (DamageMax*lpObj->EffectOption.DivPhysiDamage)/100;

		int range = (DamageMax-DamageMin);

		range = ((range<1)?1:range);

		damage = DamageMin+(GetLargeRand()%range);
	}
	else
	{
		if(lpObj->Class == CLASS_DK || lpObj->Class == CLASS_MG || lpObj->Class == CLASS_DL || lpObj->Class == CLASS_RF)
		{
			if(Right->m_Index >= GET_ITEM(0,0) && Right->m_Index < GET_ITEM(4,0) && Left->m_Index >= GET_ITEM(0,0) && Left->m_Index < GET_ITEM(4,0))
			{
				if(Right->m_IsValidItem != 0 && Left->m_IsValidItem != 0)
				{
					DualHandWeapon = 1;
				}
			}
		}

		if(lpSkill != 0)
		{
			SkillDamageMin = lpSkill->m_DamageMin;
			SkillDamageMax = lpSkill->m_DamageMax;

			SkillDamageMin += gMasterSkillTree.GetMasterSkillDamageMin(lpObj,lpSkill->m_skill);
			SkillDamageMax += gMasterSkillTree.GetMasterSkillDamageMax(lpObj,lpSkill->m_skill);

			SkillDamageMin += lpObj->SkillDamageBonus;
			SkillDamageMax += lpObj->SkillDamageBonus;

			int type = gSkillManager.GetSkillType(lpSkill->m_index);

			if(CHECK_RANGE(type,MAX_RESISTANCE_TYPE) != 0)
			{
				SkillDamageMin += lpObj->AddResistance[type];
				SkillDamageMax += lpObj->AddResistance[type];
			}

			if(lpObj->Class == CLASS_DL)
			{
				switch(lpSkill->m_skill)
				{
					case SKILL_EARTHQUAKE:
						SkillDamageMin += ((lpObj->Strength+lpObj->AddStrength)/gServerInfo.m_EarthquakeDamageConstA)+((lpObj->Leadership+lpObj->AddLeadership)/gServerInfo.m_EarthquakeDamageConstB)+(lpObj->Inventory[8].m_PetItemLevel*gServerInfo.m_EarthquakeDamageConstC);
						SkillDamageMax += ((lpObj->Strength+lpObj->AddStrength)/gServerInfo.m_EarthquakeDamageConstA)+((lpObj->Leadership+lpObj->AddLeadership)/gServerInfo.m_EarthquakeDamageConstB)+(lpObj->Inventory[8].m_PetItemLevel*gServerInfo.m_EarthquakeDamageConstC);
						break;
					case SKILL_ELECTRIC_SPARK:
						SkillDamageMin += ((lpObj->Leadership+lpObj->AddLeadership)/gServerInfo.m_ElectricSparkDamageConstA)+gServerInfo.m_ElectricSparkDamageConstB;
						SkillDamageMax += ((lpObj->Leadership+lpObj->AddLeadership)/gServerInfo.m_ElectricSparkDamageConstA)+gServerInfo.m_ElectricSparkDamageConstB;
						break;
					default:
						SkillDamageMin += ((lpObj->Strength+lpObj->AddStrength)/gServerInfo.m_DLSkillDamageConstA)+((lpObj->Energy+lpObj->AddEnergy)/gServerInfo.m_DLSkillDamageConstB);
						SkillDamageMax += ((lpObj->Strength+lpObj->AddStrength)/gServerInfo.m_DLSkillDamageConstA)+((lpObj->Energy+lpObj->AddEnergy)/gServerInfo.m_DLSkillDamageConstB);
						break;
				}
			}
		}

		if(DualHandWeapon != 0)
		{
			DamageMin = lpObj->PhysiDamageMinRight+lpObj->PhysiDamageMinLeft+SkillDamageMin;
			DamageMax = lpObj->PhysiDamageMaxRight+lpObj->PhysiDamageMaxLeft+SkillDamageMax;
		}
		else if((Right->m_Index >= GET_ITEM(0,0) && Right->m_Index < GET_ITEM(4,0)) || (Right->m_Index >= GET_ITEM(5,0) && Right->m_Index < GET_ITEM(6,0)))
		{
			DamageMin = lpObj->PhysiDamageMinRight+SkillDamageMin;
			DamageMax = lpObj->PhysiDamageMaxRight+SkillDamageMax;
		}
		else if(Right->m_Index >= GET_ITEM(4,0) && Right->m_Index < GET_ITEM(5,0) && Right->m_Index != GET_ITEM(4,15) && Right->m_Slot == 0)
		{
			DamageMin = lpObj->PhysiDamageMinRight+SkillDamageMin;
			DamageMax = lpObj->PhysiDamageMaxRight+SkillDamageMax;
		}
		else if(Left->m_Index >= GET_ITEM(4,0) && Left->m_Index < GET_ITEM(5,0) && Left->m_Index != GET_ITEM(4,7) && Left->m_Slot == 1)
		{
			DamageMin = lpObj->PhysiDamageMinLeft+SkillDamageMin;
			DamageMax = lpObj->PhysiDamageMaxLeft+SkillDamageMax;
		}
		else
		{
			DamageMin = lpObj->PhysiDamageMinLeft+SkillDamageMin;
			DamageMax = lpObj->PhysiDamageMaxLeft+SkillDamageMax;
		}

		DamageMin += lpObj->EffectOption.AddPhysiDamage;
		DamageMax += lpObj->EffectOption.AddPhysiDamage;

		DamageMin += lpObj->EffectOption.AddMinPhysiDamage;
		DamageMax += lpObj->EffectOption.AddMaxPhysiDamage;

		gSkillManager.SkillSwordPowerGetPhysiDamage(lpObj->Index,&DamageMin,&DamageMax);

		DamageMin += (DamageMin*lpObj->EffectOption.MulPhysiDamage)/100;
		DamageMax += (DamageMax*lpObj->EffectOption.MulPhysiDamage)/100;

		DamageMin -= (DamageMin*lpObj->EffectOption.DivPhysiDamage)/100;
		DamageMax -= (DamageMax*lpObj->EffectOption.DivPhysiDamage)/100;

		int range = (DamageMax-DamageMin);

		range = ((range<1)?1:range);

		damage = DamageMin+(GetLargeRand()%range);

		if((GetLargeRand()%100) < ((lpObj->CriticalDamageRate+lpObj->EffectOption.AddCriticalDamageRate)-lpTarget->ResistCriticalDamageRate))
		{
			(*effect) = 3;

			damage = DamageMax;
			damage += lpObj->CriticalDamage;
			damage += lpObj->EffectOption.AddCriticalDamage;
		}

		if((GetLargeRand()%100) < ((lpObj->ExcellentDamageRate+lpObj->EffectOption.AddExcellentDamageRate)-lpTarget->ResistExcellentDamageRate))
		{
			(*effect) = 2;

			damage = (DamageMax*120)/100;
			damage += lpObj->ExcellentDamage;
			damage += lpObj->EffectOption.AddExcellentDamage;
		}
	}

	if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
	{
		damage += lpObj->DamagePvP;
	}

	if(lpObj->MonsterSkillElementOption.CheckAttackTime() != 0)
	{
		damage += lpObj->MonsterSkillElementOption.m_SkillElementAttack;
	}

	damage -= TargetDefense;

	damage = ((damage<0)?0:damage);

	return damage;
}

int CAttack::GetAttackDamageWizard(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,WORD* effect,int TargetDefense) // OK
{
	CItem* Right = &lpObj->Inventory[0];
	CItem* Left = &lpObj->Inventory[1];

	int DamageMin = lpObj->MagicDamageMin;
	int DamageMax = lpObj->MagicDamageMax;

	if(lpSkill->m_skill == SKILL_NOVA && lpObj->SkillNovaCount >= 0)
	{
		DamageMin += (gServerInfo.m_NovaDamageConstA*(lpObj->SkillNovaCount*gServerInfo.m_NovaDamageConstB))+((lpObj->Strength+lpObj->AddStrength)/gServerInfo.m_NovaDamageConstC);
		DamageMax += (gServerInfo.m_NovaDamageConstA*(lpObj->SkillNovaCount*gServerInfo.m_NovaDamageConstB))+((lpObj->Strength+lpObj->AddStrength)/gServerInfo.m_NovaDamageConstC);
	}
	else
	{
		DamageMin += lpSkill->m_DamageMin;
		DamageMax += lpSkill->m_DamageMax;
	}

	DamageMin += gMasterSkillTree.GetMasterSkillDamageMin(lpObj,lpSkill->m_skill);
	DamageMax += gMasterSkillTree.GetMasterSkillDamageMax(lpObj,lpSkill->m_skill);

	DamageMin += lpObj->SkillDamageBonus;
	DamageMax += lpObj->SkillDamageBonus;

	DamageMin += lpObj->EffectOption.AddMagicDamage;
	DamageMax += lpObj->EffectOption.AddMagicDamage;

	DamageMin += lpObj->EffectOption.AddMinMagicDamage;
	DamageMax += lpObj->EffectOption.AddMaxMagicDamage;

	gSkillManager.SkillSwordPowerGetMagicDamage(lpObj->Index,&DamageMin,&DamageMax);

	int type = gSkillManager.GetSkillType(lpSkill->m_index);

	if(CHECK_RANGE(type,MAX_RESISTANCE_TYPE) != 0)
	{
		DamageMin += lpObj->AddResistance[type];
		DamageMax += lpObj->AddResistance[type];
	}

	DamageMin += (DamageMin*lpObj->EffectOption.MulMagicDamage)/100;
	DamageMax += (DamageMax*lpObj->EffectOption.MulMagicDamage)/100;

	DamageMin -= (DamageMin*lpObj->EffectOption.DivMagicDamage)/100;
	DamageMax -= (DamageMax*lpObj->EffectOption.DivMagicDamage)/100;

	if(Right->IsItem() != 0 && Right->m_IsValidItem != 0 && ((Right->m_Index >= GET_ITEM(0,0) && Right->m_Index < GET_ITEM(1,0)) || (Right->m_Index >= GET_ITEM(5,0) && Right->m_Index < GET_ITEM(6,0))))
	{
		int rise = (int)(((Right->m_MagicDamageRate/2)+(Right->m_Level*2))*Right->m_CurrentDurabilityState);
		DamageMin += (DamageMin*rise)/100;
		DamageMax += (DamageMax*rise)/100;
	}

	int range = (DamageMax-DamageMin);

	range = ((range<1)?1:range);

	int damage = DamageMin+(GetLargeRand()%range);

	if((GetLargeRand()%100) < ((lpObj->CriticalDamageRate+lpObj->EffectOption.AddCriticalDamageRate)-lpTarget->ResistCriticalDamageRate))
	{
		(*effect) = 3;

		damage = DamageMax;
		damage += lpObj->CriticalDamage;
		damage += lpObj->EffectOption.AddCriticalDamage;
	}

	if((GetLargeRand()%100) < ((lpObj->ExcellentDamageRate+lpObj->EffectOption.AddExcellentDamageRate)-lpTarget->ResistExcellentDamageRate))
	{
		(*effect) = 2;

		damage = (DamageMax*120)/100;
		damage += lpObj->ExcellentDamage;
		damage += lpObj->EffectOption.AddExcellentDamage;
	}

	if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
	{
		damage += lpObj->DamagePvP;
	}

	damage -= TargetDefense;

	damage = ((damage<0)?0:damage);

	return damage;
}

int CAttack::GetAttackDamageCursed(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,WORD* effect,int TargetDefense) // OK
{
	CItem* Right = &lpObj->Inventory[0];
	CItem* Left = &lpObj->Inventory[1];

	int DamageMin = lpObj->CurseDamageMin;
	int DamageMax = lpObj->CurseDamageMax;

	DamageMin += lpSkill->m_DamageMin;
	DamageMax += lpSkill->m_DamageMax;

	DamageMin += gMasterSkillTree.GetMasterSkillDamageMin(lpObj,lpSkill->m_skill);
	DamageMax += gMasterSkillTree.GetMasterSkillDamageMax(lpObj,lpSkill->m_skill);

	DamageMin += lpObj->SkillDamageBonus;
	DamageMax += lpObj->SkillDamageBonus;

	DamageMin += lpObj->EffectOption.AddCurseDamage;
	DamageMax += lpObj->EffectOption.AddCurseDamage;

	DamageMin += lpObj->EffectOption.AddMinCurseDamage;
	DamageMax += lpObj->EffectOption.AddMaxCurseDamage;

	gSkillManager.SkillSwordPowerGetCurseDamage(lpObj->Index,&DamageMin,&DamageMax);

	int type = gSkillManager.GetSkillType(lpSkill->m_index);

	if(CHECK_RANGE(type,MAX_RESISTANCE_TYPE) != 0)
	{
		DamageMin += lpObj->AddResistance[type];
		DamageMax += lpObj->AddResistance[type];
	}

	DamageMin += (DamageMin*lpObj->EffectOption.MulCurseDamage)/100;
	DamageMax += (DamageMax*lpObj->EffectOption.MulCurseDamage)/100;

	DamageMin -= (DamageMin*lpObj->EffectOption.DivCurseDamage)/100;
	DamageMax -= (DamageMax*lpObj->EffectOption.DivCurseDamage)/100;

	if(Left->IsItem() != 0 && Left->m_IsValidItem != 0 && Left->m_Index >= GET_ITEM(5,21) && Left->m_Index <= GET_ITEM(5,23))
	{
		int rise = (int)(((Left->m_MagicDamageRate/2)+(Left->m_Level*2))*Left->m_CurrentDurabilityState);
		DamageMin += (DamageMin*rise)/100;
		DamageMax += (DamageMax*rise)/100;
	}

	int range = (DamageMax-DamageMin);

	range = ((range<1)?1:range);

	int damage = DamageMin+(GetLargeRand()%range);

	if((GetLargeRand()%100) < ((lpObj->CriticalDamageRate+lpObj->EffectOption.AddCriticalDamageRate)-lpTarget->ResistCriticalDamageRate))
	{
		(*effect) = 3;

		damage = DamageMax;
		damage += lpObj->CriticalDamage;
		damage += lpObj->EffectOption.AddCriticalDamage;
	}

	if((GetLargeRand()%100) < ((lpObj->ExcellentDamageRate+lpObj->EffectOption.AddExcellentDamageRate)-lpTarget->ResistExcellentDamageRate))
	{
		(*effect) = 2;

		damage = (DamageMax*120)/100;
		damage += lpObj->ExcellentDamage;
		damage += lpObj->EffectOption.AddExcellentDamage;
	}

	if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
	{
		damage += lpObj->DamagePvP;
	}

	damage -= TargetDefense;

	damage = ((damage<0)?0:damage);

	return damage;
}

int CAttack::GetAttackDamageFenrir(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,WORD* effect,int TargetDefense) // OK
{
	int BaseDamage = 0;

	if(lpObj->Class == CLASS_DW)
	{
		BaseDamage = (lpObj->Strength/gServerInfo.m_DWPlasmaStormDamageConstA)+(lpObj->Dexterity/gServerInfo.m_DWPlasmaStormDamageConstB)+(lpObj->Vitality/gServerInfo.m_DWPlasmaStormDamageConstC)+(lpObj->Energy/gServerInfo.m_DWPlasmaStormDamageConstD);
	}
	else if(lpObj->Class == CLASS_DK)
	{
		BaseDamage = (lpObj->Strength/gServerInfo.m_DKPlasmaStormDamageConstA)+(lpObj->Dexterity/gServerInfo.m_DKPlasmaStormDamageConstB)+(lpObj->Vitality/gServerInfo.m_DKPlasmaStormDamageConstC)+(lpObj->Energy/gServerInfo.m_DKPlasmaStormDamageConstD);
	}
	else if(lpObj->Class == CLASS_FE)
	{
		BaseDamage = (lpObj->Strength/gServerInfo.m_FEPlasmaStormDamageConstA)+(lpObj->Dexterity/gServerInfo.m_FEPlasmaStormDamageConstB)+(lpObj->Vitality/gServerInfo.m_FEPlasmaStormDamageConstC)+(lpObj->Energy/gServerInfo.m_FEPlasmaStormDamageConstD);
	}
	else if(lpObj->Class == CLASS_MG)
	{
		BaseDamage = (lpObj->Strength/gServerInfo.m_MGPlasmaStormDamageConstA)+(lpObj->Dexterity/gServerInfo.m_MGPlasmaStormDamageConstB)+(lpObj->Vitality/gServerInfo.m_MGPlasmaStormDamageConstC)+(lpObj->Energy/gServerInfo.m_MGPlasmaStormDamageConstD);
	}
	else if(lpObj->Class == CLASS_DL)
	{
		BaseDamage = (lpObj->Strength/gServerInfo.m_DLPlasmaStormDamageConstA)+(lpObj->Dexterity/gServerInfo.m_DLPlasmaStormDamageConstB)+(lpObj->Vitality/gServerInfo.m_DLPlasmaStormDamageConstC)+(lpObj->Energy/gServerInfo.m_DLPlasmaStormDamageConstD)+(lpObj->Leadership/gServerInfo.m_DLPlasmaStormDamageConstE);
	}
	else if(lpObj->Class == CLASS_SU)
	{
		BaseDamage = (lpObj->Strength/gServerInfo.m_SUPlasmaStormDamageConstA)+(lpObj->Dexterity/gServerInfo.m_SUPlasmaStormDamageConstB)+(lpObj->Vitality/gServerInfo.m_SUPlasmaStormDamageConstC)+(lpObj->Energy/gServerInfo.m_SUPlasmaStormDamageConstD);
	}
	else if(lpObj->Class == CLASS_RF)
	{
		BaseDamage = (lpObj->Strength/gServerInfo.m_RFPlasmaStormDamageConstA)+(lpObj->Dexterity/gServerInfo.m_RFPlasmaStormDamageConstB)+(lpObj->Vitality/gServerInfo.m_RFPlasmaStormDamageConstC)+(lpObj->Energy/gServerInfo.m_RFPlasmaStormDamageConstD);
	}

	int range = (lpSkill->m_DamageMax-lpSkill->m_DamageMin);

	range = ((range<1)?1:range);

	int damage = (BaseDamage+lpSkill->m_DamageMin)+(GetLargeRand()%range);

	if((GetLargeRand()%100) < ((lpObj->CriticalDamageRate+lpObj->EffectOption.AddCriticalDamageRate)-lpTarget->ResistCriticalDamageRate))
	{
		(*effect) = 3;

		damage = (BaseDamage+lpSkill->m_DamageMax);
		damage += lpObj->CriticalDamage;
		damage += lpObj->EffectOption.AddCriticalDamage;
	}

	if((GetLargeRand()%100) < ((lpObj->ExcellentDamageRate+lpObj->EffectOption.AddExcellentDamageRate)-lpTarget->ResistExcellentDamageRate))
	{
		(*effect) = 2;

		damage = ((BaseDamage+lpSkill->m_DamageMax)*120)/100;
		damage += lpObj->ExcellentDamage;
		damage += lpObj->EffectOption.AddExcellentDamage;
	}

	if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
	{
		damage += lpObj->DamagePvP;
	}

	damage -= TargetDefense;

	damage = ((damage<0)?0:damage);

	return damage;
}

int CAttack::GetAttackDamageElemental(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,WORD* effect,int AttackDamage,int TargetDefense) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int DamageMin = lpObj->ElementalDamageMin;
	int DamageMax = lpObj->ElementalDamageMax;

	if(lpObj->Type == OBJECT_USER)
	{
		DamageMin += (((lpObj->Inventory[236].IsItem()==0)?0:lpObj->Inventory[236].m_DamageMin)*lpObj->PentagramOption.MulPentagramDamage)/100;
		DamageMax += (((lpObj->Inventory[236].IsItem()==0)?0:lpObj->Inventory[236].m_DamageMax)*lpObj->PentagramOption.MulPentagramDamage)/100;

		DamageMin += (AttackDamage*lpObj->PentagramOption.AddElementalAttackTransferRate)/100;
		DamageMax += (AttackDamage*lpObj->PentagramOption.AddElementalAttackTransferRate)/100;

		DamageMin += lpObj->PentagramJewelOption.AddElementalDamage;
		DamageMax += lpObj->PentagramJewelOption.AddElementalDamage;

		if(lpTarget->Type == OBJECT_USER)
		{
			DamageMin += lpObj->PentagramJewelOption.AddElementalDamagePvP;
			DamageMax += lpObj->PentagramJewelOption.AddElementalDamagePvP;

			if(lpTarget->Class == CLASS_DW || lpTarget->Class == CLASS_FE || lpTarget->Class == CLASS_MG || lpTarget->Class == CLASS_SU)
			{
				DamageMin += lpObj->PentagramJewelOption.AddElementalDamageRange;
				DamageMax += lpObj->PentagramJewelOption.AddElementalDamageRange;
			}
			else
			{
				DamageMin += lpObj->PentagramJewelOption.AddElementalDamageMelee;
				DamageMax += lpObj->PentagramJewelOption.AddElementalDamageMelee;
			}

			DamageMin += (DamageMin*lpTarget->PentagramJewelOption.MulElementalDamagePvP)/100;
			DamageMax += (DamageMax*lpTarget->PentagramJewelOption.MulElementalDamagePvP)/100;
		}
		else
		{
			DamageMin += lpObj->PentagramJewelOption.AddElementalDamagePvM;
			DamageMax += lpObj->PentagramJewelOption.AddElementalDamagePvM;

			DamageMin += (DamageMin*lpTarget->PentagramJewelOption.MulElementalDamagePvM)/100;
			DamageMax += (DamageMax*lpTarget->PentagramJewelOption.MulElementalDamagePvM)/100;
		}
	}

	int range = (DamageMax-DamageMin);

	range = ((range<1)?1:range);

	int damage = DamageMin+(GetLargeRand()%range);

	if((GetLargeRand()%100) < (lpObj->PentagramOption.AddElementalCriticalDamageRate+((lpTarget->Type==OBJECT_USER)?lpObj->PentagramJewelOption.AddElementalCriticalDamageRatePvP:lpObj->PentagramJewelOption.AddElementalCriticalDamageRatePvM)))
	{
		(*effect) = 6;

		damage = DamageMax;
	}

	if((GetLargeRand()%100) < ((lpTarget->Type==OBJECT_USER)?lpObj->PentagramJewelOption.AddElementalExcellentDamageRatePvP:lpObj->PentagramJewelOption.AddElementalExcellentDamageRatePvM))
	{
		(*effect) = 7;

		damage = (DamageMax*120)/100;
	}

	gPentagramSystem.GetPentagramRelationshipDamage(lpObj,lpTarget,&damage);

	damage -= TargetDefense;

	damage = ((damage<0)?0:damage);

	return damage;

	#else

	return 0;

	#endif
}

int CAttack::GetShieldDamage(LPOBJ lpObj,LPOBJ lpTarget,int damage) // OK
{
	int rate = lpTarget->ShieldGaugeRate;

	if((GetLargeRand()%100) < ((lpObj->IgnoreShieldGaugeRate)-lpTarget->ResistIgnoreShieldGaugeRate))
	{
		rate = 0;
	}
	else
	{
		rate -= lpObj->DecreaseShieldGaugeRate;
	}

	rate = ((rate<0)?0:((rate>100)?100:rate));

	int SDDamage = (damage*rate)/100;
	int HPDamage = damage-SDDamage;

	if(lpTarget->Shield < SDDamage)
	{
		HPDamage = HPDamage+(SDDamage-lpTarget->Shield);
		SDDamage = lpTarget->Shield;

		if(lpTarget->Shield > 0 && HPDamage > (((lpTarget->MaxLife+lpTarget->AddLife)*20)/100))
		{
			if(CC_MAP_RANGE(lpTarget->Map) == 0 || IT_MAP_RANGE(lpTarget->Map) == 0)
			{
				GCEffectInfoSend(lpTarget->Index,17);
			}
		}
	}

	return SDDamage;
}

void CAttack::GetPreviewDefense(LPOBJ lpObj,DWORD* defense) // OK
{
	(*defense) = lpObj->Defense;

	(*defense) += lpObj->EffectOption.AddDefense;

	(*defense) -= lpObj->EffectOption.SubDefense;

	gSkillManager.SkillSwordPowerGetDefense(lpObj->Index,(int*)defense);

	(*defense) += ((*defense)*lpObj->EffectOption.MulDefense)/100;

	(*defense) -= ((*defense)*lpObj->EffectOption.DivDefense)/100;
}

void CAttack::GetPreviewPhysiDamage(LPOBJ lpObj,DWORD* DamageMin,DWORD* DamageMax,DWORD* MulDamage,DWORD* DivDamage) // OK
{
	CItem* Right = &lpObj->Inventory[0];
	CItem* Left = &lpObj->Inventory[1];

	bool DualHandWeapon = 0;

	if(lpObj->Class == CLASS_DK || lpObj->Class == CLASS_MG || lpObj->Class == CLASS_DL || lpObj->Class == CLASS_RF)
	{
		if(Right->m_Index >= GET_ITEM(0,0) && Right->m_Index < GET_ITEM(4,0) && Left->m_Index >= GET_ITEM(0,0) && Left->m_Index < GET_ITEM(4,0))
		{
			if(Right->m_IsValidItem != 0 && Left->m_IsValidItem != 0)
			{
				DualHandWeapon = 1;
			}
		}
	}

	if(DualHandWeapon != 0)
	{
		(*DamageMin) = lpObj->PhysiDamageMinRight+lpObj->PhysiDamageMinLeft;
		(*DamageMax) = lpObj->PhysiDamageMaxRight+lpObj->PhysiDamageMaxLeft;
	}
	else if((Right->m_Index >= GET_ITEM(0,0) && Right->m_Index < GET_ITEM(4,0)) || (Right->m_Index >= GET_ITEM(5,0) && Right->m_Index < GET_ITEM(6,0)))
	{
		(*DamageMin) = lpObj->PhysiDamageMinRight;
		(*DamageMax) = lpObj->PhysiDamageMaxRight;
	}
	else if(Right->m_Index >= GET_ITEM(4,0) && Right->m_Index < GET_ITEM(5,0) && Right->m_Index != GET_ITEM(4,15) && Right->m_Slot == 0)
	{
		(*DamageMin) = lpObj->PhysiDamageMinRight;
		(*DamageMax) = lpObj->PhysiDamageMaxRight;
	}
	else if(Left->m_Index >= GET_ITEM(4,0) && Left->m_Index < GET_ITEM(5,0) && Left->m_Index != GET_ITEM(4,7) && Left->m_Slot == 1)
	{
		(*DamageMin) = lpObj->PhysiDamageMinLeft;
		(*DamageMax) = lpObj->PhysiDamageMaxLeft;
	}
	else
	{
		(*DamageMin) = lpObj->PhysiDamageMinLeft;
		(*DamageMax) = lpObj->PhysiDamageMaxLeft;
	}

	(*DamageMin) += lpObj->EffectOption.AddPhysiDamage;
	(*DamageMax) += lpObj->EffectOption.AddPhysiDamage;

	(*DamageMin) += lpObj->EffectOption.AddMinPhysiDamage;
	(*DamageMax) += lpObj->EffectOption.AddMaxPhysiDamage;

	gSkillManager.SkillSwordPowerGetPhysiDamage(lpObj->Index,(int*)DamageMin,(int*)DamageMax);

	(*MulDamage) = lpObj->EffectOption.MulPhysiDamage;
	(*DivDamage) = lpObj->EffectOption.DivPhysiDamage;
}

void CAttack::GetPreviewMagicDamage(LPOBJ lpObj,DWORD* DamageMin,DWORD* DamageMax,DWORD* MulDamage,DWORD* DivDamage,DWORD* DamageRate) // OK
{
	CItem* Right = &lpObj->Inventory[0];
	CItem* Left = &lpObj->Inventory[1];

	(*DamageMin) = lpObj->MagicDamageMin;
	(*DamageMax) = lpObj->MagicDamageMax;

	(*DamageMin) += lpObj->EffectOption.AddMagicDamage;
	(*DamageMax) += lpObj->EffectOption.AddMagicDamage;

	(*DamageMin) += lpObj->EffectOption.AddMinMagicDamage;
	(*DamageMax) += lpObj->EffectOption.AddMaxMagicDamage;

	gSkillManager.SkillSwordPowerGetMagicDamage(lpObj->Index,(int*)DamageMin,(int*)DamageMax);

	(*MulDamage) = lpObj->EffectOption.MulMagicDamage;
	(*DivDamage) = lpObj->EffectOption.DivMagicDamage;

	if(Right->IsItem() != 0 && Right->m_IsValidItem != 0 && ((Right->m_Index >= GET_ITEM(0,0) && Right->m_Index < GET_ITEM(1,0)) || (Right->m_Index >= GET_ITEM(5,0) && Right->m_Index < GET_ITEM(6,0))))
	{
		(*DamageRate) = (int)(((Right->m_MagicDamageRate/2)+(Right->m_Level*2))*Right->m_CurrentDurabilityState);
	}
	else
	{
		(*DamageRate) = 0;
	}
}

void CAttack::GetPreviewCurseDamage(LPOBJ lpObj,DWORD* DamageMin,DWORD* DamageMax,DWORD* MulDamage,DWORD* DivDamage,DWORD* DamageRate) // OK
{
	CItem* Right = &lpObj->Inventory[0];
	CItem* Left = &lpObj->Inventory[1];

	(*DamageMin) = lpObj->CurseDamageMin;
	(*DamageMax) = lpObj->CurseDamageMax;

	(*DamageMin) += lpObj->EffectOption.AddCurseDamage;
	(*DamageMax) += lpObj->EffectOption.AddCurseDamage;

	(*DamageMin) += lpObj->EffectOption.AddMinCurseDamage;
	(*DamageMax) += lpObj->EffectOption.AddMaxCurseDamage;

	gSkillManager.SkillSwordPowerGetCurseDamage(lpObj->Index,(int*)DamageMin,(int*)DamageMax);

	(*MulDamage) = lpObj->EffectOption.MulCurseDamage;
	(*DivDamage) = lpObj->EffectOption.DivCurseDamage;

	if(Left->IsItem() != 0 && Left->m_IsValidItem != 0 && Left->m_Index >= GET_ITEM(5,21) && Left->m_Index <= GET_ITEM(5,23))
	{
		(*DamageRate) = (int)(((Left->m_MagicDamageRate/2)+(Left->m_Level*2))*Left->m_CurrentDurabilityState);
	}
	else
	{
		(*DamageRate) = 0;
	}
}

void CAttack::GetPreviewDamageMultiplier(LPOBJ lpObj,DWORD* DamageMultiplier,DWORD* RFDamageMultiplierA,DWORD* RFDamageMultiplierB,DWORD* RFDamageMultiplierC) // OK
{
	switch(lpObj->Class)
	{
		case CLASS_DW:
			(*DamageMultiplier) = 200;
			(*RFDamageMultiplierA) = 100;
			(*RFDamageMultiplierB) = 100;
			(*RFDamageMultiplierC) = 100;
			break;
		case CLASS_DK:
			(*DamageMultiplier) = lpObj->DKDamageMultiplierRate;
			(*RFDamageMultiplierA) = 100;
			(*RFDamageMultiplierB) = 100;
			(*RFDamageMultiplierC) = 100;
			break;
		case CLASS_FE:
			(*DamageMultiplier) = 200;
			(*RFDamageMultiplierA) = 100;
			(*RFDamageMultiplierB) = 100;
			(*RFDamageMultiplierC) = 100;
			break;
		case CLASS_MG:
			(*DamageMultiplier) = 200;
			(*RFDamageMultiplierA) = 100;
			(*RFDamageMultiplierB) = 100;
			(*RFDamageMultiplierC) = 100;
			break;
		case CLASS_DL:
			(*DamageMultiplier) = lpObj->DLDamageMultiplierRate;
			(*RFDamageMultiplierA) = 100;
			(*RFDamageMultiplierB) = 100;
			(*RFDamageMultiplierC) = 100;
			break;
		case CLASS_SU:
			(*DamageMultiplier) = 200;
			(*RFDamageMultiplierA) = 100;
			(*RFDamageMultiplierB) = 100;
			(*RFDamageMultiplierC) = 100;
			break;
		case CLASS_RF:
			(*DamageMultiplier) = 200;
			(*RFDamageMultiplierA) = lpObj->RFDamageMultiplierRate[0];
			(*RFDamageMultiplierB) = lpObj->RFDamageMultiplierRate[1];
			(*RFDamageMultiplierC) = lpObj->RFDamageMultiplierRate[2];
			break;
	}
}

void CAttack::CGAttackRecv(PMSG_ATTACK_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnected(aIndex) == 0)
	{
		return;
	}

	int bIndex = MAKE_NUMBERW(lpMsg->index[0],lpMsg->index[1]);

	if(OBJECT_RANGE(bIndex) == 0)
	{
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if(lpTarget->Live == 0)
	{
		return;
	}

	if(lpObj->Map != lpTarget->Map)
	{
		return;
	}

	#if(GAMESERVER_UPDATE>=402)

	if(gDuel.GetDuelArenaBySpectator(aIndex) != 0 || gDuel.GetDuelArenaBySpectator(bIndex) != 0)
	{
		return;
	}

	#endif

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) != 0 || gMap[lpTarget->Map].CheckAttr(lpTarget->X,lpTarget->Y,1) != 0)
	{
		return;
	}

	if(lpObj->Type == OBJECT_USER && sqrt(pow(((float)lpObj->X-(float)lpTarget->X),2)+pow(((float)lpObj->Y-(float)lpTarget->Y),2)) > ((lpObj->Class==CLASS_FE)?6:3))
	{
		return;
	}

	lpObj->Dir = lpMsg->dir;

	lpObj->MultiSkillIndex = 0;

	lpObj->MultiSkillCount = 0;

	GCActionSend(lpObj,lpMsg->action,aIndex,bIndex);

	lpObj->ComboSkill.Init();

	this->Attack(lpObj,lpTarget,0,0,0,0,0,0);
}
