#include "stdafx.h"
#include "MixMgr.h"
#include "./Utilities/Log/ErrorReport.h"
#include "UIManager.h"
#include "ZzzInventory.h"
#include "CSItemOption.h"
#include "UIJewelHarmony.h"
#include "SocketSystem.h"
#include "SkillManager.h"

using namespace SEASON3A;

static BYTE bBuxCode[3] = {0xfc,0xcf,0xab};

static void BuxConvert(BYTE *Buffer,int Size)
{
	for(int i=0;i<Size;i++)
		Buffer[i] ^= bBuxCode[i%3];
}
CMixRecipeMgr g_MixRecipeMgr;

void CMixItem::Reset()
{
	m_sType = 0;
	m_iLevel = 0;
	m_iOption = 0;
	m_iDurability = 0;
	m_dwSpecialItem = 0;
	m_b380AddedItem = FALSE;
	m_bFenrirAddedItem = FALSE;
	m_bIsCharmItem = FALSE;
	m_bIsChaosCharmItem = FALSE;
	m_bIsJewelItem = FALSE;
	m_wHarmonyOption = 0;
	m_wHarmonyOptionLevel = 0;
	m_bMixLuck = FALSE;
	m_bIsEquipment = FALSE;
	m_bIsWing = FALSE;
	m_bIsUpgradedWing = FALSE;
	m_bIs3rdUpgradedWing = FALSE;
	m_bySocketCount = 0;
	for (int i = 0; i < MAX_SOCKETS; ++i)
	{
		m_bySocketSeedID[i] = SOCKET_EMPTY;
		m_bySocketSphereLv[i] = 0;
	}
	m_bCanStack = FALSE;
	m_dwMixValue = 0;
	m_iCount = 0;
	m_iTestCount = 0;
}

void CMixItem::SetItem(ITEM * pItem, DWORD dwMixValue)
{
	Reset();
	
	m_sType = pItem->Type;
	m_iLevel = ((pItem->Level>>3) & 15);
	m_iDurability = pItem->Durability;
	for (int i = 0; i < pItem->SpecialNum; i++)
	{
		switch ( pItem->Special[i])
		{
		case AT_IMPROVE_MAGIC:
		case AT_IMPROVE_CURSE:
		case AT_IMPROVE_DAMAGE:
		case AT_IMPROVE_DEFENSE:
		case AT_IMPROVE_BLOCKING:
			m_iOption = pItem->SpecialValue[i];
			break;
		case AT_LIFE_REGENERATION:
			m_iOption = pItem->SpecialValue[i] * 4;
			break;
		case AT_LUCK:
			m_bMixLuck = TRUE;
			break;
		}
	}
	if ((pItem->Option1 & 63) > 0) m_dwSpecialItem |= RCP_SP_EXCELLENT;
	if (pItem->RequireLevel >= 380) m_dwSpecialItem |= RCP_SP_ADD380ITEM;
	if ((pItem->ExtOption % 0x04) == EXT_A_SET_OPTION || (pItem->ExtOption % 0x04) == EXT_B_SET_OPTION) m_dwSpecialItem |= RCP_SP_SETITEM;
	m_b380AddedItem = pItem->option_380;

	if (pItem->Type >= ITEM_SWORD && pItem->Type <= ITEM_BOOTS+MAX_ITEM_INDEX-1)
		m_bIsEquipment = TRUE;

	if (pItem->Type == ITEM_HELPER+37 && pItem->Option1 != 0)
		m_bFenrirAddedItem = TRUE;

	if (pItem->Type == ITEM_POTION+53)
		m_bIsCharmItem = TRUE;

	if( pItem->Type == ITEM_POTION+96 )
		m_bIsChaosCharmItem = TRUE;

	if(pItem->Type == ITEM_WING+15
		|| pItem->Type == ITEM_WING+30
		|| pItem->Type == ITEM_WING+31
		|| pItem->Type == ITEM_POTION+13
		|| pItem->Type == ITEM_POTION+14
		|| pItem->Type == ITEM_POTION+16
		|| pItem->Type == ITEM_POTION+22
		|| pItem->Type == ITEM_POTION+31
		|| pItem->Type == ITEM_POTION+42
		)	
		m_bIsJewelItem = TRUE;

	m_bySocketCount = pItem->SocketCount;
	if (m_bySocketCount > 0)
	{
		m_dwSpecialItem |= RCP_SP_SOCKETITEM;
		for (int i = 0; i < MAX_SOCKETS; ++i)
		{
			m_bySocketSeedID[i] = pItem->SocketSeedID[i];
			m_bySocketSphereLv[i] = pItem->SocketSphereLv[i];
		}
		m_dwSpecialItem ^= RCP_SP_ADD380ITEM;
	}
	m_bySeedSphereID = g_SocketItemMgr.GetSeedShpereSeedID(pItem);

	if (pItem->Jewel_Of_Harmony_Option > 0)
	{
		m_dwSpecialItem |= RCP_SP_HARMONY;
		m_wHarmonyOption = pItem->Jewel_Of_Harmony_Option;
		m_wHarmonyOptionLevel = pItem->Jewel_Of_Harmony_OptionLevel;
	}

	switch(pItem->Type)
	{
	case ITEM_WING:
	case ITEM_WING+1:
	case ITEM_WING+2:
	case ITEM_WING+41:
		m_bIsWing = TRUE;
		break;
	case ITEM_WING+3:
	case ITEM_WING+4:
	case ITEM_WING+5:
	case ITEM_WING+6:
	case ITEM_HELPER+30:
	case ITEM_WING+42:
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	case ITEM_WING+49:
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		m_bIsUpgradedWing = TRUE;
		break;
	case ITEM_WING+36:
	case ITEM_WING+37:
	case ITEM_WING+38:
	case ITEM_WING+39:
	case ITEM_WING+40:
	case ITEM_WING+43:
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	case ITEM_WING+50:
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		m_bIs3rdUpgradedWing = TRUE;
	}

	if (m_bIsWing || m_bIsUpgradedWing || m_bIs3rdUpgradedWing)
	{
		if (m_dwSpecialItem & RCP_SP_EXCELLENT)
			m_dwSpecialItem ^= RCP_SP_EXCELLENT;
	}
	switch(pItem->Type)
	{
	case ITEM_POTION+3:
	case ITEM_POTION+38:
	case ITEM_POTION+39:
	case ITEM_POTION+53:
	case ITEM_POTION+88:
	case ITEM_POTION+89:
	case ITEM_POTION+90:
	case ITEM_POTION+100:
		m_bCanStack = TRUE;
		break;
	}
	m_dwMixValue = dwMixValue;

	if (m_bCanStack == TRUE) m_iCount = m_iDurability;
	else m_iCount = 1;
}

