// NewUIRegistrationLuckyCoin.cpp: implementation of the CRegistrationLuckyCoin class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/Events/NewUIRegistrationLuckyCoin.h"
#include "UI/Core/NewUISystem.h"
#include "Camera/CameraProjection.h"
#include "Render/Renderer/MuRenderer.h"
#include "I18N/All.h"


namespace mu::ui::window
{
    CRegistrationLuckyCoin::CRegistrationLuckyCoin()
    {
        m_width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
        m_height = MSGBOX_BTN_EMPTY_HEIGHT;
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
        LoadImages();
        SetBtnInfo();
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
        EnableAlphaTest();
        RenderFrame();
        RenderTexts();
        RenderButtons();
        RenderLuckyCoin();
        DisableAlphaBlend();
        return true;
    }

    void CRegistrationLuckyCoin::RenderFrame()
    {
        RenderImage(IMAGE_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
        RenderImage(IMAGE_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
        RenderImage(IMAGE_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
        RenderImage(IMAGE_RIGHT, m_Pos.x + LUCKYCOIN_REG_WIDTH - 21, m_Pos.y + 64, 21.f, 320.f);
        RenderImage(IMAGE_BOTTOM, m_Pos.x, m_Pos.y + LUCKYCOIN_REG_HEIGHT - 45, 190.f, 45.f);
    }

    void CRegistrationLuckyCoin::RenderTexts()
    {
        wchar_t szText[256] = { 0, };
        float _x = GetPos().x;
        float _y = GetPos().y + 25;

        g_pRenderText->SetBgColor(0, 0, 0, 0);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->SetFont(g_hFontBold);
        mu_swprintf(szText, I18N::Game::LuckyCoinRegistration);
        g_pRenderText->RenderText(_x, _y, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);

        g_pRenderText->SetFont(g_hFont);
        mu_swprintf(szText, I18N::Game::Register255LuckyCoinsDuringTheEvent);
        g_pRenderText->RenderText(_x, _y + 40, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);
        mu_swprintf(szText, I18N::Game::ForAChanceToGet);
        g_pRenderText->RenderText(_x, _y + 60, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);
        mu_swprintf(szText, I18N::Game::TheAbsoluteWeapon);
        g_pRenderText->RenderText(_x, _y + 80, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);
        mu_swprintf(szText, I18N::Game::PleaseCheckTheWebPageForTheEventDetails);
        g_pRenderText->RenderText(_x, _y + 100, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);

        g_pRenderText->SetFont(g_hFontBold);

        mu_swprintf(szText, I18N::Game::Registered);
        g_pRenderText->RenderText(_x, _y + 120, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);

        mu_swprintf(szText, I18N::Game::XDCoins, GetRegistCount());
        g_pRenderText->RenderText(_x + 24, _y + 150, szText, LUCKYCOIN_REG_WIDTH, 0, RT3_SORT_CENTER);
    }

    // DXP-07d increment 5, stage 1+2 (implemented together, on trust — see task doc/2026-08-01 note:
    // this panel, like increment 4, is unreachable via normal play until the server implements the
    // corresponding NPC/event, so no runtime soak was possible before this swap). Own copies of the
    // compare/log helpers. Identical shape to increment 4 (CGoldBowmanLena::Render3D): EndBitmap()
    // at entry, restore mirror runs BEFORE BeginBitmap() at the end (own pre-panel snapshot needed).
    // The only per-item call is RenderItem3D() (line 138 below) — same shared path increments 0-4
    // already proved carries no GL model transform; SetItemRotation() (lines 137/139) is a plain
    // bool-field setter (m_ItemAngle), not a GL call, consumed later inside RenderObjectScreen's
    // angle table — verified by reading NewUIRegistrationLuckyCoin.h.
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

    void CRegistrationLuckyCoin::RenderButtons()
    {
        m_CloseButton.Render();
        m_RegistButton.Render();
    }

    bool CRegistrationLuckyCoin::BtnProcess()
    {
        // Top-right corner close "X" (shared frame): hides + swallows the click.
        if (g_pNewUISystem->HandleFrameCornerClose(GetPos(), mu::ui::window::INTERFACE_LUCKYCOIN_REGISTRATION))
            return false;

        if (m_CloseButton.UpdateMouseEvent() == true)
        {
            if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_LUCKYCOIN_REGISTRATION) == true)
            {
                g_pNewUISystem->Hide(mu::ui::window::INTERFACE_LUCKYCOIN_REGISTRATION);
                return true;
            }
            return false;
        }

        if (m_RegistButton.UpdateMouseEvent() == true)
        {
            mu::ui::window::CInventoryCtrl::BackupPickedItem();
            SocketClient->ToGameServer()->SendLuckyCoinRegistrationRequest();
            LockLuckyCoinRegBtn();
            return true;
        }
        return false;
    }

    void CRegistrationLuckyCoin::SetBtnInfo()
    {
        float _x = GetPos().x + LUCKYCOIN_REG_WIDTH / 2.0f - MSGBOX_BTN_EMPTY_SMALL_WIDTH / 2.0f;
        float _y = GetPos().y + LUCKYCOIN_REG_HEIGHT - 220;

        m_RegistButton.ChangeButtonImgState(true, IMAGE_CLOSE_REGIST, true);
        m_RegistButton.ChangeButtonInfo(_x, _y, m_width, m_height);
        m_RegistButton.SetFont(g_hFontBold);
        m_RegistButton.ChangeText(&I18N::Game::Register);
        m_CloseButton.ChangeButtonImgState(true, IMAGE_CLOSE_REGIST, true);
        m_CloseButton.ChangeButtonInfo(_x, 360, m_width, m_height);
        m_CloseButton.SetFont(g_hFontBold);
        m_CloseButton.ChangeText(&I18N::Game::Close388);
    }

    bool CRegistrationLuckyCoin::Update()
    {
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

        if (CheckMouseIn(m_Pos.x, m_Pos.y, LUCKYCOIN_REG_WIDTH, LUCKYCOIN_REG_HEIGHT))
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

    void CRegistrationLuckyCoin::LoadImages()
    {
        LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_BACK, GL_LINEAR);
        LoadBitmap(L"Interface\\newui_item_back04.tga", IMAGE_TOP, GL_LINEAR);
        LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_LEFT, GL_LINEAR);
        LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_RIGHT, GL_LINEAR);
        LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_BOTTOM, GL_LINEAR);
        LoadBitmap(L"Interface\\newui_btn_empty_small.tga", IMAGE_CLOSE_REGIST, GL_LINEAR);
    }

    void CRegistrationLuckyCoin::UnloadImages()
    {
        DeleteBitmap(IMAGE_CLOSE_REGIST);
        DeleteBitmap(IMAGE_BOTTOM);
        DeleteBitmap(IMAGE_RIGHT);
        DeleteBitmap(IMAGE_LEFT);
        DeleteBitmap(IMAGE_TOP);
        DeleteBitmap(IMAGE_BACK);
    }

    void CRegistrationLuckyCoin::Release()
    {
        UnloadImages();

        if (m_pNewUIMng)
        {
            m_pNewUIMng->RemoveUIObj(this);
            m_pNewUIMng = NULL;
        }
    }

    void CRegistrationLuckyCoin::LockLuckyCoinRegBtn()
    {
        m_RegistButton.Lock();
        m_RegistButton.ChangeTextColor(0xff808080);
    }

    void CRegistrationLuckyCoin::UnLockLuckyCoinRegBtn()
    {
        m_RegistButton.UnLock();
        m_RegistButton.ChangeTextColor(0xffffffff);
    }
}
