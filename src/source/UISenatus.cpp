//////////////////////////////////////////////////////////////////////////
//  UIGuardsMan.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "UIManager.h"
#include "UIGuardsMan.h"


#include "MixMgr.h"

extern DWORD		g_dwActiveUIID;



extern int   ShopInventoryStartX;
extern int   ShopInventoryStartY;

CSenatusInfo g_SenatusInfo;

CSenatusInfo::CSenatusInfo()
{
    m_iCurrGate = 0;
    m_iCurrStatue = 0;
    memset(m_GateInfo, 0, sizeof(PMSG_NPCDBLIST) * 6);
    memset(m_StatueInfo, 0, sizeof(PMSG_NPCDBLIST) * 6);

    m_iChaosTaxRate = 0;
    m_iNormalTaxRate = 0;
    m_iRealTaxRateChaos = 0;
    m_iRealTaxRateStore = 0;
    m_i64CastleMoney = 0;
}

CSenatusInfo::~CSenatusInfo()
{
}

int CSenatusInfo::GetRepairCost(LPPMSG_NPCDBLIST pInfo)
{
    if (pInfo->iNpcNumber == GATENPC_NUMBER)
        return (pInfo->iNpcMaxHp - pInfo->iNpcHp) * 5 + pInfo->iNpcDfLevel * 1000000;
    else if (pInfo->iNpcNumber == STATUENPC_NUMBER)
        return (pInfo->iNpcMaxHp - pInfo->iNpcHp) * 3 + pInfo->iNpcDfLevel * 1000000;
    else
        return -1;
}

int CSenatusInfo::GetHP(int nType, int nLevel)
{
    if (nType == GATENPC_NUMBER)
    {
        if (nLevel == 0)		return GATELEVEL_HP_0;
        else if (nLevel == 1)	return GATELEVEL_HP_1;
        else if (nLevel == 2)	return GATELEVEL_HP_2;
        else					return GATELEVEL_HP_3;
    }
    else if (nType == STATUENPC_NUMBER)
    {
        if (nLevel == 0)		return STATUELEVEL_HP_0;
        else if (nLevel == 1)	return STATUELEVEL_HP_1;
        else if (nLevel == 2)	return STATUELEVEL_HP_2;
        else					return STATUELEVEL_HP_3;
    }
    else
        return -1;
}

int CSenatusInfo::GetHPLevel(LPPMSG_NPCDBLIST pInfo)
{
    if (pInfo->iNpcNumber == GATENPC_NUMBER)
    {
        if (pInfo->iNpcMaxHp <= GATELEVEL_HP_0)
            return 0;
        else if (GATELEVEL_HP_0 < pInfo->iNpcMaxHp && pInfo->iNpcMaxHp <= GATELEVEL_HP_1)
            return 1;
        else if (GATELEVEL_HP_1 < pInfo->iNpcMaxHp && pInfo->iNpcMaxHp <= GATELEVEL_HP_2)
            return 2;
        else
            return 3;
    }
    else if (pInfo->iNpcNumber == STATUENPC_NUMBER)
    {
        if (pInfo->iNpcMaxHp <= STATUELEVEL_HP_0)
            return 0;
        else if (STATUELEVEL_HP_0 < pInfo->iNpcMaxHp && pInfo->iNpcMaxHp <= STATUELEVEL_HP_1)
            return 1;
        else if (STATUELEVEL_HP_1 < pInfo->iNpcMaxHp && pInfo->iNpcMaxHp <= STATUELEVEL_HP_2)
            return 2;
        else
            return 3;
    }
    else
        return -1;
}

int CSenatusInfo::GetNextAddHP(LPPMSG_NPCDBLIST pInfo)
{
    if (pInfo->iNpcNumber == GATENPC_NUMBER)
    {
        if (pInfo->iNpcMaxHp == GATELEVEL_HP_0)
            return GATELEVEL_HP_1 - GATELEVEL_HP_0;
        if (pInfo->iNpcMaxHp == GATELEVEL_HP_1)
            return GATELEVEL_HP_2 - GATELEVEL_HP_1;
        else if (pInfo->iNpcMaxHp == GATELEVEL_HP_2)
            return GATELEVEL_HP_3 - GATELEVEL_HP_2;
        else
            return 0;
    }
    else if (pInfo->iNpcNumber == STATUENPC_NUMBER)
    {
        if (pInfo->iNpcMaxHp == STATUELEVEL_HP_0)
            return STATUELEVEL_HP_1 - STATUELEVEL_HP_0;
        if (pInfo->iNpcMaxHp == STATUELEVEL_HP_1)
            return STATUELEVEL_HP_2 - STATUELEVEL_HP_1;
        else if (pInfo->iNpcMaxHp == STATUELEVEL_HP_2)
            return STATUELEVEL_HP_3 - STATUELEVEL_HP_2;
        else
            return 0;
    }
    else
        return -1;
}

