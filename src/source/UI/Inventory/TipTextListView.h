#pragma once

#include "UI/Inventory/TipTextListLayout.h"
#include "UI/Inventory/TipTextListRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// Draws a RenderTipTextList() table (TipTextListRecord, UI::TipTextList) through RmlUi: the black frames and
// fills, the lines and their coloured text boxes. For the windows that were nothing but such
// tables (CItemExplanationWindow, CSetItemExplanation); each owns one with its own document.
class TipTextListView
{
public:
    TipTextListView(const char* modelName, const char* documentPath);

    void Build();
    // Unloads the document; from the window's own Release().
    void Release() { m_View.Release(); }

    // Per frame, inside the window's CManager transform scope: shown with `record`, or hidden.
    void Sync(bool visible, const TipTextListRecord& record);

private:
    UI::RmlBridge::ThemedView<TipTextListRmlModel> m_View;
};
} // namespace mu::ui::window
