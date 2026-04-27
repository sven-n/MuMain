// NewUIOptionWindow.cpp: implementation of the CNewUIOptionWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIOptionWindow.h"
#include "NewUISystem.h"
#include "ZzzTexture.h"
#include "DSPlaySound.h"
#include "GameConfig/GameConfig.h"
#include "wzAudio.h"
#include <algorithm>

extern int m_MusicOnOff;
extern int m_SoundOnOff;
extern char Mp3FileName[256];
extern unsigned int WindowWidth, WindowHeight;
extern float g_fScreenRate_x, g_fScreenRate_y;
extern int OpenglWindowWidth, OpenglWindowHeight;
extern BOOL g_bUseWindowMode;
extern HWND g_hWnd;
extern bool g_bWndActive;
void ReinitializeFonts();
void UpdateResolutionDependentSystems();
void UpdateCursorClip();
DWORD GetDesktopBitsPerPel();
float ConvertX(float x);
float ConvertY(float y);

using namespace SEASON3B;

static const struct { int width; int height; const wchar_t* label; } s_Resolutions[] = {
    { 640, 480, L"640 x 480" },
    { 800, 600, L"800 x 600" },
    { 1024, 768, L"1024 x 768" },
    { 1280, 720, L"1280 x 720" },
    { 1280, 1024, L"1280 x 1024" },
    { 1600, 900, L"1600 x 900" },
    { 1600, 1200, L"1600 x 1200" },
    { 1680, 1050, L"1680 x 1050" },
    { 1920, 1080, L"1920 x 1080" },
    { 2560, 1440, L"2560 x 1440" },
};
static const int s_NumResolutions = sizeof(s_Resolutions) / sizeof(s_Resolutions[0]);

// Label pointer array for the resolution combo box. Built once on first use
// from s_Resolutions so the combo can consume a plain `const wchar_t* const*`.
static const wchar_t* const* GetResolutionLabels()
{
    static const wchar_t* labels[s_NumResolutions] = {};
    static bool initialized = false;
    if (!initialized)
    {
        for (int i = 0; i < s_NumResolutions; i++)
            labels[i] = s_Resolutions[i].label;
        initialized = true;
    }
    return labels;
}

namespace
{
    // Volume levels are integers 0..MAX_VOLUME; the slider track is SLIDER_WIDTH pixels wide.
    constexpr int MAX_VOLUME = 10;
    constexpr int SLIDER_WIDTH = 124;        // pixels
    constexpr int SLIDER_HIT_PADDING = 8;    // extra px on each side for easier clicks
    constexpr int SLIDER_HIT_HEIGHT = 16;
    constexpr int SLIDER_X_LOCAL = 33;       // slider start relative to m_Pos.x
    constexpr int WZAUDIO_VOLUME_SCALE = 10; // 0..10 → 0..100

    // Render-level slider dimensions
    constexpr int RENDER_SLIDER_X_LOCAL = 25;
    constexpr int RENDER_SLIDER_Y_LOCAL = 196;
    constexpr int RENDER_SLIDER_WIDTH = 141;
    constexpr int RENDER_SLIDER_HEIGHT = 29;
    constexpr float RENDER_LEVEL_MAX = 5.f;

    // Resolution combo box placement (relative to m_Pos)
    constexpr int RES_COMBO_X_LOCAL = 22;
    constexpr int RES_COMBO_Y_LOCAL = 278;
    constexpr int RES_COMBO_WIDTH   = 148;  // spans the old left-to-right arrow area
    constexpr int RES_COMBO_HEIGHT  = 16;
    constexpr int RES_COMBO_MAX_VISIBLE = 4;  // scrollbar appears when list > this
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIOptionWindow::CNewUIOptionWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = 0;
    m_Pos.y = 0;

