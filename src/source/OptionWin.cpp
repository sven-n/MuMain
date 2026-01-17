//*****************************************************************************
// File: OptionWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "OptionWin.h"
#include "Input.h"
#include "UIMng.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzScene.h"
#include "DSPlaySound.h"
#include "UIControls.h"
#include "NewUISystem.h"

#define	OW_BTN_GAP		25
#define	OW_SLD_GAP		48




COptionWin::COptionWin()
{
}

COptionWin::~COptionWin()
{
}

void COptionWin::Create()
{
    CInput rInput = CInput::Instance();
    CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight());

    SImgInfo aiiBack[WE_BG_MAX] =
    {
        { BITMAP_SYS_WIN, 0, 0, 128, 128 },
        { BITMAP_OPTION_WIN, 0, 0, 213, 65 },
        { BITMAP_SYS_WIN + 2, 0, 0, 213, 43 },
        { BITMAP_SYS_WIN + 3, 0, 0, 5, 8 },
        { BITMAP_SYS_WIN + 4, 0, 0, 5, 8 }
    };
    m_winBack.Create(aiiBack, 1, 30);
    m_winBack.SetLine(30);

    for (int i = 0; i <= OW_BTN_SLIDE_HELP; ++i)
    {
        m_aBtn[i].Create(16, 16, BITMAP_CHECK_BTN, 2, 0, 0, -1, 1, 1, 1);
        CWin::RegisterButton(&m_aBtn[i]);
    }

    DWORD adwBtnClr[4] = { CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, 0 };
    m_aBtn[OW_BTN_CLOSE].Create(108, 30, BITMAP_TEXT_BTN, 4, 2, 1);
    m_aBtn[OW_BTN_CLOSE].SetText(GlobalText[388], adwBtnClr);
    CWin::RegisterButton(&m_aBtn[OW_BTN_CLOSE]);

    SImgInfo iiThumb = { BITMAP_SLIDER, 0, 0, 13, 13 };
    SImgInfo iiBack = { BITMAP_SLIDER + 2, 0, 0, 98, 13 };
    SImgInfo iiGauge = { BITMAP_SLIDER + 1, 0, 0, 4, 7 };
    RECT rcGauge = { 3, 3, 95, 10 };

    for (int i = 0; i < OW_SLD_MAX; ++i)
        m_aSlider[i].Create(&iiThumb, &iiBack, &iiGauge, &rcGauge);

    m_aSlider[OW_SLD_EFFECT_VOL].SetSlideRange(9);
    m_aSlider[OW_SLD_RENDER_LV].SetSlideRange(4);

    SetPosition((rInput.GetScreenWidth() - m_winBack.GetWidth()) / 2,
        (rInput.GetScreenHeight() - m_winBack.GetHeight()) / 2);

    UpdateDisplay();
}

void COptionWin::PreRelease()
{
    m_winBack.Release();
    for (int i = 0; i < OW_SLD_MAX; ++i)
        m_aSlider[i].Release();
}

void COptionWin::SetPosition(int nXCoord, int nYCoord)
{
    m_winBack.SetPosition(nXCoord, nYCoord);

    int nBtnPosX = m_winBack.GetXPos() + 52;
    int nBtnGap = OW_BTN_GAP + m_aBtn[0].GetHeight();
    int nBtnPosBaseTop = m_winBack.GetYPos() + 52;
    for (int i = 0; i <= OW_BTN_SLIDE_HELP; ++i)
        m_aBtn[i].SetPosition(nBtnPosX, nBtnPosBaseTop + i * nBtnGap);

    m_aBtn[OW_BTN_CLOSE].SetPosition(m_winBack.GetXPos() + (m_winBack.GetWidth() - m_aBtn[OW_BTN_CLOSE].GetWidth()) / 2, m_winBack.GetYPos() + 301);

    //	int nSldPosX = m_winBack.GetXPos()
    //		+ (m_winBack.GetWidth() - m_aSlider[0].GetWidth()) / 2;
    int nSldGap = OW_SLD_GAP + m_aSlider[0].GetHeight();
    int nSldPosBaseTop = m_aBtn[OW_BTN_SLIDE_HELP].GetYPos()
        + m_aBtn[0].GetHeight() + OW_SLD_GAP;
    for (int i = 0; i < OW_SLD_MAX; ++i)
        m_aSlider[i].SetPosition(nBtnPosX, nSldPosBaseTop + i * nSldGap);
}

void COptionWin::Show(bool bShow)
{
    CWin::Show(bShow);

    m_winBack.Show(bShow);
    for (int i = 0; i < OW_BTN_MAX; ++i)
        m_aBtn[i].Show(bShow);
    for (int i = 0; i < OW_SLD_MAX; ++i)
        m_aSlider[i].Show(bShow);
}

