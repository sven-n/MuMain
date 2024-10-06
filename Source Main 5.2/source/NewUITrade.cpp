//*****************************************************************************
// Desc: implementation of the CNewUITrade class.
//*****************************************************************************

#include "stdafx.h"

#include "NewUITrade.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"

#include "CComGem.h"
#include "DSPlaySound.h"

using namespace SEASON3B;

CNewUITrade::CNewUITrade()
{
    m_pNewUIMng = NULL;
    m_pYourInvenCtrl = m_pMyInvenCtrl = NULL;
    m_Pos.x = m_Pos.y = 0;
}

CNewUITrade::~CNewUITrade()
{
    Release();
}

bool CNewUITrade::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewUI3DRenderMng
        || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_TRADE, this);

    m_pYourInvenCtrl = new CNewUIInventoryCtrl;
    if (false == m_pYourInvenCtrl->Create(STORAGE_TYPE::UNDEFINED, g_pNewUI3DRenderMng, g_pNewItemMng,
        this, x + 16, y + 68, COLUMN_TRADE_INVEN, ROW_TRADE_INVEN))
    {
        SAFE_DELETE(m_pYourInvenCtrl);
        return false;
    }

    m_pMyInvenCtrl = new CNewUIInventoryCtrl;
    if (false == m_pMyInvenCtrl->Create(STORAGE_TYPE::TRADE, g_pNewUI3DRenderMng, g_pNewItemMng,
        this, x + 16, y + 274, COLUMN_TRADE_INVEN, ROW_TRADE_INVEN))
    {
        SAFE_DELETE(m_pMyInvenCtrl);
        return false;
    }

    SetPos(x, y);

    LoadImages();

    m_abtn[BTN_CLOSE].ChangeButtonImgState(true, IMAGE_TRADE_BTN_CLOSE);
    m_abtn[BTN_CLOSE].ChangeButtonInfo(x + 13, y + 390, 36, 29);
    m_abtn[BTN_CLOSE].ChangeToolTipText(GlobalText[1002], true);

    m_abtn[BTN_ZEN_INPUT].ChangeButtonImgState(true, IMAGE_TRADE_BTN_ZEN_INPUT);
    m_abtn[BTN_ZEN_INPUT].ChangeButtonInfo(x + 104, y + 390, 36, 29);
    m_abtn[BTN_ZEN_INPUT].ChangeToolTipText(GlobalText[227], true);

    ::memset(m_szYourID, 0, MAX_ID_SIZE + 1);
    m_bTradeAlert = false;

    InitTradeInfo();
    InitYourInvenBackUp();

    Show(false);

    return true;
}

void CNewUITrade::InitTradeInfo()
{
    m_nYourLevel = 0;
    m_nYourGuildType = -1;
    m_nYourTradeGold = 0;
    m_nMyTradeGold = 0;
    m_nMyTradeWait = 0;
    m_bYourConfirm = m_bMyConfirm = false;
}

void CNewUITrade::InitYourInvenBackUp()
{
    for (int i = 0; i < MAX_TRADE_INVEN; ++i)
        m_aYourInvenBackUp[i].Type = -1;
}

void CNewUITrade::Release()
{
    UnloadImages();

    SAFE_DELETE(m_pMyInvenCtrl);
    SAFE_DELETE(m_pYourInvenCtrl);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);
}

void CNewUITrade::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    m_posMyConfirm.x = m_Pos.x + 144;
    m_posMyConfirm.y = m_Pos.y + 390;
}

bool CNewUITrade::UpdateMouseEvent()
{
    if ((m_pYourInvenCtrl && false == m_pYourInvenCtrl->UpdateMouseEvent())
        || (m_pMyInvenCtrl && false == m_pMyInvenCtrl->UpdateMouseEvent()))
    {
        if (SEASON3B::IsPress(VK_LBUTTON)
            && CNewUIInventoryCtrl::GetPickedItem()->GetOwnerInventory() == m_pMyInvenCtrl
            && m_bMyConfirm)
        {
            m_bMyConfirm = false;
            SocketClient->ToGameServer()->SendTradeButtonStateChange(m_bMyConfirm);
        }

        return false;
    }

    ProcessMyInvenCtrl();

    if (ProcessBtns())
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, TRADE_WIDTH, TRADE_HEIGHT))
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

