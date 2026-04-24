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

extern int m_MusicOnOff;
extern int m_SoundOnOff;
extern char Mp3FileName[256];
extern unsigned int WindowWidth, WindowHeight;
extern float g_fScreenRate_x, g_fScreenRate_y;
extern int OpenglWindowWidth, OpenglWindowHeight;
extern BOOL g_bUseWindowMode;
extern HWND g_hWnd;
void ReinitializeFonts();
void UpdateResolutionDependentSystems();
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
    Show(false);
    return true;
}

void SEASON3B::CNewUIOptionWindow::SetButtonInfo()
{
    m_BtnClose.ChangeTextBackColor(RGBA(255, 255, 255, 0));
    m_BtnClose.ChangeButtonImgState(true, IMAGE_OPTION_BTN_CLOSE, true);
    m_BtnClose.ChangeButtonInfo(m_Pos.x + 68, m_Pos.y + 297, 54, 30);
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
}

bool SEASON3B::CNewUIOptionWindow::UpdateMouseEvent()
{
    if (m_BtnClose.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_OPTION);
        return false;
    }

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x + 150, m_Pos.y + 43, 15, 15))
    {
        m_bAutoAttack = !m_bAutoAttack;
    }
    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x + 150, m_Pos.y + 65, 15, 15))
    {
        m_bWhisperSound = !m_bWhisperSound;
    }
    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x + 150, m_Pos.y + 155, 15, 15))
    {
        m_bSlideHelp = !m_bSlideHelp;
    }

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x + 150, m_Pos.y + 238, 15, 15))
    {
        m_bRenderAllEffects = !m_bRenderAllEffects;
    }

    if (CheckMouseIn(m_Pos.x + 33 - 8, m_Pos.y + 104, 124 + 8, 16))
    {
        int iOldValue = m_iVolumeLevel;
        if (MouseWheel > 0)
        {
            MouseWheel = 0;
            m_iVolumeLevel++;
            if (m_iVolumeLevel > 10)
            {
                m_iVolumeLevel = 10;
            }
        }
        else if (MouseWheel < 0)
        {
            MouseWheel = 0;
            m_iVolumeLevel--;
            if (m_iVolumeLevel < 0)
            {
                m_iVolumeLevel = 0;
            }
        }
        if (SEASON3B::IsRepeat(VK_LBUTTON))
        {
            int x = MouseX - (m_Pos.x + 33);
            if (x < 0)
            {
                m_iVolumeLevel = 0;
            }
            else
            {
                float fValue = (10.f * x) / 124.f;
                m_iVolumeLevel = (int)fValue + 1;
            }
        }

        if (iOldValue != m_iVolumeLevel)
        {
            m_SoundOnOff = (m_iVolumeLevel > 0) ? 1 : 0;
            SetEffectVolumeLevel(m_iVolumeLevel);
            GameConfig::GetInstance().SetSoundVolume(m_iVolumeLevel);
            GameConfig::GetInstance().Save();
        }
    }
    // Music volume slider (y+132)
    if (CheckMouseIn(m_Pos.x + 33 - 8, m_Pos.y + 132, 124 + 8, 16))
    {
        int iOldValue = m_iMusicLevel;
        if (MouseWheel > 0)
        {
            MouseWheel = 0;
            m_iMusicLevel++;
            if (m_iMusicLevel > 10) m_iMusicLevel = 10;
        }
        else if (MouseWheel < 0)
        {
            MouseWheel = 0;
            m_iMusicLevel--;
            if (m_iMusicLevel < 0) m_iMusicLevel = 0;
        }
        if (SEASON3B::IsRepeat(VK_LBUTTON))
        {
            int x = MouseX - (m_Pos.x + 33);
            if (x < 0)
                m_iMusicLevel = 0;
            else
                m_iMusicLevel = (int)((10.f * x) / 124.f) + 1;
        }
        if (m_iMusicLevel > 10) m_iMusicLevel = 10;

        if (iOldValue != m_iMusicLevel)
        {
            bool wasOff = (m_MusicOnOff == 0);
            m_MusicOnOff = (m_iMusicLevel > 0) ? 1 : 0;

            // Set music volume using internal mode (don't touch Windows app volume)
            wzAudioSetMixerMode(_mmInternalVolume);
            wzAudioSetVolume(m_iMusicLevel * 10);

            // Clear cached filename so PlayMp3 will actually play when re-enabled
            if (wasOff && m_MusicOnOff)
            {
                Mp3FileName[0] = '\0';
            }
            // Stop music when slider reaches 0
            if (m_iMusicLevel == 0)
            {
                wzAudioStop();
            }

            GameConfig::GetInstance().SetMusicVolume(m_iMusicLevel);
            GameConfig::GetInstance().Save();
        }
    }

    // Render level slider (y+196)
    if (CheckMouseIn(m_Pos.x + 25, m_Pos.y + 196, 141, 29))
    {
        if (SEASON3B::IsRepeat(VK_LBUTTON))
        {
            int x = MouseX - (m_Pos.x + 25);
            float fValue = (5.f * x) / 141.f;
            m_iRenderLevel = (int)fValue;
        }
    }

    // Resolution left arrow box
    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x + 22, m_Pos.y + 279, 15, 15))
    {
        m_iResolutionIndex--;
        if (m_iResolutionIndex < 0) m_iResolutionIndex = s_NumResolutions - 1;
        ApplyResolution();
    }
    // Resolution right arrow box
    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x + 155, m_Pos.y + 279, 15, 15))
    {
        m_iResolutionIndex++;
        if (m_iResolutionIndex >= s_NumResolutions) m_iResolutionIndex = 0;
        ApplyResolution();
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, 190, 337) == true)
    {
        return false;
    }

    return true;
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
}

