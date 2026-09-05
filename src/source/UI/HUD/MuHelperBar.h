// MuHelperBar.h: interface for the CMuHelperBar class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Core/NewUIManager.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    // RmlUi migration (2026-08-31) -- first pilot of the CObject-tier adapter pattern
    // (docs/rmlui-ui-system/newui-tier-adapter.md; see .ai-os/memory/tasks/rmlui-sdl-gpu-port.md
    // for the full session history). Chosen as the pilot for being small, self-contained (no
    // drag-drop, no I3DRenderObj/3D-camera rendering), and having genuine interaction (two
    // real button clicks, live per-frame text) to prove the pattern against.
    //
    // This widget's own tooltips (I18N::Game::OfficialMUHelperSetting/StartOfficialMUHelper/
    // StopOfficialMUHelper) identify it as the "Official MU Helper" mini control bar (Config/
    // Start/Stop for the MUHelper::g_MuHelper bot feature) with the hero's current map/position
    // readout drawn as part of the same strip -- one legacy class, one window, one screen
    // position, no existing seam to split the two concerns along. Ported as one component to
    // match, not split into two.
    //
    // 2026-08-31, renamed from CNewUIHeroPositionInfo (a name that reflected the position readout
    // only, not what the widget actually is): the original class name, along with
    // INTERFACE_HERO_POSITION_INFO, CSystem::m_pNewHeroPositionInfo/
    // GetUI_NewHeroPositionInfo()/g_pHeroPositionInfo, and the Sync/Update/ShouldHide*Visibility
    // helper methods, all renamed to match at the same time -- see
    // .ai-os/memory/tasks/rmlui-ui-architecture-amendment.md's "Tracked deferral" section for why
    // this rename happened now (in place) rather than being deferred alongside a future physical
    // directory reorg (still deferred, unrelated to this naming pass).
    //
    // Render() is now a no-op -- RmlUi renders 100% of this widget's visuals via the normal
    // SetPreSubmitCallback seam. UpdateMouseEvent()/UpdateKeyEvent() always report "not consumed"
    // -- RmlUi's own context does hit-testing now (Rml::Context::IsMouseInteracting(), wired into
    // Input/Selection.cpp's and ZzzInterface.cpp's world-click gates via
    // RmlUiRuntime::IsMouseOverUI()). Create()/Release()/GetLayerDepth()/Show()/Enable()/
    // IsVisible() stay real -- CManager still owns z-order/registration/visibility-toggling
    // (mu::ui::window::CSystem::SyncMuHelperBarVisibility() etc.) through them exactly as before.
    // Update() still reads Hero's live position every frame, now to feed the RmlUi model instead
    // of a member later read by the (now dead) Render().
    //
    // The legacy CButton members/SetButtonInfo()/MoveTextTipPos()/BtnProcess()/LoadImages()/
    // UnloadImages() machinery is removed rather than kept-but-unused: unlike the CWin-tier
    // pattern (where a legacy CButton stays alive for redundant hit-testing bookkeeping),
    // UpdateMouseEvent() being a permanent "not consumed" here means those objects could never
    // detect a click again regardless of their own state -- keeping them would just be dead
    // weight, not real redundancy.
    class CMuHelperBar : public CObject
    {
    private:
        CManager* m_pNewUIMng;
        POINT m_CurHeroPosition;

    public:
        CMuHelperBar();
        virtual ~CMuHelperBar();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        // Vestigial (RmlUi/CSS owns this widget's screen position now, via base.rcss's
        // .anchor-top-left utility class -- see mu_helper_bar.rcss). Kept only because it's
        // public API surface; grep-confirmed zero external callers, safe to no-op.
        void SetPos(int x, int y) {}

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 4.3f

        void OpenningProcess();
        void ClosingProcess();

        // Re-derives the RmlUi document's actual Show()/Hide() state from IsVisible() (the
        // layout-driven flag CObject::Show() already sets -- panel docking etc., see
        // NewUISystem.cpp's Hide/Show(INTERFACE_MU_HELPER_BAR) call sites) ANDed with
        // sceneAllowsShow. Needed because this widget's Update() (the only place that used to
        // touch the RmlUi doc) only ever runs while SceneFlag == MAIN_SCENE (MainScene.cpp) --
        // before this widget became RmlUi-backed, that same MAIN_SCENE-only call site was already
        // a complete visibility gate, since nothing drew otherwise. Now that a persistent RmlUi
        // document owns the visuals, it needs a real gate of its own -- called every frame
        // regardless of scene from Winmain.cpp's SetPostRmlUiCallback via
        // CSystem::SyncMainSceneHudVisibility(), so leaving MAIN_SCENE is caught even though
        // Update() itself stops running.
        void SyncDocVisibility(bool sceneAllowsShow);

        // Vestigial for the same reason as SetPos() -- Update() reads Hero's position directly,
        // never through this setter (matched the original's own behavior: it never called this
        // either). Grep-confirmed zero external callers.
        void SetCurHeroPosition(int x, int y) {}

        // Invoked from the RmlUi document's data-event-click bindings (see Create()). Polled-and-
        // cleared exactly like every other migrated window's RmlClickX() pattern.
        void RmlClickConfig() { m_bRmlConfigClicked = true; }
        void RmlClickToggle() { m_bRmlToggleClicked = true; }

    private:
        struct MuHelperBarRmlModel
        {
            Rml::String positionText;   // "MapName (x, y)" -- matches the original's single RenderText call
            bool muHelperActive = false; // drives which of Start/Stop is shown
            Rml::String configTooltip, startTooltip, stopTooltip;
        };
        RmlModelBinder<MuHelperBarRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        bool m_bRmlConfigClicked = false;
        bool m_bRmlToggleClicked = false;

        void SyncRmlModel();
    };
}
