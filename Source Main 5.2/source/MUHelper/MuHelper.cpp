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
#include "MapManager.h"
#include "WSclient.h"

#include "MuHelper.h"

#define MAX_ACTIONABLE_DISTANCE        10
#define DEFAULT_DURABILITY_THRESHOLD   50

CMuHelper g_MuHelper;
std::mutex _mtx_targetSet;
std::mutex _mtx_itemSet;

CMuHelper::CMuHelper()
{
    m_iCurrentItem = -1;
    m_iCurrentTarget = -1;
    m_iComboState = 0;
    m_iCurrentBuffIndex = 0;
    m_iCurrentBuffPartyIndex = 0;
    m_iCurrentHealPartyIndex = 0;
    m_iSecondsElapsed = 0;
    m_iSecondsAway = 0;
    m_posOriginal = { 0, 0 };
    m_iHuntingDistance = 0;
    m_iObtainingDistance = 1;
}

void CALLBACK CMuHelper::TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) 
{
    g_MuHelper.WorkLoop(hwnd, uMsg, idEvent, dwTime);
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

    g_ConsoleDebug->Write(MCD_NORMAL, L"Hunting Range: %d", m_config.iHuntingRange);
    g_ConsoleDebug->Write(MCD_NORMAL, L"Original Position: %d", m_config.bReturnToOriginalPosition);
    g_ConsoleDebug->Write(MCD_NORMAL, L"DistanceMin: %d", m_config.iMaxSecondsAway);

    g_ConsoleDebug->Write(MCD_NORMAL, L"Basic Skill: %d", m_config.aiSkill[0]);
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 1: %d", m_config.aiSkill[1]);
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 2: %d", m_config.aiSkill[2]);
    g_ConsoleDebug->Write(MCD_NORMAL, L"Combo? %d", m_config.bUseCombo);

    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 1 Delay? %d", !!(m_config.aiSkillCondition[1] & MUHELPER_ATTACK_ON_TIMER));
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 1 Condition? %d", !!(m_config.aiSkillCondition[1] & MUHELPER_ATTACK_CONDITIONS_MASK));
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 1 Pre Condition Hunting Range? %d", !!(m_config.aiSkillCondition[1] & MUHELPER_ATTACK_ON_MOBS_NEARBY));
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 1 Pre Condition Attacking Me? %d", !!(m_config.aiSkillCondition[1] & MUHELPER_ATTACK_ON_MOBS_ATTACKING));
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 1 Sub Condition 2 or more? %d", !!(m_config.aiSkillCondition[1] & MUHELPER_ATTACK_ON_MORE_THAN_TWO_MOBS));
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 1 Sub Condition 3 or more? %d", !!(m_config.aiSkillCondition[1] & MUHELPER_ATTACK_ON_MORE_THAN_THREE_MOBS));
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 1 Sub Condition 4 or more? %d", !!(m_config.aiSkillCondition[1] & MUHELPER_ATTACK_ON_MORE_THAN_FOUR_MOBS));
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 1 Sub Condition 5 or more? %d", !!(m_config.aiSkillCondition[1] & MUHELPER_ATTACK_ON_MORE_THAN_FIVE_MOBS));

    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 2 Delay? %d", !!(m_config.aiSkillCondition[2] & MUHELPER_ATTACK_ON_TIMER));
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 2 Condition? %d", !!(m_config.aiSkillCondition[2] & MUHELPER_ATTACK_CONDITIONS_MASK));
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 2 Pre Condition Hunting Range? %d", !!(m_config.aiSkillCondition[2] & MUHELPER_ATTACK_ON_MOBS_NEARBY));
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 2 Pre Condition Attacking Me? %d", !!(m_config.aiSkillCondition[2] & MUHELPER_ATTACK_ON_MOBS_ATTACKING));
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 2 Sub Condition 2 or more? %d", !!(m_config.aiSkillCondition[2] & MUHELPER_ATTACK_ON_MORE_THAN_TWO_MOBS));
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 2 Sub Condition 3 or more? %d", !!(m_config.aiSkillCondition[2] & MUHELPER_ATTACK_ON_MORE_THAN_THREE_MOBS));
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 2 Sub Condition 4 or more? %d", !!(m_config.aiSkillCondition[2] & MUHELPER_ATTACK_ON_MORE_THAN_FOUR_MOBS));
    g_ConsoleDebug->Write(MCD_NORMAL, L"Skill 2 Sub Condition 5 or more? %d", !!(m_config.aiSkillCondition[2] & MUHELPER_ATTACK_ON_MORE_THAN_FIVE_MOBS));

    g_ConsoleDebug->Write(MCD_NORMAL, L"Buff 0: %d", m_config.aiBuff[0]);
    g_ConsoleDebug->Write(MCD_NORMAL, L"buff 1: %d", m_config.aiBuff[1]);
    g_ConsoleDebug->Write(MCD_NORMAL, L"Buff 2: %d", m_config.aiBuff[2]);

    g_ConsoleDebug->Write(MCD_NORMAL, L"Obtain Range: %d", m_config.iObtainingRange);
    g_ConsoleDebug->Write(MCD_NORMAL, L"Pick All: %d", m_config.bPickAllItems);
    g_ConsoleDebug->Write(MCD_NORMAL, L"Pick Zen: %d", m_config.bPickZen);
    g_ConsoleDebug->Write(MCD_NORMAL, L"Pick Jewel: %d", m_config.bPickJewel);
    g_ConsoleDebug->Write(MCD_NORMAL, L"Pick Excellent: %d", m_config.bPickExcellent);
    g_ConsoleDebug->Write(MCD_NORMAL, L"Pick Ancient: %d", m_config.bPickAncient);
    g_ConsoleDebug->Write(MCD_NORMAL, L"Pick Extra Items: %d", m_config.bPickExtraItems);
}

