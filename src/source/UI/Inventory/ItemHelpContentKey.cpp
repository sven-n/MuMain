#include "stdafx.h"

#include "UI/Inventory/ItemHelpContentKey.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Engine/Object/ZzzCharacter.h"
#include "I18N/All.h"

mu::ui::window::ItemHelpContentKey mu::ui::window::GetItemHelpContentKey(int itemType)
{
    return {itemType, g_ItemDatabase.GetVersion(), I18N::LocaleIndex(I18N::GetCurrentLocale()), Hero->Class,
            GetItemInfoHeroStats()};
}
