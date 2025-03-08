// NewUIMixInventory.cpp: implementation of the CNewUIMixInventory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIMixInventory.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "MixMgr.h"
#include "ZzzBMD.h"
#include "ZzzEffect.h"
#include "ZzzObject.h"
#include "ZzzInventory.h"
#include "ZzzInterface.h"
#include "ZzzInfomation.h"
#include "ZzzCharacter.h"

#include "DSPlaySound.h"
#include "SocketSystem.h"

using namespace SEASON3B;

CNewUIMixInventory::CNewUIMixInventory()
{
    m_pNewUIMng = NULL;
    m_pNewInventoryCtrl = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iMixState = MIX_READY;
    m_iMixEffectTimer = 0;
}
CNewUIMixInventory::~CNewUIMixInventory() { Release(); }

bool CNewUIMixInventory::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewUI3DRenderMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MIXINVENTORY, this);

    m_pNewInventoryCtrl = new CNewUIInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::CHAOS_MIX, g_pNewUI3DRenderMng, g_pNewItemMng, this, x + 15, y + 110, 8, 4))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }

    SetPos(x, y);

    LoadImages();

    POINT ptBtn = { m_Pos.x + (long)INVENTORY_WIDTH * (long)0.5f - (long)22.f, m_Pos.y + (long)380 };

    m_BtnMix.ChangeButtonImgState(true, IMAGE_MIXINVENTORY_MIXBTN, false);
    m_BtnMix.ChangeButtonInfo(m_Pos.x + INVENTORY_WIDTH * 0.5f - 22.f, m_Pos.y + 380, 44.f, 35.f);
    m_BtnMix.ChangeToolTipText(GlobalText[591], true);

    m_pNewInventoryCtrl->GetSquareColorNormal(m_fInventoryColor);
    m_pNewInventoryCtrl->GetSquareColorWarning(m_fInventoryWarningColor);

    Show(false);

    return true;
}
void CNewUIMixInventory::Release()
{
    UnloadImages();

    SAFE_DELETE(m_pNewInventoryCtrl);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);
}

void CNewUIMixInventory::SetMixState(int iMixState)
{
    m_iMixState = iMixState;

    if (iMixState == MIX_REQUESTED)
    {
        m_iMixEffectTimer = 50;
        m_pNewInventoryCtrl->LockInventory();
        g_pMyInventory->GetInventoryCtrl()->LockInventory();
        m_BtnMix.Lock();
    }
    else
    {
        m_pNewInventoryCtrl->UnlockInventory();
        g_pMyInventory->GetInventoryCtrl()->UnlockInventory();
        m_BtnMix.UnLock();
    }
}

bool CNewUIMixInventory::InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket)
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
    return false;
}

void CNewUIMixInventory::DeleteItem(int iIndex)
{
    if (m_pNewInventoryCtrl)
    {
        ITEM* pItem = m_pNewInventoryCtrl->FindItem(iIndex);
        if (pItem != NULL)
            m_pNewInventoryCtrl->RemoveItem(pItem);
    }
}

void CNewUIMixInventory::DeleteAllItems()
{
    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->RemoveAllItems();
}

void CNewUIMixInventory::OpeningProcess()
{
    g_MixRecipeMgr.SetPlusChaosRate(0);
    SocketClient->ToGameServer()->SendCrywolfChaosRateBenefitRequest();

    SetMixState(SEASON3B::CNewUIMixInventory::MIX_READY);

    if (g_MixRecipeMgr.GetMixInventoryType() == SEASON3A::MIXTYPE_GOBLIN_NORMAL)
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CChaosMixMenuMsgBoxLayout));
    }
}

bool CNewUIMixInventory::ClosingProcess()
{
    if (g_pMixInventory->GetInventoryCtrl()->GetNumberOfItems() > 0 || CNewUIInventoryCtrl::GetPickedItem() != NULL)
    {
        g_pSystemLogBox->AddText(GlobalText[593], SEASON3B::TYPE_ERROR_MESSAGE);
        return false;
    }

    switch (g_MixRecipeMgr.GetMixInventoryType())
    {
    case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
    case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
    case SEASON3A::MIXTYPE_GOBLIN_ADD380:
    case SEASON3A::MIXTYPE_CASTLE_SENIOR:
    case SEASON3A::MIXTYPE_OSBOURNE:
    case SEASON3A::MIXTYPE_JERRIDON:
    case SEASON3A::MIXTYPE_ELPIS:
    case SEASON3A::MIXTYPE_CHAOS_CARD:
    case SEASON3A::MIXTYPE_CHERRYBLOSSOM:
    case SEASON3A::MIXTYPE_EXTRACT_SEED:
    case SEASON3A::MIXTYPE_SEED_SPHERE:
        SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();
        break;
    case SEASON3A::MIXTYPE_TRAINER:
        SocketClient->ToGameServer()->SendCloseNpcRequest();
        break;
    case SEASON3A::MIXTYPE_ATTACH_SOCKET:
    case SEASON3A::MIXTYPE_DETACH_SOCKET:
        m_SocketListBox.Clear();
        SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();
        break;
    default:
        break;
    }
    g_pMixInventory->DeleteAllItems();
    g_MixRecipeMgr.ClearCheckRecipeResult();
    return true;
}

