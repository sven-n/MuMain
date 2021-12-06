#include "stdafx.h"
#include "SocketSystem.h"
#include "./Utilities/Log/ErrorReport.h"
#include "ZzzInventory.h"
#include "ZzzBMD.h"
#include "ZzzCharacter.h"
#include "WSclient.h"
#include "UIControls.h"
#include "CharacterManager.h"

using namespace SEASON4A;

static BYTE bBuxCode[3] = {0xfc,0xcf,0xab};

static void BuxConvert(BYTE *Buffer,int Size)
{
	for(int i=0;i<Size;i++)
		Buffer[i] ^= bBuxCode[i%3];
}

CSocketItemMgr g_SocketItemMgr;

CSocketItemMgr::CSocketItemMgr()
{
	m_iNumEquitSetBonusOptions = 0;
	memset(m_SocketOptionInfo, 0, sizeof(SOCKET_OPTION_INFO) * MAX_SOCKET_OPTION);
	memset(&m_StatusBonus, 0, sizeof(SOCKET_OPTION_STATUS_BONUS));
}

CSocketItemMgr::~CSocketItemMgr()
{

}

BOOL CSocketItemMgr::IsSocketItem(const ITEM* pItem)
{
	return IsSocketItem(pItem->Type);
}

BOOL CSocketItemMgr::IsSocketItem(const OBJECT* pObject)
{
	return IsSocketItem(pObject->Type - MODEL_SWORD);
}

BOOL CSocketItemMgr::IsSocketItem(int iItemType)
{
	switch (iItemType)
	{
	case ITEM_SWORD+26:
	case ITEM_SWORD+27:
	case ITEM_SWORD+28:
	case ITEM_MACE+16:
	case ITEM_MACE+17:
	case ITEM_BOW+23:
	case ITEM_STAFF+30:
	case ITEM_STAFF+31:
	case ITEM_STAFF+32:	
	case ITEM_SHIELD+17:
	case ITEM_SHIELD+18:
	case ITEM_SHIELD+19:
	case ITEM_SHIELD+20:
	case ITEM_HELM+45: case ITEM_ARMOR+45: case ITEM_PANTS+45: case ITEM_GLOVES+45: case ITEM_BOOTS+45:
	case ITEM_HELM+46: case ITEM_ARMOR+46: case ITEM_PANTS+46: case ITEM_GLOVES+46: case ITEM_BOOTS+46:
	case ITEM_HELM+47: case ITEM_ARMOR+47: case ITEM_PANTS+47: case ITEM_GLOVES+47: case ITEM_BOOTS+47:
	case ITEM_HELM+48: case ITEM_ARMOR+48: case ITEM_PANTS+48: case ITEM_GLOVES+48: case ITEM_BOOTS+48:
	case ITEM_HELM+49: case ITEM_ARMOR+49: case ITEM_PANTS+49: case ITEM_GLOVES+49: case ITEM_BOOTS+49:	
	case ITEM_HELM+50: case ITEM_ARMOR+50: case ITEM_PANTS+50: case ITEM_GLOVES+50: case ITEM_BOOTS+50:
	case ITEM_HELM+51: case ITEM_ARMOR+51: case ITEM_PANTS+51: case ITEM_GLOVES+51: case ITEM_BOOTS+51:
	case ITEM_HELM+52: case ITEM_ARMOR+52: case ITEM_PANTS+52: case ITEM_GLOVES+52: case ITEM_BOOTS+52:
	case ITEM_HELM+53: case ITEM_ARMOR+53: case ITEM_PANTS+53: case ITEM_GLOVES+53: case ITEM_BOOTS+53:
		return TRUE;
	default:
		return FALSE;
	}
// 	return (pItem->SocketCount > 0);
}

int CSocketItemMgr::GetSocketCategory(int iSeedID)
{
	if (iSeedID == SOCKET_EMPTY) return SOCKET_EMPTY;

	SOCKET_OPTION_INFO * pInfo = &m_SocketOptionInfo[SOT_SOCKET_ITEM_OPTIONS][iSeedID];
	return pInfo->m_iOptionCategory;
}

