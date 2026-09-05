// NewUIOptionWindow.cpp: implementation of the CNewUIOptionWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/Options/NewUIOptionWindow.h"
#include "UI/Core/NewUISystem.h"
#include "Render/Textures/ZzzTexture.h"
#include "Audio/DSPlaySound.h"
#include "Data/GameConfig/GameConfig.h"
#include "Data/GameConfig/GameConfigConstants.h"
#include "Audio/AudioPlayer.h"
#include <algorithm>
#include <cstring>
#include "I18N/All.h"

extern int m_MusicOnOff;
extern int m_SoundOnOff;
extern unsigned int WindowWidth, WindowHeight;
extern BOOL g_bUseWindowMode;
void ReinitializeFonts();
std::vector<std::pair<int, int>> MuGetSupportedDisplayResolutions();
void MuApplyWindowResolution(unsigned int width, unsigned int height, bool windowed);
float ConvertX(float x);
float ConvertY(float y);

using namespace SEASON3B;

std::vector<UI::Options::DisplayResolution>
UI::Options::NormalizeDisplayResolutions(std::vector<DisplayResolution> resolutions)
{
    resolutions.erase(std::remove_if(resolutions.begin(), resolutions.end(), [](const DisplayResolution& resolution)
                                     { return resolution.first <= 0 || resolution.second <= 0; }),
                      resolutions.end());
    std::sort(resolutions.begin(), resolutions.end());
    resolutions.erase(std::unique(resolutions.begin(), resolutions.end()), resolutions.end());
    return resolutions;
}

int UI::Options::FindExactDisplayResolutionIndex(const std::vector<DisplayResolution>& resolutions, int width,
                                                 int height)
{
    const auto match = std::find(resolutions.begin(), resolutions.end(), DisplayResolution(width, height));
    if (match == resolutions.end())
    {
        return -1;
    }

    return static_cast<int>(std::distance(resolutions.begin(), match));
}

int UI::Options::FindClosestDisplayResolutionIndex(const std::vector<DisplayResolution>& resolutions, int width,
                                                   int height)
{
    if (resolutions.empty())
    {
        return -1;
    }

    int bestIndex = 0;
    const auto distanceSquared = [width, height](const DisplayResolution& resolution)
    {
        const long long deltaWidth = static_cast<long long>(resolution.first) - width;
        const long long deltaHeight = static_cast<long long>(resolution.second) - height;
        return deltaWidth * deltaWidth + deltaHeight * deltaHeight;
    };
    long long bestDistance = distanceSquared(resolutions.front());

    for (size_t i = 1; i < resolutions.size(); ++i)
    {
        const long long distance = distanceSquared(resolutions[i]);
        if (distance < bestDistance)
        {
            bestIndex = static_cast<int>(i);
            bestDistance = distance;
        }
    }

    return bestIndex;
}

// I18N locale codes (ASCII) paired with the language's display name in that
// language. The set mirrors what ResxGen emits and what I18N::GetAvailableLocales()
// returns at runtime; held here as wide strings so the CNewUIComboBox can show
// them without per-frame UTF-8 -> wide conversions.
static const struct { const char* code; const wchar_t* label; } s_Languages[] = {
    { "en",    L"English" },
    // Non-ASCII characters use universal-character-name escapes so MSVC reads
    // the wide-string literals correctly regardless of source charset.
    { "de",    L"Deutsch" },
    { "es",    L"Espa\u00f1ol" },                                                  // Español
    { "id",    L"Bahasa Indonesia" },
    { "ja",    L"\u65E5\u672C\u8A9E" },                                       // 日本語
    { "pl",    L"Polski" },
    { "pt",    L"Portugu\u00eas" },                                                // Português
    { "ru",    L"\u0420\u0443\u0441\u0441\u043a\u0438\u0439" },                   // Русский
    { "tl",    L"Tagalog" },
    { "uk",    L"\u0423\u043a\u0440\u0430\u0457\u043d\u0441\u044c\u043a\u0430" }, // Українська
    { "zh-TW", L"\u7e41\u9ad4\u4e2d\u6587" },                                      // 繁體中文
};
static const int s_NumLanguages = sizeof(s_Languages) / sizeof(s_Languages[0]);

static const wchar_t* const* GetLanguageLabels()
{
    static const wchar_t* labels[s_NumLanguages] = {};
    static bool initialized = false;
    if (!initialized)
    {
        for (int i = 0; i < s_NumLanguages; i++)
            labels[i] = s_Languages[i].label;
        initialized = true;
    }
    return labels;
}

