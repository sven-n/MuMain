//*****************************************************************************
// File: NewUIStorageInventory.cpp
//*****************************************************************************

#include "stdafx.h"
#include "NewUIStorageInventory.h"

#include "DSPlaySound.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "ZzzInventory.h"

#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
#include "GameShop\MsgBoxIGSCommon.h"
#endif // KJH_PBG_ADD_INGAMESHOP_SYSTEM

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CNewUIStorageInventory::CNewUIStorageInventory()
{
    m_pNewUIMng = nullptr;
    m_pNewInventoryCtrl = nullptr;
    m_Pos.x = m_Pos.y = 0;
}

CNewUIStorageInventory::~CNewUIStorageInventory()
{
    Release();
}

bool CNewUIStorageInventory::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (nullptr == pNewUIMng || nullptr == g_pNewUI3DRenderMng
        || nullptr == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_STORAGE, this);

    m_pNewInventoryCtrl = new CNewUIInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::VAULT, g_pNewUI3DRenderMng, g_pNewItemMng, this, x + 15, y + 36, 8, 15))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }

    SetPos(x, y);

    LoadImages();

    constexpr int anToolTipText[MAX_BTN] = { 235, 236, 242 };
    for (int i = BTN_INSERT_ZEN; i < MAX_BTN; ++i)
    {
        m_abtn[i].ChangeButtonImgState(true, IMAGE_STORAGE_BTN_INSERT_ZEN + i);
        m_abtn[i].ChangeToolTipText(GlobalText[anToolTipText[i]], true);
    }

    m_BtnExpand.ChangeButtonImgState(true, IMAGE_STORAGE_EXPAND_BTN, false);
    m_BtnExpand.ChangeToolTipText(GlobalText[3338], true);

    m_bLock = false;
    SetItemAutoMove(false);
    InitBackupItemInfo();

    Show(false);

    return true;
}

void CNewUIStorageInventory::Release()
{
    UnloadImages();

    SAFE_DELETE(m_pNewInventoryCtrl);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }
}

void CNewUIStorageInventory::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->SetPos(x + 15, y + 36);
    }

    constexpr int xOffsetPerButton = 37;
    constexpr int xFirstButton = 13;
    for (int i = BTN_INSERT_ZEN; i < MAX_BTN; ++i)
    {
        m_abtn[i].ChangeButtonInfo(x + xFirstButton + i * xOffsetPerButton, y + 391, 36, 29);
    }

    m_BtnExpand.ChangeButtonInfo(x + xFirstButton + MAX_BTN * xOffsetPerButton, y + 391, 36, 29);
}

bool CNewUIStorageInventory::UpdateMouseEvent()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->UpdateMouseEvent())
        return false;

    ProcessInventoryCtrl();

    if (ProcessBtns())
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, STORAGE_WIDTH, STORAGE_HEIGHT))
    {
        if (IsPress(VK_RBUTTON))
        {
            MouseRButton = false;
            MouseRButtonPop = false;
            MouseRButtonPush = false;
            return false;
        }

        if (!IsNone(VK_LBUTTON))
        {
            return false;
        }
    }

    return true;
}

bool CNewUIStorageInventory::UpdateKeyEvent()
{
    if (!g_pNewUISystem->IsVisible(INTERFACE_STORAGE))
    {
        return true;
    }

    if (IsPress(VK_ESCAPE) == true)
    {
        g_pNewUISystem->Hide(INTERFACE_STORAGE);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }

    if (CharacterAttribute->IsVaultExtended > 0 && IsPress('H'))
    {
        g_pNewUISystem->Toggle(INTERFACE_STORAGE_EXT);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }
    return true;
}

bool CNewUIStorageInventory::Update()
{
    return !(m_pNewInventoryCtrl && !m_pNewInventoryCtrl->Update());
}

bool CNewUIStorageInventory::Render()
{
    EnableAlphaTest();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderBackImage();
    RenderText();

    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->Render();

    for (int i = BTN_INSERT_ZEN; i < MAX_BTN; ++i)
        m_abtn[i].Render();

    if (CharacterAttribute->IsVaultExtended > 0)
    {
        m_BtnExpand.Render();
    }

    DisableAlphaBlend();

    return true;
}

