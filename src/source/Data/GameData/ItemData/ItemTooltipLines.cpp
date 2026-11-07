#include "stdafx.h"

#include "Data/GameData/ItemData/ItemTooltipLines.h"

namespace Data::Items
{
std::vector<TooltipLineOf> TooltipLinesOf(const ItemDefinition& definition)
{
    std::vector<TooltipLineOf> lines;
    for (size_t index = 0; index < definition.description.size(); ++index)
    {
        lines.push_back({&definition.description[index], "description[" + std::to_string(index) + "]"});
    }
    for (size_t index = 0; index < definition.nameLines.size(); ++index)
    {
        lines.push_back({&definition.nameLines[index], "nameLines[" + std::to_string(index) + "]"});
    }
    const std::vector<ItemTooltipLayoutAddition>& additions = definition.tooltipLayoutChanges.add;
    for (size_t addition = 0; addition < additions.size(); ++addition)
    {
        for (size_t index = 0; index < additions[addition].lines.size(); ++index)
        {
            lines.push_back({&additions[addition].lines[index], "tooltipLayoutChanges.add[" + std::to_string(addition) +
                                                                    "].lines[" + std::to_string(index) + "]"});
        }
    }
    return lines;
}

std::string LineTextField(const std::string& lineField, std::string_view locale)
{
    std::string field = lineField + ".text";
    if (locale != LocalizedString::NeutralLocale)
    {
        field += "." + std::string(locale);
    }
    return field;
}
} // namespace Data::Items
