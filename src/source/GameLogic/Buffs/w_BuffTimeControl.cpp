
#include "stdafx.h"
#include "Engine/Object/ZzzInfomation.h"
#include "UI/Core/UIManager.h"
#include "GameLogic/Items/ItemAddOptioninfo.h"
#include "w_BuffTimeControl.h"
#include "Core/Time/FrameTimerScheduler.h"
#include "I18N/All.h"

BuffTimeControlPtr BuffTimeControl::Make()
{
    BuffTimeControlPtr bufftimecontrol(new BuffTimeControl);
    return bufftimecontrol;
}

BuffTimeControl::BuffTimeControl()
{
}

BuffTimeControl::~BuffTimeControl()
{
    for (auto iter = m_BuffTimeList.begin(); iter != m_BuffTimeList.end(); )
    {
        auto tempiter = iter;
        ++iter;

        auto bufftimetype = static_cast<eBuffTimeType>((*tempiter).first);

        Core::Time::FrameTimerScheduler::Instance().Kill(bufftimetype);
        m_BuffTimeList.erase(tempiter);
    }

    m_BuffTimeList.clear();
    m_EstimatedBuffTimeList.clear();
}

eBuffTimeType BuffTimeControl::CheckBuffTimeType(eBuffState bufftype)
{
    if (g_IsBuffClass(bufftype) == eBuffClass_Count)
    {
        return eBuffTime_None;
    }

    BuffInfo bInfo = g_BuffInfo(bufftype);

    return eBuffTimeType(1005 + bInfo.s_BuffEffectType);
}

DWORD BuffTimeControl::GetBuffEventTime(eBuffTimeType bufftimetype)
{
    return 1000;
}

DWORD BuffTimeControl::GetBuffMaxTime(eBuffState bufftype, DWORD curbufftime)
{
    if (curbufftime == 0)
    {
        const BuffInfo& buffinfo = g_BuffInfo(bufftype);

        if (buffinfo.s_ItemType == 255)
        {
            return -1;
        }
        else
        {
            const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ITEMINDEX(buffinfo.s_ItemType, buffinfo.s_ItemIndex));

            if (Item_data.m_Time == 0) return -1;

            return Item_data.m_Time;
        }
    }

    return curbufftime;
}

bool BuffTimeControl::IsBuffTime(eBuffTimeType bufftype)
{
    auto iter = m_BuffTimeList.find(bufftype);

    if (iter == m_BuffTimeList.end())
    {
        return false;
    }
    else
    {
        return true;
    }
}

void BuffTimeControl::RegisterBuffTime(eBuffState bufftype, DWORD curbufftime)
{
    eBuffTimeType  bufftimetype = CheckBuffTimeType(bufftype);

    curbufftime = GetBuffMaxTime(bufftype, curbufftime);

    if (bufftimetype == eBuffTime_None || curbufftime == -1)
    {
        return;
    }

    BuffTimeInfo  buffinfo;
    buffinfo.s_BuffType = bufftype;
    buffinfo.s_CurBuffTime = curbufftime * 1000;
    buffinfo.s_MaxBuffTime = buffinfo.s_CurBuffTime;
    buffinfo.s_EventBuffTime = GetTickCount();

    // A re-application refreshes the effect on the server, so it restarts the countdown here too.
    // The repeating timer below is reused, or re-armed after an earlier expiry.
    m_BuffTimeList[bufftimetype] = buffinfo;

    // Tick the buff's remaining time every 900ms; the timer removes itself once
    // the buff expires (replaces the per-buff SetTimer/WM_TIMER).
    Core::Time::FrameTimerScheduler::Instance().SetRepeating(bufftimetype, 900,
        [this, bufftimetype]
        {
            if (!CheckBuffTime(bufftimetype))
                Core::Time::FrameTimerScheduler::Instance().Kill(bufftimetype);
        });
}

bool BuffTimeControl::UnRegisterBuffTime(eBuffState bufftype)
{
    eBuffTimeType  bufftimetype = CheckBuffTimeType(bufftype);

    auto iter = m_BuffTimeList.find(bufftimetype);

    if (iter != m_BuffTimeList.end())
    {
        Core::Time::FrameTimerScheduler::Instance().Kill(bufftimetype);
        g_ConsoleDebug->Write(MCD_NORMAL, L"[Buff End] No. %d\r\n", bufftimetype);

        m_BuffTimeList.erase(iter);
        return true;
    }

    return false;
}

