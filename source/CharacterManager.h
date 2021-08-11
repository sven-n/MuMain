// CharacterManager.h: interface for the CCharacterManager class.
//////////////////////////////////////////////////////////////////////
#pragma once


class CCharacterManager
{
public:
	CCharacterManager();
	virtual ~CCharacterManager();
	BYTE ChangeServerClassTypeToClientClassType(const BYTE byServerClassType);
	BYTE GetCharacterClass(const BYTE byClass);
	bool IsSecondClass(const BYTE byClass);
	bool IsThirdClass(const BYTE byClass);
	bool IsMasterLevel(const BYTE byClass);
	const char* GetCharacterClassText(const BYTE byClass);
	int GetBaseClass( int iClass)	{ return ( 0x7 & iClass); }
	int IsFemale(int iClass) { return (this->GetBaseClass(iClass) == CLASS_ELF || this->GetBaseClass(iClass) == CLASS_SUMMONER); }
	BYTE GetSkinModelIndex(const BYTE byClass);
	BYTE GetStepClass(const BYTE byClass);
	int GetEquipedBowType(CHARACTER *pChar);
	int GetEquipedBowType();
	int GetEquipedBowType(ITEM* pItem);
	int GetEquipedBowType_Skill( );
	bool IsEquipedWing();
	void GetMagicSkillDamage( int iType, int *piMinDamage, int *piMaxDamage);
	void GetCurseSkillDamage(int iType, int *piMinDamage, int *piMaxDamage);
    void GetSkillDamage( int iType, int *piMinDamage, int *piMaxDamage);
public:

};

extern CCharacterManager gCharacterManager;
