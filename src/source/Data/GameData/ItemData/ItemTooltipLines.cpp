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

std::vector<TooltipFileLineOf> TooltipFileLinesOf(const ItemTooltipFile& file)
{
    std::vector<TooltipFileLineOf> lines;
    for (size_t layout = 0; layout < file.layouts.size(); ++layout)
    {
        const std::string field = std::string(TooltipLayoutsKey) + "[" + std::to_string(layout) + "]";
        const std::vector<TooltipLayoutLine>& own = file.layouts[layout].lines;
        for (size_t index = 0; index < own.size(); ++index)
        {
            lines.push_back({&own[index], field + ".lines[" + std::to_string(index) + "]"});
        }
        const std::vector<TooltipLayoutAddition>& additions = file.layouts[layout].add;
        for (size_t addition = 0; addition < additions.size(); ++addition)
        {
            for (size_t index = 0; index < additions[addition].lines.size(); ++index)
            {
                lines.push_back({&additions[addition].lines[index], field + ".add[" + std::to_string(addition) +
                                                                        "].lines[" + std::to_string(index) + "]"});
            }
        }
    }
    return lines;
}

const LocalizedString* FindLineText(const ItemTooltipFile& tooltips, const LocalizedString& text,
                                    const std::string& textName)
{
    return textName.empty() ? &text : tooltips.FindText(textName);
}

const LocalizedString* FindLineText(const ItemTooltipFile& tooltips, const ItemDescriptionLine& line)
{
    return FindLineText(tooltips, line.text, line.textName);
}

std::string_view GetTextLanguage(const LocalizedString& text, std::string_view locale)
{
    return text.HasOwn(locale) ? locale : LocalizedString::NeutralLocale;
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
