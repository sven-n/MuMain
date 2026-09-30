
#include "stdafx.h"
#include "UI/Events/ExchangeLuckyCoin.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/RmlBridge/RmlTheme.h"

using namespace SEASON3B;
using namespace mu::ui::window;

CExchangeLuckyCoin::CExchangeLuckyCoin()
{
    m_pNewUIMng = NULL;
    memset(&m_Pos, 0, sizeof(POINT));
}

CExchangeLuckyCoin::~CExchangeLuckyCoin()
{
    Release();
}

bool CExchangeLuckyCoin::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_EXCHANGE_LUCKYCOIN, this);

    SetPos(x, y);

    m_View.Build();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { m_View.ReloadTheme(); });

    Show(false);

    return true;
}

void CExchangeLuckyCoin::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CExchangeLuckyCoin::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CExchangeLuckyCoin::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, EXCHANGE_LUCKYCOIN_WINDOW_WIDTH, EXCHANGE_LUCKYCOIN_WINDOW_HEIGHT).Contains(MouseX, MouseY))
        return false;

    return true;
}

bool CExchangeLuckyCoin::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_EXCHANGE_LUCKYCOIN) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_EXCHANGE_LUCKYCOIN);
            return false;
        }
    }

    return true;
}

bool CExchangeLuckyCoin::Update()
{
    SyncView();

    // A click RmlUi reported (the original's button handling in BtnProcess()).
    const int pressed = m_View.TakePressedButton();
    if (!IsVisible())
        return true;
    if (pressed >= 0 && pressed < MAX_EXCHANGE_BTN)
    {
        LockExchangeBtn();
        SocketClient->ToGameServer()->SendLuckyCoinExchangeRequest(static_cast<BYTE>(10 * (pressed + 1)));
    }
    else if (pressed == MAX_EXCHANGE_BTN)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_EXCHANGE_LUCKYCOIN);
    }
    return true;
}

bool CExchangeLuckyCoin::Render()
{
    // Nothing native left: the frame, the texts and the buttons are RmlUi. Kept because CObject
    // requires the override.
    return true;
}

void CExchangeLuckyCoin::SyncView()
{
    if (IsVisible())
    {
        // The original's RenderTexts(): the title and "Exchange" bold white, the warning bold
        // yellow at y 80, the notice white two and three lines (14 units) below it.
        const float width = EXCHANGE_LUCKYCOIN_WINDOW_WIDTH;
        m_View.SetTexts({{I18N::Game::LuckyCoinExchange, width, true},
                         {I18N::Game::Exchange1940, width, true},
                         {I18N::Game::Warning, width, true},
                         {I18N::Game::ExchangedLuckyCoins, width},
                         {I18N::Game::WillNotBeReturned, width}});

        // The original's buttons: the three 108 x 29 newui_btn_empty exchange buttons with a bold
        // label from y 220, 33 units apart; Close, 64 x 29 newui_btn_empty_small, at y 360.
        std::vector<EventItemEntryView::Button> buttons;
        const wchar_t* labels[MAX_EXCHANGE_BTN] = {I18N::Game::Exchange10Coins, I18N::Game::Exchange20Coins,
                                                   I18N::Game::Exchange30Coins};
        for (int i = 0; i < MAX_EXCHANGE_BTN; i++)
        {
            buttons.push_back(
                {labels[i], m_ExchangeLocked, MSGBOX_BTN_EMPTY_WIDTH, MSGBOX_BTN_EMPTY_HEIGHT, true});
        }
        buttons.push_back({I18N::Game::Close388, false, MSGBOX_BTN_EMPTY_SMALL_WIDTH, MSGBOX_BTN_EMPTY_HEIGHT});
        m_View.SetButtons(buttons);
    }
    m_View.Sync(IsVisible(), m_Pos);
}

bool CExchangeLuckyCoin::BtnProcess()
{
    // Top-right corner close "X" (shared frame). Hides + swallows the click. The exchange and
    // Close buttons are RmlUi's (see Update()).
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_EXCHANGE_LUCKYCOIN))
        return true;

    return false;
}

float CExchangeLuckyCoin::GetLayerDepth()
{
    return 4.2f;
}

void CExchangeLuckyCoin::OpenningProcess()
{
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_INVENTORY);
    UnLockExchangeBtn();
    g_pMyInventory->GetInventoryCtrl()->LockInventory();
    PlayBuffer(SOUND_CLICK01);
}

void CExchangeLuckyCoin::ClosingProcess()
{
    PlayBuffer(SOUND_CLICK01);
    g_pMyInventory->GetInventoryCtrl()->UnlockInventory();
    SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();
}

void CExchangeLuckyCoin::LockExchangeBtn()
{
    m_ExchangeLocked = true;
}

void CExchangeLuckyCoin::UnLockExchangeBtn()
{
    m_ExchangeLocked = false;
}