    m_bAutoAttack = true;
    m_bWhisperSound = false;
    m_bSlideHelp = true;
    m_iVolumeLevel = GameConfig::GetInstance().GetSoundVolume();
    m_iMusicLevel = GameConfig::GetInstance().GetMusicVolume();
    m_iRenderLevel = 4;
    m_bRenderAllEffects = true;
    m_iResolutionIndex = FindCurrentResolutionIndex();
    m_bWindowedMode = (g_bUseWindowMode == TRUE);
}

SEASON3B::CNewUIOptionWindow::~CNewUIOptionWindow()
{
    Release();
}

bool SEASON3B::CNewUIOptionWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_OPTION, this);
    SetPos(x, y);
    LoadImages();
    SetButtonInfo();
    InitResolutionCombo();
    Show(false);
    return true;
}

void SEASON3B::CNewUIOptionWindow::InitResolutionCombo()
{
    m_ResolutionCombo.Setup(
        m_Pos.x + RES_COMBO_X_LOCAL,
        m_Pos.y + RES_COMBO_Y_LOCAL,
        RES_COMBO_WIDTH,
        RES_COMBO_HEIGHT,
        GetResolutionLabels(),
        s_NumResolutions,
        m_iResolutionIndex,
        RES_COMBO_MAX_VISIBLE);
}

void SEASON3B::CNewUIOptionWindow::SetButtonInfo()
{
    m_BtnClose.ChangeTextBackColor(RGBA(255, 255, 255, 0));
    m_BtnClose.ChangeButtonImgState(true, IMAGE_OPTION_BTN_CLOSE, true);
    m_BtnClose.ChangeButtonInfo(m_Pos.x + 68, m_Pos.y + 322, 54, 30);
    m_BtnClose.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    m_BtnClose.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
}

void SEASON3B::CNewUIOptionWindow::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIOptionWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
    m_ResolutionCombo.SetPos(m_Pos.x + RES_COMBO_X_LOCAL, m_Pos.y + RES_COMBO_Y_LOCAL);
}

