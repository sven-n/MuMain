#include "stdafx.h"
#include "GameLogic/Automation/Attack.h"
#include "GameLogic/Automation/HelperPickupFilter.h"
#include "GameLogic/Automation/Movement.h"
#include "GameLogic/Automation/Pickup.h"
#include "GameLogic/Automation/Skill.h"
#include "GameLogic/Combat/SkillExecution.h"
#include "GameLogic/Items/ShopRestrictions.h"

#include <thread>
#include <atomic>
#include <chrono>
#include <cmath>

#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/PlayerActionState.h"
#include "UI/Core/WindowSystem.h"
#include "Core/Utilities/Log/muConsoleDebug.h"
#include "Character/CharacterManager.h"
#include "GameLogic/Skills/SkillManager.h"
#include "GameLogic/Social/PartyManager.h"
#include "World/MapInfra/MapManager.h"
#include "Network/Server/WSclient.h"
#include "Scenes/SceneCore.h"

#include "MuHelper.h"

constexpr int MAX_ACTIONABLE_DISTANCE = 10;
constexpr int DEFAULT_DURABILITY_THRESHOLD = 50;
constexpr auto PICKUP_RETRY_DELAY = std::chrono::seconds(1);
constexpr auto PICKUP_REQUEST_RETRY_DELAY = std::chrono::seconds(3);
constexpr int MAX_PICKUP_REQUESTS = 3;
constexpr auto TARGET_RETRY_DELAY = std::chrono::seconds(2);

SpinLock _targetsLock;
SpinLock _itemsLock;

// Movement/target globals are defined in ZzzInterface.cpp.
extern MovementSkill g_MovementSkill;
extern int SelectedCharacter;
extern int TargetX;
extern int TargetY;

namespace MUHelper
{
	MovementSkill& g_MovementSkill = ::g_MovementSkill;
	int& SelectedCharacter = ::SelectedCharacter;
	int& TargetX = ::TargetX;
	int& TargetY = ::TargetY;

    CMuHelper g_MuHelper;

    void CALLBACK CMuHelper::TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
    {
        g_MuHelper.Tick(std::chrono::steady_clock::now());
    }

    void CMuHelper::Save(const ConfigData& config)
    {
        Load(config);

        PRECEIVE_MUHELPER_DATA netData;
        ConfigDataSerDe::Serialize(m_config, netData);

        SocketClient->ToGameServer()->SendMuHelperSaveDataRequest(reinterpret_cast<BYTE*>(&netData), sizeof(netData));
    }

    void CMuHelper::Load(const ConfigData& config)
    {
        m_attackSkills.Reset();
        m_config = config;
        m_iHuntingDistance = ComputeDistanceByRange(m_config.iHuntingRange);
        m_iObtainingDistance = ComputeDistanceByRange(m_config.iObtainingRange);
    }

    ConfigData CMuHelper::GetConfig() const {
        return m_config;
    }

    void CMuHelper::Toggle()
    {
        if (m_bActive)
        {
            TriggerStop();

            // Stop the client-driven bot immediately instead of waiting for the
            // server's status reply. After an auto-reconnect the server's new
            // session doesn't have the helper marked active, so it never replies
            // and the bot would otherwise keep running with no way to stop it.
            Stop();
        }
        else
        {
            TriggerStart();
        }
    }

