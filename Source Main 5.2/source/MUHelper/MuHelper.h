#pragma once

#include <functional>
#include <array>
#include <set>
#include <string>
#include <thread>
#include <atomic>

#define MUHELPER_ATTACK_ALWAYS                   0x00000000
#define MUHELPER_ATTACK_ON_TIMER                 0x00000001
#define MUHELPER_ATTACK_ON_MOBS_NEARBY           0x00000002
#define MUHELPER_ATTACK_ON_MOBS_ATTACKING        0x00000004

#define MUHELPER_ATTACK_PRECONDITION_MASK  \
    ~(MUHELPER_ATTACK_ON_MOBS_NEARBY | MUHELPER_ATTACK_ON_MOBS_ATTACKING)

#define MUHELPER_ATTACK_ON_MORE_THAN_TWO_MOBS    0x00000010
#define MUHELPER_ATTACK_ON_MORE_THAN_THREE_MOBS  0x00000020
#define MUHELPER_ATTACK_ON_MORE_THAN_FOUR_MOBS   0x00000040
#define MUHELPER_ATTACK_ON_MORE_THAN_FIVE_MOBS   0x00000080

#define MUHELPER_ATTACK_ON_MOBS_MASK  \
    ~(MUHELPER_ATTACK_ON_MORE_THAN_TWO_MOBS | MUHELPER_ATTACK_ON_MORE_THAN_THREE_MOBS | \
      MUHELPER_ATTACK_ON_MORE_THAN_FOUR_MOBS | MUHELPER_ATTACK_ON_MORE_THAN_FIVE_MOBS)

typedef enum
{
	eDefault                = MUHELPER_ATTACK_ALWAYS,
	eDelayElapsed           = MUHELPER_ATTACK_ON_TIMER,
	eTwoOrMoreWithinRange   = MUHELPER_ATTACK_ON_MOBS_NEARBY | MUHELPER_ATTACK_ON_MORE_THAN_TWO_MOBS,
	eThreeOrMoreWithinRange = MUHELPER_ATTACK_ON_MOBS_NEARBY | MUHELPER_ATTACK_ON_MORE_THAN_THREE_MOBS,
	eFourOrMoreWithinRange  = MUHELPER_ATTACK_ON_MOBS_NEARBY | MUHELPER_ATTACK_ON_MORE_THAN_FOUR_MOBS,
	eFiveOrMoreWithinRange  = MUHELPER_ATTACK_ON_MOBS_NEARBY | MUHELPER_ATTACK_ON_MORE_THAN_FIVE_MOBS,
	eTwoOrMoreAttacking     = MUHELPER_ATTACK_ON_MOBS_ATTACKING | MUHELPER_ATTACK_ON_MORE_THAN_TWO_MOBS,
	eThreeOrMoreAttacking   = MUHELPER_ATTACK_ON_MOBS_ATTACKING | MUHELPER_ATTACK_ON_MORE_THAN_THREE_MOBS,
	eFourOrMoreAttacking    = MUHELPER_ATTACK_ON_MOBS_ATTACKING | MUHELPER_ATTACK_ON_MORE_THAN_FOUR_MOBS,
	eFiveOrMoreAttacking    = MUHELPER_ATTACK_ON_MOBS_ATTACKING | MUHELPER_ATTACK_ON_MORE_THAN_FIVE_MOBS,
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

	bool bAutoHeal = false;
	int iHealThreshold = 0;
	bool bSupportParty = false;
	bool bAutoHealParty = false;
	int iHealPartyThreshold = 0;

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
	static void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

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
	void WorkLoop(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	void Work();
	int Buff();
	int BuffTarget(CHARACTER* pTargetChar, int iBuffSkill);
	int Heal();
	int ConsumePotion();
	int Attack();
	int RepairEquipments();
	int Regroup();
	int SimulateAttack(int iSkill);
	int SimulateSkill(int iSkill, bool bTargetRequired, int iTarget);
	int SimulateComboAttack();
	int GetNearestTarget();
	int GetFarthestTarget();
	int ComputeDistanceByRange(int iRange);
	int ComputeDistanceFromTarget(CHARACTER* pTarget);
	int ComputeDistanceBetween(POINT posA, POINT posB);
	int SimulateMove(POINT posMove);
	int ObtainItem();
	int SelectItemToObtain();
	bool ShouldObtainItem(int iItemId);
	int GetHealingSkill();
	bool HasAssignedBuffSkill();

private:
	cMuHelperConfig m_config;
	POINT m_posOriginal;
	std::thread m_timerThread;
	std::atomic<bool> m_bActive;
	std::set<int> m_setTargets;
	std::set<int> m_setItems;
	int m_iCurrentItem;
	int m_iCurrentTarget;
	int m_iCurrentBuffIndex;
	int m_iCurrentBuffPartyIndex;
	int m_iCurrentHealPartyIndex;
	int m_iComboState;
	int m_iHuntingDistance;
	int m_iObtainingDistance;
	int m_iLoopCounter;
	int m_iSecondsElapsed;
	int m_iSecondsAway;
};

extern CMuHelper g_MuHelper;