void SEASON3B::CNewUIOptionWindow::ClosingProcess()
{
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

    // Resolution arrow box backgrounds
    bool bHoverLeft = CheckMouseIn(m_Pos.x + 22, m_Pos.y + 278, 18, 16);
    bool bHoverRight = CheckMouseIn(m_Pos.x + 152, m_Pos.y + 278, 18, 16);

    // Draw solid colored arrow boxes
    glDisable(GL_TEXTURE_2D);
    for (int side = 0; side < 2; side++)
    {
        float bx = ConvertX(side == 0 ? m_Pos.x + 22.f : m_Pos.x + 152.f);
        float by = ConvertY(m_Pos.y + 278.f);
        float bw = ConvertX(18.f);
        float bh = ConvertY(16.f);
        by = (float)WindowHeight - by;
        bool hover = (side == 0) ? bHoverLeft : bHoverRight;

        if (hover)
            glColor4f(0.15f, 0.15f, 0.15f, 1.0f);
        else
            glColor4f(0.05f, 0.05f, 0.05f, 1.0f);

        glBegin(GL_QUADS);
        glVertex2f(bx, by);
        glVertex2f(bx + bw, by);
        glVertex2f(bx + bw, by - bh);
        glVertex2f(bx, by - bh);
        glEnd();
    }
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);

    // Resolution arrows and label
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(255, 230, 200, 255);
    g_pRenderText->RenderText(m_Pos.x + 24, m_Pos.y + 282, L"<<");
    g_pRenderText->RenderText(m_Pos.x + 155, m_Pos.y + 282, L">>");
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->RenderText(m_Pos.x + 50, m_Pos.y + 282, s_Resolutions[m_iResolutionIndex].label);
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
    WindowWidth = s_Resolutions[m_iResolutionIndex].width;
    WindowHeight = s_Resolutions[m_iResolutionIndex].height;

    g_fScreenRate_x = (float)WindowWidth / 640.0f;
    g_fScreenRate_y = (float)WindowHeight / 480.0f;

    OpenglWindowWidth = WindowWidth;
    OpenglWindowHeight = WindowHeight;

    GameConfig::GetInstance().SetWindowSize(WindowWidth, WindowHeight);
    GameConfig::GetInstance().Save();

    ReinitializeFonts();
    UpdateResolutionDependentSystems();

    if (g_bUseWindowMode && g_hWnd)
    {
        RECT windowRect = { 0, 0, (LONG)WindowWidth, (LONG)WindowHeight };
        AdjustWindowRect(&windowRect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN, FALSE);
        int w = windowRect.right - windowRect.left;
        int h = windowRect.bottom - windowRect.top;
        SetWindowPos(g_hWnd, HWND_TOP, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
    }
}