    void CMuHelper::TriggerStart()
    {
        if (!Hero->SafeZone)
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

        m_attackSkills.Reset();
        m_iTotalCost = 0;
        m_iComboState = 0;
        m_iCurrentBuffIndex = 0;
        m_iCurrentBuffPartyIndex = 0;
        m_iCurrentHealPartyIndex = 0;
        m_iCurrentTarget = -1;
        m_iCurrentSkill = (ActionSkillType)m_config.aiSkill[0];
        m_iCurrentItem = MAX_ITEMS;
        m_itemRetryAt.clear();
        m_itemPickupRequests.clear();
        m_targetRetryAt.clear();
        m_posOriginal = { Hero->PositionX, Hero->PositionY };

        m_iSecondsElapsed = 0;
        m_iSecondsAway = 0;
        m_now = std::chrono::steady_clock::now();
        m_nextSecondAt = m_now + std::chrono::seconds(1);
        m_iNextBuffCastAt = 0;

        m_bTimerActivatedBuffOngoing = false;
        m_bPetActivated = false;

        m_bActive = true;

        // Monsters already in view sent their packets before the helper listened.
        for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
        {
            CHARACTER& character = CharactersClient[i];
            if (character.Object.Live && character.Dead == 0 && IsMonster(&character))
            {
                AddTarget(character.Key, false);
            }
        }

        g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Started");
    }

    void CMuHelper::Stop()
    {
        m_bActive = false;
        g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Stopped");
    }

    void CMuHelper::EndSession()
    {
        Stop();
        DeleteAllTargets();
        _itemsLock.lock();
        m_setItems.clear();
        m_itemRetryAt.clear();
        m_itemPickupRequests.clear();
        _itemsLock.unlock();
        m_targetRetryAt.clear();
        m_iCurrentTarget = -1;
        m_iCurrentItem = MAX_ITEMS;
    }

    void CMuHelper::Tick(std::chrono::steady_clock::time_point now)
    {
        if (!m_bActive || Hero == nullptr || SceneFlag != MAIN_SCENE)
        {
            return;
        }

        m_now = now;

        if (Hero->SafeZone)
        {
            g_ConsoleDebug->Write(MCD_NORMAL, L"[MU Helper] Entered safezone. Stopping.");
            TriggerStop();
            return;
        }

        // Helper seconds follow the clock; timer callbacks run late or not at all when frames are slow.
        const bool bAway = ComputeDistanceBetween({Hero->PositionX, Hero->PositionY}, m_posOriginal) > 1;
        while (now >= m_nextSecondAt)
        {
            m_iSecondsElapsed++;
            m_iSecondsAway = bAway ? m_iSecondsAway + 1 : 0;
            m_nextSecondAt += std::chrono::seconds(1);
        }

        Work();
    }