int CSocketItemMgr::GetSeedShpereSeedID(const ITEM* pItem)
{
	BYTE bySocketSeedID = SOCKET_EMPTY;

	if(pItem->Type >= ITEM_WING+100 && pItem->Type <= ITEM_WING+129)
	{
		int iCategoryIndex = (pItem->Type - (ITEM_WING+100)) % 6 + 1;
		int iLevel = (pItem->Level>>3)&15;
		switch(iCategoryIndex)
		{
		case 1:	// 0~9
			bySocketSeedID = 0 + iLevel;
			break;
		case 2:	// 10~15
			bySocketSeedID = 10 + iLevel;
			break;
		case 3:	// 16~20
			bySocketSeedID = 16 + iLevel;
			break;
		case 4:	// 21~28
			bySocketSeedID = 21 + iLevel;
			break;
		case 5:	// 29~33
			bySocketSeedID = 29 + iLevel;
			break;
		case 6:	// 34~40
			bySocketSeedID = 34 + iLevel;
			break;
		}
	}

	return bySocketSeedID;
}

#ifdef KJW_FIX_SOCKET_BONUS_BIT_OPERATION
__int64 CSocketItemMgr::CalcSocketBonusItemValue( const ITEM * pItem, __int64 iOrgGold)
#else // KJW_FIX_SOCKET_BONUS_BIT_OPERATION
int CSocketItemMgr::CalcSocketBonusItemValue( const ITEM * pItem, int iOrgGold)
#endif // KJW_FIX_SOCKET_BONUS_BIT_OPERATION
{
#ifdef KJW_FIX_SOCKET_BONUS_BIT_OPERATION
	__int64 iGoldResult = 0;
#else // KJW_FIX_SOCKET_BONUS_BIT_OPERATION
	int iGoldResult = 0;
#endif // KJW_FIX_SOCKET_BONUS_BIT_OPERATION
	
	if (IsSocketItem(pItem))
	{
		iGoldResult += iOrgGold * (pItem->SocketCount * 0.8f);

		ITEM TempSeedSphere;
		for (int i = 0; i < pItem->SocketCount; ++i)
		{
			if (pItem->SocketSeedID[i] == SOCKET_EMPTY) continue;

			int iSeedSphereType = 0;
			if (pItem->SocketSeedID[i] >= 0 && pItem->SocketSeedID[i] <= 9) iSeedSphereType = 0;
			else if (pItem->SocketSeedID[i] >= 10 && pItem->SocketSeedID[i] <= 15) iSeedSphereType = 1;
			else if (pItem->SocketSeedID[i] >= 16 && pItem->SocketSeedID[i] <= 20) iSeedSphereType = 2;
			else if (pItem->SocketSeedID[i] >= 21 && pItem->SocketSeedID[i] <= 28) iSeedSphereType = 3;
			else if (pItem->SocketSeedID[i] >= 29 && pItem->SocketSeedID[i] <= 33) iSeedSphereType = 4;
			else if (pItem->SocketSeedID[i] >= 34 && pItem->SocketSeedID[i] <= 40) iSeedSphereType = 5;

			TempSeedSphere.Type = ITEM_WING+100 + (pItem->SocketSphereLv[i] - 1) * MAX_SOCKET_TYPES + iSeedSphereType;
			iGoldResult += ItemValue(&TempSeedSphere, 0);
		}
	}

	return iGoldResult;
}

