#include "stdafx.h"
#include "ReconnectDialog.h"

#include "Network/Reconnect/ReconnectManager.h"
#include "Render/Textures/ZzzOpenglUtil.h" // RenderColor, BeginBitmap/EndBitmap, Mouse*
#include <vector>
#include "Render/Sprites/GlobalBitmap.h"     // Bitmaps (texture-loaded check)
#include "UI/Widgets/UIControls.h"            // g_pRenderText, CheckMouseIn, RT3_SORT_CENTER
#include "UI/Core/WindowCommon.h"            // mu::ui::window::RenderImage
#include "UI/Dialogs/MessageBox.h"// CMessageBoxMng::IMAGE_MSGBOX_*
#include "App/Platform/Windows/Winmain.h"        // g_hFont, g_hFontBold
#include "I18N/All.h"
#include "Render/Renderer/MuRenderer.h"
#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Dialogs/ReconnectDialogRmlModel.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/ElementDocument.h>

namespace UI::Reconnect
{
namespace
{
    using MsgBox = mu::ui::window::CMessageBoxMng;

    // Frozen game frame captured at disconnect, shown during the re-login phase.
    std::uint32_t s_backgroundTex = 0;
    bool s_hasBackground = false;

    // The dialog in RmlUi (reconnect_dialog.rml): one main-context document above every other
    // document, the tooltip included, as the original drew it after the whole frame. RenderDialog()
    // fills it every frame and hides it while no reconnect runs. The frozen frame stays native
    // (a scene background, drawn before RmlUi's pass); the dim over it is the document's.
    RmlModelBinder<ReconnectDialogRmlModel> s_binder;
    Rml::ElementDocument* s_document = nullptr;
    bool s_cancelClicked = false;
    bool s_themeReloadRegistered = false;
    const int s_themeReloadOwner = 0; // the theme-reload registration's owner key

    // Native message-box frame slice sizes (match CCommonMessageBox).
    constexpr float MSGBOX_WIDTH = 230.0f;
    constexpr float TOP_H = 67.0f;
    constexpr float BOTTOM_H = 50.0f;
    constexpr float BACK_BLANK_W = 8.0f;
    constexpr float BACK_BLANK_H = 10.0f;
    // 640x480 reference-space layout: top/bottom slices are fixed, middle stretches to
    // fill, so PANEL_H is freely adjustable.
    constexpr float PANEL_W = MSGBOX_WIDTH;
    constexpr float PANEL_H = 122.0f;
    constexpr float PANEL_X = (REFERENCE_WIDTH - PANEL_W) / 2.0f;
    constexpr float PANEL_Y = (REFERENCE_HEIGHT - PANEL_H) / 2.0f + 100.0f;
    constexpr float MIDDLE_FILL_H = PANEL_H - TOP_H - BOTTOM_H;

    constexpr float TITLE_Y = PANEL_Y + 12.0f;
    constexpr float STEP_Y = PANEL_Y + 30.0f;
    constexpr float COUNTDOWN_Y = PANEL_Y + 48.0f;

    constexpr float PROG_W = 160.0f;
    constexpr float PROG_H = 18.0f;
    constexpr float PROG_X = PANEL_X + (PANEL_W - PROG_W) / 2.0f;
    constexpr float PROG_Y = PANEL_Y + 66.0f;
    // The fill bar is smaller than the trough; inset it so it sits centred.
    constexpr float PROG_BAR_MAX_W = 150.0f;
    constexpr float PROG_BAR_H = 8.0f;
    constexpr float PROG_BAR_INSET = 5.0f;
    constexpr float PROG_BAR_X = PROG_X + PROG_BAR_INSET;
    constexpr float PROG_BAR_Y = PROG_Y + PROG_BAR_INSET;

    constexpr float CANCEL_W = 54.0f;        // native cancel button size
    constexpr float CANCEL_H = 30.0f;
    constexpr float CANCEL_X = PANEL_X + (PANEL_W - CANCEL_W) / 2.0f;
    constexpr float CANCEL_Y = PANEL_Y + PANEL_H - CANCEL_H - 6.0f;

    // The cancel button texture stacks three 30px states in a 64x128 sheet.
    constexpr float BTN_SHEET_W = 64.0f;
    constexpr float BTN_SHEET_H = 128.0f;

    // Text colour (light parchment, matching the in-game message boxes).
    constexpr BYTE TEXT_R = 230;
    constexpr BYTE TEXT_G = 220;
    constexpr BYTE TEXT_B = 200;
    constexpr BYTE TEXT_A = 255;

