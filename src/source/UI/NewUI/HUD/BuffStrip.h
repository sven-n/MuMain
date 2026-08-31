// BuffStrip.h: interface for the CBuffStrip class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UI/NewUI/NewUIManager.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <vector>

namespace Rml { class ElementDocument; }

namespace SEASON3B
{
    // RmlUi migration (2026-08-31) -- second CNewUIObj-tier pilot, same adapter shape as
    // CMuHelperBar (MuHelperBar.h; see docs/rmlui-ui-system/newui-tier-adapter.md for the shape
    // itself, not repeated here). What's new here: a genuinely variable-length list -- the
    // active-buff count changes every frame, unlike the fixed-size job-button array the CWin tier
    // already proved (CCharMakeWin). Renamed from the legacy CNewUIBuffWindow at port time (not a
    // later pass) -- see .ai-os/memory/tasks/rmlui-ui-architecture-amendment.md's "Tracked
    // deferral" section for the naming policy this follows.
    //
    // Two deliberate simplifications from the original (each a scoped-out follow-up, not a
    // silent behavior drop -- see the commit message for the full rationale). (A third -- the
    // icon atlas -- was originally meant to be a fidelity cut too, an untested clipped-oversized-
    // image technique instead of per-tile named sprites; confirmed by direct testing not to work
    // in this RmlUi build at all, so it was replaced outright with generated @spritesheet rects,
    // the same proven mechanism every other migrated window's icons use -- see BuffEntry::
    // decorator's comment, BuffStrip.h, for why.)
    // - Right-click-to-cancel (Infinity Arrow / Swell of Magic Power only, the original
    //   UpdateMouseEvent()'s SEASON3B::IsRelease(VK_RBUTTON) branch) is NOT reproduced -- no
    //   RmlUi content in this codebase has ever distinguished right-click from left-click in a
    //   data-event-click binding. Deferred until that's proven generally.
    // - Tooltip text is one plain newline-joined block (BuildTooltipText()), not the original's
    //   bold-blue-header/white-body/purple-duration per-line coloring -- rich structured tooltip
    //   content is a separate thing to prove from what this pilot actually tests (dynamic array
    //   size).
    class CBuffStrip : public CNewUIObj
    {
    public:
        CBuffStrip();
        virtual ~CBuffStrip();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        // Vestigial (RmlUi/CSS owns this widget's screen position now -- base.rcss's .center-x,
        // applied in buff_strip.rml; see that file's own comment for the derivation). The
        // original's iScreenWidth overload horizontally centered this window within the
        // available "clear" screen width, shrinking as inventory-style side panels docked and ate
        // into it (Engine/Object/ZzzInventory.cpp's GetScreenWidth()) -- the panel-reactive
        // shrinking isn't reproduced (fixed-anchor policy,
        // docs/rmlui-ui-system/layout-and-scaling.md), but the underlying centering intent is,
        // via .center-x, which stays correct at any resolution.
        void SetPos(int x, int y) {}
        void SetPos(int iScreenWidth) {}

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 0.95f (the real runtime value -- the original's own "5.3f"
                                //  header comment was stale even before this port)

        void OpenningProcess();
        void ClosingProcess();

        // See CMuHelperBar::SyncDocVisibility()'s identical comment (MuHelperBar.h) -- same
        // MAIN_SCENE-only Update() gate, same fix (called every frame from
        // CNewUISystem::SyncMainSceneHudVisibility(), regardless of scene).
        void SyncDocVisibility(bool sceneAllowsShow);

    private:
        struct BuffEntry
        {
            // This icon's own grid slot position, pre-computed in C++ -- same "push already-
            // computed pixel values, bind with a plain + 'px' concat" pattern char_make.rml's job
            // buttons already prove works (CharMakeWin.h's JobButtonEntry), no arithmetic done in
            // the RML expression itself.
            float slotLeft = 0.0f, slotTop = 0.0f;
            // A full decorator value string, e.g. "image(atlas1-23)" -- picks one of the named
            // @spritesheet rects declared in buff_strip.rcss (one rect per 20x28 tile of each
            // atlas, BuffStrip.cpp's BuildIconDecorator() generates the name). Bound wholesale via
            // data-style-decorator rather than assembled from pieces in RML (same "no arithmetic/
            // concatenation logic in the RML expression itself" policy as slotLeft/Top above).
            //
            // 2026-08-31: replaced an earlier clipped-oversized-image attempt (a small
            // overflow:hidden container holding a full-atlas-sized image, positioned by a
            // negative per-tile offset) -- confirmed by direct in-game testing that RmlUi does not
            // derive a scissor clip from an *absolutely-positioned* oversized child at all
            // (ContainerBox::Close() submits the scrollable-overflow rect -- which
            // ElementUtilities::GetClippingRegion()'s has_clipping_content check reads -- before
            // ClosePositionedElements() ever places that child, so the container always looks
            // "nothing to clip" regardless of overflow/clip properties). Named @spritesheet rects
            // are the mechanism every other migrated window's icons already use successfully;
            // switched to match rather than chase clipping further.
            Rml::String decorator;
            Rml::String tooltip;
        };
        struct BuffStripRmlModel
        {
            std::vector<BuffEntry> buffs;
        };
        RmlModelBinder<BuffStripRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        CNewUIManager* m_pNewUIMng = nullptr;

        void SyncRmlModel();
    };
}