void BuffTimeControl::GetBuffStringTime(eBuffState bufftype, std::wstring& timeText)
{
    for (BuffTimeInfoMap::iterator iter = m_BuffTimeList.begin(); iter != m_BuffTimeList.end(); ++iter)
    {
        BuffTimeInfo& bufftimeinfo = (*iter).second;

        if (bufftimeinfo.s_BuffType == bufftype)
        {
            float fTime = bufftimeinfo.s_CurBuffTime * 0.001f;
            GetStringTime(fTime, timeText, true);
        }
    }
}

void BuffTimeControl::GetBuffStringTime(DWORD type, std::wstring& timeText, bool issecond)
{
    auto iter = m_BuffTimeList.find(type);

    if (iter != m_BuffTimeList.end())
    {
        BuffTimeInfo& bufftimeinfo = (*iter).second;
        float fTime = bufftimeinfo.s_CurBuffTime * 0.001f;
        GetStringTime(fTime, timeText, issecond);
    }
}

const DWORD BuffTimeControl::GetBuffTime(DWORD type)
{
    auto iter = m_BuffTimeList.find(type);

    if (iter != m_BuffTimeList.end())
    {
        BuffTimeInfo& bufftimeinfo = (*iter).second;

        return bufftimeinfo.s_CurBuffTime;
    }
    return 0;
}

bool BuffTimeControl::GetBuffRemainingFraction(eBuffState bufftype, float& fraction)
{
    for (BuffTimeInfoMap::iterator iter = m_BuffTimeList.begin(); iter != m_BuffTimeList.end(); ++iter)
    {
        BuffTimeInfo& bufftimeinfo = (*iter).second;

        if (bufftimeinfo.s_BuffType != bufftype)
            continue;

        if (bufftimeinfo.s_MaxBuffTime == 0)
            return false;

        // s_CurBuffTime only moves on the 900ms scheduler tick; subtract the live delta from the
        // tick's anchor so the bar shrinks smoothly between ticks instead of in 0.9s steps.
        DWORD elapsed = GetTickCount() - bufftimeinfo.s_EventBuffTime;
        DWORD remaining = elapsed < bufftimeinfo.s_CurBuffTime ? bufftimeinfo.s_CurBuffTime - elapsed : 0;

        fraction = remaining / static_cast<float>(bufftimeinfo.s_MaxBuffTime);
        return true;
    }

    return false;
}

void BuffTimeControl::RegisterEstimatedBuffTime(eBuffState bufftype, DWORD seconds)
{
    if (seconds == 0)
        return;

    BuffTimeInfo bufftimeinfo;
    bufftimeinfo.s_BuffType = bufftype;
    bufftimeinfo.s_CurBuffTime = seconds * 1000;
    bufftimeinfo.s_MaxBuffTime = bufftimeinfo.s_CurBuffTime;
    bufftimeinfo.s_EventBuffTime = GetTickCount();

    // A re-application refreshes the effect on the server, so it restarts the countdown here too.
    m_EstimatedBuffTimeList[bufftype] = bufftimeinfo;
}

bool BuffTimeControl::UnRegisterEstimatedBuffTime(eBuffState bufftype)
{
    return m_EstimatedBuffTimeList.erase(bufftype) != 0;
}

bool BuffTimeControl::GetEstimatedBuffRemainingFraction(eBuffState bufftype, float& fraction)
{
    auto iter = m_EstimatedBuffTimeList.find(bufftype);

    if (iter == m_EstimatedBuffTimeList.end() || (*iter).second.s_MaxBuffTime == 0)
        return false;

    BuffTimeInfo& bufftimeinfo = (*iter).second;
    DWORD elapsed = GetTickCount() - bufftimeinfo.s_EventBuffTime;
    DWORD remaining = elapsed < bufftimeinfo.s_CurBuffTime ? bufftimeinfo.s_CurBuffTime - elapsed : 0;

    fraction = remaining / static_cast<float>(bufftimeinfo.s_MaxBuffTime);
    return true;
}

