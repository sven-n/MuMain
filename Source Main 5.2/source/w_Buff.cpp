//////////////////////////////////////////////////////////////////////
// w_Buff.cpp: implementation of the Buff class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_Buff.h"

namespace
{
    void GetTokenBufflist(std::list<eBuffState>& outtokenbufflist, const eBuffState curbufftype)
    {
        if (curbufftype >= eBuff_CastleRegimentDefense && curbufftype <= eBuff_CastleRegimentAttack3)
        {
            outtokenbufflist.push_back(eBuff_CastleRegimentDefense); outtokenbufflist.push_back(eBuff_CastleRegimentAttack1);
            outtokenbufflist.push_back(eBuff_CastleRegimentAttack2); outtokenbufflist.push_back(eBuff_CastleRegimentAttack3);
        }
        if (curbufftype >= eBuff_CrywolfAltarEnable && curbufftype <= eBuff_CrywolfNPCHide)
        {
            outtokenbufflist.push_back(eBuff_CrywolfAltarEnable); outtokenbufflist.push_back(eBuff_CrywolfAltarDisable);
            outtokenbufflist.push_back(eBuff_CrywolfAltarContracted); outtokenbufflist.push_back(eBuff_CrywolfAltarAttempt);
            outtokenbufflist.push_back(eBuff_CrywolfAltarOccufied); outtokenbufflist.push_back(eBuff_CrywolfHeroContracted);
            outtokenbufflist.push_back(eBuff_CrywolfNPCHide);
        }
        if ((curbufftype >= eBuff_PcRoomSeal1 && curbufftype <= eBuff_PcRoomSeal3) || curbufftype == eBuff_NewWealthSeal)
        {
            outtokenbufflist.push_back(eBuff_NewWealthSeal);
            outtokenbufflist.push_back(eBuff_PcRoomSeal1); outtokenbufflist.push_back(eBuff_PcRoomSeal2);
            outtokenbufflist.push_back(eBuff_PcRoomSeal3);
        }
        // eBuff_Seal_HpRecovery, eBuff_Seal_MpRecovery
        if ((curbufftype >= eBuff_Seal1 && curbufftype <= eBuff_Seal4)
            || curbufftype == eBuff_AscensionSealMaster || curbufftype == eBuff_WealthSealMaster)
        {
            outtokenbufflist.push_back(eBuff_Seal1);
            outtokenbufflist.push_back(eBuff_Seal2);
            outtokenbufflist.push_back(eBuff_Seal3);
            outtokenbufflist.push_back(eBuff_Seal4);
            outtokenbufflist.push_back(eBuff_Seal_HpRecovery);
            outtokenbufflist.push_back(eBuff_Seal_MpRecovery);
            outtokenbufflist.push_back(eBuff_AscensionSealMaster);
            outtokenbufflist.push_back(eBuff_WealthSealMaster);
        }

        if (curbufftype >= eBuff_EliteScroll1 && curbufftype <= eBuff_EliteScroll6)
        {
            outtokenbufflist.push_back(eBuff_EliteScroll1); outtokenbufflist.push_back(eBuff_EliteScroll2);
            outtokenbufflist.push_back(eBuff_EliteScroll3); outtokenbufflist.push_back(eBuff_EliteScroll4);
            outtokenbufflist.push_back(eBuff_EliteScroll5); outtokenbufflist.push_back(eBuff_EliteScroll6);
            outtokenbufflist.push_back(eBuff_Scroll_Battle);
            outtokenbufflist.push_back(eBuff_Scroll_Strengthen);
        }
        if (curbufftype >= eBuff_SecretPotion1 && curbufftype <= eBuff_SecretPotion5)
        {
            outtokenbufflist.push_back(eBuff_SecretPotion1); outtokenbufflist.push_back(eBuff_SecretPotion2);
            outtokenbufflist.push_back(eBuff_SecretPotion3); outtokenbufflist.push_back(eBuff_SecretPotion4);
            outtokenbufflist.push_back(eBuff_SecretPotion5);
        }
    }
}

