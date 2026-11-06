
#include "stdafx.h"
#include "UI/Inventory/ItemMng.h"
#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/Pets/GIPetManager.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Network/Server/SocketSystem.h"

#include <algorithm>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// The byte at `index` of an item's data, 0 past its end: an item cut short reads as one without the missing options.
BYTE ItemByteAt(std::span<const BYTE> itemData, size_t index)
{
    return index < itemData.size() ? itemData[index] : 0;
}

// The socket byte at `index` (the bonus option in the high half, the socket count in the low half) and one byte per
// socket after it. The count has room for 15, but an item has at most MAX_SOCKETS sockets: the first ones are kept, and
// no more than the bytes there are.
void ReadSockets(std::span<const BYTE> itemData, size_t index, ItemCreationParams& params)
{
    const BYTE socketByte = ItemByteAt(itemData, index);
    params.SocketBonusOption = (socketByte >> 4) & 0xF;

    const size_t first = index + 1;
    const size_t present = itemData.size() > first ? itemData.size() - first : 0;
    const size_t count = std::min({static_cast<size_t>(socketByte & 0xF), static_cast<size_t>(MAX_SOCKETS), present});
    params.SocketCount = static_cast<BYTE>(count);
    for (size_t i = 0; i < count; ++i)
    {
        params.SocketOptions[i] = itemData[first + i];
    }
}
} // namespace

ItemCreationParams ParseItemData(std::span<const BYTE> itemData)
{
    ItemCreationParams params = {};

    if (itemData.size() < 5)
        return params;

    params.Group = (itemData[0] >> 4) & 0xF;
    params.Number = ((itemData[0] & 0xF) << 8) + itemData[1];
    params.Level = itemData[2];
    params.Durability = itemData[3];
    auto flags = static_cast<ItemOptionFlags>(itemData[4]);
    params.WithLuck = flags & ItemOptionFlags::HasLuck;
    params.WithSkill = flags & ItemOptionFlags::HasSkill;

    size_t offset = 0;
    if (flags & ItemOptionFlags::HasOption)
    {
        const BYTE optionByte = ItemByteAt(itemData, 5);
        params.OptionLevel = optionByte & 0xF;
        params.OptionType = (optionByte >> 4) & 0xF;
        offset++;
    }

    if (flags & ItemOptionFlags::HasExcellent)
    {
        params.ExcellentFlags = ItemByteAt(itemData, 5 + offset);
        offset++;
    }

    if (flags & ItemOptionFlags::HasAncient)
    {
        const BYTE ancientByte = ItemByteAt(itemData, 5 + offset);
        params.AncientDiscriminator = ancientByte & 0xF;
        params.AncientBonusOption = (ancientByte >> 4) & 0xF;
        offset++;
    }

    if (flags & ItemOptionFlags::HasHarmony)
    {
        const BYTE harmonyByte = ItemByteAt(itemData, 5 + offset);
        params.HasHarmonyOption = true;
        params.HarmonyOptionLevel = harmonyByte & 0xF;
        params.HarmonyOptionType = (harmonyByte >> 4) & 0xF;
        offset++;
    }

    // The guardian (380) option is only this flag: no byte of its own follows.
    params.HasGuardianOption = flags & ItemOptionFlags::HasGuardian;

    if (flags & ItemOptionFlags::HasSockets)
    {
        ReadSockets(itemData, 5 + offset, params);
    }

    return params;
}

mu::ui::window::CItemMng::CItemMng()
{
    m_dwAlternate = 0;
    m_dwAvailableKeyStream = 0x80000000;
    m_UpdateTimer.SetTimer(1000);
}

mu::ui::window::CItemMng::~CItemMng()
{
    DeleteAllItems();
}

ITEM* mu::ui::window::CItemMng::CreateItem(std::span<const BYTE> itemData)
{

    return CreateItemExtended(itemData);
}

/// <summary>
/// Layout:
///   Group:  4 bit
///   Number: 12 bit
///   Level:  8 bit
///   Dura:   8 bit
///   OptFlags: 8 bit
///     HasOpt
///     HasLuck
///     HasSkill
///     HasExc
///     HasAnc
///     HasHarmony
///     HasGuardian (no byte of its own)
///     HasSockets
///   Optional, depending on Flags:
///     Opt_Lvl 4 bit
///     Opt_Typ 4 bit
///     Exc:    8 bit
///     Anc_Dis 4 bit
///     Anc_Bon 4 bit
///     Harmony 8 bit
///     Soc_Bon 4 bit
///     Soc_Cnt 4 bit
///     Sockets n * 8 bit
///
///  Total: 5 ~ 15 bytes.
/// </summary>
ITEM* mu::ui::window::CItemMng::CreateItemExtended(std::span<const BYTE> itemData)
{
    if (itemData.size() < 5)
        return nullptr;

    ItemCreationParams params = ParseItemData(itemData);
    auto item = CItemMng::CreateItemByParameters(&params);
    return item;
}