    // Backdrop / fallback-panel opacities.
    constexpr float DIM_ALPHA = 0.45f;          // dim over the live game / snapshot
    constexpr float OPAQUE_ALPHA = 1.0f;        // full cover when no frame is shown
    constexpr float FB_BORDER_ALPHA = 0.5f;     // fallback panel border
    constexpr float FB_PANEL_ALPHA = 0.92f;     // fallback panel fill
    constexpr float FB_BORDER = 2.0f;           // fallback panel border thickness
    constexpr float FB_BAR_BG_ALPHA = 0.7f;     // fallback progress trough
    constexpr float FB_BAR_FILL_ALPHA = 0.9f;   // fallback progress fill
    constexpr float FB_CANCEL_ALPHA = 0.25f;    // fallback cancel button
    constexpr float FB_CANCEL_HOVER_ALPHA = 0.45f;

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

    // True while in the main scene (probing); may be false during re-login once the UI is torn down.
    bool NativeSkinAvailable()
    {
        return Bitmaps[MsgBox::IMAGE_MSGBOX_TOP].TextureNumber != 0
            && Bitmaps[MsgBox::IMAGE_MSGBOX_PROGRESS_BG].TextureNumber != 0
            && Bitmaps[MsgBox::IMAGE_MSGBOX_BTN_CANCEL].TextureNumber != 0;
    }

    void FillWhite(float x, float y, float w, float h, float alpha)
    {
        RenderColor(x, y, w, h, alpha, 0);
    }

    void FillBlack(float x, float y, float w, float h, float alpha)
    {
        RenderColor(x, y, w, h, alpha, 1);
    }

    void DrawCenteredText(float x, float y, float width, HFONT font, const wchar_t* text)
    {
        g_pRenderText->SetFont(font);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        g_pRenderText->SetTextColor(TEXT_R, TEXT_G, TEXT_B, TEXT_A);
        g_pRenderText->RenderText(static_cast<int>(x), static_cast<int>(y), text,
            static_cast<int>(width), 0, RT3_SORT_CENTER);
    }