bool CNewUITrade::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            SocketClient->ToGameServer()->SendTradeCancel();
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_TRADE);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }
    return true;
}

bool CNewUITrade::Update()
{
    if ((m_pYourInvenCtrl && false == m_pYourInvenCtrl->Update())
        || (m_pMyInvenCtrl && false == m_pMyInvenCtrl->Update()))
        return false;

    return true;
}

bool CNewUITrade::Render()
{
    ::EnableAlphaTest();

    ::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderBackImage();
    RenderText();

    if (m_pYourInvenCtrl)
        m_pYourInvenCtrl->Render();
    if (m_pMyInvenCtrl)
        m_pMyInvenCtrl->Render();

    for (int i = BTN_CLOSE; i < MAX_BTN; ++i)
        m_abtn[i].Render();

    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER,
            UI2DEffectCallback, this, 0, 0);

    ::DisableAlphaBlend();

    return true;
}

void CNewUITrade::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    if (pClass)
    {
        auto* pNewUITrade = (CNewUITrade*)pClass;
        pNewUITrade->RenderWarningArrow();
    }
}

void CNewUITrade::RenderBackImage()
{
    RenderImage(IMAGE_TRADE_BACK,
        m_Pos.x, m_Pos.y, float(TRADE_WIDTH), float(TRADE_HEIGHT));
    RenderImage(IMAGE_TRADE_TOP,
        m_Pos.x, m_Pos.y, float(TRADE_WIDTH), 64.f);
    RenderImage(IMAGE_TRADE_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_TRADE_RIGHT,
        m_Pos.x + TRADE_WIDTH - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_TRADE_BOTTOM,
        m_Pos.x, m_Pos.y + TRADE_HEIGHT - 45, float(TRADE_WIDTH), 45.f);

    RenderImage(IMAGE_TRADE_LINE, m_Pos.x + 1, m_Pos.y + 220, 188.f, 21.f);
    RenderImage(IMAGE_TRADE_NICK_BACK, m_Pos.x + 11, m_Pos.y + 37, 171.f, 26.f);
    RenderImage(IMAGE_TRADE_MONEY, m_Pos.x + 11, m_Pos.y + 150, 170.f, 26.f);
    RenderImage(IMAGE_TRADE_NICK_BACK, m_Pos.x + 11, m_Pos.y + 243, 171.f, 26.f);
    RenderImage(IMAGE_TRADE_MONEY, m_Pos.x + 11, m_Pos.y + 356, 170.f, 26.f);

    float fSrcY = m_bYourConfirm ? 29.f : 0.f;
    RenderImage(IMAGE_TRADE_CONFIRM, m_Pos.x + 146, m_Pos.y + 186, CONFIRM_WIDTH, CONFIRM_HEIGHT, 0.f, fSrcY);

    DWORD dwColor = m_nMyTradeWait > 0
        ? RGBA(255, 0, 0, 255) : RGBA(255, 255, 255, 255);
    fSrcY = m_bMyConfirm ? 29.f : 0.f;
    RenderImage(IMAGE_TRADE_CONFIRM, m_Pos.x + 144, m_Pos.y + 390, CONFIRM_WIDTH, CONFIRM_HEIGHT, 0.f, fSrcY, dwColor);
}

