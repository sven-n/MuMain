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
	int iHuntingRange;

	bool bUseSelfDefense;
	bool bLongDistanceCounterAttack;
	bool bReturnToOriginalPosition;
	int iMaxSecondsAway;

	std::array<int, 3> aiSkill;
	std::array<int, 3> aiSkillCondition;
	std::array<int, 3> aiSkillInterval;

	bool bUseCombo;

	std::array<int, 3> aiBuff;

	bool bBuffDuration;
	bool bBuffDurationParty;
	int iCastInterval;

	bool bSupportParty;
	bool bAutoHeal;
	int iHealThreshold;

	bool bUseHealPotion;
	int iPotionThreshold;

	bool bUseDrainLife;
	bool bUsePet;

	bool bRepairItem;

	int iObtainingRange;
	bool bPickAllItems;
	bool bPickSelectItems;
	bool bPickJewel;
	bool bPickZen;
	bool bPickAncient;
	bool bPickExcellent;
	bool bPickExtraItems;

	std::array<std::string, 12> aExtraItems;
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
	bool IsActive() { return m_bActive; }

	void AddTarget(int iTargetId, bool bIsAttacking);
	void DeleteTarget(int iTargetId);
	void DeleteAllTargets();

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
	int ComputeHuntingDistance();
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
	int m_iCurrentTarget;
	int m_iCurrentBuffIndex;
	int m_iCurrentBuffPartyIndex;
	int m_iCurrentHealPartyIndex;
	int m_iComboState;
	int m_iHuntingDistance;
	int m_iSecondsElapsed;
	int m_iSecondsAway;

	bool m_bSavedAutoAttack;
};

extern CMuHelper g_MuHelper;