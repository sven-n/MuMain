//*****************************************************************************
// File: CharMakeWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "CharMakeWin.h"
#include "Input.h"
#include "UIMng.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzOpenglUtil.h"
#include "DSPlaySound.h"
#include "ZzzAI.h"
#include "ZzzScene.h"
#include "UIControls.h"

#include "Local.h"
#include "CharacterManager.h"

#define	CMW_OK		0
#define	CMW_CANCEL	1



extern int g_iChatInputType;
extern CUITextInputBox* g_pSingleTextInputBox;

void MoveCharacterCamera(vec3_t Origin, vec3_t Position, vec3_t Angle);

CCharMakeWin::CCharMakeWin()
{
}

CCharMakeWin::~CCharMakeWin()
{
}

void CCharMakeWin::Create()
{
    CInput rInput = CInput::Instance();
    CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight());

    m_winBack.Create(454, 406, -2);

    m_asprBack[CMW_SPR_INPUT].Create(346, 38, BITMAP_LOG_IN);

    m_asprBack[CMW_SPR_STAT].Create(108, 80);

    m_asprBack[CMW_SPR_DESC].Create(454, 51);

    int i;
    for (i = CMW_SPR_STAT; i < CMW_SPR_MAX; ++i)
    {
        m_asprBack[i].SetAlpha(143);
        m_asprBack[i].SetColor(0, 0, 0);
    }

    DWORD adwJobBtnClr[8] =
    {
        CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, CLRDW_GRAY,
        CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, CLRDW_GRAY
    };

    int nText;
    for (i = 0; i < MAX_CLASS; ++i)
    {
        m_abtnJob[i].Create(108, 26, BITMAP_LOG_IN + 1, 4, 2, 1, 0, 3, 3, 3, 0);
        int _btn_classname[MAX_CLASS] = { 20, 21, 22, 23, 24, 1687, 3150 };
        nText = _btn_classname[i];
        m_abtnJob[i].SetText(GlobalText[nText], adwJobBtnClr);
        CWin::RegisterButton(&m_abtnJob[i]);
    }

    for (i = 0; i < 2; ++i)
    {
        m_aBtn[i].Create(54, 30, BITMAP_BUTTON + i, 3, 2, 1);
        CWin::RegisterButton(&m_aBtn[i]);
    }

    ::memset(m_aszJobDesc, 0,
        sizeof(char) * CMW_DESC_LINE_MAX * CMW_DESC_ROW_MAX);
    m_nDescLine = 0;

    m_nSelJob = CLASS_KNIGHT;
    m_abtnJob[m_nSelJob].SetCheck(true);

    UpdateDisplay();
}

void CCharMakeWin::PreRelease()
{
    for (int i = 0; i < CMW_SPR_MAX; ++i)
        m_asprBack[i].Release();
    m_winBack.Release();
}

void CCharMakeWin::SetPosition(int nXCoord, int nYCoord)
{
    m_winBack.SetPosition(nXCoord, nYCoord);

    int nBaseX = nXCoord + 346;
    m_asprBack[CMW_SPR_STAT].SetPosition(nBaseX, nYCoord + 24);

    int i;
    int nBaseY = nYCoord + 131;
    int nBtnHeight = m_abtnJob[0].GetHeight();
    for (i = 0; i < 3; ++i)
        m_abtnJob[i].SetPosition(nBaseX, nBaseY + i * nBtnHeight);
    m_abtnJob[CLASS_SUMMONER].SetPosition(nBaseX, nBaseY + 3 * nBtnHeight);
    nBaseY = nYCoord + 246;

    m_abtnJob[CLASS_RAGEFIGHTER].SetPosition(nBaseX, nBaseY);

    for (; i <= CLASS_DARK_LORD; ++i)
        m_abtnJob[i].SetPosition(nBaseX, nBaseY + (i - 2) * nBtnHeight);

    nBaseY = nYCoord + 325;
    m_aBtn[CMW_OK].SetPosition(nBaseX, nBaseY);
    m_aBtn[CMW_CANCEL].SetPosition(nXCoord + 400, nBaseY);

    m_asprBack[CMW_SPR_INPUT].SetPosition(nXCoord, nYCoord + 317);

    if (g_iChatInputType == 1)
    {
        g_pSingleTextInputBox->SetPosition(
            int((m_asprBack[CMW_SPR_INPUT].GetXPos() + 78) / g_fScreenRate_x),
            int((m_asprBack[CMW_SPR_INPUT].GetYPos() + 21) / g_fScreenRate_y));
    }

    m_asprBack[CMW_SPR_DESC].SetPosition(nXCoord, nYCoord + 355);
}