void CNewUITrade::RenderText()
{
    wchar_t szTemp[128];

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0);

    g_pRenderText->SetTextColor(216, 216, 216, 255);
    g_pRenderText->RenderText(
        m_Pos.x, m_Pos.y + 11, GlobalText[226], TRADE_WIDTH, 0, RT3_SORT_CENTER);

    for (int i = 0; i < MAX_MARKS; ++i)
    {
        if (GuildMark[i].Key != -1 && GuildMark[i].Key == m_nYourGuildType)
        {
            ::CreateGuildMark(i, false);
            ::RenderBitmap(BITMAP_GUILD, (float)m_Pos.x + 15, (float)m_Pos.y + 42, 16, 16);
            g_pRenderText->RenderText(m_Pos.x + 16, m_Pos.y + 30, GuildMark[i].GuildName);
            break;
        }
    }

    g_pRenderText->SetFont(g_hFontBig);

    g_pRenderText->SetTextColor(210, 230, 255, 255);
    g_pRenderText->RenderText(m_Pos.x + 32, m_Pos.y + 43, m_szYourID);

    g_pRenderText->SetFont(g_hFontBold);

    int nLevel;
    DWORD dwColor;
    ConvertYourLevel(nLevel, dwColor);
    if (nLevel == 400)
    {
        swprintf(szTemp, L"%d", nLevel);
    }
    else
    {
        swprintf(szTemp, GlobalText[369], nLevel);
    }
    g_pRenderText->SetTextColor(dwColor);
    g_pRenderText->RenderText(m_Pos.x + 134, m_Pos.y + 48, L"Lv.");
    g_pRenderText->RenderText(m_Pos.x + 148, m_Pos.y + 48, szTemp);

    ::ConvertGold(m_nYourTradeGold, szTemp);
    g_pRenderText->SetTextColor(::getGoldColor(m_nYourTradeGold));
    g_pRenderText->RenderText(
        m_Pos.x + 170, m_Pos.y + 150 + 8, szTemp, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

    ::ConvertGold(m_nMyTradeGold, szTemp);
    g_pRenderText->SetTextColor(::getGoldColor(m_nMyTradeGold));
    g_pRenderText->RenderText(
        m_Pos.x + 170, m_Pos.y + 356 + 8, szTemp, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

    g_pRenderText->SetTextColor(210, 230, 255, 255);
    g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + 253, Hero->ID);

    int nAlpha = int(min(255, sin(WorldTime / 200) * 200 + 275));
    g_pRenderText->SetTextColor(210, 0, 0, nAlpha);
    g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + 185, GlobalText[370]);
    g_pRenderText->SetTextColor(255, 220, 150, 255);
    g_pRenderText->RenderText(m_Pos.x + 45, m_Pos.y + 185, GlobalText[365]);
    g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + 200, GlobalText[366]);
    g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + 215, GlobalText[367]);
}

void CNewUITrade::RenderWarningArrow()
{
    ::EnableAlphaTest();

    int nYourItems = m_pYourInvenCtrl->GetNumberOfItems();
    ITEM* pYourItemObj;
    int nWidth;
    float fX, fY;
    POINT ptYourInvenCtrl = m_pYourInvenCtrl->GetPos();

    for (int i = 0; i < nYourItems; ++i)
    {
        pYourItemObj = m_pYourInvenCtrl->GetItem(i);
        if (ITEM_COLOR_TRADE_WARNING == pYourItemObj->byColorState)
        {
            fX = (float)ptYourInvenCtrl.x
                + (pYourItemObj->x * INVENTORY_SQUARE_WIDTH);
            fY = (float)ptYourInvenCtrl.y
                + (pYourItemObj->y * INVENTORY_SQUARE_WIDTH)
                + sinf(WorldTime * 0.015f);

            ::glColor3f(0.f, 1.f, 1.f);
            ::RenderBitmap(IMAGE_TRADE_WARNING_ARROW, fX, fY + 5, 24.f, 24.f,
                0.f, 0.4f);
            ::glColor3f(1.f, 1.f, 1.f);

            g_pRenderText->SetFont(g_hFontBold);
            g_pRenderText->SetTextColor(255, 255, 255, 255);
            g_pRenderText->SetBgColor(210, 0, 0, 255);
            nWidth = (int)ItemAttribute[pYourItemObj->Type].Width
                * INVENTORY_SQUARE_WIDTH;
            g_pRenderText->RenderText((int)fX, (int)fY, GlobalText[370],
                nWidth, 0, RT3_SORT_CENTER);
        }
    }

    ::DisableAlphaBlend();
}

void CNewUITrade::ConvertYourLevel(int& rnLevel, DWORD& rdwColor)
{
    if (m_nYourLevel >= 400)
    {
        rnLevel = 400;
        rdwColor = (255 << 24) + (153 << 16) + (153 << 8) + (255);
    }
    else if (m_nYourLevel >= 300)
    {
        rnLevel = 300;
        rdwColor = (255 << 24) + (255 << 16) + (153 << 8) + (255);
    }
    else if (m_nYourLevel >= 200)
    {
        rnLevel = 200;
        rdwColor = (255 << 24) + (255 << 16) + (230 << 8) + (210);
    }
    else if (m_nYourLevel >= 100)
    {
        rnLevel = 100;
        rdwColor = (255 << 24) + (24 << 16) + (201 << 8) + (0);
    }
    else if (m_nYourLevel >= 50)
    {
        rnLevel = 50;
        rdwColor = (255 << 24) + (0 << 16) + (150 << 8) + (255);
    }
    else							//  빨간색.
    {
        rnLevel = 10;
        rdwColor = (255 << 24) + (0 << 16) + (0 << 8) + (255);
    }
}

