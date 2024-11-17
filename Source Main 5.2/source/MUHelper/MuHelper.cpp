#include "stdafx.h"

#include <thread>
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
#include "MapManager.h"
#include "WSclient.h"

#include "MuHelper.h"

constexpr int MAX_ACTIONABLE_DISTANCE = 10;
constexpr int DEFAULT_DURABILITY_THRESHOLD = 50;

CMuHelper g_MuHelper;
SpinLock _targetsLock;
SpinLock _itemsLock;

CMuHelper::CMuHelper()
{
    m_iLoopCounter = 0;
    m_iSecondsElapsed = 0;
    m_iSecondsAway = 0;
    m_iCurrentItem = -1;
    m_iCurrentTarget = -1;
    m_iCurrentSkill = -1;
    m_iComboState = 0;
    m_iCurrentBuffIndex = 0;
    m_iCurrentBuffPartyIndex = 0;
    m_iCurrentHealPartyIndex = 0;
    m_posOriginal = { 0, 0 };
    m_iHuntingDistance = 0;
    m_iObtainingDistance = 1;
    m_bTimerActivatedBuffOngoing = false;
    m_iTotalCost = 0;
}

void CALLBACK CMuHelper::TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) 
{
    g_MuHelper.WorkLoop(hwnd, uMsg, idEvent, dwTime);
}

void CMuHelper::Save(const cMuHelperConfig& config)
{
    m_config = config;

    PRECEIVE_MUHELPER_DATA netData;
    MuHelperConfigSerDe::Serialize(m_config, netData);

    SocketClient->ToGameServer()->SendMuHelperSaveDataRequest(reinterpret_cast<BYTE*>(&netData), sizeof(netData));
}

void CMuHelper::Load(const cMuHelperConfig& config)
{
    m_config = config;
}

cMuHelperConfig CMuHelper::GetConfig() const {
    return m_config;
}

void CMuHelper::Toggle()
{
    if (m_bActive) 
    {
        TriggerStop();
    }
    else
    {
        TriggerStart();
    }
}

void CMuHelper::TriggerStart()
{
    SocketClient->ToGameServer()->SendMuHelperStatusChangeRequest(0);
}

void CMuHelper::TriggerStop()
{
    SocketClient->ToGameServer()->SendMuHelperStatusChangeRequest(1);
}

void CMuHelper::Start()
{
    if (m_bActive)
    {
        return;
    }

    m_iTotalCost = 0;
    m_iComboState = 0;
    m_iCurrentBuffIndex = 0;
    m_iCurrentBuffPartyIndex = 0;
    m_iCurrentTarget = -1;
    m_iCurrentSkill = m_config.aiSkill[0];
    m_iCurrentItem = MAX_ITEMS;
    m_posOriginal = { Hero->PositionX, Hero->PositionY };

    m_iHuntingDistance = ComputeDistanceByRange(m_config.iHuntingRange);
    m_iObtainingDistance = ComputeDistanceByRange(m_config.iObtainingRange);

    m_iSecondsElapsed = 0;
    m_iSecondsAway = 0;
    m_setTargets.clear();
    m_setTargetsAttacking.clear();

    m_iLoopCounter = 0;

    m_bActive = true;
    g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Started");
}

void CMuHelper::Stop()
{
    m_bActive = false;
    g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Stopped");
}

void CMuHelper::WorkLoop(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    if (!m_bActive)
    {
        return;
    }

    if (Hero->SafeZone)
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Entered safezone. Stopping.");
        TriggerStop();
        return;
    }

    Work();

    if (m_iLoopCounter++ == 4)
    {
        m_iSecondsElapsed++;

        if (ComputeDistanceBetween({ Hero->PositionX, Hero->PositionY }, m_posOriginal) > 1)
        {
            m_iSecondsAway++;
        }
        else
        {
            m_iSecondsAway = 0;
        }

        m_iLoopCounter = 0;
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
        || (bIsAttacking && m_config.bLongRangeCounterAttack))
    {
        _targetsLock.lock();

        m_setTargets.insert(iTargetId);

        if (bIsAttacking)
        {
            m_setTargetsAttacking.insert(iTargetId);
        }

        _targetsLock.unlock();
    }

    if (m_config.bUseSelfDefense)
    {
        pTarget->Object.Kind = KIND_MONSTER;
        m_iCurrentTarget = iTargetId;
    }
}

