// MonsterSkillElementInfo.cpp: implementation of the CMonsterSkillElementInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterSkillElementInfo.h"
#include "Attack.h"
#include "EffectManager.h"
#include "ItemManager.h"
#include "Map.h"
#include "Monster.h"
#include "ObjectManager.h"
#include "SkillManager.h"
#include "User.h"
#include "Util.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterSkillElementInfo::CMonsterSkillElementInfo() // OK
{
	this->Reset();
}

CMonsterSkillElementInfo::~CMonsterSkillElementInfo() // OK
{

}

void CMonsterSkillElementInfo::Reset() // OK
{
	memset(this->m_ElementName,0,sizeof(this->m_ElementName));

	this->m_ElementNumber = -1;
	this->m_ElementType = -1;
	this->m_SuccessRate = -1;
	this->m_ContinuanceTime = -1;
	this->m_IncAndDecType = -1;
	this->m_IncAndDecValue = -1;
	this->m_NullifiedSkill = -1;
	this->m_CharacterClass = -1;
	this->m_CharacterLevel = -1;
}

void CMonsterSkillElementInfo::ForceSkillElementInfo(int aIndex,int bIndex) // OK
{
	if((GetLargeRand()%100) > ((this->m_ElementType==MSE_TYPE_STUN)?(this->m_SuccessRate-gObj[bIndex].ResistStunRate):this->m_SuccessRate))
	{
		return;
	}

	switch(this->m_ElementType)
	{
		case MSE_TYPE_STUN:
			this->ApplyElementStun(aIndex,bIndex);
			break;
		case MSE_TYPE_MOVE:
			this->ApplyElementMove(aIndex,bIndex);
			break;
		case MSE_TYPE_HP:
			this->ApplyElementHP(aIndex,bIndex);
			break;
		case MSE_TYPE_MP:
			this->ApplyElementMP(aIndex,bIndex);
			break;
		case MSE_TYPE_AG:
			this->ApplyElementAG(aIndex,bIndex);
			break;
		case MSE_TYPE_DEFENSE:
			this->ApplyElementDefense(aIndex,bIndex);
			break;
		case MSE_TYPE_ATTACK:
			this->ApplyElementAttack(aIndex,bIndex);
			break;
		case MSE_TYPE_DURABILITY:
			this->ApplyElementDurability(aIndex,bIndex);
			break;
		case MSE_TYPE_SUMMON:
			this->ApplyElementSummon(aIndex,bIndex);
			break;
		case MSE_TYPE_PUSH:
			this->ApplyElementPush(aIndex,bIndex);
			break;
		case MSE_TYPE_STAT_ENERGY:
			this->ApplyElementStatEnergy(aIndex,bIndex);
			break;
		case MSE_TYPE_STAT_STRENGTH:
			this->ApplyElementStatStrength(aIndex,bIndex);
			break;
		case MSE_TYPE_STAT_DEXITERITY:
			this->ApplyElementStatDexiterity(aIndex,bIndex);
			break;
		case MSE_TYPE_STAT_VITALITY:
			this->ApplyElementStatVitality(aIndex,bIndex);
			break;
		case MSE_TYPE_REMOVE_SKILL:
			this->ApplyElementRemoveSkill(aIndex,bIndex);
			break;
		case MSE_TYPE_RESIST_SKILL:
			this->ApplyElementResistSkill(aIndex,bIndex);
			break;
		case MSE_TYPE_IMMUNE_SKILL:
			this->ApplyElementImmuneSkill(aIndex,bIndex);
			break;
		case MSE_TYPE_TELEPORT_SKILL:
			this->ApplyElementTeleportSkill(aIndex,bIndex);
			break;
		case MSE_TYPE_DOUBLE_HP:
			this->ApplyElementDoubleHP(aIndex,bIndex);
			break;
		case MSE_TYPE_POISON:
			this->ApplyElementPoison(aIndex,bIndex);
			break;
		case MSE_TYPE_NORMALATTACK:
			this->ApplyElementNormalAttack(aIndex,bIndex);
			break;
		case MSE_TYPE_BERSERK:
			this->ApplyElementBerserk(aIndex,bIndex);
			break;
	}
}

