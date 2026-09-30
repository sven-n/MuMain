
#include "stdafx.h"
#include "I18N/All.h"

#include "UI/Events/EnterDevilSquare.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Character/CharacterManager.h"
#include "Audio/DSPlaySound.h"
#include "Core/Text/TextLineWrap.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <string>
#include <vector>

using namespace SEASON3B;
using namespace mu::ui::window;

CEnterDevilSquare::CEnterDevilSquare() : m_View("devil_square_enter", "Data/Interface/RmlUi/devil_square_enter.rml")
{
    m_pNewUIMng = NULL;
    memset(&m_Pos, 0, sizeof(POINT));

    m_iNumActiveBtn = 1;

    m_iDevilSquareLimitLevel[0][0] = 15; m_iDevilSquareLimitLevel[0][1] = 130;
    m_iDevilSquareLimitLevel[1][0] = 131; m_iDevilSquareLimitLevel[1][1] = 180;
    m_iDevilSquareLimitLevel[2][0] = 181; m_iDevilSquareLimitLevel[2][1] = 230;
    m_iDevilSquareLimitLevel[3][0] = 231; m_iDevilSquareLimitLevel[3][1] = 280;
    m_iDevilSquareLimitLevel[4][0] = 281; m_iDevilSquareLimitLevel[4][1] = 330;
    m_iDevilSquareLimitLevel[5][0] = 331; m_iDevilSquareLimitLevel[5][1] = 400;
    m_iDevilSquareLimitLevel[6][0] = 0; m_iDevilSquareLimitLevel[6][1] = 0;

    m_iDevilSquareLimitLevel[7][0] = 15; m_iDevilSquareLimitLevel[7][1] = 110;
    m_iDevilSquareLimitLevel[8][0] = 111; m_iDevilSquareLimitLevel[8][1] = 160;
    m_iDevilSquareLimitLevel[9][0] = 161; m_iDevilSquareLimitLevel[9][1] = 210;
    m_iDevilSquareLimitLevel[10][0] = 211; m_iDevilSquareLimitLevel[10][1] = 260;
    m_iDevilSquareLimitLevel[11][0] = 261; m_iDevilSquareLimitLevel[11][1] = 310;
    m_iDevilSquareLimitLevel[12][0] = 311; m_iDevilSquareLimitLevel[12][1] = 400;
    m_iDevilSquareLimitLevel[13][0] = 0; m_iDevilSquareLimitLevel[13][1] = 0;
}

CEnterDevilSquare::~CEnterDevilSquare()
{
    Release();
}

bool CEnterDevilSquare::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_DEVILSQUARE, this);

    SetPos(x, y);

    m_View.Build();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void CEnterDevilSquare::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CEnterDevilSquare::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CEnterDevilSquare::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, ENTERDS_BASE_WINDOW_WIDTH, ENTERDS_BASE_WINDOW_HEIGHT).Contains(MouseX, MouseY))
        return false;

    return true;
}

bool CEnterDevilSquare::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_DEVILSQUARE) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_DEVILSQUARE);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return true;
}

bool CEnterDevilSquare::Update()
{
    m_View.Sync(IsVisible(), m_Pos);

    // Clicks RmlUi reported: the exit button hides the window, the enabled level button asks the
    // server to enter that level band (the original's BtnProcess()).
    const bool exitPressed = m_View.TakeExitPressed();
    const int pressed = m_View.TakePressedButton();
    if (!IsVisible())
        return true;
    if (exitPressed)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_DEVILSQUARE);
        return true;
    }
    if (pressed >= 0 && pressed == m_iNumActiveBtn)
    {
        SocketClient->ToGameServer()->SendDevilSquareEnterRequest(m_iNumActiveBtn, 0xFF);
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_DEVILSQUARE);
    }

    return true;
}

bool CEnterDevilSquare::Render()
{
    // Nothing native left: the frame, the texts and the buttons are RmlUi. Kept because CObject
    // requires the override.
    return true;
}

//---------------------------------------------------------------------------------------------
// BtnProcess
bool CEnterDevilSquare::BtnProcess()
{
    // Top-right corner close "X" (shared frame). Hides + swallows the click. The exit and level
    // buttons are RmlUi's (see Update()).
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_DEVILSQUARE))
        return true;

    return false;
}

float CEnterDevilSquare::GetLayerDepth()
{
    return 4.0f;
}

int CEnterDevilSquare::CheckLimitLV(int iIndex)
{
    int	iVal = 0;
    int iRet = 0;

    if (iIndex == 1)
    {
        iVal = 7;
    }

    int iLevel;
    if (gCharacterManager.IsMasterLevel(CharacterAttribute->Class) == true)
        iLevel = Master_Level_Data.nMLevel;
    else
        iLevel = CharacterAttribute->Level;
    //Master_Level_Data.nMLevel

    if (gCharacterManager.IsMasterLevel(CharacterAttribute->Class) == false)
    {
        for (int iCastleLV = 0; iCastleLV < MAX_ENTER_GRADE - 1; ++iCastleLV)
        {
            if (iLevel >= m_iDevilSquareLimitLevel[iVal + iCastleLV][0]
                && iLevel <= m_iDevilSquareLimitLevel[iVal + iCastleLV][1])
            {
                iRet = iCastleLV;
                break;
            }
        }
    }
    else
        iRet = MAX_ENTER_GRADE - 1;

    return iRet;
}

void CEnterDevilSquare::OpenningProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();

    int iLimitLVIndex = 0;
    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD
        || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
    {
        iLimitLVIndex = 1;
    }

    m_iNumActiveBtn = CheckLimitLV(iLimitLVIndex);

    // Every button locked (grey, never hovered) but the hero's band.
    std::vector<EventEntryView::Button> buttons(MAX_ENTER_GRADE);
    wchar_t sztext[255] = { 0, };
    for (int i = 0; i < MAX_ENTER_GRADE - 1; i++)
    {
        mu_swprintf(sztext, I18N::Game::TheDSquareDDLevel, i + 1,
                    m_iDevilSquareLimitLevel[(iLimitLVIndex * MAX_ENTER_GRADE) + i][0],
                    m_iDevilSquareLimitLevel[(iLimitLVIndex * MAX_ENTER_GRADE) + i][1]);
        buttons[i].label = sztext;
    }
    mu_swprintf(sztext, I18N::Game::SquareNoDMasterLevel, 7);
    buttons[MAX_ENTER_GRADE - 1].label = sztext;
    buttons[m_iNumActiveBtn].enabled = true;

    SetViewContent(buttons);
}

void CEnterDevilSquare::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}


//---------------------------------------------------------------------------------------------
// UnloadImages

void CEnterDevilSquare::ReloadRmlTheme()
{
    m_View.ReloadTheme();
}

void CEnterDevilSquare::SetViewContent(const std::vector<EventEntryView::Button>& buttons)
{
    // Six description lines.
    const std::vector<std::wstring> lines = {
        I18N::Game::YouVeBeenGivenAChanceToProveYourBravery,
        I18N::Game::NoOneHasEverEnteredTheDevilSquareYet,
        I18N::Game::NoHumanHasEverGoneThere,
        I18N::Game::DoNotBelieveAnythingYouSeeInThere,
        I18N::Game::OnlyTrustYourBraveryAndStrength,
        I18N::Game::OnlyYourBraveryAndStrengthWillKeepYouAlive,
    };

    m_View.SetContent(I18N::Game::DevilSquare, lines, buttons);
}
