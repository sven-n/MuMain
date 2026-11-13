#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/Common/NamesGridRows.h"
#include "Data/GameData/Common/LocalizedString.h"

#include <map>
#include <string>
#include <vector>

// Monster Translations' talk texts (MN5, Q-MN47): a row per text of an NPC's
// talk (its level message, its dialogs' titles, lines and failure messages,
// its messages), by the monster's key (ShownValues) and the text's place, after
// a row per text of the shared talk texts (SharedKey).
namespace MuEditor::Monsters
{
class MonsterTalkTextRows final : public NamesGridRows
{
public:
    const std::vector<int>& List(std::string_view search, std::string_view missingLocale) override;
    void Refresh() override;
    int GetVersion() const override;
    std::vector<int> CountMissing(std::span<const char* const> locales) const override;
    const Data::LocalizedString* FindNames(int id) const override;
    void FormatId(int id, char* buffer, std::size_t size) const override;
    void CommitName(int id, const char* locale, const char* text) override;
    int GetSelected() const override;
    int GetSelectionSerial() const override;
    void Select(int id) override;
    void ShowBrowser() override;
    const char* GetSelectTooltip() const override;
    const char* GetAllRowsLabel() const override;

private:
    // The key of the shared talk texts' rows, which belong to no monster.
    static constexpr int SharedKey = -1;

    struct Row
    {
        int id = 0;
        // The monster's key (ShownValues) and the text's field.
        int key = 0;
        std::string field;
        Data::LocalizedString text;
    };

    // Every text of every monster's talk, by key in the monsters' order.
    std::vector<Row> CollectRows() const;
    void Build() const;

    mutable std::vector<Row> m_all;
    mutable int m_builtVersion = -1;
    std::vector<int> m_rows;
    std::string m_search;
    std::string m_missingLocale;
    bool m_listed = false;
    int m_listedVersion = -1;
};
} // namespace MuEditor::Monsters

#endif // _EDITOR