int CMixItemInventory::AddItem(ITEM * pItem)
{
	BOOL bFind = FALSE;

	for (int i = 0; i < m_iNumMixItems; ++i)
	{
		if (m_MixItems[i] == pItem)
		{
			bFind = TRUE;
			m_MixItems[i].m_dwMixValue += EvaluateMixItemValue(pItem);
			if (m_MixItems[i].m_bCanStack == TRUE)
			{
				m_MixItems[i].m_iCount += pItem->Durability;
			}
			else
			{
				++m_MixItems[i].m_iCount;
			}
			break;
		}
	}
	if (bFind == FALSE)
	{
		m_MixItems[m_iNumMixItems++].SetItem(pItem, EvaluateMixItemValue(pItem));
	}

	return 0;
}

DWORD CMixItemInventory::EvaluateMixItemValue(ITEM * pItem)
{
	DWORD dwMixValue = 0;
	switch (pItem->Type)
	{
	case ITEM_WING+15:
		dwMixValue = 40000;
		break;
	case ITEM_POTION+13:
		dwMixValue = 100000;
		break;
	case ITEM_POTION+14:
		dwMixValue = 70000;
		break;
	case ITEM_POTION+22:
		dwMixValue = 450000;
		break;
	case ITEM_POTION+16:
		dwMixValue = 0;
		break;
	case ITEM_POTION+31:
	default:
		dwMixValue = ItemValue(pItem, 0);
		break;
	}
	return dwMixValue;
}

