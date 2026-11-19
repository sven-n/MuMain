#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/Common/NamesGridRows.h"

#include "Data/GameData/ItemData/ItemOptionDefinition.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

// The option texts (Data/Items/Tooltips/OptionTexts.json) as rows of the names grid, for the Texts tab of Item
// Options: a row per text in name order, its id given to its name once and kept (a text added before it does not move
// an edit or a selection to another text). An edit changes Item Options' copy of the texts, which Apply takes with the
// window's other changes.
class COptionTextRows final : public MuEditor::NamesGridRows
{
public:
    // The texts the rows show and change, and the stored ones a changed text is marked against (`*`); set every frame
    // (the copy may be taken again).
    void SetTexts(Data::Items::ItemOptionTexts* texts, const Data::Items::ItemOptionTexts* stored);
    // The text's name of a row; empty when there is none.
    const std::string& GetName(int id) const;

    const std::vector<int>& List(std::string_view search, std::string_view missingLocale) override;
    void Refresh() override;
    int GetVersion() const override;
    std::vector<int> CountMissing(std::span<const char* const> locales) const override;
    const Data::LocalizedString* FindNames(int id) const override;
    void FormatId(int id, char* buffer, std::size_t size) const override;
    float GetIdColumnWidth() const override;
    void CommitName(int id, const char* locale, const char* text) override;
    int GetSelected() const override;
    int GetSelectionSerial() const override;
    void Select(int id) override;
    void ShowBrowser() override;
    const char* GetSelectTooltip() const override;
    const char* GetAllRowsLabel() const override;

private:
    void UpdateNames();
    int IdOf(const std::string& name);

    Data::Items::ItemOptionTexts* m_texts = nullptr;
    const Data::Items::ItemOptionTexts* m_stored = nullptr;
    // The name of each id, and the id of each name.
    std::vector<std::string> m_idNames;
    std::map<std::string, int, std::less<>> m_ids;
    // The texts' ids in name order, and what they were taken from.
    std::vector<int> m_rows;
    Data::Items::ItemOptionTexts m_namesOf;
    std::vector<int> m_list;
    std::string m_search;
    std::string m_missingLocale;
    bool m_listed = false;
    int m_version = 0;
    int m_selected = -1;
    int m_selectionSerial = 0;
};

#endif // _EDITOR