float CNewUITrade::GetLayerDepth()
{
    return 2.1f;
}

void CNewUITrade::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_TRADE_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_TRADE_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_TRADE_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_TRADE_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_TRADE_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_myquest_Line.tga", IMAGE_TRADE_LINE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Account_title.tga", IMAGE_TRADE_NICK_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_money.tga", IMAGE_TRADE_MONEY, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Bt_accept.tga", IMAGE_TRADE_CONFIRM, GL_LINEAR);
    LoadBitmap(L"Interface\\CursorSitDown.tga", IMAGE_TRADE_WARNING_ARROW, GL_LINEAR, GL_CLAMP);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_TRADE_BTN_CLOSE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Bt_money01.tga", IMAGE_TRADE_BTN_ZEN_INPUT, GL_LINEAR);
}

void CNewUITrade::UnloadImages()
{
    DeleteBitmap(IMAGE_TRADE_BTN_ZEN_INPUT);
    DeleteBitmap(IMAGE_TRADE_BTN_CLOSE);
    DeleteBitmap(IMAGE_TRADE_WARNING_ARROW);
    DeleteBitmap(IMAGE_TRADE_CONFIRM);
    DeleteBitmap(IMAGE_TRADE_MONEY);
    DeleteBitmap(IMAGE_TRADE_NICK_BACK);
    DeleteBitmap(IMAGE_TRADE_LINE);
    DeleteBitmap(IMAGE_TRADE_BOTTOM);
    DeleteBitmap(IMAGE_TRADE_RIGHT);
    DeleteBitmap(IMAGE_TRADE_LEFT);
    DeleteBitmap(IMAGE_TRADE_TOP);
    DeleteBitmap(IMAGE_TRADE_BACK);
}

void CNewUITrade::ProcessClosing()
{
    m_pYourInvenCtrl->RemoveAllItems();
    m_pMyInvenCtrl->RemoveAllItems();

    if (m_bTradeAlert)
        InitYourInvenBackUp();
}