void CMonsterSkillElementInfo::ApplyElementStun(int aIndex,int bIndex) // OK
{
	if(gObjIsConnected(bIndex) == 0)
	{
		return;
	}

	if(gEffectManager.CheckEffect(&gObj[bIndex],EFFECT_IRON_DEFENSE) != 0 || gEffectManager.CheckEffect(&gObj[bIndex],EFFECT_IRON_DEFENSE_IMPROVED) != 0)
	{
		return;
	}

	if(this->m_ContinuanceTime == -1)
	{
		gEffectManager.AddEffect(&gObj[bIndex],0,EFFECT_STERN,0,0,0,0,0);
	}
	else
	{
		gEffectManager.AddEffect(&gObj[bIndex],0,EFFECT_STERN,this->m_ContinuanceTime,0,0,0,0);
	}

	gObjSetPosition(bIndex,gObj[bIndex].X,gObj[bIndex].Y);
}

void CMonsterSkillElementInfo::ApplyElementMove(int aIndex,int bIndex) // OK
{

}

void CMonsterSkillElementInfo::ApplyElementHP(int aIndex,int bIndex) // OK
{
	int IncAndDecValue = 0;

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_PERCENTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTINC)
	{
		IncAndDecValue += (int)((gObj[bIndex].Life*this->m_IncAndDecValue)/100);
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_PERCENTDEC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTDEC)
	{
		IncAndDecValue -= (int)((gObj[bIndex].Life*this->m_IncAndDecValue)/100);
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CONSTANTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTINC)
	{
		IncAndDecValue += this->m_IncAndDecValue;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CONSTANTDEC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTDEC)
	{
		IncAndDecValue -= this->m_IncAndDecValue;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENT || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTDEC)
	{
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoHPTime = this->m_ContinuanceTime;
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoHP = IncAndDecValue;
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoHPCycle = MSE_INCDEC_TYPE_CYCLE_PERCENT;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANT || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTDEC)
	{
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoHPTime = this->m_ContinuanceTime;
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoHP = IncAndDecValue;
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoHPCycle = MSE_INCDEC_TYPE_CYCLE_CONSTANT;
	}

	gObj[bIndex].Life += IncAndDecValue;

	gObj[bIndex].Life = ((gObj[bIndex].Life<0)?0:gObj[bIndex].Life);

	gObj[bIndex].Life = ((gObj[bIndex].Life>(gObj[bIndex].MaxLife+gObj[bIndex].AddLife))?(gObj[bIndex].MaxLife+gObj[bIndex].AddLife):gObj[bIndex].Life);

	GCLifeSend(bIndex,0xFF,(int)gObj[bIndex].Life,gObj[bIndex].Shield);
}

void CMonsterSkillElementInfo::ApplyElementMP(int aIndex,int bIndex) // OK
{
	int IncAndDecValue = 0;

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_PERCENTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTINC)
	{
		IncAndDecValue += (int)((gObj[bIndex].Mana*this->m_IncAndDecValue)/100);
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_PERCENTDEC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTDEC)
	{
		IncAndDecValue -= (int)((gObj[bIndex].Mana*this->m_IncAndDecValue)/100);
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CONSTANTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTINC)
	{
		IncAndDecValue += this->m_IncAndDecValue;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CONSTANTDEC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTDEC)
	{
		IncAndDecValue -= this->m_IncAndDecValue;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENT || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTDEC)
	{
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoMPTime = this->m_ContinuanceTime;
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoMP = IncAndDecValue;
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoMPCycle = MSE_INCDEC_TYPE_CYCLE_PERCENT;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANT || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTDEC)
	{
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoMPTime = this->m_ContinuanceTime;
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoMP = IncAndDecValue;
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoMPCycle = MSE_INCDEC_TYPE_CYCLE_CONSTANT;
	}

	gObj[bIndex].Mana += IncAndDecValue;

	gObj[bIndex].Mana = ((gObj[bIndex].Mana<0)?0:gObj[bIndex].Mana);

	gObj[bIndex].Mana = ((gObj[bIndex].Mana>(gObj[bIndex].MaxMana+gObj[bIndex].AddMana))?(gObj[bIndex].MaxMana+gObj[bIndex].AddMana):gObj[bIndex].Mana);

	GCManaSend(bIndex,0xFF,(int)gObj[bIndex].Mana,gObj[bIndex].BP);
}