DWORD BuffTimeControl::GetEstimatedBuffRemainingMilliseconds(eBuffState bufftype)
{
    auto iter = m_EstimatedBuffTimeList.find(bufftype);

    if (iter == m_EstimatedBuffTimeList.end())
        return 0;

    BuffTimeInfo& bufftimeinfo = (*iter).second;
    DWORD elapsed = GetTickCount() - bufftimeinfo.s_EventBuffTime;

    return elapsed < bufftimeinfo.s_CurBuffTime ? bufftimeinfo.s_CurBuffTime - elapsed : 0;
}

namespace
{
DWORD ScaledSeconds(int base, int divisor, int cap, int energy)
{
    int seconds = base + energy / divisor;

    if (cap > 0)
        seconds = std::min(seconds, cap);

    return static_cast<DWORD>(std::max(seconds, 1));
}
} // namespace

DWORD BuffDurationSeconds(eBuffState bufftype)
{
    // TotalEnergy is the base plus the added stat, as the server's attribute system sees it.
    const int energy = CharacterAttribute != nullptr ? CharacterAttribute->Energy + CharacterAttribute->AddEnergy : 0;

    switch (bufftype)
    {
    // Skill effects: base + TotalEnergy / divisor, capped. The server sends only the effect id for
    // these, so the duration has to come from its catalogue. Master skills that extend a duration
    // are not modelled: the client carries no ExtendsDuration value.
    case eBuff_Attack: return 60;
    case eBuff_Defense: return 60;
    case eBuff_HelpNpc: return 3600;
    case eBuff_WizDefense: return ScaledSeconds(60, 40, 0, energy);
    case eBuff_AddCriticalDamage: return ScaledSeconds(60, 10, 180, energy);
    case eBuff_InfinityArrow: return 600;
    case eBuff_Life: return ScaledSeconds(60, 180, 180, energy);
    case eBuff_BlessPotion: return 120;
    case eBuff_SoulPotion: return 60;
    case eBuff_Cloaking: return 86400;

    case eDeBuff_Poison: return 20;
    case eDeBuff_Freeze: return 5;
    case eDeBuff_Harden: return 5;
    case eDeBuff_Defense: return 10;
    case eDeBuff_Stun: return 2;

    case eBuff_Thorns: return ScaledSeconds(30, 24, 180, energy);
    // The hero is a player, so the PvP variants apply; the server's +/- Level terms cancel there.
    case eDeBuff_Sleep: return ScaledSeconds(4, 250, 10, energy);
    case eDeBuff_NeilDOT: return 5;
    case eDeBuff_SahamuttDOT: return 5;
    case eDeBuff_AttackDown: return 10;
    case eDeBuff_DefenseDown: return ScaledSeconds(5, 300, 18, energy);
    case eBuff_Berserker: return ScaledSeconds(30, 20, 180, energy);
    case eBuff_SwellOfMagicPower: return 1800;
    case eDeBuff_BlowOfDestruction: return 10;
    case eBuff_Att_up_Ourforces: return ScaledSeconds(60, 5, 180, energy);
    case eBuff_Hp_up_Ourforces: return ScaledSeconds(60, 5, 180, energy);
    case eBuff_Def_up_Ourforces: return ScaledSeconds(60, 5, 180, energy);

    case EFFECT_GREATER_LIFE_ENHANCED:
    case EFFECT_GREATER_LIFE_MASTERED:
        return ScaledSeconds(60, 180, 180, energy);
    case EFFECT_MAGIC_CIRCLE_IMPROVED:
    case EFFECT_MAGIC_CIRCLE_ENHANCED:
        return 1800;
    case EFFECT_GREATER_CRITICAL_DAMAGE_MASTERED:
    case EFFECT_GREATER_CRITICAL_DAMAGE_EXTENDED:
        return ScaledSeconds(60, 10, 180, energy);
    case EFFECT_GREATER_DEFENSE_SUCCESS_RATE_IMPROVED:
    case EFFECT_GREATER_DEFENSE_SUCCESS_RATE_ENHANCED:
        return ScaledSeconds(60, 5, 180, energy);
    case EFFECT_FITNESS_IMPROVED:
        return ScaledSeconds(60, 5, 180, energy);

    // Item effects the server may send with 0 seconds (no mapped attribute) or not at all.
    case eBuff_PcRoomSeal1:
    case eBuff_PcRoomSeal2:
    case eBuff_PcRoomSeal3:
    case eBuff_Seal1:
    case eBuff_Seal2:
    case eBuff_Seal3:
    case eBuff_Seal4:
    case eBuff_EliteScroll1:
    case eBuff_EliteScroll2:
    case eBuff_EliteScroll3:
    case eBuff_EliteScroll4:
    case eBuff_EliteScroll5:
    case eBuff_EliteScroll6:
    case eBuff_CherryBlossom_Liguor:
    case eBuff_CherryBlossom_RiceCake:
    case eBuff_CherryBlossom_Petal:
    case eBuff_Seal_HpRecovery:
    case eBuff_Seal_MpRecovery:
    case eBuff_Scroll_Battle:
    case eBuff_Scroll_Strengthen:
    case eBuff_Scroll_Healing:
    case eBuff_AscensionSealMaster:
    case eBuff_WealthSealMaster:
    case eBuff_AG_Addition:
    case eBuff_SD_Addition:
        return 1800;

    case eBuff_SecretPotion1:
    case eBuff_SecretPotion2:
    case eBuff_SecretPotion3:
    case eBuff_SecretPotion4:
    case eBuff_SecretPotion5:
        return 10 * 24 * 60 * 60;

    default:
        return 0;
    }
}

