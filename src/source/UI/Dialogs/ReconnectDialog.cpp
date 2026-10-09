#include "stdafx.h"
#include "ReconnectDialog.h"

#include "Network/Reconnect/ReconnectManager.h"
#include "Render/Textures/ZzzOpenglUtil.h" // BeginBitmap/EndBitmap, RenderColorBitmap
#include <vector>
#include "I18N/All.h"
#include "Render/Renderer/MuRenderer.h"
#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Dialogs/ReconnectDialogRmlModel.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/ElementDocument.h>

namespace UI::Reconnect
{
namespace
{
    // Frozen game frame captured at disconnect, shown during the re-login phase.
    std::uint32_t s_backgroundTex = 0;
    bool s_hasBackground = false;

    // The dialog in RmlUi (reconnect_dialog.rml): one main-context document above every other
    // document, the tooltip included, as the original drew it after the whole frame. RenderDialog()
    // fills it every frame and hides it while no reconnect runs. The frozen frame stays native
    // (a scene background, drawn before RmlUi's pass); the dim over it is the document's.
    void BindModel(Rml::DataModelConstructor& c, ReconnectDialogRmlModel& model);
    UI::RmlBridge::ThemedView<ReconnectDialogRmlModel> s_view{"reconnect_dialog", BindModel,
        {{"Data/Interface/RmlUi/reconnect_dialog.rml"}}};
    bool s_cancelClicked = false;

    // The panel's place in the original's 640x480 screen, below the centre.
    constexpr float PANEL_W = 230.0f;
    constexpr float PANEL_H = 122.0f;
    constexpr float PANEL_X = (REFERENCE_WIDTH - PANEL_W) / 2.0f;
    constexpr float PANEL_Y = (REFERENCE_HEIGHT - PANEL_H) / 2.0f + 100.0f;

    constexpr float PROG_BAR_MAX_W = 150.0f;

    constexpr float DIM_ALPHA = 0.45f;   // dim over the live game / snapshot
    constexpr float OPAQUE_ALPHA = 1.0f; // full cover when no frame is shown

    const wchar_t* StepLabel(ReconnectManager::Phase phase)
    {
        using Phase = ReconnectManager::Phase;
        switch (phase)
        {
        case Phase::Probing:       return I18N::Game::CheckingServerAvailability;
        case Phase::Retrying:      return I18N::Game::ConnectingToTheServer;
        case Phase::Connecting:    return I18N::Game::ConnectingToTheServer;
        case Phase::LoggingIn:     return I18N::Game::LoggingIn;
        case Phase::SelectingChar: return I18N::Game::LoadingCharacterList;
        case Phase::Joining:       return I18N::Game::EnteringTheGame;
        default:                   return L"";
        }
    }

    float Progress()
    {
        const int steps = ReconnectManager::GetStepCount();
        const int current = ReconnectManager::Instance().GetStepIndex();
        if (steps <= 0 || current <= 0)
        {
            return 0.0f;
        }
        return static_cast<float>(current) / static_cast<float>(steps);
    }

    void BindModel(Rml::DataModelConstructor& c, ReconnectDialogRmlModel& model)
    {
        c.Bind("dim_alpha", &model.dimAlpha);
        c.Bind("root_x", &model.rootX);
        c.Bind("root_y", &model.rootY);
        c.Bind("root_scale", &model.rootScale);
        c.Bind("text_px", &model.textPx);
        c.Bind("bold_text_px", &model.boldTextPx);
        c.Bind("title_text", &model.titleText);
        c.Bind("step_text", &model.stepText);
        c.Bind("countdown_text", &model.countdownText);
        c.Bind("progress_width", &model.progressWidth);
        c.BindEventCallback("reconnect_cancel", [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                            { s_cancelClicked = true; });
    }

    // DrawBackdrop()'s dim and DrawNative()'s panel, gauge and Cancel.
    void SyncView(float dimAlpha, float fraction)
    {
        UI::RmlBridge::SyncDocumentVisibilityInFront(s_view.Document(), true);

        // The original's 640x480 screen stretched over the window.
        const UI::Scaling::Transform transform =
            UI::Scaling::ScreenOverlayTransform(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));
        SyncField(s_view.Binder(), &ReconnectDialogRmlModel::dimAlpha, "dim_alpha", dimAlpha);
        SyncField(s_view.Binder(), &ReconnectDialogRmlModel::rootX, "root_x", UI::Scaling::PositionX(transform, PANEL_X));
        SyncField(s_view.Binder(), &ReconnectDialogRmlModel::rootY, "root_y", UI::Scaling::PositionY(transform, PANEL_Y));
        SyncField(s_view.Binder(), &ReconnectDialogRmlModel::rootScale, "root_scale", transform.scaleX);
        SyncField(s_view.Binder(), &ReconnectDialogRmlModel::textPx, "text_px",
                  UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, transform));
        SyncField(s_view.Binder(), &ReconnectDialogRmlModel::boldTextPx, "bold_text_px",
                  UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, transform));
        // DrawStatusTexts(): the title, the step and the countdown. The theme gives each its
        // row and centres it across the panel.
        ReconnectManager& mgr = ReconnectManager::Instance();
        SyncField(s_view.Binder(), &ReconnectDialogRmlModel::titleText, "title_text",
                  StringUtils::WideToNarrow(I18N::Game::ConnectionLost));
        SyncField(s_view.Binder(), &ReconnectDialogRmlModel::stepText, "step_text",
                  StringUtils::WideToNarrow(StepLabel(mgr.GetPhase())));
        Rml::String countdownText;
        if (const int seconds = mgr.GetCountdownSeconds(); seconds > 0)
        {
            wchar_t countdown[64];
            mu_swprintf(countdown, I18N::Game::RetryingInSeconds, seconds);
            countdownText = StringUtils::WideToNarrow(countdown);
        }
        SyncField(s_view.Binder(), &ReconnectDialogRmlModel::countdownText, "countdown_text", std::move(countdownText));
        SyncField(s_view.Binder(), &ReconnectDialogRmlModel::progressWidth, "progress_width", PROG_BAR_MAX_W * fraction);
    }

}

void CaptureBackground()
{
    if (WindowWidth == 0 || WindowHeight == 0)
    {
        return;
    }

    s_backgroundTex = mu::GetRenderer().CaptureFrameTexture(s_backgroundTex);
    s_hasBackground = s_backgroundTex != 0u;
}

void RenderDialog()
{
    ReconnectManager& mgr = ReconnectManager::Instance();
    if (!mgr.IsActive())
    {
        UI::RmlBridge::SyncDocumentVisibility(s_view.Document(), false);
        s_cancelClicked = false;
        return;
    }

    s_view.Ensure();
    if (s_view.Document() == nullptr)
        return;

    // Natively only the frozen frame (the re-login phase); the dim, panel and Cancel are the
    // document's. Cancel is its click, as the original's press on the button.
    const bool probing = mgr.GetPhase() == ReconnectManager::Phase::Probing;
    const bool frozenFrame = !probing && s_hasBackground && s_backgroundTex != 0;
    if (frozenFrame)
    {
        BeginBitmap();
        RenderColorBitmap(static_cast<int>(s_backgroundTex), 0.0f, 0.0f, REFERENCE_WIDTH, REFERENCE_HEIGHT, 0.0f, 0.0f,
                          1.0f, 1.0f, 0xFFFFFFFF);
        EndBitmap();
    }
    SyncView(probing || frozenFrame ? DIM_ALPHA : OPAQUE_ALPHA, Progress());
    if (s_cancelClicked)
    {
        s_cancelClicked = false;
        mgr.RequestCancel();
    }
}
}