void CNewUIStorageInventory::RenderBackImage()
{
    const auto x = static_cast<float>(m_Pos.x);
    const auto y = static_cast<float>(m_Pos.y);
    RenderImage(IMAGE_STORAGE_BACK, x, y, STORAGE_WIDTH, STORAGE_HEIGHT);
    RenderImage(IMAGE_STORAGE_TOP, x, y, STORAGE_WIDTH, 64.f);
    RenderImage(IMAGE_STORAGE_LEFT, x, y + 64, 21.f, 320.f);
    RenderImage(IMAGE_STORAGE_RIGHT, x + STORAGE_WIDTH - 21, y + 64, 21.f, 320.f);
    RenderImage(IMAGE_STORAGE_BOTTOM, x, y + STORAGE_HEIGHT - 45, STORAGE_WIDTH, 45.f);

    RenderImage(IMAGE_STORAGE_MONEY, x + 10, y + 342, 170.f, 46.f);
}

void CNewUIStorageInventory::RenderText()
{
    wchar_t szTemp[128];
    int nTempZen;

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0);

    swprintf(
        szTemp, L"%s (%s)", GlobalText[234], GlobalText[m_bLock ? 241 : 240]);
    if (m_bLock)
        g_pRenderText->SetTextColor(240, 32, 32, 255);
    else
        g_pRenderText->SetTextColor(216, 216, 216, 255);
    g_pRenderText->RenderText(
        m_Pos.x, m_Pos.y + 11, szTemp, STORAGE_WIDTH, 0, RT3_SORT_CENTER);

    nTempZen = CharacterMachine->StorageGold;
    ConvertGold(nTempZen, szTemp);
    g_pRenderText->SetTextColor(getGoldColor(nTempZen));
    g_pRenderText->RenderText(
        m_Pos.x + 168, m_Pos.y + 342 + 8, szTemp, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

    g_pRenderText->SetTextColor(240, 64, 64, 255);
    g_pRenderText->RenderText(m_Pos.x + 10 + 15, m_Pos.y + 342 + 29, GlobalText[266]);

    __int64 iTotalLevel = (__int64)CharacterAttribute->Level + Master_Level_Data.nMLevel;

    nTempZen = int(double(iTotalLevel) * double(iTotalLevel) * 0.04);
    nTempZen += m_bLock ? int(CharacterAttribute->Level) * 2 : 0;
    nTempZen = max(1, nTempZen);

    if (nTempZen >= 1000)
        nTempZen = nTempZen / 100 * 100;
    else if (nTempZen >= 100)
        nTempZen = nTempZen / 10 * 10;

    ConvertGold(nTempZen, szTemp);
    g_pRenderText->SetTextColor(255, 220, 150, 255);
    g_pRenderText->RenderText(m_Pos.x + 168, m_Pos.y + 342 + 29, szTemp, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);
}

float CNewUIStorageInventory::GetLayerDepth()
{
    return 2.2f;
}

CNewUIInventoryCtrl* CNewUIStorageInventory::GetInventoryCtrl() const
{
    return m_pNewInventoryCtrl;
}

void CNewUIStorageInventory::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_STORAGE_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_STORAGE_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_STORAGE_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_STORAGE_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_STORAGE_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Bt_money01.tga", IMAGE_STORAGE_BTN_INSERT_ZEN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Bt_money02.tga", IMAGE_STORAGE_BTN_TAKE_ZEN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Bt_lock02.tga", IMAGE_STORAGE_BTN_UNLOCK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Bt_lock.tga", IMAGE_STORAGE_BTN_LOCK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_money3.tga", IMAGE_STORAGE_MONEY, GL_LINEAR);
}

