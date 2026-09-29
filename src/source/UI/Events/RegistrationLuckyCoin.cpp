
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
        m_View.Build();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { m_View.ReloadTheme(); });
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
        // Nothing native left but the 3D coin: the frame, the texts and the buttons are RmlUi.
        EnableAlphaTest();
        RenderLuckyCoin();
        DisableAlphaBlend();
        return true;
    }

    void CRegistrationLuckyCoin::SyncView()
    {
        if (IsVisible())
        {
            // The original's RenderTexts(): white, centred on the 190-unit panel from y 25.
            const DWORD white = RGBA(255, 255, 255, 255);
            const float width = LUCKYCOIN_REG_WIDTH;
            const float top = 25.f;
            wchar_t count[256] = {};
            mu_swprintf(count, I18N::Game::XDCoins, GetRegistCount());
            m_View.SetTexts({{I18N::Game::LuckyCoinRegistration, 0.f, top, width, true, white},
                             {I18N::Game::Register255LuckyCoinsDuringTheEvent, 0.f, top + 40, width, false, white},
                             {I18N::Game::ForAChanceToGet, 0.f, top + 60, width, false, white},
                             {I18N::Game::TheAbsoluteWeapon, 0.f, top + 80, width, false, white},
                             {I18N::Game::PleaseCheckTheWebPageForTheEventDetails, 0.f, top + 100, width, false, white},
                             {I18N::Game::Registered, 0.f, top + 120, width, true, white},
                             {count, 24.f, top + 150, width, true, white}});

            // The original's SetBtnInfo(): both 64 x 29 newui_btn_empty_small with a bold label,
            // Register at the panel's height - 220, Close at y 360.
            const float buttonX = LUCKYCOIN_REG_WIDTH / 2.0f - MSGBOX_BTN_EMPTY_SMALL_WIDTH / 2.0f;
            m_View.SetButtons({{I18N::Game::Register, buttonX, LUCKYCOIN_REG_HEIGHT - 220, m_RegisterLocked,
                                MSGBOX_BTN_EMPTY_SMALL_WIDTH, MSGBOX_BTN_EMPTY_HEIGHT, true, "small"},
                               {I18N::Game::Close388, buttonX, 360.f, false, MSGBOX_BTN_EMPTY_SMALL_WIDTH,
                                MSGBOX_BTN_EMPTY_HEIGHT, true, "small"}});
        }
        m_View.Sync(IsVisible(), m_Pos);
    }

    // Pre-panel proj/view snapshot, restored around EndBitmap()/BeginBitmap() -- same shape as CGoldBowmanLena::Render3D.
    static float s_PreLuckyCoinProj[16];
    static float s_PreLuckyCoinView[16];

    void CRegistrationLuckyCoin::RenderLuckyCoin()
    {
        float x, y, width, height;

        x = GetPos().x - 20;
        y = GetPos().y + 50;

        width = LUCKYCOIN_REG_WIDTH;
        height = LUCKYCOIN_REG_HEIGHT;

        EndBitmap();

        mu::GetRenderer().SetMatrixMode(GL_PROJECTION);
        mu::GetRenderer().PushMatrix();
        mu::GetRenderer().LoadIdentity();
        SetRenderViewport(0, 0, WindowWidth, WindowHeight);
        gluPerspective2(1.f, (float)(WindowWidth) / (float)(WindowHeight), RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);
        mu::GetRenderer().SetMatrixMode(GL_MODELVIEW);
        mu::GetRenderer().PushMatrix();
        mu::GetRenderer().LoadIdentity();
        CameraProjection::GetOpenGLMatrix(g_Camera.Matrix);
        EnableDepthTest();
        EnableDepthMask();

        mu::GetRenderer().ClearDepthBuffer();

        SetItemRotation(true);
        RenderItem3D(x, y, width, height, m_CoinItem->Type, m_CoinItem->Level, 0, 0, true);
        SetItemRotation(false);

        UpdateMousePositionn();

        mu::GetRenderer().SetMatrixMode(GL_MODELVIEW);
        mu::GetRenderer().PopMatrix();
        mu::GetRenderer().SetMatrixMode(GL_PROJECTION);
        mu::GetRenderer().PopMatrix();

        BeginBitmap();
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

        if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, LUCKYCOIN_REG_WIDTH, LUCKYCOIN_REG_HEIGHT).Contains(MouseX, MouseY))
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
        UI::RmlBridge::UnregisterForThemeReload(this);

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
