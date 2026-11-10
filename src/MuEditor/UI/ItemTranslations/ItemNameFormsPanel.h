#pragma once

#ifdef _EDITOR

#include <string>
#include <vector>

namespace Data::Items
{
struct ItemDefinition;
}

// The forms of the selected item's names in Item Translations ("nameForms"; docs/item-data.md, "Name forms"): one row
// for the item's name and one for each name line whose forms a text can show (the name at a level: a text of its own,
// no conditions, no placeholders), per language but English that declines names (NameCases.json), with a field per
// case. A text that names the item in a case ({item:14,25:acc})
// shows the form; an empty field shows the name itself, greyed. Above it, how many texts name the item.
class CItemNameFormsPanel
{
public:
    void Render(int itemType);

private:
    void RenderUses(int itemType);
    // The Cases button and its popup: which cases each language declines item names in (NameCases.json).
    void RenderCasesButton() const;
    void RenderTable(const Data::Items::ItemDefinition& definition);
    // One row of the table: the name `index` (-1: the item's name, else the name line) in the language.
    void RenderRow(const Data::Items::ItemDefinition& definition, int index, const std::string& locale);
    void RenderFormCell(const Data::Items::ItemDefinition& definition, int index, const std::string& locale,
                        const std::string& caseKey);
    void CommitForm(const Data::Items::ItemDefinition& definition, int index, const std::string& locale,
                    const std::string& caseKey, std::string text) const;

    // The texts that name the item, found again when the item data changes.
    int m_usesItemType = -1;
    int m_usesVersion = -1;
    std::vector<std::string> m_uses;
};

#endif // _EDITOR