void CMonsterSkillElementInfo::ApplyElementAG(int aIndex,int bIndex) // OK
{
	int IncAndDecValue = 0;

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_PERCENTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTINC)
	{
		IncAndDecValue += (int)((gObj[bIndex].BP*this->m_IncAndDecValue)/100);
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_PERCENTDEC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTDEC)
	{
		IncAndDecValue -= (int)((gObj[bIndex].BP*this->m_IncAndDecValue)/100);
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CONSTANTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTINC)
	{
		IncAndDecValue += this->m_IncAndDecValue;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CONSTANTDEC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTDEC)
	{
		IncAndDecValue -= this->m_IncAndDecValue;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENT || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTDEC)
	{
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoAGTime = this->m_ContinuanceTime;
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoAG = IncAndDecValue;
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoAGCycle = MSE_INCDEC_TYPE_CYCLE_PERCENT;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANT || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTDEC)
	{
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoAGTime = this->m_ContinuanceTime;
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoAG = IncAndDecValue;
		gObj[bIndex].MonsterSkillElementOption.m_SkillElementAutoAGCycle = MSE_INCDEC_TYPE_CYCLE_CONSTANT;
	}

	gObj[bIndex].BP += IncAndDecValue;

	gObj[bIndex].BP = ((gObj[bIndex].BP<0)?0:gObj[bIndex].BP);

	gObj[bIndex].BP = ((gObj[bIndex].BP>(gObj[bIndex].MaxBP+gObj[bIndex].AddBP))?(gObj[bIndex].MaxBP+gObj[bIndex].AddBP):gObj[bIndex].BP);

	GCManaSend(bIndex,0xFF,(int)gObj[bIndex].Mana,gObj[bIndex].BP);
}

void CMonsterSkillElementInfo::ApplyElementDefense(int aIndex,int bIndex) // OK
{
	int IncAndDecValue = 0;

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_PERCENTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTINC)
	{
		IncAndDecValue += (gObj[bIndex].Defense*this->m_IncAndDecValue)/100;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_PERCENTDEC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTDEC)
	{
		IncAndDecValue -= (gObj[bIndex].Defense*this->m_IncAndDecValue)/100;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CONSTANTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTINC)
	{
		IncAndDecValue += this->m_IncAndDecValue;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CONSTANTDEC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTDEC)
	{
		IncAndDecValue -= this->m_IncAndDecValue;
	}

	gObj[bIndex].MonsterSkillElementOption.m_SkillElementDefenseTime = this->m_ContinuanceTime;
	gObj[bIndex].MonsterSkillElementOption.m_SkillElementDefense = IncAndDecValue;
}

void CMonsterSkillElementInfo::ApplyElementAttack(int aIndex,int bIndex) // OK
{
	int IncAndDecValue = 0;

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_PERCENTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTINC)
	{
		IncAndDecValue += (((gObj[bIndex].PhysiDamageMin+gObj[bIndex].PhysiDamageMax)/2)*this->m_IncAndDecValue)/100;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_PERCENTDEC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTDEC)
	{
		IncAndDecValue -= (((gObj[bIndex].PhysiDamageMin+gObj[bIndex].PhysiDamageMax)/2)*this->m_IncAndDecValue)/100;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CONSTANTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTINC)
	{
		IncAndDecValue += this->m_IncAndDecValue;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CONSTANTDEC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTDEC)
	{
		IncAndDecValue -= this->m_IncAndDecValue;
	}

	gObj[bIndex].MonsterSkillElementOption.m_SkillElementAttackTime = this->m_ContinuanceTime;
	gObj[bIndex].MonsterSkillElementOption.m_SkillElementAttack = IncAndDecValue;
}