void CNewUIStorageInventory::UnloadImages()
{
    DeleteBitmap(IMAGE_STORAGE_MONEY);
    DeleteBitmap(IMAGE_STORAGE_BTN_LOCK);
    DeleteBitmap(IMAGE_STORAGE_BTN_UNLOCK);
    DeleteBitmap(IMAGE_STORAGE_BTN_TAKE_ZEN);
    DeleteBitmap(IMAGE_STORAGE_BTN_INSERT_ZEN);
    DeleteBitmap(IMAGE_STORAGE_BOTTOM);
    DeleteBitmap(IMAGE_STORAGE_RIGHT);
    DeleteBitmap(IMAGE_STORAGE_LEFT);
    DeleteBitmap(IMAGE_STORAGE_TOP);
    DeleteBitmap(IMAGE_STORAGE_BACK);
}

void CNewUIStorageInventory::LockStorage(bool bLock)
{
    m_bLock = bLock;
    ChangeLockBtnImage();
}

void CNewUIStorageInventory::ChangeLockBtnImage()
{
    m_abtn[BTN_LOCK].UnRegisterButtonState();
    if (m_bLock)
        m_abtn[BTN_LOCK].ChangeButtonImgState(true, IMAGE_STORAGE_BTN_LOCK);
    else
        m_abtn[BTN_LOCK].ChangeButtonImgState(true, IMAGE_STORAGE_BTN_UNLOCK);
}

bool CNewUIStorageInventory::ProcessClosing()
{
    if (EquipmentItem)
        return false;

    CNewUIInventoryCtrl::BackupPickedItem();
    DeleteAllItems();
    SocketClient->ToGameServer()->SendVaultClosed();
    return true;
}

bool CNewUIStorageInventory::InsertItem(int nIndex, std::span<const BYTE> pbyItemPacket)
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->AddItem(nIndex, pbyItemPacket);

    return false;
}

void CNewUIStorageInventory::DeleteAllItems()
{
    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->RemoveAllItems();
}

void CNewUIStorageInventory::ProcessInventoryCtrl()
{
    if (nullptr == m_pNewInventoryCtrl)
    {
        return;
    }

    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    if (pPickedItem)
    {
        ITEM* pItemObj = pPickedItem->GetItem();
        if (nullptr == pItemObj)
        {
            return;
        }

        if (IsPress(VK_LBUTTON) || IsRelease(VK_LBUTTON))
        {
            const int nDstIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);

            if (nDstIndex >= 0 && m_pNewInventoryCtrl->CanMove(nDstIndex, pItemObj))
            {
                const int nSrcIndex = pPickedItem->GetSourceLinealPos();
                const auto sourceStorageType = pPickedItem->GetSourceStorageType();
                const auto targetStorageType = m_pNewInventoryCtrl->GetStorageType();
                SendRequestEquipmentItem(sourceStorageType, nSrcIndex,
                    pItemObj, targetStorageType, nDstIndex);
            }
        }
        else
        {
            if (::IsStoreBan(pItemObj))
            {
                m_pNewInventoryCtrl->SetSquareColorNormal(1.0f, 0.0f, 0.0f);
            }
            else
            {
                m_pNewInventoryCtrl->SetSquareColorNormal(0.1f, 0.4f, 0.8f);
            }
        }
    }
    else if (IsPress(VK_RBUTTON))
    {
        ProcessStorageItemAutoMove();
    }
}

void CNewUIStorageInventory::ProcessStorageItemAutoMove()
{
    if (g_pPickedItem)
        if (g_pPickedItem->GetItem())
            return;

    if (IsItemAutoMove())
        return;

    ITEM* pItemObj = m_pNewInventoryCtrl->FindItemAtPt(MouseX, MouseY);
    if (pItemObj)
    {
        int nDstIndex = g_pMyInventory->FindEmptySlot(pItemObj);
        if (-1 != nDstIndex)
        {
            SetItemAutoMove(true);

            int nSrcIndex
                = pItemObj->y * m_pNewInventoryCtrl->GetNumberOfColumn()
                + pItemObj->x;
            SendRequestItemToMyInven(pItemObj, nSrcIndex, nDstIndex);

            PlayBuffer(SOUND_GET_ITEM01);
        }
    }
}

