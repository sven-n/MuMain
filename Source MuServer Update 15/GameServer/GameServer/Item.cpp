// Item.cpp: implementation of the CItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Item.h"
#include "380ItemOption.h"
#include "CustomJewel.h"
#include "CustomWing.h"
#include "EventInventory.h"
#include "ItemManager.h"
#include "ItemOption.h"
#include "ItemValue.h"
#include "ItemStack.h"
#include "JewelOfHarmonyOption.h"
#include "LuckyItem.h"
#include "MuunSystem.h"
#include "ObjectManager.h"
#include "PentagramSystem.h"
#include "ServerInfo.h"
#include "SetItemOption.h"
#include "SkillManager.h"
#include "SocketItemOption.h"
#include "Util.h"

CPetItemExp gPetItemExp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItem::CItem() // OK
{
	this->Clear();
}

void CItem::Clear() // OK
{
	this->m_Serial = 0;
	this->m_Index = -1;
	this->m_Level = 0;
	this->m_Slot = 0;
	this->m_Class = 0;
	this->m_TwoHand = 0;
	this->m_AttackSpeed = 0;
	this->m_DamageMin = 0;
	this->m_DamageMax = 0;
	this->m_DefenseSuccessRate = 0;
	this->m_Defense = 0;
	this->m_MagicDefense = 0;
	this->m_Durability = 0;
	this->m_DurabilitySmall = 0;
	this->m_Leadership = 0;

	memset(this->m_SpecialIndex,0,sizeof(this->m_SpecialIndex));

	memset(this->m_SpecialValue,0,sizeof(this->m_SpecialValue));

	memset(this->m_Resistance,0,sizeof(this->m_Resistance));

	this->m_Value = 0;
	this->m_PShopValue = -1;
	#if(GAMESERVER_UPDATE>=802)
	this->m_PShopJoBValue = -1;
	this->m_PShopJoSValue = -1;
	this->m_PShopJoCValue = -1;
	#endif
	this->m_PcPointValue = 0;
	this->m_ItemExist = 1;
	this->m_Option1 = 0;
	this->m_Option2 = 0;
	this->m_Option3 = 0;
	this->m_NewOption = 0;
	this->m_CurrentDurabilityState = 1.0;
	this->m_QuestItem = 0;
	this->m_SetOption = 0;
	this->m_IsExpiredItem = 0;
	this->m_IsPeriodicItem = 0;
	this->m_LoadPeriodicItem = 0;
	this->m_PeriodicItemTime = 0;
	this->m_LoadPetItemInfo = 0;
	this->m_PetItemLevel = 1;
	this->m_PetItemExp = 0;
	this->m_JewelOfHarmonyOption = 0;
	this->m_SubRequireStr = 0;
	this->m_SubRequireDex = 0;
	this->m_ItemOptionEx = 0;
	this->m_SocketOptionBonus = 0xFF;

	this->Coin1 = 0;
	this->Coin2 = 0;
	this->Coin3 = 0;
	this->Zen	= 0;

	memset(this->m_SocketOption,0xFF,sizeof(this->m_SocketOption));
}

