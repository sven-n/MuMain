#pragma once

#include <array>
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
	CMuHelper() : m_bActive(false) {}
	~CMuHelper() = default;

public:
	void Save(const cMuHelperConfig& config);
	void Load(const cMuHelperConfig& config);
	void Start();
	void Stop();
	bool IsActive() { return m_bActive; }

private:
	void WorkLoop();
	void Work();

private:
	cMuHelperConfig m_config;
	std::thread m_timerThread;
	std::atomic<bool> m_bActive;
};

extern CMuHelper g_MuHelper;