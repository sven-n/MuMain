
#pragma once

#include "UI/Core/WindowManager.h"
#include "UI/RmlBridge/RmlThemedView.h"

#include <vector>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    // RmlUi-backed active-buff strip, a variable-length list (unlike CMuHelperBar's fixed layout).
    // Right-clicking Infinity Arrow or Swell of Magic Power asks to cancel it, as the original did.
    class CBuffStrip : public CObject
    {
    public:
        CBuffStrip();
        virtual ~CBuffStrip();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        // Vestigial -- RmlUi/CSS owns this widget's position now (.center-x in buff_strip.rml).
        void SetPos(int x, int y) {}
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 0.95f

        void OpenningProcess();
        void ClosingProcess();

        // Same MAIN_SCENE-only Update() gate and fix as CMuHelperBar::SyncDocVisibility() (MuHelperBar.h).
        void SyncDocVisibility(bool sceneAllowsShow);


    private:
        void BuildRmlUi();

        struct BuffEntry
        {
            // Pre-computed pixel grid-slot position, bound as plain values (no arithmetic in RML).
            // Full decorator string, e.g. "image(atlas1-23)", selecting a named @spritesheet rect
            // from buff_strip.rcss (one rect per 20x28 tile; BuildIconDecorator() in BuffStrip.cpp
            // generates the name). Bound wholesale via data-style-decorator.
            //
            // Note: an absolutely-positioned oversized child does NOT get clipped by RmlUi even
            // with overflow:hidden on its container (ContainerBox::Close() computes the clip rect
            // before ClosePositionedElements() places such children) -- don't retry a clipped-atlas
            // approach; named @spritesheet rects are the working mechanism.
            Rml::String decorator;
            Rml::String tooltip;
            // The same tooltip split like the original's (RenderBuffTooltip()): the name (blue,
            // bold), the description lines, and the remaining duration (purple; empty if none).
            Rml::String tooltipTitle;
            Rml::String tooltipBody;
            Rml::String tooltipDuration;
        };
        struct BuffStripRmlModel
        {
            std::vector<BuffEntry> buffs;
            // The original's strip box in real pixels: 200-unit rows centred on the free width, in
            // the strip's own stretched HUD space. Data a theme may follow, not a placement.
            float stripSlotLeft = 0.0f;
            float stripSlotWidth = 0.0f;
            // Native tooltip row advance in real pixels: RenderTipTextList() steps 1.1 text
            // heights of the native renderer per line.
            float tooltipLinePx = 0.0f;
        };
        void BindRmlModel(Rml::DataModelConstructor& c, BuffStripRmlModel& model);
        UI::RmlBridge::ThemedView<BuffStripRmlModel> m_RmlView{"buff_strip",
            [this](Rml::DataModelConstructor& c, BuffStripRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/buff_strip.rml"}}};

        CManager* m_pNewUIMng = nullptr;

        // The buffs shown, in slot order, for mapping a slot's event back to its buff.
        std::vector<int> m_ShownBuffs;

        void SyncRmlModel();
        void OnBuffRightClick(int slot);
        void SyncStripSlot();
        void SyncTooltipLineHeight();
    };
}