    void CMuHelper::Work()
    {
        try
        {
            if (!ActivatePet())
            {
                return;
            }

            // Emergency potions never wait behind actions that can.
            ConsumePotion();

            if (!Buff())
            {
                return;
            }

            if (!RecoverHealth())
            {
                return;
            }

            // A drop already walked to is picked up before regrouping; otherwise a short return delay turns
            // every loot walk into a round trip. Regrouping still comes before choosing more loot.
            if (m_iCurrentItem != MAX_ITEMS && !ObtainItem())
            {
                return;
            }

            if (!Regroup())
            {
                return;
            }

            if (!ObtainItem())
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

        if ((iDistance > m_iHuntingDistance) && !(bIsAttacking && m_config.bLongRangeCounterAttack))
        {
            return;
        }

        _targetsLock.lock();

        m_setTargets.insert(iTargetId);

        if (bIsAttacking)
        {
            m_setTargetsAttacking.insert(iTargetId);
        }

        _targetsLock.unlock();

        // Only an attack on the hero may redirect combat; a sighting must not.
        if (bIsAttacking && m_config.bUseSelfDefense && IsMonster(pTarget) && !IsTargetCoolingDown(iTargetId))
        {
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

    // An unreachable monster is re-added by its next packet; the cooldown stops the nearest-target pick from
    // choosing it again on every tick while reachable ones wait.
    void CMuHelper::ForgetTarget(int iTargetId, bool bUnreachable)
    {
        DeleteTarget(iTargetId);
        if (bUnreachable)
        {
            m_targetRetryAt[iTargetId] = m_now + TARGET_RETRY_DELAY;
        }
    }

    bool CMuHelper::IsTargetCoolingDown(int iTargetId)
    {
        const auto retry = m_targetRetryAt.find(iTargetId);
        if (retry == m_targetRetryAt.end())
        {
            return false;
        }
        if (m_now < retry->second)
        {
            return true;
        }
        m_targetRetryAt.erase(retry);
        return false;
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
        const POINT posHero = { Hero->PositionX, Hero->PositionY };

        const POINT posCurrent = { pTarget->PositionX, pTarget->PositionY };
        const POINT posNext    = { pTarget->TargetX,   pTarget->TargetY };

        return std::min(
            ComputeDistanceBetween(posHero, posCurrent),
            ComputeDistanceBetween(posHero, posNext)
        );
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
        int iMinDistance = m_iHuntingDistance;
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

            if (!IsMonster(pTarget) || IsTargetCoolingDown(iMonsterId))
            {
                continue;
            }

            int iDistance = ComputeDistanceFromTarget(pTarget);
            if (iDistance <= iMinDistance)
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

            if (!IsMonster(pTarget) || IsTargetCoolingDown(iMonsterId))
            {
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

    void CMuHelper::CleanupTargets()
    {
        std::set<int> setTargets;
        {
            _targetsLock.lock();
            setTargets = m_setTargets;
            _targetsLock.unlock();
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
            if (pTarget->Dead > 0 || !pTarget->Object.Live)
            {
                DeleteTarget(iMonsterId);
            }
        }
    }

    // Out-of-range monsters stay tracked: one standing still sends no packet that would add it back.
    bool CMuHelper::IsWithinHuntingRange(int iTargetId)
    {
        CHARACTER* pTarget = FindCharacterByKey(iTargetId);
        if (pTarget == nullptr)
        {
            return false;
        }

        const bool bCounterAttack = m_config.bLongRangeCounterAttack && m_setTargetsAttacking.count(iTargetId) > 0;
        return bCounterAttack || ComputeDistanceFromTarget(pTarget) <= m_iHuntingDistance;
    }

    int CMuHelper::ActivatePet()
    {
        if (!m_config.bUseDarkRaven)
        {
            return 1;
        }

        if (m_bPetActivated)
        {
            return 1;
        }

        if (m_config.iDarkRavenMode == PET_ATTACK_CEASE)
        {
            SocketClient->ToGameServer()->SendPetCommandRequest(PetType::DarkRaven, PetCommandMode::Normal, 0xFFFF);
        }
        else if (m_config.iDarkRavenMode == PET_ATTACK_AUTO)
        {
            SocketClient->ToGameServer()->SendPetCommandRequest(PetType::DarkRaven, PetCommandMode::AttackRandom, 0xFFFF);
        }
        else if (m_config.iDarkRavenMode == PET_ATTACK_TOGETHER)
        {
            SocketClient->ToGameServer()->SendPetCommandRequest(PetType::DarkRaven, PetCommandMode::AttackWithOwner, 0xFFFF);
        }

        m_bPetActivated = true;
        return 1;
    }

    int CMuHelper::Buff()
    {
        if (!HasAssignedBuffSkill())
        {
            return 1;
        }

        if (m_config.bSupportParty && g_pPartyManager->IsPartyActive())
        {
            m_iCurrentBuffPartyIndex %= PartyNumber;

            PARTY_t* pMember = &Party[m_iCurrentBuffPartyIndex];
            CHARACTER* pChar = g_pPartyManager->GetPartyMemberChar(pMember);

            if (pChar != NULL
                && ComputeDistanceFromTarget(pChar) <= MAX_ACTIONABLE_DISTANCE)
            {
                StartTimedBuffRefresh(m_config.bBuffDurationParty);

                // A cast still under way keeps this member and buff for the next tick.
                if (!BuffTarget(pChar, (ActionSkillType)m_config.aiBuff[m_iCurrentBuffIndex]))
                {
                    return 0;
                }
            }

            m_iCurrentBuffPartyIndex = (m_iCurrentBuffPartyIndex + 1) % PartyNumber;
        }
        else
        {
            m_iCurrentBuffPartyIndex = 0;

            StartTimedBuffRefresh(m_config.bBuffDuration);

            if (!BuffTarget(Hero, (ActionSkillType)m_config.aiBuff[m_iCurrentBuffIndex]))
            {
                return 0;
            }
        }

        if (m_iCurrentBuffPartyIndex == 0)
        {
            m_iCurrentBuffIndex = (m_iCurrentBuffIndex + 1) % m_config.aiBuff.size();

            // Everyone has had every buff: a scheduled refresh is complete, and the next one counts from now.
            if (m_iCurrentBuffIndex == 0 && m_bTimerActivatedBuffOngoing)
            {
                m_bTimerActivatedBuffOngoing = false;
                m_iNextBuffCastAt = m_iSecondsElapsed + m_config.iBuffCastInterval;
            }
        }

        return 1;
    }

    void CMuHelper::StartTimedBuffRefresh(bool bRefreshOnExpiryOnly)
    {
        if (!bRefreshOnExpiryOnly && m_config.iBuffCastInterval != 0 && !m_bTimerActivatedBuffOngoing &&
            m_iSecondsElapsed >= m_iNextBuffCastAt)
        {
            m_bTimerActivatedBuffOngoing = true;
        }
    }

    int CMuHelper::BuffTarget(CHARACTER* pTargetChar, ActionSkillType iBuffSkill)
    {
        OBJECT* obj = &pTargetChar->Object;

        auto CastIfMissing = [&](bool bBuffActive, bool bTimerRespected, bool bNeedsTarget) -> int
        {
            if (!bBuffActive || (bTimerRespected && m_bTimerActivatedBuffOngoing))
                return SimulateSupportSkill(iBuffSkill, bNeedsTarget, pTargetChar->Key);
            return 1;
        };

        switch (iBuffSkill)
        {
        case AT_SKILL_ATTACK:
        case AT_SKILL_ATTACK_STR:
            return CastIfMissing(g_isCharacterBuff(obj, eBuff_Attack), true, true);

        case AT_SKILL_DEFENSE:
        case AT_SKILL_DEFENSE_STR:
        case AT_SKILL_DEFENSE_MASTERY:
            return CastIfMissing(g_isCharacterBuff(obj, eBuff_Defense), true, true);

        case AT_SKILL_INFINITY_ARROW:
        case AT_SKILL_INFINITY_ARROW_STR:
            return CastIfMissing(g_isCharacterBuff(obj, eBuff_InfinityArrow), false, false);

        case AT_SKILL_SOUL_BARRIER:
        case AT_SKILL_SOUL_BARRIER_STR:
        case AT_SKILL_SOUL_BARRIER_PROFICIENCY:
            return CastIfMissing(g_isCharacterBuff(obj, eBuff_WizDefense), true, true);

        case AT_SKILL_SWELL_LIFE:
        case AT_SKILL_SWELL_LIFE_STR:
        case AT_SKILL_SWELL_LIFE_PROFICIENCY:
            if (m_iComboState == 2)
            {
                return 1;
            }
            return CastIfMissing(g_isCharacterBuff(obj, eBuff_Life), true, false);

        case AT_SKILL_EXPANSION_OF_WIZARDRY:
        case AT_SKILL_EXPANSION_OF_WIZARDRY_STR:
        case AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY:
            return CastIfMissing(g_isCharacterBuff(obj, eBuff_SwellOfMagicPower), false, false);

        case AT_SKILL_ADD_CRITICAL:
        case AT_SKILL_ADD_CRITICAL_STR1:
        case AT_SKILL_ADD_CRITICAL_STR2:
        case AT_SKILL_ADD_CRITICAL_STR3:
            return CastIfMissing(g_isCharacterBuff(obj, eBuff_AddCriticalDamage), false, false);

        case AT_SKILL_ALICE_BERSERKER:
        case AT_SKILL_ALICE_BERSERKER_STR:
            return CastIfMissing(g_isCharacterBuff(obj, eBuff_Berserker), false, false);

        case AT_SKILL_ALICE_THORNS:
            return CastIfMissing(g_isCharacterBuff(obj, eBuff_Thorns), false, false);

        // Rage Fighter party buffs — self/party AoE, no explicit target needed.
        case AT_SKILL_ATT_UP_OURFORCES:
            return CastIfMissing(g_isCharacterBuff(obj, eBuff_Att_up_Ourforces), true, false);

        case AT_SKILL_HP_UP_OURFORCES:
        case AT_SKILL_HP_UP_OURFORCES_STR:
            return CastIfMissing(g_isCharacterBuff(obj, eBuff_Hp_up_Ourforces), true, false);

        case AT_SKILL_DEF_UP_OURFORCES:
        case AT_SKILL_DEF_UP_OURFORCES_STR:
        case AT_SKILL_DEF_UP_OURFORCES_MASTERY:
            return CastIfMissing(g_isCharacterBuff(obj, eBuff_Def_up_Ourforces), true, false);

        default:
            return 1;
        }
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

    int CMuHelper::RecoverHealth()
    {
        if (!Heal())
        {
            return 0;
        }
        
        if (!DrainLife())
        {
            return 0;
        }

        return 1;
    }

    int CMuHelper::Heal()
    {
        if (!m_config.bAutoHeal)
        {
            return 1;
        }

        auto iHealingSkill = GetHealingSkill();
        if (iHealingSkill == AT_SKILL_UNDEFINED)
        {
            return 1;
        }

        if (m_config.bAutoHealParty && g_pPartyManager->IsPartyActive())
        {
            m_iCurrentHealPartyIndex %= PartyNumber;

            PARTY_t* pMember = &Party[m_iCurrentHealPartyIndex];
            CHARACTER* pChar = g_pPartyManager->GetPartyMemberChar(pMember);
            int iHealResult = 1;

            if (pChar != NULL)
            {
                if (pChar == Hero)
                {
                    iHealResult = HealSelf(iHealingSkill);
                }
                else if (pMember->stepHP * 10 <= m_config.iHealPartyThreshold
                    && ComputeDistanceFromTarget(pChar) <= MAX_ACTIONABLE_DISTANCE)
                {
                    iHealResult = SimulateSupportSkill(iHealingSkill, true, pChar->Key);
                }
            }

            m_iCurrentHealPartyIndex = (m_iCurrentHealPartyIndex + 1) % PartyNumber;

            return iHealResult;
        }
        else
        {
            m_iCurrentHealPartyIndex = 0;
            return HealSelf(iHealingSkill);
        }

        return 1;
    }

    int CMuHelper::HealSelf(ActionSkillType iHealingSkill)
    {
        int64_t iLife = CharacterAttribute->Life;
        int64_t iLifeMax = CharacterAttribute->LifeMax;
        if (iLifeMax <= 0)
        {
            return 1;
        }
        int64_t iRemaining = (iLife * 100 + iLifeMax - 1) / iLifeMax;

        if (iRemaining <= m_config.iHealThreshold)
        {
            return SimulateSupportSkill(iHealingSkill, true, HeroKey);
        }

        return 1;
    }

    int CMuHelper::DrainLife()
    {
        if (!m_config.bUseDrainLife)
        {
            return 1;
        }

        auto iDrainLife = GetDrainLifeSkill();
        if (iDrainLife == AT_SKILL_UNDEFINED)
        {
            return 1;
        }

        int64_t iLife = CharacterAttribute->Life;
        int64_t iLifeMax = CharacterAttribute->LifeMax;
        if (iLifeMax <= 0)
        {
            return 1;
        }
        int64_t iRemaining = (iLife * 100 + iLifeMax - 1) / iLifeMax;

        if (iRemaining <= m_config.iHealThreshold)
        {
            m_iCurrentTarget = GetNearestTarget();
            if (m_iCurrentTarget != -1)
            {
                return SimulateSupportSkill(iDrainLife, true, m_iCurrentTarget);
            }
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
                if (!pItem || pItem->Type == -1 || GameLogic::Items::IsRepairBan(pItem))
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
                if (iMaxDurability <= 0)
                {
                    continue;
                }

                int64_t iHealth = (iDurability * 100 + iMaxDurability - 1) / iMaxDurability;

                if (iHealth <= DEFAULT_DURABILITY_THRESHOLD)
                {
                    int64_t iGoldCost = CalcSelfRepairCost(ItemValue(pItem, 2), iDurability, iMaxDurability, pItem->Type);
                    if (iGoldCost <= CharacterMachine->Gold)
                    {
                        SocketClient->ToGameServer()->SendRepairItemRequest(i, 1);
                    }
                }
            }
        }

        return 1;
    }

    bool CMuHelper::SelectAttackTarget()
    {
        // Condition counts read these sets, so they are refreshed even while the current target lives.
        CleanupTargets();
        if (m_iCurrentTarget != -1)
        {
            const auto* target = FindCharacterByKey(m_iCurrentTarget);
            if (target == nullptr || !target->Object.Live || target->Dead > 0)
                DeleteTarget(m_iCurrentTarget);
            else if (IsWithinHuntingRange(m_iCurrentTarget))
                return true;
            else
                m_iCurrentTarget = -1; // Still tracked, but no longer chased out of hunting range.
        }
        if (m_config.bLongRangeCounterAttack)
            m_iCurrentTarget = GetFarthestAttackingTarget();
        if (m_iCurrentTarget == -1)
            m_iCurrentTarget = GetNearestTarget();
        return m_iCurrentTarget != -1;
    }

    int CMuHelper::Attack()
    {
        if (!SelectAttackTarget())
        {
            if (m_setTargets.empty())
                m_iComboState = 0;
            return 0;
        }

        if (m_config.bUseCombo)
        {
            return SimulateComboAttack();
        }

        m_iCurrentSkill = SelectAttackSkill();
        if (m_iCurrentSkill > AT_SKILL_UNDEFINED)
        {
            const float fSkillDistance = gSkillManager.GetSkillDistance(m_iCurrentSkill, Hero);
            if (GameLogic::Combat::CanExecuteSkill(Hero, m_iCurrentSkill, fSkillDistance))
            {
                const auto result = CastSkill(m_iCurrentSkill, true, m_iCurrentTarget);
                m_attackSkills.Complete(result);
                return result == GameLogic::Automation::SkillResult::Cast ? 1 : 0;
            }
            m_attackSkills.Complete(GameLogic::Automation::SkillResult::Refused);
        }

        if (m_config.bFallbackBasicAttack)
        {
            if (!Hero->Movement)
            {
                return SimulateBasicAttack(m_iCurrentTarget);
            }
        }

        return 1;
    }

    ActionSkillType CMuHelper::SelectAttackSkill()
    {
        const auto inRange = [this](int iTargetId) { return IsWithinHuntingRange(iTargetId); };
        const auto nearby = std::count_if(m_setTargets.begin(), m_setTargets.end(), inRange);
        const auto attacking = std::count_if(m_setTargetsAttacking.begin(), m_setTargetsAttacking.end(), inRange);
        return m_attackSkills.Select(m_config, m_iSecondsElapsed, static_cast<std::size_t>(nearby),
                                     static_cast<std::size_t>(attacking));
    }

    int CMuHelper::SimulateComboAttack()
    {
        for (int i = 0; i < m_config.aiSkill.size(); i++)
        {
            if (m_config.aiSkill[i] <= AT_SKILL_UNDEFINED || m_config.aiSkill[i] >= MAX_SKILLS)
            {
                return 0;
            }
        }

        if (SimulateAttack((ActionSkillType)m_config.aiSkill[m_iComboState]))
        {
            m_iComboState = (m_iComboState + 1) % 3;
        }

        return 1;
    }

    int CMuHelper::SimulateAttack(ActionSkillType iSkill)
    {
        return SimulateSkill(iSkill, true, m_iCurrentTarget);
    }

    int CMuHelper::SimulateSkill(ActionSkillType iSkill, bool bTargetRequired, int iTarget)
    {
        return CastSkill(iSkill, bTargetRequired, iTarget) == GameLogic::Automation::SkillResult::Cast ? 1 : 0;
    }

    int CMuHelper::SimulateSupportSkill(ActionSkillType iSkill, bool bTargetRequired, int iTarget)
    {
        using GameLogic::Automation::SkillResult;

        // Only a cast under way holds up the actions after it; one that cannot happen must not starve them.
        const SkillResult result = CastSkill(iSkill, bTargetRequired, iTarget);
        return result == SkillResult::Busy || result == SkillResult::Approaching ? 0 : 1;
    }

    GameLogic::Automation::SkillResult CMuHelper::CastSkill(ActionSkillType iSkill, bool bTargetRequired, int iTarget)
    {
        using GameLogic::Automation::SkillResult;

        // Players are allowed here, and must be: the helper casts its buffs and
        // heals on party members and on the hero itself. Only the plain attack
        // is monster-only, as it always was. A target that is not attackable is
        // still refused by the primitive when it is dead.
        const SkillResult result =
            GameLogic::Automation::CastSkill(iSkill, bTargetRequired, iTarget, true, m_iHuntingDistance);

        if (iTarget != -1 && GameLogic::Automation::ShouldForgetTarget(result))
        {
            ForgetTarget(iTarget, result == SkillResult::NoPath || result == SkillResult::Blocked);
        }

        return result;
    }

    int CMuHelper::SimulateBasicAttack(int iTarget)
    {
        using GameLogic::Automation::AttackResult;

        // No target is not a failed attack: without this the helper asks the
        // automation for object -1 and then forgets a target it never had.
        if (iTarget == -1)
        {
            return 0;
        }

        const AttackResult result = GameLogic::Automation::AttackObject(iTarget, false, m_iHuntingDistance);

        if (GameLogic::Automation::ShouldForgetTarget(result))
        {
            ForgetTarget(iTarget, result == AttackResult::NoPath || result == AttackResult::Blocked);
        }

        return result == AttackResult::Attacked ? 1 : 0;
    }

    int CMuHelper::Regroup()
    {
        if (!m_config.bReturnToOriginalPosition || m_iSecondsAway <= m_config.iMaxSecondsAway)
            return 1;

        using GameLogic::Automation::MoveResult;
        const MoveResult result = GameLogic::Automation::WalkTo(m_posOriginal.x, m_posOriginal.y);
        if (result == MoveResult::Walking)
            return 0;
        if (result == MoveResult::NoPath)
            return 1;

        m_iSecondsAway = 0;
        m_iComboState = 0;
        m_iCurrentTarget = -1;

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

    ActionSkillType CMuHelper::GetHealingSkill()
    {
        std::vector<ActionSkillType> aiHealingSkills =
        {
            AT_SKILL_HEALING,
            AT_SKILL_HEALING_STR,
        };

        for (int i = 0; i < aiHealingSkills.size(); i++)
        {
            int iSkillIndex = g_pSkillList->GetSkillIndex(aiHealingSkills[i]);
            if (iSkillIndex != -1)
            {
                return aiHealingSkills[i];
            }
        }

        return AT_SKILL_UNDEFINED;
    }

    bool CMuHelper::IsSelfPositionSkill(ActionSkillType iSkill)
    {
        return GameLogic::Automation::IsSelfPositionSkill(iSkill);
    }

    ActionSkillType CMuHelper::GetDrainLifeSkill()
    {
        std::vector<ActionSkillType> aiDrainLifeSkills =
        {
            AT_SKILL_ALICE_DRAINLIFE,
            AT_SKILL_ALICE_DRAINLIFE_STR
        };

        for (int i = 0; i < aiDrainLifeSkills.size(); i++)
        {
            int iSkillIndex = g_pSkillList->GetSkillIndex(aiDrainLifeSkills[i]);
            if (iSkillIndex != -1)
            {
                return aiDrainLifeSkills[i];
            }
        }

        return AT_SKILL_UNDEFINED;
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

        using GameLogic::Automation::PickupResult;
        const PickupResult result = GameLogic::Automation::PickUpItem(m_iCurrentItem, m_iObtainingDistance);

        switch (result)
        {
        case PickupResult::Requested:
        {
            // The server's removal packet forgets the drop. A refused one (full inventory, someone else's
            // loot) is retried with a doubling delay, then given up so it cannot keep pulling the hero back.
            const int iRequests = ++m_itemPickupRequests[m_iCurrentItem];
            if (iRequests >= MAX_PICKUP_REQUESTS)
            {
                DeleteItem(m_iCurrentItem);
                return 1;
            }
            m_itemRetryAt[m_iCurrentItem] = m_now + PICKUP_REQUEST_RETRY_DELAY * (1 << (iRequests - 1));
            m_iCurrentItem = MAX_ITEMS;
            return 1;
        }
        case PickupResult::Gone:
            DeleteItem(m_iCurrentItem);
            return 1;
        case PickupResult::NoPath:
            m_itemRetryAt[m_iCurrentItem] = m_now + PICKUP_RETRY_DELAY;
            m_iCurrentItem = MAX_ITEMS;
            return 1;
        case PickupResult::Approaching:
            // Walking towards it: the helper's other work waits a frame.
            return 0;
        case PickupResult::TooFar:
            m_iCurrentItem = MAX_ITEMS;
            return 1;
        case PickupResult::Busy:
            return 1;
        }

        return 1;
    }

    bool CMuHelper::ShouldObtainItem(int iItemId, std::chrono::steady_clock::time_point now)
    {
        const auto retry = m_itemRetryAt.find(iItemId);
        if (retry != m_itemRetryAt.end() && now < retry->second)
            return false;

        return GameLogic::Automation::ShouldPickUpHelperItem(m_config, &Items[iItemId].Item);
    }

    void CMuHelper::AddItem(int iItemId, POINT posWhere)
    {
        _itemsLock.lock();
        m_setItems.insert(iItemId);
        m_itemRetryAt.erase(iItemId);
        m_itemPickupRequests.erase(iItemId);
        _itemsLock.unlock();
    }

    void CMuHelper::DeleteItem(int iItemId)
    {
        _itemsLock.lock();
        m_setItems.erase(iItemId);
        m_itemRetryAt.erase(iItemId);
        m_itemPickupRequests.erase(iItemId);
        _itemsLock.unlock();

        if (iItemId == m_iCurrentItem)
        {
            m_iCurrentItem = MAX_ITEMS;
        }
    }

    int CMuHelper::SelectItemToObtain()
    {
        int iClosestItemId = MAX_ITEMS;
        int iMinDistance = m_config.iObtainingRange;

        std::set<int> setItems;
        {
            _itemsLock.lock();
            setItems = m_setItems;
            _itemsLock.unlock();
        }

        for (const int& iItemId : setItems)
        {
            if (!ShouldObtainItem(iItemId, m_now))
            {
                continue;
            }

            int iItemX = (int)(Items[iItemId].Object.Position[0] / TERRAIN_SCALE);
            int iItemY = (int)(Items[iItemId].Object.Position[1] / TERRAIN_SCALE);

            int iDistance = ComputeDistanceBetween({ Hero->PositionX, Hero->PositionY }, { iItemX, iItemY });
            if (iDistance <= iMinDistance)
            {
                iMinDistance = iDistance;
                iClosestItemId = iItemId;
            }
        }

        return iClosestItemId;
    }
}