cMuHelperConfig CMuHelper::GetConfig() const {
    return m_config;
}

void CMuHelper::Toggle()
{
    // Determine if the thread should be started
    bool bStart = !m_bActive; 

    if (m_bActive) {
        m_bActive = false;
        g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Stopped");
        return;
    }

    m_iComboState = 0;
    m_iCurrentBuffIndex = 0;
    m_iCurrentBuffPartyIndex = 0;
    m_iCurrentTarget = -1;
    m_iCurrentItem = MAX_ITEMS;
    m_posOriginal = { Hero->PositionX, Hero->PositionY };

    m_iHuntingDistance = ComputeDistanceByRange(m_config.iHuntingRange);
    m_iObtainingDistance = ComputeDistanceByRange(m_config.iObtainingRange);

    m_iSecondsElapsed = 0;
    m_iSecondsAway = 0;
    m_setTargets.clear();

    if (bStart && !Hero->SafeZone)
    {
        // Start the thread
        g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Starting");
        m_bActive = true;
        m_iLoopCounter = 0;
        m_iSecondsElapsed = 0;
        m_iSecondsAway = 0;
    }
}

void CMuHelper::Start()
{
    if (!m_bActive) {
        Toggle();
    }
}

void CMuHelper::Stop()
{
    if (m_bActive) {
        Toggle();
    }
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
        m_bActive = false;
        return;
    }

    Work();

    if (m_iLoopCounter++ == 5)
    {
        m_iSecondsSinceLastBuff++;
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
        || (bIsAttacking && m_config.bLongRangeCounterAttack))
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
        std::lock_guard<std::mutex> lock(_mtx_targetSet);
        setTargets = m_setTargets;
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
            if (!BuffTarget(pChar, m_config.aiBuff[m_iCurrentBuffIndex]))
            {
                return 0;
            }
        }

        m_iCurrentBuffPartyIndex = (m_iCurrentBuffPartyIndex + 1) % (sizeof(Party) / sizeof(Party[0]));
    }
    else
    {
        if (!BuffTarget(Hero, m_config.aiBuff[m_iCurrentBuffIndex]))
        {
            return 0;
        }
    }

    if (m_iCurrentBuffPartyIndex == 0)
    {
        m_iCurrentBuffIndex = (m_iCurrentBuffIndex + 1) % m_config.aiBuff.size();
    }

    m_iSecondsSinceLastBuff = 0;

    return 1;
}

