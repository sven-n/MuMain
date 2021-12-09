// MonsterSkillElementInfo.h: interface for the CMonsterSkillElementInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

enum MONSTER_SKILL_ELEMENT_TYPE
{
	MSE_TYPE_STUN = 0,
	MSE_TYPE_MOVE = 1,
	MSE_TYPE_HP = 2,
	MSE_TYPE_MP = 3,
	MSE_TYPE_AG = 4,
	MSE_TYPE_DEFENSE = 5,
	MSE_TYPE_ATTACK = 6,
	MSE_TYPE_DURABILITY = 7,
	MSE_TYPE_SUMMON = 8,
	MSE_TYPE_PUSH = 9,
	MSE_TYPE_STAT_ENERGY = 10,
	MSE_TYPE_STAT_STRENGTH = 11,
	MSE_TYPE_STAT_DEXITERITY = 12,
	MSE_TYPE_STAT_VITALITY = 13,
	MSE_TYPE_REMOVE_SKILL = 14,
	MSE_TYPE_RESIST_SKILL = 15,
	MSE_TYPE_IMMUNE_SKILL = 16,
	MSE_TYPE_TELEPORT_SKILL = 17,
	MSE_TYPE_DOUBLE_HP = 18,
	MSE_TYPE_POISON = 19,
	MSE_TYPE_NORMALATTACK = 20,
	MSE_TYPE_BERSERK = 21,
};

enum MONSTER_SKILL_ELEMENT_INCDEC_TYPE
{
	MSE_INCDEC_TYPE_NONE = 0,
	MSE_INCDEC_TYPE_PERCENTINC = 1,
	MSE_INCDEC_TYPE_PERCENTDEC = 2,
	MSE_INCDEC_TYPE_CONSTANTINC = 11,
	MSE_INCDEC_TYPE_CONSTANTDEC = 12,
	MSE_INCDEC_TYPE_CYCLE_PERCENT = 100,
	MSE_INCDEC_TYPE_CYCLE_PERCENTINC = 101,
	MSE_INCDEC_TYPE_CYCLE_PERCENTDEC = 102,
	MSE_INCDEC_TYPE_CYCLE_CONSTANT = 110,
	MSE_INCDEC_TYPE_CYCLE_CONSTANTINC = 111,
	MSE_INCDEC_TYPE_CYCLE_CONSTANTDEC = 112,
};

class CMonsterSkillElementInfo
{
public:
	CMonsterSkillElementInfo();
	virtual ~CMonsterSkillElementInfo();
	void Reset();
	void ForceSkillElementInfo(int aIndex,int bIndex);
	void ApplyElementStun(int aIndex,int bIndex);
	void ApplyElementMove(int aIndex,int bIndex);
	void ApplyElementHP(int aIndex,int bIndex);
	void ApplyElementMP(int aIndex,int bIndex);
	void ApplyElementAG(int aIndex,int bIndex);
	void ApplyElementDefense(int aIndex,int bIndex);
	void ApplyElementAttack(int aIndex,int bIndex);
	void ApplyElementDurability(int aIndex,int bIndex);
	void ApplyElementSummon(int aIndex,int bIndex);
	void ApplyElementPush(int aIndex,int bIndex);
	void ApplyElementStatEnergy(int aIndex,int bIndex);
	void ApplyElementStatStrength(int aIndex,int bIndex);
	void ApplyElementStatDexiterity(int aIndex,int bIndex);
	void ApplyElementStatVitality(int aIndex,int bIndex);
	void ApplyElementRemoveSkill(int aIndex,int bIndex);
	void ApplyElementResistSkill(int aIndex,int bIndex);
	void ApplyElementImmuneSkill(int aIndex,int bIndex);
	void ApplyElementTeleportSkill(int aIndex,int bIndex);
	void ApplyElementDoubleHP(int aIndex,int bIndex);
	void ApplyElementPoison(int aIndex,int bIndex);
	void ApplyElementNormalAttack(int aIndex,int bIndex);
	void ApplyElementBerserk(int aIndex,int bIndex);
public:
	char m_ElementName[50];
	int m_ElementNumber;
	int m_ElementType;
	int m_SuccessRate;
	int m_ContinuanceTime;
	int m_IncAndDecType;
	int m_IncAndDecValue;
	int m_NullifiedSkill;
	int m_CharacterClass;
	int m_CharacterLevel;
};
