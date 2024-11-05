#include "stdafx.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <cmath>

#include "ZzzAI.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "NewUISystem.h"
#include "Utilities/Log/muConsoleDebug.h"
#include "SkillManager.h"
#include "PartyManager.h"
#include "WSclient.h"

#include "MuHelper.h"

#define MAX_ACTIONABLE_DISTANCE 10

CMuHelper g_MuHelper;
std::mutex _mtx_targetSet;

CMuHelper::CMuHelper()
{
    m_iCurrentTarget = -1;
    m_iComboState = 0;
    m_iCurrentBuffIndex = 0;
    m_iCurrentPartyMemberIndex = 0;
    m_iHuntingDistance = MAX_ACTIONABLE_DISTANCE;
    m_iSecondsElapsed = 0;
    m_iSecondsAway = 0;
    m_bSavedAutoAttack = false;
}

void CMuHelper::Save(const cMuHelperConfig& config)
{
    // Save config data by sending to server
    m_config = config;
}

void CMuHelper::Load(const cMuHelperConfig& config)
{
    // Load config data received from the server
    m_config = config;
}

void CMuHelper::Start()
{
    if (m_bActive)
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"MU Helper is already active!");
        return;
    }

    m_bActive = true;
    m_iComboState = 0;
    m_iCurrentBuffIndex = 0;
    m_iCurrentPartyMemberIndex = 0;
    m_iCurrentTarget = -1;
    m_posOriginal = { Hero->PositionX, Hero->PositionY };
    m_iHuntingDistance = ComputeHuntingDistance();

    m_iSecondsElapsed = 0;
    m_iSecondsAway = 0;
    m_setTargets.clear();

    // TO-DO: Use value from UI
    m_config.iMaxSecondsAway = 5;
    m_config.iPotionThreshold = 50;

    m_timerThread = std::thread(&CMuHelper::WorkLoop, this);

    m_bSavedAutoAttack = g_pOption->IsAutoAttack();
    g_pOption->SetAutoAttack(false);

    g_ConsoleDebug->Write(MCD_NORMAL, L"MU Helper started");
}

void CMuHelper::Stop()
{
    m_bActive = false;
    if (m_timerThread.joinable()) 
    {
        m_timerThread.join();
    }

    g_pOption->SetAutoAttack(m_bSavedAutoAttack);

    g_ConsoleDebug->Write(MCD_NORMAL, L"MU Helper stopped");
}

void CMuHelper::WorkLoop()
{
    int iLoopCounter = 0;

    m_iSecondsElapsed = 0;
    m_iSecondsAway = 0;

    while (m_bActive) {
        Work();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (++iLoopCounter == 5) {
            ++m_iSecondsElapsed;

            if (ComputeDistanceBetween({ Hero->PositionX, Hero->PositionY }, m_posOriginal) > 1)
            {
                ++m_iSecondsAway;
            }
            else
            {
                m_iSecondsAway = 0;
            }

            iLoopCounter = 0;
        }
    }
}

void CMuHelper::Work()
{
    try
    {
        if (!Buff())
        {
            return;
        }

        if (!Heal())
        {
            return;
        }

        if (!ConsumePotion())
        {
            return;
        }

        if (!ObtainItem())
        {
            return;
        }

        if (!Regroup())
        {
            return;
        }

        Attack();

        RepairEquipments();
    }
    catch (...)
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Exception occurred. Ignoring...");
    }
}

void CMuHelper::AddTarget(int iTargetId, bool bIsAttacking)
{
    std::lock_guard<std::mutex> lock(_mtx_targetSet);

    if (!m_bActive)
    {
        return;
    }

    CHARACTER* pTarget = FindCharacterByKey(iTargetId);
    if (!pTarget || pTarget == Hero)
    {
        return;
    }

    int iDistance = ComputeDistanceFromTarget(pTarget);

    if ((iDistance <= m_iHuntingDistance)
        || (bIsAttacking && m_config.bLongDistanceCounterAttack))
    {
        m_setTargets.insert(iTargetId);
    }

    if (m_config.bUseSelfDefense)
    {
        pTarget->Object.Kind = KIND_MONSTER;
        m_iCurrentTarget = iTargetId;
    }
}

void CMuHelper::DeleteTarget(int iTargetId)
{
    std::lock_guard<std::mutex> lock(_mtx_targetSet);

    m_setTargets.erase(iTargetId);

    if (iTargetId == m_iCurrentTarget)
    {
        m_iCurrentTarget = -1;
    }
}

