#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Data::Items
{
class ItemDatabase;
} // namespace Data::Items

// The problems of the item references in texts ({item:14,25}; docs/translation-system.md, "Texts that name items"),
// against the item data: a number no item has, a level whose name needs game values (a skill's name), a level without
// a name line of its own, a case the language does not decline in or a form the item lacks, and a translation that
// names other items than the English text. Shown by the editor's Data Checks ("Texts").
namespace Data::Translation
{
// One text of a language: what the problems name as their file, item and field.
struct CheckedText
{
    std::string_view text; // UTF-8
    std::string_view locale;
    std::string source;                       // "Game.pl.resx", "Tooltip.json", or empty for an item's own text
    int group = Items::ItemDataIssue::NoItem; // the item whose text it is, or none
    int number = Items::ItemDataIssue::NoItem;
    std::string field; // the key, or the item file's field ("description[0].text.pl")
    // A UI text (Game, Dialog), a printf format: a name with a '%' cannot be put in (ResolveUiTextName).
    bool uiText = false;
};

// The problems of one text. `english` is the English text of the same key or line (empty for the English text
// itself): a translation with references must name the same items.
void CheckItemReferences(const Items::ItemDatabase& database, const CheckedText& text, std::string_view english,
                         std::vector<Items::ItemDataIssue>& issues);

// The texts of the UI groups that name items (I18N::GetItemTexts), each language that has its own.
void CheckUiItemTexts(const Items::ItemDatabase& database, std::vector<Items::ItemDataIssue>& issues);

// The item data's texts: the items' description lines, name lines and lines added to their layouts, and the shared
// texts and the layouts' lines of the tooltip file (`tooltipFile` names it).
void CheckItemDataTexts(const Items::ItemDatabase& database, const std::string& tooltipFile,
                        std::vector<Items::ItemDataIssue>& issues);

// The texts that name the item in some language, each once, as the editor lists them: "Game: Jewel of Bless",
// "Dialog: Answer_65_Slot_0", "Sealed Golden Box (14,121): description[0]", "Tooltip.json: texts.talisman",
// "Tooltip.json: layouts[0].lines[3]".
std::vector<std::string> FindTextsNamingItem(const Items::ItemDatabase& database, int group, int number);
} // namespace Data::Translation