ITEM* mu::ui::window::CItemMng::CreateItemOld(std::span<const BYTE> pbyItemPacket)
{
    WORD wType = ExtractItemType(pbyItemPacket);
    BYTE byOption380 = 0, byOptionHarmony = 0;

    byOption380 = pbyItemPacket[5];
    byOptionHarmony = pbyItemPacket[6];

    BYTE bySocketOption[5] = {pbyItemPacket[7], pbyItemPacket[8], pbyItemPacket[9], pbyItemPacket[10],
                              pbyItemPacket[11]};

    return CItemMng::CreateItem(wType / MAX_ITEM_INDEX, wType % MAX_ITEM_INDEX, pbyItemPacket[1], pbyItemPacket[2],
                                     pbyItemPacket[3], pbyItemPacket[4], byOption380, byOptionHarmony, bySocketOption);
}

void FillItemFromParameters(ITEM& item, const ItemCreationParams& parameters)
{
    memset(&item, 0, sizeof(ITEM));
    item.bPeriodItem = parameters.WithExpiration;
    item.bExpiredPeriod = parameters.IsExpired;
    // item.lExpireTime is received by another packet? should we integrate that?
    item.Type = parameters.Group * MAX_ITEM_INDEX + parameters.Number;
    item.Level = parameters.Level;
    item.Durability = parameters.Durability;
    item.HasLuck = parameters.WithLuck;
    item.HasSkill = parameters.WithSkill;
    item.OptionType = parameters.OptionType;
    item.OptionLevel = parameters.OptionLevel;
    item.ExcellentFlags = parameters.ExcellentFlags;
    item.AncientDiscriminator = parameters.AncientDiscriminator;
    item.AncientBonusOption = parameters.AncientBonusOption;
    item.Jewel_Of_Harmony_Option = parameters.HarmonyOptionType;
    item.Jewel_Of_Harmony_OptionLevel = parameters.HarmonyOptionLevel;
    item.option_380 = parameters.HasGuardianOption;
    item.SocketCount = parameters.SocketCount;
    item.SocketSeedSetOption = parameters.SocketBonusOption;
    for (int i = 0; i < MAX_SOCKETS; ++i)
    {
        item.bySocketOption[i] = parameters.SocketOptions[i];
        if (item.bySocketOption[i] == SOCKET_EMPTY)
        {
            item.SocketSeedID[i] = SOCKET_EMPTY;
        }
        else
        {
            item.SocketSeedID[i] = item.bySocketOption[i] % SEASON4A::MAX_SOCKET_OPTION;
            item.SocketSphereLv[i] = (item.bySocketOption[i] / SEASON4A::MAX_SOCKET_OPTION) + 1;
        }
    }

    item.byColorState = ITEM_COLOR_NORMAL;

    SetItemAttributes(&item);
}

ITEM* mu::ui::window::CItemMng::CreateItemByParameters(const ItemCreationParams* parameters)
{
    if (parameters == nullptr)
    {
        return nullptr;
    }

    ITEM* pNewItem = new ITEM;
    FillItemFromParameters(*pNewItem, *parameters);
    pNewItem->RefCount = 1;
    pNewItem->Key = GenerateItemKey();

    m_listItem.push_back(pNewItem);
    return pNewItem;
}

ITEM* mu::ui::window::CItemMng::CreateItem(BYTE byType, BYTE bySubType, BYTE byLevel /* = 0 */,
                                          BYTE byDurability /* = 255 */, BYTE byOption1 /* = 0 */,
                                          BYTE ancientByte /* = 0 */, BYTE byOption380 /* = 0 */,
                                          BYTE byOptionHarmony /* = 0 */, BYTE* pbySocketOptions /*= NULL*/)
{
    ITEM* pNewItem = new ITEM;
    memset(pNewItem, 0, sizeof(ITEM));

    WORD wType = byType * MAX_ITEM_INDEX + bySubType;
    pNewItem->Key = GenerateItemKey();
    pNewItem->Type = wType;
    pNewItem->Durability = byDurability;
    pNewItem->ExcellentFlags = byOption1;
    pNewItem->AncientDiscriminator = ancientByte & 0x03;
    pNewItem->AncientBonusOption = (ancientByte & (0x04 + 0x08)) >> 2;
    if ((((byOption380 & 0x08) << 4) >> 7) > 0)
        pNewItem->option_380 = true;
    else
        pNewItem->option_380 = false;
    pNewItem->Jewel_Of_Harmony_Option = (byOptionHarmony & 0xf0) >> 4;
    pNewItem->Jewel_Of_Harmony_OptionLevel = byOptionHarmony & 0x0f;

    if (pbySocketOptions == NULL)
    {
        pNewItem->SocketCount = 0;
        assert(!"Socket options expected but missing");
    }
    else
    {
        pNewItem->SocketCount = MAX_SOCKETS;

        for (int i = 0; i < MAX_SOCKETS; ++i)
        {
            pNewItem->bySocketOption[i] = pbySocketOptions[i];
        }

        for (int i = 0; i < MAX_SOCKETS; ++i)
        {
            if (pbySocketOptions[i] == 0xFF)
            {
                pNewItem->SocketCount = i;
                break;
            }
            else if (pbySocketOptions[i] == 0xFE)
            {
                pNewItem->SocketSeedID[i] = SOCKET_EMPTY;
            }
            else
            {
                pNewItem->SocketSeedID[i] = pbySocketOptions[i] % SEASON4A::MAX_SOCKET_OPTION;
                pNewItem->SocketSphereLv[i] = int(pbySocketOptions[i] / SEASON4A::MAX_SOCKET_OPTION) + 1;
            }
        }

        if (g_SocketItemMgr.IsSocketItem(pNewItem))
        {
            pNewItem->SocketSeedSetOption = byOptionHarmony;
            pNewItem->Jewel_Of_Harmony_Option = 0;
            pNewItem->Jewel_Of_Harmony_OptionLevel = 0;
        }
        else
        {
            pNewItem->SocketSeedSetOption = SOCKET_EMPTY;
        }
    }

    pNewItem->byColorState = ITEM_COLOR_NORMAL;

    pNewItem->RefCount = 1;

    if (((byOption380 & 0x02) >> 1) > 0)
    {
        pNewItem->bPeriodItem = true;
    }
    else
    {
        pNewItem->bPeriodItem = false;
    }

    if (((byOption380 & 0x04) >> 2) > 0)
    {
        pNewItem->bExpiredPeriod = true;
    }
    else
    {
        pNewItem->bExpiredPeriod = false;
    }

    SetItemAttributes(pNewItem);

    m_listItem.push_back(pNewItem);
    return pNewItem;
}