void CMixRecipes::Reset()
{
	ClearCheckRecipeResult();
	std::vector<MIX_RECIPE *>::iterator iter;
	for (iter = m_Recipes.begin(); iter != m_Recipes.end(); ++iter)
	{
		if (*iter != NULL)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	m_Recipes.clear();
}

void CMixRecipes::AddRecipe(MIX_RECIPE * pMixRecipe)
{
	if (pMixRecipe != NULL)
		m_Recipes.push_back(pMixRecipe);
}

BOOL CMixRecipes::IsMixSource(ITEM * pItem)
{
	CMixItem mixitem;
	mixitem.SetItem(pItem, 0);

#ifdef LEM_ADD_LUCKYITEM
	if( Check_LuckyItem( pItem->Type ) && g_MixRecipeMgr.GetMixInventoryType() != MIXTYPE_JERRIDON )	return FALSE;
#endif // LEM_ADD_LUCKYITEM

	if (IsCharmItem(mixitem))
	{
		if ((GetCurRecipe() == NULL || GetCurRecipe()->m_bCharmOption == 'A')
			&& m_wTotalCharmBonus + mixitem.m_iCount <= 10)
		{
			return TRUE;
		}
	}

	if( IsChaosCharmItem(mixitem)) 
	{
		if ((GetCurRecipe() == NULL || GetCurRecipe()->m_bCharmOption == 'A') && m_wTotalChaosCharmCount < 1) 
		{
			return TRUE;
		}
	}

	for (std::vector<MIX_RECIPE *>::iterator iter = m_Recipes.begin(); iter != m_Recipes.end(); ++iter)
	{
		for (int j = 0; j < (*iter)->m_iNumMixSoruces; ++j)
		{
			if (m_wTotalCharmBonus > 0 && (*iter)->m_bCharmOption != 'A')
			{
				continue;
			}
			if (CheckItem((*iter)->m_MixSources[j], mixitem)
				&& !((*iter)->m_bMixOption == 'B' && IsChaosItem(mixitem))
				&& !((*iter)->m_bMixOption == 'C' && Is380AddedItem(mixitem))
				&& !((*iter)->m_bMixOption == 'D' && IsFenrirAddedItem(mixitem))
				&& !((*iter)->m_bMixOption == 'E' && !IsUpgradableItem(mixitem))
				&& !((*iter)->m_bMixOption == 'G' && !IsSourceOfRefiningStone(mixitem))
				)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

void CMixRecipes::ClearCheckRecipeResult()
{
	m_iCurMixIndex = 0;
	m_iMostSimilarMixIndex = 0;
	m_iSuccessRate = 0;
	m_dwRequiredZen = 0;
	m_bFindMixLuckItem = FALSE;
	m_dwTotalItemValue = 0;
	m_dwExcellentItemValue = 0;
	m_dwEquipmentItemValue = 0;
	m_dwWingItemValue = 0;
	m_dwSetItemValue = 0;
	m_iFirstItemLevel = 0;
	m_iFirstItemType = 0;
	m_dwTotalNonJewelItemValue = 0;
	m_byFirstItemSocketCount = 0;
	for (int i = 0; i < MAX_SOCKETS; ++i)
	{
		m_byFirstItemSocketSeedID[i] = SOCKET_EMPTY;
		m_byFirstItemSocketSphereLv[i] = 0;
	}
	m_wTotalCharmBonus = 0;
}

int CMixRecipes::CheckRecipe(int iNumMixItems, CMixItem * pMixItems)
{
	m_iCurMixIndex = 0;

	std::vector<MIX_RECIPE *>::iterator iter;
	for (iter = m_Recipes.begin(); iter != m_Recipes.end(); ++iter)
	{
		for (int i = 0; i < iNumMixItems; ++i)
		{
			pMixItems[i].m_iTestCount = pMixItems[i].m_iCount;
		}
		if (CheckRecipeSub(iter, iNumMixItems, pMixItems) == TRUE)
		{
			m_iCurMixIndex = (*iter)->m_iMixIndex + 1;
			EvaluateMixItems(iNumMixItems, pMixItems);
			CalcCharmBonusRate(iNumMixItems, pMixItems);
			CalcMixRate(iNumMixItems, pMixItems);
			CalcMixReqZen(iNumMixItems, pMixItems);
			return GetCurRecipe()->m_iMixID;
		}
		else
		{
			m_iSuccessRate = 0;
			m_dwRequiredZen = 0;
		}
	}
	return (-1);
}

BOOL CMixRecipes::CheckRecipeSub(std::vector<MIX_RECIPE *>::iterator iter, int iNumMixItems, CMixItem * pMixItems)
{
	BOOL bFind = FALSE;
	int iMixRecipeTest[MAX_MIX_SOURCES];
	memset(iMixRecipeTest, 0, sizeof(int) * MAX_MIX_SOURCES);

	for (int j = 0; j < (*iter)->m_iNumMixSoruces; ++j)
	{
		if (!IsOptionItem((*iter)->m_MixSources[j])) bFind = FALSE;
		for (int i = 0; i < iNumMixItems; ++i)
		{
			if (CheckItem((*iter)->m_MixSources[j], pMixItems[i]) && pMixItems[i].m_iTestCount > 0 &&
				(*iter)->m_MixSources[j].m_iCountMax >= iMixRecipeTest[j] + pMixItems[i].m_iTestCount
				&& !((*iter)->m_bMixOption == 'H' && IsSourceOfAttachSeedSphereToArmor(pMixItems[i]))
				&& !((*iter)->m_bMixOption == 'I' && IsSourceOfAttachSeedSphereToWeapon(pMixItems[i]))
				)
			{
				if (pMixItems[i].m_iTestCount >= (*iter)->m_MixSources[j].m_iCountMax)
				{
					iMixRecipeTest[j] += (*iter)->m_MixSources[j].m_iCountMax;
					pMixItems[i].m_iTestCount -= (*iter)->m_MixSources[j].m_iCountMax;
				}
				else
				{
					iMixRecipeTest[j] += pMixItems[i].m_iTestCount;
					pMixItems[i].m_iTestCount = 0;
				}
				bFind = TRUE;
				if (j == 0)
				{
					m_iFirstItemLevel = pMixItems[i].m_iLevel;
					m_iFirstItemType = pMixItems[i].m_sType;
					m_byFirstItemSocketCount = pMixItems[i].m_bySocketCount;
					if (m_byFirstItemSocketCount > 0)
					{
						for (int k = 0; k < MAX_SOCKETS; ++k)
						{
							m_byFirstItemSocketSeedID[k] = pMixItems[i].m_bySocketSeedID[k];
							m_byFirstItemSocketSphereLv[k] = pMixItems[i].m_bySocketSphereLv[k];
						}
					}
				}
			}
		}
		if (bFind == FALSE || (*iter)->m_MixSources[j].m_iCountMin > iMixRecipeTest[j])	
		{
			return FALSE;
		}
	}

	for (int i = 0; i < iNumMixItems; ++i)
	{
		if (pMixItems[i].m_iTestCount > 0)
		{
			if (pMixItems[i].m_bIsCharmItem && (*iter)->m_bCharmOption == 'A');
			else
			if(pMixItems[i].m_bIsChaosCharmItem && (*iter)->m_bChaosCharmOption == 'A');
			else
				return FALSE;
		}
	}
	return TRUE;
}

int CMixRecipes::CheckRecipeSimilarity(int iNumMixItems, CMixItem * pMixItems)
{
	if (iNumMixItems == 0 && m_Recipes.size() == 1)
	{
		m_iMostSimilarMixIndex = 1;
		for (int i = 0; i < (*m_Recipes.begin())->m_iNumMixSoruces; ++i)
		{
			m_iMostSimilarMixSourceTest[i] = (*m_Recipes.begin())->m_MixSources[0].m_iCountMax;
		}
		return m_iMostSimilarMixIndex;
	}

	int iMostSimiliarRecipe = 0;
	int iMostSimiliarityPoint = 0;
	memset(m_iMostSimilarMixSourceTest, 0, sizeof(int) * MAX_MIX_SOURCES);
	memset(m_iMixSourceTest, 0, sizeof(int) * MAX_MIX_SOURCES);

	int iSimilarityPoint;
	for (std::vector<MIX_RECIPE *>::iterator iter = m_Recipes.begin(); iter != m_Recipes.end(); ++iter)
	{
		memset(m_iMixSourceTest, 0, sizeof(int) * MAX_MIX_SOURCES);
		for (int i = 0; i < (*iter)->m_iNumMixSoruces; ++i)
			m_iMixSourceTest[i] = (*iter)->m_MixSources[i].m_iCountMax;

		for (int i = 0; i < iNumMixItems; ++i)
			pMixItems[i].m_iTestCount = pMixItems[i].m_iCount;

		iSimilarityPoint = CheckRecipeSimilaritySub(iter, iNumMixItems, pMixItems);
		if (iSimilarityPoint == 1 && m_Recipes.size() > 1) iSimilarityPoint = 0;
		if (iSimilarityPoint > iMostSimiliarityPoint ||
			(iSimilarityPoint == iMostSimiliarityPoint && iSimilarityPoint > 0 && m_iCurMixIndex == (*iter)->m_iMixIndex + 1))
		{
			iMostSimiliarityPoint = iSimilarityPoint;
			iMostSimiliarRecipe = (*iter)->m_iMixIndex + 1;
			memset(m_iMostSimilarMixSourceTest, 0, sizeof(int) * MAX_MIX_SOURCES);
			for (int i = 0; i < (*iter)->m_iNumMixSoruces; ++i)
			{
				m_iMostSimilarMixSourceTest[i] = m_iMixSourceTest[i];
			}
		}
	}
	m_iMostSimilarMixIndex = iMostSimiliarRecipe;
	return iMostSimiliarRecipe;
}

int CMixRecipes::CheckRecipeSimilaritySub(std::vector<MIX_RECIPE *>::iterator iter, int iNumMixItems, CMixItem * pMixItems)
{
	int iFindTotalPoint = 0;
	int iFindPoint = 0;

	for (int j = 0; j < (*iter)->m_iNumMixSoruces; ++j)
	{
		for (int i = 0; i < iNumMixItems; ++i)
		{
			if (CheckItem((*iter)->m_MixSources[j], pMixItems[i]) && pMixItems[i].m_iTestCount > 0
				&& !((*iter)->m_bMixOption == 'H' && IsSourceOfAttachSeedSphereToArmor(pMixItems[i]))
				&& !((*iter)->m_bMixOption == 'I' && IsSourceOfAttachSeedSphereToWeapon(pMixItems[i]))
				)
			{
				if (IsChaosJewel(pMixItems[i])) iFindPoint = 1;
 				else if ((*iter)->m_MixSources[j].m_iCountMax < pMixItems[i].m_iTestCount) iFindPoint = 1;
				else if (j == 0) iFindPoint = 10;
				else if (j == 1) iFindPoint = 5;
				else iFindPoint = 3;

				iFindTotalPoint += iFindPoint;

 				if (pMixItems[i].m_iTestCount > 0 && m_iMixSourceTest[j] > 0)
				{
					m_iMixSourceTest[j] -= pMixItems[i].m_iTestCount;
					pMixItems[i].m_iTestCount -= (*iter)->m_MixSources[j].m_iCountMax;
				}
			}
		}
	}
	for (int i = 0; i < iNumMixItems; ++i)
	{
		if (pMixItems[i].m_iTestCount > 0)
		{
			if (pMixItems[i].m_bIsCharmItem && (*iter)->m_bCharmOption == 'A');
			else
			if (pMixItems[i].m_bIsChaosCharmItem && (*iter)->m_bChaosCharmOption == 'A');
			else
				return 0;
		}
	}
	return iFindTotalPoint;
}

bool CMixRecipes::CheckItem(MIX_RECIPE_ITEM & rItem, CMixItem & rSource)
{
	if (rItem.m_sTypeMin <= rSource.m_sType && rItem.m_sTypeMax >= rSource.m_sType &&
		rItem.m_iLevelMin <= rSource.m_iLevel && rItem.m_iLevelMax >= rSource.m_iLevel &&
		rItem.m_iDurabilityMin <= rSource.m_iDurability && rItem.m_iDurabilityMax >= rSource.m_iDurability &&
		rItem.m_iOptionMin <= rSource.m_iOption && rItem.m_iOptionMax >= rSource.m_iOption &&
		(rItem.m_dwSpecialItem & RCP_SP_EXCELLENT) <= (rSource.m_dwSpecialItem & RCP_SP_EXCELLENT) &&
		(rItem.m_dwSpecialItem & RCP_SP_ADD380ITEM) <= (rSource.m_dwSpecialItem & RCP_SP_ADD380ITEM) &&
		(rItem.m_dwSpecialItem & RCP_SP_SETITEM) <= (rSource.m_dwSpecialItem & RCP_SP_SETITEM)
		&& (rItem.m_dwSpecialItem & RCP_SP_HARMONY) <= (rSource.m_dwSpecialItem & RCP_SP_HARMONY)
		&& (rItem.m_dwSpecialItem & RCP_SP_SOCKETITEM) <= (rSource.m_dwSpecialItem & RCP_SP_SOCKETITEM)
		)
	{
		return true;
	}
	return false;
}

MIX_RECIPE * CMixRecipes::GetCurRecipe()
{
	if (m_iCurMixIndex == 0) return NULL;
	return m_Recipes[m_iCurMixIndex - 1];
}

MIX_RECIPE * CMixRecipes::GetMostSimilarRecipe()
{
	if (m_iMostSimilarMixIndex == 0) return NULL;
	return m_Recipes[m_iMostSimilarMixIndex - 1];
}

int CMixRecipes::GetCurMixID()
{
	if (m_iCurMixIndex == 0) return 0;
	return m_Recipes[m_iCurMixIndex - 1]->m_iMixID;
}

BOOL CMixRecipes::GetCurRecipeName(unicode::t_char * pszNameOut, int iNameLine)
{
	if (!IsReadyToMix())
	{
		if (iNameLine == 1)
		{
			switch(g_MixRecipeMgr.GetMixInventoryType())
			{
			case MIXTYPE_TRAINER:
				unicode::_sprintf (pszNameOut, GlobalText[1213], GlobalText[1212]);
				break;
			case MIXTYPE_OSBOURNE:
				unicode::_sprintf (pszNameOut, GlobalText[1213], GlobalText[2061]);
				break;
			case MIXTYPE_JERRIDON:
				unicode::_sprintf (pszNameOut, GlobalText[1213], GlobalText[2062]);
				break;
			case MIXTYPE_ELPIS:
				unicode::_sprintf (pszNameOut, GlobalText[1213], GlobalText[2063]);
				break;
			default:
				unicode::_sprintf (pszNameOut, "%s", GlobalText[601]);
				break;
			}
			return TRUE;
		}
		else return FALSE;
	}
	return GetRecipeName(GetCurRecipe(), pszNameOut, iNameLine, FALSE);
}

BOOL CMixRecipes::GetRecipeName(MIX_RECIPE * pRecipe, unicode::t_char * pszNameOut, int iNameLine, BOOL bSimilarRecipe)
{
	if (pRecipe == NULL) return FALSE;
	if (iNameLine > 2 || iNameLine < 1) return FALSE;
	if (pRecipe->m_bMixOption == 'C')
	{
		std::vector<std::string> optionTextlist;

		g_pItemAddOptioninfo->GetItemAddOtioninfoText(optionTextlist, m_iFirstItemType);
		if (optionTextlist.empty() || bSimilarRecipe)
		{
			if (iNameLine == 1)
			{
				unicode::_sprintf (pszNameOut, "%s", GlobalText[2194]);
				return TRUE;
			}
			return FALSE;
		}
		assert(optionTextlist.size() == 2 && "옵션은 2개여야 함");
		if (iNameLine == 1)
		{
			unicode::_strcpy(pszNameOut, optionTextlist[0].c_str());
			return TRUE;
		}
		else if (iNameLine == 2)
		{
			unicode::_strcpy(pszNameOut, optionTextlist[1].c_str());
			return TRUE;
		}
		return FALSE;
	}
	else
	{
		if (iNameLine == 1)
		{
			if (pRecipe->m_iMixName[1] == 0)
				unicode::_sprintf (pszNameOut, "%s", GlobalText[pRecipe->m_iMixName[0]]);
			else if (pRecipe->m_iMixName[2] == 0)
				unicode::_sprintf (pszNameOut, "%s %s", GlobalText[pRecipe->m_iMixName[0]], GlobalText[pRecipe->m_iMixName[1]]);
			else
				unicode::_sprintf (pszNameOut, "%s %s %s", GlobalText[pRecipe->m_iMixName[0]],
					GlobalText[pRecipe->m_iMixName[1]], GlobalText[pRecipe->m_iMixName[2]]);
			return TRUE;
		}
		return FALSE;
	}
}

BOOL CMixRecipes::GetCurRecipeDesc(unicode::t_char * pszDescOut, int iDescLine)
{
	if (iDescLine > 3 || iDescLine < 1) return FALSE;
	if (GetCurRecipe() == NULL) return FALSE;
	if (GetCurRecipe()->m_iMixDesc[iDescLine - 1] > 0)
		unicode::_strcpy(pszDescOut, GlobalText[GetCurRecipe()->m_iMixDesc[iDescLine - 1]]);
	else
		return FALSE;
	return TRUE;
}

BOOL CMixRecipes::GetMostSimilarRecipeName(unicode::t_char * pszNameOut, int iNameLine)
{
	return GetRecipeName(GetMostSimilarRecipe(), pszNameOut, iNameLine, TRUE);
}

BOOL CMixRecipes::GetRecipeAdvice(unicode::t_char * pszAdviceOut, int iAdivceLine)
{
	if (GetMostSimilarRecipe() == NULL) return FALSE;
	if (iAdivceLine > 3 || iAdivceLine < 1) return FALSE;

	if (GetMostSimilarRecipe()->m_iMixAdvice[iAdivceLine - 1] > 0)
		unicode::_strcpy(pszAdviceOut, GlobalText[GetMostSimilarRecipe()->m_iMixAdvice[iAdivceLine - 1]]);
	else
		return FALSE;
	return TRUE;
}

int CMixRecipes::GetSourceName(int iItemNum, unicode::t_char * pszNameOut, int iNumMixItems, CMixItem * pMixItems)
{

	if(iNumMixItems < 0)	return MIX_SOURCE_ERROR;
	if(pMixItems == NULL)	return MIX_SOURCE_ERROR;

	if (GetMostSimilarRecipe() == NULL) return MIX_SOURCE_ERROR;
	if (iItemNum >= GetMostSimilarRecipe()->m_iNumMixSoruces) return MIX_SOURCE_ERROR;

	MIX_RECIPE_ITEM * pMixRecipeItem = &GetMostSimilarRecipe()->m_MixSources[iItemNum];

	unicode::t_char szTempName[100];
	GetItemName(pMixRecipeItem->m_sTypeMin, pMixRecipeItem->m_iLevelMin, szTempName);

 	if (pMixRecipeItem->m_sTypeMin == pMixRecipeItem->m_sTypeMax &&
		(pMixRecipeItem->m_iLevelMin == pMixRecipeItem->m_iLevelMax || (pMixRecipeItem->m_iLevelMin == 0 && pMixRecipeItem->m_iLevelMax == 255)) &&
		(pMixRecipeItem->m_iOptionMin == pMixRecipeItem->m_iOptionMax || (pMixRecipeItem->m_iOptionMin == 0 && pMixRecipeItem->m_iOptionMax == 255)))
	{
		if (pMixRecipeItem->m_iDurabilityMin == pMixRecipeItem->m_iDurabilityMax)
			unicode::_sprintf(szTempName, "%s(%d)", szTempName, pMixRecipeItem->m_iDurabilityMin);
	}
	else
	{
			if (pMixRecipeItem->m_dwSpecialItem & RCP_SP_ADD380ITEM)
			unicode::_sprintf(szTempName, GlobalText[2335]);
		else if (pMixRecipeItem->m_sTypeMin == 0 && pMixRecipeItem->m_sTypeMax == ITEM_BOOTS+MAX_ITEM_INDEX-1)
			unicode::_sprintf(szTempName, GlobalText[2336]);
		else if (pMixRecipeItem->m_sTypeMin == 0 && pMixRecipeItem->m_sTypeMax == ITEM_HELPER+MAX_ITEM_INDEX-1)
			unicode::_sprintf(szTempName, GlobalText[2336]);
		else if (pMixRecipeItem->m_sTypeMin == 0 && pMixRecipeItem->m_sTypeMax == ITEM_STAFF+MAX_ITEM_INDEX-1)
			unicode::_sprintf(szTempName, GlobalText[2337]);
		else if (pMixRecipeItem->m_sTypeMin == ITEM_SHIELD && pMixRecipeItem->m_sTypeMax == ITEM_BOOTS+MAX_ITEM_INDEX-1)
			unicode::_sprintf(szTempName, GlobalText[2338]);
		else if (pMixRecipeItem->m_sTypeMin == ITEM_WING && pMixRecipeItem->m_sTypeMax == ITEM_WING+2)
			unicode::_sprintf(szTempName, GlobalText[2339]);
		else if (pMixRecipeItem->m_sTypeMin == ITEM_WING+3 && pMixRecipeItem->m_sTypeMax == ITEM_WING+6)
			unicode::_sprintf(szTempName, GlobalText[2348]);
		else if (pMixRecipeItem->m_sTypeMin == ITEM_WING+41 && pMixRecipeItem->m_sTypeMax == ITEM_WING+41)
			unicode::_sprintf(szTempName, GlobalText[2339]);
		else if (pMixRecipeItem->m_sTypeMin == ITEM_WING+42 && pMixRecipeItem->m_sTypeMax == ITEM_WING+42)
			unicode::_sprintf(szTempName, GlobalText[2348]);
		else if (pMixRecipeItem->m_sTypeMin == pMixRecipeItem->m_sTypeMax &&
			(pMixRecipeItem->m_sTypeMin == ITEM_MACE+6 || pMixRecipeItem->m_sTypeMin == ITEM_BOW+6 || pMixRecipeItem->m_sTypeMin == ITEM_STAFF+7))
			unicode::_sprintf(szTempName, GlobalText[2340]);
		else if (pMixRecipeItem->m_sTypeMin == ITEM_WING+60 && pMixRecipeItem->m_sTypeMax == ITEM_WING+65)
			unicode::_sprintf(szTempName, GlobalText[2680]);
		else if (pMixRecipeItem->m_sTypeMin == ITEM_WING+70 && pMixRecipeItem->m_sTypeMax == ITEM_WING+74)
			unicode::_sprintf(szTempName, GlobalText[2681]);
		else if (pMixRecipeItem->m_sTypeMin == ITEM_WING+100 && pMixRecipeItem->m_sTypeMax == ITEM_WING+129)
			unicode::_sprintf(szTempName, GlobalText[2682]);
		else if (pMixRecipeItem->m_sTypeMin == ITEM_WING+100 && pMixRecipeItem->m_sTypeMax == ITEM_WING+128)
			unicode::_sprintf(szTempName, "%s (%s)", GlobalText[2682], GlobalText[2684]);
		else if (pMixRecipeItem->m_sTypeMin == ITEM_WING+101 && pMixRecipeItem->m_sTypeMax == ITEM_WING+129)
			unicode::_sprintf(szTempName, "%s (%s)", GlobalText[2682], GlobalText[2685]);
		else
		{
			int iNameLen = unicode::_strlen(szTempName);
			for (int j = 1; j <= 3 && iNameLen - j - 1 >= 0; ++j)
				if (szTempName[iNameLen - j] == '+') szTempName[iNameLen - j - 1] = '\0';
		}
		if (pMixRecipeItem->m_iDurabilityMin == pMixRecipeItem->m_iDurabilityMax)
			unicode::_sprintf(szTempName, "%s(%d)", szTempName, pMixRecipeItem->m_iDurabilityMin);

		if (pMixRecipeItem->m_iLevelMin == 0 && pMixRecipeItem->m_iLevelMax == 255);
		else if (pMixRecipeItem->m_iLevelMin == pMixRecipeItem->m_iLevelMax)
			unicode::_sprintf(szTempName, "%s +%d", szTempName, pMixRecipeItem->m_iLevelMin);
		else if (pMixRecipeItem->m_iLevelMin == 0)
			unicode::_sprintf(szTempName, "%s +%d%s", szTempName, pMixRecipeItem->m_iLevelMax, GlobalText[2342]);
		else if (pMixRecipeItem->m_iLevelMax == 255)
			unicode::_sprintf(szTempName, "%s +%d%s", szTempName, pMixRecipeItem->m_iLevelMin, GlobalText[2341]);
		else
			unicode::_sprintf(szTempName, "%s +%d~%d", szTempName, pMixRecipeItem->m_iLevelMin, pMixRecipeItem->m_iLevelMax);

		if (pMixRecipeItem->m_iOptionMin == 0 && pMixRecipeItem->m_iOptionMax == 255);
		else if (pMixRecipeItem->m_iOptionMin == pMixRecipeItem->m_iOptionMax)
			unicode::_sprintf(szTempName, "%s +%d%s", szTempName, pMixRecipeItem->m_iOptionMin, GlobalText[2343]);
		else if (pMixRecipeItem->m_iOptionMin == 0)
			unicode::_sprintf(szTempName, "%s +%d%s%s", szTempName, pMixRecipeItem->m_iOptionMax, GlobalText[2343], GlobalText[2342]);
		else if (pMixRecipeItem->m_iOptionMax == 255)
			unicode::_sprintf(szTempName, "%s +%d%s%s", szTempName, pMixRecipeItem->m_iOptionMin, GlobalText[2343], GlobalText[2341]);
		else
			unicode::_sprintf(szTempName, "%s +%d~%d%s", szTempName, pMixRecipeItem->m_iOptionMin, pMixRecipeItem->m_iOptionMax, GlobalText[2343]);
	}

	if (pMixRecipeItem->m_iCountMin == 0 && pMixRecipeItem->m_iCountMax == 255)
		unicode::_sprintf(szTempName, "%s (%s)", szTempName, GlobalText[2344]);
	else if (pMixRecipeItem->m_iCountMin == pMixRecipeItem->m_iCountMax)
		unicode::_sprintf(szTempName, "%s %d%s", szTempName, pMixRecipeItem->m_iCountMin, GlobalText[2345]);
	else if (pMixRecipeItem->m_iCountMin == 0)
		unicode::_sprintf(szTempName, "%s %d%s %s", szTempName, pMixRecipeItem->m_iCountMax, GlobalText[2345], GlobalText[2342]);
	else if (pMixRecipeItem->m_iCountMax == 255)
		unicode::_sprintf(szTempName, "%s %d%s %s", szTempName, pMixRecipeItem->m_iCountMin, GlobalText[2345], GlobalText[2341]);
	else
		unicode::_sprintf(szTempName, "%s %d~%d%s", szTempName, pMixRecipeItem->m_iCountMin, pMixRecipeItem->m_iCountMax, GlobalText[2345]);

	BOOL bPreName = FALSE;
	if (pMixRecipeItem->m_dwSpecialItem & RCP_SP_EXCELLENT)
	{
		unicode::_sprintf(pszNameOut, "%s %s", GlobalText[620], szTempName);
		bPreName = TRUE;
	}
	if (pMixRecipeItem->m_dwSpecialItem & RCP_SP_SETITEM)
	{
		unicode::_sprintf(pszNameOut, "%s %s", GlobalText[1089], szTempName);
		bPreName = TRUE;
	}
	if (pMixRecipeItem->m_dwSpecialItem & RCP_SP_HARMONY)
	{
		unicode::_sprintf(pszNameOut, "%s %s", GlobalText[1550], szTempName);
		bPreName = TRUE;
	}
	if (pMixRecipeItem->m_dwSpecialItem & RCP_SP_SOCKETITEM)
	{
		unicode::_sprintf(pszNameOut, "%s %s", GlobalText[2650], szTempName);
		bPreName = TRUE;
	}
	if (bPreName == FALSE)
	{
		unicode::_strcpy(pszNameOut ,szTempName);
	}

	if (g_MixRecipeMgr.IsMixInit())
	{
		if (pMixRecipeItem->m_iCountMin == 0) return MIX_SOURCE_PARTIALLY;
		else return MIX_SOURCE_NO;
	}

	if (m_iMostSimilarMixSourceTest[iItemNum] == 0)
		return MIX_SOURCE_YES;
	else if (m_iMostSimilarMixSourceTest[iItemNum] < pMixRecipeItem->m_iCountMax)
	{
		if (pMixRecipeItem->m_iCountMin <= 1) return MIX_SOURCE_YES;
		else return MIX_SOURCE_PARTIALLY;
	}
	else
	{
		if (pMixRecipeItem->m_iCountMin == 0) return MIX_SOURCE_PARTIALLY;
		else return MIX_SOURCE_NO;
	}
}

void CMixRecipes::EvaluateMixItems(int iNumMixItems, CMixItem * pMixItems)
{
	m_bFindMixLuckItem = FALSE;
	m_dwTotalItemValue = 0;
	m_dwExcellentItemValue = 0;
	m_dwEquipmentItemValue = 0;
	m_dwWingItemValue = 0;
	m_dwSetItemValue = 0;
	m_dwTotalNonJewelItemValue = 0;

	for (int i = 0; i < iNumMixItems; ++i)
	{
		if (pMixItems[i].m_bMixLuck == TRUE) m_bFindMixLuckItem = TRUE;
		if (pMixItems[i].m_dwSpecialItem & RCP_SP_EXCELLENT) m_dwExcellentItemValue += pMixItems[i].m_dwMixValue;
		if (pMixItems[i].m_bIsEquipment == TRUE) m_dwEquipmentItemValue += pMixItems[i].m_dwMixValue;
		if (pMixItems[i].m_bIsWing == TRUE) m_dwWingItemValue += pMixItems[i].m_dwMixValue;
		if (pMixItems[i].m_dwSpecialItem & RCP_SP_SETITEM) m_dwSetItemValue += pMixItems[i].m_dwMixValue;
		if (pMixItems[i].m_bIsJewelItem == FALSE) m_dwTotalNonJewelItemValue += pMixItems[i].m_dwMixValue;
		m_dwTotalItemValue += pMixItems[i].m_dwMixValue;
	}
}

void CMixRecipes::CalcCharmBonusRate(int iNumMixItems, CMixItem * pMixItems)
{
	m_wTotalCharmBonus = 0;
	for (int i = 0; i < iNumMixItems; ++i)
	{
		if (pMixItems[i].m_bIsCharmItem == TRUE) m_wTotalCharmBonus += pMixItems[i].m_iCount;
	}
}

void CMixRecipes::CalcChaosCharmCount(int iNumMixItems, CMixItem * pMixItems)
{
	m_wTotalChaosCharmCount = 0;
	for (int i = 0; i < iNumMixItems; ++i)
	{
		if (pMixItems[i].m_bIsChaosCharmItem == TRUE) m_wTotalChaosCharmCount += 1;
	}
}

void CMixRecipes::CalcMixRate(int iNumMixItems, CMixItem * pMixItems)
{
	if(iNumMixItems < 0)	return;
	if(pMixItems == NULL)	return;

	m_iSuccessRate = 0;
	if (GetCurRecipe() == NULL) return;

	m_pMixRates = GetCurRecipe()->m_RateToken;
	m_iMixRateIter = 0;
	m_iSuccessRate = (int)MixrateAddSub();

	if (m_iSuccessRate > GetCurRecipe()->m_iSuccessRate)
	{
		m_iSuccessRate = GetCurRecipe()->m_iSuccessRate;
	}
	if (GetCurRecipe()->m_bCharmOption == 'A')
	{
		m_iSuccessRate += m_wTotalCharmBonus;
	}
	if (m_iSuccessRate > 100)
	{
		m_iSuccessRate = 100;
	}
}

float CMixRecipes::MixrateAddSub()
{
	float fLvalue = 0;
	while(1)
	{
		if (m_iMixRateIter >= GetCurRecipe()->m_iNumRateData || m_pMixRates[m_iMixRateIter].op == MRCP_RP)
		{
			return fLvalue;
		}
		switch(m_pMixRates[m_iMixRateIter].op)
		{
		case MRCP_ADD:
			++m_iMixRateIter;
			fLvalue += MixrateMulDiv();
			break;
		case MRCP_SUB:
 			++m_iMixRateIter;
			fLvalue -= MixrateMulDiv();
			break;
		default:
			fLvalue = MixrateMulDiv();
			break;
		};
	}
}

float CMixRecipes::MixrateMulDiv()
{
	float fLvalue = 0;
	while(1)
	{
		if (m_iMixRateIter >= GetCurRecipe()->m_iNumRateData || m_pMixRates[m_iMixRateIter].op == MRCP_RP)
		{
			return fLvalue;
		}
		switch(m_pMixRates[m_iMixRateIter].op)
		{
		case MRCP_ADD:
		case MRCP_SUB:
			return fLvalue;
		case MRCP_MUL:
 			++m_iMixRateIter;
			fLvalue *= MixrateFactor();
 			break;
		case MRCP_DIV:
			++m_iMixRateIter;
			fLvalue /= MixrateFactor();
 			break;
		default:
			fLvalue = MixrateFactor();
 			break;
		};
	}
}

float CMixRecipes::MixrateFactor()
{
	float fValue = 0;
	switch(m_pMixRates[m_iMixRateIter].op)
	{
	case MRCP_LP:
		++m_iMixRateIter;
		fValue = MixrateAddSub();
		break;
	case MRCP_INT:
		++m_iMixRateIter;
		if (m_pMixRates[m_iMixRateIter].op != MRCP_LP) assert(!"m_pMixRates error");
		++m_iMixRateIter;
		fValue = int(MixrateAddSub());
		break;
	case MRCP_NUMBER:
		fValue = (float)m_pMixRates[m_iMixRateIter].value;
		break;
	case MRCP_MAXRATE:
		fValue = GetCurRecipe()->m_iSuccessRate;
		break;
	case MRCP_ITEM:
		fValue = m_dwTotalItemValue;
		break;
	case MRCP_WING:
		fValue = m_dwWingItemValue;
		break;
	case MRCP_EXCELLENT:
		fValue = m_dwExcellentItemValue;
		break;
	case MRCP_EQUIP:
		fValue = m_dwEquipmentItemValue;
		break;
	case MRCP_SET:
		fValue = m_dwSetItemValue;
		break;
	case MRCP_LUCKOPT:
		if (m_bFindMixLuckItem) fValue = 25;
		else fValue = 0;
		break;
	case MRCP_LEVEL1:
		fValue = m_iFirstItemLevel;
		break;
	case MRCP_NONJEWELITEM:
		fValue = m_dwTotalNonJewelItemValue;
		break;
	}
	++m_iMixRateIter;
	return fValue;
}

void CMixRecipes::CalcMixReqZen(int iNumMixItems, CMixItem * pMixItems)
{
	if(iNumMixItems < 0)	return;
	if(pMixItems == NULL)	return;

	m_dwRequiredZen = 0;
	if (GetCurRecipe() == NULL) return;
	switch(GetCurRecipe()->m_bRequiredZenType)
	{
	case 'A':
	case 'C':
		m_dwRequiredZen = GetCurRecipe()->m_dwRequiredZen;
		break;
	case 'B':
		m_dwRequiredZen = m_iSuccessRate * GetCurRecipe()->m_dwRequiredZen;
		break;
	case 'D':
		{
			int iItemType = 0;
			if( ITEM_SWORD <= pMixItems[0].m_sType && ITEM_STAFF > pMixItems[0].m_sType )
			{
				iItemType = SI_Weapon;
			}
			else if( ITEM_STAFF <= pMixItems[0].m_sType && ITEM_SHIELD > pMixItems[0].m_sType )
			{
				iItemType = SI_Staff;
			}
			else if( ITEM_SHIELD <= pMixItems[0].m_sType && ITEM_WING > pMixItems[0].m_sType )
			{
				iItemType = SI_Defense;
			}
			m_dwRequiredZen = g_pUIJewelHarmonyinfo->GetHarmonyJewelOptionInfo(iItemType,
				pMixItems[0].m_wHarmonyOption ).Zen[pMixItems[0].m_wHarmonyOptionLevel];
		}
		break;
	default:
		break;
	}
}

BOOL CMixRecipes::IsChaosItem(CMixItem & rSource)
{
	if (rSource.m_sType == ITEM_MACE+6 || rSource.m_sType == ITEM_BOW+6 || rSource.m_sType == ITEM_STAFF+7) return TRUE;
	return FALSE;
}

BOOL CMixRecipes::IsChaosJewel(CMixItem & rSource)
{
	if (rSource.m_sType == ITEM_WING+15) return TRUE;
	return FALSE;
}

BOOL CMixRecipes::Is380AddedItem(CMixItem & rSource)
{
	return rSource.m_b380AddedItem;
}

BOOL CMixRecipes::IsFenrirAddedItem(CMixItem & rSource)
{
	return rSource.m_bFenrirAddedItem;
}

BOOL CMixRecipes::IsUpgradableItem(CMixItem & rSource)
{
	return (rSource.m_bIsEquipment || rSource.m_bIsWing || rSource.m_bIsUpgradedWing || rSource.m_bIs3rdUpgradedWing);
}

BOOL CMixRecipes::IsSourceOfRefiningStone(CMixItem & rSource)
{
	if (rSource.m_iLevel < 4)
	{
		switch(rSource.m_sType)
		{
		case ITEM_SWORD+0: case ITEM_SWORD+1: case ITEM_SWORD+2: case ITEM_SWORD+4:
		case ITEM_AXE+0: case ITEM_AXE+1: case ITEM_AXE+2:
		case ITEM_MACE+0: case ITEM_MACE+1: case ITEM_MACE+2:
		case ITEM_SPEAR+1: case ITEM_SPEAR+2: case ITEM_SPEAR+3: case ITEM_SPEAR+5: case ITEM_SPEAR+6:
		case ITEM_BOW+0: case ITEM_BOW+1: case ITEM_BOW+2: case ITEM_BOW+3:
		case ITEM_BOW+8: case ITEM_BOW+9: case ITEM_BOW+10: case ITEM_BOW+11:
		case ITEM_STAFF+0: case ITEM_STAFF+1: case ITEM_STAFF+2:
		case ITEM_SHIELD+0: case ITEM_SHIELD+1: case ITEM_SHIELD+2: case ITEM_SHIELD+3: case ITEM_SHIELD+4:
		case ITEM_SHIELD+6: case ITEM_SHIELD+7: case ITEM_SHIELD+9: case ITEM_SHIELD+10:
		case ITEM_HELM+0: case ITEM_HELM+2: case ITEM_HELM+4: case ITEM_HELM+5: case ITEM_HELM+6:
		case ITEM_HELM+7: case ITEM_HELM+8: case ITEM_HELM+10: case ITEM_HELM+11: case ITEM_HELM+12:
		case ITEM_ARMOR+0: case ITEM_ARMOR+2: case ITEM_ARMOR+4: case ITEM_ARMOR+5: case ITEM_ARMOR+6:
		case ITEM_ARMOR+7: case ITEM_ARMOR+8: case ITEM_ARMOR+10: case ITEM_ARMOR+11: case ITEM_ARMOR+12:
		case ITEM_PANTS+0: case ITEM_PANTS+2: case ITEM_PANTS+4: case ITEM_PANTS+5: case ITEM_PANTS+6:
		case ITEM_PANTS+7: case ITEM_PANTS+8: case ITEM_PANTS+10: case ITEM_PANTS+11: case ITEM_PANTS+12:
		case ITEM_GLOVES+0: case ITEM_GLOVES+2: case ITEM_GLOVES+4: case ITEM_GLOVES+5: case ITEM_GLOVES+6:
		case ITEM_GLOVES+7: case ITEM_GLOVES+8: case ITEM_GLOVES+10: case ITEM_GLOVES+11: case ITEM_GLOVES+12:
		case ITEM_BOOTS+0: case ITEM_BOOTS+2: case ITEM_BOOTS+4: case ITEM_BOOTS+5: case ITEM_BOOTS+6:
		case ITEM_BOOTS+7: case ITEM_BOOTS+8: case ITEM_BOOTS+10: case ITEM_BOOTS+11: case ITEM_BOOTS+12:
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CMixRecipes::IsSourceOfAttachSeedSphereToWeapon(CMixItem & rSource)
{
	if (rSource.m_sType >= ITEM_WING+100 && rSource.m_sType <= ITEM_WING+129)
	{
		int iSeedSphereType = rSource.m_sType - ITEM_WING;
		if (iSeedSphereType % 2 == 0) return TRUE;
	}
	return FALSE;
}

BOOL CMixRecipes::IsSourceOfAttachSeedSphereToArmor(CMixItem & rSource)
{
	if (rSource.m_sType >= ITEM_WING+100 && rSource.m_sType <= ITEM_WING+129)
	{
		int iSeedSphereType = rSource.m_sType - ITEM_WING;

		if (iSeedSphereType % 2 == 1) return TRUE;
	}
	return FALSE;
}

BOOL CMixRecipes::IsCharmItem(CMixItem & rSource)
{
	return rSource.m_bIsCharmItem;
}

BOOL CMixRecipes::IsChaosCharmItem(CMixItem & rSource)
{
	return rSource.m_bIsChaosCharmItem;
}

BOOL CMixRecipes::IsJewelItem(CMixItem & rSource)
{
	return rSource.m_bIsJewelItem;
}

void CMixRecipeMgr::OpenRecipeFile(const unicode::t_char * szFileName)
{
	int i, j;
	for (j = 0; j < MAX_MIX_TYPES; ++j)
	{
		m_MixRecipe[j].Reset();
	}

	FILE *fp = fopen(szFileName,"rb");
	if(fp == NULL)
	{
		unicode::t_char Text[256];
    	unicode::_sprintf(Text,"%s - File not exist.",szFileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		exit(0);
	}

	int iNumMixRecipes[MAX_MIX_TYPES];
	int iSize = sizeof(int) * MAX_MIX_TYPES;
	fread(iNumMixRecipes, iSize, 1, fp);
	BuxConvert((BYTE*)iNumMixRecipes, iSize);

	iSize = sizeof(MIX_RECIPE);
	for (j = 0; j < MAX_MIX_TYPES; ++j)
	{
		if (feof(fp) || iNumMixRecipes[j] > 1000)
		{
			unicode::t_char Text[256];
    		unicode::_sprintf(Text,"%s - Version not matched.",szFileName);
			g_ErrorReport.Write( Text);
			MessageBox(g_hWnd,Text,NULL,MB_OK);
			SendMessage(g_hWnd,WM_DESTROY,0,0);
			fclose(fp);
			exit(0);
		}
		for (i = 0; i < iNumMixRecipes[j]; ++i)
		{
			MIX_RECIPE * pMixRecipe = new MIX_RECIPE;
			fread(pMixRecipe, iSize, 1, fp);
			BuxConvert((BYTE*)pMixRecipe, iSize);
			m_MixRecipe[j].AddRecipe(pMixRecipe);
		}
	}
	fclose(fp);
}

int CMixRecipeMgr::GetMixInventoryType()
{
	assert(m_iMixType >= MIXTYPE_GOBLIN_NORMAL && m_iMixType < MAX_MIX_TYPES && "정의되지 않은 조합창");
	return m_iMixType;
}

int CMixRecipeMgr::GetMixInventoryEquipmentIndex()
{
	switch(GetMixInventoryType())
	{
	case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
	case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
	case SEASON3A::MIXTYPE_GOBLIN_ADD380:
		return REQUEST_EQUIPMENT_CHAOS_MIX;
	case SEASON3A::MIXTYPE_CASTLE_SENIOR:
		return REQUEST_EQUIPMENT_CHAOS_MIX;
	case SEASON3A::MIXTYPE_TRAINER:
		return REQUEST_EQUIPMENT_TRAINER_MIX;
	case SEASON3A::MIXTYPE_OSBOURNE:
		return REQUEST_EQUIPMENT_OSBOURNE_MIX;
	case SEASON3A::MIXTYPE_JERRIDON:
		return REQUEST_EQUIPMENT_JERRIDON_MIX;
	case SEASON3A::MIXTYPE_ELPIS:
		return REQUEST_EQUIPMENT_ELPIS_MIX;
	case SEASON3A::MIXTYPE_CHAOS_CARD:
		return REQUEST_EQUIPMENT_CHAOS_CARD_MIX;
	case SEASON3A::MIXTYPE_CHERRYBLOSSOM:
		return REQUEST_EQUIPMENT_CHERRYBLOSSOM_MIX;
	case SEASON3A::MIXTYPE_EXTRACT_SEED:
		return REQUEST_EQUIPMENT_EXTRACT_SEED_MIX;
	case SEASON3A::MIXTYPE_SEED_SPHERE:
		return REQUEST_EQUIPMENT_SEED_SPHERE_MIX;
	case SEASON3A::MIXTYPE_ATTACH_SOCKET:
		return REQUEST_EQUIPMENT_ATTACH_SOCKET_MIX;
	case SEASON3A::MIXTYPE_DETACH_SOCKET:
		return REQUEST_EQUIPMENT_DETACH_SOCKET_MIX;
	default:
		assert(!"Mix error");
		return REQUEST_EQUIPMENT_CHAOS_MIX;
	}
}

void CMixRecipeMgr::ResetMixItemInventory()
{
	m_MixItemInventory.Reset();
}

void CMixRecipeMgr::AddItemToMixItemInventory(ITEM * pItem)
{
	m_MixItemInventory.AddItem(pItem);
}

void CMixRecipeMgr::CheckMixInventory()
{
	if (m_MixItemInventory.GetNumMixItems() == 0) m_bIsMixInit = TRUE;
	else m_bIsMixInit = FALSE;

	CheckRecipe(m_MixItemInventory.GetNumMixItems(), m_MixItemInventory.GetMixItems());
	CheckRecipeSimilarity(m_MixItemInventory.GetNumMixItems(), m_MixItemInventory.GetMixItems());
}

int CMixRecipeMgr::GetSeedSphereID(int iOrder)
{
	int iCurrOrder = 0;
	CMixItem * pItems = m_MixItemInventory.GetMixItems();
	for (int i = 0; i < m_MixItemInventory.GetNumMixItems(); ++i)
	{
		if (pItems[i].m_bySeedSphereID != SOCKET_EMPTY)
		{
			if (iCurrOrder == iOrder)
			{
				return pItems[i].m_bySeedSphereID;
			}
			else
			{
				++iCurrOrder;
			}
		}
	}
	return SOCKET_EMPTY;
}
