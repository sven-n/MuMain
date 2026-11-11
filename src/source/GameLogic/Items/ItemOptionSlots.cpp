#include "stdafx.h"
#include "GameLogic/Items/ItemOptionSlots.h"

#include "Core/Utilities/Log/MuLogger.h"

#include <bitset>

namespace
{
// ITEM::SpecialSource of a slot the code wrote; a definition's slot holds its index + 1 in the high word, so a zeroed
// item has no sources.
constexpr DWORD NoSource = 0;
constexpr int SourceDefinitionShift = 16;
constexpr DWORD SourceOptionMask = 0xFFFF;

// The item types that dropped an option, so the log names each type once.
std::bitset<MAX_ITEM> g_typesWithDroppedOptions;

void ReportDroppedOption(const ITEM& item, std::uint16_t option)
{
    if (item.Type < 0 || item.Type >= MAX_ITEM || g_typesWithDroppedOptions.test(item.Type))
    {
        return;
    }
    g_typesWithDroppedOptions.set(item.Type);
    MU_LOG_WARN(mu::log::Get("gameplay"), "Item {} has more than {} options; option {} and those after it are dropped",
                item.Type, MAX_ITEM_SPECIAL, option);
}

void AddSlot(ITEM& item, std::uint16_t option, int value, DWORD source)
{
    if (item.SpecialNum >= MAX_ITEM_SPECIAL)
    {
        ReportDroppedOption(item, option);
        return;
    }
    item.Special[item.SpecialNum] = option;
    item.SpecialValue[item.SpecialNum] = static_cast<WORD>(value);
    item.SpecialSource[item.SpecialNum] = source;
    ++item.SpecialNum;
}
} // namespace

namespace GameLogic::Items
{
void AddOptionSlot(ITEM& item, std::uint16_t option, int value)
{
    AddSlot(item, option, value, NoSource);
}

void AddOptionSlot(ITEM& item, std::uint16_t option, int value, OptionSource source)
{
    const DWORD packed = ((static_cast<DWORD>(source.definition) + 1) << SourceDefinitionShift) | source.option;
    AddSlot(item, option, value, packed);
}

void SetOptionSlot(ITEM& item, std::uint16_t option, int value)
{
    for (int i = 0; i < item.SpecialNum; ++i)
    {
        if (item.Special[i] == option)
        {
            item.SpecialValue[i] = static_cast<WORD>(value);
            return;
        }
    }
    AddOptionSlot(item, option, value);
}

std::optional<OptionSource> GetOptionSource(const ITEM& item, int slot)
{
    if (slot < 0 || slot >= item.SpecialNum || item.SpecialSource[slot] == NoSource)
    {
        return std::nullopt;
    }
    const DWORD packed = item.SpecialSource[slot];
    return OptionSource{static_cast<std::uint16_t>((packed >> SourceDefinitionShift) - 1),
                        static_cast<std::uint16_t>(packed & SourceOptionMask)};
}
} // namespace GameLogic::Items
