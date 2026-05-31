#include "stdafx.h"
#include "ReconnectDialog.h"

#include "Network/Reconnect/ReconnectManager.h"
#include "Render/Textures/ZzzOpenglUtil.h"   // RenderColor, BeginBitmap/EndBitmap, Mouse*
#include "Render/Sprites/GlobalBitmap.h"     // Bitmaps (texture-loaded check)
#include "UI/Legacy/UIControls.h"            // g_pRenderText, CheckMouseIn, RT3_SORT_CENTER
#include "UI/NewUI/NewUICommon.h"            // SEASON3B::RenderImage
#include "UI/NewUI/Dialogs/NewUIMessageBox.h"// CNewUIMessageBoxMng::IMAGE_MSGBOX_*
#include "Platform/Windows/Winmain.h"        // g_hFont, g_hFontBold
#include "I18N/All.h"

namespace UI::Reconnect
{
namespace
{
    using MsgBox = SEASON3B::CNewUIMessageBoxMng;

    // Frozen game frame captured at disconnect, shown during the re-login phase.
    GLuint s_backgroundTex = 0;
    bool s_hasBackground = false;

    // Native message-box frame slice sizes (match CNewUICommonMessageBox).
    constexpr float MSGBOX_WIDTH = 230.0f;
    constexpr float TOP_H = 67.0f;
    constexpr float BOTTOM_H = 50.0f;
    constexpr float BACK_BLANK_W = 8.0f;
    constexpr float BACK_BLANK_H = 10.0f;
    // Layout in the 640x480 reference space the 2D render helpers use. The
    // frame's top/bottom slices are fixed; the middle slice is stretched to make
    // up the rest, so the panel height can be set freely.
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

    // True when the in-game message-box textures are resident (they are while
    // we're still in the main scene - i.e. the long probing wait). During the
    // brief re-login the UI is torn down and they may be gone, so we fall back.
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
        // While probing we're still in the main scene: dim the live game lightly
        // and let it show through.
        if (ReconnectManager::Instance().GetPhase() == ReconnectManager::Phase::Probing)
        {
            FillBlack(0.0f, 0.0f, REFERENCE_WIDTH, REFERENCE_HEIGHT, DIM_ALPHA);
            return;
        }

        // Re-login: the world is torn down. Show the frozen disconnect frame
        // (BindTexture takes a negative id as a raw GL texture; the framebuffer
        // is bottom-up so V is flipped) so the player doesn't see a black screen
        // or the login world.
        if (s_hasBackground && s_backgroundTex != 0)
        {
            glEnable(GL_TEXTURE_2D);
            RenderColorBitmap(-static_cast<int>(s_backgroundTex), 0.0f, 0.0f,
                REFERENCE_WIDTH, REFERENCE_HEIGHT, 0.0f, 1.0f, 1.0f, -1.0f, 0xFFFFFFFF);
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

    // ---- Native (message-box textured) rendering ----------------------------

    void DrawNative(bool cancelHovered)
    {
        glEnable(GL_TEXTURE_2D);

        // Inner background, then the top/middle*n/bottom border frame on top.
        SEASON3B::RenderImage(MsgBox::IMAGE_MSGBOX_BACK, PANEL_X, PANEL_Y + 2.0f,
            MSGBOX_WIDTH - BACK_BLANK_W, PANEL_H - BACK_BLANK_H);

        SEASON3B::RenderImage(MsgBox::IMAGE_MSGBOX_TOP, PANEL_X, PANEL_Y, MSGBOX_WIDTH, TOP_H);
        SEASON3B::RenderImage(MsgBox::IMAGE_MSGBOX_MIDDLE, PANEL_X, PANEL_Y + TOP_H, MSGBOX_WIDTH,
            MIDDLE_FILL_H, 0.0f, 0.0f, 1.0f, 1.0f);  // stretched to fill
        SEASON3B::RenderImage(MsgBox::IMAGE_MSGBOX_BOTTOM, PANEL_X, PANEL_Y + TOP_H + MIDDLE_FILL_H,
            MSGBOX_WIDTH, BOTTOM_H);

        // Progress bar (native two-texture gauge), fill inset into the trough.
        SEASON3B::RenderImage(MsgBox::IMAGE_MSGBOX_PROGRESS_BG, PROG_X, PROG_Y, PROG_W, PROG_H);
        const float fraction = Progress();
        if (fraction > 0.0f)
        {
            SEASON3B::RenderImage(MsgBox::IMAGE_MSGBOX_PROGRESS_BAR, PROG_BAR_X, PROG_BAR_Y,
                PROG_BAR_MAX_W * fraction, PROG_BAR_H);
        }

        // Cancel button: pick the normal / hover row from the button sheet.
        const float sv = cancelHovered ? (CANCEL_H / BTN_SHEET_H) : 0.0f;
        SEASON3B::RenderImage(MsgBox::IMAGE_MSGBOX_BTN_CANCEL, CANCEL_X, CANCEL_Y, CANCEL_W, CANCEL_H,
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

    if (s_backgroundTex == 0)
    {
        glGenTextures(1, &s_backgroundTex);
    }

    glBindTexture(GL_TEXTURE_2D, s_backgroundTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Read the displayed (front) buffer - this runs right after the frame was
    // presented and before the dialog draws, so it's a clean game frame.
    glReadBuffer(GL_FRONT);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0,
        static_cast<GLsizei>(WindowWidth), static_cast<GLsizei>(WindowHeight), 0);
    glReadBuffer(GL_BACK);

    s_hasBackground = true;
}

void RenderDialog()
{
    ReconnectManager& mgr = ReconnectManager::Instance();
    if (!mgr.IsActive())
    {
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
