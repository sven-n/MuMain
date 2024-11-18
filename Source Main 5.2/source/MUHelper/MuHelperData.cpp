#include "stdafx.h"
#include "MuHelperData.h"

namespace MUHelper
{

	void ConfigDataSerDe::Serialize(const ConfigData& gameData, PRECEIVE_MUHELPER_DATA& netData)
	{
		memset(&netData, 0, sizeof(netData));

		netData.HuntingRange = static_cast<BYTE>(gameData.iHuntingRange & 0x0F);
		netData.DistanceMin = static_cast<BYTE>(gameData.iMaxSecondsAway & 0x0F);
		netData.LongDistanceAttack = gameData.bLongRangeCounterAttack ? 1 : 0;
		netData.OriginalPosition = gameData.bReturnToOriginalPosition ? 1 : 0;

		netData.BasicSkill1 = static_cast<WORD>(gameData.aiSkill[0] & 0xFFFF);
		netData.ActivationSkill1 = static_cast<WORD>(gameData.aiSkill[1] & 0xFFFF);
		netData.ActivationSkill2 = static_cast<WORD>(gameData.aiSkill[2] & 0xFFFF);

		netData.DelayMinSkill1 = static_cast<WORD>(gameData.aiSkillInterval[1] & 0xFFFF);
		netData.DelayMinSkill2 = static_cast<WORD>(gameData.aiSkillInterval[2] & 0xFFFF);

		if (gameData.aiSkillCondition[1] & ON_TIMER)
		{
			netData.Skill1Delay = 1;
		}
		if (gameData.aiSkillCondition[1] & ON_CONDITION)
		{
			netData.Skill1Con = 1;
		}
		if (gameData.aiSkillCondition[1] & ON_MOBS_NEARBY)
		{
			netData.Skill1PreCon = 0;
		}
		else if (gameData.aiSkillCondition[1] & ON_MOBS_ATTACKING)
		{
			netData.Skill1PreCon = 1;
		}

		if (gameData.aiSkillCondition[1] & ON_MORE_THAN_TWO_MOBS)
		{
			netData.Skill1SubCon = 0;
		}
		else if (gameData.aiSkillCondition[1] & ON_MORE_THAN_THREE_MOBS)
		{
			netData.Skill1SubCon = 1;
		}
		else if (gameData.aiSkillCondition[1] & ON_MORE_THAN_FOUR_MOBS)
		{
			netData.Skill1SubCon = 2;
		}
		else if (gameData.aiSkillCondition[1] & ON_MORE_THAN_FIVE_MOBS)
		{
			netData.Skill1SubCon = 3;
		}

		if (gameData.aiSkillCondition[2] & ON_TIMER)
		{
			netData.Skill2Delay = 1;
		}
		if (gameData.aiSkillCondition[2] & ON_CONDITION)
		{
			netData.Skill2Con = 1;
		}
		if (gameData.aiSkillCondition[2] & ON_MOBS_NEARBY)
		{
			netData.Skill2PreCon = 0;
		}
		else if (gameData.aiSkillCondition[2] & ON_MOBS_ATTACKING)
		{
			netData.Skill2PreCon = 1;
		}

		if (gameData.aiSkillCondition[2] & ON_MORE_THAN_TWO_MOBS)
		{
			netData.Skill2SubCon = 0;
		}
		else if (gameData.aiSkillCondition[2] & ON_MORE_THAN_THREE_MOBS)
		{
			netData.Skill2SubCon = 1;
		}
		else if (gameData.aiSkillCondition[2] & ON_MORE_THAN_FOUR_MOBS)
		{
			netData.Skill2SubCon = 2;
		}
		else if (gameData.aiSkillCondition[2] & ON_MORE_THAN_FIVE_MOBS)
		{
			netData.Skill2SubCon = 3;
		}

		netData.Combo = gameData.bUseCombo ? 1 : 0;

		netData.BuffSkill0NumberID = static_cast<WORD>(gameData.aiBuff[0] & 0xFFFF);
		netData.BuffSkill1NumberID = static_cast<WORD>(gameData.aiBuff[1] & 0xFFFF);
		netData.BuffSkill2NumberID = static_cast<WORD>(gameData.aiBuff[2] & 0xFFFF);

		netData.BuffDuration = gameData.bBuffDuration ? 1 : 0;
		netData.BuffDurationforAllPartyMembers = gameData.bBuffDurationParty ? 1 : 0;
		netData.CastingBuffMin = static_cast<WORD>(gameData.iBuffCastInterval);

		netData.AutoHeal = gameData.bAutoHeal ? 1 : 0;
		netData.HPStatusAutoPotion = static_cast<BYTE>((gameData.iPotionThreshold / 10) & 0x0F);
		netData.HPStatusAutoHeal = static_cast<BYTE>((gameData.iHealThreshold / 10) & 0x0F);
		netData.AutoPotion = gameData.bUseHealPotion ? 1 : 0;
		netData.DrainLife = gameData.bUseDrainLife ? 1 : 0;
		netData.Party = gameData.bSupportParty ? 1 : 0;
		netData.PreferenceOfPartyHeal = gameData.bAutoHealParty ? 1 : 0;

		netData.HPStatusOfPartyMembers = static_cast<BYTE>((gameData.iHealPartyThreshold / 10) & 0x0F);
		netData.HPStatusDrainLife = static_cast<BYTE>((gameData.iHealThreshold / 10) & 0x0F);

		netData.UseDarkSpirits = gameData.bUseDarkRaven ? 1 : 0;
		netData.PetAttack = static_cast<BYTE>(gameData.iDarkRavenMode);

		netData.RepairItem = gameData.bRepairItem ? 1 : 0;
		netData.ObtainRange = static_cast<BYTE>(gameData.iObtainingRange & 0x0F);
		netData.PickAllNearItems = gameData.bPickAllItems ? 1 : 0;
		netData.PickSelectedItems = gameData.bPickSelectItems ? 1 : 0;
		netData.Zen = gameData.bPickZen ? 1 : 0;
		netData.JewelOrGem = gameData.bPickJewel ? 1 : 0;
		netData.ExcellentItem = gameData.bPickExcellent ? 1 : 0;
		netData.SetItem = gameData.bPickAllItems ? 1 : 0;
		netData.AddExtraItem = gameData.bPickExtraItems ? 1 : 0;

		memset(netData.ExtraItems, 0, sizeof(netData.ExtraItems));
		int iItemIndex = 0;
		for (const auto& wsItem : gameData.aExtraItems)
		{
			if (iItemIndex >= 12)
			{
				break;
			}

			size_t n = wcstombs(netData.ExtraItems[iItemIndex], wsItem.c_str(), 15);
			if (n == (size_t)-1 || n == 15)
			{
				memset(netData.ExtraItems[iItemIndex], 0, 15);
			}
			iItemIndex++;
		}
	}

