#include "stdafx.h"

#include "Engine/Object/ItemTooltipSnapshot.h"

#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"

#include <string>

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

std::vector<ItemTooltipLine> TooltipOfParameters(const ItemCreationParams& parameters)
{
    ITEM item;
    FillItemFromParameters(item, parameters);
    return BuildItemTooltipLines(&item, false);
}

int WriteItemTooltipSnapshot(std::ostream& out, int onlyGroup)
{
    int tooltips = 0;
    for (const Data::Items::ItemDefinition& definition : g_ItemDatabase.GetAllSlots())
    {
        if (!definition.Exists() || (onlyGroup >= 0 && definition.group != onlyGroup))
        {
            continue;
        }
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
                    const std::string text = line.text == L"\n" ? "<blank>" : Core::Text::ToUtf8(line.text.c_str());
                    out << "  " << line.color << (line.bold ? " bold " : " ") << text << '\n';
                }
                ++tooltips;
            }
        }
    }
    return tooltips;
}
