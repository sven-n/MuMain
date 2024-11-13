#pragma once

#include <set>
#include <array>
#include <string>
#include "WSclient.h"

#define MUHELPER_ATTACK_ALWAYS                   0x00000000
#define MUHELPER_ATTACK_ON_TIMER                 0x00000001
#define MUHELPER_ATTACK_ON_CONDITION             0x00000002
#define MUHELPER_ATTACK_ON_MOBS_NEARBY           0x00000004
#define MUHELPER_ATTACK_ON_MOBS_ATTACKING        0x00000008

#define MUHELPER_ATTACK_PRECONDITION_CLEAR_MASK  \
    ~(MUHELPER_ATTACK_ON_MOBS_NEARBY | MUHELPER_ATTACK_ON_MOBS_ATTACKING)

#define MUHELPER_ATTACK_ON_MORE_THAN_TWO_MOBS    0x00000010
#define MUHELPER_ATTACK_ON_MORE_THAN_THREE_MOBS  0x00000020
#define MUHELPER_ATTACK_ON_MORE_THAN_FOUR_MOBS   0x00000040
#define MUHELPER_ATTACK_ON_MORE_THAN_FIVE_MOBS   0x00000080

#define MUHELPER_ATTACK_ON_MOBS_CLEAR_MASK  \
    ~(MUHELPER_ATTACK_ON_MORE_THAN_TWO_MOBS | MUHELPER_ATTACK_ON_MORE_THAN_THREE_MOBS | \
      MUHELPER_ATTACK_ON_MORE_THAN_FOUR_MOBS | MUHELPER_ATTACK_ON_MORE_THAN_FIVE_MOBS)

#define PET_ATTACK_CEASE    0x00
#define PET_ATTACK_AUTO     0x01
#define PET_ATTACK_TOGETHER 0x02

typedef struct
{
	int iHuntingRange = 0;

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
	int iBuffCastInterval = 0;

	bool bAutoHeal = false;
	int iHealThreshold = 0;
	bool bSupportParty = false;
	bool bAutoHealParty = false;
	int iHealPartyThreshold = 0;

	bool bUseHealPotion = false;
	int iPotionThreshold = 0;

	bool bUseDrainLife = false;
	bool bUseDarkRaven = false;
	int iDarkRavenMode = 0;

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

	bool bUseSelfDefense = false;
	bool bAutoAcceptFriend = false;
	bool bAutoAcceptGuild = false;
} cMuHelperConfig;

class MuHelperConfigSerDe {
public:
	static void Serialize(const cMuHelperConfig& domain, PRECEIVE_MUHELPER_DATA& packet);
	static void Deserialize(const PRECEIVE_MUHELPER_DATA& packet, cMuHelperConfig& domain);
};
