#pragma once

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemTooltipLayouts.h"

#include <string>
#include <string_view>
#include <vector>

namespace Data::Items
{
// A line of an item's own that its tooltip writes, and the field the item file writes it at ("description[2]",
// "nameLines[0]", "tooltipLayoutChanges.add[1].lines[0]").
struct TooltipLineOf
{
    const ItemDescriptionLine* line;
    std::string field;
};

// The lines ForEachItemTooltipLine walks (the description lines, the name lines, the added lines), each with its
// field in the item file.
std::vector<TooltipLineOf> TooltipLinesOf(const ItemDefinition& definition);

// A line of a layout of the tooltip file, and its field as the file's reader names it, by the layout's place in the
// list ("layouts[0].lines[3]", "layouts[2].add[0].lines[1]").
struct TooltipFileLineOf
{
    const TooltipLayoutLine* line;
    std::string field;
};

// The lines of the tooltip file's layouts: each layout's own lines and the lines a preset adds.
std::vector<TooltipFileLineOf> TooltipFileLinesOf(const ItemTooltipFile& file);

// A text of an item: written in place (`text`), or the shared text of the tooltip file that `textName` names ("text":
// "@talisman"); nullptr when the file doesn't have that text.
const LocalizedString* FindLineText(const ItemTooltipFile& tooltips, const LocalizedString& text,
                                    const std::string& textName);
const LocalizedString* FindLineText(const ItemTooltipFile& tooltips, const ItemDescriptionLine& line);

// The language `text` is shown in for `locale`: `locale` where it has a text of its own there, else English, which
// LocalizedString::Get falls back to. Its item references name the items in that language.
std::string_view GetTextLanguage(const LocalizedString& text, std::string_view locale);

// The field of a line's text in one language, as the item file writes it: "description[2].text" for English,
// "description[2].text.de" for a translation.
std::string LineTextField(const std::string& lineField, std::string_view locale);
} // namespace Data::Items
