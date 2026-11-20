#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/Common/NamesGridRows.h"

#include <string>
#include <vector>

namespace MuEditor::Warps
{
// The warps of the Warp List window as rows of the names grid, by their
// index, in the M window's order. A name edit is an undo step of the window
// and reaches the M window at once.
class WarpNameRows final : public NamesGridRows
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
    std::vector<int> m_rows;
    // What the rows were listed for.
    std::string m_search;
    std::string m_missingLocale;
    int m_builtVersion = -1;
    bool m_built = false;
};
} // namespace MuEditor::Warps

#endif // _EDITOR
