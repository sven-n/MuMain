#include "stdafx.h"

#ifdef _EDITOR

#include "ItemLayoutChangeNotes.h"

#include "I18N/All.h"

#include <string>

namespace MuEditor::Items
{
namespace
{
// An added line as written in the locale: a shared text's own text (its "@name" where the file lacks it), a text
// written in place, or a blank line; with its levels.
std::string DescribeAddedLine(const Data::Items::ItemDescriptionLine& line,
                              const Data::Items::ItemTooltipFile& tooltips, std::string_view locale)
{
    std::string text;
    if (!line.textName.empty())
    {
        const Data::LocalizedString* shared = tooltips.FindText(line.textName);
        text = shared != nullptr ? shared->Get(locale) : "@" + line.textName;
    }
    else
    {
        text = line.text.Get(locale);
    }
    if (text.empty())
    {
        text = I18N::Editor::ABlankLine;
    }
    if (line.HasLevels())
    {
        text += " " + I18N::Format(I18N::Editor::AtLevels0To1,
                                   {std::to_string(line.minLevel), std::to_string(line.maxLevel)});
    }
    return text;
}
} // namespace

std::vector<ItemLayoutChangeNote> DescribeItemLayoutChanges(const Data::Items::ItemTooltipLayoutChanges& changes,
                                                            const Data::Items::ItemTooltipFile& tooltips,
                                                            std::string_view locale)
{
    std::vector<ItemLayoutChangeNote> notes;
    if (!changes.hide.empty())
    {
        std::string ids;
        for (const std::string& id : changes.hide)
        {
            ids += (ids.empty() ? "" : ", ") + id;
        }
        notes.push_back({I18N::Format(I18N::Editor::ItsOwnChangesHide0, {ids}), false});
    }
    for (const Data::Items::ItemTooltipLayoutAddition& addition : changes.add)
    {
        notes.push_back({I18N::Format(I18N::Editor::ItAddsAfterTheLine0, {addition.after}), false});
        for (size_t index = 0; index < addition.lines.size(); ++index)
        {
            notes.push_back(
                {std::to_string(index + 1) + ". " + DescribeAddedLine(addition.lines[index], tooltips, locale), true});
        }
    }
    return notes;
}
} // namespace MuEditor::Items

#endif // _EDITOR