void CNewUITrade::ProcessMyInvenCtrl()
{
    if (NULL == m_pMyInvenCtrl)
        return;

    if (SEASON3B::IsPress(VK_LBUTTON))
    {
        CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
        if (NULL == pPickedItem)
            return;

        ITEM* pItemObj = pPickedItem->GetItem();
        if (pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
        {
            int nSrcIndex = pPickedItem->GetSourceLinealPos();
            int nDstIndex = pPickedItem->GetTargetLinealPos(m_pMyInvenCtrl);
            if (nDstIndex != -1 && m_pMyInvenCtrl->CanMove(nDstIndex, pItemObj))
                SendRequestItemToTrade(pItemObj, nSrcIndex, nDstIndex);
        }
        else if (pPickedItem->GetOwnerInventory() == m_pMyInvenCtrl)
        {
            int nSrcIndex = pPickedItem->GetSourceLinealPos();
            int nDstIndex = pPickedItem->GetTargetLinealPos(m_pMyInvenCtrl);
            if (nDstIndex != -1 && m_pMyInvenCtrl->CanMove(nDstIndex, pItemObj))
            {
                SendRequestEquipmentItem(STORAGE_TYPE::TRADE, nSrcIndex, pItemObj, STORAGE_TYPE::TRADE, nDstIndex);
            }
        }
        else if (pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
        {
            int nSrcIndex = pPickedItem->GetSourceLinealPos();
            int nDstIndex = pPickedItem->GetTargetLinealPos(m_pMyInvenCtrl);
            if (nDstIndex != -1 && m_pMyInvenCtrl->CanMove(nDstIndex, pItemObj))
                SendRequestItemToTrade(pItemObj, nSrcIndex, nDstIndex);
        }
    }
}

void CNewUITrade::SendRequestItemToTrade(ITEM* pItemObj, int nInvenIndex,
    int nTradeIndex)
{
    if (::IsTradeBan(pItemObj))
    {
        g_pSystemLogBox->AddText(GlobalText[494], SEASON3B::TYPE_ERROR_MESSAGE);
    }
    else
    {
        m_bMyConfirm = false;
        SocketClient->ToGameServer()->SendTradeButtonStateChange(m_bMyConfirm);

        SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, nInvenIndex,
            pItemObj, STORAGE_TYPE::TRADE, nTradeIndex);
    }
}

void CNewUITrade::SendRequestItemToMyInven(ITEM* pItemObj, int nTradeIndex, int nInvenIndex)
{
    SendRequestEquipmentItem(STORAGE_TYPE::TRADE, nTradeIndex, pItemObj, STORAGE_TYPE::INVENTORY, nInvenIndex);

    if (m_bMyConfirm)
    {
        AlertTrade();
    }
    m_nMyTradeWait = 150;
}

void CNewUITrade::SendRequestMyGoldInput(int nInputGold)
{
    if (nInputGold <= (int)CharacterMachine->Gold + m_nMyTradeGold)
    {
        if (m_bMyConfirm)
        {
            m_bMyConfirm = false;
            SocketClient->ToGameServer()->SendTradeButtonStateChange(m_bMyConfirm);
        }

        if (m_nMyTradeGold > 0)
            m_nMyTradeWait = 150;

        m_nTempMyTradeGold = nInputGold;
        SocketClient->ToGameServer()->SendSetTradeMoney(nInputGold);
    }
    else
    {
        SEASON3B::CreateOkMessageBox(GlobalText[423]);
    }
}

void CNewUITrade::ProcessCloseBtn()
{
    if (CNewUIInventoryCtrl::GetPickedItem() == NULL)
    {
        m_bTradeAlert = false;
        SocketClient->ToGameServer()->SendTradeCancel();
    }
}

bool CNewUITrade::ProcessBtns()
{
    if (m_nMyTradeWait > 0)
        --m_nMyTradeWait;

    if (m_abtn[BTN_CLOSE].UpdateMouseEvent())
    {
        ::PlayBuffer(SOUND_CLICK01);
        ProcessCloseBtn();
        return true;
    }
    else if (SEASON3B::IsPress(VK_LBUTTON)
        && CheckMouseIn(m_Pos.x + 169, m_Pos.y + 7, 13, 12))
    {
        ::PlayBuffer(SOUND_CLICK01);
        ProcessCloseBtn();
        return true;
    }
    else if (m_abtn[BTN_ZEN_INPUT].UpdateMouseEvent())
    {
        SEASON3B::CreateMessageBox(
            MSGBOX_LAYOUT_CLASS(SEASON3B::CTradeZenMsgBoxLayout));
        ::PlayBuffer(SOUND_CLICK01);
        return true;
    }
    else if (SEASON3B::IsRelease(VK_LBUTTON)
        && CheckMouseIn(m_posMyConfirm.x, m_posMyConfirm.y, CONFIRM_WIDTH, CONFIRM_HEIGHT))
    {
        if (0 == m_nMyTradeWait && CNewUIInventoryCtrl::GetPickedItem() == NULL)
        {
            ::PlayBuffer(SOUND_CLICK01);

            if (m_bTradeAlert && !m_bMyConfirm)
            {
                SEASON3B::CreateMessageBox(
                    MSGBOX_LAYOUT_CLASS(SEASON3B::CTradeAlertMsgBoxLayout));
            }
            else
            {
                AlertTrade();
            }
        }
        return true;
    }

    return false;
}

void CNewUITrade::AlertTrade()
{
    m_bMyConfirm = !m_bMyConfirm;

    m_bTradeAlert = true;
    SocketClient->ToGameServer()->SendTradeButtonStateChange(m_bMyConfirm);
}

void CNewUITrade::GetYourID(wchar_t* pszYourID)
{
    ::wcscpy(pszYourID, m_szYourID);
}

void CNewUITrade::ProcessToReceiveTradeRequest(char* pbyYourID)
{
    if (g_pNewUISystem->IsImpossibleTradeInterface())
    {
        SocketClient->ToGameServer()->SendTradeRequestResponse(false);
        return;
    }

    CMultiLanguage::ConvertFromUtf8(m_szYourID, pbyYourID);

    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CTradeMsgBoxLayout));

    SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
}

