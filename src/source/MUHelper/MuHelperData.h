#pragma once

#include <cstdint>
#include <set>
#include <array>
#include <string>
#include "WSclient.h"

namespace MUHelper
{
	enum ESkillActivationBase : uint32_t
	{
		ALWAYS = 0x00000000,
		ON_TIMER = 0x00000001,
		ON_CONDITION = 0x00000002,
	};

	enum ESkillActivationPreCon : uint32_t
	{
		ON_MOBS_NEARBY = 0x00000004,
		ON_MOBS_ATTACKING = 0x00000008
	};

	enum ESkillActivationSubCon : uint32_t
	{
		ON_MORE_THAN_TWO_MOBS = 0x00000010,
		ON_MORE_THAN_THREE_MOBS = 0x00000020,
		ON_MORE_THAN_FOUR_MOBS = 0x00000040,
		ON_MORE_THAN_FIVE_MOBS = 0x00000080
	};

	DEFINE_ENUM_FLAG_OPERATORS(ESkillActivationBase);
	DEFINE_ENUM_FLAG_OPERATORS(ESkillActivationPreCon);
	DEFINE_ENUM_FLAG_OPERATORS(ESkillActivationSubCon);

	constexpr uint32_t MUHELPER_SKILL_PRECON_CLEAR =
		~(static_cast<uint32_t>(ON_MOBS_NEARBY) | 
			static_cast<uint32_t>(ON_MOBS_ATTACKING));

	constexpr uint32_t MUHELPER_SKILL_SUBCON_CLEAR =
		~(static_cast<uint32_t>(ON_MORE_THAN_TWO_MOBS) | 
			static_cast<uint32_t>(ON_MORE_THAN_THREE_MOBS) |
			static_cast<uint32_t>(ON_MORE_THAN_FOUR_MOBS) | 
			static_cast<uint32_t>(ON_MORE_THAN_FIVE_MOBS));

	enum EPetAttackMode : BYTE
	{
		PET_ATTACK_CEASE = 0x00,
		PET_ATTACK_AUTO = 0x01,
		PET_ATTACK_TOGETHER = 0x02
	};

	typedef struct
	{
		int iHuntingRange = 0;

		bool bLongRangeCounterAttack = false;
		bool bReturnToOriginalPosition = false;
		int iMaxSecondsAway = 0;

		std::array<uint32_t, 3> aiSkill = { 0, 0, 0 };
		std::array<uint32_t, 3> aiSkillCondition = { 0, 0, 0 };
		std::array<uint32_t, 3> aiSkillInterval = { 0, 0, 0 };

		bool bUseCombo = false;

		std::array<uint32_t, 3> aiBuff = { 0, 0, 0 };

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
	} ConfigData;

	class ConfigDataSerDe {
	public:
		static void Serialize(const ConfigData& domain, PRECEIVE_MUHELPER_DATA& packet);
		static void Deserialize(const PRECEIVE_MUHELPER_DATA& packet, ConfigData& domain);
	};
}