// UI font families offered by the font combo. `name` is the GameConfig font
// family value ([UI] Font); empty = the platform default (so the look is
// unchanged). The curated entries are bundled in the client's ./fonts directory
// (see Core/Platform/GdiText.cpp), so they resolve even without a system install.
static const struct { const wchar_t* name; const wchar_t* label; } s_Fonts[] = {
    { L"",                L"Default" },
    { L"Liberation Sans", L"Liberation Sans" },
    { L"DejaVu Sans",     L"DejaVu Sans" },
};
static const int s_NumFonts = sizeof(s_Fonts) / sizeof(s_Fonts[0]);

static const wchar_t* const* GetFontLabels()
{
    // Rebuilt on every call so the localized "Default" entry follows a live
    // language switch. The curated font names (Liberation Sans, ...) are proper
    // nouns and stay as-is.
    static const wchar_t* labels[s_NumFonts] = {};
    labels[0] = I18N::Game::DefaultFont;
    for (int i = 1; i < s_NumFonts; i++)
        labels[i] = s_Fonts[i].label;
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

    // Render-level slider ("Effect limitation"). Drawn at ~half the legacy
    // 141x29 and horizontally centered: the window content centers on x+95, so a
    // 70-wide bar starts at x+60. Y nudged down to keep it centered in its row.
    // Both the render (RenderButtons) and the hit test (HandleRenderLevelSlider)
    // read these, so size/position stay in sync.
    constexpr int RENDER_SLIDER_X_LOCAL = 60;
    constexpr int RENDER_SLIDER_Y_LOCAL = 191;
    constexpr int RENDER_SLIDER_WIDTH = 70;
    constexpr int RENDER_SLIDER_HEIGHT = 15;
    constexpr float RENDER_LEVEL_MAX = 5.f;
    // Native size of the effect-bar sprite (the 5 numbered squares). Scaled down
    // to RENDER_SLIDER_WIDTH x HEIGHT via RenderImageStretch, so the whole bar
    // shrinks instead of cropping.
    constexpr int EFFECT_BAR_SRC_WIDTH  = 141;
    constexpr int EFFECT_BAR_SRC_HEIGHT = 29;

    // Resolution combo box placement (relative to m_Pos)
    constexpr int RES_COMBO_X_LOCAL = 22;
    constexpr int RES_COMBO_Y_LOCAL = 335;
    constexpr int RES_COMBO_WIDTH   = 148;  // spans the old left-to-right arrow area
    constexpr int RES_COMBO_HEIGHT  = 16;
    constexpr int RES_COMBO_MAX_VISIBLE = 4;  // scrollbar appears when list > this

    // Language combo box placement (relative to m_Pos).
    constexpr int LANG_LABEL_Y_LOCAL = 283;
    constexpr int LANG_COMBO_X_LOCAL = 22;
    constexpr int LANG_COMBO_Y_LOCAL = 296;
    constexpr int LANG_COMBO_WIDTH   = 148;
    constexpr int LANG_COMBO_HEIGHT  = 16;
    constexpr int LANG_COMBO_MAX_VISIBLE = 5;

    // Font combo box placement (relative to m_Pos). Row order below the effect
    // rows is: Font, Language, Resolution, Windowed mode (combos grouped at the
    // top so an open dropdown never overlaps the Close button).
    constexpr int FONT_LABEL_Y_LOCAL = 244;
    constexpr int FONT_COMBO_X_LOCAL = 22;
    constexpr int FONT_COMBO_Y_LOCAL = 257;
    constexpr int FONT_COMBO_WIDTH   = 148;
    constexpr int FONT_COMBO_HEIGHT  = 16;
    constexpr int FONT_COMBO_MAX_VISIBLE = 5;
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
    m_iResolutionIndex = 0;
    m_bWindowedMode = (g_bUseWindowMode == TRUE);
    m_iLanguageIndex = FindCurrentLanguageIndex();
    m_iFontIndex = FindCurrentFontIndex();
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
    InitLanguageCombo();
    InitFontCombo();
    Show(false);
    return true;
}