int CSocketItemMgr::CalcSocketOptionValue(int iOptionType, float fOptionValue)
{
	switch(iOptionType)
	{
	case 1:
		return int(fOptionValue);
	case 2:
		return int(fOptionValue);
	case 3:
		{
			WORD wLevel;

			if(gCharacterManager.IsMasterLevel(CharacterAttribute->Class) == true)
				wLevel = CharacterAttribute->Level + Master_Level_Data.nMLevel;
			else
				wLevel = CharacterAttribute->Level;
			
			return int((float)wLevel / fOptionValue);
		}
	case 4:
		{
			WORD wLifeMax;

			if(gCharacterManager.IsMasterLevel( Hero->Class ) == true )
				wLifeMax = Master_Level_Data.wMaxLife;
			else
				wLifeMax = CharacterAttribute->LifeMax;

			return int((float)wLifeMax / fOptionValue);
		}
	case 5:
		{
			WORD wManaMax;
			if(gCharacterManager.IsMasterLevel( Hero->Class ) == true )
				wManaMax = Master_Level_Data.wMaxMana;
			else
				wManaMax = CharacterAttribute->ManaMax;

			return int((float)wManaMax / fOptionValue);
		}
	default:
		return 0;
	}
}

void CSocketItemMgr::CalcSocketOptionValueText(char * pszOptionValueText, int iOptionType, float fOptionValue)
{
	switch(iOptionType)
	{
	case 1:
		sprintf(pszOptionValueText, "+%d", CalcSocketOptionValue(iOptionType, fOptionValue));
		break;
	case 2:
		sprintf(pszOptionValueText, "+%d%%", CalcSocketOptionValue(iOptionType, fOptionValue));
		break;
	case 3:
		sprintf(pszOptionValueText, "+%d", CalcSocketOptionValue(iOptionType, fOptionValue));
		break;
	case 4:
		sprintf(pszOptionValueText, "+%d", CalcSocketOptionValue(iOptionType, fOptionValue));
		break;
	case 5:
		sprintf(pszOptionValueText, "+%d", CalcSocketOptionValue(iOptionType, fOptionValue));
		break;
	}
}

void CSocketItemMgr::CreateSocketOptionText(char * pszOptionText, int iSeedID, int iSphereLv)
{
	if (pszOptionText == NULL) return;

	char szOptionValueText[16] = { 0, };

	SOCKET_OPTION_INFO * pInfo = &m_SocketOptionInfo[SOT_SOCKET_ITEM_OPTIONS][iSeedID];

	float fOptionValue = (float)pInfo->m_iOptionValue[iSphereLv - 1];

	CalcSocketOptionValueText(szOptionValueText, pInfo->m_bOptionType, fOptionValue);

	sprintf(pszOptionText, "%s(%s %s)", GlobalText[2640 + pInfo->m_iOptionCategory - 1], pInfo->m_szOptionName, szOptionValueText);
}

extern int SkipNum;

int CSocketItemMgr::AttachToolTipForSocketItem(const ITEM* pItem, int iTextNum)
{
	if (pItem->SocketCount == 0) return iTextNum;

	sprintf ( TextList[iTextNum], "\n" ); ++iTextNum; ++SkipNum;
	sprintf ( TextList[iTextNum], "%s %s", GlobalText[2650], GlobalText[159] );
	TextListColor[iTextNum] = TEXT_COLOR_PURPLE; 
	TextBold[iTextNum] = false;
	++iTextNum;
	sprintf(TextList[iTextNum], "\n"); ++iTextNum; ++SkipNum;

	char szOptionText[64] = { 0, };
	char szOptionValueText[16] = { 0, };
	SOCKET_OPTION_INFO * pInfo = NULL;
	for (int i = 0; i < pItem->SocketCount; ++i)
	{
		if (pItem->SocketSeedID[i] == SOCKET_EMPTY)
		{
			sprintf(szOptionText, GlobalText[2652]);
			TextListColor[iTextNum] = TEXT_COLOR_GRAY; 
		}
		else if (pItem->SocketSeedID[i] < MAX_SOCKET_OPTION)
		{
			CreateSocketOptionText(szOptionText, pItem->SocketSeedID[i], pItem->SocketSphereLv[i]);
			TextListColor[iTextNum] = TEXT_COLOR_BLUE; 
		}
		else
		{
			assert(!"소켓 인덱스 에러");
		}

		sprintf(TextList[iTextNum], GlobalText[2655], i + 1, szOptionText);
		TextBold[iTextNum] = false;
		++iTextNum;
	}

	if (pItem->SocketSeedSetOption < MAX_SOCKET_OPTION)
	{
		sprintf(TextList[iTextNum], "\n"); ++iTextNum; ++SkipNum;

		sprintf ( TextList[iTextNum], "%s", GlobalText[2656]);
		TextListColor[iTextNum] = TEXT_COLOR_PURPLE; 
		TextBold[iTextNum] = false;
		++iTextNum;
		sprintf(TextList[iTextNum], "\n"); ++iTextNum; ++SkipNum;

		pInfo = &m_SocketOptionInfo[SOT_MIX_SET_BONUS_OPTIONS][pItem->SocketSeedSetOption];
		CalcSocketOptionValueText(szOptionValueText, pInfo->m_bOptionType, (float)pInfo->m_iOptionValue[0]);
		sprintf(TextList[iTextNum], "%s %s", pInfo->m_szOptionName, szOptionValueText);
		TextListColor[iTextNum] = TEXT_COLOR_BLUE; 
		TextBold[iTextNum] = false;
		++iTextNum;
	}
	return iTextNum;
}