void CNewUIMixInventory::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    m_pNewInventoryCtrl->SetPos(x + 15, y + 110);
}

bool CNewUIMixInventory::UpdateMouseEvent()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->UpdateMouseEvent())
        return false;

    if (true == InventoryProcess())
        return false;

    if (true == BtnProcess())
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT))
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
bool CNewUIMixInventory::UpdateKeyEvent()
{
    return true;
}
bool CNewUIMixInventory::Update()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
        return false;

    if (IsVisible())
    {
        CheckMixInventory();
        switch (g_MixRecipeMgr.GetMixInventoryType())
        {
        case SEASON3A::MIXTYPE_ATTACH_SOCKET:
        case SEASON3A::MIXTYPE_DETACH_SOCKET:
        {
            if (m_SocketListBox.GetLineNum() == 0)
            {
                wchar_t szText[64] = { 0, };
                wchar_t szSocketText[64] = { 0, };
                for (int i = 0; i < g_MixRecipeMgr.GetFirstItemSocketCount(); ++i)
                {
                    if (g_MixRecipeMgr.GetFirstItemSocketSeedID(i) == SOCKET_EMPTY)
                    {
                        swprintf(szSocketText, GlobalText[2652]);
                    }
                    else
                    {
                        g_SocketItemMgr.CreateSocketOptionText(szSocketText, g_MixRecipeMgr.GetFirstItemSocketSeedID(i), g_MixRecipeMgr.GetFirstItemSocketShpereLv(i));
                    }
                    swprintf(szText, L"%d: %s", i + 1, szSocketText);
                    m_SocketListBox.AddText(i, szText);
                }
                m_SocketListBox.SLSetSelectLine(0);
            }
            else
            {
                if (g_MixRecipeMgr.GetFirstItemSocketCount() == 0)
                {
                    m_SocketListBox.Clear();
                }
            }
            m_SocketListBox.DoAction();
        }
        break;
        }
    }

    return true;
}
bool CNewUIMixInventory::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    RenderFrame();

    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->Render();

    if (GetMixState() >= MIX_REQUESTED && g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);

    DisableAlphaBlend();

    return true;
}

float CNewUIMixInventory::GetLayerDepth()
{
    return 3.4f;
}

CNewUIInventoryCtrl* CNewUIMixInventory::GetInventoryCtrl() const
{
    return m_pNewInventoryCtrl;
}

void CNewUIMixInventory::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    if (pClass)
    {
        auto* pMixInventory = (CNewUIMixInventory*)pClass;
        pMixInventory->RenderMixEffect();
    }
}

void CNewUIMixInventory::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_MIXINVENTORY_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back04.tga", IMAGE_MIXINVENTORY_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_MIXINVENTORY_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_MIXINVENTORY_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_MIXINVENTORY_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_bt_mix.tga", IMAGE_MIXINVENTORY_MIXBTN, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_scrollbar_up.tga", CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLL_TOP);
    LoadBitmap(L"Interface\\newui_scrollbar_m.tga", CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLL_MIDDLE);
    LoadBitmap(L"Interface\\newui_scrollbar_down.tga", CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLL_BOTTOM);
    LoadBitmap(L"Interface\\newui_scroll_on.tga", CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLLBAR_ON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_scroll_off.tga", CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLLBAR_OFF, GL_LINEAR);
}
void CNewUIMixInventory::UnloadImages()
{
    DeleteBitmap(IMAGE_MIXINVENTORY_BOTTOM);
    DeleteBitmap(IMAGE_MIXINVENTORY_RIGHT);
    DeleteBitmap(IMAGE_MIXINVENTORY_LEFT);
    DeleteBitmap(IMAGE_MIXINVENTORY_TOP);
    DeleteBitmap(IMAGE_MIXINVENTORY_BACK);
    DeleteBitmap(IMAGE_MIXINVENTORY_MIXBTN);

    DeleteBitmap(CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLL_TOP);
    DeleteBitmap(CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLL_MIDDLE);
    DeleteBitmap(CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLL_BOTTOM);
    DeleteBitmap(CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLLBAR_ON);
    DeleteBitmap(CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLLBAR_OFF);
}

