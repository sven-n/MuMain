// NewUIHeroPositionInfo.h: interface for the CNewUIHeroPositionInfo class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIHEROPOSITIONINFO_H__5D452B62_B315_41EE_A862_5929286925F7__INCLUDED_)
#define AFX_NEWUIHEROPOSITIONINFO_H__5D452B62_B315_41EE_A862_5929286925F7__INCLUDED_

#pragma once

#include "UI/NewUI/NewUIBase.h"
#include "UI/NewUI/NewUIManager.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml { class ElementDocument; }

namespace SEASON3B
{
    // RmlUi migration (2026-08-31) -- first pilot of the CNewUIObj-tier adapter pattern
    // (docs/rmlui-ui-system/ -- a NewUI-tier companion writeup follows once this pilot is
    // verified; see .ai-os/memory/tasks/rmlui-sdl-gpu-port.md in the meantime). Chosen as the
    // pilot for being small, self-contained (no drag-drop, no INewUI3DRenderObj/3D-camera
    // rendering), and having genuine interaction (two real button clicks, live per-frame text) to
    // prove the pattern against -- see that same task doc for the full rationale and the
    // alternatives considered.
    //
    // Despite the class name, this widget's own tooltips (I18N::Game::OfficialMUHelperSetting/
    // StartOfficialMUHelper/StopOfficialMUHelper) identify it as the "Official MU Helper" mini
    // control bar (Config/Start/Stop for the MUHelper::g_MuHelper bot feature) with the hero's
    // current map/position readout drawn as part of the same strip -- one legacy class, one
    // window, one screen position, no existing seam to split the two concerns along. Ported as
    // one component to match, not split into two; the new RmlUi-facing assets are named
    // "mu_helper_bar" (not "hero_position_info") to reflect what the widget actually is -- see
    // Create()'s own comment. The legacy class/file name is left as CNewUIHeroPositionInfo/
    // NewUIHeroPositionInfo.* since renaming it would also mean renaming
    // INTERFACE_HERO_POSITION_INFO, CNewUISystem::m_pNewHeroPositionInfo, and the
    // g_pHeroPositionInfo macro -- out of scope for this pilot.
    //
    // Render() is now a no-op -- RmlUi renders 100% of this widget's visuals via the normal
    // SetPreSubmitCallback seam. UpdateMouseEvent()/UpdateKeyEvent() always report "not consumed"
    // -- RmlUi's own context does hit-testing now (Rml::Context::IsMouseInteracting(), wired into
    // Input/Selection.cpp's and ZzzInterface.cpp's world-click gates via
    // RmlUiRuntime::IsMouseOverUI() as of this same pilot). Create()/Release()/GetLayerDepth()/
    // Show()/Enable()/IsVisible() stay real -- CNewUIManager still owns z-order/registration/
    // visibility-toggling (SEASON3B::CNewUISystem::SyncHeroPositionInfoVisibility() etc.) through
    // them exactly as before. Update() still reads Hero's live position every frame, now to feed
    // the RmlUi model instead of a member later read by the (now dead) Render().
    //
    // The legacy CNewUIButton members/SetButtonInfo()/MoveTextTipPos()/BtnProcess()/LoadImages()/
    // UnloadImages() machinery is removed rather than kept-but-unused: unlike the CWin-tier
    // pattern (where a legacy CButton stays alive for redundant hit-testing bookkeeping),
    // UpdateMouseEvent() being a permanent "not consumed" here means those objects could never
    // detect a click again regardless of their own state -- keeping them would just be dead
    // weight, not real redundancy.
    class CNewUIHeroPositionInfo : public CNewUIObj
    {
    private:
        CNewUIManager* m_pNewUIMng;
        POINT m_CurHeroPosition;

    public:
        CNewUIHeroPositionInfo();
        virtual ~CNewUIHeroPositionInfo();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        // Vestigial (RmlUi/CSS owns this widget's screen position now, via base.rcss's
        // .anchor-top-left utility class -- see hero_position_info.rcss). Kept only because it's
        // public API surface; grep-confirmed zero external callers, safe to no-op.
        void SetPos(int x, int y) {}

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 4.3f

        void OpenningProcess();
        void ClosingProcess();

        // Vestigial for the same reason as SetPos() -- Update() reads Hero's position directly,
        // never through this setter (matched the original's own behavior: it never called this
        // either). Grep-confirmed zero external callers.
        void SetCurHeroPosition(int x, int y) {}

        // Invoked from the RmlUi document's data-event-click bindings (see Create()). Polled-and-
        // cleared exactly like every other migrated window's RmlClickX() pattern.
        void RmlClickConfig() { m_bRmlConfigClicked = true; }
        void RmlClickToggle() { m_bRmlToggleClicked = true; }

    private:
        struct HeroPositionInfoRmlModel
        {
            Rml::String positionText;   // "MapName (x, y)" -- matches the original's single RenderText call
            bool muHelperActive = false; // drives which of Start/Stop is shown
            Rml::String configTooltip, startTooltip, stopTooltip;
        };
        RmlModelBinder<HeroPositionInfoRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        bool m_bRmlConfigClicked = false;
        bool m_bRmlToggleClicked = false;

        void SyncRmlModel();
    };
}

#endif // !defined(AFX_NEWUIHEROPOSITIONINFO_H__5D452B62_B315_41EE_A862_5929286925F7__INCLUDED_)
