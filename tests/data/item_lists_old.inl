// One-time copy of the lucky item, transformation ring and potion checks before phase 3b-4
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