bool CItem::IsItem() // OK
{
	if(CHECK_RANGE(this->m_Index,MAX_ITEM) == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

bool CItem::IsExcItem() // OK
{
	if(this->m_Index == GET_ITEM(13,3) || this->m_Index == GET_ITEM(13,37)) // Dinorant,Fenrir
	{
		return 0;
	}

	if(this->m_NewOption == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

bool CItem::IsSetItem() // OK
{
	return gSetItemOption.IsSetItem(this);
}

bool CItem::IsJewelOfHarmonyItem() // OK
{
	return gJewelOfHarmonyOption.IsJewelOfHarmonyItem(this);
}

bool CItem::Is380Item() // OK
{
	return g380ItemOption.Is380Item(this);
}

bool CItem::IsSocketItem() // OK
{
	return gSocketItemOption.IsSocketItem(this);
}

bool CItem::IsLuckyItem() // OK
{
	return gLuckyItem.IsLuckyItem(this);
}

bool CItem::IsPentagramItem() // OK
{
	return gPentagramSystem.IsPentagramItem(this);
}

bool CItem::IsPentagramJewel() // OK
{
	return gPentagramSystem.IsPentagramJewel(this);
}

bool CItem::IsPentagramMithril() // OK
{
	return gPentagramSystem.IsPentagramMithril(this);
}

bool CItem::IsEventItem() // OK
{
	return gEventInventory.IsEventItem(this);
}

bool CItem::IsMuunItem() // OK
{
	return gMuunSystem.IsMuunItem(this);
}

bool CItem::IsMuunUtil() // OK
{
	return gMuunSystem.IsMuunUtil(this);
}

bool CItem::IsClass(int Class,int ChangeUp) // OK
{
	if(CHECK_RANGE(Class,MAX_CLASS) == 0)
	{
		return 0;
	}

	ITEM_INFO ItemInfo;

	if(gItemManager.GetInfo(this->m_Index,&ItemInfo) == 0)
	{
		return 0;
	}

	if(ItemInfo.RequireClass[Class] <= 0 || (ChangeUp+1) < ItemInfo.RequireClass[Class])
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void CItem::Convert(int index,BYTE Option1,BYTE Option2,BYTE Option3,BYTE NewOption,BYTE SetOption,BYTE JewelOfHarmonyOption,BYTE ItemOptionEx,BYTE SocketOption[MAX_SOCKET_OPTION],BYTE SocketOptionBonus) // OK
{
	ITEM_INFO ItemInfo;

	if(gItemManager.GetInfo(index,&ItemInfo) == 0)
	{
		return;
	}

	this->m_Index = index;

	this->m_Level &= 15;

	this->m_Slot = ItemInfo.Slot;

	this->m_Option1 = Option1;

	this->m_Option2 = Option2;

	this->m_Option3 = Option3;

	this->m_NewOption = NewOption;

	this->m_SetOption = 0;

	this->m_JewelOfHarmonyOption = JewelOfHarmonyOption;

	this->m_ItemOptionEx = ItemOptionEx;

	this->m_SocketOptionBonus = SocketOptionBonus;

	if(SocketOption == 0)
	{
		memset(this->m_SocketOption,0xFF,MAX_SOCKET_OPTION);
	}
	else
	{
		memcpy(this->m_SocketOption,SocketOption,MAX_SOCKET_OPTION);
	}

	if(this->m_Index == GET_ITEM(0,19) || this->m_Index == GET_ITEM(2,13) || this->m_Index == GET_ITEM(4,18) || this->m_Index == GET_ITEM(5,10) || this->m_Index == GET_ITEM(5,36)) // Sword of Archangel,Scepter of Archangel,Crossbow of Archangel,Staff of Archangel,Stick of Archangel
	{
		NewOption = 0;
	}

	if(this->m_Slot == 7)
	{
		NewOption = 0;
	}

	if(this->m_Index == GET_ITEM(13,3) || this->m_Index == GET_ITEM(13,37)) // Dinorant,Fenrir
	{
		NewOption = 0;
	}

	if(((SetOption & 3) != 1 && (SetOption & 3) != 2) || (((SetOption >> 2) & 3) != 1 && ((SetOption >> 2) & 3) != 2))
	{
		this->m_SetOption = 0;
	}
	else
	{
		this->m_SetOption = SetOption & 15;
	}

	if(this->m_Index == GET_ITEM(14,23) || this->m_Index == GET_ITEM(14,24) || this->m_Index == GET_ITEM(14,25) || this->m_Index == GET_ITEM(14,26) || this->m_Index == GET_ITEM(14,65) || this->m_Index == GET_ITEM(14,66) || this->m_Index == GET_ITEM(14,67) || this->m_Index == GET_ITEM(14,68))
	{
		this->m_QuestItem = 1;
	}
	else
	{
		this->m_QuestItem = 0;
	}

	if(gItemManager.GetItemDurability(this->m_Index,0,0,0) == 0)
	{
		this->m_BaseDurability = 0;
	}
	else
	{
		this->m_BaseDurability = (float)gItemManager.GetItemDurability(this->m_Index,this->m_Level,NewOption,this->m_SetOption);
	}

	this->m_Value = ItemInfo.Value;
	this->m_AttackSpeed = ItemInfo.AttackSpeed;
	this->m_TwoHand = ItemInfo.TwoHand;
	this->m_DamageMin = ItemInfo.DamageMin;
	this->m_DamageMax = ItemInfo.DamageMax;
	this->m_DefenseSuccessRate = ItemInfo.DefenseSuccessRate;
	this->m_Defense = ItemInfo.Defense;
	this->m_MagicDefense = ItemInfo.MagicDefense;
	this->m_WalkSpeed = ItemInfo.WalkSpeed;
	this->m_MagicDamageRate = ItemInfo.MagicDamageRate;

	this->m_DurabilityState[0] = (float)(this->m_BaseDurability*0.5);
	this->m_DurabilityState[1] = (float)(this->m_BaseDurability*0.4);
	this->m_DurabilityState[2] = (float)(this->m_BaseDurability*0.3);
	this->m_DurabilityState[3] = (float)(this->m_BaseDurability*0.2);
	this->m_CurrentDurabilityState = 1.0;

	if(this->m_Durability == 0)
	{
		this->m_CurrentDurabilityState = (float)0.0;
	}
	else if(this->m_Durability < this->m_DurabilityState[3])
	{
		this->m_CurrentDurabilityState = (float)0.5;
	}
	else if(this->m_Durability < this->m_DurabilityState[2])
	{
		this->m_CurrentDurabilityState = (float)0.6;
	}
	else if(this->m_Durability < this->m_DurabilityState[1])
	{
		this->m_CurrentDurabilityState = (float)0.7;
	}
	else if(this->m_Durability < this->m_DurabilityState[0])
	{
		this->m_CurrentDurabilityState = (float)0.8;
	}

	for(int n=0;n < MAX_RESISTANCE_TYPE;n++)
	{
		this->m_Resistance[n] = (((ItemInfo.Resistance[n]*this->m_Level)>255)?255:(ItemInfo.Resistance[n]*this->m_Level));
	}

	int ItemLevel = ItemInfo.Level;

	if(NewOption != 0 || this->m_SetOption != 0)
	{
		ItemLevel += 25;
	}

	this->m_RequireStrength = 0;
	this->m_RequireDexterity = 0;
	this->m_RequireEnergy = 0;
	this->m_RequireLevel = 0;
	this->m_RequireVitality = 0;
	this->m_RequireLeadership = 0;
	this->m_Leadership = 0;

	if(ItemInfo.RequireStrength != 0)
	{
		this->m_RequireStrength = (((ItemInfo.RequireStrength*((this->m_Level*3)+ItemLevel))*3)/100)+20;
	}

	if(ItemInfo.RequireDexterity != 0)
	{
		this->m_RequireDexterity = (((ItemInfo.RequireDexterity*((this->m_Level*3)+ItemLevel))*3)/100)+20;
	}

	if(ItemInfo.RequireVitality != 0)
	{
		this->m_RequireVitality = (((ItemInfo.RequireVitality*((this->m_Level*3)+ItemLevel))*3)/100)+20;
	}

	if((this->m_Index/MAX_ITEM_TYPE) == 5 && ItemInfo.Slot == 1)
	{
		if(ItemInfo.RequireEnergy != 0)
		{
			this->m_RequireEnergy = (((ItemInfo.RequireEnergy*(this->m_Level+ItemLevel))*3)/100)+20;
		}
	}
	else
	{
		if(ItemInfo.RequireEnergy != 0)
		{
			this->m_RequireEnergy = (((ItemInfo.RequireEnergy*((this->m_Level*3)+ItemLevel))*4)/100)+20;
		}
	}

	if(ItemInfo.RequireLeadership != 0)
	{
		this->m_RequireLeadership = (((ItemInfo.RequireLeadership*((this->m_Level*3)+ItemLevel))*3)/100)+20;
	}

	if(this->m_Index == GET_ITEM(13,5)) // Dark Reaven
	{
		this->m_RequireLeadership = 185+(this->m_PetItemLevel*15);
	}

	this->m_SubRequireStr = gJewelOfHarmonyOption.GetJewelOfHarmonySubRequireStrOption(this);

	this->m_SubRequireDex = gJewelOfHarmonyOption.GetJewelOfHarmonySubRequireDexOption(this);

	if(ItemInfo.RequireLevel != 0)
	{
		if(this->m_Index >= GET_ITEM(0,0) && this->m_Index < GET_ITEM(12,0))
		{
			this->m_RequireLevel = ItemInfo.RequireLevel;
		}
		else if((this->m_Index >= GET_ITEM(12,3) && this->m_Index <= GET_ITEM(12,6)) || this->m_Index == GET_ITEM(12,42)) // Wings
		{
			this->m_RequireLevel = ItemInfo.RequireLevel+(this->m_Level*5);
		}
		else if((this->m_Index >= GET_ITEM(12,7) && this->m_Index <= GET_ITEM(12,24) && this->m_Index != GET_ITEM(12,15)) || (this->m_Index >= GET_ITEM(12,44) && this->m_Index <= GET_ITEM(12,48))) // Orbs,Scrolls
		{
			this->m_RequireLevel = ItemInfo.RequireLevel;
		}
		else if((this->m_Index >= GET_ITEM(12,36) && this->m_Index <= GET_ITEM(12,40)) || this->m_Index == GET_ITEM(12,43) || this->m_Index == GET_ITEM(12,50)) // 3rd Wings
		{
			this->m_RequireLevel = ItemInfo.RequireLevel;
		}
		else if(this->m_Index >= GET_ITEM(12,130) && this->m_Index <= GET_ITEM(12,135)) // Mini Wings
		{
			this->m_RequireLevel = ItemInfo.RequireLevel;
		}
		else if(this->m_Index >= GET_ITEM(12,262) && this->m_Index <= GET_ITEM(12,265)) // Monster Wings
		{
			this->m_RequireLevel = ItemInfo.RequireLevel+(this->m_Level*4);
		}
		else if(this->m_Index >= GET_ITEM(12,266) && this->m_Index <= GET_ITEM(12,267)) // Special Wings
		{
			this->m_RequireLevel = ItemInfo.RequireLevel;
		}
		else if(gCustomWing.CheckCustomWingByItem(this->m_Index) != 0)
		{
			this->m_RequireLevel = ItemInfo.RequireLevel;
		}
		else if(this->m_Index == GET_ITEM(13,4)) // Dark Horse
		{
			this->m_RequireLevel = 218+(this->m_PetItemLevel*2);
		}
		else
		{
			this->m_RequireLevel = ItemInfo.RequireLevel+(this->m_Level*4);
		}
	}

	if(this->m_Index == GET_ITEM(13,10)) // Transformation Ring
	{
		if(ItemLevel <= 2)
		{
			this->m_RequireLevel = 20;
		}
		else
		{
			this->m_RequireLevel = 50;
		}
	}

	if(NewOption != 0 && this->m_RequireLevel > 0)
	{
		if(this->m_Index >= GET_ITEM(12,0))
		{
			this->m_RequireLevel += 20;
		}
	}

	if(this->m_SetOption != 0)
	{
		ItemLevel = ItemInfo.Level+30;
	}

	int ChaosItem = 0;

	if(this->m_Index == GET_ITEM(2,6))
	{
		ChaosItem = 15;
	}
	else if(this->m_Index == GET_ITEM(5,7))
	{
		ChaosItem = 25;
	}
	else if(this->m_Index == GET_ITEM(4,6))
	{
		ChaosItem = 30;
	}

	if(this->m_DamageMax > 0)
	{
		if(this->m_SetOption != 0 && ItemInfo.Level != 0)
		{
			this->m_DamageMax += ((this->m_DamageMin*25)/ItemInfo.Level)+5;
			this->m_DamageMax += (ItemLevel/40)+5;
		}
		else if(NewOption != 0)
		{
			if(ChaosItem != 0)
			{
				this->m_DamageMax += ChaosItem;
			}
			else if(ItemInfo.Level != 0)
			{
				this->m_DamageMax += ((this->m_DamageMin*25)/ItemInfo.Level)+5;
			}
		}

		if(this->IsPentagramItem() == 0)
		{
			this->m_DamageMax += (this->m_Level*3);
		}
		else
		{
			this->m_DamageMax += (this->m_Level*4);
		}

		if(this->m_Level >= 10)
		{
			this->m_DamageMax += ((this->m_Level-9)*(this->m_Level-8))/2;
		}
	}

	if(this->m_DamageMin > 0)
	{
		if(this->m_SetOption != 0 && ItemInfo.Level != 0)
		{
			this->m_DamageMin += ((this->m_DamageMin*25)/ItemInfo.Level)+5;
			this->m_DamageMin += (ItemLevel/40)+5;
		}
		else if(NewOption != 0)
		{
			if(ChaosItem != 0)
			{
				this->m_DamageMin += ChaosItem;
			}
			else if(ItemInfo.Level != 0)
			{
				this->m_DamageMin += ((this->m_DamageMin*25)/ItemInfo.Level)+5;
			}
		}

		if(this->IsPentagramItem() == 0)
		{
			this->m_DamageMin += (this->m_Level*3);
		}
		else
		{
			this->m_DamageMin += (this->m_Level*4);
		}

		if(this->m_Level >= 10)
		{
			this->m_DamageMin += ((this->m_Level-9)*(this->m_Level-8))/2;
		}
	}

	if(this->m_MagicDamageRate > 0)
	{
		if(this->m_SetOption != 0 && ItemInfo.Level != 0)
		{
			this->m_MagicDamageRate += ((this->m_MagicDamageRate*25)/ItemInfo.Level)+5;
			this->m_MagicDamageRate += (ItemLevel/60)+2;
		}
		else if(NewOption != 0)
		{
			if(ChaosItem != 0)
			{
				this->m_MagicDamageRate += ChaosItem;
			}
			else if(ItemInfo.Level != 0)
			{
				this->m_MagicDamageRate += ((this->m_MagicDamageRate*25)/ItemInfo.Level)+5;
			}
		}

		this->m_MagicDamageRate += (this->m_Level*3);

		if(this->m_Level >= 10)
		{
			this->m_MagicDamageRate += ((this->m_Level-9)*(this->m_Level-8))/2;
		}
	}

	if(this->m_DefenseSuccessRate > 0)
	{
		if(this->m_SetOption != 0 && ItemInfo.Level != 0)
		{
			this->m_DefenseSuccessRate += ((this->m_DefenseSuccessRate*25)/ItemInfo.Level)+5;
			this->m_DefenseSuccessRate += (ItemLevel/40)+5;
		}
		else if(NewOption != 0 && ItemInfo.Level != 0)
		{
			this->m_DefenseSuccessRate += ((this->m_DefenseSuccessRate*25)/ItemInfo.Level)+5;
		}

		this->m_DefenseSuccessRate += (this->m_Level*3);

		if(this->m_Level >= 10)
		{
			this->m_DefenseSuccessRate += ((this->m_Level-9)*(this->m_Level-8))/2;
		}
	}

	if(this->m_Index == GET_ITEM(13,30) || this->m_Index == GET_ITEM(12,49)) // Cloak of Lord,Cloak of Fighter
	{
		this->m_Defense = 15;
	}

	if(this->m_Defense > 0)
	{
		if(this->m_Index >= GET_ITEM(6,0) && this->m_Index < GET_ITEM(7,0)) // Shields
		{
			this->m_Defense += this->m_Level;

			if(this->m_SetOption != 0 && ItemLevel != 0)
			{
				this->m_Defense += ((this->m_Defense*20)/ItemLevel)+2;
			}
		}
		else
		{
			if(this->m_SetOption != 0 && ItemInfo.Level != 0 && ItemLevel != 0)
			{
				this->m_Defense += (((this->m_Defense*12)/ItemInfo.Level)+(ItemInfo.Level/5))+4;
				this->m_Defense += (((this->m_Defense*3)/ItemLevel)+(ItemLevel/30))+2;
			}
			else if(NewOption != 0 && ItemInfo.Level != 0)
			{
				this->m_Defense += (((this->m_Defense*12)/ItemInfo.Level)+(ItemInfo.Level/5))+4;
			}

			if((this->m_Index >= GET_ITEM(12,3) && this->m_Index <= GET_ITEM(12,6)) || this->m_Index == GET_ITEM(12,42) || this->m_Index == GET_ITEM(13,4)) // 2sd Wings,Dark Horse
			{
				this->m_Defense += (this->m_Level*2);
			}
			else if((this->m_Index >= GET_ITEM(12,36) && this->m_Index <= GET_ITEM(12,40)) || this->m_Index == GET_ITEM(12,43) || this->m_Index == GET_ITEM(12,50)) // 3rd Wings
			{
				this->m_Defense += (this->m_Level*4);
			}
			else if(this->m_Index >= GET_ITEM(12,130) && this->m_Index <= GET_ITEM(12,135)) // Mini Wings
			{
				this->m_Defense += (this->m_Level*2);
			}
			else if(this->m_Index >= GET_ITEM(12,262) && this->m_Index <= GET_ITEM(12,265)) // Monster Wings
			{
				this->m_Defense += (this->m_Level*3);
			}
			else if(this->m_Index >= GET_ITEM(12,266) && this->m_Index <= GET_ITEM(12,267)) // Special Wings
			{
				this->m_Defense += (this->m_Level*3);
			}
			else if(gCustomWing.CheckCustomWingByItem(this->m_Index) != 0)
			{
				this->m_Defense += gCustomWing.GetCustomWingDefense(this->m_Index,this->m_Level);
			}
			else
			{
				this->m_Defense += (this->m_Level*3);
			}

			if(this->m_Level >= 10)
			{
				this->m_Defense += ((this->m_Level-9)*(this->m_Level-8))/2;
			}
		}
	}

	if(this->m_MagicDefense > 0)
	{
		this->m_MagicDefense += (this->m_Level*3);

		if(this->m_Level >= 10)
		{
			this->m_MagicDefense += ((this->m_Level-9)*(this->m_Level-8))/2;
		}
	}

	memset(this->m_SpecialIndex,0,sizeof(this->m_SpecialIndex));

	memset(this->m_SpecialValue,0,sizeof(this->m_SpecialValue));

	this->m_SkillChange = 0;

	if(gItemOption.GetItemOption(SPECIAL_OPTION1,this) == 0)
	{
		this->m_Option1 = 0;
		this->m_SpecialIndex[SPECIAL_OPTION1] = 0;
		this->m_SpecialValue[SPECIAL_OPTION1] = 0;
		this->m_SkillChange = 0;
	}
	else
	{
		this->m_Option1 = ((this->m_Option1==0)?1:this->m_Option1);
		this->m_SpecialIndex[SPECIAL_OPTION1] = ((ItemInfo.Skill==SKILL_NONE)?this->m_SpecialIndex[SPECIAL_OPTION1]:((ItemInfo.Skill==SKILL_FORCE_WAVE)?SKILL_NONE:ItemInfo.Skill));
		this->m_SpecialValue[SPECIAL_OPTION1] = this->m_SpecialValue[SPECIAL_OPTION1];
		this->m_SkillChange = ((ItemInfo.Skill==SKILL_FORCE_WAVE)?1:0);
	}

	if(gItemOption.GetItemOption(SPECIAL_OPTION2,this) == 0)
	{
		this->m_Option2 = 0;
		this->m_SpecialIndex[SPECIAL_OPTION2] = 0;
		this->m_SpecialValue[SPECIAL_OPTION2] = 0;
	}
	else
	{
		this->m_Option2 = ((this->m_Option2==0)?1:this->m_Option2);
		this->m_SpecialIndex[SPECIAL_OPTION2] = this->m_SpecialIndex[SPECIAL_OPTION2];
		this->m_SpecialValue[SPECIAL_OPTION2] = this->m_SpecialValue[SPECIAL_OPTION2];
	}

	if(gItemOption.GetItemOption(SPECIAL_OPTION3,this) == 0)
	{
		this->m_Option3 = 0;
		this->m_SpecialIndex[SPECIAL_OPTION3] = 0;
		this->m_SpecialValue[SPECIAL_OPTION3] = 0;
	}
	else
	{
		this->m_Option3 = ((this->m_Option3==0)?1:this->m_Option3);
		this->m_SpecialIndex[SPECIAL_OPTION3] = this->m_SpecialIndex[SPECIAL_OPTION3];
		this->m_SpecialValue[SPECIAL_OPTION3] = this->m_SpecialValue[SPECIAL_OPTION3]*this->m_Option3;

		switch(this->m_Index)
		{
			case GET_ITEM(12,1): // Wings of Angel
				this->m_RequireStrength += (this->m_Option3*4);
				break;
			case GET_ITEM(12,2): // Wings of Satan
				this->m_RequireStrength += (this->m_Option3*4);
				break;
			case GET_ITEM(12,3): // Wings of Spirit
				this->m_RequireStrength += (this->m_Option3*4);
				break;
			case GET_ITEM(12,4): // Wings of Soul
				this->m_RequireStrength += (this->m_Option3*4);
				break;
			case GET_ITEM(12,5): // Wings of Devil
				this->m_RequireStrength += (this->m_Option3*4);
				break;
			case GET_ITEM(12,6): // Wings of Darkness
				this->m_RequireStrength += (this->m_Option3*4);
				break;
			case GET_ITEM(12,41): // Wings of Misery
				this->m_RequireStrength += (this->m_Option3*4);
				break;
			case GET_ITEM(12,42): // Wings of Despair
				this->m_RequireStrength += (this->m_Option3*4);
				break;
			case GET_ITEM(12,49): // Cloak of Fighter
				this->m_RequireStrength += (this->m_Option3*4);
				break;
			case GET_ITEM(13,30): // Cloak of Lord
				this->m_RequireStrength += (this->m_Option3*4);
				break;
			default:
				this->m_RequireStrength += ((this->m_Index<GET_ITEM(12,0))?(this->m_Option3*4):0);
				break;
		}
	}

	if(gItemOption.GetItemOption(SPECIAL_EXCELLENT1,this) == 0)
	{
		this->m_SpecialIndex[SPECIAL_EXCELLENT1] = 0;
		this->m_SpecialValue[SPECIAL_EXCELLENT1] = 0;
	}
	else
	{
		this->m_SpecialIndex[SPECIAL_EXCELLENT1] = this->m_SpecialIndex[SPECIAL_EXCELLENT1];
		this->m_SpecialValue[SPECIAL_EXCELLENT1] = this->m_SpecialValue[SPECIAL_EXCELLENT1];
	}

	if(gItemOption.GetItemOption(SPECIAL_EXCELLENT2,this) == 0)
	{
		this->m_SpecialIndex[SPECIAL_EXCELLENT2] = 0;
		this->m_SpecialValue[SPECIAL_EXCELLENT2] = 0;
	}
	else
	{
		this->m_SpecialIndex[SPECIAL_EXCELLENT2] = this->m_SpecialIndex[SPECIAL_EXCELLENT2];
		this->m_SpecialValue[SPECIAL_EXCELLENT2] = this->m_SpecialValue[SPECIAL_EXCELLENT2];
	}

	if(gItemOption.GetItemOption(SPECIAL_EXCELLENT3,this) == 0)
	{
		this->m_SpecialIndex[SPECIAL_EXCELLENT3] = 0;
		this->m_SpecialValue[SPECIAL_EXCELLENT3] = 0;
	}
	else
	{
		this->m_SpecialIndex[SPECIAL_EXCELLENT3] = this->m_SpecialIndex[SPECIAL_EXCELLENT3];
		this->m_SpecialValue[SPECIAL_EXCELLENT3] = this->m_SpecialValue[SPECIAL_EXCELLENT3];
	}

	if(gItemOption.GetItemOption(SPECIAL_EXCELLENT4,this) == 0)
	{
		this->m_SpecialIndex[SPECIAL_EXCELLENT4] = 0;
		this->m_SpecialValue[SPECIAL_EXCELLENT4] = 0;
	}
	else
	{
		this->m_SpecialIndex[SPECIAL_EXCELLENT4] = this->m_SpecialIndex[SPECIAL_EXCELLENT4];
		this->m_SpecialValue[SPECIAL_EXCELLENT4] = this->m_SpecialValue[SPECIAL_EXCELLENT4];
	}

	if(gItemOption.GetItemOption(SPECIAL_EXCELLENT5,this) == 0)
	{
		this->m_SpecialIndex[SPECIAL_EXCELLENT5] = 0;
		this->m_SpecialValue[SPECIAL_EXCELLENT5] = 0;
	}
	else
	{
		this->m_SpecialIndex[SPECIAL_EXCELLENT5] = this->m_SpecialIndex[SPECIAL_EXCELLENT5];
		this->m_SpecialValue[SPECIAL_EXCELLENT5] = this->m_SpecialValue[SPECIAL_EXCELLENT5];
	}

	if(gItemOption.GetItemOption(SPECIAL_EXCELLENT6,this) == 0)
	{
		this->m_SpecialIndex[SPECIAL_EXCELLENT6] = 0;
		this->m_SpecialValue[SPECIAL_EXCELLENT6] = 0;
	}
	else
	{
		this->m_SpecialIndex[SPECIAL_EXCELLENT6] = this->m_SpecialIndex[SPECIAL_EXCELLENT6];
		this->m_SpecialValue[SPECIAL_EXCELLENT6] = this->m_SpecialValue[SPECIAL_EXCELLENT6];
	}

	if(gItemOption.GetItemOption(SPECIAL_COMMON1,this) == 0)
	{
		this->m_SpecialIndex[SPECIAL_COMMON1] = 0;
		this->m_SpecialValue[SPECIAL_COMMON1] = 0;
	}
	else
	{
		this->m_SpecialIndex[SPECIAL_COMMON1] = this->m_SpecialIndex[SPECIAL_COMMON1];
		this->m_SpecialValue[SPECIAL_COMMON1] = this->m_SpecialValue[SPECIAL_COMMON1];
	}

	if(gItemOption.GetItemOption(SPECIAL_COMMON2,this) == 0)
	{
		this->m_SpecialIndex[SPECIAL_COMMON2] = 0;
		this->m_SpecialValue[SPECIAL_COMMON2] = 0;
	}
	else
	{
		this->m_SpecialIndex[SPECIAL_COMMON2] = this->m_SpecialIndex[SPECIAL_COMMON2];
		this->m_SpecialValue[SPECIAL_COMMON2] = this->m_SpecialValue[SPECIAL_COMMON2];
	}

	if(gItemOption.GetItemOption(SPECIAL_COMMON3,this) == 0)
	{
		this->m_SpecialIndex[SPECIAL_COMMON3] = 0;
		this->m_SpecialValue[SPECIAL_COMMON3] = 0;
	}
	else
	{
		this->m_SpecialIndex[SPECIAL_COMMON3] = this->m_SpecialIndex[SPECIAL_COMMON3];
		this->m_SpecialValue[SPECIAL_COMMON3] = this->m_SpecialValue[SPECIAL_COMMON3];
	}

	if(gItemOption.GetItemOption(SPECIAL_COMMON4,this) == 0)
	{
		this->m_SpecialIndex[SPECIAL_COMMON4] = 0;
		this->m_SpecialValue[SPECIAL_COMMON4] = 0;
	}
	else
	{
		this->m_SpecialIndex[SPECIAL_COMMON4] = this->m_SpecialIndex[SPECIAL_COMMON4];
		this->m_SpecialValue[SPECIAL_COMMON4] = this->m_SpecialValue[SPECIAL_COMMON4];
	}

	if(gItemOption.GetItemOption(SPECIAL_COMMON5,this) == 0)
	{
		this->m_SpecialIndex[SPECIAL_COMMON5] = 0;
		this->m_SpecialValue[SPECIAL_COMMON5] = 0;
	}
	else
	{
		this->m_SpecialIndex[SPECIAL_COMMON5] = this->m_SpecialIndex[SPECIAL_COMMON5];
		this->m_SpecialValue[SPECIAL_COMMON5] = this->m_SpecialValue[SPECIAL_COMMON5];
	}

	if(gCustomWing.CheckCustomWingByItem(this->m_Index) != 0)
	{
		if(Option2 != 0)
		{
			this->m_Option2 = Option2;
			this->m_SpecialIndex[SPECIAL_OPTION2] = ITEM_OPTION_ADD_CRITICAL_DAMAGE_RATE;
			this->m_SpecialValue[SPECIAL_OPTION2] = 5;
		}

		if(Option3 != 0)
		{
			this->m_Option3 = Option3;
			this->m_SpecialIndex[SPECIAL_OPTION3] = gCustomWing.GetCustomWingOptionIndex(this->m_Index,0);
			this->m_SpecialValue[SPECIAL_OPTION3] = gCustomWing.GetCustomWingOptionValue(this->m_Index,0);

			if((this->m_NewOption & 16) != 0)
			{
				this->m_SpecialIndex[SPECIAL_OPTION3] = gCustomWing.GetCustomWingOptionIndex(this->m_Index,1);
				this->m_SpecialValue[SPECIAL_OPTION3] = gCustomWing.GetCustomWingOptionValue(this->m_Index,1);
			}

			if((this->m_NewOption & 32) != 0)
			{
				this->m_SpecialIndex[SPECIAL_OPTION3] = gCustomWing.GetCustomWingOptionIndex(this->m_Index,2);
				this->m_SpecialValue[SPECIAL_OPTION3] = gCustomWing.GetCustomWingOptionValue(this->m_Index,2);
			}
		}

		if((this->m_NewOption & 1) != 0)
		{
			this->m_SpecialIndex[SPECIAL_EXCELLENT1] = gCustomWing.GetCustomWingNewOptionIndex(this->m_Index,0);
			this->m_SpecialValue[SPECIAL_EXCELLENT1] = gCustomWing.GetCustomWingNewOptionValue(this->m_Index,0);
		}

		if((this->m_NewOption & 2) != 0)
		{
			this->m_SpecialIndex[SPECIAL_EXCELLENT2] = gCustomWing.GetCustomWingNewOptionIndex(this->m_Index,1);
			this->m_SpecialValue[SPECIAL_EXCELLENT2] = gCustomWing.GetCustomWingNewOptionValue(this->m_Index,1);
		}

		if((this->m_NewOption & 4) != 0)
		{
			this->m_SpecialIndex[SPECIAL_EXCELLENT3] = gCustomWing.GetCustomWingNewOptionIndex(this->m_Index,2);
			this->m_SpecialValue[SPECIAL_EXCELLENT3] = gCustomWing.GetCustomWingNewOptionValue(this->m_Index,2);
		}

		if((this->m_NewOption & 8) != 0)
		{
			this->m_SpecialIndex[SPECIAL_EXCELLENT4] = gCustomWing.GetCustomWingNewOptionIndex(this->m_Index,3);
			this->m_SpecialValue[SPECIAL_EXCELLENT4] = gCustomWing.GetCustomWingNewOptionValue(this->m_Index,3);
		}
	}

	this->Value();

	if(this->m_Index != GET_ITEM(4,7) && this->m_Index != GET_ITEM(4,15) && this->IsPentagramItem() == 0) // Arrow,Bolt
	{
		this->m_BaseDamageMin = this->m_DamageMin;
		this->m_BaseDefense = this->m_Defense;
		this->m_DamageMin = (WORD)(this->m_DamageMin*this->m_CurrentDurabilityState);
		this->m_DamageMax = (WORD)(this->m_DamageMax*this->m_CurrentDurabilityState);
		this->m_Defense = (WORD)(this->m_Defense*this->m_CurrentDurabilityState);
		this->m_DefenseSuccessRate = (BYTE)(this->m_DefenseSuccessRate*this->m_CurrentDurabilityState);
	}
}

void CItem::Value() // OK
{
	if(this->IsItem() == 0)
	{
		return;
	}

	ITEM_INFO ItemInfo;

	if(gItemManager.GetInfo(this->m_Index,&ItemInfo) == 0)
	{
		return;
	}

	if (this->Coin1 != 0) this->Coin1 = 0;
	if (this->Coin2 != 0) this->Coin2 = 0;
	if (this->Coin3 != 0) this->Coin3 = 0;

	int value = 0;
	int coin1 = 0;
	int coin2 = 0;
	int coin3 = 0;
	int sell  = 0;

	if(gItemValue.GetItemValueNew(this,&value,&coin1,&coin2,&coin3,&sell) != 0)
	{
		this->m_BuyMoney = value;

		this->m_SellMoney = value/3;

		this->Coin1 = coin1;
		this->Coin2 = coin2;
		this->Coin3 = coin3;

		if (sell > 0)
		{
			this->m_SellMoney = sell;
			this->Sell = 1;
		}
		else
		{
			this->Sell = 0;
		}

		return;
	}

	if(ItemInfo.BuyMoney != 0)
	{
		this->m_BuyMoney = ItemInfo.BuyMoney;

		this->m_SellMoney = (DWORD)(ItemInfo.BuyMoney/3);

		return;
	}

	LONGLONG price = 0;

	if(this->IsPentagramItem() != 0)
	{
		price = ((((this->m_Level*10)*this->m_Level)*(this->m_Level+50))*ItemInfo.RequireLevel)+100000;
	}
	else if(this->IsMuunItem() != 0)
	{
		price = 1500000;
	}
	else if(gCustomJewel.CheckCustomJewelByItem(this->m_Index) != 0)
	{
		price = gCustomJewel.GetCustomJewelSalePrice(this->m_Index);
	}
	else if(ItemInfo.Value > 0)
	{
		price = ((ItemInfo.Value*ItemInfo.Value)*10)/12;

		if(this->m_Index >= GET_ITEM(14,0) && this->m_Index <= GET_ITEM(14,8))
		{
			if(this->m_Index == GET_ITEM(14,3) || this->m_Index == GET_ITEM(14,6))
			{
				price *= 2;
			}

			price *= (LONGLONG)(1 << this->m_Level);

			//price *= (LONGLONG)this->m_Durability;

			price = ((price>MAX_ITEM_PRICE)?MAX_ITEM_PRICE:price);

			if (price <= 0)
			{
				price  = 3;
			}

			this->m_BuyMoney = (DWORD)price;
			this->m_SellMoney = (DWORD)(price/3);

			return;
		}
	}
	else
	{
		int ItemLevel = ItemInfo.Level+(this->m_Level*3);

		for(int n=0;n < MAX_SPECIAL;n++)
		{
			if(n >= SPECIAL_EXCELLENT1 && n <= SPECIAL_EXCELLENT6)
			{
				if(this->m_SpecialIndex[n] != 0 && this->m_Index < GET_ITEM(12,0))
				{
					ItemLevel += 25;
					break;
				}
			}
		}

		if(((this->m_Index/MAX_ITEM_TYPE) == 12 && ((this->m_Index > GET_ITEM(12,6) && this->m_Index < GET_ITEM(12,36)) || this->m_Index > GET_ITEM(12,43)) && this->m_Index != GET_ITEM(12,50) && (this->m_Index < GET_ITEM(12,262) || this->m_Index > GET_ITEM(12,265)) && gCustomWing.CheckCustomWingByItem(this->m_Index) == 0) || (this->m_Index/MAX_ITEM_TYPE) == 13 || (this->m_Index/MAX_ITEM_TYPE) == 15)
		{
			price = ((ItemLevel*ItemLevel)*ItemLevel)+100;
			price = ((this->m_SpecialIndex[SPECIAL_OPTION3]==ITEM_OPTION_ADD_HP_RECOVERY_RATE)?(price+(price*this->m_Option3)):price);
		}
		else
		{
			switch(this->m_Level)
			{
				case 5:
					ItemLevel += 4;
					break;
				case 6:
					ItemLevel += 10;
					break;
				case 7:
					ItemLevel += 25;
					break;
				case 8:
					ItemLevel += 45;
					break;
				case 9:
					ItemLevel += 65;
					break;
				case 10:
					ItemLevel += 95;
					break;
				case 11:
					ItemLevel += 135;
					break;
				case 12:
					ItemLevel += 185;
					break;
				case 13:
					ItemLevel += 245;
					break;
				case 14:
					ItemLevel += 305;
					break;
				case 15:
					ItemLevel += 365;
					break;
			}

			if((this->m_Index >= GET_ITEM(12,0) && this->m_Index <= GET_ITEM(12,6)) || (this->m_Index >= GET_ITEM(12,36) && this->m_Index <= GET_ITEM(12,43)) || this->m_Index == GET_ITEM(12,50) || (this->m_Index >= GET_ITEM(12,262) && this->m_Index <= GET_ITEM(12,265)) || gCustomWing.CheckCustomWingByItem(this->m_Index) != 0) // Wings
			{
				price = ((((ItemLevel+40)*ItemLevel)*ItemLevel)*11)+100; //Default 40000000
			}
			else
			{
				price = ((((ItemLevel+40)*ItemLevel)*ItemLevel)/8)+100;
			}

			if(this->m_Index >= GET_ITEM(0,0) && this->m_Index < GET_ITEM(6,0))
			{
				if(ItemInfo.TwoHand == 0)
				{
					price = (price*80)/100;
				}
			}

			for(int n=0;n < MAX_SPECIAL;n++)
			{
				if(this->m_SpecialIndex[n] != 0)
				{
					switch(n)
					{
						case SPECIAL_OPTION1:
							price += (price*150)/100;
							break;
						case SPECIAL_OPTION2:
							price += (price*25)/100;
							break;
						case SPECIAL_OPTION3:
							price += ((this->m_Option3==1)?((price*60)/100):0);
							price += ((this->m_Option3==2)?((price*140)/100):0);
							price += ((this->m_Option3==3)?((price*280)/100):0);
							price += ((this->m_Option3==4)?((price*560)/100):0);
							break;
						case SPECIAL_EXCELLENT1:
							price += ((this->m_Index<GET_ITEM(12,0))?((price*100)/100):((price*25)/100));
							break;
						case SPECIAL_EXCELLENT2:
							price += ((this->m_Index<GET_ITEM(12,0))?((price*100)/100):((price*25)/100));
							break;
						case SPECIAL_EXCELLENT3:
							price += ((this->m_Index<GET_ITEM(12,0))?((price*100)/100):((price*25)/100));
							break;
						case SPECIAL_EXCELLENT4:
							price += ((this->m_Index<GET_ITEM(12,0))?((price*100)/100):((price*25)/100));
							break;
						case SPECIAL_EXCELLENT5:
							price += ((this->m_Index<GET_ITEM(12,0))?((price*100)/100):((price*25)/100));
							break;
						case SPECIAL_EXCELLENT6:
							price += ((this->m_Index<GET_ITEM(12,0))?((price*100)/100):((price*25)/100));
							break;
					}
				}
			}
		}
	}

	if(this->Is380Item() != 0)
	{
		price += (price*16)/100;
	}

	if(this->IsSocketItem() != 0)
	{
		price += ((price*GetSocketOptionCount(this->m_SocketOption))*80)/100; //original *80
		price += gSocketItemOption.GetSocketItemExtraMoney(this);
	}

	if(this->IsPentagramItem() != 0)
	{
		price += ((price*GetSocketOptionCount(this->m_SocketOption))*80)/100;
	}

	if(this->IsPentagramJewel() != 0)
	{
		int level = 0;

		for(int n=0;n < MAX_SOCKET_OPTION;n++)
		{
			if(this->m_SocketOption[n] != 0xFF)
			{
				level += (this->m_SocketOption[n] & 0xF0)/16;
			}
		}

		level = (level*5)+ItemInfo.RequireLevel;

		price = ((((level/8)*level)*(level+40))+100)*(GetSocketOptionCount(this->m_SocketOption)*2);
	}

	price = ((price>(LONGLONG)2000000000)?(LONGLONG)2000000000:price);

	this->m_BuyMoney = (DWORD)price;

	this->m_SellMoney = (DWORD)(this->m_BuyMoney/3);


	//this->m_SellMoney = ((ItemInfo.Slot>=0&&ItemInfo.Slot<=11)?(this->m_SellMoney-(DWORD)((this->m_SellMoney*0.6)*(1-(this->m_Durability/this->m_BaseDurability)))):this->m_SellMoney);
}

void CItem::OldValue() // OK
{
	if(this->IsItem() == 0)
	{
		return;
	}

	if(this->m_Index == GET_ITEM(14,13)) // Jewel of Bless
	{
		this->m_OldBuyMoney = 100000;
	}
	else if(this->m_Index == GET_ITEM(14,14)) // Jewel of Soul
	{
		this->m_OldBuyMoney = 70000;
	}
	else if(this->m_Index == GET_ITEM(12,15)) // Jewel of Chaos
	{
		this->m_OldBuyMoney = 40000;
	}
	else if(this->m_Index == GET_ITEM(14,22)) // Jewel of Creation
	{
		this->m_OldBuyMoney = 450000;
	}
	else if(this->m_Index == GET_ITEM(14,16)) // Jewel of Life
	{
		this->m_OldBuyMoney = 450000;
	}
	else
	{
		this->m_OldBuyMoney = this->m_BuyMoney;
	}
}

void CItem::PetValue() // OK
{
	int price = this->m_BuyMoney;

	if(this->m_Index == GET_ITEM(13,4))
	{
		price = (this->m_PetItemLevel*2000000);
	}

	if(this->m_Index == GET_ITEM(13,5))
	{
		price = (this->m_PetItemLevel*1000000);
	}

	this->m_BuyMoney = price;

	this->m_BuyMoney = ((this->m_BuyMoney>=100)?((this->m_BuyMoney/10)*10):this->m_BuyMoney);

	this->m_BuyMoney = ((this->m_BuyMoney>=1000)?((this->m_BuyMoney/100)*100):this->m_BuyMoney);

	this->m_SellMoney = price/3;

	this->m_SellMoney = ((this->m_SellMoney>=100)?((this->m_SellMoney/10)*10):this->m_SellMoney);

	this->m_SellMoney = ((this->m_SellMoney>=1000)?((this->m_SellMoney/100)*100):this->m_SellMoney);
}

void CItem::SetPetItemInfo(int PetLevel,int PetExp) // OK
{
	if(this->m_Index == GET_ITEM(13,4)) // Dark Horse
	{
		this->m_LoadPetItemInfo = 1;

		this->m_PetItemLevel = PetLevel;

		this->m_PetItemExp = PetExp;

		this->m_PetItemExp = ((this->m_PetItemExp<gPetItemExp.m_DarkHorseExpTable[this->m_PetItemLevel])?gPetItemExp.m_DarkHorseExpTable[this->m_PetItemLevel]:this->m_PetItemExp);

		this->m_PetItemExp = ((this->m_PetItemExp>=gPetItemExp.m_DarkHorseExpTable[this->m_PetItemLevel+1])?gPetItemExp.m_DarkHorseExpTable[this->m_PetItemLevel]:this->m_PetItemExp);

		this->Convert(this->m_Index,this->m_Option1,this->m_Option2,this->m_Option3,this->m_NewOption,this->m_SetOption,this->m_JewelOfHarmonyOption,this->m_ItemOptionEx,this->m_SocketOption,this->m_SocketOptionBonus);

		this->PetValue();
	}

	if(this->m_Index == GET_ITEM(13,5)) // Dark Reaven
	{
		this->m_LoadPetItemInfo = 1;

		this->m_PetItemLevel = PetLevel;

		this->m_PetItemExp = PetExp;

		this->m_PetItemExp = ((this->m_PetItemExp<gPetItemExp.m_DarkSpiritExpTable[this->m_PetItemLevel])?gPetItemExp.m_DarkSpiritExpTable[this->m_PetItemLevel]:this->m_PetItemExp);

		this->m_PetItemExp = ((this->m_PetItemExp>=gPetItemExp.m_DarkSpiritExpTable[this->m_PetItemLevel+1])?gPetItemExp.m_DarkSpiritExpTable[this->m_PetItemLevel]:this->m_PetItemExp);

		this->Convert(this->m_Index,this->m_Option1,this->m_Option2,this->m_Option3,this->m_NewOption,this->m_SetOption,this->m_JewelOfHarmonyOption,this->m_ItemOptionEx,this->m_SocketOption,this->m_SocketOptionBonus);

		this->PetValue();
	}
}

int CItem::GetDamageMin() // OK
{
	if(this->IsItem() == 0 || this->m_IsValidItem == 0 || this->m_Durability == 0)
	{
		return 0;
	}
	else
	{
		return this->m_DamageMin;
	}
}

int CItem::GetDamageMax() // OK
{
	if(this->IsItem() == 0 || this->m_IsValidItem == 0 || this->m_Durability == 0)
	{
		return 0;
	}
	else
	{
		return this->m_DamageMax;
	}
}

int CItem::GetDefense() // OK
{
	if(this->IsItem() == 0 || this->m_IsValidItem == 0 || this->m_Durability == 0)
	{
		return 0;
	}
	else
	{
		return this->m_Defense;
	}
}

int CItem::GetDefenseSuccessRate() // OK
{
	if(this->IsItem() == 0 || this->m_IsValidItem == 0 || this->m_Durability == 0)
	{
		return 0;
	}
	else
	{
		return this->m_DefenseSuccessRate;
	}
}

int CItem::GetBookSuccessRate() // OK
{
	if(this->IsItem() == 0 || this->m_IsValidItem == 0 || this->m_Durability == 0)
	{
		return 0;
	}
	else
	{
		return (this->m_MagicDamageRate/6);
	}
}

bool CItem::WeaponDurabilityDown(int aIndex,int defense,int type) // OK
{
	if(this->m_Durability < 1 || this->IsLuckyItem() != 0 || this->m_IsPeriodicItem != 0)
	{
		return 0;
	}

	if(this->m_Index == GET_ITEM(4,7) || this->m_Index == GET_ITEM(4,15))
	{
		return 0;
	}

	int damage = 0;

	int MaxDurSmall = 0;

	switch(type)
	{
		case 0:
			damage = this->m_BaseDamageMin+(this->m_BaseDamageMin/2);
			damage = damage+(this->m_Option3*4);
			MaxDurSmall = (564*gServerInfo.m_WeaponDurabilityRate)/100;
			break;
		case 1:
			damage = this->m_BaseDamageMin+(this->m_BaseDamageMin/2);
			damage = damage+(this->m_Option3*4);
			MaxDurSmall = (780*gServerInfo.m_WeaponDurabilityRate)/100;
			break;
		case 2:
			damage = (this->m_MagicDamageRate/2)+(this->m_Level*2);
			damage = damage+(damage/3);
			damage = damage+(this->m_Option3*4);
			MaxDurSmall = (1050*gServerInfo.m_WeaponDurabilityRate)/100;
			break;
		case 3:
			damage = (this->m_MagicDamageRate/2)+(this->m_Level*2);
			damage = damage+(damage/3);
			damage = damage+(this->m_Option3*4);
			MaxDurSmall = (1050*gServerInfo.m_WeaponDurabilityRate)/100;
			break;
	}

	this->m_DurabilitySmall += ((damage==0)?0:(defense/damage));

	MaxDurSmall = (MaxDurSmall*gObj[aIndex].WeaponDurabilityRate)/100;

	if(this->m_DurabilitySmall > MaxDurSmall)
	{
		this->m_Durability = (((--this->m_Durability)<1)?0:this->m_Durability);
		this->m_DurabilitySmall = 0;
		if(this->CheckDurabilityState() != 0){gObjectManager.CharacterCalcAttribute(aIndex);}
		return 1;
	}

	return 0;
}

bool CItem::ArmorDurabilityDown(int aIndex,int damage) // OK
{
	if(this->m_Durability < 1 || this->IsLuckyItem() != 0 || this->m_IsPeriodicItem != 0)
	{
		return 0;
	}

	ITEM_INFO ItemInfo;

	if(gItemManager.GetInfo(this->m_Index,&ItemInfo) == 0)
	{
		return 0;
	}

	int defense = this->m_BaseDefense;

	int MaxDurSmall = (69*gServerInfo.m_ArmorDurabilityRate)/100;

	if(this->m_Index >= GET_ITEM(6,0) && this->m_Index < GET_ITEM(7,0)) // Shield
	{
		defense *= 5;
		defense += this->m_Option3*5;
	}
	else if(ItemInfo.RequireClass[CLASS_DW] != 0)
	{
		defense *= 3;
		defense += this->m_Option3*4;
	}
	else if(ItemInfo.RequireClass[CLASS_DK] != 0)
	{
		defense *= 3;
		defense += this->m_Option3*4;
	}
	else if(ItemInfo.RequireClass[CLASS_FE] != 0)
	{
		defense *= 2;
		defense += this->m_Option3*4;
	}
	else if(ItemInfo.RequireClass[CLASS_MG] != 0)
	{
		defense *= 7;
		defense += this->m_Option3*4;
	}
	else if(ItemInfo.RequireClass[CLASS_DL] != 0)
	{
		defense *= 6;
		defense += this->m_Option3*4;
	}
	else if(ItemInfo.RequireClass[CLASS_SU] != 0)
	{
		defense *= 3;
		defense += this->m_Option3*4;
	}
	else if(ItemInfo.RequireClass[CLASS_RF] != 0)
	{
		defense *= 6;
		defense += this->m_Option3*4;
	}

	this->m_DurabilitySmall += ((defense==0)?0:(damage/defense));

	MaxDurSmall = (MaxDurSmall*gObj[aIndex].ArmorDurabilityRate)/100;

	if(this->m_DurabilitySmall > MaxDurSmall)
	{
		this->m_Durability = (((--this->m_Durability)<1)?0:this->m_Durability);
		this->m_DurabilitySmall = 0;
		if(this->CheckDurabilityState() != 0){gObjectManager.CharacterCalcAttribute(aIndex);}
		return 1;
	}

	return 0;
}

bool CItem::WingDurabilityDown(int aIndex,int decrease) // OK
{
	if(this->m_Durability < 1 || this->IsLuckyItem() != 0 || this->m_IsPeriodicItem != 0)
	{
		return 0;
	}

	this->m_DurabilitySmall += decrease;

	int MaxDurSmall = (564*gServerInfo.m_WingDurabilityRate)/100;

	MaxDurSmall = (MaxDurSmall*gObj[aIndex].WingDurabilityRate)/100;

	if(this->m_DurabilitySmall > MaxDurSmall)
	{
		this->m_Durability = (((--this->m_Durability)<1)?0:this->m_Durability);
		this->m_DurabilitySmall = 0;
		if(this->CheckDurabilityState() != 0){gObjectManager.CharacterCalcAttribute(aIndex);}
		return 1;
	}

	return 0;
}

bool CItem::PendantDurabilityDown(int aIndex,int decrease) // OK
{
	if(this->m_Durability < 1 || this->IsLuckyItem() != 0 || this->m_IsPeriodicItem != 0)
	{
		return 0;
	}

	this->m_DurabilitySmall += decrease;

	int MaxDurSmall = (564*gServerInfo.m_PendantDurabilityRate)/100;

	MaxDurSmall = (MaxDurSmall*gObj[aIndex].PendantDurabilityRate)/100;

	if(this->m_DurabilitySmall > MaxDurSmall)
	{
		this->m_Durability = (((--this->m_Durability)<1)?0:this->m_Durability);
		this->m_DurabilitySmall = 0;
		if(this->CheckDurabilityState() != 0){gObjectManager.CharacterCalcAttribute(aIndex);}
		return 1;
	}

	return 0;
}

bool CItem::RingDurabilityDown(int aIndex,int decrease) // OK
{
	if(this->m_Durability < 1 || this->IsLuckyItem() != 0 || this->m_IsPeriodicItem != 0)
	{
		return 0;
	}

	this->m_DurabilitySmall += decrease;

	int MaxDurSmall = (564*gServerInfo.m_RingDurabilityRate)/100;

	MaxDurSmall = (MaxDurSmall*gObj[aIndex].RingDurabilityRate)/100;

	if(this->m_DurabilitySmall > MaxDurSmall)
	{
		this->m_Durability = (((--this->m_Durability)<1)?0:this->m_Durability);
		this->m_DurabilitySmall = 0;
		if(this->CheckDurabilityState() != 0){gObjectManager.CharacterCalcAttribute(aIndex);}
		return 1;
	}

	return 0;
}

bool CItem::LuckyDurabilityDown(int aIndex,int decrease) // OK
{
	if(this->m_Durability < 1 || this->IsLuckyItem() == 0)
	{
		return 0;
	}

	this->m_DurabilitySmall += decrease;

	int MaxDurSmall = 0;

	gLuckyItem.GetLuckyItemDecay(this->m_Index,&MaxDurSmall);

	if(this->m_DurabilitySmall > MaxDurSmall)
	{
		this->m_Durability = (((--this->m_Durability)<1)?0:this->m_Durability);
		this->m_DurabilitySmall = 0;
		if(this->CheckDurabilityState() != 0){gObjectManager.CharacterCalcAttribute(aIndex);}
		return 1;
	}

	return 0;
}

bool CItem::CheckDurabilityState() // OK
{
	if(this->m_Durability == 0)
	{
		if(this->m_CurrentDurabilityState != 0.0)
		{
			this->m_CurrentDurabilityState = (float)0.0;
			this->Convert(this->m_Index,this->m_Option1,this->m_Option2,this->m_Option3,this->m_NewOption,this->m_SetOption,this->m_JewelOfHarmonyOption,this->m_ItemOptionEx,this->m_SocketOption,this->m_SocketOptionBonus);
			return 1;
		}
	}
	else if(this->m_Durability < this->m_DurabilityState[3])
	{
		if(this->m_CurrentDurabilityState != 0.5)
		{
			this->m_CurrentDurabilityState = (float)0.5;
			this->Convert(this->m_Index,this->m_Option1,this->m_Option2,this->m_Option3,this->m_NewOption,this->m_SetOption,this->m_JewelOfHarmonyOption,this->m_ItemOptionEx,this->m_SocketOption,this->m_SocketOptionBonus);
			return 1;
		}
	}
	else if(this->m_Durability < this->m_DurabilityState[2])
	{
		if(this->m_CurrentDurabilityState != 0.6)
		{
			this->m_CurrentDurabilityState = (float)0.6;
			this->Convert(this->m_Index,this->m_Option1,this->m_Option2,this->m_Option3,this->m_NewOption,this->m_SetOption,this->m_JewelOfHarmonyOption,this->m_ItemOptionEx,this->m_SocketOption,this->m_SocketOptionBonus);
			return 1;
		}
	}
	else if(this->m_Durability < this->m_DurabilityState[1])
	{
		if(this->m_CurrentDurabilityState != 0.7)
		{
			this->m_CurrentDurabilityState = (float)0.7;
			this->Convert(this->m_Index,this->m_Option1,this->m_Option2,this->m_Option3,this->m_NewOption,this->m_SetOption,this->m_JewelOfHarmonyOption,this->m_ItemOptionEx,this->m_SocketOption,this->m_SocketOptionBonus);
			return 1;
		}
	}
	else if(this->m_Durability < this->m_DurabilityState[0])
	{
		if(this->m_CurrentDurabilityState != 0.8)
		{
			this->m_CurrentDurabilityState = (float)0.8;
			this->Convert(this->m_Index,this->m_Option1,this->m_Option2,this->m_Option3,this->m_NewOption,this->m_SetOption,this->m_JewelOfHarmonyOption,this->m_ItemOptionEx,this->m_SocketOption,this->m_SocketOptionBonus);
			return 1;
		}
	}
	else
	{
		if(this->m_CurrentDurabilityState != 1.0)
		{
			this->m_CurrentDurabilityState = (float)1.0;
			this->Convert(this->m_Index,this->m_Option1,this->m_Option2,this->m_Option3,this->m_NewOption,this->m_SetOption,this->m_JewelOfHarmonyOption,this->m_ItemOptionEx,this->m_SocketOption,this->m_SocketOptionBonus);
			return 1;
		}
	}

	return 0;
}

bool CItem::AddPetItemExp(int amount) // OK
{
	if(this->m_Durability == 0 || this->m_LoadPetItemInfo == 0)
	{
		return 0;
	}

	if(this->m_PetItemLevel < 1)
	{
		this->m_PetItemLevel = 1;
		this->m_PetItemExp = 0;
		this->Convert(this->m_Index,this->m_Option1,this->m_Option2,this->m_Option3,this->m_NewOption,this->m_SetOption,this->m_JewelOfHarmonyOption,this->m_ItemOptionEx,this->m_SocketOption,this->m_SocketOptionBonus);
		this->PetValue();
		return 1;
	}

	if(this->m_Index == GET_ITEM(13,4)) // Dark Horse
	{
		if(this->m_PetItemLevel >= MAX_PET_LEVEL)
		{
			return 0;
		}

		if((this->m_PetItemExp+amount) >= gPetItemExp.m_DarkHorseExpTable[this->m_PetItemLevel+1])
		{
			this->m_PetItemLevel++;
			this->m_PetItemExp = gPetItemExp.m_DarkHorseExpTable[this->m_PetItemLevel];
			this->Convert(this->m_Index,this->m_Option1,this->m_Option2,this->m_Option3,this->m_NewOption,this->m_SetOption,this->m_JewelOfHarmonyOption,this->m_ItemOptionEx,this->m_SocketOption,this->m_SocketOptionBonus);
			this->PetValue();
			return 1;
		}
	}

	if(this->m_Index == GET_ITEM(13,5)) // Dark Reaven
	{
		if(this->m_PetItemLevel >= MAX_PET_LEVEL)
		{
			return 0;
		}

		if((this->m_PetItemExp+amount) >= gPetItemExp.m_DarkSpiritExpTable[this->m_PetItemLevel+1])
		{
			this->m_PetItemLevel++;
			this->m_PetItemExp = gPetItemExp.m_DarkSpiritExpTable[this->m_PetItemLevel];
			this->Convert(this->m_Index,this->m_Option1,this->m_Option2,this->m_Option3,this->m_NewOption,this->m_SetOption,this->m_JewelOfHarmonyOption,this->m_ItemOptionEx,this->m_SocketOption,this->m_SocketOptionBonus);
			this->PetValue();
			return 1;
		}
	}

	this->m_PetItemExp += amount;
	return 0;
}

bool CItem::DecPetItemExp(int amount) // OK
{
	if(this->m_Durability == 0 || this->m_LoadPetItemInfo == 0)
	{
		return 0;
	}

	if(this->m_PetItemLevel < 1)
	{
		this->m_PetItemLevel = 1;
		this->m_PetItemExp = 0;
		this->Convert(this->m_Index,this->m_Option1,this->m_Option2,this->m_Option3,this->m_NewOption,this->m_SetOption,this->m_JewelOfHarmonyOption,this->m_ItemOptionEx,this->m_SocketOption,this->m_SocketOptionBonus);
		this->PetValue();
		return 1;
	}

	if(this->m_Index == GET_ITEM(13,4)) // Dark Horse
	{
		if((this->m_PetItemExp-amount) < gPetItemExp.m_DarkHorseExpTable[this->m_PetItemLevel])
		{
			this->m_PetItemLevel--;
			this->m_PetItemExp = gPetItemExp.m_DarkHorseExpTable[this->m_PetItemLevel+1]-1;
			this->Convert(this->m_Index,this->m_Option1,this->m_Option2,this->m_Option3,this->m_NewOption,this->m_SetOption,this->m_JewelOfHarmonyOption,this->m_ItemOptionEx,this->m_SocketOption,this->m_SocketOptionBonus);
			this->PetValue();
			return 1;
		}
	}

	if(this->m_Index == GET_ITEM(13,5)) // Dark Reaven
	{
		if((this->m_PetItemExp-amount) < gPetItemExp.m_DarkSpiritExpTable[this->m_PetItemLevel])
		{
			this->m_PetItemLevel--;
			this->m_PetItemExp = gPetItemExp.m_DarkSpiritExpTable[this->m_PetItemLevel+1]-1;
			this->Convert(this->m_Index,this->m_Option1,this->m_Option2,this->m_Option3,this->m_NewOption,this->m_SetOption,this->m_JewelOfHarmonyOption,this->m_ItemOptionEx,this->m_SocketOption,this->m_SocketOptionBonus);
			this->PetValue();
			return 1;
		}
	}

	this->m_PetItemExp -= amount;
	return 0;
}

//MC bot
BOOL CItem::IsClassBot(char aClass, int ChangeUP)
{
	int addChangeUp = 0;

	if ((aClass < 0) || (aClass >= MAX_TYPE_PLAYER_BOTS ))
	{
		LogAdd(LOG_BLUE,"Wrong Class:%d (%s %d)", aClass, __FILE__, __LINE__); // 클래스 에러 : %d (%s %d)
		return 0;
	}
	
	int requireclass=this->m_RequireClass[aClass];
	if (requireclass == 0)
	{
		return 0;
	}

	if (ChangeUP > 0)
	{
		if ((aClass == CLASS_DARKLORD) || (aClass == CLASS_MAGICGLADIATOR) || (aClass == CLASS_RAGEFIGHTER))
			addChangeUp = 2;
		else
			addChangeUp = 1;
	}

	if (requireclass > 1)
	{
		if ( requireclass > (ChangeUP+addChangeUp) )
		{
			return 0;
		}
	}
	return 1;
}
//MC bot

LPSTR CItem::GetName()
{
	return &ItemAttribute[this->m_Index].Name[32];
}