// One-time copy of the lucky item, transformation ring, potion and hotkey checks before phase 3b-4
// (see test_item_lists_equivalence.cpp).

// _define.h and ZzzInventory.cpp, Check_LuckyItem. In a namespace so that the constants
// cannot clash with the originals.
namespace OldLuckySet
{
// Lucky Set armor constants - verified as indices 62-72 (Phoenix Soul is at 73)
constexpr int LUCKY_SET_ARMOR_START_INDEX = 62;
constexpr int LUCKY_SET_ARMOR_COUNT = 11;  // Indices 62-72

bool OldCheck_LuckyItem(int _nIndex, int _nType = 0)
{
    int		nItemTabIndex = (_nIndex + _nType) % MAX_ITEM_INDEX;

    if (_nIndex < ITEM_HELM || _nIndex > ITEM_WING)	return false;
    if (nItemTabIndex >= LUCKY_SET_ARMOR_START_INDEX
        && nItemTabIndex < LUCKY_SET_ARMOR_START_INDEX + LUCKY_SET_ARMOR_COUNT)
        return true;

    return false;
}
} // namespace OldLuckySet

// LuckyItemWnd.cpp, CLuckyItemWnd::Check_LuckyItem_Refinery.
bool OldCheck_LuckyItem_Refinery(ITEM* _pItem)
{
    if (_pItem->Type >= ITEM_ARMOR + 62 && _pItem->Type <= ITEM_ARMOR + 72)		return true;
    else if (_pItem->Type >= ITEM_HELM + 62 && _pItem->Type <= ITEM_HELM + 72)		return true;
    else if (_pItem->Type >= ITEM_BOOTS + 62 && _pItem->Type <= ITEM_BOOTS + 72)		return true;
    else if (_pItem->Type >= ITEM_GLOVES + 62 && _pItem->Type <= ITEM_GLOVES + 72)	return true;
    else if (_pItem->Type >= ITEM_PANTS + 62 && _pItem->Type <= ITEM_PANTS + 72)		return true;

    return false;
}

// ChangeRingManager.cpp: the transformation rings and the checks that read them.
template <typename Container, typename T> constexpr bool OldContains(const Container& container, const T& value)
{
    for (const auto& item : container)
    {
        if (item == value)
        {
            return true;
        }
    }
    return false;
}

constexpr std::array<short, 8> kChangeRingTypes{
    ITEM_TRANSFORMATION_RING,           ITEM_ELITE_TRANSFER_SKELETON_RING,    ITEM_JACK_OLANTERN_TRANSFORMATION_RING,
    ITEM_CHRISTMAS_TRANSFORMATION_RING, ITEM_GAME_MASTER_TRANSFORMATION_RING, ITEM_SNOWMAN_TRANSFORMATION_RING,
    ITEM_PANDA_TRANSFORMATION_RING,     ITEM_SKELETON_TRANSFORMATION_RING,
};

constexpr std::array<short, 6> kIcarusBanRingTypes{
    ITEM_TRANSFORMATION_RING,           ITEM_ELITE_TRANSFER_SKELETON_RING,    ITEM_JACK_OLANTERN_TRANSFORMATION_RING,
    ITEM_CHRISTMAS_TRANSFORMATION_RING, ITEM_GAME_MASTER_TRANSFORMATION_RING, ITEM_SNOWMAN_TRANSFORMATION_RING,
};

bool OldCheckChangeRing(short RingType)
{
    return OldContains(kChangeRingTypes, RingType);
}

bool OldCheckMoveMap(short sLeftRingType, short sRightRingType)
{
    return OldContains(kChangeRingTypes, sLeftRingType) || OldContains(kChangeRingTypes, sRightRingType);
}

bool OldCheckBanMoveIcarusMap(short sLeftRingType, short sRightRingType)
{
    return OldContains(kIcarusBanRingTypes, sLeftRingType) || OldContains(kIcarusBanRingTypes, sRightRingType);
}

// MyInventory.cpp, CMyInventory::FindItemIndexWithTagHighestFirst: the order in which it
// looks for the items of a tag.
std::vector<int> OldItemTypesWithTagHighestFirst(Data::Items::ItemTag tag)
{
    const auto itemTypes = g_ItemDatabase.GetItemTypesWithTag(tag);
    return std::vector<int>(itemTypes.rbegin(), itemTypes.rend());
}

