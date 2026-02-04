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
#include "Scenes/SceneCore.h"
#include "UIControls.h"

#include "Local.h"
#include "CharacterManager.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cwchar>
#include <iterator>
#include <string>

namespace
{
    constexpr std::array<DWORD, BTN_IMG_MAX> kJobButtonColors{
        CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, CLRDW_GRAY,
        CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, CLRDW_GRAY
    };

    constexpr std::array<int, MAX_CLASS> kClassButtonTextIds{
        20, 21, 22, 23, 24, 1687, 3150
    };

    constexpr std::size_t kMinCharacterNameLength = 4;

    constexpr int kSummonerDescriptionTextId = 1690;
    constexpr int kRageFighterDescriptionTextId = 3152;
    constexpr int kDefaultDescriptionBase = 1705;

    constexpr int kStatLabelBaseId = 1701;
    constexpr int kStatLineSpacing = 17;
    constexpr int kStatYOffset = 10;
    constexpr int kStatValueOffset = 54;
    constexpr int kStatTextOffsetX = 22;
    constexpr int kDarkLordStatHeight = 96;
    constexpr int kDefaultStatHeight = 80;
    constexpr const wchar_t* kDarkLordLeadershipStatValue = L"25";
    constexpr int kDarkLordLeadershipTextId = 1738;

    constexpr int kJobButtonStartY = 131;
    constexpr int kJobButtonSummonerRow = 3;
    constexpr int kJobButtonRageFighterY = 246;
    constexpr int kOkButtonOffsetY = 325;
    constexpr int kCancelButtonOffsetX = 400;
    constexpr int kInputSpriteOffsetY = 317;
    constexpr int kInputTextOffsetX = 78;
    constexpr int kInputTextOffsetY = 21;
    constexpr int kDescSpriteOffsetY = 355;
    constexpr int kStatSpriteOffsetY = 24;
    constexpr int kDescriptionTextOffsetX = 10;
    constexpr int kDescriptionTextOffsetY = 12;
    constexpr int kDescriptionLineSpacing = 19;

    struct ClassStats
    {
        std::array<const wchar_t*, 4> values;
    };

    constexpr std::array<ClassStats, MAX_CLASS> kClassStatTable{ {
        ClassStats{ { L"18", L"18", L"15", L"30" } }, // Knight
        ClassStats{ { L"28", L"20", L"25", L"10" } }, // Wizard
        ClassStats{ { L"22", L"25", L"20", L"15" } }, // Elf
        ClassStats{ { L"26", L"26", L"26", L"26" } }, // Magic Gladiator
        ClassStats{ { L"26", L"20", L"20", L"15" } }, // Dark Lord
        ClassStats{ { L"21", L"21", L"18", L"23" } }, // Summoner
        ClassStats{ { L"32", L"27", L"25", L"20" } }, // Rage Fighter
    } };

    struct ClassRenderParameters
    {
        bool overrideAngle;
        float angleX;
        float angleY;
        float angleZ;
        float scale;
        float positionOffsetX;
        float positionOffsetZ;
    };

    constexpr ClassRenderParameters GetRenderParameters(CLASS_TYPE classType)
    {
        switch (classType)
        {
        case CLASS_KNIGHT:
            return { true, 0.0f, 0.0f, -12.0f, 6.05f, 0.0f, 0.0f };
        case CLASS_WIZARD:
            return { true, 0.0f, 0.0f, -40.0f, 5.9f, 0.0f, 0.0f };
        case CLASS_ELF:
            return { true, 8.0f, 0.0f, 5.0f, 9.1f, 4.8f, 0.0f };
        case CLASS_DARK:
            return { true, 8.0f, 0.0f, -13.0f, 6.0f, 0.0f, 1.8f };
        case CLASS_DARK_LORD:
            return { true, 8.0f, 0.0f, -18.0f, 6.0f, 0.0f, 0.0f };
        case CLASS_SUMMONER:
            return { true, 2.0f, 0.0f, 2.0f, 9.1f, 4.8f, 4.0f };
        case CLASS_RAGEFIGHTER:
            return { false, 0.0f, 0.0f, 0.0f, 6.0f, 9.8f, -7.5f };
        default:
            return { false, 0.0f, 0.0f, 0.0f, 6.0f, 0.0f, 0.0f };
        }
    }

