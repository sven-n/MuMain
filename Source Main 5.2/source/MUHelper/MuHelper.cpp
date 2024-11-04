#include "stdafx.h"

#undef max
#include <limits>

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
#include "WSclient.h"

#include "MuHelper.h"


CMuHelper g_MuHelper;
std::mutex _mtx_targetSet;

CMuHelper::CMuHelper()
{
    m_iCurrentTarget = -1;
    m_iComboState = 0;
    m_iCurrentSkill = MAX_MAGIC;
    m_iCurrentBuffIndex = 0;
    m_bSavedAutoAttack = false;
}

void CMuHelper::Save(const cMuHelperConfig& config)
{
    m_config = config;

    // Save config data by sending to server
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
    m_iCurrentSkill = m_config.aiSkill[0];

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
    while (m_bActive) {
        Work();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
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

        ConsumePotion();
        Attack();
        ObtainItem();
        RepairEquipments();
    }
    catch (...)
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Exception occurred. Ignoring...");
    }
}

void CMuHelper::AddTarget(int iTargetId)
{
    std::lock_guard<std::mutex> lock(_mtx_targetSet);

    if (iTargetId == HeroKey || m_setTargets.find(iTargetId) != m_setTargets.end()) 
    {
        return;
    }

    if (m_setTargets.size() >= 5)
    {
        int iFarthestTarget = GetFarthestTarget();
        m_setTargets.erase(iFarthestTarget);
    }

    if (m_config.bUseSelfDefense)
    {
        CHARACTER* pTarget = FindCharacterByKey(iTargetId);
        if (pTarget)
        {
            pTarget->Object.Kind = KIND_MONSTER;
            m_iCurrentTarget = iTargetId;
        }
    }

    m_setTargets.insert(iTargetId);

    g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Added target -> %d", iTargetId);
}

void CMuHelper::DeleteTarget(int iTargetId)
{
    std::lock_guard<std::mutex> lock(_mtx_targetSet);

    g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Deleted target -> %d", iTargetId);
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

int CMuHelper::ComputeDistanceFromTarget(CHARACTER* pTarget)
{
    int iPlayerX = (int)(Hero->Object.Position[0] / TERRAIN_SCALE);
    int iPlayerY = (int)(Hero->Object.Position[1] / TERRAIN_SCALE);

    int iTargetX = (int)(pTarget->Object.Position[0] / TERRAIN_SCALE);
    int iTargetY = (int)(pTarget->Object.Position[1] / TERRAIN_SCALE);

    int iDx = iTargetX - iPlayerX;
    int iDy = iTargetY - iPlayerY;
    return iDx * iDx + iDy * iDy;
}

int CMuHelper::GetNearestTarget()
{
    int iClosestMonsterId = -1;
    int iMinDistance = std::numeric_limits<int>::max();

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
        bool bPartyActive = false;

        for (int i = 0; i < ((sizeof(Party) / sizeof(Party[0])) - 1); i++)
        {
            PARTY_t* pMember = &Party[i];
            CHARACTER* pTargetChar = FindCharacterByID(pMember->Name);
            if (pTargetChar == Hero)
            {
                bPartyActive = true;
                break;
            }
        }
        
        if (bPartyActive)
        {
            PARTY_t* pMember = &Party[m_iCurrentPartyMemberIndex];
            CHARACTER* pTargetChar = FindCharacterByID(pMember->Name);
            if (pTargetChar)
            {
                if (!BuffTarget(pTargetChar, m_config.aiBuff[m_iCurrentBuffIndex]))
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
        m_iCurrentTarget = pTargetChar->Key;
        return SimulateSkill(iBuffSkill, true);
    }

    if ((iBuffSkill == AT_SKILL_DEFENSE
        || iBuffSkill == AT_SKILL_DEF_POWER_UP
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 1
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 2
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 3
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 4)
        && !g_isCharacterBuff((&pTargetChar->Object), eBuff_Defense))
    {
        m_iCurrentTarget = pTargetChar->Key;
        return SimulateSkill(iBuffSkill, true);
    }

    if ((iBuffSkill == AT_SKILL_WIZARDDEFENSE
        || iBuffSkill == AT_SKILL_SOUL_UP
        || iBuffSkill == AT_SKILL_SOUL_UP + 1
        || iBuffSkill == AT_SKILL_SOUL_UP + 2
        || iBuffSkill == AT_SKILL_SOUL_UP + 3
        || iBuffSkill == AT_SKILL_SOUL_UP + 4)
        && !g_isCharacterBuff((&pTargetChar->Object), eBuff_WizDefense))
    {
        m_iCurrentTarget = pTargetChar->Key;
        return SimulateSkill(iBuffSkill, true);
    }

    if ((iBuffSkill == AT_SKILL_VITALITY
        || iBuffSkill == AT_SKILL_LIFE_UP
        || iBuffSkill == AT_SKILL_LIFE_UP + 1
        || iBuffSkill == AT_SKILL_LIFE_UP + 2
        || iBuffSkill == AT_SKILL_LIFE_UP + 3
        || iBuffSkill == AT_SKILL_LIFE_UP + 4)
        && !g_isCharacterBuff((&pTargetChar->Object), eBuff_Life))
    {
        return SimulateSkill(iBuffSkill, false);
    }

    if ((iBuffSkill == AT_SKILL_SWELL_OF_MAGICPOWER)
        && !g_isCharacterBuff((&pTargetChar->Object), eBuff_SwellOfMagicPower))
    {
        return SimulateSkill(iBuffSkill, false);
    }

    if ((iBuffSkill == AT_SKILL_ADD_CRITICAL)
        && !g_isCharacterBuff((&pTargetChar->Object), eBuff_AddCriticalDamage))
    {
        return SimulateSkill(iBuffSkill, false);
    }

    return 1;
}

int CMuHelper::ConsumePotion()
{
    return 1;
}

int CMuHelper::Heal()
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
            m_iCurrentTarget = GetNearestTarget();

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

    return SimulateAttack(m_iCurrentSkill);
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
    return SimulateSkill(iSkill, true);
}

int CMuHelper::SimulateSkill(int iSkill, bool bTargetRequired)
{
    extern MovementSkill g_MovementSkill;
    extern int SelectedCharacter;
    extern int TargetX, TargetY;

    if (bTargetRequired)
    {
        if (m_iCurrentTarget == -1)
        {
            return 0;
        }

        SelectedCharacter = FindCharacterIndex(m_iCurrentTarget);
        if (SelectedCharacter == MAX_CHARACTERS_CLIENT)
        {
            DeleteTarget(m_iCurrentTarget);
            return 0;
        }

        CHARACTER* pTarget = &CharactersClient[SelectedCharacter];
        if (pTarget->Dead)
        {
            DeleteTarget(m_iCurrentTarget);
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
