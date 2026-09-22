
#pragma once

#include "UI/Core/WindowManager.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <vector>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    // RmlUi-backed active-buff strip, a variable-length list (unlike CMuHelperBar's fixed layout).
    // Two known fidelity gaps vs the legacy window: right-click-to-cancel (Infinity Arrow / Swell
    // of Magic Power) isn't reproduced -- no RmlUi content here distinguishes right-click from
    // left-click in a data-event-click binding yet. Tooltip text is one plain newline-joined block
    // (BuildTooltipText()), not the original's per-line bold/white/purple coloring.
    class CBuffStrip : public CObject
    {
    public:
        CBuffStrip();
        virtual ~CBuffStrip();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        // Vestigial -- RmlUi/CSS owns this widget's position now (.center-x in buff_strip.rml).
        // Note: panel-reactive width shrinking (the original's GetScreenWidth() behavior when
        // side panels docked) isn't reproduced; centering itself still holds at any resolution.
        void SetPos(int x, int y) {}
        void SetPos(int iScreenWidth) {}

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 0.95f

        void OpenningProcess();
        void ClosingProcess();

        // Same MAIN_SCENE-only Update() gate and fix as CMuHelperBar::SyncDocVisibility() (MuHelperBar.h).
        void SyncDocVisibility(bool sceneAllowsShow);

        void ReloadRmlTheme();

    private:
        void BuildRmlUi();

        struct BuffEntry
        {
            // Pre-computed pixel grid-slot position, bound as plain values (no arithmetic in RML).
            float slotLeft = 0.0f, slotTop = 0.0f;
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
        };
        struct BuffStripRmlModel
        {
            std::vector<BuffEntry> buffs;
        };
        RmlModelBinder<BuffStripRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        CManager* m_pNewUIMng = nullptr;

        void SyncRmlModel();
    };
}