void BuffTimeControl::GetStringTime(DWORD time, std::wstring& timeText, bool isSecond)
{
    wchar_t buffer[100];

    if (isSecond)
    {
        DWORD day = time / (1440 * 60);
        DWORD oClock = (time - (day * (1440 * 60))) / 3600;
        DWORD minutes = (time - ((oClock * 3600) + (day * (1440 * 60)))) / 60;
        DWORD second = time % 60;

        if (day != 0)
        {
            mu_swprintf(buffer, L"%d %ls %d %ls %d %ls %d %ls", day, I18N::Game::Day, oClock, I18N::Game::Hour, minutes, I18N::Game::Minute, second, I18N::Game::Second);
            timeText = buffer;
        }
        else if (day == 0 && oClock != 0)
        {
            mu_swprintf(buffer, L"%d %ls %d %ls %d %ls", oClock, I18N::Game::Hour, minutes, I18N::Game::Minute, second, I18N::Game::Second);
            timeText = buffer;
        }
        else if (day == 0 && oClock == 0 && minutes != 0)
        {
            mu_swprintf(buffer, L"%d %ls %d %ls", minutes, I18N::Game::Minute, second, I18N::Game::Second);
            timeText = buffer;
        }
        else if (day == 0 && oClock == 0 && minutes == 0)
        {
            mu_swprintf(buffer, L"%ls", I18N::Game::LessThan1Minutes);
            timeText = buffer;
        }
    }
    else
    {
        DWORD day = time / 1440;
        DWORD oClock = (time - (day * 1440)) / 60;
        DWORD minutes = time % 60;

        if (day != 0)
        {
            mu_swprintf(buffer, L"%d %ls %d %ls %d %ls", day, I18N::Game::Day, oClock, I18N::Game::Hour, minutes, I18N::Game::Minute);
            timeText = buffer;
        }
        else if (day == 0 && oClock != 0)
        {
            mu_swprintf(buffer, L"%d %ls %d %ls", oClock, I18N::Game::Hour, minutes, I18N::Game::Minute);
            timeText = buffer;
        }
        else if (day == 0 && oClock == 0 && minutes != 0)
        {
            mu_swprintf(buffer, L"%d %ls", minutes, I18N::Game::Minute);
            timeText = buffer;
        }
    }
}

bool BuffTimeControl::CheckBuffTime(DWORD type)
{
    auto iter = m_BuffTimeList.find(type);

    if (iter != m_BuffTimeList.end())
    {
        BuffTimeInfo& bufftimeinfo = (*iter).second;

        DWORD iCurBufftime = bufftimeinfo.s_CurBuffTime;
        if (iCurBufftime > GetTickCount() - bufftimeinfo.s_EventBuffTime)
        {
            iCurBufftime -= GetTickCount() - bufftimeinfo.s_EventBuffTime;
        }
        else
        {
            iCurBufftime = 0;
        }
        bufftimeinfo.s_EventBuffTime = GetTickCount();

        if (iCurBufftime <= 0)
        {
            bufftimeinfo.s_CurBuffTime = 0;
            return false;
        }
        else
        {
            bufftimeinfo.s_CurBuffTime = iCurBufftime;
            return true;
        }
    }
    return false;
}

