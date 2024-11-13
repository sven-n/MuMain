#include "stdafx.h"
#include "MuHelperData.h"

void MuHelperConfigSerDe::Serialize(const cMuHelperConfig& gameData, PRECEIVE_MUHELPER_DATA& netData) 
{
	netData.HuntingRange = static_cast<BYTE>(gameData.iHuntingRange);
	netData.ObtainRange = static_cast<BYTE>(gameData.iObtainingRange);

	netData.SetItem = gameData.bPickAllItems ? 1 : 0;
	netData.ExcellentItem = gameData.bPickExcellent ? 1 : 0;
	netData.JewelOrGem = gameData.bPickJewel ? 1 : 0;
	netData.Zen = gameData.bPickZen ? 1 : 0;
	netData.AddExtraItem = gameData.bPickExtraItems ? 1 : 0;

	// TODO: Complete
}

void MuHelperConfigSerDe::Deserialize(const PRECEIVE_MUHELPER_DATA& netData, cMuHelperConfig& gameData) 
{
	gameData.iHuntingRange = static_cast<int>(netData.HuntingRange);

	gameData.iMaxSecondsAway = static_cast<int>(netData.DistanceMin);
	gameData.bLongRangeCounterAttack = (bool)netData.LongDistanceAttack;
	gameData.bReturnToOriginalPosition = (bool)netData.OriginalPosition;

	gameData.aiSkill.fill(0);
	gameData.aiSkill[0] = static_cast<int>(netData.BasicSkill1);
	gameData.aiSkill[1] = static_cast<int>(netData.ActivationSkill1);
	gameData.aiSkill[2] = static_cast<int>(netData.ActivationSkill2);

	gameData.aiSkillInterval.fill(0);
	gameData.aiSkillInterval[1] = static_cast<int>(netData.DelayMinSkill1);
	gameData.aiSkillInterval[2] = static_cast<int>(netData.DelayMinSkill2);

	gameData.aiSkillCondition.fill(0);
	gameData.aiSkillCondition[1] |= netData.Skill1Delay ? MUHELPER_ATTACK_ON_TIMER: 0;
	gameData.aiSkillCondition[1] |= netData.Skill1PreCon == 0 ? MUHELPER_ATTACK_ON_MOBS_NEARBY : MUHELPER_ATTACK_ON_MOBS_ATTACKING;
	gameData.aiSkillCondition[1] |= netData.Skill1SubCon == 0 ? MUHELPER_ATTACK_ON_MORE_THAN_TWO_MOBS : 
		netData.Skill1SubCon == 1 ? MUHELPER_ATTACK_ON_MORE_THAN_THREE_MOBS : 
		netData.Skill1SubCon == 2 ? MUHELPER_ATTACK_ON_MORE_THAN_FOUR_MOBS :
		netData.Skill1SubCon == 3 ? MUHELPER_ATTACK_ON_MORE_THAN_FIVE_MOBS : 
		0;

	gameData.aiSkillCondition[2] |= netData.Skill2Delay ? MUHELPER_ATTACK_ON_TIMER : 0;
	gameData.aiSkillCondition[2] |= netData.Skill2PreCon == 0 ? MUHELPER_ATTACK_ON_MOBS_NEARBY : MUHELPER_ATTACK_ON_MOBS_ATTACKING;
	gameData.aiSkillCondition[2] |= netData.Skill2SubCon == 0 ? MUHELPER_ATTACK_ON_MORE_THAN_TWO_MOBS :
		netData.Skill2SubCon == 1 ? MUHELPER_ATTACK_ON_MORE_THAN_THREE_MOBS :
		netData.Skill2SubCon == 2 ? MUHELPER_ATTACK_ON_MORE_THAN_FOUR_MOBS :
		netData.Skill2SubCon == 3 ? MUHELPER_ATTACK_ON_MORE_THAN_FIVE_MOBS :
		0;
	gameData.bUseCombo = (bool)netData.Combo;

	gameData.aiBuff.fill(0);
	gameData.aiBuff[0] = static_cast<int>(netData.BuffSkill0NumberID);
	gameData.aiBuff[1] = static_cast<int>(netData.BuffSkill1NumberID);
	gameData.aiBuff[2] = static_cast<int>(netData.BuffSkill2NumberID);

	gameData.bBuffDuration = (bool)netData.BuffDuration;
	gameData.bBuffDurationParty = (bool)netData.BuffDurationforAllPartyMembers;
	gameData.iBuffCastInterval = static_cast<int>(netData.CastingBuffMin);

	gameData.bAutoHeal = (bool)netData.AutoHeal;
	gameData.iHealThreshold = static_cast<int>(netData.HPStatusAutoHeal);
	gameData.bUseDrainLife = static_cast<int>(netData.DrainLife);
	gameData.bUseHealPotion = (bool)netData.AutoPotion;
	gameData.iPotionThreshold = static_cast<int>(netData.HPStatusAutoPotion);
	gameData.bSupportParty = (bool)netData.Party;
	gameData.bAutoHealParty = (bool)netData.PreferenceOfPartyHeal;
	gameData.iHealPartyThreshold = static_cast<int>(netData.HPStatusOfPartyMembers);

	gameData.bUseDarkRaven = (bool)netData.UseDarkSpirits;
	gameData.iDarkRavenMode = static_cast<int>(netData.PetAttack);
	gameData.bRepairItem = (bool)netData.RepairItem;

	gameData.iObtainingRange = static_cast<int>(netData.ObtainRange);
	gameData.bPickAllItems = (bool)netData.PickAllNearItems;
	gameData.bPickZen = (bool)netData.Zen;
	gameData.bPickJewel = (bool)netData.JewelOrGem;
	gameData.bPickExcellent = (bool)netData.ExcellentItem;
	gameData.bPickAncient = (bool)netData.SetItem;
	gameData.bPickExtraItems = (bool) netData.AddExtraItem;

	wchar_t wsExtraItemBuffer[15];
	for (int i = 0; i < sizeof(netData.ExtraItems) / sizeof(netData.ExtraItems[0]); ++i)
	{
		std::mbstowcs(wsExtraItemBuffer, netData.ExtraItems[i], 15);
		if (wsExtraItemBuffer[i] != L'\0')
		{
			gameData.aExtraItems.insert(std::wstring(wsExtraItemBuffer));
		}
	}
}
