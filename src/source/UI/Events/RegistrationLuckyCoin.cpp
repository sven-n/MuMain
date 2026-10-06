
#include "stdafx.h"
#include "UI/Events/RegistrationLuckyCoin.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "Camera/CameraProjection.h"
#include "Render/Renderer/MuRenderer.h"
#include "I18N/All.h"
#include "UI/RmlBridge/RmlTheme.h"

namespace mu::ui::window
{
    CRegistrationLuckyCoin::CRegistrationLuckyCoin()
    {
        m_RegistCount = 0;
        m_CoinItem = NULL;
        m_ItemAngle = false;
    }

    CRegistrationLuckyCoin::~CRegistrationLuckyCoin()
    {
        Release();
    }

    bool CRegistrationLuckyCoin::Create(CManager* pNewUIMng, int x, int y)
    {
        if (pNewUIMng == NULL)
            return false;

        m_pNewUIMng = pNewUIMng;
        m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_LUCKYCOIN_REGISTRATION, this);

        SetPos(x, y);
        m_View.SetItemDrawer(
            [this]
            {
                if (m_CoinItem)
                    RenderLuckyCoin();
            },
            this);
        m_View.Build();
        Show(false);
        return true;
    }

    void CRegistrationLuckyCoin::SetPos(int x, int y)
    {
        m_Pos.x = x;
        m_Pos.y = y;
    }

    bool CRegistrationLuckyCoin::Render()
    {
        return true;
    }

    void CRegistrationLuckyCoin::SyncView()
    {
        if (IsVisible())
        {
            // The original's RenderTexts(): white, centred on the 190-unit panel from y 25.
            const float width = LUCKYCOIN_REG_WIDTH;
            wchar_t count[256] = {};
            mu_swprintf(count, I18N::Game::XDCoins, GetRegistCount());
            m_View.SetTexts({{I18N::Game::LuckyCoinRegistration, width, true},
                             {I18N::Game::Register255LuckyCoinsDuringTheEvent, width},
                             {I18N::Game::ForAChanceToGet, width},
                             {I18N::Game::TheAbsoluteWeapon, width},
                             {I18N::Game::PleaseCheckTheWebPageForTheEventDetails, width},
                             {I18N::Game::Registered, width, true},
                             {count, width, true}});

            // The original's SetBtnInfo(): both 64 x 29 newui_btn_empty_small with a bold label,
            // Register at the panel's height - 220, Close at y 360.
            m_View.SetButtons({{I18N::Game::Register, m_RegisterLocked, MSGBOX_BTN_EMPTY_SMALL_WIDTH,
                                MSGBOX_BTN_EMPTY_HEIGHT, true},
                               {I18N::Game::Close388, false, MSGBOX_BTN_EMPTY_SMALL_WIDTH,
                                MSGBOX_BTN_EMPTY_HEIGHT, true}});
        }
        m_View.Sync(IsVisible(), m_Pos);
    }

    // Into the document's #entry_item, under the item camera EventItemEntryView sets up.
    void CRegistrationLuckyCoin::RenderLuckyCoin()
    {
        SetItemRotation(true);
        RenderItem3D(GetPos().x - 20.f, GetPos().y + 50.f, LUCKYCOIN_REG_WIDTH, LUCKYCOIN_REG_HEIGHT, m_CoinItem->Type,
                     m_CoinItem->Level, 0, 0, true);
        SetItemRotation(false);
    }

    bool CRegistrationLuckyCoin::BtnProcess()
    {
        // Top-right corner close "X" (shared frame): hides + swallows the click. The Register and
        // Close buttons are RmlUi's (see Update()).
        if (g_pNewUISystem->HandleFrameCornerClose(GetPos(), mu::ui::window::INTERFACE_LUCKYCOIN_REGISTRATION))
            return false;

        return false;
    }

    bool CRegistrationLuckyCoin::Update()
    {
        SyncView();

        // A click RmlUi reported (the original's button handling in BtnProcess()).
        const int pressed = m_View.TakePressedButton();
        if (!IsVisible())
            return true;
        if (pressed == 0)
        {
            mu::ui::window::CInventoryCtrl::BackupPickedItem();
            SocketClient->ToGameServer()->SendLuckyCoinRegistrationRequest();
            LockLuckyCoinRegBtn();
        }
        else if (pressed == 1)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_LUCKYCOIN_REGISTRATION);
        }
        return true;
    }

    bool CRegistrationLuckyCoin::UpdateMouseEvent()
    {
        if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_LUCKYCOIN_REGISTRATION) == false)
        {
            return true;
        }

        if (BtnProcess() == true)
        {
            return false;
        }

        float panelWidth = static_cast<float>(LUCKYCOIN_REG_WIDTH);
        float panelHeight = static_cast<float>(LUCKYCOIN_REG_HEIGHT);
        m_View.RefreshPanelSize(panelWidth, panelHeight);
        if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth),
                                           static_cast<int>(panelHeight)).Contains(MouseX, MouseY))
        {
            if (mu::ui::window::IsPress(VK_RBUTTON))
            {
                MouseRButton = false;
                MouseRButtonPop = false;
                MouseRButtonPush = false;
                return false;
            }

            if (mu::ui::window::IsNone(VK_LBUTTON) == false)
            {
                return false;
            }
        }
        return true;
    }

    bool CRegistrationLuckyCoin::UpdateKeyEvent()
    {
        if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_LUCKYCOIN_REGISTRATION) == true)
        {
            if (mu::ui::window::IsPress(VK_ESCAPE) == true)
            {
                g_pNewUISystem->Hide(mu::ui::window::INTERFACE_LUCKYCOIN_REGISTRATION);
                return false;
            }
        }
        return true;
    }

    void CRegistrationLuckyCoin::OpeningProcess()
    {
        g_pMyInventory->GetInventoryCtrl()->LockInventory();

        m_RegistCount = 0;

        UnLockLuckyCoinRegBtn();

        SocketClient->ToGameServer()->SendLuckyCoinCountRequest();

        m_CoinItem = new ITEM;
        if (m_CoinItem == NULL)	return;
        memset(m_CoinItem, 0, sizeof(ITEM));

        m_CoinItem->Type = ITEM_POTION + 100;
        m_CoinItem->Level = 0;
        m_CoinItem->ExcellentFlags = 0;
        m_CoinItem->AncientDiscriminator = 0;
    }

    void CRegistrationLuckyCoin::ClosingProcess()
    {
        SAFE_DELETE(m_CoinItem);
        g_pMyInventory->GetInventoryCtrl()->UnlockInventory();
        SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();
    }

    void CRegistrationLuckyCoin::Release()
    {
        m_View.Release();

        if (m_pNewUIMng)
        {
            m_pNewUIMng->RemoveUIObj(this);
            m_pNewUIMng = NULL;
        }
    }

    void CRegistrationLuckyCoin::LockLuckyCoinRegBtn()
    {
        m_RegisterLocked = true;
    }

    void CRegistrationLuckyCoin::UnLockLuckyCoinRegBtn()
    {
        m_RegisterLocked = false;
    }
}