bool CNewUIStorageInventory::ProcessMyInvenItemAutoMove()
{
    if (g_pPickedItem && g_pPickedItem->GetItem())
    {
        return false;
    }

    if (IsItemAutoMove())
    {
        return false;
    }

    const auto pMyInvenCtrl = g_pMyInventory->GetInventoryCtrl();
    if (const auto pItemObj = pMyInvenCtrl->FindItemAtPt(MouseX, MouseY))
    {
        if (pItemObj->Type == ITEM_WIZARDS_RING)
            return false;

        const int emptySlotIndex = FindEmptySlot(pItemObj);
        if (-1 != emptySlotIndex)
        {
            SetItemAutoMove(true);

            const int nSrcIndex = pMyInvenCtrl->GetIndexByItem(pItemObj);
            SendRequestItemToStorage(pItemObj, nSrcIndex, emptySlotIndex);
            PlayBuffer(SOUND_GET_ITEM01);
            return true;
        }
    }

    return false;
}

void CNewUIStorageInventory::SendRequestItemToMyInven(ITEM* pItemObj, int nStorageIndex, int nInvenIndex)
{
    if (!IsStorageLocked() || IsCorrectPassword())
    {
        SendRequestEquipmentItem(STORAGE_TYPE::VAULT, nStorageIndex,
            pItemObj, STORAGE_TYPE::INVENTORY, nInvenIndex);
    }
    else
    {
        SetBackupInvenIndex(nInvenIndex);
        if (!IsItemAutoMove())
            g_pPickedItem->HidePickedItem();

        CreateMessageBox(
            MSGBOX_LAYOUT_CLASS(SEASON3B::CPasswordKeyPadMsgBoxLayout));
    }
}

void CNewUIStorageInventory::SendRequestItemToStorage(ITEM* pItemObj, int nInvenIndex, int nStorageIndex)
{
    if (IsStoreBan(pItemObj))
    {
#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
        // MessageBox
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[3028], GlobalText[667]);
#endif // KJH_PBG_ADD_INGAMESHOP_SYSTEM

        g_pSystemLogBox->AddText(GlobalText[667], TYPE_ERROR_MESSAGE);
        CNewUIInventoryCtrl::BackupPickedItem();

        if (IsItemAutoMove())
            SetItemAutoMove(false);
    }
    else
    {
        SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, nInvenIndex,
            pItemObj, STORAGE_TYPE::VAULT, nStorageIndex);
    }
}

bool CNewUIStorageInventory::ProcessBtns()
{
    if (CharacterAttribute->IsVaultExtended > 0 && m_BtnExpand.UpdateMouseEvent())
    {
        g_pNewUISystem->Toggle(INTERFACE_STORAGE_EXT);
        return true;
    }

    if (m_abtn[BTN_INSERT_ZEN].UpdateMouseEvent())
    {
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CZenReceiptMsgBoxLayout));
        return true;
    }

    if (m_abtn[BTN_TAKE_ZEN].UpdateMouseEvent())
    {
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CZenPaymentMsgBoxLayout));
        return true;
    }

    if (m_abtn[BTN_LOCK].UpdateMouseEvent())
    {
        if (m_bLock)
        {
            CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CStorageUnlockMsgBoxLayout));
        }
        else
        {
            CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CStorageLockKeyPadMsgBoxLayout));
        }
        return true;
    }

    if (IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x + 169, m_Pos.y + 7, 13, 12))
    {
        g_pNewUISystem->Hide(INTERFACE_STORAGE);
        return true;
    }

    return false;
}

void CNewUIStorageInventory::SetItemAutoMove(bool bItemAutoMove)
{
    m_bItemAutoMove = bItemAutoMove;

    if (bItemAutoMove)
    {
        m_nBackupMouseX = MouseX;
        m_nBackupMouseY = MouseY;
    }
    else
        m_nBackupMouseX = m_nBackupMouseY = 0;
}

void CNewUIStorageInventory::InitBackupItemInfo()
{
    m_bTakeZen = false;
    m_nBackupTakeZen = 0;
    m_nBackupInvenIndex = -1;
}

void CNewUIStorageInventory::SetBackupTakeZen(int nZen)
{
    m_bTakeZen = true;
    m_nBackupTakeZen = nZen;
}

