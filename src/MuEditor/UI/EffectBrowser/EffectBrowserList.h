#pragma once

#ifdef _EDITOR

#include "EffectBrowserModel.h"

#include <optional>
#include <span>
#include <vector>

// The list of the effect browser: the search and the filters, and the table of
// the types of one kind they list. Ctrl+C copies the name of the selected type.
class CEffectBrowserList
{
public:
    // Returns the type clicked, -1 for none.
    int Render(const MuEditor::Effects::EffectBrowserModel& model, Data::Effects::EffectKind kind, int selectedType);

    // Scrolls to the type when its kind is shown next; clears the search and
    // the filters when they hide it.
    void Show(const MuEditor::Effects::EffectBrowserModel& model, MuEditor::Effects::EffectTypeRef type);

private:
    void RenderFilters(Data::Effects::EffectKind kind);
    void UpdateListed(const MuEditor::Effects::EffectBrowserModel& model, Data::Effects::EffectKind kind);
    int RenderTable(const MuEditor::Effects::EffectBrowserModel& model, Data::Effects::EffectKind kind,
                    int selectedType);
    int RenderRows(std::span<const MuEditor::Effects::EffectBrowserRow> rows, Data::Effects::EffectKind kind,
                   int selectedType);
    // The row of m_listed to scroll to, -1 for none.
    int TakeScrollRow(std::span<const MuEditor::Effects::EffectBrowserRow> rows, Data::Effects::EffectKind kind);

    char m_search[128] = {};
    MuEditor::Effects::EffectBrowserFilter m_filter;

    // The rows the filter lists, and what they were listed for.
    std::vector<int> m_listed;
    std::optional<Data::Effects::EffectKind> m_listedKind;
    MuEditor::Effects::EffectBrowserFilter m_listedFilter;
    int m_listedAssetGeneration = -1;

    std::optional<MuEditor::Effects::EffectTypeRef> m_scrollTo;
};

#endif // _EDITOR