void CMuHelper::DeleteTarget(int iTargetId)
{
    _targetsLock.lock();

    m_setTargets.erase(iTargetId);
    m_setTargetsAttacking.erase(iTargetId);

    _targetsLock.unlock();

    if (iTargetId == m_iCurrentTarget)
    {
        m_iCurrentTarget = -1;
    }
}

void CMuHelper::DeleteAllTargets()
{
    _targetsLock.lock();

    m_setTargets.clear();
    m_setTargetsAttacking.clear();

    _targetsLock.unlock();
}

int CMuHelper::ComputeDistanceByRange(int iRange)
{
    return ComputeDistanceBetween({ 0, 0 }, { iRange, iRange });
}

int CMuHelper::ComputeDistanceFromTarget(CHARACTER* pTarget)
{
    POINT posA, posB;

    posA = { Hero->PositionX, Hero->PositionY };
    posB = { pTarget->PositionX, pTarget->PositionY };
    int iPrevDistance = ComputeDistanceBetween(posA, posB);

    posA = { Hero->PositionX, Hero->PositionY };
    posB = { pTarget->TargetX, pTarget->TargetX };
    int iNextDistance = ComputeDistanceBetween(posA, posB);

    return min(iPrevDistance, iNextDistance);
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
        _targetsLock.lock();
        setTargets = m_setTargets;
        _targetsLock.unlock();
    }

    for (const int& iMonsterId : setTargets)
    {
        int iIndex = FindCharacterIndex(iMonsterId);
        CHARACTER* pTarget = &CharactersClient[iIndex];

        int iDistance = ComputeDistanceFromTarget(pTarget);
        if (iDistance < iMinDistance)
        {
            iMinDistance = iDistance;
            iClosestMonsterId = iMonsterId;
        }
    }

    return iClosestMonsterId;
}