bool SEASON3B::CNewUIOptionWindow::UpdateMouseEvent()
{
    if (m_BtnClose.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_OPTION);
        return false;
    }

    // Process the resolution combo box BEFORE checkboxes/sliders. The expanded
    // dropdown overflows below the option window and overlaps the windowed-mode
    // checkbox; if checkboxes ran first, a click on a dropdown item would also
    // toggle the checkbox behind it. Consume the click here when it's inside
    // the combo's hit box (closed widget OR open dropdown).
    if (m_ResolutionCombo.UpdateMouseEvent())
    {
        m_iResolutionIndex = m_ResolutionCombo.GetSelectedIndex();
        ApplyResolution();
        return false;
    }
    if (m_ResolutionCombo.IsMouseOverWidget())
        return false;

    bool oldWindowedMode = m_bWindowedMode;
    HandleCheckboxInputs();

    // Apply windowed/fullscreen toggle. Saving to config alone isn't enough —
    // we also have to swap the window style and display mode live so the user
    // sees the switch without restarting.
    if (m_bWindowedMode != oldWindowedMode)
    {
        // Capture the size we INTEND to end up at. WM_SIZE fires multiple times
        // during the style/display transition and can stomp WindowWidth/Height
        // with stale outer-chrome values; we force-correct back to these at the
        // end of the transition.
        const unsigned int desiredW = WindowWidth;
        const unsigned int desiredH = WindowHeight;

        g_bUseWindowMode = m_bWindowedMode ? TRUE : FALSE;
        GameConfig::GetInstance().SetWindowMode(m_bWindowedMode);
        GameConfig::GetInstance().Save();

        if (g_hWnd)
        {
            bool switched = false;
            if (m_bWindowedMode)
            {
                // Fullscreen → windowed: restore default display mode + OS-framed style.
                // Use the same restrictive style as the initial CreateWindow path
                // (Winmain.cpp) and ApplyResolution() so window behaviour is identical
                // regardless of which UI route the user took to enter windowed mode.
                ChangeDisplaySettings(nullptr, 0);

                DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
                            | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN
                            | WS_VISIBLE;
                SetWindowLongPtr(g_hWnd, GWL_STYLE, style);

                RECT windowRect = { 0, 0, (LONG)WindowWidth, (LONG)WindowHeight };
                AdjustWindowRect(&windowRect, style, FALSE);
                SetWindowPos(g_hWnd, HWND_NOTOPMOST, 100, 100,
                             windowRect.right - windowRect.left,
                             windowRect.bottom - windowRect.top,
                             SWP_SHOWWINDOW | SWP_FRAMECHANGED);
                switched = true;
            }
            else
            {
                // Windowed → fullscreen: try to change display mode to current
                // (WindowWidth × WindowHeight, desktop bpp). Windows can reject
                // this when a previous display change is still settling, so use
                // a drain-pump + retry loop, and first force-reset the display
                // so we always start from a known clean desktop mode.
                DEVMODE dmScreenSettings = {};
                dmScreenSettings.dmSize        = sizeof(dmScreenSettings);
                dmScreenSettings.dmPelsWidth   = WindowWidth;
                dmScreenSettings.dmPelsHeight  = WindowHeight;
                dmScreenSettings.dmBitsPerPel  = GetDesktopBitsPerPel();
                dmScreenSettings.dmFields      = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

                auto pumpMessages = []()
                {
                    MSG msg;
                    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                };

                // Clean slate: reset to desktop default, drain messages, then
                // try the real change. This avoids "DISP_CHANGE_FAILED because
                // a change is already in progress" after rapid toggles.
                ChangeDisplaySettings(nullptr, 0);
                pumpMessages();

                LONG result = DISP_CHANGE_FAILED;
                for (int attempt = 0; attempt < 3; attempt++)
                {
                    result = ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
                    if (result == DISP_CHANGE_SUCCESSFUL) break;
                    pumpMessages();
                }

                if (result == DISP_CHANGE_SUCCESSFUL)
                {
                    // Trust the requested mode. Mid-transition SM_CXSCREEN returns
                    // stale desktop/chrome dimensions, and EnumDisplaySettings
                    // CURRENT reads the persistent registry mode (CDS_FULLSCREEN
                    // is temporary and doesn't update the registry) — both lied
                    // to us in practice. If the monitor actually substitutes a
                    // different mode, the user can pick a supported resolution
                    // explicitly from the dropdown.
                    DWORD style = WS_POPUP | WS_VISIBLE;
                    SetWindowLongPtr(g_hWnd, GWL_STYLE, style);
                    SetWindowPos(g_hWnd, HWND_TOPMOST, 0, 0, (int)WindowWidth, (int)WindowHeight,
                                 SWP_SHOWWINDOW | SWP_FRAMECHANGED);
                    switched = true;
                }
                else
                {
                    m_bWindowedMode   = true;
                    g_bUseWindowMode  = TRUE;
                    GameConfig::GetInstance().SetWindowMode(true);
                    GameConfig::GetInstance().Save();
                }
            }

            // Whether the switch succeeded or got rolled back, both branches went
            // through ChangeDisplaySettings (and sometimes SetWindowPos), which
            // queue WM_ACTIVATE / WM_DISPLAYCHANGE. If a stray WM_ACTIVATE(INACTIVE)
            // lands and we don't re-activate, g_bWndActive stays false — IsPress()
            // short-circuits on it, silently dropping every click on the checkbox
            // until *any* other window op (e.g. a resolution resize) reactivates
            // the window. So always drain + re-activate after an attempt, even
            // when the mode switch itself didn't succeed.
            MSG msg;
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            SetForegroundWindow(g_hWnd);
            SetFocus(g_hWnd);
            g_bWndActive = true;

            // WM_SIZE fired during the transition can leave WindowWidth/Height
            // at a stale outer-chrome value (seen as 1296×1063 when we asked
            // for 1280×1024). Force them back to the requested size, resync
            // scale factors, and re-issue SetWindowPos so the actual client
            // rect matches. Only relevant when the mode switch succeeded and
            // the final window should be at `desired*`.
            if (switched && (WindowWidth != desiredW || WindowHeight != desiredH))
            {
                WindowWidth        = desiredW;
                WindowHeight       = desiredH;
                g_fScreenRate_x    = (float)WindowWidth  / (float)REFERENCE_WIDTH;
                g_fScreenRate_y    = (float)WindowHeight / (float)REFERENCE_HEIGHT;
                OpenglWindowWidth  = (int)WindowWidth;
                OpenglWindowHeight = (int)WindowHeight;
                if (m_bWindowedMode)
                {
                    RECT windowRect = { 0, 0, (LONG)desiredW, (LONG)desiredH };
                    AdjustWindowRect(&windowRect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN, FALSE);
                    SetWindowPos(g_hWnd, HWND_TOP, 0, 0,
                                 windowRect.right - windowRect.left,
                                 windowRect.bottom - windowRect.top,
                                 SWP_NOMOVE | SWP_NOZORDER);
                }
                else
                {
                    SetWindowPos(g_hWnd, HWND_TOPMOST, 0, 0, (int)desiredW, (int)desiredH,
                                 SWP_SHOWWINDOW | SWP_FRAMECHANGED);
                }
                ReinitializeFonts();
                UpdateResolutionDependentSystems();
            }

            UpdateCursorClip();  // clip in fullscreen, release in windowed

            // Clear the in-flight VK_LBUTTON press so the same click doesn't
            // toggle again next frame. User must release and click again.
            g_pNewKeyInput->SetKeyState(VK_LBUTTON, SEASON3B::CNewKeyInput::KEY_NONE);
        }
    }

    if (HandleVolumeSlider(m_iVolumeLevel, 104))
        OnSoundVolumeChanged();

    if (HandleVolumeSlider(m_iMusicLevel, 132))
        OnMusicVolumeChanged();

    HandleRenderLevelSlider();

    // Combo box already processed at the top. Just consume clicks inside the
    // option window itself so they don't fall through to the world.
    if (CheckMouseIn(m_Pos.x, m_Pos.y, 190, 362))
        return false;

    return true;
}

