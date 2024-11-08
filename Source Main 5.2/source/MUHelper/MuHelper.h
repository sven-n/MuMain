#pragma once

#include <array>
#include <set>
#include <string>
#include <thread>
#include <atomic>

typedef enum
{
	eDefault = 0,
	eDelayElapsed,
	eTwoOrMoreWithinRange,
	eThreeOrMoreWithinRange,
	eFourOrMoreWithinRange,
	eFiveOrMoreWithinRange,
	eTwoOrMoreAttacking,
	eThreeOrMoreAttacking,
	eFourOrMoreAttacking,
	eFiveOrMoreAttacking,
} eSkillActivationType;

typedef struct
{
	int iHuntingRange = 0;

	bool bUseSelfDefense = false;
	bool bLongRangeCounterAttack = false;
	bool bReturnToOriginalPosition = false;
	int iMaxSecondsAway = 0;

	std::array<int, 3> aiSkill = { 0, 0, 0 };
	std::array<int, 3> aiSkillCondition = { 0, 0, 0 };
	std::array<int, 3> aiSkillInterval = { 0, 0, 0 };

	bool bUseCombo = false;

	std::array<int, 3> aiBuff = { 0, 0, 0 };

	bool bBuffDuration = false;
	bool bBuffDurationParty = false;
	int iCastInterval = 0;

	bool bSupportParty = false;
	bool bAutoHeal = false;
	int iHealThreshold = 0;

	bool bUseHealPotion = false;
	int iPotionThreshold = 0;

	bool bUseDrainLife = false;
	bool bUsePet = false;

	bool bRepairItem = false;

	int iObtainingRange = 0;
	bool bPickAllItems = false;
	bool bPickSelectItems = false;
	bool bPickJewel = false;
	bool bPickZen = false;
	bool bPickAncient = false;
	bool bPickExcellent = false;
	bool bPickExtraItems = false;
	std::set<std::wstring> aExtraItems;
} cMuHelperConfig;

class CMuHelper
{
public:
	CMuHelper();
	~CMuHelper() = default;

public:
	void Save(const cMuHelperConfig& config);
	void Load(const cMuHelperConfig& config);
	void Start();
	void Stop();
	void Toggle();
	bool IsActive() { return m_bActive; }

	void AddTarget(int iTargetId, bool bIsAttacking);
	void DeleteTarget(int iTargetId);
	void DeleteAllTargets();

	void AddItem(int iItemId, POINT posDropped);
	void DeleteItem(int iItemId);

private:
	void WorkLoop();
	void Work();
	int Buff();
	int BuffTarget(CHARACTER* pTargetChar, int iBuffSkill);
	int Heal();
	int ConsumePotion();
	int Attack();
	int ObtainItem();
	int RepairEquipments();
	int Regroup();
	int SimulateAttack(int iSkill);
	int SimulateSkill(int iSkill, bool bTargetRequired, int iTarget);
	int SimulateComboAttack();
	int GetNearestTarget();
	int GetFarthestTarget();
	int GetNearestItem();
	int ComputeDistanceByRange(int iRange);
	int ComputeDistanceFromTarget(CHARACTER* pTarget);
	int ComputeDistanceBetween(POINT posA, POINT posB);
	int SimulateMove(POINT posMove);
	int GetHealingSkill();
	bool HasAssignedBuffSkill();

private:
	cMuHelperConfig m_config;
	POINT m_posOriginal;
	std::thread m_timerThread;
	std::atomic<bool> m_bActive;
	std::set<int> m_setTargets;
	std::set<int> m_setItems;
	int m_iCurrentTarget;
	int m_iCurrentBuffIndex;
	int m_iCurrentBuffPartyIndex;
	int m_iCurrentHealPartyIndex;
	int m_iComboState;
	int m_iHuntingDistance;
	int m_iObtainingDistance;
	int m_iSecondsElapsed;
	int m_iSecondsAway;
};

extern CMuHelper g_MuHelper;