void CMuHelper::DeleteAllTargets()
{
    std::lock_guard<std::mutex> lock(_mtx_targetSet);
    m_setTargets.clear();
}

int CMuHelper::ComputeHuntingDistance()
{
    POINT posA = { 0, 0 };
    POINT posB = { m_config.iHuntingRange, m_config.iHuntingRange * (-1) };
    return ComputeDistanceBetween(posA, posB);
}

int CMuHelper::ComputeDistanceFromTarget(CHARACTER* pTarget)
{
    POINT posA, posB;

    if (m_config.bReturnToOriginalPosition)
    {
        posA = { m_posOriginal.x, m_posOriginal.y };
        posB = { pTarget->PositionX, pTarget->PositionY };
        int iPrevDistance = ComputeDistanceBetween(posA, posB);

        posA = { m_posOriginal.x, m_posOriginal.y };
        posB = { pTarget->TargetX, pTarget->TargetX };
        int iNextDistance = ComputeDistanceBetween(posA, posB);

        return min(iPrevDistance, iNextDistance);
    }
    else
    {
        posA = { Hero->PositionX, Hero->PositionY };
        posB = { pTarget->PositionX, pTarget->PositionY };
        int iPrevDistance = ComputeDistanceBetween(posA, posB);

        posA = { Hero->PositionX, Hero->PositionY };
        posB = { pTarget->TargetX, pTarget->TargetX };
        int iNextDistance = ComputeDistanceBetween(posA, posB);

        return min(iPrevDistance, iNextDistance);
    }
}

int CMuHelper::ComputeDistanceBetween(POINT posA, POINT posB)
{
    int iDx = posA.x - posB.x;
    int iDy = posA.y - posB.y;

    return static_cast<int>(std::ceil(std::sqrt(iDx * iDx + iDy * iDy)));
}

int CMuHelper::GetNearestTarget()
{
    int iClosestMonsterId = -1;
    int iMinDistance = m_config.iHuntingRange + 1;

    std::set<int> setTargets;
    {
        std::lock_guard<std::mutex> lock(_mtx_targetSet);
        setTargets = m_setTargets;
    }

    for (const int& iMonsterId : setTargets)
    {
        int iIndex = FindCharacterIndex(iMonsterId);
        if (iIndex == MAX_CHARACTERS_CLIENT)
        {
            DeleteTarget(iMonsterId);
            continue;
        }

        CHARACTER* pTarget = &CharactersClient[iIndex];
        if (pTarget->Dead)
        {
            DeleteTarget(iMonsterId);
            continue;
        }

        int iDistance = ComputeDistanceFromTarget(pTarget);
        if (iDistance < iMinDistance)
        {
            iMinDistance = iDistance;
            iClosestMonsterId = iMonsterId;
        }
    }

    return iClosestMonsterId;
}

int CMuHelper::GetFarthestTarget()
{
    int iFarthestMonsterId = -1;
    int iMaxDistance = -1;

    std::set<int> setTargets;
    {
        std::lock_guard<std::mutex> lock(_mtx_targetSet);
        setTargets = m_setTargets;
    }

    for (const int& iMonsterId : setTargets)
    {
        int iIndex = FindCharacterIndex(iMonsterId);
        if (iIndex == MAX_CHARACTERS_CLIENT)
        {
            DeleteTarget(iMonsterId);
            continue;
        }

        CHARACTER* pTarget = &CharactersClient[iIndex];
        if (pTarget->Dead)
        {
            DeleteTarget(iMonsterId);
            continue;
        }

        int iDistance = ComputeDistanceFromTarget(pTarget);
        if (iDistance > iMaxDistance)
        {
            iMaxDistance = iDistance;
            iFarthestMonsterId = iMonsterId;
        }
    }

    return iFarthestMonsterId;
}