	void ConfigDataSerDe::Deserialize(const PRECEIVE_MUHELPER_DATA& netData, ConfigData& gameData)
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
		gameData.aiSkillCondition[1] |= netData.Skill1Delay ? ON_TIMER : 0;
		gameData.aiSkillCondition[1] |= netData.Skill1Con ? ON_CONDITION : 0;
		gameData.aiSkillCondition[1] |= netData.Skill1PreCon == 0 ? ON_MOBS_NEARBY : ON_MOBS_ATTACKING;
		gameData.aiSkillCondition[1] |= netData.Skill1SubCon == 0 ? ON_MORE_THAN_TWO_MOBS :
			netData.Skill1SubCon == 1 ? ON_MORE_THAN_THREE_MOBS :
			netData.Skill1SubCon == 2 ? ON_MORE_THAN_FOUR_MOBS :
			netData.Skill1SubCon == 3 ? ON_MORE_THAN_FIVE_MOBS :
			0;

		gameData.aiSkillCondition[2] |= netData.Skill2Delay ? ON_TIMER : 0;
		gameData.aiSkillCondition[2] |= netData.Skill2Con ? ON_CONDITION : 0;
		gameData.aiSkillCondition[2] |= netData.Skill2PreCon == 0 ? ON_MOBS_NEARBY : ON_MOBS_ATTACKING;
		gameData.aiSkillCondition[2] |= netData.Skill2SubCon == 0 ? ON_MORE_THAN_TWO_MOBS :
			netData.Skill2SubCon == 1 ? ON_MORE_THAN_THREE_MOBS :
			netData.Skill2SubCon == 2 ? ON_MORE_THAN_FOUR_MOBS :
			netData.Skill2SubCon == 3 ? ON_MORE_THAN_FIVE_MOBS :
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
		gameData.iHealThreshold = static_cast<int>(netData.HPStatusAutoHeal) * 10;
		gameData.bUseDrainLife = static_cast<int>(netData.DrainLife);
		gameData.bUseHealPotion = (bool)netData.AutoPotion;
		gameData.iPotionThreshold = static_cast<int>(netData.HPStatusAutoPotion) * 10;
		gameData.bSupportParty = (bool)netData.Party;
		gameData.bAutoHealParty = (bool)netData.PreferenceOfPartyHeal;
		gameData.iHealPartyThreshold = static_cast<int>(netData.HPStatusOfPartyMembers) * 10;

		gameData.bUseDarkRaven = (bool)netData.UseDarkSpirits;
		gameData.iDarkRavenMode = static_cast<int>(netData.PetAttack);
		gameData.bRepairItem = (bool)netData.RepairItem;

		gameData.iObtainingRange = static_cast<int>(netData.ObtainRange);
		gameData.bPickAllItems = (bool)netData.PickAllNearItems;
		gameData.bPickSelectItems = (bool)netData.PickSelectedItems;
		gameData.bPickZen = (bool)netData.Zen;
		gameData.bPickJewel = (bool)netData.JewelOrGem;
		gameData.bPickExcellent = (bool)netData.ExcellentItem;
		gameData.bPickAncient = (bool)netData.SetItem;
		gameData.bPickExtraItems = (bool)netData.AddExtraItem;

		wchar_t wsExtraItemBuffer[15 + 1];
		for (int i = 0; i < sizeof(netData.ExtraItems) / sizeof(netData.ExtraItems[0]); i++)
		{
			memset(wsExtraItemBuffer, 0, sizeof(wsExtraItemBuffer));

			size_t n = std::mbstowcs(wsExtraItemBuffer, &netData.ExtraItems[i][0], 15);
			if (n > 0 && n <= 15)
			{
				wsExtraItemBuffer[n] = L'\0';
				if (wsExtraItemBuffer[0] != L'\0')
				{
					gameData.aExtraItems.insert(std::wstring(wsExtraItemBuffer));
				}
			}
		}
	}

}
