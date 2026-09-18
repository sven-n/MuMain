
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    // RmlUi-backed "Official MU Helper" control bar (Config/Start/Stop for MUHelper::g_MuHelper)
    // plus the hero's live map/position readout. Render() is a no-op -- RmlUi draws everything via
    // SetPreSubmitCallback. UpdateMouseEvent()/UpdateKeyEvent() always report "not consumed" --
    // RmlUi's own context does hit-testing. Create()/Release()/GetLayerDepth()/Show()/Enable()/
    // IsVisible() still drive CManager's z-order/registration/visibility as before. Update() still
    // reads Hero's position every frame, now to feed the RmlUi model.
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

        // Vestigial -- RmlUi/CSS owns this widget's position now (mu_helper_bar.rcss). No-op kept for API compatibility.
        void SetPos(int x, int y) {}

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 4.3f

        void OpenningProcess();
        void ClosingProcess();

        // Gates the RmlUi doc's Show()/Hide() on IsVisible() AND sceneAllowsShow. Needed because
        // Update() (which used to own doc visibility) only runs during MAIN_SCENE; this is called
        // every frame regardless of scene from CSystem::SyncMainSceneHudVisibility(), so leaving
        // MAIN_SCENE still hides the doc.
        void SyncDocVisibility(bool sceneAllowsShow);

        void ReloadRmlTheme() override;

        // Vestigial -- Update() reads Hero's position directly, never through this setter.
        void SetCurHeroPosition(int x, int y) {}

        // Set by the RmlUi document's data-event-click bindings; polled and cleared like other windows' RmlClickX().
        void RmlClickConfig() { m_bRmlConfigClicked = true; }
        void RmlClickToggle() { m_bRmlToggleClicked = true; }

    private:
        struct MuHelperBarRmlModel
        {
            Rml::String positionText;   // "MapName (x, y)"
            bool muHelperActive = false; // drives which of Start/Stop is shown
            Rml::String configTooltip, startTooltip, stopTooltip;
        };
        RmlModelBinder<MuHelperBarRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        bool m_bRmlConfigClicked = false;
        bool m_bRmlToggleClicked = false;

        void SyncRmlModel();
        void BuildRmlUi();
    };
}