void SEASON3B::CNewUIOptionWindow::HandleCheckboxInputs()
{
    struct Checkbox { int yLocal; bool* target; };
    const Checkbox boxes[] = {
        {  43, &m_bAutoAttack        },
        {  65, &m_bWhisperSound      },
        { 155, &m_bSlideHelp         },
        { 238, &m_bRenderAllEffects  },
        { 300, &m_bWindowedMode      },
    };

    constexpr int CHECKBOX_X_LOCAL = 150;
    constexpr int CHECKBOX_SIZE = 15;

    if (!SEASON3B::IsPress(VK_LBUTTON))
        return;

    for (const auto& cb : boxes)
    {
        if (CheckMouseIn(m_Pos.x + CHECKBOX_X_LOCAL, m_Pos.y + cb.yLocal, CHECKBOX_SIZE, CHECKBOX_SIZE))
            *cb.target = !*cb.target;
    }
}

// Handles wheel + drag input on a volume slider track.
// Returns true if the level changed this frame.
bool SEASON3B::CNewUIOptionWindow::HandleVolumeSlider(int& level, int yOffset)
{
    if (!CheckMouseIn(m_Pos.x + SLIDER_X_LOCAL - SLIDER_HIT_PADDING,
                      m_Pos.y + yOffset,
                      SLIDER_WIDTH + SLIDER_HIT_PADDING,
                      SLIDER_HIT_HEIGHT))
    {
        return false;
    }

    const int oldValue = level;

    if (MouseWheel > 0)
    {
        MouseWheel = 0;
        level++;
    }
    else if (MouseWheel < 0)
    {
        MouseWheel = 0;
        level--;
    }

    if (SEASON3B::IsRepeat(VK_LBUTTON))
    {
        int x = MouseX - (m_Pos.x + SLIDER_X_LOCAL);
        if (x < 0)
            level = 0;
        else
            level = (int)(((float)MAX_VOLUME * x) / (float)SLIDER_WIDTH + 0.5f);
    }

    // Clamp once after all adjustments
    level = std::clamp(level, 0, MAX_VOLUME);

    return (level != oldValue);
}