void SEASON3B::CNewUIOptionWindow::InitResolutionCombo()
{
    m_resolutions = UI::Options::NormalizeDisplayResolutions(MuGetSupportedDisplayResolutions());
    if (m_resolutions.empty())
    {
        const int fallbackWidth = WindowWidth > 0 ? static_cast<int>(WindowWidth) : CfgDefaults::CfgDefaultWindowWidth;
        const int fallbackHeight =
            WindowHeight > 0 ? static_cast<int>(WindowHeight) : CfgDefaults::CfgDefaultWindowHeight;
        m_resolutions.emplace_back(fallbackWidth, fallbackHeight);
    }

    m_iResolutionIndex = FindCurrentResolutionIndex();
    m_resolutionLabels.clear();
    m_resolutionLabels.reserve(m_resolutions.size());
    for (const auto& [width, height] : m_resolutions)
    {
        m_resolutionLabels.push_back(std::to_wstring(width) + L" x " + std::to_wstring(height));
    }

    m_resolutionLabelPointers.clear();
    m_resolutionLabelPointers.reserve(m_resolutionLabels.size());
    for (const std::wstring& label : m_resolutionLabels)
    {
        m_resolutionLabelPointers.push_back(label.c_str());
    }

    m_ResolutionCombo.Setup(
        m_Pos.x + RES_COMBO_X_LOCAL,
        m_Pos.y + RES_COMBO_Y_LOCAL,
        RES_COMBO_WIDTH,
        RES_COMBO_HEIGHT,
        m_resolutionLabelPointers.data(),
        static_cast<int>(m_resolutions.size()),
        m_iResolutionIndex,
        RES_COMBO_MAX_VISIBLE);
}

void SEASON3B::CNewUIOptionWindow::InitLanguageCombo()
{
    m_LanguageCombo.Setup(
        m_Pos.x + LANG_COMBO_X_LOCAL,
        m_Pos.y + LANG_COMBO_Y_LOCAL,
        LANG_COMBO_WIDTH,
        LANG_COMBO_HEIGHT,
        GetLanguageLabels(),
        s_NumLanguages,
        m_iLanguageIndex,
        LANG_COMBO_MAX_VISIBLE);
}

void SEASON3B::CNewUIOptionWindow::InitFontCombo()
{
    m_FontCombo.Setup(
        m_Pos.x + FONT_COMBO_X_LOCAL,
        m_Pos.y + FONT_COMBO_Y_LOCAL,
        FONT_COMBO_WIDTH,
        FONT_COMBO_HEIGHT,
        GetFontLabels(),
        s_NumFonts,
        m_iFontIndex,
        FONT_COMBO_MAX_VISIBLE);
}

void SEASON3B::CNewUIOptionWindow::SetButtonInfo()
{
    m_BtnClose.ChangeTextBackColor(RGBA(255, 255, 255, 0));
    m_BtnClose.ChangeButtonImgState(true, IMAGE_OPTION_BTN_CLOSE, true);
    m_BtnClose.ChangeButtonInfo(m_Pos.x + 68, m_Pos.y + 388, 54, 30);
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
    m_LanguageCombo.SetPos(m_Pos.x + LANG_COMBO_X_LOCAL, m_Pos.y + LANG_COMBO_Y_LOCAL);
    m_FontCombo.SetPos(m_Pos.x + FONT_COMBO_X_LOCAL, m_Pos.y + FONT_COMBO_Y_LOCAL);
}