void CNewUIMixInventory::RenderFrame()
{
    RenderImage(IMAGE_MIXINVENTORY_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
    RenderImage(IMAGE_MIXINVENTORY_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
    RenderImage(IMAGE_MIXINVENTORY_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_MIXINVENTORY_RIGHT, m_Pos.x + INVENTORY_WIDTH - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_MIXINVENTORY_BOTTOM, m_Pos.x, m_Pos.y + INVENTORY_HEIGHT - 45, 190.f, 45.f);

    wchar_t szText[256] = { 0, };
    float fPos_x = m_Pos.x + 15.0f, fPos_y = m_Pos.y;
    float fLine_y = 13.0f;

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(220, 220, 220, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    switch (g_MixRecipeMgr.GetMixInventoryType())
    {
    case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
        swprintf(szText, L"%s", GlobalText[735]);
        break;
    case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
        swprintf(szText, L"%s", GlobalText[736]);
        break;
    case SEASON3A::MIXTYPE_GOBLIN_ADD380:
        swprintf(szText, L"%s", GlobalText[2193]);
        break;
    case SEASON3A::MIXTYPE_CASTLE_SENIOR:
        fLine_y += 5.0f;
        swprintf(szText, L"%s", GlobalText[1640]);
        break;
    case SEASON3A::MIXTYPE_TRAINER:
        swprintf(szText, L"%s", GlobalText[1205]);
        break;
    case SEASON3A::MIXTYPE_OSBOURNE:
        fLine_y += 5.0f;
        swprintf(szText, L"%s", GlobalText[2061]);
        break;
    case SEASON3A::MIXTYPE_JERRIDON:
        fLine_y += 5.0f;
        swprintf(szText, L"%s", GlobalText[2062]);
        break;
    case SEASON3A::MIXTYPE_ELPIS:
        fLine_y += 5.0f;
        swprintf(szText, L"%s", GlobalText[2063]);
        break;
    case SEASON3A::MIXTYPE_CHAOS_CARD:
        fLine_y += 5.0f;
        swprintf(szText, L"%s", GlobalText[2265]);
        break;
    case SEASON3A::MIXTYPE_CHERRYBLOSSOM:
        fLine_y += 5.0f;
        swprintf(szText, L"%s", GlobalText[2563]);
        break;
    case SEASON3A::MIXTYPE_EXTRACT_SEED:
        fLine_y += 5.0f;
        swprintf(szText, L"%s", GlobalText[2660]);
        break;
    case SEASON3A::MIXTYPE_SEED_SPHERE:
        fLine_y += 5.0f;
        swprintf(szText, L"%s", GlobalText[2661]);
        break;
    case SEASON3A::MIXTYPE_ATTACH_SOCKET:
        fLine_y += 5.0f;
        swprintf(szText, L"%s", GlobalText[2662]);
        break;
    case SEASON3A::MIXTYPE_DETACH_SOCKET:
        fLine_y += 5.0f;
        swprintf(szText, L"%s", GlobalText[2663]);
        break;
    default:
        fLine_y += 5.0f;
        swprintf(szText, L"%s", GlobalText[583]);
        break;
    }
    g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText, 160.0f, 0, RT3_SORT_CENTER);

    fLine_y += 12;
    switch (g_MixRecipeMgr.GetMixInventoryType())
    {
    case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
    case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
    case SEASON3A::MIXTYPE_GOBLIN_ADD380:
    case SEASON3A::MIXTYPE_TRAINER:
        swprintf(szText, GlobalText[1623], g_nChaosTaxRate);
        g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText, 160.0f, 0, RT3_SORT_CENTER);
        break;
    default:
        fLine_y -= 5;
        break;
    }

    if (GetMixState() == MIX_FINISHED)
    {
        return;
    }

    fLine_y += 24;
    if (!g_MixRecipeMgr.IsReadyToMix())
        g_pRenderText->SetTextColor(255, 48, 48, 255);
    else
        g_pRenderText->SetTextColor(255, 255, 48, 255);
    g_pRenderText->SetBgColor(40, 40, 40, 128);

    g_MixRecipeMgr.GetCurRecipeName(szText, 1);
    g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText);
    fLine_y += 10;
    if (g_MixRecipeMgr.GetCurRecipeName(szText, 2))
        g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText);
    fLine_y += 10;

    switch (g_MixRecipeMgr.GetMixInventoryType())
    {
    case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
    case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
    case SEASON3A::MIXTYPE_GOBLIN_ADD380:
    case SEASON3A::MIXTYPE_OSBOURNE:
    case SEASON3A::MIXTYPE_ELPIS:
    case SEASON3A::MIXTYPE_TRAINER:
    case SEASON3A::MIXTYPE_EXTRACT_SEED:
    case SEASON3A::MIXTYPE_SEED_SPHERE:
        if (g_MixRecipeMgr.IsReadyToMix() &&
            g_MixRecipeMgr.GetPlusChaosRate() > 0 && g_MixRecipeMgr.GetCurRecipe()->m_bMixOption == 'F')
        {
            g_pRenderText->SetTextColor(255, 255, 48, 255);
            g_pRenderText->SetBgColor(40, 40, 40, 128);
            swprintf(szText, GlobalText[584], GlobalText[591], g_MixRecipeMgr.GetSuccessRate());
            swprintf(szText, L"%s + %d%%", szText, g_MixRecipeMgr.GetPlusChaosRate());
            g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText);
            g_pRenderText->SetTextColor(210, 230, 255, 255);
        }
        else
        {
            g_pRenderText->SetTextColor(210, 230, 255, 255);
            g_pRenderText->SetBgColor(40, 40, 40, 128);
            switch (g_MixRecipeMgr.GetMixInventoryType())
            {
            case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
            case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
            case SEASON3A::MIXTYPE_GOBLIN_ADD380:
            case SEASON3A::MIXTYPE_EXTRACT_SEED:
            case SEASON3A::MIXTYPE_SEED_SPHERE:
                swprintf(szText, GlobalText[584], GlobalText[591], g_MixRecipeMgr.GetSuccessRate());
                break;
            case SEASON3A::MIXTYPE_TRAINER:
                swprintf(szText, GlobalText[584], GlobalText[1212], g_MixRecipeMgr.GetSuccessRate());
                break;
            case SEASON3A::MIXTYPE_OSBOURNE:
                swprintf(szText, GlobalText[584], GlobalText[2061], g_MixRecipeMgr.GetSuccessRate());
                break;
            case SEASON3A::MIXTYPE_ELPIS:
                swprintf(szText, GlobalText[584], GlobalText[2063], g_MixRecipeMgr.GetSuccessRate());
                break;
            }
            g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText);
        }
        fLine_y += 20;
        break;
    }

    switch (g_MixRecipeMgr.GetMixInventoryType())
    {
    case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
    case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
    case SEASON3A::MIXTYPE_GOBLIN_ADD380:
    case SEASON3A::MIXTYPE_CASTLE_SENIOR:
    case SEASON3A::MIXTYPE_TRAINER:
    case SEASON3A::MIXTYPE_JERRIDON:
    case SEASON3A::MIXTYPE_EXTRACT_SEED:
    case SEASON3A::MIXTYPE_SEED_SPHERE:
    case SEASON3A::MIXTYPE_ATTACH_SOCKET:
    case SEASON3A::MIXTYPE_DETACH_SOCKET:
    {
        wchar_t szGoldText[32];
        wchar_t szGoldText2[32];

        ConvertGold(g_MixRecipeMgr.GetReqiredZen(), szGoldText);
        ConvertChaosTaxGold(g_MixRecipeMgr.GetReqiredZen(), szGoldText2);
        if (g_MixRecipeMgr.IsReadyToMix() && g_MixRecipeMgr.GetCurRecipe()->m_bRequiredZenType == 'C')
        {
            swprintf(szText, GlobalText[1636], szGoldText2, szGoldText);
        }
        else
        {
            swprintf(szText, GlobalText[1622], szGoldText2, szGoldText);
        }

        g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText);
    }
    fLine_y += 20;
    break;
    }

    fLine_y = 203;
    int iTextPos_y = 0;
    if (g_MixRecipeMgr.GetMostSimilarRecipe() != NULL)
    {
        g_pRenderText->SetTextColor(220, 220, 220, 255);
        g_pRenderText->SetBgColor(40, 40, 40, 128);

        wchar_t szTempText[2][100] = { 0 };
        int iTextLines = 0;
        if (!g_MixRecipeMgr.IsReadyToMix() && g_MixRecipeMgr.GetMostSimilarRecipeName(szTempText[0], 1) == TRUE)
        {
            swprintf(szText, GlobalText[2334], szTempText[0]);
            iTextLines = CutStr(szText, szTempText[0], 150, 2, 100);

            for (int i = 0; i < iTextLines; i++)
            {
                if (i >= 2)
                    break;

                g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y + (iTextPos_y * 15), szTempText[i]);
                iTextPos_y++;
            }
        }
        int iResult;
        for (int iLine = 0; iLine < 8; ++iLine)
        {
            iResult = g_MixRecipeMgr.GetSourceName(iLine, szText);
            if (iResult == SEASON3A::MIX_SOURCE_ERROR) break;
            else if (iResult == SEASON3A::MIX_SOURCE_NO) g_pRenderText->SetTextColor(255, 50, 20, 255);
            else if (iResult == SEASON3A::MIX_SOURCE_PARTIALLY) g_pRenderText->SetTextColor(210, 230, 255, 255);
            else if (iResult == SEASON3A::MIX_SOURCE_YES) g_pRenderText->SetTextColor(255, 255, 48, 255);

            iTextLines = CutStr(szText, szTempText[0], 156, 2, 100);

            for (int i = 0; i < iTextLines; i++)
            {
                if (i >= 2)
                    break;

                g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y + (iTextPos_y * 15), szTempText[i]);
                iTextPos_y++;
            }
        }
    }
    else if (g_MixRecipeMgr.IsMixInit())
    {
        g_pRenderText->SetTextColor(255, 50, 20, 255);
        g_pRenderText->SetBgColor(40, 40, 40, 128);

        swprintf(szText, GlobalText[2346]);
        g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText);
        iTextPos_y++;
    }
    else
    {
        g_pRenderText->SetTextColor(255, 50, 20, 255);
        g_pRenderText->SetBgColor(40, 40, 40, 128);

        swprintf(szText, GlobalText[2334], L" ");
        g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText);

        swprintf(szText, GlobalText[601]);
        g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y + (++iTextPos_y) * 15, szText);
    }

    ++iTextPos_y;
    g_pRenderText->SetTextColor(255, 50, 20, 255);
    g_pRenderText->SetBgColor(40, 40, 40, 128);

    if (g_MixRecipeMgr.IsReadyToMix())
    {
        if (g_MixRecipeMgr.GetCurRecipeDesc(szText, 1) == TRUE)
            g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y + (++iTextPos_y) * 15, szText);
        if (g_MixRecipeMgr.GetCurRecipeDesc(szText, 2) == TRUE)
            g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y + (++iTextPos_y) * 15, szText);
        if (g_MixRecipeMgr.GetCurRecipeDesc(szText, 3) == TRUE)
            g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y + (++iTextPos_y) * 15, szText);
    }
    else if (g_MixRecipeMgr.GetMostSimilarRecipe() != NULL)
    {
        if (g_MixRecipeMgr.GetRecipeAdvice(szText, 1) == TRUE)
            g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y + (++iTextPos_y) * 15, szText);
        if (g_MixRecipeMgr.GetRecipeAdvice(szText, 2) == TRUE)
            g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y + (++iTextPos_y) * 15, szText);
        if (g_MixRecipeMgr.GetRecipeAdvice(szText, 3) == TRUE)
            g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y + (++iTextPos_y) * 15, szText);
    }

    RenderMixDescriptions(fPos_x, fPos_y);

    switch (g_MixRecipeMgr.GetMixInventoryType())
    {
    case SEASON3A::MIXTYPE_TRAINER:
        m_BtnMix.ChangeToolTipText(GlobalText[1212], true);
        break;
    case SEASON3A::MIXTYPE_OSBOURNE:
        m_BtnMix.ChangeToolTipText(GlobalText[2061], true);
        break;
    case SEASON3A::MIXTYPE_JERRIDON:
        m_BtnMix.ChangeToolTipText(GlobalText[2062], true);
        break;
    case SEASON3A::MIXTYPE_ELPIS:
        m_BtnMix.ChangeToolTipText(GlobalText[2063], true);
        break;
    case SEASON3A::MIXTYPE_EXTRACT_SEED:
        m_BtnMix.ChangeToolTipText(GlobalText[2660], true);
        break;
    case SEASON3A::MIXTYPE_SEED_SPHERE:
        m_BtnMix.ChangeToolTipText(GlobalText[2661], true);
        break;
    case SEASON3A::MIXTYPE_ATTACH_SOCKET:
        m_BtnMix.ChangeToolTipText(GlobalText[2662], true);
        break;
    case SEASON3A::MIXTYPE_DETACH_SOCKET:
        m_BtnMix.ChangeToolTipText(GlobalText[2663], true);
        break;
    default:
        m_BtnMix.ChangeToolTipText(GlobalText[591], true);
        break;
    }
    m_BtnMix.Render();
}

