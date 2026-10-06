
#include "stdafx.h"
#include "UI/Events/DoppelGangerWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Effects/ZzzEffect.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzCharacter.h"
#include "I18N/All.h"
#include "UI/RmlBridge/RmlTheme.h"

#include "Audio/DSPlaySound.h"
#include "Render/Text/CUIRenderText.h"
#include "Render/Text/TextWrap.h"

using namespace SEASON3B;
using namespace mu::ui::window;

CDoppelGangerWindow::CDoppelGangerWindow()
{
    m_pNewUIMng = NULL;
    m_pNewUI3DRenderMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iRemainTime = 0;
    m_bIsEnterButtonLocked = FALSE;
}

CDoppelGangerWindow::~CDoppelGangerWindow()
{
    Release();
}

bool CDoppelGangerWindow::Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == pNewUI3DRenderMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_DOPPELGANGER_NPC, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;
    m_pNewUI3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

    SetPos(x, y);

    m_View.Build();

    Show(false);

    return true;
}

void CDoppelGangerWindow::Release()
{
    m_View.Release();

    if (m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->Remove3DRenderObj(this);
        m_pNewUI3DRenderMng = NULL;
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CDoppelGangerWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CDoppelGangerWindow::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    // #panel's own live RCSS size is the source of truth -- INVENTORY_WIDTH/HEIGHT only cover the
    // first frame after Create()/Show(true)/ReloadRmlTheme(), before RmlUi's next layout pass.
    float panelWidth = INVENTORY_WIDTH;
    float panelHeight = INVENTORY_HEIGHT;
    m_View.RefreshPanelSize(panelWidth, panelHeight);
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth),
                                      static_cast<int>(panelHeight))
            .Contains(MouseX, MouseY))
        return false;

    return true;
}

bool CDoppelGangerWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_DOPPELGANGER_NPC) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_DOPPELGANGER_NPC);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool CDoppelGangerWindow::Update()
{
    SyncView();

    // A click RmlUi reported (the original's button handling in BtnProcess()).
    const int pressed = m_View.TakePressedButton();
    if (!IsVisible())
        return true;
    if (pressed == 0)
    {
        SocketClient->ToGameServer()->SendDoppelgangerEnterRequest(0xFF);
    }
    else if (pressed == 1)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_DOPPELGANGER_NPC);
    }
    return true;
}

bool CDoppelGangerWindow::IsVisible() const
{
    return CObject::IsVisible();
}

bool CDoppelGangerWindow::Render()
{
    // Nothing native left but the 3D preview (Render3D()): the frame, the texts and the buttons
    // are RmlUi. Kept because CObject requires the override.
    return true;
}

void CDoppelGangerWindow::SyncView()
{
    if (IsVisible())
    {
        // The original's Render(): every text in (220, 220, 220), the colour RenderFrame() left
        // set. Both descriptions are cut into one buffer, the second over the first without a
        // terminator, so a shorter line keeps the tail of the longer one ("the...sion of a") --
        // shared with the original, kept. The original also dropped the second line of each cut
        // (finding EV1): that came from its uninitialised stack buffer, not from its layout, so
        // those lines are kept here.
        wchar_t szTextOut[2][300] = {};
        std::vector<EventItemEntryView::Text> texts;
        texts.push_back({I18N::Game::Lugard, 160.f, true});
        g_pRenderText->SetFont(g_hFont);
        CutStr(I18N::Game::OnlyThoseInPossessionOfAMirrorOfDimensions, szTextOut[0], 140, 2, 300);
        texts.push_back({szTextOut[0], 190.f});
        texts.push_back({szTextOut[1], 190.f});
        CutStr(I18N::Game::MayPassThroughTheDoppelgangerGate, szTextOut[0], 100, 2, 300);
        texts.push_back({szTextOut[0], 190.f});
        texts.push_back({szTextOut[1], 190.f});
        texts.push_back({I18N::Game::WillYouShowMeYourMirror, 190.f});
        texts.push_back({I18N::Game::MirrorOfDimensions, 190.f, true});
        texts.push_back({I18N::Game::EntryTime, 190.f});
        wchar_t szText[256] = {};
        if (m_iRemainTime == 0)
            mu_swprintf(szText, I18N::Game::YouMayNowEnter);
        else
            mu_swprintf(szText, I18N::Game::EnterAfterDMinutes, m_iRemainTime);
        texts.push_back({szText, 190.f});
        m_View.SetTexts(std::move(texts));

        m_View.SetButtons({{I18N::Game::Enter, m_bIsEnterButtonLocked == TRUE}, {I18N::Game::Close388, false}});
    }
    m_View.Sync(IsVisible(), m_Pos);
}

void CDoppelGangerWindow::Render3D()
{
    RenderItem3D();
}

void CDoppelGangerWindow::RenderItem3D()
{
    POINT ptOrigin = { m_Pos.x, m_Pos.y + 50 };

    int nItemType = (14 * MAX_ITEM_INDEX) + 111;
    int nItemLevel = 0;

    ::RenderItem3D(ptOrigin.x + (190 - 20) / 2, ptOrigin.y + 75, 20.f, 27, nItemType, nItemLevel, 0, 0, false);
}

void CDoppelGangerWindow::OpeningProcess()
{
    LockEnterButton(FALSE);
}

void CDoppelGangerWindow::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

float CDoppelGangerWindow::GetLayerDepth()
{
    return 5.0f;
}

bool CDoppelGangerWindow::BtnProcess()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    // The Enter and Close buttons are RmlUi's (see Update()).
    g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_DOPPELGANGER_NPC);

    return false;
}

void CDoppelGangerWindow::SetRemainTime(int iTime)
{
    m_iRemainTime = iTime;
    if (iTime != 0)
    {
        LockEnterButton(TRUE);
    }
}

void CDoppelGangerWindow::LockEnterButton(BOOL bLock)
{
    m_bIsEnterButtonLocked = bLock;
}
