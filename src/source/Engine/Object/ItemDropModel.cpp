#include "stdafx.h"

#include "Engine/Object/ItemDropModel.h"

#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "GameLogic/Items/ItemCategories.h"

namespace Engine::Items::Drops
{
int DrawnModel(int itemType, int level)
{
    int model = Data::Items::ToModelSlot(itemType);
    if (itemType == ITEM_BOX_OF_LUCK)
    {
        switch (level)
        {
        case 1:
            model = MODEL_EVENT + 4;
            break;
        case 2:
            model = MODEL_EVENT + 5;
            break;
        case 3:
            model = MODEL_EVENT + 6;
            break;
        case 5:
            model = MODEL_EVENT + 8;
            break;
        case 6:
            model = MODEL_EVENT + 9;
            break;
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            model = MODEL_EVENT + 10;
            break;
        case 13:
            model = MODEL_EVENT + 6;
            break;
        case 14:
        case 15:
            model = MODEL_EVENT + 5;
        }
    }
    if (itemType >= ITEM_JACK_OLANTERN_BLESSINGS && itemType <= ITEM_JACK_OLANTERN_CRY)
    {
        model = Data::Items::ToModelSlot(itemType);
    }
    if (GameLogic::Items::IsChocolateBoxType(itemType))
    {
        if (level == 1)
        {
            const int box = itemType - ITEM_PINK_CHOCOLATE_BOX;
            model = MODEL_EVENT + 21 + box;
        }
    }
    else if (itemType == ITEM_RENA)
    {
        switch (level)
        {
        case 1:
        case 2:
            model = MODEL_EVENT + 11;
            break;
        }
    }
    else if (itemType == ITEM_WEAPON_OF_ARCHANGEL)
    {
        // The level tells the three weapons apart: the item keeps it (its name), the model is drawn at +0
        // (DrawnItemLevel in ZzzObject.cpp).
        switch (level)
        {
        case 0:
            model = MODEL_DIVINE_STAFF_OF_ARCHANGEL;
            break;
        case 1:
            model = MODEL_DIVINE_SWORD_OF_ARCHANGEL;
            break;
        case 2:
            model = MODEL_DIVINE_CB_OF_ARCHANGEL;
            break;
        }
    }
    else if (itemType == ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR)
    {
        switch (level)
        {
        case 1:
            model = MODEL_EVENT + 12;
            break;
        }
    }
    else if (itemType == ITEM_BROKEN_SWORD_DARK_STONE)
    {
        switch (level)
        {
        case 1:
            model = MODEL_EVENT + 13;
            break;
        }
    }
    else if (itemType == ITEM_WIZARDS_RING)
    {
        switch (level)
        {
        case 0:
            model = MODEL_EVENT + 15;
            break;
        case 1:
        case 2:
        case 3:
            model = MODEL_EVENT + 14;
            break;
        }
    }
    else if (itemType == ITEM_ALE)
    {
        switch (level)
        {
        case 1:
            model = MODEL_EVENT + 7;
            break;
        }
    }
    else if (itemType == ITEM_LOCHS_FEATHER && level == 1)
    {
        model = MODEL_EVENT + 16;
    }
    else if (itemType == ITEM_LIFE_STONE_ITEM && level == 1)
    {
        model = MODEL_EVENT + 18;
    }
    else if (itemType == ITEM_GEMSTONE)
    {
        model = MODEL_GEMSTONE;
    }
    else if (itemType == ITEM_JEWEL_OF_HARMONY)
    {
        model = MODEL_JEWEL_OF_HARMONY;
    }
    else if (itemType == ITEM_LOWER_REFINE_STONE)
    {
        model = MODEL_LOWER_REFINE_STONE;
    }
    else if (itemType == ITEM_HIGHER_REFINE_STONE)
    {
        model = MODEL_HIGHER_REFINE_STONE;
    }
    return model;
}
} // namespace Engine::Items::Drops
