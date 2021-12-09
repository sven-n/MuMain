// CharacterManager.cpp: implementation of the CCharacterManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CharacterManager.h"
#include "CSItemOption.h"
#include "GlobalText.h"
#include "SkillManager.h"
//#include "CameraMove.h"
//#include "CDirection.h"
//#include "GMBattleCastle.h"
//#include "GMHellas.h"
//#include "GM_Kanturu_3rd.h"
//#include "DSPlaySound.h"
//#include "GlobalBitmap.h"
//#include "GM_Kanturu_2nd.h"
//#include "GM3rdChangeUp.h"
//#include "GMCryWolf1st.h"
//#include "GMDoppelGanger2.h"
//#include "LoadData.h"
//#include "NewUISystem.h"
//#include "PersonalShopTitleImp.h"
//#include "ZzzBMD.h"
#include "ZzzInfomation.h"
//#include "ZzzEffect.h"
//#include "ZzzLodTerrain.h"
//#include "ZzzObject.h"
//#include "ZzzOpenData.h"
//#include "ZzzTexture.h"
//#include "w_CursedTemple.h"
//#include "WSclient.h"
#include "./Utilities/Log/ErrorReport.h"

CCharacterManager gCharacterManager;

CCharacterManager::CCharacterManager() // OK
{
}

CCharacterManager::~CCharacterManager() // OK
{
}

BYTE CCharacterManager::ChangeServerClassTypeToClientClassType(const BYTE byServerClassType)
{
	BYTE byClass = (((byServerClassType >> 4) & 0x01) << 3) | (byServerClassType >> 5) | (((byServerClassType >> 3 ) & 0x01) << 4);	
	return byClass;
}

BYTE CCharacterManager::GetCharacterClass(const BYTE byClass)
{
	BYTE byCharacterClass = 0;
	BYTE byFirstClass = byClass & 0x7;
	BYTE bySecondClass = (byClass >> 3) & 0x01;
	BYTE byThirdClass = (byClass >> 4) & 0x01;

	switch(byFirstClass)
	{
	case 0:
		{
			if(byThirdClass)
			{
				byCharacterClass = CLASS_GRANDMASTER;
			}
			else if(bySecondClass)
			{
				byCharacterClass = CLASS_SOULMASTER;
			}
			else
			{
				byCharacterClass = CLASS_WIZARD;
			}
		}
		
		break;
	case 1:
		{
			if(byThirdClass)
			{
				byCharacterClass = CLASS_BLADEMASTER;
			}
			else if(bySecondClass)
			{
				byCharacterClass = CLASS_BLADEKNIGHT;
			}
			else
			{
				byCharacterClass = CLASS_KNIGHT;
			}
		}
		break;
	case 2:	
		{
			if(byThirdClass)
			{
				byCharacterClass = CLASS_HIGHELF;
			}
			else if(bySecondClass)
			{
				byCharacterClass = CLASS_MUSEELF;
			}
			else
			{
				byCharacterClass = CLASS_ELF;
			}
		}
		break;
	case 3:	
		{
			if(byThirdClass)
			{
				byCharacterClass = CLASS_DUELMASTER;
			}
			else
			{
				byCharacterClass = CLASS_DARK;
			}
		}
		break;
	case 4:	
		{
			if(byThirdClass)
			{
				byCharacterClass = CLASS_LORDEMPEROR;
			}
			else
			{
				byCharacterClass = CLASS_DARK_LORD;
			}
		}
		break;
	case 5:	
		{
			if (byThirdClass)
				byCharacterClass = CLASS_DIMENSIONMASTER;
			else if (bySecondClass)
				byCharacterClass = CLASS_BLOODYSUMMONER;
			else
				byCharacterClass = CLASS_SUMMONER;
		}
		break;
#ifdef PBG_ADD_NEWCHAR_MONK
	case 6:
		{
			if(byThirdClass)
			{
				byCharacterClass = CLASS_TEMPLENIGHT;
			}
			else
			{
				byCharacterClass = CLASS_RAGEFIGHTER;
			}
		}
#endif //PBG_ADD_NEWCHAR_MONK
	}

	return byCharacterClass;
}

bool CCharacterManager::IsSecondClass(const BYTE byClass)
{
	return (((signed int)byClass >> 3) & 1) != 0;;
}

bool CCharacterManager::IsThirdClass(const BYTE byClass)
{
	return (((signed int)byClass >> 4) & 1) != 0;;
}

bool CCharacterManager::IsMasterLevel(const BYTE byClass)
{
	return this->IsThirdClass(byClass);
}

