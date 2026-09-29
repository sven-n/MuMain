
#include "stdafx.h"
#include "I18N/All.h"

#include "UI/Events/BloodCastleEnter.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"

#include "Character/CharacterManager.h"
#include "Audio/DSPlaySound.h"
#include "Core/Text/TextLineWrap.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <string>
#include <vector>

using namespace SEASON3B;
using namespace mu::ui::window;

CEnterBloodCastle::CEnterBloodCastle() : m_View("blood_castle_enter", "Data/Interface/RmlUi/blood_castle_enter.rml")
{
    m_pNewUIMng = NULL;
    memset(&m_Pos, 0, sizeof(POINT));
    memset(&m_EnterUITextPos, 0, sizeof(POINT));

    m_iNumActiveBtn = 1;
    m_BtnEnterStartPos.x = m_BtnEnterStartPos.y = 0;

    m_iBloodCastleLimitLevel[0][0] = 15;  m_iBloodCastleLimitLevel[0][1] = 80;
    m_iBloodCastleLimitLevel[1][0] = 81;  m_iBloodCastleLimitLevel[1][1] = 130;
    m_iBloodCastleLimitLevel[2][0] = 131; m_iBloodCastleLimitLevel[2][1] = 180;
    m_iBloodCastleLimitLevel[3][0] = 181; m_iBloodCastleLimitLevel[3][1] = 230;
    m_iBloodCastleLimitLevel[4][0] = 231; m_iBloodCastleLimitLevel[4][1] = 280;
    m_iBloodCastleLimitLevel[5][0] = 281; m_iBloodCastleLimitLevel[5][1] = 330;
    m_iBloodCastleLimitLevel[6][0] = 331; m_iBloodCastleLimitLevel[6][1] = 400;
    m_iBloodCastleLimitLevel[7][0] = 0;   m_iBloodCastleLimitLevel[7][1] = 0;

    m_iBloodCastleLimitLevel[8][0] = 10;  m_iBloodCastleLimitLevel[8][1] = 60;
    m_iBloodCastleLimitLevel[9][0] = 61;  m_iBloodCastleLimitLevel[9][1] = 110;
    m_iBloodCastleLimitLevel[10][0] = 111; m_iBloodCastleLimitLevel[10][1] = 160;
    m_iBloodCastleLimitLevel[11][0] = 161; m_iBloodCastleLimitLevel[11][1] = 210;
    m_iBloodCastleLimitLevel[12][0] = 211; m_iBloodCastleLimitLevel[12][1] = 260;
    m_iBloodCastleLimitLevel[13][0] = 261; m_iBloodCastleLimitLevel[13][1] = 310;
    m_iBloodCastleLimitLevel[14][0] = 311; m_iBloodCastleLimitLevel[14][1] = 400;
    m_iBloodCastleLimitLevel[15][0] = 0;   m_iBloodCastleLimitLevel[15][1] = 0;
}

CEnterBloodCastle::~CEnterBloodCastle()
{
    Release();
}

//---------------------------------------------------------------------------------------------
// Create
bool CEnterBloodCastle::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_BLOODCASTLE, this);

    SetPos(x, y);

    m_View.Build();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

//---------------------------------------------------------------------------------------------
// Release
void CEnterBloodCastle::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

//---------------------------------------------------------------------------------------------
// SetPos
void CEnterBloodCastle::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
    m_EnterUITextPos.x = m_Pos.x + 3;
    m_EnterUITextPos.y = m_Pos.y + 55;

    SetBtnPos(m_Pos.x + 6, m_Pos.y + 125);

}

//---------------------------------------------------------------------------------------------
// SetBtnPos
void CEnterBloodCastle::SetBtnPos(int x, int y)
{
    m_BtnEnterStartPos.x = x;
    m_BtnEnterStartPos.y = y;
}

//---------------------------------------------------------------------------------------------
// UpdateMouseEvent
bool CEnterBloodCastle::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, ENTERBC_BASE_WINDOW_WIDTH, ENTERBC_BASE_WINDOW_HEIGHT).Contains(MouseX, MouseY))
        return false;

    return true;
}

//---------------------------------------------------------------------------------------------
// UpdateKeyEvent
bool CEnterBloodCastle::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_BLOODCASTLE) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_BLOODCASTLE);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return true;
}

int CEnterBloodCastle::CheckLimitLV(int iIndex)
{
    int	iVal = 0;
    int iRet = 0;

    if (iIndex == 1)
    {
        iVal = 8;
    }

    int iLevel = CharacterAttribute->Level;

    if (gCharacterManager.IsMasterLevel(CharacterAttribute->Class) == false)
    {
        for (int iCastleLV = 0; iCastleLV < MAX_ENTER_GRADE - 1; ++iCastleLV)
        {
            if (iLevel >= m_iBloodCastleLimitLevel[iVal + iCastleLV][0]
                && iLevel <= m_iBloodCastleLimitLevel[iVal + iCastleLV][1])
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

bool CEnterBloodCastle::Update()
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
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_BLOODCASTLE);
        return true;
    }
    if (pressed >= 0 && pressed == m_iNumActiveBtn)
    {
        SocketClient->ToGameServer()->SendBloodCastleEnterRequest(m_iNumActiveBtn + 1, 0xFF);
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_BLOODCASTLE);
    }

    return true;
}

bool CEnterBloodCastle::Render()
{
    // Nothing native left: the frame, the texts and the buttons are RmlUi. Kept because CObject
    // requires the override.
    return true;
}

bool CEnterBloodCastle::BtnProcess()
{
    // Top-right corner close "X" (shared frame). Hides + swallows the click. The exit and level
    // buttons are RmlUi's (see Update()).
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_BLOODCASTLE))
        return true;

    return false;
}

float CEnterBloodCastle::GetLayerDepth()
{
    return 4.1f;
}

void CEnterBloodCastle::OpenningProcess()
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
        mu_swprintf(sztext, I18N::Game::CastleDLevelDD, i + 1
            , m_iBloodCastleLimitLevel[(iLimitLVIndex * MAX_ENTER_GRADE) + i][0]
            , m_iBloodCastleLimitLevel[(iLimitLVIndex * MAX_ENTER_GRADE) + i][1]);
        buttons[i].label = sztext;
    }
    mu_swprintf(sztext, I18N::Game::CastleNoDMasterLevel, 8);
    buttons[MAX_ENTER_GRADE - 1].label = sztext;
    buttons[m_iNumActiveBtn].enabled = true;

    SetViewContent(buttons);
}

void CEnterBloodCastle::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

void CEnterBloodCastle::ReloadRmlTheme()
{
    m_View.ReloadTheme();
}

void CEnterBloodCastle::SetViewContent(const std::vector<EventEntryView::Button>& buttons)
{
    // The description, cut into lines of at most MAX_LENGTH_CMB characters, 20 units apart.
    wchar_t txtline[NUM_LINE_CMB][MAX_LENGTH_CMB] = {0};
    const int tl =
        SeparateTextIntoLines(I18N::Game::YourWillToHelpTheArchangel, txtline[0], NUM_LINE_CMB, MAX_LENGTH_CMB);
    std::vector<std::wstring> lines;
    for (int j = 0; j < tl; ++j)
        lines.emplace_back(txtline[j]);

    m_View.SetContent(I18N::Game::MessengerOfArchangel, lines, float(m_EnterUITextPos.y - m_Pos.y), 20.f, buttons,
                      float(m_BtnEnterStartPos.y - m_Pos.y), float(ENTER_BTN_VAL));
}