int CSenatusInfo::GetDefense(int nType, int nLevel)
{
    if (nType == GATENPC_NUMBER)
    {
        if (nLevel == 0)		return GATELEVEL_DEFENSE_0;
        else if (nLevel == 1)	return GATELEVEL_DEFENSE_1;
        else if (nLevel == 2)	return GATELEVEL_DEFENSE_2;
        else					return GATELEVEL_DEFENSE_3;
    }
    else if (nType == STATUENPC_NUMBER)
    {
        if (nLevel == 0)		return STATUELEVEL_DEFENSE_0;
        else if (nLevel == 1)	return STATUELEVEL_DEFENSE_1;
        else if (nLevel == 2)	return STATUELEVEL_DEFENSE_2;
        else					return STATUELEVEL_DEFENSE_3;
    }
    else
        return -1;
}

int CSenatusInfo::GetDefenseLevel(LPPMSG_NPCDBLIST pInfo)
{
    if (pInfo->iNpcNumber == GATENPC_NUMBER)
        return pInfo->iNpcDfLevel;
    else if (pInfo->iNpcNumber == STATUENPC_NUMBER)
        return pInfo->iNpcDfLevel;
    else
        return -1;
}

int CSenatusInfo::GetNextAddDefense(LPPMSG_NPCDBLIST pInfo)
{
    if (pInfo->iNpcNumber == GATENPC_NUMBER)
    {
        if (pInfo->iNpcDfLevel == 0)
            return GATELEVEL_DEFENSE_1 - GATELEVEL_DEFENSE_0;
        if (pInfo->iNpcDfLevel == 1)
            return GATELEVEL_DEFENSE_2 - GATELEVEL_DEFENSE_1;
        else if (pInfo->iNpcDfLevel == 2)
            return GATELEVEL_DEFENSE_3 - GATELEVEL_DEFENSE_2;
        else
            return 0;
    }
    else if (pInfo->iNpcNumber == STATUENPC_NUMBER)
    {
        if (pInfo->iNpcDfLevel == 0)
            return STATUELEVEL_DEFENSE_1 - STATUELEVEL_DEFENSE_0;
        if (pInfo->iNpcDfLevel == 1)
            return STATUELEVEL_DEFENSE_2 - STATUELEVEL_DEFENSE_1;
        else if (pInfo->iNpcDfLevel == 2)
            return STATUELEVEL_DEFENSE_3 - STATUELEVEL_DEFENSE_2;
        else
            return 0;
    }
    else
        return -1;
}

int CSenatusInfo::GetRecover(int nType, int nLevel)
{
    if (nType == GATENPC_NUMBER)
    {
        return 0;
    }
    else if (nType == STATUENPC_NUMBER)
    {
        if (nLevel == 0)		return STATUELEVEL_RECOVER_0;
        else if (nLevel == 1)	return STATUELEVEL_RECOVER_1;
        else if (nLevel == 2)	return STATUELEVEL_RECOVER_2;
        else					return STATUELEVEL_RECOVER_3;
    }
    else
        return -1;
}

int CSenatusInfo::GetRecoverLevel(LPPMSG_NPCDBLIST pInfo)
{
    if (pInfo->iNpcNumber == GATENPC_NUMBER)
        return 0;
    else if (pInfo->iNpcNumber == STATUENPC_NUMBER)
        return pInfo->iNpcRgLevel;
    else
        return -1;
}

int CSenatusInfo::GetNextAddRecover(LPPMSG_NPCDBLIST pInfo)
{
    if (pInfo->iNpcNumber == GATENPC_NUMBER)
    {
        return 0;
    }
    else if (pInfo->iNpcNumber == STATUENPC_NUMBER)
    {
        if (pInfo->iNpcRgLevel == STATUELEVEL_RECOVER_0)
            return STATUELEVEL_RECOVER_1 - STATUELEVEL_RECOVER_0;
        if (pInfo->iNpcRgLevel == STATUELEVEL_RECOVER_1)
            return STATUELEVEL_RECOVER_2 - STATUELEVEL_RECOVER_1;
        else if (pInfo->iNpcRgLevel == STATUELEVEL_RECOVER_2)
            return STATUELEVEL_RECOVER_3 - STATUELEVEL_RECOVER_2;
        else
            return 0;
    }
    else
        return -1;
}