void SEASON3B::CNewUIOptionWindow::OnSoundVolumeChanged()
{
    m_SoundOnOff = (m_iVolumeLevel > 0) ? 1 : 0;
    SetEffectVolumeLevel(m_iVolumeLevel);
    GameConfig::GetInstance().SetSoundVolume(m_iVolumeLevel);
    GameConfig::GetInstance().Save();
}

void SEASON3B::CNewUIOptionWindow::OnMusicVolumeChanged()
{
    // Mute via volume only — do NOT wzAudioStop() the stream. Once stopped,
    // wzAudioSetVolume on its own won't revive it, so raising the slider back
    // up leaves the user with silence until the next scene change triggers
    // PlayMp3 for a *different* track. Keeping the stream alive at volume 0
    // means raising the slider becomes audible immediately.
    m_MusicOnOff = (m_iMusicLevel > 0) ? 1 : 0;

    wzAudioSetMixerMode(_mmInternalVolume);
    wzAudioSetVolume(m_iMusicLevel * WZAUDIO_VOLUME_SCALE);

    GameConfig::GetInstance().SetMusicVolume(m_iMusicLevel);
    GameConfig::GetInstance().Save();
}

void SEASON3B::CNewUIOptionWindow::HandleRenderLevelSlider()
{
    if (!CheckMouseIn(m_Pos.x + RENDER_SLIDER_X_LOCAL, m_Pos.y + RENDER_SLIDER_Y_LOCAL,
                      RENDER_SLIDER_WIDTH, RENDER_SLIDER_HEIGHT))
        return;

    if (!SEASON3B::IsRepeat(VK_LBUTTON))
        return;

    int x = MouseX - (m_Pos.x + RENDER_SLIDER_X_LOCAL);
    m_iRenderLevel = (int)((RENDER_LEVEL_MAX * x) / (float)RENDER_SLIDER_WIDTH + 0.5f);
}


bool SEASON3B::CNewUIOptionWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_OPTION) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_OPTION);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return true;
}

bool SEASON3B::CNewUIOptionWindow::Update()
{
    return true;
}

bool SEASON3B::CNewUIOptionWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderContents();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

float SEASON3B::CNewUIOptionWindow::GetLayerDepth()	//. 10.5f
{
    return 10.5f;
}

float SEASON3B::CNewUIOptionWindow::GetKeyEventOrder()	// 10.f;
{
    return 10.0f;
}

void SEASON3B::CNewUIOptionWindow::OpenningProcess()
{
    // Resync state that may have been changed externally while the window was hidden.
    m_iResolutionIndex = FindCurrentResolutionIndex();
    m_ResolutionCombo.SetSelectedIndex(m_iResolutionIndex);
    m_ResolutionCombo.Close();
    m_bWindowedMode = (g_bUseWindowMode == TRUE);
}

void SEASON3B::CNewUIOptionWindow::ClosingProcess()
{
    m_ResolutionCombo.Close();
}

void SEASON3B::CNewUIOptionWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_button_close.tga", IMAGE_OPTION_BTN_CLOSE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_OPTION_FRAME_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_OPTION_FRAME_DOWN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_top.tga", IMAGE_OPTION_FRAME_UP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_back06(L).tga", IMAGE_OPTION_FRAME_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_back06(R).tga", IMAGE_OPTION_FRAME_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_line.jpg", IMAGE_OPTION_LINE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_point.tga", IMAGE_OPTION_POINT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_check.tga", IMAGE_OPTION_BTN_CHECK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_effect03.tga", IMAGE_OPTION_EFFECT_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_effect04.tga", IMAGE_OPTION_EFFECT_COLOR, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_volume01.tga", IMAGE_OPTION_VOLUME_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_volume02.tga", IMAGE_OPTION_VOLUME_COLOR, GL_LINEAR);
}

