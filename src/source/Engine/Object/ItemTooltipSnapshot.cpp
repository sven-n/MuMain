#include "stdafx.h"

#include "Engine/Object/ItemTooltipSnapshot.h"

#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"

#include <algorithm>
#include <sstream>
#include <string>

namespace Engine::Items::TooltipSnapshot
{
namespace
{
struct TooltipVariant
{
    const char* name;
    void (*apply)(ItemCreationParams& params);
};

constexpr TooltipVariant TooltipVariants[] = {
    {"plain", [](ItemCreationParams&) {}},
    {"full",
     [](ItemCreationParams& params)
     {
         params.WithLuck = true;
         params.WithSkill = true;
         params.WithOption = true;
         params.OptionLevel = 4;
         params.HasExcellentOption = true;
         params.ExcellentFlags = 63;
     }},
    {"ancient",
     [](ItemCreationParams& params)
     {
         params.IsAncient = true;
         params.AncientDiscriminator = 1;
     }},
    {"harmony",
     [](ItemCreationParams& params)
     {
         params.HasHarmonyOption = true;
         params.HarmonyOptionType = 1;
         params.HarmonyOptionLevel = 13;
     }},
    // Each of the full item's parts alone, since a line can depend on one of them only.
    {"luck", [](ItemCreationParams& params) { params.WithLuck = true; }},
    {"skill", [](ItemCreationParams& params) { params.WithSkill = true; }},
    {"option",
     [](ItemCreationParams& params)
     {
         params.WithOption = true;
         params.OptionLevel = 1;
     }},
    {"excellent",
     [](ItemCreationParams& params)
     {
         params.HasExcellentOption = true;
         params.ExcellentFlags = 1;
     }},
    // A worn and a broken item; for a stack, its count (one, none).
    {"durability 1", [](ItemCreationParams& params) { params.Durability = 1; }},
    {"durability 0", [](ItemCreationParams& params) { params.Durability = 0; }},
    // A timed item that isn't expired shows its end date, which depends on the time zone; the tests check it apart.
    {"expired",
     [](ItemCreationParams& params)
     {
         params.WithExpiration = true;
         params.IsExpired = true;
     }},
};
} // namespace

ItemCreationParams FullItemParameters(int group, int number)
{
    ItemCreationParams params{};
    params.Group = group;
    params.Number = number;
    params.Durability = static_cast<BYTE>(ItemAttribute[group * MAX_ITEM_INDEX + number].Durability);
    return params;
}

BYTE FullDurability(const ItemCreationParams& parameters)
{
    ITEM item;
    FillItemFromParameters(item, parameters);
    const int maxDurability = CalcMaxDurability(&item, &ItemAttribute[item.Type], item.Level);
    return static_cast<BYTE>(std::min(maxDurability, 255));
}

std::vector<ItemTooltipLine> TooltipOfParameters(const ItemCreationParams& parameters)
{
    ITEM item;
    FillItemFromParameters(item, parameters);
    return BuildItemTooltipLines(&item, false);
}

int VisitItemTooltipSnapshots(const ItemSnapshot& item, int onlyGroup)
{
    int tooltips = 0;
    for (const Data::Items::ItemDefinition& definition : g_ItemDatabase.GetAllSlots())
    {
        if (!definition.Exists() || (onlyGroup >= 0 && definition.group != onlyGroup))
        {
            continue;
        }
        std::ostringstream out;
        for (const TooltipVariant& variant : TooltipVariants)
        {
            for (int level = 0; level < Data::Items::ItemLevelCount; ++level)
            {
                ItemCreationParams params = FullItemParameters(definition.group, definition.number);
                params.Level = static_cast<BYTE>(level);
                variant.apply(params);
                out << definition.group << '/' << definition.number << ' ' << variant.name << " +" << level << '\n';
                for (const ItemTooltipLine& line : TooltipOfParameters(params))
                {
                    // A blank line shows no color or style: what its slot keeps of an earlier tooltip is left out.
                    if (line.text == L"\n")
                    {
                        out << "  <blank>\n";
                        continue;
                    }
                    out << "  " << line.color << (line.bold ? " bold " : " ") << Core::Text::ToUtf8(line.text.c_str())
                        << '\n';
                }
                ++tooltips;
            }
        }
        item(definition.group, definition.number, out.str());
    }
    return tooltips;
}

int WriteItemTooltipSnapshot(std::ostream& out, int onlyGroup)
{
    return VisitItemTooltipSnapshots([&out](int, int, const std::string& text) { out << text; }, onlyGroup);
}
} // namespace Engine::Items::TooltipSnapshot