    void DrawBackdrop()
    {
        // While probing, dim the live game lightly so it shows through.
        if (ReconnectManager::Instance().GetPhase() == ReconnectManager::Phase::Probing)
        {
            FillBlack(0.0f, 0.0f, REFERENCE_WIDTH, REFERENCE_HEIGHT, DIM_ALPHA);
            return;
        }

        // Re-login: world is torn down, show the frozen disconnect frame instead of black/login screen.
        if (s_hasBackground && s_backgroundTex != 0)
        {
            RenderColorBitmap(static_cast<int>(s_backgroundTex), 0.0f, 0.0f,
                REFERENCE_WIDTH, REFERENCE_HEIGHT, 0.0f, 0.0f, 1.0f, 1.0f, 0xFFFFFFFF);
            FillBlack(0.0f, 0.0f, REFERENCE_WIDTH, REFERENCE_HEIGHT, DIM_ALPHA);
            return;
        }

        FillBlack(0.0f, 0.0f, REFERENCE_WIDTH, REFERENCE_HEIGHT, OPAQUE_ALPHA);
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

    void DrawStatusTexts()
    {
        ReconnectManager& mgr = ReconnectManager::Instance();

        DrawCenteredText(PANEL_X, TITLE_Y, PANEL_W, g_hFontBold, I18N::Game::ConnectionLost);
        DrawCenteredText(PANEL_X, STEP_Y, PANEL_W, g_hFont, StepLabel(mgr.GetPhase()));

        const int seconds = mgr.GetCountdownSeconds();
        if (seconds > 0)
        {
            wchar_t countdown[64];
            mu_swprintf(countdown, I18N::Game::RetryingInSeconds, seconds);
            DrawCenteredText(PANEL_X, COUNTDOWN_Y, PANEL_W, g_hFont, countdown);
        }
    }

    // ---- RmlUi rendering --------------------------------------------------------

    Rml::Context* DialogContext()
    {
        return RmlUiRuntime::Instance().GetContext();
    }

    void BuildView();

    void ReloadTheme()
    {
        if (s_document == nullptr)
            return;
        s_binder.Destroy(DialogContext());
        DialogContext()->UnloadDocument(s_document);
        s_document = nullptr;
        BuildView();
    }

    void BuildView()
    {
        if (s_document != nullptr || !RmlUiRuntime::Instance().IsCreated() || DialogContext() == nullptr)
            return;

        const bool modelCreated = s_binder.Create(
            DialogContext(), "reconnect_dialog",
            [](Rml::DataModelConstructor& c, ReconnectDialogRmlModel& model)
            {
                c.Bind("dim_alpha", &model.dimAlpha);
                c.Bind("root_x", &model.rootX);
                c.Bind("root_y", &model.rootY);
                c.Bind("root_scale", &model.rootScale);
                c.Bind("text_px", &model.textPx);
                c.Bind("bold_text_px", &model.boldTextPx);
                auto line = c.RegisterStruct<ReconnectLineEntry>();
                line.RegisterMember("text", &ReconnectLineEntry::text);
                line.RegisterMember("left", &ReconnectLineEntry::left);
                line.RegisterMember("top", &ReconnectLineEntry::top);
                line.RegisterMember("bold", &ReconnectLineEntry::bold);
                c.RegisterArray<std::vector<ReconnectLineEntry>>();
                c.Bind("lines", &model.lines);
                c.Bind("progress_width", &model.progressWidth);
                c.BindEventCallback("reconnect_cancel", [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                                    { s_cancelClicked = true; });
            });
        if (modelCreated)
            s_document =
                UI::RmlBridge::LoadThemedDocument(DialogContext(), "Data/Interface/RmlUi/reconnect_dialog.rml");
        if (s_document != nullptr && !s_themeReloadRegistered)
        {
            UI::RmlBridge::RegisterForThemeReload(&s_themeReloadOwner, [] { ReloadTheme(); });
            s_themeReloadRegistered = true;
        }
    }

    template <typename T> void SyncField(T ReconnectDialogRmlModel::* field, const char* name, T value)
    {
        auto& model = s_binder.GetModel();
        if (model.*field == value)
            return;
        model.*field = std::move(value);
        s_binder.MarkDirty(name);
    }

    // DrawStatusTexts(): the title (bold), the step and the countdown, each centred in the panel's
    // width.
    std::vector<ReconnectLineEntry> StatusLines()
    {
        ReconnectManager& mgr = ReconnectManager::Instance();
        std::vector<ReconnectLineEntry> lines;
        auto add = [&lines](const wchar_t* text, float top, bool bold)
        {
            g_pRenderText->SetFont(bold ? g_hFontBold : g_hFont);
            const SIZE size = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text)));
            const float left = size.cx < PANEL_W ? (PANEL_W - static_cast<float>(size.cx)) / 2.0f : 0.0f;
            lines.push_back({StringUtils::WideToNarrow(text), left, top - PANEL_Y, bold});
        };
        add(I18N::Game::ConnectionLost, TITLE_Y, true);
        add(StepLabel(mgr.GetPhase()), STEP_Y, false);
        const int seconds = mgr.GetCountdownSeconds();
        if (seconds > 0)
        {
            wchar_t countdown[64];
            mu_swprintf(countdown, I18N::Game::RetryingInSeconds, seconds);
            add(countdown, COUNTDOWN_Y, false);
        }
        return lines;
    }