void SEASON3B::CNewUIOptionWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_OPTION_BTN_CLOSE);
    DeleteBitmap(IMAGE_OPTION_FRAME_BACK);
    DeleteBitmap(IMAGE_OPTION_FRAME_DOWN);
    DeleteBitmap(IMAGE_OPTION_FRAME_UP);
    DeleteBitmap(IMAGE_OPTION_FRAME_LEFT);
    DeleteBitmap(IMAGE_OPTION_FRAME_RIGHT);
    DeleteBitmap(IMAGE_OPTION_LINE);
    DeleteBitmap(IMAGE_OPTION_POINT);
    DeleteBitmap(IMAGE_OPTION_BTN_CHECK);
    DeleteBitmap(IMAGE_OPTION_EFFECT_BACK);
    DeleteBitmap(IMAGE_OPTION_EFFECT_COLOR);
    DeleteBitmap(IMAGE_OPTION_VOLUME_BACK);
    DeleteBitmap(IMAGE_OPTION_VOLUME_COLOR);
}

void SEASON3B::CNewUIOptionWindow::RenderFrame()
{
    float x, y;
    x = m_Pos.x;
    y = m_Pos.y;
    RenderImage(IMAGE_OPTION_FRAME_BACK, x, y, 190.f, 337.f);
    RenderImage(IMAGE_OPTION_FRAME_UP, x, y, 190.f, 64.f);
    y += 64.f;
    for (int i = 0; i < 23; ++i)
    {
        RenderImage(IMAGE_OPTION_FRAME_LEFT, x, y, 21.f, 10.f);
        RenderImage(IMAGE_OPTION_FRAME_RIGHT, x + 190 - 21, y, 21.f, 10.f);
        y += 10.f;
    }
    RenderImage(IMAGE_OPTION_FRAME_DOWN, x, y, 190.f, 45.f);

    y = m_Pos.y + 60.f;
    RenderImage(IMAGE_OPTION_LINE, x + 18, y, 154.f, 2.f);     // after auto attack
    y += 22.f;
    RenderImage(IMAGE_OPTION_LINE, x + 18, y, 154.f, 2.f);     // after whisper

    y = m_Pos.y + 150.f;
    RenderImage(IMAGE_OPTION_LINE, x + 18, y, 154.f, 2.f);     // after music vol

    y += 22.f;
    RenderImage(IMAGE_OPTION_LINE, x + 18, y, 154.f, 2.f);     // after slide help

    y += 60.f;
    RenderImage(IMAGE_OPTION_LINE, x + 18, y, 154.f, 2.f);     // after render level

    y += 28.f;
    RenderImage(IMAGE_OPTION_LINE, x + 18, y, 154.f, 2.f);     // after render full effects
}

void SEASON3B::CNewUIOptionWindow::RenderContents()
{
    float x, y;
    x = m_Pos.x + 20.f;
    y = m_Pos.y + 46.f;
    RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);       // Auto Attack
    y += 22.f;
    RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);       // Whisper Sound
    y += 22.f;
    RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);       // Sound Volume
    y += 28.f;
    RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);       // Music Volume
    y += 40.f;
    RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);       // Slide Help
    y += 22.f;
    RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);       // Render Level

    y += 60.f;
    RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);       // Render Full Effects

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 48, GlobalText[386]);
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 70, GlobalText[387]);
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 92, L"Sound Volume");
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 120, L"Music Volume");
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 160, GlobalText[919]);
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 182, GlobalText[1840]);
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 242, L"Render Full Effects");

    y += 22.f;
    RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);       // Resolution
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 265, L"Resolution");

    y += 35.f;
    RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);       // Windowed Mode
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 302, L"Windowed Mode");
}