ITEM* mu::ui::window::CItemMng::CreateItem(ITEM* pItem)
{
    pItem->RefCount++;
    return pItem;
}

ITEM* mu::ui::window::CItemMng::DuplicateItem(ITEM* pItem)
{
    ITEM* pNewItem = new ITEM;
    memcpy(pNewItem, pItem, sizeof(ITEM));
    pNewItem->Key = GenerateItemKey();
    pNewItem->RefCount = 1;
    m_listItem.push_back(pNewItem);
    return pNewItem;
}

void mu::ui::window::CItemMng::DeleteItem(ITEM* pItem)
{
    if (pItem == NULL)
        return;

    if (--pItem->RefCount <= 0)
    {
        auto li = m_listItem.begin();
        for (; li != m_listItem.end(); li++)
        {
            if ((*li) == pItem)
            {
                SAFE_DELETE(*li);
                m_listItem.erase(li);
                break;
            }
        }
    }
}

void mu::ui::window::CItemMng::DeleteDuplicatedItem(ITEM* pItem)
{
    if (pItem == NULL)
        return;

    if (--pItem->RefCount <= 0)
    {
        DeleteItem(pItem);
    }
}

void mu::ui::window::CItemMng::DeleteAllItems()
{
    auto li = m_listItem.begin();
    for (; li != m_listItem.end(); li++)
    {
        SAFE_DELETE(*li);
    }
    m_listItem.clear();

    m_dwAlternate = 0;
    m_dwAvailableKeyStream = 0x80000000;
}

bool mu::ui::window::CItemMng::IsEmpty()
{
    return m_listItem.empty();
}

#ifdef _EDITOR
bool mu::ui::window::CItemMng::HasItemOfType(int itemType) const
{
    for (const ITEM* item : m_listItem)
    {
        if (item->Type == itemType)
        {
            return true;
        }
    }
    return false;
}
#endif

void mu::ui::window::CItemMng::Update()
{
    m_UpdateTimer.UpdateTime();
    if (m_UpdateTimer.IsTime())
    {
        auto li = m_listItem.begin();
        for (; li != m_listItem.end();)
        {
            if ((*li)->RefCount <= 0)
            {
                delete (*li);
                li = m_listItem.erase(li);
            }
            else
                li++;
        }
    }
}

DWORD mu::ui::window::CItemMng::GenerateItemKey()
{
    DWORD dwAvailableItemKey = FindAvailableKeyIndex(m_dwAvailableKeyStream);
    if (dwAvailableItemKey >= 0x8F000000)
    {
        m_dwAvailableKeyStream = 0;
        m_dwAlternate++;
        dwAvailableItemKey = FindAvailableKeyIndex(m_dwAvailableKeyStream);
    }
    return m_dwAvailableKeyStream = dwAvailableItemKey;
}

DWORD mu::ui::window::CItemMng::FindAvailableKeyIndex(DWORD dwSeed)
{
    if (m_dwAlternate > 0)
    {
        auto li = m_listItem.begin();
        for (; li != m_listItem.end(); li++)
        {
            ITEM* pItem = (*li);
            if (pItem->Key == dwSeed + 1)
                return FindAvailableKeyIndex(dwSeed + 1);
        }
    }
    return dwSeed + 1;
}

WORD mu::ui::window::CItemMng::ExtractItemType(std::span<const BYTE> pbyItemPacket)
{
    return pbyItemPacket[0] + (pbyItemPacket[3] & 128) * 2 + (pbyItemPacket[5] & 240) * 32;
}
