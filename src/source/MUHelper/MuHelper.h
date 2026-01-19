#pragma once

#include <functional>
#include <array>
#include <set>
#include <string>
#include <thread>
#include <atomic>

#include "MuHelperData.h"

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
		void Toggle();
		void TriggerStart();
		void TriggerStop();
		bool IsActive() { return m_bActive; }
		void AddCost(int iCost) { m_iTotalCost += iCost; }
		int GetTotalCost() { return m_iTotalCost; }

		void AddTarget(int iTargetId, bool bIsAttacking);
		void DeleteTarget(int iTargetId);
		void DeleteAllTargets();

		void AddItem(int iItemId, POINT posDropped);
		void DeleteItem(int iItemId);

	private:
		void WorkLoop(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
		void Work();
		int ActivatePet();
		int Buff();
		int BuffTarget(CHARACTER* pTargetChar, ActionSkillType iBuffSkill);
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
		int SimulateComboAttack();
		int GetNearestTarget();
		int GetFarthestAttackingTarget();
		void CleanupTargets();
		int ComputeDistanceByRange(int iRange);
		int ComputeDistanceFromTarget(CHARACTER* pTarget);
		int ComputeDistanceBetween(POINT posA, POINT posB);
		int SimulateMove(POINT posMove);
		int ObtainItem();
		int SelectItemToObtain();
		bool ShouldObtainItem(int iItemId);
		ActionSkillType GetHealingSkill();
		ActionSkillType GetDrainLifeSkill();
		bool HasAssignedBuffSkill();

	private:
		ConfigData m_config;
		POINT m_posOriginal;
		std::thread m_timerThread;
		std::atomic<bool> m_bActive;
		std::set<int> m_setTargets;
		std::set<int> m_setTargetsAttacking;
		std::set<int> m_setItems;
		int m_iCurrentItem;
		int m_iCurrentTarget;
		int m_iCurrentBuffIndex;
		int m_iCurrentBuffPartyIndex;
		int m_iCurrentHealPartyIndex;
		int m_iComboState;
		ActionSkillType m_iCurrentSkill;
		int m_iHuntingDistance;
		int m_iObtainingDistance;
		int m_iLoopCounter;
		int m_iSecondsElapsed;
		int m_iSecondsAway;
		bool m_bTimerActivatedBuffOngoing;
		bool m_bPetActivated;
		int m_iTotalCost;
	};

	extern CMuHelper g_MuHelper;
}