    constexpr int ResolveDescriptionTextId(CLASS_TYPE selectedClass)
    {
        if (selectedClass == CLASS_SUMMONER)
            return kSummonerDescriptionTextId;
        if (selectedClass == CLASS_RAGEFIGHTER)
            return kRageFighterDescriptionTextId;
        return kDefaultDescriptionBase + selectedClass;
    }
}

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
    CInput& rInput = CInput::Instance();
    CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight());

    m_winBack.Create(454, 406, -2);

    m_asprBack[CMW_SPR_INPUT].Create(346, 38, BITMAP_LOG_IN);

    m_asprBack[CMW_SPR_STAT].Create(108, 80);

    m_asprBack[CMW_SPR_DESC].Create(454, 51);

    for (int spriteIndex = CMW_SPR_STAT; spriteIndex < CMW_SPR_MAX; ++spriteIndex)
    {
        m_asprBack[spriteIndex].SetAlpha(143);
        m_asprBack[spriteIndex].SetColor(0, 0, 0);
    }

    std::array<DWORD, BTN_IMG_MAX> jobButtonColors = kJobButtonColors;
    for (int classIndex = 0; classIndex < MAX_CLASS; ++classIndex)
    {
        m_abtnJob[classIndex].Create(108, 26, BITMAP_LOG_IN + 1, 4, 2, 1, 0, 3, 3, 3, 0);
        const int textId = kClassButtonTextIds[classIndex];
        m_abtnJob[classIndex].SetText(GlobalText[textId], jobButtonColors.data());
        CWin::RegisterButton(&m_abtnJob[classIndex]);
    }

    for (int i = 0; i < 2; ++i)
    {
        m_aBtn[i].Create(54, 30, BITMAP_BUTTON + i, 3, 2, 1);
        CWin::RegisterButton(&m_aBtn[i]);
    }

    std::fill(&m_aszJobDesc[0][0],
        &m_aszJobDesc[0][0] + (CMW_DESC_LINE_MAX * CMW_DESC_ROW_MAX), L'\0');
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

    const int baseX = nXCoord + 346;
    m_asprBack[CMW_SPR_STAT].SetPosition(baseX, nYCoord + kStatSpriteOffsetY);

    const int buttonHeight = m_abtnJob[0].GetHeight();
    int baseY = nYCoord + kJobButtonStartY;

    for (int classIndex = 0; classIndex < 3; ++classIndex)
        m_abtnJob[classIndex].SetPosition(baseX, baseY + classIndex * buttonHeight);

    m_abtnJob[CLASS_SUMMONER].SetPosition(baseX, baseY + kJobButtonSummonerRow * buttonHeight);

    baseY = nYCoord + kJobButtonRageFighterY;
    m_abtnJob[CLASS_RAGEFIGHTER].SetPosition(baseX, baseY);

    for (int classIndex = CLASS_DARK; classIndex <= CLASS_DARK_LORD; ++classIndex)
    {
        const int row = (classIndex - CLASS_DARK) + 1;
        m_abtnJob[classIndex].SetPosition(baseX, baseY + row * buttonHeight);
    }

    baseY = nYCoord + kOkButtonOffsetY;
    m_aBtn[CMW_OK].SetPosition(baseX, baseY);
    m_aBtn[CMW_CANCEL].SetPosition(nXCoord + kCancelButtonOffsetX, baseY);

    m_asprBack[CMW_SPR_INPUT].SetPosition(nXCoord, nYCoord + kInputSpriteOffsetY);

    if (g_iChatInputType == 1)
    {
        g_pSingleTextInputBox->SetPosition(
            int((m_asprBack[CMW_SPR_INPUT].GetXPos() + kInputTextOffsetX) / g_fScreenRate_x),
            int((m_asprBack[CMW_SPR_INPUT].GetYPos() + kInputTextOffsetY) / g_fScreenRate_y));
    }

    m_asprBack[CMW_SPR_DESC].SetPosition(nXCoord, nYCoord + kDescSpriteOffsetY);
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
        InputTextMax[0] = MAX_USERNAME_SIZE;
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
    for (auto& button : m_abtnJob)
        button.SetEnable(true);

#ifdef PBG_ADD_CHARACTERCARD
    for (int i = 0; i < CLASS_CHARACTERCARD_TOTALCNT; ++i)
    {
        if (!g_CharCardEnable.bCharacterEnable[i])
            m_abtnJob[i + CLASS_DARK].SetEnable(false);
    }
#else //PBG_ADD_CHARACTERCARD
    m_abtnJob[CLASS_SUMMONER].SetEnable(true);
#endif //PBG_ADD_CHARACTERCARD

    const bool isDarkLord = (m_nSelJob == CLASS_DARK_LORD);
    m_asprBack[CMW_SPR_STAT].SetSize(0, isDarkLord ? kDarkLordStatHeight : kDefaultStatHeight, Y);

    const int descriptionTextId = ResolveDescriptionTextId(m_nSelJob);
    m_nDescLine = ::SeparateTextIntoLines(
        GlobalText[descriptionTextId],
        m_aszJobDesc[0],
        CMW_DESC_LINE_MAX,
        CMW_DESC_ROW_MAX);

    SelectCreateCharacter();
}

