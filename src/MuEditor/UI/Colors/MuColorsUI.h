#pragma once

#ifdef _EDITOR

#include "GlowColorsTab.h"

#include <optional>

namespace MuEditor::Colors
{
// The tabs of the Colors window.
enum class ColorsTab
{
    Glow,
};
} // namespace MuEditor::Colors

// The Colors window (D69): the editor's named colors, a tab per use, each drawn with the shared panel (NamedColorList).
// The glow colors for now. Windows > Colors opens it on the tab shown last, Items > Glow Colors on the Glow tab. It
// docks with the item tools, and its changes are item edits.
class CMuColorsUI
{
public:
    static CMuColorsUI& GetInstance();

    void Render(bool* open);

    // Opens the window on the tab and brings it to the front.
    void Show(MuEditor::Colors::ColorsTab tab);
    // The tab the window shows, or showed last while it is closed.
    MuEditor::Colors::ColorsTab GetShownTab() const
    {
        return m_shown;
    }

private:
    CMuColorsUI() = default;

    // The tab's BeginTabItem; true while it is the one shown.
    bool BeginTab(MuEditor::Colors::ColorsTab tab, const char* label, const char* id);

    MuEditor::Colors::ColorsTab m_shown = MuEditor::Colors::ColorsTab::Glow;
    // The tab Show asked for, until the tab bar is drawn.
    std::optional<MuEditor::Colors::ColorsTab> m_select;
    MuEditor::Colors::GlowColorsTab m_glow;
};

#define g_MuColorsUI CMuColorsUI::GetInstance()

#endif // _EDITOR