void CCharMakeWin::Show(bool bShow)
{
    CWin::Show(bShow);

    int i;
    for (i = 0; i < CMW_SPR_MAX; ++i)
        m_asprBack[i].Show(bShow);

    for (i = 0; i < MAX_CLASS; ++i)
        m_abtnJob[i].Show(bShow);
    for (i = 0; i < 2; ++i)
        m_aBtn[i].Show(bShow);

    if (bShow)
    {
        InputTextWidth = 73;
        ClearInput();
        InputEnable = true;
        InputNumber = 1;
        InputTextMax[0] = MAX_ID_SIZE;
        if (g_iChatInputType == 1)
        {
            g_pSingleTextInputBox->SetState(UISTATE_NORMAL);
            g_pSingleTextInputBox->SetOption(UIOPTION_NULL);
            g_pSingleTextInputBox->SetBackColor(0, 0, 0, 0);
            g_pSingleTextInputBox->SetTextLimit(10);
            g_pSingleTextInputBox->GiveFocus();
        }
    }
    else
    {
        if (g_iChatInputType == 1)
        {
            g_pSingleTextInputBox->SetText(NULL);
            g_pSingleTextInputBox->SetState(UISTATE_HIDE);
        }
    }
}

bool CCharMakeWin::CursorInWin(int nArea)
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

void CCharMakeWin::UpdateDisplay()
{
    int i;

    const int _SecondClassCnt = 3;

    for (i = 0; i <= (MAX_CLASS - 1); ++i)
    {
        m_abtnJob[i].SetEnable(true);
    }

#ifdef PBG_ADD_CHARACTERCARD
    for (i = 0; i < CLASS_CHARACTERCARD_TOTALCNT; ++i)
    {
        if (!g_CharCardEnable.bCharacterEnable[i])
            m_abtnJob[i + CLASS_DARK].SetEnable(false);
    }
#else //PBG_ADD_CHARACTERCARD
    m_abtnJob[CLASS_SUMMONER].SetEnable(true);
#endif //PBG_ADD_CHARACTERCARD

    if (m_nSelJob == CLASS_DARK_LORD)
        m_asprBack[CMW_SPR_STAT].SetSize(0, 96, Y);
    else
        m_asprBack[CMW_SPR_STAT].SetSize(0, 80, Y);

    int nText = m_nSelJob == CLASS_SUMMONER ? 1690 : 1705 + m_nSelJob;
    if (m_nSelJob == CLASS_RAGEFIGHTER)
        nText = 3152;
    m_nDescLine = ::SeparateTextIntoLines(GlobalText[nText], m_aszJobDesc[0], CMW_DESC_LINE_MAX, CMW_DESC_ROW_MAX);

    SelectCreateCharacter();
}