bool SEASON3B::CNewUIOptionWindow::UpdateMouseEvent()
{
    // A combo selects on mouse-PRESS and closes its dropdown there and then; the
    // mouse is still held. The Close button fires on RELEASE-over-button, so a
    // press on a dropdown row that happens to sit over Close would pick the item
    // AND, on the release, shut the window (feels like a fast double-click).
    // Once a combo has consumed a click we swallow the rest of that hold until the
    // button comes up, so the release can't fall through to Close.
    if (m_bSwallowClickHold)
    {
        if (!SEASON3B::IsRepeat(VK_LBUTTON))   // button released → hold is over
            m_bSwallowClickHold = false;
        return false;
    }

    // Combos are processed BEFORE the Close button (and the checkboxes/sliders):
    // an open dropdown overflows below the window and can overlap the Close button
    // and the windowed-mode checkbox, so handling combos first lets the dropdown
    // consume the click - picking an item never also closes the window or toggles
    // a control behind it.

    // Z-order matters: an OPEN dropdown is drawn on top, so it must win the click
    // over a closed combo whose field its list overlaps (e.g. the Font dropdown
    // extends down over the Language field). Process the open combo first, then
    // the closed ones - a fixed Resolution/Language/Font order would let the
    // closed combo underneath grab the click and open instead. Selecting an item
    // also sets m_bSwallowClickHold so the still-held press's release can't fall
    // through to the Close button or a checkbox behind the dropdown.
    struct ComboSlot { CNewUIComboBox* combo; int* index; void (CNewUIOptionWindow::*apply)(); };
    const ComboSlot slots[] = {
        { &m_ResolutionCombo, &m_iResolutionIndex, &CNewUIOptionWindow::ApplyResolution },
        { &m_LanguageCombo,   &m_iLanguageIndex,   &CNewUIOptionWindow::ApplyLanguage   },
        { &m_FontCombo,       &m_iFontIndex,       &CNewUIOptionWindow::ApplyFont        },
    };
    for (int pass = 0; pass < 2; ++pass)   // pass 0 = open combo (on top), pass 1 = closed
    {
        for (const ComboSlot& s : slots)
        {
            const bool wasOpen = s.combo->IsOpen();
            if (wasOpen != (pass == 0))
                continue;
            if (s.combo->UpdateMouseEvent())
            {
                *s.index = s.combo->GetSelectedIndex();
                (this->*s.apply)();
                m_bSwallowClickHold = true;
                return false;
            }
            if (s.combo->IsMouseOverWidget())
                return false;
            // A press elsewhere closed this open dropdown (clicked outside, or
            // re-picked the current item): consume it and swallow the hold too.
            if (wasOpen && !s.combo->IsOpen() && SEASON3B::IsPress(VK_LBUTTON))
            {
                m_bSwallowClickHold = true;
                return false;
            }
        }
    }

    // Close button after the combos, so an open dropdown drawn over it wins the
    // click instead of closing the window.
    if (m_BtnClose.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_OPTION);
        return false;
    }

    bool oldWindowedMode = m_bWindowedMode;
    HandleCheckboxInputs();

    if (m_bWindowedMode != oldWindowedMode)
        ApplyWindowModeToggle();

    if (HandleVolumeSlider(m_iVolumeLevel, 104))
        OnSoundVolumeChanged();

    if (HandleVolumeSlider(m_iMusicLevel, 132))
        OnMusicVolumeChanged();

    HandleRenderLevelSlider();

    // Combo box already processed at the top. Just consume clicks inside the
    // option window itself so they don't fall through to the world.
    if (CheckMouseIn(m_Pos.x, m_Pos.y, 190, 419))
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
        { 356, &m_bWindowedMode      },
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
    // Mute via volume only — do not stop the stream.  Once stopped, the
    // current track is gone and raising the slider back up leaves silence
    // until the next scene change triggers PlayMp3 for a different track.
    // Keeping the track alive at gain 0 means raising the slider becomes
    // audible immediately.
    m_MusicOnOff = (m_iMusicLevel > 0) ? 1 : 0;

    AudioPlayer::SetMusicVolume(m_iMusicLevel);

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
    m_bSwallowClickHold = false;   // drop any stale combo click-swallow latch
    InitResolutionCombo();
    m_iLanguageIndex = FindCurrentLanguageIndex();
    m_LanguageCombo.SetSelectedIndex(m_iLanguageIndex);
    m_LanguageCombo.Close();
    m_iFontIndex = FindCurrentFontIndex();
    m_FontCombo.SetSelectedIndex(m_iFontIndex);
    m_FontCombo.Close();
    m_bWindowedMode = (g_bUseWindowMode == TRUE);
}