const char* CCharacterManager::GetCharacterClassText(const BYTE byClass)
{
	BYTE byCharacterClass = this->GetCharacterClass(byClass);

	if(byCharacterClass == CLASS_WIZARD)
	{
		return GlobalText[20];
	}
	else if(byCharacterClass == CLASS_SOULMASTER)
	{
		return GlobalText[25];
	}
	else if(byCharacterClass == CLASS_GRANDMASTER)
	{
		return GlobalText[1669];
	}
	else if(byCharacterClass == CLASS_KNIGHT)
	{
		return GlobalText[21];
	}
	else if(byCharacterClass == CLASS_BLADEKNIGHT)
	{
		return GlobalText[26];
	}
	else if(byCharacterClass == CLASS_BLADEMASTER)
	{
		return GlobalText[1668];
	}
	else if(byCharacterClass == CLASS_ELF)
	{
		return GlobalText[22];
	}
	else if(byCharacterClass == CLASS_MUSEELF)
	{
		return GlobalText[27];
	}
	else if(byCharacterClass == CLASS_HIGHELF)
	{
		return GlobalText[1670];
	}
	else if(byCharacterClass == CLASS_DARK)
	{
		return GlobalText[23];
	}
	else if(byCharacterClass == CLASS_DUELMASTER)
	{
		return GlobalText[1671];
	}
	else if(byCharacterClass == CLASS_DARK_LORD)
	{
		return GlobalText[24];
	}
	else if(byCharacterClass == CLASS_LORDEMPEROR)
	{
		return GlobalText[1672];
	}
	else if (byCharacterClass == CLASS_SUMMONER)
		return GlobalText[1687];
	else if (byCharacterClass == CLASS_BLOODYSUMMONER)
		return GlobalText[1688];
	else if (byCharacterClass == CLASS_DIMENSIONMASTER)
		return GlobalText[1689];
#ifdef PBG_ADD_NEWCHAR_MONK
	else if(byCharacterClass == CLASS_RAGEFIGHTER)
		return GlobalText[3150];	// 3150 "레이지파이터"
	else if(byCharacterClass == CLASS_TEMPLENIGHT)
		return GlobalText[3151];	// 3151 "템플나이트"
#endif //PBG_ADD_NEWCHAR_MONK
	return GlobalText[2305];
}

BYTE CCharacterManager::GetSkinModelIndex(const BYTE byClass)
{
	BYTE bySkinIndex = 0;
	BYTE byFirstClass = byClass & 0x7;
	BYTE bySecondClass = (byClass >> 3) & 0x01;
	BYTE byThirdClass = (byClass >> 4) & 0x01;

	if (byFirstClass == CLASS_WIZARD || byFirstClass == CLASS_KNIGHT || byFirstClass == CLASS_ELF || byFirstClass == CLASS_SUMMONER)
	{
		bySkinIndex = byFirstClass + (bySecondClass + byThirdClass) * MAX_CLASS;
	}
	else
	{
		bySkinIndex = byFirstClass + (byThirdClass * 2) * MAX_CLASS;
	}

	return bySkinIndex;
}