void CCharMakeWin::UpdateWhileActive(double dDeltaTick)
{
    for (int classIndex = 0; classIndex < MAX_CLASS; ++classIndex)
    {
        if (!m_abtnJob[classIndex].IsClick())
            continue;

        for (auto& button : m_abtnJob)
            button.SetCheck(false);
        m_abtnJob[classIndex].SetCheck(true);

        if (m_nSelJob != classIndex)
        {
            m_nSelJob = static_cast<CLASS_TYPE>(classIndex);
            UpdateDisplay();
        }
        break;
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

    const std::wstring characterName = InputText[0];

    // todo: check with regex from server
    if (characterName.length() < kMinCharacterNameLength)
        rUIMng.PopUpMsgWin(MESSAGE_MIN_LENGTH);
    else if (::CheckName())
        rUIMng.PopUpMsgWin(MESSAGE_ID_SPACE_ERROR);
    else if (CheckSpecialText(InputText[0]))
        rUIMng.PopUpMsgWin(MESSAGE_SPECIAL_NAME);
    else
    {
        const std::uint8_t classByte =
            static_cast<std::uint8_t>((CharacterView.Class << 2) + CharacterView.Skin);
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

    for (auto& sprite : m_asprBack)
    {
        sprite.Render();
    }
    CWin::RenderButtons();
    g_pRenderText->SetFont(g_hFixFont);
    g_pRenderText->SetTextColor(CLRDW_WHITE);
    g_pRenderText->SetBgColor(0);

    const auto& stats = kClassStatTable[static_cast<std::size_t>(m_nSelJob)];
    const int statBaseX = m_asprBack[CMW_SPR_STAT].GetXPos() + kStatTextOffsetX;
    for (std::size_t statIndex = 0; statIndex < stats.values.size(); ++statIndex)
    {
        const int statScreenY = int(
            (m_asprBack[CMW_SPR_STAT].GetYPos() + kStatYOffset + static_cast<int>(statIndex) * kStatLineSpacing)
            / g_fScreenRate_y);

        g_pRenderText->SetTextColor(CLRDW_ORANGE);
        g_pRenderText->RenderText(
            int((statBaseX + kStatValueOffset) / g_fScreenRate_x),
            statScreenY,
            stats.values[statIndex]);

        g_pRenderText->SetTextColor(CLRDW_WHITE);
        g_pRenderText->RenderText(
            int(statBaseX / g_fScreenRate_x),
            statScreenY,
            GlobalText[kStatLabelBaseId + static_cast<int>(statIndex)]);
    }

    if (m_nSelJob == CLASS_DARK_LORD)
    {
        const int leadershipY = int(
            (m_asprBack[CMW_SPR_STAT].GetYPos() + kStatYOffset + 4 * kStatLineSpacing) / g_fScreenRate_y);

        g_pRenderText->SetTextColor(CLRDW_ORANGE);
        g_pRenderText->RenderText(
            int((statBaseX + kStatValueOffset) / g_fScreenRate_x),
            leadershipY,
            kDarkLordLeadershipStatValue);
        g_pRenderText->SetTextColor(CLRDW_WHITE);
        g_pRenderText->RenderText(
            int(statBaseX / g_fScreenRate_x),
            leadershipY,
            GlobalText[kDarkLordLeadershipTextId]);
    }

    for (int lineIndex = 0; lineIndex < m_nDescLine; ++lineIndex)
    {
        g_pRenderText->RenderText(
            int((m_asprBack[CMW_SPR_DESC].GetXPos() + kDescriptionTextOffsetX) / g_fScreenRate_x),
            int((m_asprBack[CMW_SPR_DESC].GetYPos() + kDescriptionTextOffsetY + lineIndex * kDescriptionLineSpacing)
                / g_fScreenRate_y),
            m_aszJobDesc[lineIndex]);
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

    const ClassRenderParameters params = GetRenderParameters(CharacterView.Class);
    if (params.overrideAngle)
        Vector(params.angleX, params.angleY, params.angleZ, o->Angle);

    o->Scale = params.scale;

    if (params.positionOffsetX != 0.0f)
        CharacterView.Object.Position[0] += params.positionOffsetX;
    if (params.positionOffsetZ != 0.0f)
        CharacterView.Object.Position[2] += params.positionOffsetZ;

    RenderCharacter(&CharacterView, o);

    glViewport2(0, 0, WindowWidth, WindowHeight);

    EndOpengl();
}