void SEASON3B::CNewUIOptionWindow::ClosingProcess()
{
    m_ResolutionCombo.Close();
    m_LanguageCombo.Close();
    m_FontCombo.Close();
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
    // Frame is composed of: 64px top + N*10px middle slats + 45px bottom. The
    // slat count is tuned so the frame reaches the Close button (Y 388) plus the
    // bottom border, after the Font/Language/Resolution/Windowed rows.
    constexpr int SLAT_COUNT = 30;
    constexpr float FRAME_HEIGHT = 64.f + SLAT_COUNT * 10.f + 45.f;
    RenderImage(IMAGE_OPTION_FRAME_BACK, x, y, 190.f, FRAME_HEIGHT);
    RenderImage(IMAGE_OPTION_FRAME_UP, x, y, 190.f, 64.f);
    y += 64.f;
    for (int i = 0; i < SLAT_COUNT; ++i)
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

    y += 39.f;
    RenderImage(IMAGE_OPTION_LINE, x + 18, y, 154.f, 2.f);     // after render level

    y += 25.f;
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

    y += 39.f;
    RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);       // Render Full Effects

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 48, I18N::Game::AutomaticAttack);
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 70, I18N::Game::BeepSoundForWhispering);
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 92, I18N::Game::SoundVolume);
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 120, I18N::Game::MusicVolume);
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 160, I18N::Game::SlideHelp);
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 182, I18N::Game::EffectLimitation);
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 221, I18N::Game::RenderFullEffects);

    y += 25.f;
    RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);       // Font
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + FONT_LABEL_Y_LOCAL, I18N::Game::Font);

    y += 39.f;
    RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);       // Language
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + LANG_LABEL_Y_LOCAL, I18N::Game::Language);

    y += 39.f;
    RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);       // Resolution
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 322, I18N::Game::Resolution);

    y += 39.f;
    RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);       // Windowed Mode
    g_pRenderText->RenderText(m_Pos.x + 40, m_Pos.y + 361, I18N::Game::WindowedMode);
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

    RenderImageStretch(IMAGE_OPTION_EFFECT_BACK, m_Pos.x + RENDER_SLIDER_X_LOCAL, m_Pos.y + RENDER_SLIDER_Y_LOCAL,
                       (float)RENDER_SLIDER_WIDTH, (float)RENDER_SLIDER_HEIGHT,
                       0.f, 0.f, (float)EFFECT_BAR_SRC_WIDTH, (float)EFFECT_BAR_SRC_HEIGHT);
    if (m_iRenderLevel >= 0)
    {
        // Reveal proportionally to the level: shrink both the dest width and the
        // sampled source width by the same fraction so the squares stay aligned.
        const float fill = 0.2f * (m_iRenderLevel + 1);
        RenderImageStretch(IMAGE_OPTION_EFFECT_COLOR, m_Pos.x + RENDER_SLIDER_X_LOCAL, m_Pos.y + RENDER_SLIDER_Y_LOCAL,
                           (float)RENDER_SLIDER_WIDTH * fill, (float)RENDER_SLIDER_HEIGHT,
                           0.f, 0.f, (float)EFFECT_BAR_SRC_WIDTH * fill, (float)EFFECT_BAR_SRC_HEIGHT);
    }

    if (m_bRenderAllEffects)
    {
        RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x + 150, m_Pos.y + 217, 15, 15, 0, 0);
    }
    else
    {
        RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x + 150, m_Pos.y + 217, 15, 15, 0, 15.f);
    }

    if (m_bWindowedMode)
    {
        RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x + 150, m_Pos.y + 356, 15, 15, 0, 0);
    }
    else
    {
        RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x + 150, m_Pos.y + 356, 15, 15, 0, 15.f);
    }

    // Combo boxes drawn last so their expanded dropdowns sit on top of
    // anything else in the window. Within the combo pair, render the
    // closed one(s) first and any open dropdown last - otherwise a combo
    // physically below an open one would draw its closed field on top of
    // that open dropdown's list (since they overlap in screen space when
    // the upper one expands downward).
    CNewUIComboBox* combos[] = { &m_ResolutionCombo, &m_LanguageCombo, &m_FontCombo };
    for (auto* c : combos) if (!c->IsOpen()) c->Render();
    for (auto* c : combos) if (c->IsOpen())  c->Render();
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
    return UI::Options::FindClosestDisplayResolutionIndex(m_resolutions, static_cast<int>(WindowWidth),
                                                          static_cast<int>(WindowHeight));
}

int SEASON3B::CNewUIOptionWindow::FindCurrentLanguageIndex()
{
    const char* current = I18N::GetCurrentLocale();
    if (current == nullptr) return 0;
    for (int i = 0; i < s_NumLanguages; ++i)
    {
        if (std::strcmp(s_Languages[i].code, current) == 0)
            return i;
    }
    return 0;  // default to English
}

void SEASON3B::CNewUIOptionWindow::ApplyLanguage()
{
    const char* code = s_Languages[m_iLanguageIndex].code;

    // Persist as wide string so it round-trips cleanly through the existing
    // GameConfig string-IO. Locale codes are ASCII so the conversion is safe.
    std::wstring wide(code, code + std::strlen(code));

    // Re-selecting the active language is a no-op; skip the relocalize and disk write.
    if (GameConfig::GetInstance().GetUILocale() == wide)
        return;

    I18N::SetLocale(code);
    GameConfig::GetInstance().SetUILocale(wide);
    GameConfig::GetInstance().Save();
}