void CCharMakeWin::UpdateWhileActive(double dDeltaTick)
{
    int i, j;
    {
        for (i = 0; i < MAX_CLASS; ++i)
        {
            if (m_abtnJob[i].IsClick())
            {
                for (j = 0; j < MAX_CLASS; ++j)
                    m_abtnJob[j].SetCheck(false);
                m_abtnJob[i].SetCheck(true);

                if (m_nSelJob == i)
                    break;

                m_nSelJob = (CLASS_TYPE)i;
                UpdateDisplay();
                break;
            }
        }
    }

    {
        if (m_aBtn[CMW_OK].IsClick())
        {
            RequestCreateCharacter();
        }
        else if (m_aBtn[CMW_CANCEL].IsClick())
        {
            CUIMng::Instance().HideWin(this);
        }
        else if (CInput::Instance().IsKeyDown(VK_RETURN))
        {
            ::PlayBuffer(SOUND_CLICK01);
            RequestCreateCharacter();
        }
        else if (CInput::Instance().IsKeyDown(VK_ESCAPE))
        {
            ::PlayBuffer(SOUND_CLICK01);
            CUIMng::Instance().HideWin(this);
            CUIMng::Instance().SetSysMenuWinShow(false);
        }
    }
    UpdateCreateCharacter();
}

void CCharMakeWin::RequestCreateCharacter()
{
    if (g_iChatInputType == 1)
        g_pSingleTextInputBox->GetText(InputText[0]);

    CUIMng& rUIMng = CUIMng::Instance();

    const auto character_name = std::wstring(InputText[0]);

    // todo: check with regex from server
    if (character_name.length() < 4)
        rUIMng.PopUpMsgWin(MESSAGE_MIN_LENGTH);
    else if (::CheckName())
        rUIMng.PopUpMsgWin(MESSAGE_ID_SPACE_ERROR);
    else if (CheckSpecialText(InputText[0]))
        rUIMng.PopUpMsgWin(MESSAGE_SPECIAL_NAME);
    else
    {
        BYTE classByte = (BYTE)(((CharacterView.Class) << 2) + (CharacterView.Skin));
        CurrentProtocolState = REQUEST_CREATE_CHARACTER;
        SocketClient->ToGameServer()->SendCreateCharacter(InputText[0], classByte);
        //SendRequestCreateCharacter(InputText[0], CharacterView.Class, CharacterView.Skin);
        rUIMng.HideWin(this);
        rUIMng.PopUpMsgWin(MESSAGE_WAIT);
    }
}

void CCharMakeWin::RenderControls()
{
    RenderCreateCharacter();
    ::EnableAlphaTest();

    int i;
    for (i = 0; i < CMW_SPR_MAX; ++i)
    {
        m_asprBack[i].Render();
    }
    CWin::RenderButtons();
    g_pRenderText->SetFont(g_hFixFont);
    g_pRenderText->SetTextColor(CLRDW_WHITE);
    g_pRenderText->SetBgColor(0);

    wchar_t* apszStat[MAX_CLASS][4] =
    {
        L"18", L"18", L"15", L"30",
        L"28", L"20", L"25", L"10",
        L"22", L"25", L"20", L"15",
        L"26", L"26", L"26", L"26",
        L"26", L"20", L"20", L"15",
        L"21", L"21", L"18", L"23",
        L"32", L"27", L"25", L"20",
    };
    int nStatBaseX = m_asprBack[CMW_SPR_STAT].GetXPos() + 22;
    int nStatY;
    for (i = 0; i < 4; ++i)
    {
        nStatY = int((m_asprBack[CMW_SPR_STAT].GetYPos() + 10 + i * 17)
            / g_fScreenRate_y);

        g_pRenderText->SetTextColor(CLRDW_ORANGE);
        g_pRenderText->RenderText(int((nStatBaseX + 54) / g_fScreenRate_x), nStatY,
            apszStat[m_nSelJob][i]);
        g_pRenderText->SetTextColor(CLRDW_WHITE);
        g_pRenderText->RenderText(int(nStatBaseX / g_fScreenRate_x), nStatY,
            GlobalText[1701 + i]);
    }

    if (m_nSelJob == CLASS_DARK_LORD)
    {
        nStatY = int((m_asprBack[CMW_SPR_STAT].GetYPos() + 10 + 4 * 17) / g_fScreenRate_y);

        g_pRenderText->SetTextColor(CLRDW_ORANGE);
        g_pRenderText->RenderText(int((nStatBaseX + 54) / g_fScreenRate_x), nStatY, L"25");
        g_pRenderText->SetTextColor(CLRDW_WHITE);
        g_pRenderText->RenderText(int(nStatBaseX / g_fScreenRate_x), nStatY, GlobalText[1738]);
    }

    {
        for (i = 0; i < m_nDescLine; ++i)
        {
            g_pRenderText->RenderText(int((m_asprBack[CMW_SPR_DESC].GetXPos() + 10) / g_fScreenRate_x),
                int((m_asprBack[CMW_SPR_DESC].GetYPos() + 12 + i * 19)
                    / g_fScreenRate_y), m_aszJobDesc[i]);
        }
    }

    g_pRenderText->SetFont(g_hFont);

    if (g_iChatInputType == 1)
        g_pSingleTextInputBox->Render();
    else if (g_iChatInputType == 0)
        ::RenderInputText(
            int((m_asprBack[CMW_SPR_INPUT].GetXPos() + 78) / g_fScreenRate_x),
            int((m_asprBack[CMW_SPR_INPUT].GetYPos() + 21) / g_fScreenRate_y),
            0);
}

