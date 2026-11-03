#pragma once

#ifdef _EDITOR

#include "MonsterListModel.h"

#include "../MuEditor/UI/Common/NamesGridRows.h"

namespace MuEditor::Monsters
{
// The monsters as rows of the names grid: the monster numbers with an entry,
// their names and the monster tools' selection; a name edit goes through the
// monster tools' history and reaches the monsters in the game at once.
class MonsterNameRows final : public NamesGridRows
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
    // Assigned in place, so the strings keep their memory from frame to frame.
    MonsterListModel::Filter m_filter;
    MonsterListModel m_list;
};
} // namespace MuEditor::Monsters

#endif // _EDITOR
