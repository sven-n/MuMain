#pragma once

#include "UI/Inventory/ItemHelpRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace mu::ui::window
{
// The RmlUi side of an item help box (CItemExplanationWindow, CSetItemExplanation): what the
// original drew with RenderTipTextList(), as lines and a levels table the theme lays out. Each
// window owns one, with its own document.
class ItemHelpView
{
public:
    using Table = std::vector<ItemHelpColumnEntry>;

    ItemHelpView(const char* modelName, const char* documentPath);

    void Build();
    // Unloads the document; from the window's own Release().
    void Release() { m_View.Release(); }

    // Per frame: shown with its content, or hidden.
    void Sync(bool visible, std::vector<ItemHelpLineEntry> lines, Table table = {},
              std::vector<ItemHelpLineEntry> tailLines = {});
    // Per frame instead of Sync() while the content stays the same: shown with the content it has.
    void SyncShown();

    // The TextList / TextListColor / TextBold globals' first `count` lines, as RenderTipTextList()
    // read them.
    static std::vector<ItemHelpLineEntry> TextListLines(int count);

private:
    // Shows or hides the document and keeps the text sizes of a shown one current. False when the
    // document isn't shown.
    bool SyncDocument(bool visible);

    UI::RmlBridge::ThemedView<ItemHelpRmlModel> m_View;
};
} // namespace mu::ui::window
