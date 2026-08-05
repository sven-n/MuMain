// NewUIRegistrationLuckyCoin.cpp: implementation of the CNewUIRegistrationLuckyCoin class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/NewUI/Events/NewUIRegistrationLuckyCoin.h"
#include "UI/NewUI/NewUISystem.h"
#include "Camera/CameraProjection.h"
#include "Render/Core/RenderConfig.h"
#include "Render/Core/GlobalUBO.h"
#include "Core/Utilities/Log/ErrorReport.h"
#include "I18N/All.h"


namespace SEASON3B
{
    CNewUIRegistrationLuckyCoin::CNewUIRegistrationLuckyCoin()
    {
        m_width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
        m_height = MSGBOX_BTN_EMPTY_HEIGHT;
        m_RegistCount = 0;
        m_CoinItem = NULL;
        m_ItemAngle = false;
    }

    CNewUIRegistrationLuckyCoin::~CNewUIRegistrationLuckyCoin()
    {
        Release();
    }

    bool CNewUIRegistrationLuckyCoin::Create(CNewUIManager* pNewUIMng, int x, int y)
    {
        if (pNewUIMng == NULL)
            return false;

        m_pNewUIMng = pNewUIMng;
        m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION, this);

        SetPos(x, y);
        LoadImages();
        SetBtnInfo();
        Show(false);
        return true;
    }

    void CNewUIRegistrationLuckyCoin::SetPos(int x, int y)
    {
        m_Pos.x = x;
        m_Pos.y = y;
    }

    bool CNewUIRegistrationLuckyCoin::Render()
    {
        EnableAlphaTest();
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        RenderFrame();
        RenderTexts();
        RenderButtons();
        RenderLuckyCoin();
        DisableAlphaBlend();
        return true;
    }

    void CNewUIRegistrationLuckyCoin::RenderFrame()
    {
        RenderImage(IMAGE_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
        RenderImage(IMAGE_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
        RenderImage(IMAGE_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
        RenderImage(IMAGE_RIGHT, m_Pos.x + LUCKYCOIN_REG_WIDTH - 21, m_Pos.y + 64, 21.f, 320.f);
        RenderImage(IMAGE_BOTTOM, m_Pos.x, m_Pos.y + LUCKYCOIN_REG_HEIGHT - 45, 190.f, 45.f);
    }

    void CNewUIRegistrationLuckyCoin::RenderTexts()
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
    // compare/log helpers. Identical shape to increment 4 (CNewUIGoldBowmanLena::Render3D): EndBitmap()
    // at entry, restore mirror runs BEFORE BeginBitmap() at the end (own pre-panel snapshot needed).
    // The only per-item call is RenderItem3D() (line 138 below) — same shared path increments 0-4
    // already proved carries no GL model transform; SetItemRotation() (lines 137/139) is a plain
    // bool-field setter (m_ItemAngle), not a GL call, consumed later inside RenderObjectScreen's
    // angle table — verified by reading NewUIRegistrationLuckyCoin.h.
    static float s_PreLuckyCoinProj[16];
    static float s_PreLuckyCoinView[16];

    void CNewUIRegistrationLuckyCoin::RenderLuckyCoin()
    {
        float x, y, width, height;

        x = GetPos().x - 20;
        y = GetPos().y + 50;

        width = LUCKYCOIN_REG_WIDTH;
        height = LUCKYCOIN_REG_HEIGHT;

        EndBitmap();

        // Snapshot the CPU source of truth right after EndBitmap() restores it (DXP-07a), before
        // this panel overwrites GlobalUBO — used to check the post-pop restore below for symmetry.
        memcpy(s_PreLuckyCoinProj, GlobalUBO::Instance().GetProj(), sizeof(s_PreLuckyCoinProj));
        memcpy(s_PreLuckyCoinView, GlobalUBO::Instance().GetView(), sizeof(s_PreLuckyCoinView));

        SaveCameraPerspective();
        glViewport2(0, 0, WindowWidth, WindowHeight);
        gluPerspective2(1.f, (float)(WindowWidth) / (float)(WindowHeight), RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);
        // DXP-08a: the matching glMatrixMode/glPushMatrix/glLoadIdentity bracket,
        // CameraProjection::GetOpenGLMatrix(g_Camera.Matrix) read, and shadow-compare diagnostic
        // are deleted — DXP-07d already proved this closed form matches bit-for-bit, and
        // GlobalUBO is the only consumer. This panel's projection is a plain
        // gluPerspective(1.f deg, aspect, RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR) closed
        // form, and its view is identity.
        {
            float aspect = (float)WindowWidth / (float)WindowHeight;
            float fovRad = 1.f * 0.5f * Q_PI / 180.0f;
            float f = 1.0f / tanf(fovRad);
            float zNear = RENDER_ITEMVIEW_NEAR;
            float zFar  = RENDER_ITEMVIEW_FAR;
            float cpuProj[16];
            BuildPerspectiveProjection(f, aspect, zNear, zFar, cpuProj);
            float cpuView[16] = {
                1.f,0.f,0.f,0.f,  0.f,1.f,0.f,0.f,  0.f,0.f,1.f,0.f,  0.f,0.f,0.f,1.f
            };

            GlobalUBO::Instance().SetProj(cpuProj);
            GlobalUBO::Instance().SetView(cpuView);

            // g_Camera.Matrix (3x4 row-major, same layout CameraProjection::GetOpenGLMatrix() used
            // to produce from a GL_MODELVIEW_MATRIX read) is identity here, matching cpuView above.
            static const float s_IdentityCameraMatrix[3][4] = {
                {1.f,0.f,0.f,0.f}, {0.f,1.f,0.f,0.f}, {0.f,0.f,1.f,0.f}
            };
            memcpy(g_Camera.Matrix, s_IdentityCameraMatrix, sizeof(g_Camera.Matrix));
        }
        EnableDepthTest();
        EnableDepthMask();

        ClearDepthBuffer();

        SetItemRotation(true);
        RenderItem3D(x, y, width, height, m_CoinItem->Type, m_CoinItem->Level, 0, 0, true);
        SetItemRotation(false);

        UpdateMousePositionn();

        // DXP-08a: the matching glMatrixMode/glPopMatrix pops and the restore-symmetry
        // shadow-compare are deleted — GlobalUBO is restored directly from the pre-panel snapshot
        // taken at entry.
        RestoreCameraPerspective();
        GlobalUBO::Instance().SetProj(s_PreLuckyCoinProj);
        GlobalUBO::Instance().SetView(s_PreLuckyCoinView);
        BeginBitmap();
    }

    void CNewUIRegistrationLuckyCoin::RenderButtons()
    {
        m_CloseButton.Render();
        m_RegistButton.Render();
    }

    bool CNewUIRegistrationLuckyCoin::BtnProcess()
    {
        // Top-right corner close "X" (shared frame): hides + swallows the click.
        if (g_pNewUISystem->HandleFrameCornerClose(GetPos(), SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION))
            return false;

        if (m_CloseButton.UpdateMouseEvent() == true)
        {
            if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION) == true)
            {
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION);
                return true;
            }
            return false;
        }

        if (m_RegistButton.UpdateMouseEvent() == true)
        {
            SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
            SocketClient->ToGameServer()->SendLuckyCoinRegistrationRequest();
            LockLuckyCoinRegBtn();
            return true;
        }
        return false;
    }

    void CNewUIRegistrationLuckyCoin::SetBtnInfo()
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

    bool CNewUIRegistrationLuckyCoin::Update()
    {
        return true;
    }

    bool CNewUIRegistrationLuckyCoin::UpdateMouseEvent()
    {
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION) == false)
        {
            return true;
        }

        if (BtnProcess() == true)
        {
            return false;
        }

        if (CheckMouseIn(m_Pos.x, m_Pos.y, LUCKYCOIN_REG_WIDTH, LUCKYCOIN_REG_HEIGHT))
        {
            if (SEASON3B::IsPress(VK_RBUTTON))
            {
                MouseRButton = false;
                MouseRButtonPop = false;
                MouseRButtonPush = false;
                return false;
            }

            if (SEASON3B::IsNone(VK_LBUTTON) == false)
            {
                return false;
            }
        }
        return true;
    }

    bool CNewUIRegistrationLuckyCoin::UpdateKeyEvent()
    {
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION) == true)
        {
            if (SEASON3B::IsPress(VK_ESCAPE) == true)
            {
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION);
                return false;
            }
        }
        return true;
    }

    void CNewUIRegistrationLuckyCoin::OpeningProcess()
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

    void CNewUIRegistrationLuckyCoin::ClosingProcess()
    {
        SAFE_DELETE(m_CoinItem);
        g_pMyInventory->GetInventoryCtrl()->UnlockInventory();
        SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();
    }

    void CNewUIRegistrationLuckyCoin::LoadImages()
    {
        LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_BACK, GL_LINEAR);
        LoadBitmap(L"Interface\\newui_item_back04.tga", IMAGE_TOP, GL_LINEAR);
        LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_LEFT, GL_LINEAR);
        LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_RIGHT, GL_LINEAR);
        LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_BOTTOM, GL_LINEAR);
        LoadBitmap(L"Interface\\newui_btn_empty_small.tga", IMAGE_CLOSE_REGIST, GL_LINEAR);
    }

    void CNewUIRegistrationLuckyCoin::UnloadImages()
    {
        DeleteBitmap(IMAGE_CLOSE_REGIST);
        DeleteBitmap(IMAGE_BOTTOM);
        DeleteBitmap(IMAGE_RIGHT);
        DeleteBitmap(IMAGE_LEFT);
        DeleteBitmap(IMAGE_TOP);
        DeleteBitmap(IMAGE_BACK);
    }

    void CNewUIRegistrationLuckyCoin::Release()
    {
        UnloadImages();

        if (m_pNewUIMng)
        {
            m_pNewUIMng->RemoveUIObj(this);
            m_pNewUIMng = NULL;
        }
    }

    void CNewUIRegistrationLuckyCoin::LockLuckyCoinRegBtn()
    {
        m_RegistButton.Lock();
        m_RegistButton.ChangeTextColor(0xff808080);
    }

    void CNewUIRegistrationLuckyCoin::UnLockLuckyCoinRegBtn()
    {
        m_RegistButton.UnLock();
        m_RegistButton.ChangeTextColor(0xffffffff);
    }
}