// ItemHotKey.cpp, CItemHotKey::GetHotKeyCommonItem and the start of GetHotKeyItemIndex, with
// the bound item type and level as parameters instead of members. OldHotKeyItemTypes gives
// the item types of the loop that follows, from iStartItemType down to iEndItemType.
namespace OldHotKey
{
using namespace mu::ui::window;

bool OldGetHotKeyCommonItem(int boundType, int boundLevel, int& iStart, int& iEnd)
{
    switch (boundType)
    {
    case ITEM_SIEGE_POTION:
    case ITEM_ANTIDOTE:
    case ITEM_ALE:
    case ITEM_TOWN_PORTAL_SCROLL:
    case ITEM_REMEDY_OF_LOVE:
    case ITEM_JACK_OLANTERN_BLESSINGS:
    case ITEM_JACK_OLANTERN_WRATH:
    case ITEM_JACK_OLANTERN_CRY:
    case ITEM_JACK_OLANTERN_FOOD:
    case ITEM_JACK_OLANTERN_DRINK:
    case ITEM_ELITE_HEALING_POTION:
    case ITEM_ELITE_MANA_POTION:
    case ITEM_ELIXIR_OF_STRENGTH:
    case ITEM_ELIXIR_OF_AGILITY:
    case ITEM_ELIXIR_OF_HEALTH:
    case ITEM_ELIXIR_OF_ENERGY:
    case ITEM_ELIXIR_OF_CONTROL:
    case ITEM_MEDIUM_ELITE_HEALING_POTION:
    case ITEM_CHERRY_BLOSSOM_WINE:
    case ITEM_CHERRY_BLOSSOM_RICE_CAKE:
    case ITEM_CHERRY_BLOSSOM_FLOWER_PETAL:
    case ITEM_ELITE_SD_POTION:
        if (boundType != ITEM_REMEDY_OF_LOVE || boundLevel == 0)
        {
            iStart = iEnd = boundType;
            return true;
        }
        break;
    default:
        if (boundType >= ITEM_SMALL_SHIELD_POTION && boundType <= ITEM_LARGE_SHIELD_POTION)
        {
            iStart = ITEM_LARGE_SHIELD_POTION; iEnd = ITEM_SMALL_SHIELD_POTION;
            return true;
        }
        else if (GameLogic::Items::IsComplexPotionType(boundType))
        {
            iStart = ITEM_LARGE_COMPLEX_POTION; iEnd = ITEM_SMALL_COMPLEX_POTION;
            return true;
        }
        break;
    }
    return false;
}

std::vector<int> OldHotKeyItemTypes(int iType, int boundType, int boundLevel)
{
    int iStartItemType = 0, iEndItemType = 0;

    switch (iType)
    {
    case HOTKEY_Q:
        if (OldGetHotKeyCommonItem(boundType, boundLevel, iStartItemType, iEndItemType) == false)
        {
            if (GameLogic::Items::IsManaPotionType(boundType))
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
            else
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
        }
        break;
    case HOTKEY_W:
        if (OldGetHotKeyCommonItem(boundType, boundLevel, iStartItemType, iEndItemType) == false)
        {
            if (GameLogic::Items::IsHealingPotionType(boundType))
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
            else
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
        }
        break;
    case HOTKEY_E:
        if (OldGetHotKeyCommonItem(boundType, boundLevel, iStartItemType, iEndItemType) == false)
        {
            if (GameLogic::Items::IsHealingPotionType(boundType))
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
            else if (GameLogic::Items::IsManaPotionType(boundType))
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
            else
            {
                iStartItemType = ITEM_ANTIDOTE; iEndItemType = ITEM_ANTIDOTE;
            }
        }
        break;
    case HOTKEY_R:
        if (OldGetHotKeyCommonItem(boundType, boundLevel, iStartItemType, iEndItemType) == false)
        {
            if (GameLogic::Items::IsHealingPotionType(boundType))
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
            else if (GameLogic::Items::IsManaPotionType(boundType))
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
            else
            {
                iStartItemType = ITEM_LARGE_SHIELD_POTION; iEndItemType = ITEM_SMALL_SHIELD_POTION;
            }
        }
        break;
    }

    std::vector<int> itemTypes;
    for (int i = iStartItemType; i >= iEndItemType; --i)
    {
        itemTypes.push_back(i);
    }
    return itemTypes;
}
} // namespace OldHotKey
