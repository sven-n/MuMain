#pragma once

#include "Engine/Object/ZzzInventory.h"
#include "UI/Inventory/TipTextListRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// Draws what RenderTipTextList() recorded (TipTextListRecord) through RmlUi: the black frames and
// fills, the lines and their coloured text boxes. For the windows that were nothing but such
// tables (CItemExplanationWindow, CSetItemExplanation); each owns one with its own document.
class TipTextListView
{
public:
    TipTextListView(const char* modelName, const char* documentPath);

    void Build();
    void ReloadTheme();

    // Per frame, inside the window's CManager transform scope: shown with `record`, or hidden.
    void Sync(bool visible, const TipTextListRecord& record);

private:
    const char* m_ModelName;
    const char* m_DocumentPath;
    RmlModelBinder<TipTextListRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
};
} // namespace mu::ui::window