bool CNewUIMixInventory::BtnProcess()
{
    POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_MIXINVENTORY);
    }

    if (GetMixState() == MIX_FINISHED)
    {
        return false;
    }

    if (m_BtnMix.UpdateMouseEvent() == true)
    {
        Mix();
        return true;
    }

    return false;
}

void CNewUIMixInventory::RenderMixDescriptions(float fPos_x, float fPos_y)
{
    wchar_t szText[256] = { 0, };
    switch (g_MixRecipeMgr.GetMixInventoryType())
    {
    case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
    case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
    case SEASON3A::MIXTYPE_GOBLIN_ADD380:
        break;
    case SEASON3A::MIXTYPE_CASTLE_SENIOR:
    {
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        g_pRenderText->SetTextColor(200, 200, 200, 255);
        for (int i = 0; i < 6; ++i)
            g_pRenderText->RenderText(fPos_x, fPos_y + 270 + i * 13, GlobalText[1644 + i], 160.0f, 0, RT3_SORT_CENTER);
    }
    break;
    case SEASON3A::MIXTYPE_TRAINER:
        break;
    case SEASON3A::MIXTYPE_OSBOURNE:
    {
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        swprintf(szText, GlobalText[2220]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 0 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
        swprintf(szText, GlobalText[2221]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 1 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
        swprintf(szText, GlobalText[2084], GlobalText[2061], GlobalText[2082]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 2 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
        swprintf(szText, GlobalText[2098]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 3 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
        g_pRenderText->SetTextColor(255, 0, 0, 255);
        swprintf(szText, GlobalText[2222]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 4 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
    }
    break;
    case SEASON3A::MIXTYPE_JERRIDON:
    {
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        swprintf(szText, GlobalText[2102]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 0 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
        swprintf(szText, GlobalText[2103]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 1 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
        swprintf(szText, GlobalText[2104]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 2 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
        swprintf(szText, GlobalText[2088]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 3 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
        swprintf(szText, GlobalText[2100]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 4 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
        swprintf(szText, GlobalText[2101]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 5 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
    }
    break;
    case SEASON3A::MIXTYPE_ELPIS:
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        swprintf(szText, GlobalText[2071]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 0 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
        swprintf(szText, GlobalText[2095]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 1 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
        swprintf(szText, GlobalText[2096]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 2 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
        break;
    case SEASON3A::MIXTYPE_CHAOS_CARD:
    {
        g_pRenderText->SetBgColor(0, 0, 0, 0);

        g_pRenderText->SetTextColor(255, 40, 20, 255);
        swprintf(szText, GlobalText[2223]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 4 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);

        g_pRenderText->SetTextColor(255, 255, 255, 255);
        swprintf(szText, GlobalText[2262]);
        g_pRenderText->RenderText(fPos_x - 10, fPos_y + 250 + 6 * 13, szText, 200.0f, 0, RT3_SORT_LEFT);

        g_pRenderText->SetTextColor(255, 255, 255, 255);
        swprintf(szText, GlobalText[2306]);
        g_pRenderText->RenderText(fPos_x - 10, fPos_y + 250 + 7 * 13, szText, 200.0f, 0, RT3_SORT_LEFT);

        g_pRenderText->SetTextColor(255, 255, 255, 255);
        swprintf(szText, GlobalText[2261]);
        g_pRenderText->RenderText(fPos_x - 10, fPos_y + 250 + 8 * 13, szText, 200.0f, 0, RT3_SORT_LEFT);
    }
    break;
    case SEASON3A::MIXTYPE_CHERRYBLOSSOM:
    {
        g_pRenderText->SetBgColor(0, 0, 0, 0);

        g_pRenderText->SetTextColor(255, 40, 20, 255);
        swprintf(szText, GlobalText[2223]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 250 + 0 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);

        g_pRenderText->SetTextColor(255, 255, 255, 255);
        swprintf(szText, GlobalText[2565]);
        g_pRenderText->RenderText(fPos_x - 10, fPos_y + 250 + 2 * 13, szText, 160.0f, 0, RT3_SORT_LEFT);

        g_pRenderText->SetTextColor(255, 255, 255, 255);
        swprintf(szText, GlobalText[2540]);
        g_pRenderText->RenderText(fPos_x - 10, fPos_y + 250 + 3 * 13, szText, 180.0f, 0, RT3_SORT_LEFT);

        g_pRenderText->SetTextColor(255, 255, 255, 255);
        swprintf(szText, GlobalText[2306]);
        g_pRenderText->RenderText(fPos_x - 10, fPos_y + 250 + 4 * 13, szText, 200.0f, 0, RT3_SORT_LEFT);
    }
    break;
    case SEASON3A::MIXTYPE_ATTACH_SOCKET:
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        g_pRenderText->SetTextColor(200, 200, 200, 255);
        swprintf(szText, GlobalText[2674]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 280 + 0 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
        m_SocketListBox.Render();
        break;
    case SEASON3A::MIXTYPE_DETACH_SOCKET:
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        g_pRenderText->SetTextColor(200, 200, 200, 255);
        swprintf(szText, GlobalText[2675]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 280 + 0 * 13, szText, 160.0f, 0, RT3_SORT_CENTER);
        m_SocketListBox.Render();
        break;
    default:
        break;
    }
}

int CNewUIMixInventory::Rtn_MixRequireZen(int _nMixZen, int _nTax)
{
    if (_nTax)		_nMixZen += ((LONGLONG)_nMixZen * g_nChaosTaxRate) / 100;
    return _nMixZen;
}

bool CNewUIMixInventory::Mix()
{
    PlayBuffer(SOUND_CLICK01);

    DWORD dwGold = CharacterMachine->Gold;
    int	  nMixZen = g_MixRecipeMgr.GetReqiredZen();

    nMixZen = Rtn_MixRequireZen(nMixZen, g_nChaosTaxRate);

    if (nMixZen > (int)dwGold)
    {
        g_pSystemLogBox->AddText(GlobalText[596], SEASON3B::TYPE_ERROR_MESSAGE);
        return false;
    }

    if (!g_MixRecipeMgr.IsReadyToMix())
    {
        wchar_t szText[100];
        swprintf(szText, GlobalText[580], GlobalText[591]);
        g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_ERROR_MESSAGE);
        return false;
    }

    int iLevel = CharacterAttribute->Level;
    if (iLevel < g_MixRecipeMgr.GetCurRecipe()->m_iRequiredLevel)
    {
        wchar_t szText[100];
        wchar_t szText2[100];
        g_MixRecipeMgr.GetCurRecipeName(szText2, 1);
        swprintf(szText, GlobalText[2347], g_MixRecipeMgr.GetCurRecipe()->m_iRequiredLevel, szText2);
        g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_ERROR_MESSAGE);
        return false;
    }

    if (g_MixRecipeMgr.GetCurRecipe()->m_iWidth != -1 &&
        g_pMyInventory->FindEmptySlot(g_MixRecipeMgr.GetCurRecipe()->m_iWidth, g_MixRecipeMgr.GetCurRecipe()->m_iHeight) == -1)
    {
        g_pSystemLogBox->AddText(GlobalText[581], SEASON3B::TYPE_ERROR_MESSAGE);
        return false;
    }

    if (g_MixRecipeMgr.GetMixInventoryType() == SEASON3A::MIXTYPE_ATTACH_SOCKET)
    {
        int iSelectedLine = m_SocketListBox.GetLineNum() - m_SocketListBox.SLGetSelectLineNum();

        for (int i = 0; i < m_SocketListBox.GetLineNum(); ++i)
        {
            BYTE bySocketSeedID = g_MixRecipeMgr.GetFirstItemSocketSeedID(i);
            if (bySocketSeedID != SOCKET_EMPTY)
            {
                BYTE bySeedSphereID = g_MixRecipeMgr.GetSeedSphereID(0);
                if (bySocketSeedID == bySeedSphereID)
                {
                    g_pSystemLogBox->AddText(GlobalText[2683], SEASON3B::TYPE_ERROR_MESSAGE);
                    return false;
                }
            }
        }

        if (m_SocketListBox.SLGetSelectLineNum() == 0)
        {
            g_pSystemLogBox->AddText(GlobalText[2676], SEASON3B::TYPE_ERROR_MESSAGE);
            return false;
        }
        else if (iSelectedLine > g_MixRecipeMgr.GetFirstItemSocketCount()
            || g_MixRecipeMgr.GetFirstItemSocketSeedID(iSelectedLine) != SOCKET_EMPTY)
        {
            g_pSystemLogBox->AddText(GlobalText[2677], SEASON3B::TYPE_ERROR_MESSAGE);
            return false;
        }

        g_MixRecipeMgr.SetMixSubType(iSelectedLine);
    }
    else if (g_MixRecipeMgr.GetMixInventoryType() == SEASON3A::MIXTYPE_DETACH_SOCKET)
    {
        int iSelectedLine = m_SocketListBox.GetLineNum() - m_SocketListBox.SLGetSelectLineNum();
        if (m_SocketListBox.SLGetSelectLineNum() == 0)
        {
            g_pSystemLogBox->AddText(GlobalText[2678], SEASON3B::TYPE_ERROR_MESSAGE);
            return false;
        }
        else if (iSelectedLine > g_MixRecipeMgr.GetFirstItemSocketCount()
            || g_MixRecipeMgr.GetFirstItemSocketSeedID(iSelectedLine) == SOCKET_EMPTY)
        {
            g_pSystemLogBox->AddText(GlobalText[2679], SEASON3B::TYPE_ERROR_MESSAGE);
            return false;
        }
        g_MixRecipeMgr.SetMixSubType(iSelectedLine);
    }

#ifdef LJH_MOD_CANNOT_USE_CHARMITEM_AND_CHAOSCHARMITEM_SIMULTANEOUSLY
    if (g_MixRecipeMgr.GetTotalChaosCharmCount() > 0 && g_MixRecipeMgr.GetTotalCharmCount() > 0)
    {
        g_pSystemLogBox->AddText(GlobalText[3286], SEASON3B::TYPE_ERROR_MESSAGE);
        return FALSE;
    }
#endif //LJH_MOD_CANNOT_USE_CHARMITEM_AND_CHAOSCHARMITEM_SIMULTANEOUSLY

    if (CNewUIInventoryCtrl::GetPickedItem() == NULL)
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CMixCheckMsgBoxLayout));
        return true;
    }

    return false;
}

bool CNewUIMixInventory::InventoryProcess()
{
    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

    if (m_pNewInventoryCtrl && pPickedItem)
    {
        const auto iCurInventory = g_MixRecipeMgr.GetMixInventoryEquipmentIndex();

        ITEM* pItemObj = pPickedItem->GetItem();
        if (GetMixState() == MIX_READY && g_MixRecipeMgr.IsMixSource(pPickedItem->GetItem()) &&
            pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
        {
            m_pNewInventoryCtrl->SetSquareColorNormal(m_fInventoryColor[0], m_fInventoryColor[1], m_fInventoryColor[2]);
            if (SEASON3B::IsPress(VK_LBUTTON))
            {
                int iSourceIndex = pPickedItem->GetSourceLinealPos();
                int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
                if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
                {
                    if (SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSourceIndex,
                        pItemObj, iCurInventory, iTargetIndex))
                    {
                        return true;
                    }
                }
            }
        }
        else if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
        {
            m_pNewInventoryCtrl->SetSquareColorNormal(m_fInventoryColor[0], m_fInventoryColor[1], m_fInventoryColor[2]);
            if (SEASON3B::IsPress(VK_LBUTTON))
            {
                int iSourceIndex = pPickedItem->GetSourceLinealPos();
                int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
                if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
                {
                    if (SendRequestEquipmentItem(iCurInventory, iSourceIndex,
                        pItemObj, iCurInventory, iTargetIndex))
                    {
                        return true;
                    }
                }
            }
        }
        else if (GetMixState() == MIX_READY && g_MixRecipeMgr.IsMixSource(pPickedItem->GetItem()) &&
            pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
        {
            m_pNewInventoryCtrl->SetSquareColorNormal(m_fInventoryColor[0], m_fInventoryColor[1], m_fInventoryColor[2]);
            if (SEASON3B::IsPress(VK_LBUTTON))
            {
                int iSourceIndex = pPickedItem->GetSourceLinealPos();
                int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
                if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
                {
                    SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSourceIndex,
                        pItemObj, iCurInventory, iTargetIndex);
                    return true;
                }
            }
        }
        else
        {
            m_pNewInventoryCtrl->SetSquareColorNormal(m_fInventoryWarningColor[0], m_fInventoryWarningColor[1], m_fInventoryWarningColor[2]);
        }
    }
    return false;
}

void CNewUIMixInventory::CheckMixInventory()
{
    g_MixRecipeMgr.ResetMixItemInventory();
    ITEM* pItem = NULL;
    for (int i = 0; i < (int)m_pNewInventoryCtrl->GetNumberOfItems(); ++i)
    {
        pItem = m_pNewInventoryCtrl->GetItem(i);
        g_MixRecipeMgr.AddItemToMixItemInventory(pItem);
    }
    g_MixRecipeMgr.CheckMixInventory();
}

void CNewUIMixInventory::RenderMixEffect()
{
    if (m_iMixEffectTimer <= 0)
    {
        return;
    }
    else
    {
        --m_iMixEffectTimer;
    }
    EnableAlphaBlend();

    for (int i = 0; i < (int)m_pNewInventoryCtrl->GetNumberOfItems(); ++i)
    {
        int iWidth = ItemAttribute[m_pNewInventoryCtrl->GetItem(i)->Type].Width;
        int iHeight = ItemAttribute[m_pNewInventoryCtrl->GetItem(i)->Type].Height;

        for (int h = 0; h < iHeight; ++h)
        {
            for (int w = 0; w < iWidth; ++w)
            {
                glColor3f((float)(rand() % 6 + 6) * 0.1f, (float)(rand() % 4 + 4) * 0.1f, 0.2f);
                float Rotate = (float)((int)(WorldTime) % 100) * 20.f;
                float Scale = 5.f + (rand() % 10);
                float x = m_pNewInventoryCtrl->GetPos().x +
                    (m_pNewInventoryCtrl->GetItem(i)->x + w) * INVENTORY_SQUARE_WIDTH +
                    (rand() % INVENTORY_SQUARE_WIDTH);
                float y = m_pNewInventoryCtrl->GetPos().y +
                    (m_pNewInventoryCtrl->GetItem(i)->y + h) * INVENTORY_SQUARE_WIDTH +
                    (rand() % INVENTORY_SQUARE_WIDTH);
                RenderBitmapRotate(BITMAP_SHINY, x, y, Scale, Scale, 0);
                RenderBitmapRotate(BITMAP_SHINY, x, y, Scale, Scale, Rotate);
                RenderBitmapRotate(BITMAP_SHINY + 1, x, y, Scale * 3.f, Scale * 3.f, Rotate);
                RenderBitmapRotate(BITMAP_LIGHT, x, y, Scale * 6.f, Scale * 6.f, 0);
            }
        }
    }
    DisableAlphaBlend();
}

int SEASON3B::CNewUIMixInventory::GetPointedItemIndex()
{
    return m_pNewInventoryCtrl->GetPointedSquareIndex();
}