void CMonsterSkillElementInfo::ApplyElementDurability(int aIndex,int bIndex) // OK
{
	if(gObj[bIndex].Type != OBJECT_USER)
	{
		return;
	}

	int IncAndDecSlot = (GetLargeRand()%6)+2;

	int IncAndDecValue = 0;

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_PERCENTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTINC)
	{
		IncAndDecValue += (int)((gObj[bIndex].Inventory[IncAndDecSlot].m_Durability*this->m_IncAndDecValue)/100);
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_PERCENTDEC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_PERCENTDEC)
	{
		IncAndDecValue -= (int)((gObj[bIndex].Inventory[IncAndDecSlot].m_Durability*this->m_IncAndDecValue)/100);
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CONSTANTINC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTINC)
	{
		IncAndDecValue += this->m_IncAndDecValue;
	}

	if(this->m_IncAndDecType == MSE_INCDEC_TYPE_CONSTANTDEC || this->m_IncAndDecType == MSE_INCDEC_TYPE_CYCLE_CONSTANTDEC)
	{
		IncAndDecValue -= this->m_IncAndDecValue;
	}

	gObj[bIndex].Inventory[IncAndDecSlot].m_Durability += IncAndDecValue;

	gObj[bIndex].Inventory[IncAndDecSlot].m_Durability = ((gObj[bIndex].Inventory[IncAndDecSlot].m_Durability<0)?0:gObj[bIndex].Inventory[IncAndDecSlot].m_Durability);

	gItemManager.GCItemDurSend(bIndex,IncAndDecSlot,(BYTE)gObj[bIndex].Inventory[IncAndDecSlot].m_Durability,0);
}

void CMonsterSkillElementInfo::ApplyElementSummon(int aIndex,int bIndex) // OK
{
	if(gObjIsConnected(bIndex) == 0 || gObj[bIndex].Type != OBJECT_MONSTER)
	{
		return;
	}

	if(gObj[aIndex].Class == 459 && gObj[aIndex].Map == MAP_RAKLION2 && aIndex == bIndex)
	{
		return;
	}

	gObj[bIndex].Live = 1;
	gObj[bIndex].Life = gObj[bIndex].MaxLife+gObj[bIndex].AddLife;
	gObj[bIndex].Mana = gObj[bIndex].MaxMana+gObj[bIndex].MaxMana;
	gObj[bIndex].TargetNumber = -1;
	gObj[bIndex].LastAttackerID = -1;
	gObj[bIndex].NextActionTime = 5000;
	gObj[bIndex].ActionState.Rest = 0;
	gObj[bIndex].ActionState.Attack = 0;
	gObj[bIndex].ActionState.Move = 0;
	gObj[bIndex].ActionState.Escape = 0;
	gObj[bIndex].ActionState.Emotion = 0;
	gObj[bIndex].ActionState.EmotionCount = 0;
	gObj[bIndex].DieRegen = 0;
	gObj[bIndex].State = OBJECT_CREATE;
	gObj[bIndex].Agro.ResetAll();

	gEffectManager.ClearAllEffect(&gObj[bIndex]);

	int px = gObj[aIndex].X;
	int py = gObj[aIndex].Y;

	if(gObjGetRandomFreeLocation(gObj[aIndex].Map,&px,&py,((this->m_IncAndDecValue<0)?10:this->m_IncAndDecValue),((this->m_IncAndDecValue<0)?10:this->m_IncAndDecValue),100) == 0)
	{
		px = gObj[aIndex].X;
		py = gObj[aIndex].Y;
	}

	gObj[bIndex].X = px;
	gObj[bIndex].Y = py;
	gObj[bIndex].TX = px;
	gObj[bIndex].TY = py;
	gObj[bIndex].MTX = px;
	gObj[bIndex].MTY = py;
	gObj[bIndex].OldX = px;
	gObj[bIndex].OldY = py;
	gObj[bIndex].StartX = px;
	gObj[bIndex].StartY = py;

	gObjMonsterInitHitDamage(&gObj[bIndex]);

	gObjViewportListCreate(bIndex);
}