void CNewUITrade::ProcessToReceiveTradeResult(LPPTRADE pTradeData)
{
    switch (pTradeData->SubCode)
    {
    case 0:
        g_pSystemLogBox->AddText(GlobalText[492], SEASON3B::TYPE_ERROR_MESSAGE);
        break;

    case 2:
        g_pSystemLogBox->AddText(GlobalText[493], SEASON3B::TYPE_ERROR_MESSAGE);
        break;

    case 1:
        g_pNewUISystem->Show(SEASON3B::INTERFACE_TRADE);

        InitTradeInfo();

        int x = 260 * MouseX / 640;
        SetCursorPos(x * WindowWidth / 640, MouseY * WindowHeight / 480);

        wchar_t szTempID[MAX_ID_SIZE + 1]{ };
        CMultiLanguage::ConvertFromUtf8(szTempID, pTradeData->ID, MAX_ID_SIZE);

        if (!m_bTradeAlert && ::wcscmp(m_szYourID, szTempID))
            InitYourInvenBackUp();

        m_bTradeAlert = false;
        m_nYourGuildType = pTradeData->GuildKey;
        wcsncpy(m_szYourID, szTempID, MAX_ID_SIZE);
        m_nYourLevel = pTradeData->Level;   //  상대방 레벨.
        break;
    }
}

void CNewUITrade::ProcessToReceiveYourItemDelete(BYTE byYourInvenIndex)
{
    BackUpYourInven(int(byYourInvenIndex));
    ITEM* pYourItemObj = m_pYourInvenCtrl->FindItem(int(byYourInvenIndex));
    m_pYourInvenCtrl->RemoveItem(pYourItemObj);
    AlertYourTradeInven();
    ::PlayBuffer(SOUND_GET_ITEM01);
}

void CNewUITrade::BackUpYourInven(int nYourInvenIndex)
{
    ITEM* pYourItemObj = m_pYourInvenCtrl->FindItem(nYourInvenIndex);
    BackUpYourInven(pYourItemObj);
}

void CNewUITrade::BackUpYourInven(ITEM* pYourItemObj)
{
    if ((pYourItemObj->Type >= ITEM_HELPER && pYourItemObj->Type <= ITEM_DARK_HORSE_ITEM)
        || (pYourItemObj->Type == ITEM_JEWEL_OF_BLESS || pYourItemObj->Type == ITEM_JEWEL_OF_SOUL || pYourItemObj->Type == ITEM_JEWEL_OF_LIFE)
        || (pYourItemObj->Type >= ITEM_JEWEL_OF_GUARDIAN)
        || (COMGEM::isCompiledGem(pYourItemObj))
        || (pYourItemObj->Type >= ITEM_WING && pYourItemObj->Type <= ITEM_WINGS_OF_DARKNESS)
        || (pYourItemObj->Type >= ITEM_CAPE_OF_LORD)
        || (pYourItemObj->Type >= ITEM_WING_OF_STORM && pYourItemObj->Type <= ITEM_WING_OF_DIMENSION)
        || (pYourItemObj->Type == ITEM_JEWEL_OF_CHAOS)
        || (pYourItemObj->Type >= ITEM_CAPE_OF_FIGHTER && pYourItemObj->Type <= ITEM_CAPE_OF_OVERRULE)
        || ((pYourItemObj->Level > 4 && pYourItemObj->Type < ITEM_WING) || pYourItemObj->ExcellentFlags > 0))
    {
        int nCompareValue;
        bool bSameItem = false;

        for (int i = 0; i < MAX_TRADE_INVEN; ++i)
        {
            if (-1 == m_aYourInvenBackUp[i].Type)
                continue;

            nCompareValue = ::CompareItem(m_aYourInvenBackUp[i], *pYourItemObj);
            if (0 == nCompareValue)
            {
                bSameItem = true;
                break;
            }
            else if (-1 == nCompareValue)
            {
                bSameItem = true;
                m_aYourInvenBackUp[i] = *pYourItemObj;
                break;
            }
            else if (2 != nCompareValue)
            {
                bSameItem = true;
            }
        }

        if (!bSameItem)
        {
            for (int i = 0; i < MAX_TRADE_INVEN; ++i)
            {
                if (-1 == m_aYourInvenBackUp[i].Type)
                {
                    m_aYourInvenBackUp[i] = *pYourItemObj;
                    break;
                }
            }
        }
    }
}

