#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/Common/NamesGridRows.h"
#include "../MuEditor/UI/ItemEditor/ItemListModel.h"

// The items as rows of the names grid: the item types, the item names and the
// item tools' selection; a name edit goes through the item tools' history.
class CItemNameRows final : public MuEditor::NamesGridRows
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
    CItemListModel::Filter m_filter;
    CItemListModel m_list;
};

#endif // _EDITOR
