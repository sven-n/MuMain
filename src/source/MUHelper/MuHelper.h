#pragma once

#include <functional>
#include <array>
#include <set>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <map>

#include "MuHelperData.h"
#include "GameLogic/Automation/AttackSkillRotation.h"

namespace MUHelper
{
class CMuHelper
{
public:
    CMuHelper() = default;
    ~CMuHelper() = default;

public:
    static void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

    ConfigData GetConfig() const;
    void Save(const ConfigData& config);
    void Load(const ConfigData& config);
    void Start();
    void Stop();
    // Stops locally and forgets the torn-down world's targets and drops.
    void EndSession();
    void Toggle();
    void TriggerStart();
    void TriggerStop();
    bool IsActive()
    {
        return m_bActive;
    }
    void AddCost(int iCost)
    {
        m_iTotalCost += iCost;
    }
    int GetTotalCost()
    {
        return m_iTotalCost;
    }

    void AddTarget(int iTargetId, bool bIsAttacking);
    void DeleteTarget(int iTargetId);
    void DeleteAllTargets();

    void AddItem(int iItemId, POINT posDropped);
    void DeleteItem(int iItemId);

    // One helper step at `now`; TimerProc passes the current time.
    void Tick(std::chrono::steady_clock::time_point now);

private:
    void Work();
    int ActivatePet();
    int Buff();
    int BuffTarget(CHARACTER* pTargetChar, ActionSkillType iBuffSkill);
    void StartTimedBuffRefresh(bool bRefreshOnExpiryOnly);
    int RecoverHealth();
    int Heal();
    int HealSelf(ActionSkillType iHealingSkill);
    int DrainLife();
    int ConsumePotion();
    int Attack();
    int RepairEquipments();
    int Regroup();
    ActionSkillType SelectAttackSkill();
    int SimulateAttack(ActionSkillType iSkill);
    int SimulateSkill(ActionSkillType iSkill, bool bTargetRequired, int iTarget);
    int SimulateSupportSkill(ActionSkillType iSkill, bool bTargetRequired, int iTarget);
    int SimulateBasicAttack(int iTarget);
    int SimulateComboAttack();
    int GetNearestTarget();
    int GetFarthestAttackingTarget();
    void CleanupTargets();
    bool IsWithinHuntingRange(int iTargetId);
    void ForgetTarget(int iTargetId, bool bUnreachable);
    bool IsTargetCoolingDown(int iTargetId);
    int ComputeDistanceByRange(int iRange);
    int ComputeDistanceFromTarget(CHARACTER* pTarget);
    int ComputeDistanceBetween(POINT posA, POINT posB);
    int ObtainItem();
    int SelectItemToObtain();
    bool ShouldObtainItem(int iItemId, std::chrono::steady_clock::time_point now);
    ActionSkillType GetHealingSkill();
    ActionSkillType GetDrainLifeSkill();
    bool HasAssignedBuffSkill();
    bool IsSelfPositionSkill(ActionSkillType iSkill);

private:
    ConfigData m_config;
    POINT m_posOriginal;
    std::thread m_timerThread;
    std::atomic<bool> m_bActive;
    std::set<int> m_setTargets;
    std::set<int> m_setTargetsAttacking;
    std::set<int> m_setItems;
    std::map<int, std::chrono::steady_clock::time_point> m_itemRetryAt;
    std::map<int, int> m_itemPickupRequests;
    std::map<int, std::chrono::steady_clock::time_point> m_targetRetryAt;
    int m_iCurrentItem;
    int m_iCurrentTarget;
    int m_iCurrentBuffIndex;
    int m_iCurrentBuffPartyIndex;
    int m_iCurrentHealPartyIndex;
    int m_iComboState;
    ActionSkillType m_iCurrentSkill;
    int m_iHuntingDistance;
    int m_iObtainingDistance;
    std::chrono::steady_clock::time_point m_now;
    std::chrono::steady_clock::time_point m_nextSecondAt;
    int m_iSecondsElapsed;
    int m_iNextBuffCastAt;
    int m_iSecondsAway;
    bool m_bTimerActivatedBuffOngoing;
    bool m_bPetActivated;
    int m_iTotalCost;

    GameLogic::Automation::AttackSkillRotation m_attackSkills;
    bool SelectAttackTarget();
    GameLogic::Automation::SkillResult CastSkill(ActionSkillType skill, bool targetRequired, int target);

public:
    ActionSkillType GetCurrentSkill() const
    {
        return m_iCurrentSkill;
    }
};

extern CMuHelper g_MuHelper;
} // namespace MUHelper