int CMuHelper::Buff()
{
    bool bNoBuff = true;

    for (int i = 0; i < m_config.aiBuff.size(); i++)
    {
        if (m_config.aiBuff[i] != 0)
        {
            bNoBuff = false;
            break;
        }
    }

    if (bNoBuff)
    {
        return 1;
    }

    if (m_config.bSupportParty)
    {
        int iMemberCount = 0;
        int iHeroMap;

        for (int i = 0; i < ((sizeof(Party) / sizeof(Party[0])) - 1); i++)
        {
            PARTY_t* pMember = &Party[i];
            CHARACTER* pChar = FindCharacterByID(pMember->Name);
            if (pChar != NULL)
            {
                if (pChar == Hero)
                {
                    iHeroMap = pMember->Map;
                }
                iMemberCount++;
            }
        }

        bool bPartyActive = iMemberCount > 1;
        
        if (bPartyActive)
        {
            PARTY_t* pMember = &Party[m_iCurrentPartyMemberIndex];
            CHARACTER* pChar = FindCharacterByID(pMember->Name);

            if (pChar != NULL
                && pMember->Map == iHeroMap
                && ComputeDistanceFromTarget(pChar) <= MAX_ACTIONABLE_DISTANCE
                )
            {
                if (!BuffTarget(pChar, m_config.aiBuff[m_iCurrentBuffIndex]))
                {
                    return 0;
                }
            }

            m_iCurrentPartyMemberIndex = (m_iCurrentPartyMemberIndex + 1) % (sizeof(Party) / sizeof(Party[0]));
        }
        else
        {
            if (!BuffTarget(Hero, m_config.aiBuff[m_iCurrentBuffIndex]))
            {
                return 0;
            }
        }
    }
    else
    {
        if (!BuffTarget(Hero, m_config.aiBuff[m_iCurrentBuffIndex]))
        {
            return 0;
        }
    }

    if (m_iCurrentPartyMemberIndex == 0)
    {
        m_iCurrentBuffIndex = (m_iCurrentBuffIndex + 1) % m_config.aiBuff.size();
    }

    return 1;
}

int CMuHelper::BuffTarget(CHARACTER* pTargetChar, int iBuffSkill)
{
    // TODO: List other buffs here

    if ((iBuffSkill == AT_SKILL_ATTACK
        || iBuffSkill == AT_SKILL_ATT_POWER_UP
        || iBuffSkill == AT_SKILL_ATT_POWER_UP + 1
        || iBuffSkill == AT_SKILL_ATT_POWER_UP + 2
        || iBuffSkill == AT_SKILL_ATT_POWER_UP + 3
        || iBuffSkill == AT_SKILL_ATT_POWER_UP + 4)
        && !g_isCharacterBuff((&pTargetChar->Object), eBuff_Attack))
    {
        return SimulateSkill(iBuffSkill, true, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_DEFENSE
        || iBuffSkill == AT_SKILL_DEF_POWER_UP
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 1
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 2
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 3
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 4)
        && !g_isCharacterBuff((&pTargetChar->Object), eBuff_Defense))
    {
        return SimulateSkill(iBuffSkill, true, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_WIZARDDEFENSE
        || iBuffSkill == AT_SKILL_SOUL_UP
        || iBuffSkill == AT_SKILL_SOUL_UP + 1
        || iBuffSkill == AT_SKILL_SOUL_UP + 2
        || iBuffSkill == AT_SKILL_SOUL_UP + 3
        || iBuffSkill == AT_SKILL_SOUL_UP + 4)
        && !g_isCharacterBuff((&pTargetChar->Object), eBuff_WizDefense))
    {
        return SimulateSkill(iBuffSkill, true, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_VITALITY
        || iBuffSkill == AT_SKILL_LIFE_UP
        || iBuffSkill == AT_SKILL_LIFE_UP + 1
        || iBuffSkill == AT_SKILL_LIFE_UP + 2
        || iBuffSkill == AT_SKILL_LIFE_UP + 3
        || iBuffSkill == AT_SKILL_LIFE_UP + 4)
        && !g_isCharacterBuff((&pTargetChar->Object), eBuff_Life))
    {
        return SimulateSkill(iBuffSkill, false, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_SWELL_OF_MAGICPOWER)
        && !g_isCharacterBuff((&pTargetChar->Object), eBuff_SwellOfMagicPower))
    {
        return SimulateSkill(iBuffSkill, false, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_ADD_CRITICAL)
        && !g_isCharacterBuff((&pTargetChar->Object), eBuff_AddCriticalDamage))
    {
        return SimulateSkill(iBuffSkill, false, pTargetChar->Key);
    }

    return 1;
}

int CMuHelper::ConsumePotion()
{
    int64_t iLife = CharacterAttribute->Life;
    int64_t iLifeMax = CharacterAttribute->LifeMax;

    if (m_config.bUseHealPotion && iLifeMax > 0 && iLife > 0)
    {
        int64_t iRemaining = (iLife * 100 + iLifeMax - 1) / iLifeMax;
        if (iRemaining <= m_config.iPotionThreshold)
        {
            int iPotionIndex = g_pMyInventory->FindHealingItemIndex();
            if (iPotionIndex != -1)
            {
                SendRequestUse(iPotionIndex, 0);
            }
        }
    }

    return 1;
}