void CCharMakeWin::SelectCreateCharacter()
{
    CharacterView.Class = m_nSelJob;
    CreateCharacterPointer(&CharacterView, MODEL_FACE + CharacterView.Class, 0, 0);
    CharacterView.Object.Kind = 0;
    SetAction(&CharacterView.Object, 1);
}

void CCharMakeWin::UpdateCreateCharacter()
{
    if (!CharacterAnimation(&CharacterView, &CharacterView.Object))
        SetAction(&CharacterView.Object, 0);
}

void CCharMakeWin::RenderCreateCharacter()
{
    OBJECT* o = &CharacterView.Object;
    vec3_t Position, Angle;

    Vector(1.0f, 1.0f, 1.0f, o->Light);
    Vector(10, -500.f, 48.f, Position);
    Vector(-90.f, 0.f, 0.f, Angle);
    CameraFOV = 10.f;
    MoveCharacterCamera(CharacterView.Object.Position, Position, Angle);

    BeginOpengl(m_winBack.GetXPos() / g_fScreenRate_x, m_winBack.GetYPos() / g_fScreenRate_y, 410 / g_fScreenRate_x, 335 / g_fScreenRate_y);

    if (CharacterView.Class == CLASS_WIZARD)
    {
        Vector(0.f, 0.f, -40.0f, o->Angle);
        o->Scale = 5.9f;
    }
    else if (CharacterView.Class == CLASS_KNIGHT)
    {
        Vector(0.f, 0.f, -12.0f, o->Angle);
        o->Scale = 6.05f;
    }
    else if (CharacterView.Class == CLASS_ELF)
    {
        Vector(8.f, 0.f, 5.0f, o->Angle);
        o->Scale = 9.1f;
        CharacterView.Object.Position[0] += 4.8f;
    }
    else if (CharacterView.Class == CLASS_DARK)
    {
        Vector(8.f, 0.f, -13.0f, o->Angle);
        o->Scale = 6.0f;
        CharacterView.Object.Position[2] += 1.8f;
    }
    else if (CharacterView.Class == CLASS_DARK_LORD)
    {
        Vector(8.f, 0.f, -18.0f, o->Angle);
        o->Scale = 6.0f;
    }
    else if (CharacterView.Class == CLASS_SUMMONER)
    {
        Vector(2.f, 0.f, 2.0f, o->Angle);
        o->Scale = 9.1f;
        CharacterView.Object.Position[0] += 4.8f;
        CharacterView.Object.Position[2] += 4.0f;
    }
    else if (CharacterView.Class == CLASS_RAGEFIGHTER)
    {
        o->Scale = 6.0f;
        CharacterView.Object.Position[0] += 9.8f;
        CharacterView.Object.Position[2] -= 7.5f;
    }

    RenderCharacter(&CharacterView, o);

    glViewport2(0, 0, WindowWidth, WindowHeight);

    EndOpengl();
}