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
	bool bOriginalPosition;
	int iAttackDistance;

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

	void AddTarget(int iTargetId);
	void DeleteTarget(int iTargetId);
	void DeleteAllTargets();

private:
	void WorkLoop();
	void Work();
	int Buff();
	int BuffTarget(CHARACTER* pTargetChar, int iBuffSkill);
	int Heal();
	int HealTarget(CHARACTER* pTargetChar);
	int ConsumePotion();
	int Attack();
	int ObtainItem();
	int RepairEquipments();
	int SimulateAttack(int iSkill);
	int SimulateSkill(int iSkill, bool bTargetRequired);
	int SimulateComboAttack();
	int GetNearestTarget();
	int GetFarthestTarget();
	int ComputeDistanceFromTarget(CHARACTER* pTarget);

private:
	cMuHelperConfig m_config;
	std::thread m_timerThread;
	std::atomic<bool> m_bActive;
	std::set<int> m_setTargets;
	int m_iCurrentTarget;
	int m_iCurrentSkill;
	int m_iCurrentBuffIndex;
	int m_iCurrentPartyMemberIndex;
	int m_iComboState;

	bool m_bSavedAutoAttack;
};

extern CMuHelper g_MuHelper;