void CSenatusInfo::DoGateRepairAction()
{
    if (GetCurrGateInfo().btNpcLive == 0)
    {
        SocketClient->ToGameServer()->SendCastleSiegeDefenseBuyRequest(GetCurrGateInfo().iNpcNumber, GetCurrGateInfo().iNpcIndex);
    }
    else
    {
        SocketClient->ToGameServer()->SendCastleSiegeDefenseRepairRequest(GetCurrGateInfo().iNpcNumber, GetCurrGateInfo().iNpcIndex);
    }
}

void CSenatusInfo::DoGateUpgradeHPAction()
{
    int nHPLevel = g_SenatusInfo.GetHPLevel(&GetCurrGateInfo());
    int nNextHP = g_SenatusInfo.GetHP(GetCurrGateInfo().iNpcNumber, nHPLevel + 1);
    SocketClient->ToGameServer()->SendCastleSiegeDefenseUpgradeRequest(GetCurrGateInfo().iNpcNumber, GetCurrGateInfo().iNpcIndex, NPCUPGRADE_HP, nNextHP);
}

void CSenatusInfo::DoGateUpgradeDefenseAction()
{
    int nLevel = g_SenatusInfo.GetDefenseLevel(&GetCurrGateInfo()) + 1;
    SocketClient->ToGameServer()->SendCastleSiegeDefenseUpgradeRequest(GetCurrGateInfo().iNpcNumber, GetCurrGateInfo().iNpcIndex, NPCUPGRADE_DEFENSE, nLevel);
}

void CSenatusInfo::DoStatueRepairAction()
{
    if (GetCurrStatueInfo().btNpcLive == 0)
    {
        SocketClient->ToGameServer()->SendCastleSiegeDefenseBuyRequest(GetCurrStatueInfo().iNpcNumber, GetCurrStatueInfo().iNpcIndex);
    }
    else
    {
        SocketClient->ToGameServer()->SendCastleSiegeDefenseRepairRequest(GetCurrStatueInfo().iNpcNumber, GetCurrStatueInfo().iNpcIndex);
    }
}

void CSenatusInfo::DoStatueUpgradeHPAction()
{
    int nHPLevel = g_SenatusInfo.GetHPLevel(&GetCurrStatueInfo());
    int nNextHP = g_SenatusInfo.GetHP(GetCurrStatueInfo().iNpcNumber, nHPLevel + 1);
    SocketClient->ToGameServer()->SendCastleSiegeDefenseUpgradeRequest(GetCurrStatueInfo().iNpcNumber, GetCurrStatueInfo().iNpcIndex, NPCUPGRADE_HP, nNextHP);
}

void CSenatusInfo::DoStatueUpgradeDefenseAction()
{
    int nLevel = g_SenatusInfo.GetDefenseLevel(&GetCurrStatueInfo()) + 1;
    SocketClient->ToGameServer()->SendCastleSiegeDefenseUpgradeRequest(GetCurrStatueInfo().iNpcNumber, GetCurrStatueInfo().iNpcIndex, NPCUPGRADE_DEFENSE, nLevel);
}

void CSenatusInfo::DoStatueUpgradeRecoverAction()
{
    int nLevel = g_SenatusInfo.GetRecoverLevel(&GetCurrStatueInfo()) + 1;
    SocketClient->ToGameServer()->SendCastleSiegeDefenseUpgradeRequest(GetCurrStatueInfo().iNpcNumber, GetCurrStatueInfo().iNpcIndex, NPCUPGRADE_RECOVER, nLevel);
}

void CSenatusInfo::DoApplyTaxAction()
{
    SocketClient->ToGameServer()->SendCastleSiegeTaxChangeRequest(1, m_iChaosTaxRate);
    SocketClient->ToGameServer()->SendCastleSiegeTaxChangeRequest(2, m_iNormalTaxRate);
}

void CSenatusInfo::DoWithdrawAction(DWORD dwMoney)
{
    SocketClient->ToGameServer()->SendCastleSiegeTaxMoneyWithdraw(dwMoney);
}

void CSenatusInfo::SetNPCInfo(LPPMSG_NPCDBLIST pInfo)
{
    if (pInfo->iNpcNumber == GATENPC_NUMBER)
        memcpy(&m_GateInfo[pInfo->iNpcIndex - 1], pInfo, sizeof(PMSG_NPCDBLIST));
    if (pInfo->iNpcNumber == STATUENPC_NUMBER)
        memcpy(&m_StatueInfo[pInfo->iNpcIndex - 1], pInfo, sizeof(PMSG_NPCDBLIST));
}