int CMuHelper::GetFarthestAttackingTarget()
{
    int iFarthestMonsterId = -1;
    int iMaxDistance = -1;

    std::set<int> setTargets;
    {
        _targetsLock.lock();
        setTargets = m_setTargetsAttacking;
        _targetsLock.unlock();
    }

    for (const int& iMonsterId : setTargets)
    {
        int iIndex = FindCharacterIndex(iMonsterId);
        CHARACTER* pTarget = &CharactersClient[iIndex];

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
    if (!HasAssignedBuffSkill())
    {
        return 1;
    }

    if (m_config.bSupportParty && g_pPartyManager->IsPartyActive())
    {
        PARTY_t* pMember = &Party[m_iCurrentBuffPartyIndex];
        CHARACTER* pChar = g_pPartyManager->GetPartyMemberChar(pMember);

        if (pChar != NULL
            && pMember->Map == gMapManager.WorldActive
            && ComputeDistanceFromTarget(pChar) <= MAX_ACTIONABLE_DISTANCE)
        {
            if (!m_config.bBuffDurationParty
                && m_config.iBuffCastInterval != 0
                && m_iSecondsElapsed % m_config.iBuffCastInterval == 0)
            {
                m_bTimerActivatedBuffOngoing = true;
            }

            if (!BuffTarget(pChar, m_config.aiBuff[m_iCurrentBuffIndex]))
            {
                return 0;
            }
        }

        m_iCurrentBuffPartyIndex = (m_iCurrentBuffPartyIndex + 1) % (sizeof(Party) / sizeof(Party[0]));
    }
    else
    {
        if (!m_config.bBuffDuration
            && m_config.iBuffCastInterval != 0
            && m_iSecondsElapsed % m_config.iBuffCastInterval == 0)
        {
            m_bTimerActivatedBuffOngoing = true;
        }

        if (!BuffTarget(Hero, m_config.aiBuff[m_iCurrentBuffIndex]))
        {
            return 0;
        }
    }

    if (m_iCurrentBuffPartyIndex == 0)
    {
        m_iCurrentBuffIndex = (m_iCurrentBuffIndex + 1) % m_config.aiBuff.size();

        // Reaching this branch means everyone's been buffed, 
        // so we're resetting the timer activated buff flag
        if (m_iCurrentBuffIndex == 0)
        {
            m_bTimerActivatedBuffOngoing = false;
        }
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
        && (!g_isCharacterBuff((&pTargetChar->Object), eBuff_Attack) || m_bTimerActivatedBuffOngoing))
    {
        return SimulateSkill(iBuffSkill, true, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_DEFENSE
        || iBuffSkill == AT_SKILL_DEF_POWER_UP
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 1
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 2
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 3
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 4)
        && (!g_isCharacterBuff((&pTargetChar->Object), eBuff_Defense) || m_bTimerActivatedBuffOngoing))
    {
        return SimulateSkill(iBuffSkill, true, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_INFINITY_ARROW) &&
        (!g_isCharacterBuff((&pTargetChar->Object), eBuff_InfinityArrow)))
    {
        return SimulateSkill(iBuffSkill, false, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_WIZARDDEFENSE
        || iBuffSkill == AT_SKILL_SOUL_UP
        || iBuffSkill == AT_SKILL_SOUL_UP + 1
        || iBuffSkill == AT_SKILL_SOUL_UP + 2
        || iBuffSkill == AT_SKILL_SOUL_UP + 3
        || iBuffSkill == AT_SKILL_SOUL_UP + 4)
        && (!g_isCharacterBuff((&pTargetChar->Object), eBuff_WizDefense) || m_bTimerActivatedBuffOngoing))
    {
        return SimulateSkill(iBuffSkill, true, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_VITALITY
        || iBuffSkill == AT_SKILL_LIFE_UP
        || iBuffSkill == AT_SKILL_LIFE_UP + 1
        || iBuffSkill == AT_SKILL_LIFE_UP + 2
        || iBuffSkill == AT_SKILL_LIFE_UP + 3
        || iBuffSkill == AT_SKILL_LIFE_UP + 4)
        && (!g_isCharacterBuff((&pTargetChar->Object), eBuff_Life) || m_bTimerActivatedBuffOngoing))
    {
        if (m_iComboState == 2)
        {
            return 1;
        }
        
        return SimulateSkill(iBuffSkill, false, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_SWELL_OF_MAGICPOWER)
        && (!g_isCharacterBuff((&pTargetChar->Object), eBuff_SwellOfMagicPower)))
    {
        return SimulateSkill(iBuffSkill, false, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_ADD_CRITICAL)
        && (!g_isCharacterBuff((&pTargetChar->Object), eBuff_AddCriticalDamage)))
    {
        return SimulateSkill(iBuffSkill, false, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_ALICE_BERSERKER)
        && (!g_isCharacterBuff((&pTargetChar->Object), eBuff_Berserker)))
    {
        return SimulateSkill(iBuffSkill, false, pTargetChar->Key);
    }
    if ((iBuffSkill == AT_SKILL_ALICE_THORNS)
        && (!g_isCharacterBuff((&pTargetChar->Object), eBuff_Thorns)))
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
    if (!m_config.bAutoHeal)
    {
        return 1;
    }

    int iHealingSkill = GetHealingSkill();
    if (iHealingSkill == -1)
    {
        return 1;
    }

    if (m_config.bAutoHealParty && g_pPartyManager->IsPartyActive())
    {
        PARTY_t* pMember = &Party[m_iCurrentHealPartyIndex];
        CHARACTER* pChar = g_pPartyManager->GetPartyMemberChar(pMember);

        if (pChar != NULL)
        {
            if (pChar == Hero)
            {
                return HealSelf(iHealingSkill);
            }
            else if (pMember->Map == gMapManager.WorldActive 
                && pMember->stepHP * 10 <= m_config.iHealPartyThreshold
                && ComputeDistanceFromTarget(pChar) <= MAX_ACTIONABLE_DISTANCE)
            {
                return SimulateSkill(iHealingSkill, true, pChar->Key);
            }
        }
        m_iCurrentHealPartyIndex = (m_iCurrentHealPartyIndex + 1) % (sizeof(Party) / sizeof(Party[0]));
    }
    else
    {
        return HealSelf(iHealingSkill);
    }

    return 1;
}

int CMuHelper::HealSelf(int iHealingSkill)
{
    int64_t iLife = CharacterAttribute->Life;
    int64_t iLifeMax = CharacterAttribute->LifeMax;
    int64_t iRemaining = (iLife * 100 + iLifeMax - 1) / iLifeMax;

    if (iRemaining <= m_config.iHealThreshold)
    {
        return SimulateSkill(iHealingSkill, true, HeroKey);
    }

    return 1;
}

int CMuHelper::RepairEquipments()
{
    if (m_config.bRepairItem)
    {
        for (int i = 0; i < MAX_EQUIPMENT; i++)
        {
            ITEM* pItem = &CharacterMachine->Equipment[i];
            if (!pItem || pItem->Type == -1)
            {
                continue;
            }

            ITEM_ATTRIBUTE* pAttr = &ItemAttribute[pItem->Type];
            if (!pAttr)
            {
                continue;
            }

            int iLevel = pItem->Level;
            int iDurability = pItem->Durability;
            int iMaxDurability = CalcMaxDurability(pItem, pAttr, iLevel);

            int64_t iHealth = (iDurability * 100 + iMaxDurability - 1) / iMaxDurability;

            if (iHealth <= DEFAULT_DURABILITY_THRESHOLD)
            {
                SocketClient->ToGameServer()->SendRepairItemRequest(i, 1);
            }
        }
    }

    return 1;
}

int CMuHelper::Attack()
{
    if (m_iCurrentTarget == -1)
    {
        if (!m_setTargets.empty())
        {
            if (m_config.bLongRangeCounterAttack)
            {
                m_iCurrentTarget = GetFarthestAttackingTarget();
            }
            else
            {
                m_iCurrentTarget = GetNearestTarget();
            }
        }
        else 
        {
            m_iComboState = 0;
            return 0;
        }
    }

    if (m_config.bUseCombo)
    {
        return SimulateComboAttack();
    }

    m_iCurrentSkill = SelectAttackSkill();
    if (m_iCurrentSkill > 0)
    {
        SimulateAttack(m_iCurrentSkill);
    }

    return 1;
}

int CMuHelper::SelectAttackSkill()
{
    // try skill 2 activation conditions
    if (m_config.aiSkill[1] > 0)
    {
        if ((m_config.aiSkillCondition[1] & ON_TIMER)
            && m_config.aiSkillInterval[1] != 0
            && m_iSecondsElapsed % m_config.aiSkillInterval[1] == 0)
        {
            g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 2 Timer");
            return m_config.aiSkill[1];
        }

        if (m_config.aiSkillCondition[1] & ON_CONDITION)
        {
            if (m_config.aiSkillCondition[1] & ON_MOBS_NEARBY)
            {
                int iCount = m_setTargets.size();

                if (((m_config.aiSkillCondition[1] & ON_MORE_THAN_TWO_MOBS) && iCount >= 2)
                    || ((m_config.aiSkillCondition[1] & ON_MORE_THAN_THREE_MOBS) && iCount >= 3)
                    || ((m_config.aiSkillCondition[1] & ON_MORE_THAN_FOUR_MOBS) && iCount >= 4)
                    || ((m_config.aiSkillCondition[1] & ON_MORE_THAN_FIVE_MOBS) && iCount >= 5))
                {
                    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 2 Hunting Range Condition");
                    return m_config.aiSkill[1];
                }
            }
            else if (m_config.aiSkillCondition[1] & ON_MOBS_ATTACKING)
            {
                int iCount = m_setTargetsAttacking.size();

                if (((m_config.aiSkillCondition[1] & ON_MORE_THAN_TWO_MOBS) && iCount >= 2)
                    || ((m_config.aiSkillCondition[1] & ON_MORE_THAN_THREE_MOBS) && iCount >= 3)
                    || ((m_config.aiSkillCondition[1] & ON_MORE_THAN_FOUR_MOBS) && iCount >= 4)
                    || ((m_config.aiSkillCondition[1] & ON_MORE_THAN_FIVE_MOBS) && iCount >= 5))
                {
                    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 2 Attacking Me Condition");
                    return m_config.aiSkill[1];
                }
            }
        }
    }

    // try skill 3 activation conditions
    if (m_config.aiSkill[2] > 0
        && (m_config.aiSkillCondition[2] & ON_TIMER
            || m_config.aiSkillCondition[2] & ON_CONDITION))
    {
        if ((m_config.aiSkillCondition[2] & ON_TIMER)
            && m_config.aiSkillInterval[2] != 0
            && m_iSecondsElapsed % m_config.aiSkillInterval[2] == 0)
        {
            g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 3 Timer");
            return m_config.aiSkill[2];
        }

        if (m_config.aiSkillCondition[2] & ON_CONDITION)
        {
            if (m_config.aiSkillCondition[2] & ON_MOBS_NEARBY)
            {
                int iCount = m_setTargets.size();

                if (((m_config.aiSkillCondition[2] & ON_MORE_THAN_TWO_MOBS) && iCount >= 2)
                    || ((m_config.aiSkillCondition[2] & ON_MORE_THAN_THREE_MOBS) && iCount >= 3)
                    || ((m_config.aiSkillCondition[2] & ON_MORE_THAN_FOUR_MOBS) && iCount >= 4)
                    || ((m_config.aiSkillCondition[2] & ON_MORE_THAN_FIVE_MOBS) && iCount >= 5))
                {
                    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 3 Hunting Range Condition");
                    return m_config.aiSkill[2];
                }
            }
            else if (m_config.aiSkillCondition[2] & ON_MOBS_ATTACKING)
            {
                int iCount = m_setTargetsAttacking.size();

                if (((m_config.aiSkillCondition[2] & ON_MORE_THAN_TWO_MOBS) && iCount >= 2)
                    || ((m_config.aiSkillCondition[2] & ON_MORE_THAN_THREE_MOBS) && iCount >= 3)
                    || ((m_config.aiSkillCondition[2] & ON_MORE_THAN_FOUR_MOBS) && iCount >= 4)
                    || ((m_config.aiSkillCondition[2] & ON_MORE_THAN_FIVE_MOBS) && iCount >= 5))
                {
                    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 3 Attacking Me Condition");
                    return m_config.aiSkill[2];
                }
            }
        }
    }

    // no skill for activation yet, default to basic skill
    if (m_config.aiSkill[0] > 0)
    {
        return m_config.aiSkill[0];
    }

    return -1;
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

    if (SimulateAttack(m_config.aiSkill[m_iComboState])) 
    {
        m_iComboState = (m_iComboState + 1) % 3;
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

        g_MovementSkill.m_iSkill = iSkill;
        g_MovementSkill.m_iTarget = SelectedCharacter;
        g_MovementSkill.m_bMagic = FALSE;
    }

    float fDistance = gSkillManager.GetSkillDistance(iSkill, Hero);

    return ExecuteSkill(Hero, iSkill, fDistance);
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

    if (!CheckTile(Hero, &Hero->Object, 1.5f))
    {
        if (PathFinding2((Hero->PositionX), (Hero->PositionY), TargetX, TargetY, &Hero->Path))
        {
            SendMove(Hero, &Hero->Object);
        }
        return 0;
    }

    return 1;
}

bool CMuHelper::HasAssignedBuffSkill()
{
    for (int i = 0; i < m_config.aiBuff.size(); i++)
    {
        if (m_config.aiBuff[i] != 0)
        {
            return true;
        }
    }

    return false;
}

int CMuHelper::GetHealingSkill()
{
    int aiHealingSkills[] =
    {
        AT_SKILL_HEAL_UP,
        AT_SKILL_HEAL_UP +1,
        AT_SKILL_HEAL_UP +2,
        AT_SKILL_HEAL_UP +3,
        AT_SKILL_HEAL_UP +4,
        AT_SKILL_HEALING,
        AT_SKILL_ALICE_DRAINLIFE_UP,
        AT_SKILL_ALICE_DRAINLIFE_UP + 1,
        AT_SKILL_ALICE_DRAINLIFE_UP + 2,
        AT_SKILL_ALICE_DRAINLIFE_UP + 3,
        AT_SKILL_ALICE_DRAINLIFE_UP + 4,
        AT_SKILL_ALICE_DRAINLIFE,
    };

    for (int i = 0; i < 6; i++)
    {
        int iSkillIndex = g_pSkillList->GetSkillIndex(aiHealingSkills[i]);
        if (iSkillIndex != -1)
        {
            return aiHealingSkills[i];
        }
    }

    return -1;
}

int CMuHelper::ObtainItem()
{
    if (m_iCurrentItem == MAX_ITEMS)
    {
        m_iCurrentItem = SelectItemToObtain();
        if (m_iCurrentItem == MAX_ITEMS)
        {
            return 1;
        }
    }

    ITEM_t* pDrop = &Items[m_iCurrentItem];
    ITEM* pItem = &pDrop->Item;

    if (!pDrop->Object.Live)
    {
        DeleteItem(m_iCurrentItem);
        return 1;
    }

    extern int TargetX;
    extern int TargetY;

    TargetX = (int)(Items[m_iCurrentItem].Object.Position[0] / TERRAIN_SCALE);
    TargetY = (int)(Items[m_iCurrentItem].Object.Position[1] / TERRAIN_SCALE);

    int iDistance = ComputeDistanceBetween({ Hero->PositionX, Hero->PositionY }, {TargetX, TargetY});
    if (iDistance <= m_iObtainingDistance)
    {
        if (!CheckTile(Hero, &Hero->Object, 1.5f))
        {
            if (PathFinding2((Hero->PositionX), (Hero->PositionY), TargetX, TargetY, &Hero->Path))
            {
                SendMove(Hero, &Hero->Object);
            }

            return 0;
        }
        else 
        {
            SocketClient->ToGameServer()->SendPickupItemRequest(m_iCurrentItem);
            DeleteItem(m_iCurrentItem);
        }
    }

    return 1;
}

bool CMuHelper::ShouldObtainItem(int iItemId)
{
    ITEM_t* pDrop = &Items[iItemId];
    ITEM* pItem = &pDrop->Item;

    if ((m_config.bPickZen && IsMoneyItem(pItem))
        || (m_config.bPickJewel && IsJewelItem(pItem))
        || (m_config.bPickAncient && IsAncientItem(pItem))
        || (m_config.bPickExcellent && IsExcellentItem(pItem)))
    {
        return true;
    }

    if (m_config.bPickExtraItems)
    {
        std::wstring strDisplayName = GetItemDisplayName(pItem);

        for (const auto& str : m_config.aExtraItems)
        {
            // Check if the search keyword is in the item's display name
            if (strDisplayName.find(str) != std::wstring::npos)
            {
                return true;
            }
        }
    }

    return m_config.bPickAllItems;
}

void CMuHelper::AddItem(int iItemId, POINT posWhere)
{
    _itemsLock.lock();
    m_setItems.insert(iItemId);
    _itemsLock.unlock();
}

void CMuHelper::DeleteItem(int iItemId)
{
    _itemsLock.lock();
    m_setItems.erase(iItemId);
    _itemsLock.unlock();

    if (iItemId == m_iCurrentItem)
    {
        m_iCurrentItem = MAX_ITEMS;
    }
}

int CMuHelper::SelectItemToObtain()
{
    int iClosestItemId = MAX_ITEMS;
    int iMinDistance = m_config.iObtainingRange + 1;

    std::set<int> setItems;
    {
        _itemsLock.lock();
        setItems = m_setItems;
        _itemsLock.unlock();
    }

    for (const int& iItemId : setItems)
    {
        if (!ShouldObtainItem(iItemId))
        {
            continue;
        }

        int iItemX = (int)(Items[iItemId].Object.Position[0] / TERRAIN_SCALE);
        int iItemY = (int)(Items[iItemId].Object.Position[1] / TERRAIN_SCALE);

        int iDistance = ComputeDistanceBetween({ Hero->PositionX, Hero->PositionY }, { iItemX, iItemY });
        if (iDistance < iMinDistance)
        {
            iMinDistance = iDistance;
            iClosestItemId = iItemId;
        }
    }

    return iClosestItemId;
}