int CSocketItemMgr::AttachToolTipForSeedSphereItem(const ITEM* pItem, int iTextNum)
{
	SOCKET_OPTION_INFO * pInfo = NULL;

	if(pItem->Type >= ITEM_WING+60 && pItem->Type <= ITEM_WING+65)
	{
		int iCategoryIndex = pItem->Type - (ITEM_WING+60) + 1;
		sprintf(TextList[iTextNum], GlobalText[2653], GlobalText[2640 + iCategoryIndex - 1]);
		TextListColor[iTextNum] = TEXT_COLOR_WHITE;
		TextBold[iTextNum] = false;
		++iTextNum;

		sprintf ( TextList[iTextNum], "\n" ); ++iTextNum; ++SkipNum;

		int iSocketSeedID = 0;
		int iLevel = (pItem->Level>>3)&15;
		switch(iCategoryIndex)
		{
		case 1:	// 0~9
			iSocketSeedID = 0 + iLevel;
			break;
		case 2:	// 10~15
			iSocketSeedID = 10 + iLevel;
			break;
		case 3:	// 16~20
			iSocketSeedID = 16 + iLevel;
			break;
		case 4:	// 21~28
			iSocketSeedID = 21 + iLevel;
			break;
		case 5:	// 29~33
			iSocketSeedID = 29 + iLevel;
			break;
		case 6:	// 34~40
			iSocketSeedID = 34 + iLevel;
			break;
		}
		pInfo = &m_SocketOptionInfo[SOT_SOCKET_ITEM_OPTIONS][iSocketSeedID];
		sprintf(TextList[iTextNum], pInfo->m_szOptionName);
		TextListColor[iTextNum] = TEXT_COLOR_BLUE;
		TextBold[iTextNum] = false;
		++iTextNum;
	}
	else if(pItem->Type >= ITEM_WING+70 && pItem->Type <= ITEM_WING+74)
	{
		int iSphereLevel = pItem->Type - (ITEM_WING+70) + 1;
		sprintf(TextList[iTextNum], GlobalText[2654], iSphereLevel);
		TextListColor[iTextNum] = TEXT_COLOR_WHITE;
		TextBold[iTextNum] = false;
		++iTextNum;
	}
	else if(pItem->Type >= ITEM_WING+100 && pItem->Type <= ITEM_WING+129)
	{
		int iCategoryIndex = (pItem->Type - (ITEM_WING+100)) % 6 + 1;
		sprintf(TextList[iTextNum], GlobalText[2653], GlobalText[2640 + iCategoryIndex - 1]);
		TextListColor[iTextNum] = TEXT_COLOR_WHITE;
		TextBold[iTextNum] = false;
		++iTextNum;

		sprintf ( TextList[iTextNum], "\n" ); ++iTextNum; ++SkipNum;

		int iSocketSeedID = 0;
		int iLevel = (pItem->Level>>3)&15;
		switch(iCategoryIndex)
		{
		case 1:	// 0~9
			iSocketSeedID = 0 + iLevel;
			break;
		case 2:	// 10~15
			iSocketSeedID = 10 + iLevel;
			break;
		case 3:	// 16~20
			iSocketSeedID = 16 + iLevel;
			break;
		case 4:	// 21~28
			iSocketSeedID = 21 + iLevel;
			break;
		case 5:	// 29~33
			iSocketSeedID = 29 + iLevel;
			break;
		case 6:	// 34~40
			iSocketSeedID = 34 + iLevel;
			break;
		}

		pInfo = &m_SocketOptionInfo[SOT_SOCKET_ITEM_OPTIONS][iSocketSeedID];


		char szOptionValueText[16] = { 0, };

		float fOptionValue = (float)pInfo->m_iOptionValue[(pItem->Type - (ITEM_WING+100)) / 6];
		CalcSocketOptionValueText(szOptionValueText, pInfo->m_bOptionType, fOptionValue);

		sprintf(TextList[iTextNum], "%s %s", pInfo->m_szOptionName, szOptionValueText);
		TextListColor[iTextNum] = TEXT_COLOR_BLUE;
		TextBold[iTextNum] = false;
		++iTextNum;
	}

	return iTextNum;
}