void CNewUIStorageInventory::SetBackupInvenIndex(int nInvenIndex)
{
    m_bTakeZen = false;
    m_nBackupInvenIndex = nInvenIndex;
}

int CNewUIStorageInventory::FindEmptySlot(ITEM* pItemObj)
{
    if (pItemObj == nullptr)
        return -1;

    ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItemObj->Type];

    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindEmptySlot(pItemAttr->Width, pItemAttr->Height);

    return -1;
}

void CNewUIStorageInventory::ProcessToReceiveStorageStatus(BYTE byStatus)
{
    switch (byStatus)
    {
    case 0:
        LockStorage(false);
        SetCorrectPassword(false);
        break;

    case 1:
        LockStorage(true);
        SetCorrectPassword(false);
        break;

    case 10:
        CreateOkMessageBox(GlobalText[440]);
        CNewUIInventoryCtrl::BackupPickedItem();
        ProcessStorageItemAutoMoveFailure();
        break;

    case 11:
        CreateOkMessageBox(GlobalText[441]);
        break;

    case 12:
        if (IsStorageLocked() && !IsCorrectPassword())
        {
            if (m_bTakeZen)
            {
                SocketClient->ToGameServer()->SendVaultMoveMoneyRequest(1, GetBackupTakeZen());
                InitBackupItemInfo();
            }
            else
            {
                ITEM* pItemObj;
                int nStorageIndex;

                if (IsItemAutoMove())
                {
                    pItemObj = m_pNewInventoryCtrl->FindItemAtPt(
                        m_nBackupMouseX, m_nBackupMouseY);
                    nStorageIndex
                        = pItemObj->y * m_pNewInventoryCtrl->GetNumberOfColumn()
                        + pItemObj->x;
                }
                else
                {
                    nStorageIndex = g_pPickedItem->GetSourceLinealPos();
                    pItemObj = g_pPickedItem->GetItem();
                }

                SendRequestEquipmentItem(
                    STORAGE_TYPE::VAULT, nStorageIndex,
                    pItemObj, STORAGE_TYPE::INVENTORY, GetBackupInvenIndex());

                InitBackupItemInfo();
            }
        }
        LockStorage(true);
        SetCorrectPassword(true);
        break;

    case 13:
        CreateOkMessageBox(GlobalText[401]);
        break;
    }
}

void CNewUIStorageInventory::ProcessToReceiveStorageItems(int nIndex, std::span<const BYTE> pbyItemPacket)
{
    CNewUIInventoryCtrl::DeletePickedItem();

    if (nIndex >= 0 && nIndex < (m_pNewInventoryCtrl->GetNumberOfColumn()
        * m_pNewInventoryCtrl->GetNumberOfRow()))
    {
        if (IsItemAutoMove())
        {
            CNewUIInventoryCtrl* pMyInvenCtrl = g_pMyInventory->GetInventoryCtrl();
            ITEM* pItemObj = pMyInvenCtrl->FindItemAtPt(m_nBackupMouseX, m_nBackupMouseY);
            g_pMyInventory->GetInventoryCtrl()->RemoveItem(pItemObj);

            SetItemAutoMove(false);
        }

        InsertItem(nIndex, pbyItemPacket);
    }
}

void CNewUIStorageInventory::ProcessStorageItemAutoMoveSuccess()
{
    if (!IsVisible())
        return;

    if (IsItemAutoMove())
    {
        ITEM* pItemObj = m_pNewInventoryCtrl->FindItemAtPt(m_nBackupMouseX, m_nBackupMouseY);
        m_pNewInventoryCtrl->RemoveItem(pItemObj);

        SetItemAutoMove(false);
    }
}

void CNewUIStorageInventory::ProcessStorageItemAutoMoveFailure()
{
    if (!IsVisible())
        return;

    InitBackupItemInfo();
    SetItemAutoMove(false);
}

int CNewUIStorageInventory::GetPointedItemIndex()
{
    return m_pNewInventoryCtrl->GetPointedSquareIndex();
}