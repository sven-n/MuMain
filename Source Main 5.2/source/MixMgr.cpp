#include "stdafx.h"
#include "MixMgr.h"

#include "UIManager.h"
#include "ZzzInventory.h"
#include "CSItemOption.h"
#include "UIJewelHarmony.h"
#include "SocketSystem.h"
#include "SkillManager.h"

using namespace SEASON3A;

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

void CMixItem::SetItem(ITEM* pItem, DWORD dwMixValue)
{
    Reset();

    m_sType = pItem->Type;
    m_iLevel = pItem->Level;
    
    m_iDurability = pItem->Durability;
    for (int i = 0; i < pItem->SpecialNum; i++)
    {
        switch (pItem->Special[i])
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
    if (pItem->ExcellentFlags > 0) m_dwSpecialItem |= RCP_SP_EXCELLENT;
    if (pItem->RequireLevel >= 380) m_dwSpecialItem |= RCP_SP_ADD380ITEM;
    if (pItem->AncientDiscriminator > 0) m_dwSpecialItem |= RCP_SP_SETITEM;
    m_b380AddedItem = pItem->option_380;

    if (pItem->Type >= ITEM_SWORD && pItem->Type <= ITEM_BOOTS + MAX_ITEM_INDEX - 1)
        m_bIsEquipment = TRUE;

    if (pItem->Type == ITEM_HORN_OF_FENRIR && pItem->ExcellentFlags != 0)
        m_bFenrirAddedItem = TRUE;

    if (pItem->Type == ITEM_POTION + 53)
        m_bIsCharmItem = TRUE;

    if (pItem->Type == ITEM_POTION + 96)
        m_bIsChaosCharmItem = TRUE;

    if (pItem->Type == ITEM_JEWEL_OF_CHAOS
        || pItem->Type == ITEM_PACKED_JEWEL_OF_BLESS
        || pItem->Type == ITEM_PACKED_JEWEL_OF_SOUL
        || pItem->Type == ITEM_JEWEL_OF_BLESS
        || pItem->Type == ITEM_JEWEL_OF_SOUL
        || pItem->Type == ITEM_JEWEL_OF_LIFE
        || pItem->Type == ITEM_JEWEL_OF_CREATION
        || pItem->Type == ITEM_JEWEL_OF_GUARDIAN
        || pItem->Type == ITEM_JEWEL_OF_HARMONY
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

    switch (pItem->Type)
    {
    case ITEM_WING:
    case ITEM_WINGS_OF_HEAVEN:
    case ITEM_WINGS_OF_SATAN:
    case ITEM_WING_OF_CURSE:
        m_bIsWing = TRUE;
        break;
    case ITEM_WINGS_OF_SPIRITS:
    case ITEM_WINGS_OF_SOUL:
    case ITEM_WINGS_OF_DRAGON:
    case ITEM_WINGS_OF_DARKNESS:
    case ITEM_CAPE_OF_LORD:
    case ITEM_WINGS_OF_DESPAIR:
    case ITEM_CAPE_OF_FIGHTER:
        m_bIsUpgradedWing = TRUE;
        break;
    case ITEM_WING_OF_STORM:
    case ITEM_WING_OF_ETERNAL:
    case ITEM_WING_OF_ILLUSION:
    case ITEM_WING_OF_RUIN:
    case ITEM_CAPE_OF_EMPEROR:
    case ITEM_WING_OF_DIMENSION:
    case ITEM_CAPE_OF_OVERRULE:
        m_bIs3rdUpgradedWing = TRUE;
    }

    if (m_bIsWing || m_bIsUpgradedWing || m_bIs3rdUpgradedWing)
    {
        if (m_dwSpecialItem & RCP_SP_EXCELLENT)
            m_dwSpecialItem ^= RCP_SP_EXCELLENT;
    }
    switch (pItem->Type)
    {
    case ITEM_LARGE_HEALING_POTION:
    case ITEM_SMALL_COMPLEX_POTION:
    case ITEM_MEDIUM_COMPLEX_POTION:
    case ITEM_POTION + 53:
    case ITEM_POTION + 88:
    case ITEM_POTION + 89:
    case ITEM_GOLDEN_CHERRY_BLOSSOM_BRANCH:
    case ITEM_POTION + 100:
        m_bCanStack = TRUE;
        break;
    }
    m_dwMixValue = dwMixValue;

    if (m_bCanStack == TRUE) m_iCount = m_iDurability;
    else m_iCount = 1;
}

int CMixItemInventory::AddItem(ITEM* pItem)
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

DWORD CMixItemInventory::EvaluateMixItemValue(ITEM* pItem)
{
    DWORD dwMixValue = 0;
    switch (pItem->Type)
    {
    case ITEM_JEWEL_OF_CHAOS:
        dwMixValue = 40000;
        break;
    case ITEM_JEWEL_OF_BLESS:
        dwMixValue = 100000;
        break;
    case ITEM_JEWEL_OF_SOUL:
        dwMixValue = 70000;
        break;
    case ITEM_JEWEL_OF_CREATION:
        dwMixValue = 450000;
        break;
    case ITEM_JEWEL_OF_LIFE:
        dwMixValue = 0;
        break;
    case ITEM_JEWEL_OF_GUARDIAN:
    default:
        dwMixValue = ItemValue(pItem, 0);
        break;
    }
    return dwMixValue;
}

void CMixRecipes::Reset()
{
    ClearCheckRecipeResult();
    std::vector<MIX_RECIPE*>::iterator iter;
    for (iter = m_Recipes.begin(); iter != m_Recipes.end(); ++iter)
    {
        if (*iter != NULL)
        {
            delete* iter;
            *iter = NULL;
        }
    }
    m_Recipes.clear();
}

void CMixRecipes::AddRecipe(MIX_RECIPE* pMixRecipe)
{
    if (pMixRecipe != NULL)
        m_Recipes.push_back(pMixRecipe);
}

BOOL CMixRecipes::IsMixSource(ITEM* pItem)
{
    CMixItem mixitem;
    mixitem.SetItem(pItem, 0);

    if (Check_LuckyItem(pItem->Type) && g_MixRecipeMgr.GetMixInventoryType() != MIXTYPE_JERRIDON)	return FALSE;

    if (IsCharmItem(mixitem))
    {
        if ((GetCurRecipe() == NULL || GetCurRecipe()->m_bCharmOption == 'A')
            && m_wTotalCharmBonus + mixitem.m_iCount <= 10)
        {
            return TRUE;
        }
    }

    if (IsChaosCharmItem(mixitem))
    {
        if ((GetCurRecipe() == NULL || GetCurRecipe()->m_bCharmOption == 'A') && m_wTotalChaosCharmCount < 1)
        {
            return TRUE;
        }
    }

    for (std::vector<MIX_RECIPE*>::iterator iter = m_Recipes.begin(); iter != m_Recipes.end(); ++iter)
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

int CMixRecipes::CheckRecipe(int iNumMixItems, CMixItem* pMixItems)
{
    m_iCurMixIndex = 0;

    std::vector<MIX_RECIPE*>::iterator iter;
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

BOOL CMixRecipes::CheckRecipeSub(std::vector<MIX_RECIPE*>::iterator iter, int iNumMixItems, CMixItem* pMixItems)
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
                if (pMixItems[i].m_bIsChaosCharmItem && (*iter)->m_bChaosCharmOption == 'A');
                else
                    return FALSE;
        }
    }
    return TRUE;
}

int CMixRecipes::CheckRecipeSimilarity(int iNumMixItems, CMixItem* pMixItems)
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
    for (auto iter = m_Recipes.begin(); iter != m_Recipes.end(); ++iter)
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

int CMixRecipes::CheckRecipeSimilaritySub(std::vector<MIX_RECIPE*>::iterator iter, int iNumMixItems, CMixItem* pMixItems)
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

bool CMixRecipes::CheckItem(MIX_RECIPE_ITEM& rItem, CMixItem& rSource)
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

MIX_RECIPE* CMixRecipes::GetCurRecipe()
{
    if (m_iCurMixIndex == 0) return NULL;
    return m_Recipes[m_iCurMixIndex - 1];
}

MIX_RECIPE* CMixRecipes::GetMostSimilarRecipe()
{
    if (m_iMostSimilarMixIndex == 0) return NULL;
    return m_Recipes[m_iMostSimilarMixIndex - 1];
}

int CMixRecipes::GetCurMixID()
{
    if (m_iCurMixIndex == 0) return 0;
    return m_Recipes[m_iCurMixIndex - 1]->m_iMixID;
}

BOOL CMixRecipes::GetCurRecipeName(wchar_t* pszNameOut, int iNameLine)
{
    if (!IsReadyToMix())
    {
        if (iNameLine == 1)
        {
            switch (g_MixRecipeMgr.GetMixInventoryType())
            {
            case MIXTYPE_TRAINER:
                swprintf(pszNameOut, GlobalText[1213], GlobalText[1212]);
                break;
            case MIXTYPE_OSBOURNE:
                swprintf(pszNameOut, GlobalText[1213], GlobalText[2061]);
                break;
            case MIXTYPE_JERRIDON:
                swprintf(pszNameOut, GlobalText[1213], GlobalText[2062]);
                break;
            case MIXTYPE_ELPIS:
                swprintf(pszNameOut, GlobalText[1213], GlobalText[2063]);
                break;
            default:
                swprintf(pszNameOut, L"%s", GlobalText[601]);
                break;
            }
            return TRUE;
        }
        else return FALSE;
    }
    return GetRecipeName(GetCurRecipe(), pszNameOut, iNameLine, FALSE);
}

BOOL CMixRecipes::GetRecipeName(MIX_RECIPE* pRecipe, wchar_t* pszNameOut, int iNameLine, BOOL bSimilarRecipe)
{
    if (pRecipe == NULL) return FALSE;
    if (iNameLine > 2 || iNameLine < 1) return FALSE;
    if (pRecipe->m_bMixOption == 'C')
    {
        std::vector<std::wstring> optionTextlist;

        g_pItemAddOptioninfo->GetItemAddOtioninfoText(optionTextlist, m_iFirstItemType);
        if (optionTextlist.empty() || bSimilarRecipe)
        {
            if (iNameLine == 1)
            {
                swprintf(pszNameOut, L"%s", GlobalText[2194]);
                return TRUE;
            }
            return FALSE;
        }
        assert(optionTextlist.size() == 2 && L"옵션은 2개여야 함");
        if (iNameLine == 1)
        {
            wcscpy(pszNameOut, optionTextlist[0].c_str());
            return TRUE;
        }
        else if (iNameLine == 2)
        {
            wcscpy(pszNameOut, optionTextlist[1].c_str());
            return TRUE;
        }
        return FALSE;
    }
    else
    {
        if (iNameLine == 1)
        {
            if (pRecipe->m_iMixName[1] == 0)
                swprintf(pszNameOut, L"%s", GlobalText[pRecipe->m_iMixName[0]]);
            else if (pRecipe->m_iMixName[2] == 0)
                swprintf(pszNameOut, L"%s %s", GlobalText[pRecipe->m_iMixName[0]], GlobalText[pRecipe->m_iMixName[1]]);
            else
                swprintf(pszNameOut, L"%s %s %s", GlobalText[pRecipe->m_iMixName[0]],
                    GlobalText[pRecipe->m_iMixName[1]], GlobalText[pRecipe->m_iMixName[2]]);
            return TRUE;
        }
        return FALSE;
    }
}

BOOL CMixRecipes::GetCurRecipeDesc(wchar_t* pszDescOut, int iDescLine)
{
    if (iDescLine > 3 || iDescLine < 1) return FALSE;
    if (GetCurRecipe() == NULL) return FALSE;
    if (GetCurRecipe()->m_iMixDesc[iDescLine - 1] > 0)
        wcscpy(pszDescOut, GlobalText[GetCurRecipe()->m_iMixDesc[iDescLine - 1]]);
    else
        return FALSE;
    return TRUE;
}

BOOL CMixRecipes::GetMostSimilarRecipeName(wchar_t* pszNameOut, int iNameLine)
{
    return GetRecipeName(GetMostSimilarRecipe(), pszNameOut, iNameLine, TRUE);
}

BOOL CMixRecipes::GetRecipeAdvice(wchar_t* pszAdviceOut, int iAdivceLine)
{
    if (GetMostSimilarRecipe() == NULL) return FALSE;
    if (iAdivceLine > 3 || iAdivceLine < 1) return FALSE;

    if (GetMostSimilarRecipe()->m_iMixAdvice[iAdivceLine - 1] > 0)
        wcscpy(pszAdviceOut, GlobalText[GetMostSimilarRecipe()->m_iMixAdvice[iAdivceLine - 1]]);
    else
        return FALSE;
    return TRUE;
}

int CMixRecipes::GetSourceName(int iItemNum, wchar_t* pszNameOut, int iNumMixItems, CMixItem* pMixItems)
{
    if (iNumMixItems < 0)	return MIX_SOURCE_ERROR;
    if (pMixItems == NULL)	return MIX_SOURCE_ERROR;

    if (GetMostSimilarRecipe() == NULL) return MIX_SOURCE_ERROR;
    if (iItemNum >= GetMostSimilarRecipe()->m_iNumMixSoruces) return MIX_SOURCE_ERROR;

    MIX_RECIPE_ITEM* pMixRecipeItem = &GetMostSimilarRecipe()->m_MixSources[iItemNum];

    wchar_t szTempName[100];
    GetItemName(pMixRecipeItem->m_sTypeMin, pMixRecipeItem->m_iLevelMin, szTempName);

    if (pMixRecipeItem->m_sTypeMin == pMixRecipeItem->m_sTypeMax &&
        (pMixRecipeItem->m_iLevelMin == pMixRecipeItem->m_iLevelMax || (pMixRecipeItem->m_iLevelMin == 0 && pMixRecipeItem->m_iLevelMax == 255)) &&
        (pMixRecipeItem->m_iOptionMin == pMixRecipeItem->m_iOptionMax || (pMixRecipeItem->m_iOptionMin == 0 && pMixRecipeItem->m_iOptionMax == 255)))
    {
        if (pMixRecipeItem->m_iDurabilityMin == pMixRecipeItem->m_iDurabilityMax)
            swprintf(szTempName, L"%s(%d)", szTempName, pMixRecipeItem->m_iDurabilityMin);
    }
    else
    {
        if (pMixRecipeItem->m_dwSpecialItem & RCP_SP_ADD380ITEM)
            swprintf(szTempName, GlobalText[2335]);
        else if (pMixRecipeItem->m_sTypeMin == 0 && pMixRecipeItem->m_sTypeMax == ITEM_BOOTS + MAX_ITEM_INDEX - 1)
            swprintf(szTempName, GlobalText[2336]);
        else if (pMixRecipeItem->m_sTypeMin == 0 && pMixRecipeItem->m_sTypeMax == ITEM_HELPER + MAX_ITEM_INDEX - 1)
            swprintf(szTempName, GlobalText[2336]);
        else if (pMixRecipeItem->m_sTypeMin == 0 && pMixRecipeItem->m_sTypeMax == ITEM_STAFF + MAX_ITEM_INDEX - 1)
            swprintf(szTempName, GlobalText[2337]);
        else if (pMixRecipeItem->m_sTypeMin == ITEM_SHIELD && pMixRecipeItem->m_sTypeMax == ITEM_BOOTS + MAX_ITEM_INDEX - 1)
            swprintf(szTempName, GlobalText[2338]);
        else if (pMixRecipeItem->m_sTypeMin == ITEM_WING && pMixRecipeItem->m_sTypeMax == ITEM_WINGS_OF_SATAN)
            swprintf(szTempName, GlobalText[2339]);
        else if (pMixRecipeItem->m_sTypeMin == ITEM_WINGS_OF_SPIRITS && pMixRecipeItem->m_sTypeMax == ITEM_WINGS_OF_DARKNESS)
            swprintf(szTempName, GlobalText[2348]);
        else if (pMixRecipeItem->m_sTypeMin == ITEM_WING_OF_CURSE && pMixRecipeItem->m_sTypeMax == ITEM_WING_OF_CURSE)
            swprintf(szTempName, GlobalText[2339]);
        else if (pMixRecipeItem->m_sTypeMin == ITEM_WINGS_OF_DESPAIR && pMixRecipeItem->m_sTypeMax == ITEM_WINGS_OF_DESPAIR)
            swprintf(szTempName, GlobalText[2348]);
        else if (pMixRecipeItem->m_sTypeMin == pMixRecipeItem->m_sTypeMax &&
            (pMixRecipeItem->m_sTypeMin == ITEM_CHAOS_DRAGON_AXE || pMixRecipeItem->m_sTypeMin == ITEM_CHAOS_NATURE_BOW || pMixRecipeItem->m_sTypeMin == ITEM_CHAOS_LIGHTNING_STAFF))
            swprintf(szTempName, GlobalText[2340]);
        else if (pMixRecipeItem->m_sTypeMin == ITEM_SEED_FIRE && pMixRecipeItem->m_sTypeMax == ITEM_SEED_EARTH)
            swprintf(szTempName, GlobalText[2680]);
        else if (pMixRecipeItem->m_sTypeMin == ITEM_SPHERE_MONO && pMixRecipeItem->m_sTypeMax == ITEM_SPHERE_5)
            swprintf(szTempName, GlobalText[2681]);
        else if (pMixRecipeItem->m_sTypeMin == ITEM_SEED_SPHERE_FIRE_1 && pMixRecipeItem->m_sTypeMax == ITEM_SEED_SPHERE_EARTH_5)
            swprintf(szTempName, GlobalText[2682]);
        else if (pMixRecipeItem->m_sTypeMin == ITEM_SEED_SPHERE_FIRE_1 && pMixRecipeItem->m_sTypeMax == ITEM_SEED_SPHERE_LIGHTNING_5)
            swprintf(szTempName, L"%s (%s)", GlobalText[2682], GlobalText[2684]);
        else if (pMixRecipeItem->m_sTypeMin == ITEM_SEED_SPHERE_WATER_1 && pMixRecipeItem->m_sTypeMax == ITEM_SEED_SPHERE_EARTH_5)
            swprintf(szTempName, L"%s (%s)", GlobalText[2682], GlobalText[2685]);
        else
        {
            int iNameLen = wcslen(szTempName);
            for (int j = 1; j <= 3 && iNameLen - j - 1 >= 0; ++j)
                if (szTempName[iNameLen - j] == '+') szTempName[iNameLen - j - 1] = '\0';
        }
        if (pMixRecipeItem->m_iDurabilityMin == pMixRecipeItem->m_iDurabilityMax)
            swprintf(szTempName, L"%s(%d)", szTempName, pMixRecipeItem->m_iDurabilityMin);

        if (pMixRecipeItem->m_iLevelMin == 0 && pMixRecipeItem->m_iLevelMax == 255);
        else if (pMixRecipeItem->m_iLevelMin == pMixRecipeItem->m_iLevelMax)
            swprintf(szTempName, L"%s +%d", szTempName, pMixRecipeItem->m_iLevelMin);
        else if (pMixRecipeItem->m_iLevelMin == 0)
            swprintf(szTempName, L"%s +%d%s", szTempName, pMixRecipeItem->m_iLevelMax, GlobalText[2342]);
        else if (pMixRecipeItem->m_iLevelMax == 255)
            swprintf(szTempName, L"%s +%d%s", szTempName, pMixRecipeItem->m_iLevelMin, GlobalText[2341]);
        else
            swprintf(szTempName, L"%s +%d~%d", szTempName, pMixRecipeItem->m_iLevelMin, pMixRecipeItem->m_iLevelMax);

        if (pMixRecipeItem->m_iOptionMin == 0 && pMixRecipeItem->m_iOptionMax == 255);
        else if (pMixRecipeItem->m_iOptionMin == pMixRecipeItem->m_iOptionMax)
            swprintf(szTempName, L"%s +%d%s", szTempName, pMixRecipeItem->m_iOptionMin, GlobalText[2343]);
        else if (pMixRecipeItem->m_iOptionMin == 0)
            swprintf(szTempName, L"%s +%d%s%s", szTempName, pMixRecipeItem->m_iOptionMax, GlobalText[2343], GlobalText[2342]);
        else if (pMixRecipeItem->m_iOptionMax == 255)
            swprintf(szTempName, L"%s +%d%s%s", szTempName, pMixRecipeItem->m_iOptionMin, GlobalText[2343], GlobalText[2341]);
        else
            swprintf(szTempName, L"%s +%d~%d%s", szTempName, pMixRecipeItem->m_iOptionMin, pMixRecipeItem->m_iOptionMax, GlobalText[2343]);
    }

    if (pMixRecipeItem->m_iCountMin == 0 && pMixRecipeItem->m_iCountMax == 255)
        swprintf(szTempName, L"%s (%s)", szTempName, GlobalText[2344]);
    else if (pMixRecipeItem->m_iCountMin == pMixRecipeItem->m_iCountMax)
        swprintf(szTempName, L"%s %d%s", szTempName, pMixRecipeItem->m_iCountMin, GlobalText[2345]);
    else if (pMixRecipeItem->m_iCountMin == 0)
        swprintf(szTempName, L"%s %d%s %s", szTempName, pMixRecipeItem->m_iCountMax, GlobalText[2345], GlobalText[2342]);
    else if (pMixRecipeItem->m_iCountMax == 255)
        swprintf(szTempName, L"%s %d%s %s", szTempName, pMixRecipeItem->m_iCountMin, GlobalText[2345], GlobalText[2341]);
    else
        swprintf(szTempName, L"%s %d~%d%s", szTempName, pMixRecipeItem->m_iCountMin, pMixRecipeItem->m_iCountMax, GlobalText[2345]);

    BOOL bPreName = FALSE;
    if (pMixRecipeItem->m_dwSpecialItem & RCP_SP_EXCELLENT)
    {
        swprintf(pszNameOut, L"%s %s", GlobalText[620], szTempName);
        bPreName = TRUE;
    }
    if (pMixRecipeItem->m_dwSpecialItem & RCP_SP_SETITEM)
    {
        swprintf(pszNameOut, L"%s %s", GlobalText[1089], szTempName);
        bPreName = TRUE;
    }
    if (pMixRecipeItem->m_dwSpecialItem & RCP_SP_HARMONY)
    {
        swprintf(pszNameOut, L"%s %s", GlobalText[1550], szTempName);
        bPreName = TRUE;
    }
    if (pMixRecipeItem->m_dwSpecialItem & RCP_SP_SOCKETITEM)
    {
        swprintf(pszNameOut, L"%s %s", GlobalText[2650], szTempName);
        bPreName = TRUE;
    }
    if (bPreName == FALSE)
    {
        wcscpy(pszNameOut, szTempName);
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

void CMixRecipes::EvaluateMixItems(int iNumMixItems, CMixItem* pMixItems)
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

void CMixRecipes::CalcCharmBonusRate(int iNumMixItems, CMixItem* pMixItems)
{
    m_wTotalCharmBonus = 0;
    for (int i = 0; i < iNumMixItems; ++i)
    {
        if (pMixItems[i].m_bIsCharmItem == TRUE) m_wTotalCharmBonus += pMixItems[i].m_iCount;
    }
}

void CMixRecipes::CalcChaosCharmCount(int iNumMixItems, CMixItem* pMixItems)
{
    m_wTotalChaosCharmCount = 0;
    for (int i = 0; i < iNumMixItems; ++i)
    {
        if (pMixItems[i].m_bIsChaosCharmItem == TRUE) m_wTotalChaosCharmCount += 1;
    }
}

void CMixRecipes::CalcMixRate(int iNumMixItems, CMixItem* pMixItems)
{
    if (iNumMixItems < 0)	return;
    if (pMixItems == NULL)	return;

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
    while (1)
    {
        if (m_iMixRateIter >= GetCurRecipe()->m_iNumRateData || m_pMixRates[m_iMixRateIter].op == MRCP_RP)
        {
            return fLvalue;
        }
        switch (m_pMixRates[m_iMixRateIter].op)
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
    while (1)
    {
        if (m_iMixRateIter >= GetCurRecipe()->m_iNumRateData || m_pMixRates[m_iMixRateIter].op == MRCP_RP)
        {
            return fLvalue;
        }
        switch (m_pMixRates[m_iMixRateIter].op)
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
    switch (m_pMixRates[m_iMixRateIter].op)
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

void CMixRecipes::CalcMixReqZen(int iNumMixItems, CMixItem* pMixItems)
{
    if (iNumMixItems < 0)	return;
    if (pMixItems == NULL)	return;

    m_dwRequiredZen = 0;
    if (GetCurRecipe() == NULL) return;
    switch (GetCurRecipe()->m_bRequiredZenType)
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
        if (ITEM_SWORD <= pMixItems[0].m_sType && ITEM_STAFF > pMixItems[0].m_sType)
        {
            iItemType = SI_Weapon;
        }
        else if (ITEM_STAFF <= pMixItems[0].m_sType && ITEM_SHIELD > pMixItems[0].m_sType)
        {
            iItemType = SI_Staff;
        }
        else if (ITEM_SHIELD <= pMixItems[0].m_sType && ITEM_WING > pMixItems[0].m_sType)
        {
            iItemType = SI_Defense;
        }
        m_dwRequiredZen = g_pUIJewelHarmonyinfo->GetHarmonyJewelOptionInfo(iItemType,
            pMixItems[0].m_wHarmonyOption).Zen[pMixItems[0].m_wHarmonyOptionLevel];
    }
    break;
    default:
        break;
    }
}

BOOL CMixRecipes::IsChaosItem(CMixItem& rSource)
{
    if (rSource.m_sType == ITEM_CHAOS_DRAGON_AXE || rSource.m_sType == ITEM_CHAOS_NATURE_BOW || rSource.m_sType == ITEM_CHAOS_LIGHTNING_STAFF) return TRUE;
    return FALSE;
}

BOOL CMixRecipes::IsChaosJewel(CMixItem& rSource)
{
    if (rSource.m_sType == ITEM_JEWEL_OF_CHAOS) return TRUE;
    return FALSE;
}

BOOL CMixRecipes::Is380AddedItem(CMixItem& rSource)
{
    return rSource.m_b380AddedItem;
}

BOOL CMixRecipes::IsFenrirAddedItem(CMixItem& rSource)
{
    return rSource.m_bFenrirAddedItem;
}

BOOL CMixRecipes::IsUpgradableItem(CMixItem& rSource)
{
    return (rSource.m_bIsEquipment || rSource.m_bIsWing || rSource.m_bIsUpgradedWing || rSource.m_bIs3rdUpgradedWing);
}

BOOL CMixRecipes::IsSourceOfRefiningStone(CMixItem& rSource)
{
    if (rSource.m_iLevel < 4)
    {
        switch (rSource.m_sType)
        {
        case ITEM_KRIS: case ITEM_SHORT_SWORD: case ITEM_RAPIER: case ITEM_SWORD_OF_ASSASSIN:
        case ITEM_SMALL_AXE: case ITEM_HAND_AXE: case ITEM_DOUBLE_AXE:
        case ITEM_SMALLMACE: case ITEM_MORNING_STAR: case ITEM_FLAIL:
        case ITEM__SPEAR: case ITEM_DRAGON_LANCE: case ITEM_GIANT_TRIDENT: case ITEM_DOUBLE_POLEAXE: case ITEM_HALBERD:
        case ITEM_SHORT_BOW: case ITEM_SMALL_BOW: case ITEM_ELVEN_BOW: case ITEM_BATTLE_BOW:
        case ITEM_CROSSBOW: case ITEM_GOLDEN_CROSSBOW: case ITEM_ARQUEBUS: case ITEM_LIGHT_CROSSBOW:
        case ITEM_SKULL_STAFF: case ITEM_ANGELIC_STAFF: case ITEM_SERPENT_STAFF:
        case ITEM_SMALL_SHIELD: case ITEM_HORN_SHIELD: case ITEM_KITE_SHIELD: case ITEM_ELVEN_SHIELD: case ITEM_BUCKLER:
        case ITEM_SKULL_SHIELD: case ITEM_SPIKED_SHIELD: case ITEM_PLATE_SHIELD: case ITEM_BIG_ROUND_SHIELD:
        case ITEM_BRONZE_HELM: case ITEM_PAD_HELM: case ITEM_BONE_HELM: case ITEM_LEATHER_HELM: case ITEM_SCALE_HELM:
        case ITEM_SPHINX_MASK: case ITEM_BRASS_HELM: case ITEM_VINE_HELM: case ITEM_SILK_HELM: case ITEM_WIND_HELM:
        case ITEM_BRONZE_ARMOR: case ITEM_PAD_ARMOR: case ITEM_BONE_ARMOR: case ITEM_LEATHER_ARMOR: case ITEM_SCALE_ARMOR:
        case ITEM_SPHINX_ARMOR: case ITEM_BRASS_ARMOR: case ITEM_VINE_ARMOR: case ITEM_SILK_ARMOR: case ITEM_WIND_ARMOR:
        case ITEM_BRONZE_PANTS: case ITEM_PAD_PANTS: case ITEM_BONE_PANTS: case ITEM_LEATHER_PANTS: case ITEM_SCALE_PANTS:
        case ITEM_SPHINX_PANTS: case ITEM_BRASS_PANTS: case ITEM_VINE_PANTS: case ITEM_SILK_PANTS: case ITEM_WIND_PANTS:
        case ITEM_BRONZE_GLOVES: case ITEM_PAD_GLOVES: case ITEM_BONE_GLOVES: case ITEM_LEATHER_GLOVES: case ITEM_SCALE_GLOVES:
        case ITEM_SPHINX_GLOVES: case ITEM_BRASS_GLOVES: case ITEM_VINE_GLOVES: case ITEM_SILK_GLOVES: case ITEM_WIND_GLOVES:
        case ITEM_BRONZE_BOOTS: case ITEM_PAD_BOOTS: case ITEM_BONE_BOOTS: case ITEM_LEATHER_BOOTS: case ITEM_SCALE_BOOTS:
        case ITEM_SPHINX_BOOTS: case ITEM_BRASS_BOOTS: case ITEM_VINE_BOOTS: case ITEM_SILK_BOOTS: case ITEM_WIND_BOOTS:
            return FALSE;
        }
    }
    return TRUE;
}

BOOL CMixRecipes::IsSourceOfAttachSeedSphereToWeapon(CMixItem& rSource)
{
    if (rSource.m_sType >= ITEM_SEED_SPHERE_FIRE_1 && rSource.m_sType <= ITEM_SEED_SPHERE_EARTH_5)
    {
        int iSeedSphereType = rSource.m_sType - ITEM_WING;
        if (iSeedSphereType % 2 == 0) return TRUE;
    }
    return FALSE;
}

BOOL CMixRecipes::IsSourceOfAttachSeedSphereToArmor(CMixItem& rSource)
{
    if (rSource.m_sType >= ITEM_SEED_SPHERE_FIRE_1 && rSource.m_sType <= ITEM_SEED_SPHERE_EARTH_5)
    {
        int iSeedSphereType = rSource.m_sType - ITEM_WING;

        if (iSeedSphereType % 2 == 1) return TRUE;
    }
    return FALSE;
}

BOOL CMixRecipes::IsCharmItem(CMixItem& rSource)
{
    return rSource.m_bIsCharmItem;
}

BOOL CMixRecipes::IsChaosCharmItem(CMixItem& rSource)
{
    return rSource.m_bIsChaosCharmItem;
}

BOOL CMixRecipes::IsJewelItem(CMixItem& rSource)
{
    return rSource.m_bIsJewelItem;
}

void CMixRecipeMgr::OpenRecipeFile(const wchar_t* szFileName)
{
    int i, j;
    for (j = 0; j < MAX_MIX_TYPES; ++j)
    {
        m_MixRecipe[j].Reset();
    }

    FILE* fp = _wfopen(szFileName, L"rb");
    if (fp == NULL)
    {
        wchar_t Text[256];
        swprintf(Text, L"%s - File not exist.", szFileName);
        g_ErrorReport.Write(Text);
        MessageBox(g_hWnd, Text, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
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
            wchar_t Text[256];
            swprintf(Text, L"%s - Version not matched.", szFileName);
            g_ErrorReport.Write(Text);
            MessageBox(g_hWnd, Text, NULL, MB_OK);
            SendMessage(g_hWnd, WM_DESTROY, 0, 0);
            fclose(fp);
            exit(0);
        }
        for (i = 0; i < iNumMixRecipes[j]; ++i)
        {
            auto* pMixRecipe = new MIX_RECIPE;
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

STORAGE_TYPE CMixRecipeMgr::GetMixInventoryEquipmentIndex()
{
    switch (GetMixInventoryType())
    {
    case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
    case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
    case SEASON3A::MIXTYPE_GOBLIN_ADD380:
        return STORAGE_TYPE::CHAOS_MIX;
    case SEASON3A::MIXTYPE_CASTLE_SENIOR:
        return STORAGE_TYPE::CHAOS_MIX;
    case SEASON3A::MIXTYPE_TRAINER:
        return STORAGE_TYPE::TRAINER_MIX;
    case SEASON3A::MIXTYPE_OSBOURNE:
        return STORAGE_TYPE::OSBOURNE_MIX;
    case SEASON3A::MIXTYPE_JERRIDON:
        return STORAGE_TYPE::JERRIDON_MIX;
    case SEASON3A::MIXTYPE_ELPIS:
        return STORAGE_TYPE::ELPIS_MIX;
    case SEASON3A::MIXTYPE_CHAOS_CARD:
        return STORAGE_TYPE::CHAOS_CARD_MIX;
    case SEASON3A::MIXTYPE_CHERRYBLOSSOM:
        return STORAGE_TYPE::CHERRYBLOSSOM_MIX;
    case SEASON3A::MIXTYPE_EXTRACT_SEED:
        return STORAGE_TYPE::EXTRACT_SEED_MIX;
    case SEASON3A::MIXTYPE_SEED_SPHERE:
        return STORAGE_TYPE::SEED_SPHERE_MIX;
    case SEASON3A::MIXTYPE_ATTACH_SOCKET:
        return STORAGE_TYPE::ATTACH_SOCKET_MIX;
    case SEASON3A::MIXTYPE_DETACH_SOCKET:
        return STORAGE_TYPE::DETACH_SOCKET_MIX;
    default:
        assert(!"Mix error");
        return STORAGE_TYPE::CHAOS_MIX;
    }
}

void CMixRecipeMgr::ResetMixItemInventory()
{
    m_MixItemInventory.Reset();
}

void CMixRecipeMgr::AddItemToMixItemInventory(ITEM* pItem)
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
    CMixItem* pItems = m_MixItemInventory.GetMixItems();
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