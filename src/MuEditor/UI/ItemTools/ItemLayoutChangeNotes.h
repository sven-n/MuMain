#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemTooltipLayouts.h"

#include <string>
#include <string_view>
#include <vector>

namespace MuEditor::Items
{
// A row of what Item Tooltip shows of an item's own changes to its tooltip layout; `indented` for an added line under
// the line id it follows.
struct ItemLayoutChangeNote
{
    std::string text;
    bool indented = false;
};

// The item's own changes (tooltipLayoutChanges), a row each: the line ids it hides, then for each entry of its added
// lines the line id they follow and each line numbered from 1 as written in `locale` (a shared text's own text, its
// "@name" where the tooltip file lacks it, placeholders as written), with its levels. So a problem Item Checks names
// (tooltipLayoutChanges.add[0].lines[1]) can be found in Item Tooltip.
std::vector<ItemLayoutChangeNote> DescribeItemLayoutChanges(const Data::Items::ItemTooltipLayoutChanges& changes,
                                                            const Data::Items::ItemTooltipFile& tooltips,
                                                            std::string_view locale);
} // namespace MuEditor::Items

#endif // _EDITOR