BOOL CSocketItemMgr::IsSocketSetOptionEnabled()
{
	return (!m_EquipSetBonusList.empty());
}

void CSocketItemMgr::RenderToolTipForSocketSetOption(int iPos_x, int iPos_y)
{
	if (IsSocketSetOptionEnabled())
	{
		g_pRenderText->SetTextColor(255, 255, 255, 255);
		g_pRenderText->SetBgColor(100, 0, 0, 0);

        int PosX, PosY;

        PosX = iPos_x+95;//+60;
        PosY = iPos_y+40;

        BYTE TextNum = 0;
        BYTE SkipNum = 0;
        BYTE setIndex = 0;

        sprintf(TextList[TextNum], "\n"); TextListColor[TextNum] = 0; TextBold[TextNum] = false; TextNum++; SkipNum++;
        sprintf(TextList[TextNum], "\n"); TextListColor[TextNum] = 0; TextBold[TextNum] = false; TextNum++; SkipNum++;
        sprintf(TextList[TextNum], "\n"); TextListColor[TextNum] = 0; TextBold[TextNum] = false; TextNum++; SkipNum++;

        sprintf (TextList[TextNum], GlobalText[2657]);
		TextListColor[TextNum] = TEXT_COLOR_PURPLE;
		TextBold[TextNum] = true;
		TextNum++;

		char szOptionText[64] = { 0, };
		char szOptionValueText[16] = { 0, };
		SOCKET_OPTION_INFO * pInfo = NULL;
		for (std::deque<DWORD>::iterator iter = m_EquipSetBonusList.begin(); iter != m_EquipSetBonusList.end(); ++iter)
		{
			pInfo = &m_SocketOptionInfo[SOT_EQUIP_SET_BONUS_OPTIONS][*iter];
			CalcSocketOptionValueText(szOptionValueText, pInfo->m_bOptionType, (float)pInfo->m_iOptionValue[0]);
			sprintf(TextList[TextNum], "%s %s", pInfo->m_szOptionName, szOptionValueText);
			TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = false;
			TextNum++;
		}

		RenderTipTextList( PosX, PosY, TextNum, 140, RT3_SORT_CENTER );
	}
}

