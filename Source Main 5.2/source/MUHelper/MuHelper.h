#pragma once

#include <array>
#include <set>
#include <string>
#include <thread>
#include <atomic>

typedef struct 
{
	int iHuntingRange;

	bool bLongDistanceCounterAttack;
	bool bOriginalPosition;
	int iAttackDistance;

	int iBasicSkill;

	int iSkill2;
	bool bSkill2UseCondition;
	int iSkill2ConditionType;
	bool bSkill2UseDelay;
	int iSkill2Interval;

	int iSkill3;
	bool bSkill3UseCondition;
	int iSkill3ConditionType;
	bool bSkill3UseDelay;
	int iSkill3Interval;

	bool bUseCombo;

	bool bUseSelfDefense;

	int iBuff1;
	int iBuff2;
	int iBuff3;

	bool bSupportParty;

	bool bBuffDuration;
	bool bBuffDurationParty;
	int iCastInterval;

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
	int Attack();
	int SimulateAttack(int iSkill);
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
	int m_iComboState;

	bool m_bSavedAutoAttack;
};

extern CMuHelper g_MuHelper;