void CMonsterSkillElementInfo::ApplyElementPush(int aIndex,int bIndex) // OK
{
	if(gObjIsConnected(bIndex) == 0)
	{
		return;
	}

	if(gObj[aIndex].Class == 459 && gObj[aIndex].Map == MAP_RAKLION2 && aIndex == bIndex)
	{
		return;
	}

	if(this->m_NullifiedSkill < 1)
	{
		gObjBackSpring2(&gObj[bIndex],&gObj[aIndex],((this->m_IncAndDecValue<1)?3:this->m_IncAndDecValue));
	}
	else
	{
		gObjAddMsgSendDelay(&gObj[aIndex],57,bIndex,this->m_NullifiedSkill,((this->m_IncAndDecValue<1)?3:this->m_IncAndDecValue));
	}
}

void CMonsterSkillElementInfo::ApplyElementStatEnergy(int aIndex,int bIndex) // OK
{

}

void CMonsterSkillElementInfo::ApplyElementStatStrength(int aIndex,int bIndex) // OK
{

}

void CMonsterSkillElementInfo::ApplyElementStatDexiterity(int aIndex,int bIndex) // OK
{

}

void CMonsterSkillElementInfo::ApplyElementStatVitality(int aIndex,int bIndex) // OK
{

}

void CMonsterSkillElementInfo::ApplyElementRemoveSkill(int aIndex,int bIndex) // OK
{
	if(gObjIsConnected(bIndex) == 0)
	{
		return;
	}

	if(gObj[bIndex].Type != OBJECT_USER)
	{
		return;
	}

	if(this->m_NullifiedSkill == SKILL_GREATER_DAMAGE)
	{
		gEffectManager.DelEffect(&gObj[bIndex],EFFECT_GREATER_DAMAGE);
	}

	if(this->m_NullifiedSkill == SKILL_GREATER_DEFENSE)
	{
		gEffectManager.DelEffect(&gObj[bIndex],EFFECT_GREATER_DEFENSE);
	}
}

void CMonsterSkillElementInfo::ApplyElementResistSkill(int aIndex,int bIndex) // OK
{
	gObj[aIndex].MonsterSkillElementOption.m_SkillElementResistNumber = this->m_NullifiedSkill;

	gObj[aIndex].MonsterSkillElementOption.m_SkillElementResistTime = this->m_ContinuanceTime;
}

void CMonsterSkillElementInfo::ApplyElementImmuneSkill(int aIndex,int bIndex) // OK
{
	gObj[aIndex].MonsterSkillElementOption.m_SkillElementImmuneNumber = this->m_NullifiedSkill;

	gObj[aIndex].MonsterSkillElementOption.m_SkillElementImmuneTime = this->m_ContinuanceTime;

	if(this->m_ContinuanceTime == -1)
	{
		gEffectManager.AddEffect(&gObj[aIndex],0,EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY,0,0,0,0,0);
	}
	else
	{
		gEffectManager.AddEffect(&gObj[aIndex],0,EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY,this->m_ContinuanceTime,0,0,0,0);
	}
}