void SEASON3B::CNewUIOptionWindow::RenderButtons()
{
    m_BtnClose.Render();

    if (m_bAutoAttack)
    {
        RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x + 150, m_Pos.y + 43, 15, 15, 0, 0);
    }
    else
    {
        RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x + 150, m_Pos.y + 43, 15, 15, 0, 15.f);
    }

    if (m_bWhisperSound)
    {
        RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x + 150, m_Pos.y + 65, 15, 15, 0, 0);
    }
    else
    {
        RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x + 150, m_Pos.y + 65, 15, 15, 0, 15.f);
    }

    if (m_bSlideHelp)
    {
        RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x + 150, m_Pos.y + 155, 15, 15, 0, 0);
    }
    else
    {
        RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x + 150, m_Pos.y + 155, 15, 15, 0, 15.f);
    }

    RenderImage(IMAGE_OPTION_VOLUME_BACK, m_Pos.x + 33, m_Pos.y + 104, 124.f, 16.f);
    if (m_iVolumeLevel > 0)
    {
        RenderImage(IMAGE_OPTION_VOLUME_COLOR, m_Pos.x + 33, m_Pos.y + 104, 124.f * 0.1f * (m_iVolumeLevel), 16.f);
    }

    // Music volume bar
    RenderImage(IMAGE_OPTION_VOLUME_BACK, m_Pos.x + 33, m_Pos.y + 132, 124.f, 16.f);
    if (m_iMusicLevel > 0)
    {
        RenderImage(IMAGE_OPTION_VOLUME_COLOR, m_Pos.x + 33, m_Pos.y + 132, 124.f * 0.1f * (m_iMusicLevel), 16.f);
    }

    RenderImage(IMAGE_OPTION_EFFECT_BACK, m_Pos.x + 25, m_Pos.y + 196, 141.f, 29.f);
    if (m_iRenderLevel >= 0)
    {
        RenderImage(IMAGE_OPTION_EFFECT_COLOR, m_Pos.x + 25, m_Pos.y + 196, 141.f * 0.2f * (m_iRenderLevel + 1), 29.f);
    }

    if (m_bRenderAllEffects)
    {
        RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x + 150, m_Pos.y + 238, 15, 15, 0, 0);
    }
    else
    {
        RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x + 150, m_Pos.y + 238, 15, 15, 0, 15.f);
    }

    if (m_bWindowedMode)
    {
        RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x + 150, m_Pos.y + 300, 15, 15, 0, 0);
    }
    else
    {
        RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x + 150, m_Pos.y + 300, 15, 15, 0, 15.f);
    }

    // Resolution combo box. Drawn last so its expanded dropdown sits on top
    // of anything else in the window.
    m_ResolutionCombo.Render();
}

void SEASON3B::CNewUIOptionWindow::SetAutoAttack(bool bAuto)
{
    m_bAutoAttack = bAuto;
}

bool SEASON3B::CNewUIOptionWindow::IsAutoAttack()
{
    return m_bAutoAttack;
}

void SEASON3B::CNewUIOptionWindow::SetWhisperSound(bool bSound)
{
    m_bWhisperSound = bSound;
}

bool SEASON3B::CNewUIOptionWindow::IsWhisperSound()
{
    return m_bWhisperSound;
}

void SEASON3B::CNewUIOptionWindow::SetSlideHelp(bool bHelp)
{
    m_bSlideHelp = bHelp;
}

bool SEASON3B::CNewUIOptionWindow::IsSlideHelp()
{
    return m_bSlideHelp;
}

void SEASON3B::CNewUIOptionWindow::SetVolumeLevel(int iVolume)
{
    m_iVolumeLevel = iVolume;
}