void CNewUITrade::ProcessToReceiveYourItemAdd(BYTE byYourInvenIndex, std::span<const BYTE> pbyItemPacket)
{
    m_pYourInvenCtrl->AddItem(byYourInvenIndex, pbyItemPacket);
    AlertYourTradeInven();
    ::PlayBuffer(SOUND_GET_ITEM01);
}

void CNewUITrade::AlertYourTradeInven()
{
    int nCount = 0;
    int nCompareItemType[10];

    m_bTradeAlert = false;

    int nYourItems = m_pYourInvenCtrl->GetNumberOfItems();
    ITEM* pYourItemObj;
    int nCompareValue;

    for (int i = 0; i < nYourItems; ++i)
    {
        pYourItemObj = m_pYourInvenCtrl->GetItem(i);
        for (int j = 0; j < MAX_TRADE_INVEN; ++j)
        {
            if (m_aYourInvenBackUp[j].Type == pYourItemObj->Type)
            {
                nCompareValue = ::CompareItem(m_aYourInvenBackUp[j], *pYourItemObj);
                if (1 == nCompareValue)
                {
                    m_bTradeAlert = true;
                    pYourItemObj->byColorState = ITEM_COLOR_TRADE_WARNING;
                }
                else
                {
                    if (0 == nCompareValue)
                        nCompareItemType[nCount++] = m_aYourInvenBackUp[j].Type;

                    pYourItemObj->byColorState = ITEM_COLOR_NORMAL;
                    break;
                }
            }
        }
    }

    if (nCount > 0)
    {
        m_bTradeAlert = false;
        for (int i = 0; i < nCount; ++i)
        {
            for (int j = 0; j < nYourItems; ++j)
            {
                pYourItemObj = m_pYourInvenCtrl->GetItem(j);
                if (nCompareItemType[i] == pYourItemObj->Type)
                    pYourItemObj->byColorState = ITEM_COLOR_NORMAL;
            }
        }
    }
}

void CNewUITrade::ProcessToReceiveMyTradeGold(BYTE bySuccess)
{
    m_nMyTradeGold = bySuccess ? m_nTempMyTradeGold : 0;
}

void CNewUITrade::ProcessToReceiveYourConfirm(BYTE byState)
{
    switch (byState)
    {
    case 0:
        m_bYourConfirm = false;
        break;
    case 1:
        m_bYourConfirm = true;
        break;
    case 2:
        m_bMyConfirm = false;
        m_bYourConfirm = false;
        m_nMyTradeWait = 150;
        break;
    case 3:
        break;
    }

    PlayBuffer(SOUND_CLICK01);
}

void CNewUITrade::ProcessToReceiveTradeExit(BYTE byState)
{
    switch (byState)
    {
    case 0:
    {
        g_pSystemLogBox->AddText(GlobalText[492], SEASON3B::TYPE_ERROR_MESSAGE);

        m_bTradeAlert = false;

        int nYourItems = m_pYourInvenCtrl->GetNumberOfItems();
        for (int i = 0; i < nYourItems; ++i)
            BackUpYourInven(m_pYourInvenCtrl->GetItem(i));
    }
    break;

    case 2:
        g_pSystemLogBox->AddText(GlobalText[495], SEASON3B::TYPE_ERROR_MESSAGE);
        break;

    case 3:
        g_pSystemLogBox->AddText(GlobalText[496], SEASON3B::TYPE_ERROR_MESSAGE);
        break;

    case 4:
        g_pSystemLogBox->AddText(GlobalText[2108], SEASON3B::TYPE_ERROR_MESSAGE);
        break;
    }

    SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();

    g_MessageBox->PopMessageBox();

    g_pNewUISystem->Hide(SEASON3B::INTERFACE_TRADE);
}

void CNewUITrade::ProcessToReceiveTradeItems(int nIndex, std::span<const BYTE> pbyItemPacket)
{
    SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();

    if (nIndex >= 0 && nIndex < (m_pMyInvenCtrl->GetNumberOfColumn()
        * m_pMyInvenCtrl->GetNumberOfRow()))
        m_pMyInvenCtrl->AddItem(nIndex, pbyItemPacket);
}

int SEASON3B::CNewUITrade::GetPointedItemIndexMyInven()
{
    return m_pMyInvenCtrl->GetPointedSquareIndex();
}

int SEASON3B::CNewUITrade::GetPointedItemIndexYourInven()
{
    return m_pYourInvenCtrl->GetPointedSquareIndex();
}