void CMonsterSkillElementInfo::ApplyElementTeleportSkill(int aIndex,int bIndex) // OK
{
	if(gEffectManager.CheckStunEffect(&gObj[aIndex]) != 0)
	{
		return;
	}

	int px = gObj[aIndex].X;
	int py = gObj[aIndex].Y;

	if(gObjGetRandomFreeLocation(gObj[aIndex].Map,&px,&py,5,5,100) == 0)
	{
		px = gObj[aIndex].X;
		py = gObj[aIndex].Y;
	}

	if(gObjCheckTeleportArea(aIndex,px,py) == 0)
	{
		return;
	}

	gSkillManager.GCSkillAttackSend(&gObj[aIndex],SKILL_TELEPORT,aIndex,1);

	gObjTeleportMagicUse(aIndex,px,py);

	gObj[aIndex].TargetNumber = -1;
}

void CMonsterSkillElementInfo::ApplyElementDoubleHP(int aIndex,int bIndex) // OK
{
	if(gObjIsConnected(bIndex) == 0)
	{
		return;
	}

	gObjAddMsgSendDelay(&gObj[aIndex],55,bIndex,400,0);

	gObjAddMsgSendDelay(&gObj[aIndex],55,bIndex,1000,0);
}

void CMonsterSkillElementInfo::ApplyElementPoison(int aIndex,int bIndex) // OK
{
	if(gObjIsConnected(bIndex) == 0)
	{
		return;
	}

	if(this->m_NullifiedSkill < 1)
	{
		if(gObjCheckResistance(&gObj[bIndex],1) == 0)
		{
			gEffectManager.AddEffect(&gObj[bIndex],0,EFFECT_POISON,this->m_IncAndDecValue,aIndex,2,3,0);
		}
	}
	else
	{
		gObjAddMsgSendDelay(&gObj[aIndex],56,bIndex,this->m_NullifiedSkill,this->m_IncAndDecValue);
	}
}

void CMonsterSkillElementInfo::ApplyElementNormalAttack(int aIndex,int bIndex) // OK
{
	if(gObjIsConnected(bIndex) == 0)
	{
		return;
	}

	if(this->m_IncAndDecType != -1 && this->m_IncAndDecValue != -1)
	{
		if(gObj[bIndex].Inventory[9].m_Index != GET_ITEM(this->m_IncAndDecType,this->m_IncAndDecValue) && gObj[bIndex].Inventory[10].m_Index != GET_ITEM(this->m_IncAndDecType,this->m_IncAndDecValue) && gObj[bIndex].Inventory[11].m_Index != GET_ITEM(this->m_IncAndDecType,this->m_IncAndDecValue))
		{
			gObj[bIndex].Life = 0;
			gObjectManager.CharacterLifeCheck(&gObj[aIndex],&gObj[bIndex],(int)(gObj[bIndex].MaxLife+gObj[bIndex].AddLife),0,0,0,0,0);
			return;
		}
	}

	if(this->m_NullifiedSkill < 1)
	{
		gAttack.Attack(&gObj[aIndex],&gObj[bIndex],0,0,0,0,0,0);
	}
	else
	{
		gObjAddMsgSendDelay(&gObj[aIndex],55,bIndex,this->m_NullifiedSkill,0);
	}
}

void CMonsterSkillElementInfo::ApplyElementBerserk(int aIndex,int bIndex) // OK
{
	if(gObjIsConnected(bIndex) == 0)
	{
		return;
	}

	if(this->m_ContinuanceTime == -1)
	{
		gEffectManager.AddEffect(&gObj[bIndex],0,EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY,0,0,0,0,0);
		gEffectManager.AddEffect(&gObj[bIndex],0,EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY,0,0,0,0,0);
	}
	else
	{
		gEffectManager.AddEffect(&gObj[bIndex],0,EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY,this->m_ContinuanceTime,0,0,0,0);
		gEffectManager.AddEffect(&gObj[bIndex],0,EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY,this->m_ContinuanceTime,0,0,0,0);
	}

	gObj[bIndex].MonsterSkillElementOption.m_SkillElementBerserkTime = this->m_ContinuanceTime;
}