BYTE CCharacterManager::GetStepClass(const BYTE byClass)
{
	if(IsThirdClass(byClass))
	{
		return 3;
	}
	else if(this->IsSecondClass(byClass) == true && this->IsThirdClass(byClass) == false)
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

int CCharacterManager::GetEquipedBowType(CHARACTER *pChar)
{
	if( (pChar->Weapon[1].Type != MODEL_BOW+7) && ((pChar->Weapon[1].Type >= MODEL_BOW) && (pChar->Weapon[1].Type < MODEL_BOW+MAX_ITEM_INDEX)))	
	{
		return BOWTYPE_BOW;
	}
	else if( (pChar->Weapon[0].Type != MODEL_BOW+15) && ((pChar->Weapon[0].Type >= MODEL_BOW+8) && (pChar->Weapon[0].Type < MODEL_BOW+MAX_ITEM_INDEX)))
	{
		return BOWTYPE_CROSSBOW;
	}
	return BOWTYPE_NONE;
}

int CCharacterManager::GetEquipedBowType( )
{
	if( (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type != ITEM_BOW+7) && ((CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type >= ITEM_BOW) && (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type < ITEM_BOW+MAX_ITEM_INDEX)))
	{
		return BOWTYPE_BOW;
	}
	else if( (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type != ITEM_BOW+15) && ((CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type >= ITEM_BOW+8) && (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type < ITEM_BOW+MAX_ITEM_INDEX)))
	{
		return BOWTYPE_CROSSBOW;
	}
	return BOWTYPE_NONE;
}

int CCharacterManager::GetEquipedBowType(ITEM* pItem)
{	
	if(((pItem->Type >= ITEM_BOW) && (pItem->Type <= ITEM_BOW+6)) || (pItem->Type == ITEM_BOW+17) || ((pItem->Type >= ITEM_BOW+20) && (pItem->Type <= ITEM_BOW+23)) || (pItem->Type == ITEM_BOW+24))
	{
		return BOWTYPE_BOW;
	}
	
	else if(((pItem->Type >= ITEM_BOW+8) && (pItem->Type <= ITEM_BOW+14)) || (pItem->Type == ITEM_BOW+16) || ((pItem->Type >= ITEM_BOW+18) && (pItem->Type <= ITEM_BOW+19)))
	{
		return BOWTYPE_CROSSBOW;
	}
	return BOWTYPE_NONE;
}

int CCharacterManager::GetEquipedBowType_Skill()
{
	if( (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type != ITEM_BOW+7) && ((CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type >= ITEM_BOW) && (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type < ITEM_BOW+MAX_ITEM_INDEX)))
	{
		if(CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type == ITEM_BOW+15)
			return BOWTYPE_BOW;
	}
	else if( (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type != ITEM_BOW+15) && ((CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type >= ITEM_BOW+8) && (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type < ITEM_BOW+MAX_ITEM_INDEX)))
	{
		if(CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type == ITEM_BOW+7)
			return BOWTYPE_CROSSBOW;
	}
	return BOWTYPE_NONE;
}

bool CCharacterManager::IsEquipedWing()
{
	ITEM* pEquippedItem = &CharacterMachine->Equipment[EQUIPMENT_WING];

	if( (pEquippedItem->Type >= ITEM_WING && pEquippedItem->Type <= ITEM_WING+6) 
		|| (pEquippedItem->Type >= ITEM_WING+36 && pEquippedItem->Type <= ITEM_WING+43)
		|| (pEquippedItem->Type == ITEM_HELPER+30)
		|| ( ITEM_WING+130 <= pEquippedItem->Type && pEquippedItem->Type <= ITEM_WING+134 )
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| (pEquippedItem->Type >= ITEM_WING+49 && pEquippedItem->Type <= ITEM_WING+50)	|| (pEquippedItem->Type == ITEM_WING+135)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)
	{
		return true;
	}

	return false;
}

void CCharacterManager::GetMagicSkillDamage( int iType, int *piMinDamage, int *piMaxDamage)
{
	if (AT_SKILL_SUMMON_EXPLOSION <= iType && iType <= AT_SKILL_SUMMON_REQUIEM)
	{
		*piMinDamage = CharacterMachine->Character.MagicDamageMin;
		*piMaxDamage = CharacterMachine->Character.MagicDamageMax;
		return;
	}

	SKILL_ATTRIBUTE *p = &SkillAttribute[iType];
	
    int Damage = p->Damage;

	*piMinDamage = CharacterMachine->Character.MagicDamageMin+Damage;
	*piMaxDamage = CharacterMachine->Character.MagicDamageMax+Damage+Damage/2;

	Damage = 0;
	g_csItemOption.PlusSpecial ( (WORD*)&Damage,  AT_SET_OPTION_IMPROVE_MAGIC_POWER );
	if(Damage != 0)
	{
		float fratio = 1.f + (float)Damage/100.f;
		*piMinDamage *= fratio;
		*piMaxDamage *= fratio;
	}

    Damage = 0;
    g_csItemOption.ClearListOnOff();
    g_csItemOption.PlusMastery ( &Damage, p->MasteryType );
    g_csItemOption.PlusSpecial ( (WORD*)&Damage,  AT_SET_OPTION_IMPROVE_SKILL_ATTACK );
    *piMinDamage += Damage;
    *piMaxDamage += Damage;
}

void CCharacterManager::GetCurseSkillDamage(int iType, int *piMinDamage, int *piMaxDamage)
{
	if (CLASS_SUMMONER != gCharacterManager.GetBaseClass(CharacterMachine->Character.Class))
		return;

	if (AT_SKILL_SUMMON_EXPLOSION <= iType && iType <= AT_SKILL_SUMMON_REQUIEM)
	{
		SKILL_ATTRIBUTE *p = &SkillAttribute[iType];
		*piMinDamage = CharacterMachine->Character.CurseDamageMin+p->Damage;
		*piMaxDamage = CharacterMachine->Character.CurseDamageMax+p->Damage+p->Damage/2;
	}
	else
	{
		*piMinDamage = CharacterMachine->Character.CurseDamageMin;
		*piMaxDamage = CharacterMachine->Character.CurseDamageMax;
	}
}

void CCharacterManager::GetSkillDamage( int iType, int *piMinDamage, int *piMaxDamage)
{

	SKILL_ATTRIBUTE *p = &SkillAttribute[iType];
	
    int Damage = p->Damage;

	*piMinDamage = Damage;
	*piMaxDamage = Damage+Damage/2;

    Damage = 0;
    g_csItemOption.ClearListOnOff();
    g_csItemOption.PlusMastery ( &Damage, p->MasteryType );
    g_csItemOption.PlusSpecial ( (WORD*)&Damage,  AT_SET_OPTION_IMPROVE_SKILL_ATTACK );
    *piMinDamage += Damage;
    *piMaxDamage += Damage;

}