int SEASON3B::CNewUIOptionWindow::FindCurrentFontIndex()
{
    const std::wstring current = GameConfig::GetInstance().GetFontSelection();
    for (int i = 0; i < s_NumFonts; ++i)
    {
        if (current == s_Fonts[i].name)
            return i;
    }
    return 0;  // default ("")
}

void SEASON3B::CNewUIOptionWindow::ApplyFont()
{
    // Re-selecting the active font is a no-op; skip the font rebuild and disk write.
    if (GameConfig::GetInstance().GetFontSelection() == s_Fonts[m_iFontIndex].name)
        return;

    GameConfig::GetInstance().SetFontSelection(s_Fonts[m_iFontIndex].name);
    // Recreate the GDI fonts from config so the change takes effect live.
    ReinitializeFonts();
    GameConfig::GetInstance().Save();
}

void SEASON3B::CNewUIOptionWindow::ApplyResolution()
{
    if (m_iResolutionIndex < 0 || m_iResolutionIndex >= static_cast<int>(m_resolutions.size()))
    {
        return;
    }

    const auto [newWidth, newHeight] = m_resolutions[m_iResolutionIndex];

    // SDL owns the window on every platform, so resize through SDL. The old
    // Windows path drove Win32 SetWindowPos/ChangeDisplaySettings on g_hWnd,
    // which SDL clamped straight back to the current size for a non-resizable
    // window — the resolution only "took" when a windowed/fullscreen toggle
    // reset the window style first (issue #462). MuApplyWindowResolution
    // resizes via SDL regardless of the resizable flag and updates
    // WindowWidth/Height synchronously through HandleWindowResize, so the
    // Save() below records the size the user actually got.
    MuApplyWindowResolution(static_cast<unsigned int>(newWidth), static_cast<unsigned int>(newHeight),
                            g_bUseWindowMode != FALSE);

    // SDL may have coerced the request (closest fullscreen mode, borderless
    // desktop fallback): WindowWidth/Height now hold the size that actually
    // resulted. Persist that, and snap the combo to it so the UI never claims
    // a resolution the display did not adopt.
    SyncResolutionComboToWindow();
    GameConfig::GetInstance().SetWindowSize(WindowWidth, WindowHeight);
    GameConfig::GetInstance().Save();
}

// Point the resolution combo at the mode the window really has. If the actual
// size is not a listed mode, keep the current selection - config still
// records the real size.
void SEASON3B::CNewUIOptionWindow::SyncResolutionComboToWindow()
{
    const int listed = UI::Options::FindExactDisplayResolutionIndex(m_resolutions, static_cast<int>(WindowWidth),
                                                                    static_cast<int>(WindowHeight));
    if (listed < 0)
        return;
    m_iResolutionIndex = listed;
    m_ResolutionCombo.SetSelectedIndex(listed);
}

// Windowed/fullscreen toggle. SDL owns the window, so switch modes through it
// (MuApplyWindowResolution -> SDL_SetWindowFullscreen / SDL_SetWindowSize)
// rather than driving the OS directly: the old Win32 ChangeDisplaySettings /
// SetWindowLongPtr path fought SDL and left its state inconsistent with a
// later resolution change. Keeps the current size and applies the new mode.
void SEASON3B::CNewUIOptionWindow::ApplyWindowModeToggle()
{
    g_bUseWindowMode = m_bWindowedMode ? TRUE : FALSE;
    GameConfig::GetInstance().SetWindowMode(m_bWindowedMode);

    MuApplyWindowResolution(WindowWidth, WindowHeight, m_bWindowedMode);

    // The mode switch may have coerced the size (closest fullscreen mode,
    // borderless desktop fallback), so save AFTER the apply and persist the
    // size that actually resulted along with the new mode.
    SyncResolutionComboToWindow();
    GameConfig::GetInstance().SetWindowSize(WindowWidth, WindowHeight);
    GameConfig::GetInstance().Save();

    // Consume the in-flight VK_LBUTTON press so the same click doesn't
    // toggle again next frame; the user must release and click again.
    g_pNewKeyInput->SetKeyState(VK_LBUTTON, SEASON3B::CNewKeyInput::KEY_NONE);
}