BuffPtr Buff::Make()
{
    BuffPtr buff(new Buff());
    return buff;
}

Buff::Buff()
{
}

Buff::~Buff()
{
    ClearBuff();
}

bool Buff::isBuff()
{
    if (m_Buff.size() != 0) return true;
    return false;
}

bool Buff::isBuff(eBuffState buffstate)
{
    if (!isBuff()) return false;

    auto iter = m_Buff.find(buffstate);

    if (iter != m_Buff.end())
    {
        return true;
    }

    return false;
}

const eBuffState Buff::isBuff(std::list<eBuffState> buffstatelist)
{
    if (!isBuff()) return eBuffNone;

    for (auto iter = buffstatelist.begin();
         iter != buffstatelist.end(); )
    {
        auto Tempiter = iter;
        ++iter;
        eBuffState tempbufftype = (*Tempiter);

        if (isBuff(tempbufftype)) return tempbufftype;
    }

    return eBuffNone;
}

void Buff::TokenBuff(eBuffState curbufftype)
{
    std::list<eBuffState> tokenbufflist;
    GetTokenBufflist(tokenbufflist, curbufftype);
    UnRegisterBuff(tokenbufflist);
    RegisterBuff(curbufftype);
}

const DWORD Buff::GetBuffCount(eBuffState buffstate)
{
    DWORD tempcount = 0;

    if (!isBuff()) return tempcount;

    auto iter = m_Buff.find(buffstate);

    if (iter != m_Buff.end())
    {
        tempcount = (*iter).second;
        return tempcount;
    }

    return tempcount;
}

const DWORD Buff::GetBuffSize()
{
    return m_Buff.size();
}

const eBuffState Buff::GetBuff(int iterindex)
{
    if (iterindex >= (int)GetBuffSize()) return eBuffNone;

    int i = 0;

    for (auto iter = m_Buff.begin(); iter != m_Buff.end(); )
    {
        auto tempiter = iter;
        ++iter;

        if (i == iterindex)
        {
            return (*tempiter).first;
        }

        i += 1;
    }

    return eBuffNone;
}

bool Buff::IsEqualBuffType(IN int iBuffType, OUT wchar_t* szBuffName)
{
    auto iter = m_Buff.begin();
    BuffInfo buffinfo;

    while (iter != m_Buff.end())
    {
        buffinfo = TheBuffInfo().GetBuffinfo(iter->first);
        if (buffinfo.s_BuffEffectType == iBuffType)
        {
            wcscpy(szBuffName, buffinfo.s_BuffName);
            return true;
        }

        iter++;
    }

    return false;
}

void Buff::RegisterBuff(eBuffState buffstate)
{
    auto iter = m_Buff.find(buffstate);

    if (iter == m_Buff.end())
    {
        m_Buff.insert(std::make_pair(buffstate, 1));
    }
}

void Buff::RegisterBuff(std::list<eBuffState> buffstate)
{
    for (auto iter = buffstate.begin(); iter != buffstate.end(); )
    {
        auto tempiter = iter;
        ++iter;
        eBuffState& tempdata = (*tempiter);

        RegisterBuff(tempdata);
    }
}

void Buff::UnRegisterBuff(eBuffState buffstate)
{
    if (!isBuff())
    {
        return;
    }

    auto iter = m_Buff.find(buffstate);

    if (iter != m_Buff.end())
    {
        DWORD& tempcount = (*iter).second;
        {
            m_Buff.erase(iter);
        }
    }
    else
    {
        return;
    }
}

void Buff::UnRegisterBuff(std::list<eBuffState> buffstate)
{
    for (auto iter = buffstate.begin(); iter != buffstate.end(); )
    {
        auto tempiter = iter;
        ++iter;
        eBuffState& tempdata = (*tempiter);

        UnRegisterBuff(tempdata);
    }
}

void Buff::ClearBuff()
{
    m_Buff.clear();
}