int SEASON3B::CNewUIOptionWindow::GetVolumeLevel()
{
    return m_iVolumeLevel;
}

void SEASON3B::CNewUIOptionWindow::SetRenderLevel(int iRender)
{
    m_iRenderLevel = iRender;
}

int SEASON3B::CNewUIOptionWindow::GetRenderLevel()
{
    return m_iRenderLevel;
}

void SEASON3B::CNewUIOptionWindow::SetRenderAllEffects(bool bRenderAllEffects)
{
    m_bRenderAllEffects = bRenderAllEffects;
}

bool SEASON3B::CNewUIOptionWindow::GetRenderAllEffects()
{
    return m_bRenderAllEffects;
}

int SEASON3B::CNewUIOptionWindow::FindCurrentResolutionIndex()
{
    for (int i = 0; i < s_NumResolutions; ++i)
    {
        if (s_Resolutions[i].width == (int)WindowWidth && s_Resolutions[i].height == (int)WindowHeight)
            return i;
    }
    return 8; // default to 1920x1080
}

void SEASON3B::CNewUIOptionWindow::ApplyResolution()
{
    const unsigned int newWidth  = s_Resolutions[m_iResolutionIndex].width;
    const unsigned int newHeight = s_Resolutions[m_iResolutionIndex].height;

    // Change display mode FIRST (for fullscreen) before recreating fonts and
    // the GL text buffer. ReinitializeTextRenderer allocates a DIB sized off
    // g_hDC/WindowWidth/g_fScreenRate, so it must run after the window and
    // display already sit at the new dimensions — otherwise the text-buffer
    // bitmap ends up at a stale size and rendered text vanishes until you
    // bump the resolution again.
    if (g_hWnd && !g_bUseWindowMode)
    {
        DEVMODE dmScreenSettings = {};
        dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth  = newWidth;
        dmScreenSettings.dmPelsHeight = newHeight;
        dmScreenSettings.dmBitsPerPel = GetDesktopBitsPerPel();
        dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
        {
            // Monitor rejected this mode at 32bpp fullscreen — keep the old
            // resolution and snap the combo selection back so the UI doesn't lie.
            m_iResolutionIndex = FindCurrentResolutionIndex();
            m_ResolutionCombo.SetSelectedIndex(m_iResolutionIndex);
            return;
        }
    }

    // Resize the window to the new dimensions.
    if (g_hWnd && g_bUseWindowMode)
    {
        RECT windowRect = { 0, 0, (LONG)newWidth, (LONG)newHeight };
        AdjustWindowRect(&windowRect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN, FALSE);
        SetWindowPos(g_hWnd, HWND_TOP, 0, 0,
                     windowRect.right - windowRect.left,
                     windowRect.bottom - windowRect.top,
                     SWP_NOMOVE | SWP_NOZORDER);
    }
    else if (g_hWnd)
    {
        SetWindowPos(g_hWnd, HWND_TOPMOST, 0, 0,
                     (int)newWidth, (int)newHeight,
                     SWP_SHOWWINDOW | SWP_FRAMECHANGED);
    }

    // SetWindowPos above triggered WM_SIZE synchronously, which already set
    // WindowWidth/Height/screen rates and called ReinitializeFonts() and
    // UpdateResolutionDependentSystems(). Don't repeat that work here — just
    // persist the new size in config.
    GameConfig::GetInstance().SetWindowSize(WindowWidth, WindowHeight);
    GameConfig::GetInstance().Save();

    // Fullscreen SetWindowPos with SWP_FRAMECHANGED deactivated the window;
    // re-assert focus and activation so subsequent clicks aren't dropped.
    if (g_hWnd && !g_bUseWindowMode)
    {
        SetForegroundWindow(g_hWnd);
        SetFocus(g_hWnd);
        g_bWndActive = true;
    }
    UpdateCursorClip();  // rebind cursor to new client rect (or release in windowed)
}