void CSocketItemMgr::CheckSocketSetOption()
{
	m_EquipSetBonusList.clear();

	int iSeedSum[6] = { 0, 0, 0, 0, 0, 0 };
	ITEM * pItem = NULL;
	SOCKET_OPTION_INFO * pInfo = NULL;

	for (int i = 0; i < MAX_EQUIPMENT; ++i)
	{
		pItem = &CharacterMachine->Equipment[i];
		for (int j = 0; j < pItem->SocketCount; ++j)
		{
			if (pItem->SocketSeedID[j] != SOCKET_EMPTY)
			{
				pInfo = &m_SocketOptionInfo[SOT_SOCKET_ITEM_OPTIONS][pItem->SocketSeedID[j]];
				++iSeedSum[pInfo->m_iOptionCategory - 1];
			}
		}
	}

	for (int i = 0; i < m_iNumEquitSetBonusOptions; ++i)
	{
		int icnt = 0;
		BYTE * pbySetTest = m_SocketOptionInfo[SOT_EQUIP_SET_BONUS_OPTIONS][i].m_bySocketCheckInfo;
		for (int j = 0; j < 6; ++j)
		{
			icnt = j;
			if (iSeedSum[j] < pbySetTest[j]) break;
		}
		if (icnt < 6) continue;

		m_EquipSetBonusList.push_back(m_SocketOptionInfo[SOT_EQUIP_SET_BONUS_OPTIONS][i].m_iOptionID);
	}
}

int CSocketItemMgr::GetSocketOptionValue(const ITEM * pItem, int iSocketIndex)
{
	if (pItem->SocketCount > 0 && pItem->SocketSeedID[iSocketIndex] != SOCKET_EMPTY)
	{
		SOCKET_OPTION_INFO * pInfo = NULL;
		pInfo = &m_SocketOptionInfo[SOT_SOCKET_ITEM_OPTIONS][pItem->SocketSeedID[iSocketIndex]];
		float fOptionValue = (float)pInfo->m_iOptionValue[pItem->SocketSphereLv[iSocketIndex] - 1];
		return CalcSocketOptionValue(pInfo->m_bOptionType, fOptionValue);
	}
	else
	{
		return 0;
	}
}