int CMuHelper::BuffTarget(CHARACTER* pTargetChar, int iBuffSkill)
{
    bool bForceBuff = false;

    if (m_config.bBuffDurationParty && m_iSecondsSinceLastBuff >= m_config.iBuffCastInterval)
    {
        bForceBuff = true;
    }

    // TODO: List other buffs here
    if ((iBuffSkill == AT_SKILL_ATTACK
        || iBuffSkill == AT_SKILL_ATT_POWER_UP
        || iBuffSkill == AT_SKILL_ATT_POWER_UP + 1
        || iBuffSkill == AT_SKILL_ATT_POWER_UP + 2
        || iBuffSkill == AT_SKILL_ATT_POWER_UP + 3
        || iBuffSkill == AT_SKILL_ATT_POWER_UP + 4)
        && (!g_isCharacterBuff((&pTargetChar->Object), eBuff_Attack) || bForceBuff))
    {
        return SimulateSkill(iBuffSkill, true, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_DEFENSE
        || iBuffSkill == AT_SKILL_DEF_POWER_UP
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 1
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 2
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 3
        || iBuffSkill == AT_SKILL_DEF_POWER_UP + 4)
        && (!g_isCharacterBuff((&pTargetChar->Object), eBuff_Defense) || bForceBuff))
    {
        return SimulateSkill(iBuffSkill, true, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_WIZARDDEFENSE
        || iBuffSkill == AT_SKILL_SOUL_UP
        || iBuffSkill == AT_SKILL_SOUL_UP + 1
        || iBuffSkill == AT_SKILL_SOUL_UP + 2
        || iBuffSkill == AT_SKILL_SOUL_UP + 3
        || iBuffSkill == AT_SKILL_SOUL_UP + 4)
        && (!g_isCharacterBuff((&pTargetChar->Object), eBuff_WizDefense) || bForceBuff))
    {
        return SimulateSkill(iBuffSkill, true, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_VITALITY
        || iBuffSkill == AT_SKILL_LIFE_UP
        || iBuffSkill == AT_SKILL_LIFE_UP + 1
        || iBuffSkill == AT_SKILL_LIFE_UP + 2
        || iBuffSkill == AT_SKILL_LIFE_UP + 3
        || iBuffSkill == AT_SKILL_LIFE_UP + 4)
        && (!g_isCharacterBuff((&pTargetChar->Object), eBuff_Life) || bForceBuff))
    {
        if (m_iComboState == 2)
        {
            return 1;
        }
        
        return SimulateSkill(iBuffSkill, false, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_SWELL_OF_MAGICPOWER)
        && (!g_isCharacterBuff((&pTargetChar->Object), eBuff_SwellOfMagicPower) || bForceBuff))
    {
        return SimulateSkill(iBuffSkill, false, pTargetChar->Key);
    }

    if ((iBuffSkill == AT_SKILL_ADD_CRITICAL)
        && (!g_isCharacterBuff((&pTargetChar->Object), eBuff_AddCriticalDamage) || bForceBuff))
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
                m_iCurrentTarget = GetFarthestTarget();
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

    // to-do select skill
    if (m_config.aiSkill[0] != 0)
    {
        return SimulateAttack(m_config.aiSkill[0]);
    }

    return 1;
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
        AT_SKILL_HEALING
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
    std::lock_guard<std::mutex> lock(_mtx_itemSet);
    m_setItems.insert(iItemId);
}

void CMuHelper::DeleteItem(int iItemId)
{
    std::lock_guard<std::mutex> lock(_mtx_itemSet);
    m_setItems.erase(iItemId);

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
        std::lock_guard<std::mutex> lock(_mtx_targetSet);
        setItems = m_setItems;
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