LPPMSG_NPCDBLIST CSenatusInfo::GetNPCInfo(int iNpcNumber, int iNpcIndex)
{
    if (iNpcNumber == GATENPC_NUMBER)
        return &m_GateInfo[iNpcIndex - 1];
    else if (iNpcNumber == STATUENPC_NUMBER)
        return &m_StatueInfo[iNpcIndex - 1];
    else
        return NULL;
}

void CSenatusInfo::BuyNewNPC(int iNpcNumber, int iNpcIndex)
{
    LPPMSG_NPCDBLIST pNPCInfo = GetNPCInfo(iNpcNumber, iNpcIndex);
    memset(pNPCInfo, 0, sizeof(PMSG_NPCDBLIST));

    pNPCInfo->btNpcLive = 1;
    pNPCInfo->iNpcDfLevel = 0;
    if (iNpcNumber == GATENPC_NUMBER)	// gate
        pNPCInfo->iNpcHp = pNPCInfo->iNpcMaxHp = GATELEVEL_HP_0;
    else
        pNPCInfo->iNpcHp = pNPCInfo->iNpcMaxHp = STATUELEVEL_HP_0;
    pNPCInfo->iNpcNumber = iNpcNumber;
    pNPCInfo->iNpcIndex = iNpcIndex;
    pNPCInfo->iNpcDfLevel = 0;
    pNPCInfo->iNpcRgLevel = 0;
}

void CSenatusInfo::SetTaxInfo(LPPMSG_ANS_TAXMONEYINFO pInfo)
{
    m_iChaosTaxRate = m_iRealTaxRateChaos = pInfo->btTaxRateChaos;
    m_iNormalTaxRate = m_iRealTaxRateStore = pInfo->btTaxRateStore;
    BYTE* pMoney = (BYTE*)&m_i64CastleMoney;
    *pMoney++ = pInfo->btMoney8;
    *pMoney++ = pInfo->btMoney7;
    *pMoney++ = pInfo->btMoney6;
    *pMoney++ = pInfo->btMoney5;
    *pMoney++ = pInfo->btMoney4;
    *pMoney++ = pInfo->btMoney3;
    *pMoney++ = pInfo->btMoney2;
    *pMoney++ = pInfo->btMoney1;
}

void CSenatusInfo::ChangeTaxInfo(LPPMSG_ANS_TAXRATECHANGE pInfo)
{
    if (pInfo->btTaxType == 1)
    {
        m_iRealTaxRateChaos = (pInfo->btTaxRate1 << 24) | (pInfo->btTaxRate2 << 16) | (pInfo->btTaxRate3 << 8) | (pInfo->btTaxRate4);
        m_iChaosTaxRate = m_iRealTaxRateChaos;
    }
    else if (pInfo->btTaxType == 2)
    {
        m_iRealTaxRateStore = (pInfo->btTaxRate1 << 24) | (pInfo->btTaxRate2 << 16) | (pInfo->btTaxRate3 << 8) | (pInfo->btTaxRate4);
        m_iNormalTaxRate = m_iRealTaxRateStore;
    }
    else
        ;
}

void CSenatusInfo::ChangeCastleMoney(LPPMSG_ANS_MONEYDRAWOUT pInfo)
{
    BYTE* pMoney = (BYTE*)&m_i64CastleMoney;
    *pMoney++ = pInfo->btMoney8;
    *pMoney++ = pInfo->btMoney7;
    *pMoney++ = pInfo->btMoney6;
    *pMoney++ = pInfo->btMoney5;
    *pMoney++ = pInfo->btMoney4;
    *pMoney++ = pInfo->btMoney3;
    *pMoney++ = pInfo->btMoney2;
    *pMoney++ = pInfo->btMoney1;
}

void CSenatusInfo::PlusChaosTaxRate(int iValue)
{
    m_iChaosTaxRate += iValue;
    if (m_iChaosTaxRate > MAX_TAX_RATE) m_iChaosTaxRate = MAX_TAX_RATE;
    else if (m_iChaosTaxRate < MIN_TAX_RATE) m_iChaosTaxRate = MIN_TAX_RATE;
}

void CSenatusInfo::PlusNormalTaxRate(int iValue)
{
    m_iNormalTaxRate += iValue;
    if (m_iNormalTaxRate > MAX_NORMAL_TAX_RATE) m_iNormalTaxRate = MAX_NORMAL_TAX_RATE;
    else if (m_iNormalTaxRate < MIN_TAX_RATE) m_iNormalTaxRate = MIN_TAX_RATE;
}

void CSenatusInfo::RollbackTaxRates()
{
    m_iChaosTaxRate = m_iRealTaxRateChaos;
    m_iNormalTaxRate = m_iRealTaxRateStore;
}