void CSocketItemMgr::CalcSocketStatusBonus()
{
	memset(&m_StatusBonus, 0, sizeof(SOCKET_OPTION_STATUS_BONUS));
	m_StatusBonus.m_fDefenceRateBonus = 1.0f;

	ITEM * pItem = NULL;
	SOCKET_OPTION_INFO * pInfo = NULL;

	for (int i = 0; i < MAX_EQUIPMENT; ++i)
	{
		pItem = &CharacterMachine->Equipment[i];

		if (!IsSocketItem(pItem)) continue;
			
		for (int j = 0; j < pItem->SocketCount; ++j)
		{
			if (pItem->SocketSeedID[j] != SOCKET_EMPTY)
			{
				pInfo = &m_SocketOptionInfo[SOT_SOCKET_ITEM_OPTIONS][pItem->SocketSeedID[j]];
				float fOptionValue = (float)pInfo->m_iOptionValue[pItem->SocketSphereLv[j] - 1];
				int iBonus = CalcSocketOptionValue(pInfo->m_bOptionType, fOptionValue);

				switch(pInfo->m_iOptionID)
				{
				case SOPT_ATTACK_N_MAGIC_DAMAGE_BONUS_BY_LEVEL:
				case SOPT_ATTACK_N_MAGIC_DAMAGE_BONUS:
					m_StatusBonus.m_iAttackDamageMinBonus += iBonus;
					m_StatusBonus.m_iAttackDamageMaxBonus += iBonus;
					break;
				case SOPT_ATTACK_SPEED_BONUS:
					m_StatusBonus.m_iAttackSpeedBonus += iBonus;
					break;
				case SOPT_ATTACT_N_MAGIC_DAMAGE_MAX_BONUS:
					m_StatusBonus.m_iAttackDamageMaxBonus += iBonus;
					break;
				case SOPT_ATTACK_N_MAGIC_DAMAGE_MIN_BONUS:
					m_StatusBonus.m_iAttackDamageMinBonus += iBonus;
					break;
				case SOPT_DEFENCE_RATE_BONUS:
					m_StatusBonus.m_fDefenceRateBonus *= 1.0f + iBonus * 0.01f;
					break;
				case SOPT_DEFENCE_BONUS:
					m_StatusBonus.m_iDefenceBonus += iBonus;
					break;
				case SOPT_SHIELD_DEFENCE_BONUS:
					m_StatusBonus.m_iShieldDefenceBonus += iBonus;
					break;
				case SOPT_SKILL_DAMAGE_BONUS:
					m_StatusBonus.m_iSkillAttackDamageBonus += iBonus;
					break;
				case SOPT_ATTACK_RATE_BONUS:
					m_StatusBonus.m_iAttackRateBonus += iBonus;
					break;
				case SOPT_STRENGTH_BONUS:
					m_StatusBonus.m_iStrengthBonus += iBonus;
					break;
				case SOPT_DEXTERITY_BONUS:
					m_StatusBonus.m_iDexterityBonus += iBonus;
					break;
				case SOPT_VITALITY_BONUS:
					m_StatusBonus.m_iVitalityBonus += iBonus;
					break;
				case SOPT_ENERGY_BONUS:	
					m_StatusBonus.m_iEnergyBonus += iBonus;
					break;
				}
			}
		}
		
		if (pItem->SocketSeedSetOption != SOCKET_EMPTY)
		{
			pInfo = &m_SocketOptionInfo[SOT_MIX_SET_BONUS_OPTIONS][pItem->SocketSeedSetOption];
			int iBonus = CalcSocketOptionValue(pInfo->m_bOptionType, (float)pInfo->m_iOptionValue[0]);

			switch(pInfo->m_iOptionID)
			{
			case SBOPT_ATTACK_DAMAGE_BONUS:
				m_StatusBonus.m_iAttackDamageMinBonus += iBonus;
				m_StatusBonus.m_iAttackDamageMaxBonus += iBonus;
				break;
			case SBOPT_SKILL_DAMAGE_BONUS:	
			case SBOPT_SKILL_DAMAGE_BONUS_2:
				m_StatusBonus.m_iSkillAttackDamageBonus += iBonus;
				break;
			case SBOPT_MAGIC_POWER_BONUS:
				m_StatusBonus.m_iAttackDamageMinBonus += iBonus;
				m_StatusBonus.m_iMagicPowerBonus += iBonus;
				break;
			case SBOPT_DEFENCE_BONUS:
				m_StatusBonus.m_iDefenceBonus += iBonus;
				break;
			}
		}
	}
}

void CSocketItemMgr::OpenSocketItemScript(const unicode::t_char * szFileName)
{
	FILE *fp = fopen(szFileName,"rb");
	if(fp == NULL)
	{
		unicode::t_char Text[256];
    	unicode::_sprintf(Text,"%s - File not exist.",szFileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		return;
	}

	int iSize = sizeof(SOCKET_OPTION_INFO);
	for (int j = 0; j < MAX_SOCKET_OPTION_TYPES; ++j)
	{
		for (int i = 0; i < MAX_SOCKET_OPTION; ++i)
		{
			fread(&m_SocketOptionInfo[j][i], iSize, 1, fp);
			BuxConvert((BYTE*)&m_SocketOptionInfo[j][i], iSize);
		}
	}

	fclose(fp); 

	for (int i = 0; i < MAX_SOCKET_OPTION; ++i)
	{
		m_iNumEquitSetBonusOptions = i;
		BYTE * pbySetTest = m_SocketOptionInfo[SOT_EQUIP_SET_BONUS_OPTIONS][i].m_bySocketCheckInfo;
		if (pbySetTest[0] + pbySetTest[1] + pbySetTest[2] + pbySetTest[3] + pbySetTest[4] + pbySetTest[5] == 0) break;
	}
}