    // DrawBackdrop()'s dim and DrawNative()'s panel, gauge and Cancel.
    void SyncView(float dimAlpha, float fraction)
    {
        UI::RmlBridge::SyncDocumentVisibilityInFront(s_document, true);

        const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
        SyncField(&ReconnectDialogRmlModel::dimAlpha, "dim_alpha", dimAlpha);
        SyncField(&ReconnectDialogRmlModel::rootX, "root_x", UI::Scaling::PositionX(transform, PANEL_X));
        SyncField(&ReconnectDialogRmlModel::rootY, "root_y", UI::Scaling::PositionY(transform, PANEL_Y));
        SyncField(&ReconnectDialogRmlModel::rootScale, "root_scale", transform.scaleX);
        SyncField(&ReconnectDialogRmlModel::textPx, "text_px",
                  UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, transform));
        SyncField(&ReconnectDialogRmlModel::boldTextPx, "bold_text_px",
                  UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, transform));
        SyncField(&ReconnectDialogRmlModel::lines, "lines", StatusLines());
        SyncField(&ReconnectDialogRmlModel::progressWidth, "progress_width", PROG_BAR_MAX_W * fraction);
    }

    // ---- Native (message-box textured) rendering ----------------------------

    void DrawNative(bool cancelHovered)
    {
        // Inner background, then the top/middle*n/bottom border frame on top.
        mu::ui::window::RenderImage(MsgBox::IMAGE_MSGBOX_BACK, PANEL_X, PANEL_Y + 2.0f,
            MSGBOX_WIDTH - BACK_BLANK_W, PANEL_H - BACK_BLANK_H);

        mu::ui::window::RenderImage(MsgBox::IMAGE_MSGBOX_TOP, PANEL_X, PANEL_Y, MSGBOX_WIDTH, TOP_H);
        mu::ui::window::RenderImage(MsgBox::IMAGE_MSGBOX_MIDDLE, PANEL_X, PANEL_Y + TOP_H, MSGBOX_WIDTH,
            MIDDLE_FILL_H, 0.0f, 0.0f, 1.0f, 1.0f);  // stretched to fill
        mu::ui::window::RenderImage(MsgBox::IMAGE_MSGBOX_BOTTOM, PANEL_X, PANEL_Y + TOP_H + MIDDLE_FILL_H,
            MSGBOX_WIDTH, BOTTOM_H);

        // Progress bar (native two-texture gauge), fill inset into the trough.
        mu::ui::window::RenderImage(MsgBox::IMAGE_MSGBOX_PROGRESS_BG, PROG_X, PROG_Y, PROG_W, PROG_H);
        const float fraction = Progress();
        if (fraction > 0.0f)
        {
            mu::ui::window::RenderImage(MsgBox::IMAGE_MSGBOX_PROGRESS_BAR, PROG_BAR_X, PROG_BAR_Y,
                PROG_BAR_MAX_W * fraction, PROG_BAR_H);
        }

        // Cancel button: pick the normal / hover row from the button sheet.
        const float sv = cancelHovered ? (CANCEL_H / BTN_SHEET_H) : 0.0f;
        mu::ui::window::RenderImage(MsgBox::IMAGE_MSGBOX_BTN_CANCEL, CANCEL_X, CANCEL_Y, CANCEL_W, CANCEL_H,
            0.0f, sv, CANCEL_W / BTN_SHEET_W, CANCEL_H / BTN_SHEET_H);

        DrawStatusTexts();
    }

    // ---- Fallback (flat-colour) rendering, used when the skin is unloaded ----

    void DrawFallback(bool cancelHovered)
    {
        FillWhite(PANEL_X - FB_BORDER, PANEL_Y - FB_BORDER,
            PANEL_W + 2 * FB_BORDER, PANEL_H + 2 * FB_BORDER, FB_BORDER_ALPHA);
        FillBlack(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, FB_PANEL_ALPHA);

        FillBlack(PROG_X, PROG_Y, PROG_W, PROG_H, FB_BAR_BG_ALPHA);
        const float fraction = Progress();
        if (fraction > 0.0f)
        {
            FillWhite(PROG_BAR_X, PROG_BAR_Y, PROG_BAR_MAX_W * fraction, PROG_BAR_H, FB_BAR_FILL_ALPHA);
        }

        FillWhite(CANCEL_X, CANCEL_Y, CANCEL_W, CANCEL_H,
            cancelHovered ? FB_CANCEL_HOVER_ALPHA : FB_CANCEL_ALPHA);

        EndRenderColor();  // restore texturing for the glyphs

        DrawStatusTexts();
        DrawCenteredText(CANCEL_X, CANCEL_Y + 8.0f, CANCEL_W, g_hFont, I18N::Game::Cancel);
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
        UI::RmlBridge::SyncDocumentVisibility(s_document, false);
        s_cancelClicked = false;
        return;
    }

    BuildView();
    if (s_document != nullptr)
    {
        // Natively only the frozen frame (the re-login phase); the dim, panel and Cancel are the
        // document's. Cancel is its click, as the original's press on the button.
        const bool probing = mgr.GetPhase() == ReconnectManager::Phase::Probing;
        const bool frozenFrame = !probing && s_hasBackground && s_backgroundTex != 0;
        if (frozenFrame)
        {
            BeginBitmap();
            RenderColorBitmap(static_cast<int>(s_backgroundTex), 0.0f, 0.0f, REFERENCE_WIDTH, REFERENCE_HEIGHT, 0.0f,
                              0.0f, 1.0f, 1.0f, 0xFFFFFFFF);
            EndBitmap();
        }
        SyncView(probing || frozenFrame ? DIM_ALPHA : OPAQUE_ALPHA, Progress());
        if (s_cancelClicked)
        {
            s_cancelClicked = false;
            mgr.RequestCancel();
        }
        return;
    }

    const bool cancelHovered = CheckMouseIn(static_cast<int>(CANCEL_X), static_cast<int>(CANCEL_Y),
        static_cast<int>(CANCEL_W), static_cast<int>(CANCEL_H)) == TRUE;

    BeginBitmap();

    DrawBackdrop();

    if (NativeSkinAvailable())
    {
        DrawNative(cancelHovered);
    }
    else
    {
        DrawFallback(cancelHovered);
    }

    EndBitmap();

    if (cancelHovered && MouseLButtonPush)
    {
        mgr.RequestCancel();
    }
}
}