bool COptionWin::CursorInWin(int nArea)
{
    if (!CWin::m_bShow)
        return false;

    switch (nArea)
    {
    case WA_MOVE:
        return false;
    }

    return CWin::CursorInWin(nArea);
}

void COptionWin::UpdateDisplay()
{
    m_aBtn[OW_BTN_AUTO_ATTACK].SetCheck(g_pOption->IsAutoAttack());
    m_aBtn[OW_BTN_WHISPER_ALARM].SetCheck(g_pOption->IsWhisperSound());
    m_aBtn[OW_BTN_SLIDE_HELP].SetCheck(g_pOption->IsSlideHelp());
    m_aSlider[OW_SLD_EFFECT_VOL].SetSlidePos(g_pOption->GetVolumeLevel());
    m_aSlider[OW_SLD_RENDER_LV].SetSlidePos(g_pOption->GetRenderLevel());
}

void COptionWin::UpdateWhileActive(double dDeltaTick)
{
    for (int i = 0; i < OW_SLD_MAX; ++i)
        m_aSlider[i].Update(dDeltaTick);

    if (m_aBtn[OW_BTN_AUTO_ATTACK].IsClick())
    {
        g_pOption->SetAutoAttack(m_aBtn[OW_BTN_AUTO_ATTACK].IsCheck());
    }
    else if (m_aBtn[OW_BTN_WHISPER_ALARM].IsClick())
    {
        g_pOption->SetWhisperSound(m_aBtn[OW_BTN_WHISPER_ALARM].IsCheck());
    }
    else if (m_aBtn[OW_BTN_SLIDE_HELP].IsClick())
    {
        g_pOption->SetSlideHelp(m_aBtn[OW_BTN_SLIDE_HELP].IsCheck());
    }
    else if (m_aBtn[OW_BTN_CLOSE].IsClick())
    {
        CUIMng::Instance().HideWin(this);
        CUIMng::Instance().SetSysMenuWinShow(false);
    }
    else if (m_aSlider[OW_SLD_EFFECT_VOL].GetState())
    {
        int nSlidePos = m_aSlider[OW_SLD_EFFECT_VOL].GetSlidePos();

        if (g_pOption->GetVolumeLevel() != nSlidePos)
        {
            g_pOption->SetVolumeLevel(nSlidePos);
            ::SetEffectVolumeLevel(g_pOption->GetVolumeLevel());
        }
    }
    else if (m_aSlider[OW_SLD_RENDER_LV].GetState())
    {
        int nSlidePos = m_aSlider[OW_SLD_RENDER_LV].GetSlidePos();
        if (g_pOption->GetRenderLevel() != nSlidePos)
        {
            g_pOption->SetRenderLevel(nSlidePos);
        }
    }
    else if (CInput::Instance().IsKeyDown(VK_ESCAPE))
    {
        ::PlayBuffer(SOUND_CLICK01);
        CUIMng::Instance().HideWin(this);
        CUIMng::Instance().SetSysMenuWinShow(false);
    }
}

void COptionWin::RenderControls()
{
    m_winBack.Render();

    g_pRenderText->SetFont(g_hFixFont);
    g_pRenderText->SetTextColor(CLRDW_WHITE);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->RenderText(int(m_winBack.GetXPos() / g_fScreenRate_x),
        int((m_winBack.GetYPos() + 10) / g_fScreenRate_y),
        GlobalText[385], m_winBack.GetWidth() / g_fScreenRate_x, 0, RT3_SORT_CENTER);

    const wchar_t* apszBtnText[3] =
    { GlobalText[386], GlobalText[387], GlobalText[919] };
    for (int i = 0; i <= OW_BTN_SLIDE_HELP; ++i)
    {
        g_pRenderText->RenderText(int((m_aBtn[i].GetXPos() + 24) / g_fScreenRate_x),
            int((m_aBtn[i].GetYPos() + 4) / g_fScreenRate_y), apszBtnText[i]);
    }

    int nTextPosY;
    const wchar_t* apszSldText[OW_SLD_MAX] = { GlobalText[389], GlobalText[1840] };
    int anVal[OW_SLD_MAX] = { g_pOption->GetVolumeLevel(), g_pOption->GetRenderLevel() * 2 + 5 };

    wchar_t szVal[3];

    for (int i = 0; i < OW_SLD_MAX; ++i)
    {
        nTextPosY = int((m_aSlider[i].GetYPos() - 18) / g_fScreenRate_y);
        g_pRenderText->RenderText(int(m_aSlider[i].GetXPos() / g_fScreenRate_x), nTextPosY, apszSldText[i]);

        ::_itow(anVal[i], szVal, 10);
        g_pRenderText->RenderText(int((m_aSlider[i].GetXPos() + 85) / g_fScreenRate_x), nTextPosY, szVal);

        m_aSlider[i].Render();
    }

    CWin::RenderButtons();
}