int CMuHelper::Heal()
{
    return 1;
}

int CMuHelper::HealTarget(CHARACTER* pTargetChar, int iHealSkill)
{
    return 1;
}

int CMuHelper::ObtainItem()
{
    return 1;
}

int CMuHelper::RepairEquipments()
{
    return 1;
}

int CMuHelper::Attack()
{
    if (m_iCurrentTarget == -1)
    {
        if (!m_setTargets.empty())
        {
            if (m_config.bLongDistanceCounterAttack)
            {
                m_iCurrentTarget = GetFarthestTarget();
            }
            else
            {
                m_iCurrentTarget = GetNearestTarget();
            }

            if (m_config.bUseCombo && m_iComboState < 2)
            {
                m_iComboState = 0;
            }
        }

        m_iComboState = 0;
        return 0;
    }

    if (m_config.bUseCombo)
    {
        return SimulateComboAttack();
    }

    // to-do select skill
    return SimulateAttack(m_config.aiSkill[0]);
}

int CMuHelper::SimulateComboAttack()
{
    for (int i = 0; i < m_config.aiSkill.size(); i++)
    {
        if (m_config.aiSkill[i] == 0)
        {
            return 0;
        }
    }

    if (m_iComboState == 0 && SimulateAttack(m_config.aiSkill[m_iComboState]))
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] First Skill -> %d", m_iCurrentTarget);
        m_iComboState = 1;
    }
    else if (m_iComboState == 1 && SimulateAttack(m_config.aiSkill[m_iComboState]))
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Second Skill -> %d", m_iCurrentTarget);
        m_iComboState = 2;
    }
    else if (m_iComboState == 2 && SimulateAttack(m_config.aiSkill[m_iComboState]))
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Third Skill -> %d", m_iCurrentTarget);
        m_iComboState = 0;
    }

    return 1;
}

int CMuHelper::SimulateAttack(int iSkill)
{
    return SimulateSkill(iSkill, true, m_iCurrentTarget);
}

int CMuHelper::SimulateSkill(int iSkill, bool bTargetRequired, int iTarget)
{
    extern MovementSkill g_MovementSkill;
    extern int SelectedCharacter;
    extern int TargetX, TargetY;

    if (bTargetRequired)
    {
        if (iTarget == -1)
        {
            return 0;
        }

        SelectedCharacter = FindCharacterIndex(iTarget);
        if (SelectedCharacter == MAX_CHARACTERS_CLIENT)
        {
            DeleteTarget(iTarget);
            return 0;
        }

        CHARACTER* pTarget = &CharactersClient[SelectedCharacter];
        if (pTarget->Dead)
        {
            DeleteTarget(iTarget);
            return 0;
        }
        TargetX = (int)(pTarget->Object.Position[0] / TERRAIN_SCALE);
        TargetY = (int)(pTarget->Object.Position[1] / TERRAIN_SCALE);
    }

    int iSkillIndex = g_pSkillList->GetSkillIndex(iSkill);
    if (iSkillIndex == -1)
    {
        return 0;
    }

    Hero->CurrentSkill = iSkillIndex;

    g_MovementSkill.m_iSkill = iSkill;
    g_MovementSkill.m_iTarget = SelectedCharacter;
    g_MovementSkill.m_bMagic = FALSE;

    return ExecuteAttack(Hero);
}

int CMuHelper::Regroup()
{
    if (m_config.bReturnToOriginalPosition && m_iSecondsAway > m_config.iMaxSecondsAway)
    {
        if (!SimulateMove(m_posOriginal))
        {
            return 0;
        }

        m_iSecondsAway = 0;
        m_iComboState = 0;
        m_iCurrentTarget = -1;
    }

    return 1;
}

int CMuHelper::SimulateMove(POINT posMove)
{
    extern int TargetX, TargetY;

    Hero->MovementType = MOVEMENT_MOVE;
    TargetX = (int)posMove.x;
    TargetY = (int)posMove.y;

    if (PathFinding2((Hero->PositionX), (Hero->PositionY), TargetX, TargetY, &Hero->Path))
    {
        SendMove(Hero, &Hero->Object);
        return 0;
    }

    return !Hero->Movement;
}
