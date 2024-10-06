// NewUILuckyItemWnd.cpp: implementation of the CNewUILuckyItemWnd class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUILuckyItemWnd.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "ZzzBMD.h"
#include "ZzzEffect.h"
#include "ZzzObject.h"
#include "ZzzInventory.h"
#include "ZzzInterface.h"
#include "ZzzInfomation.h"
#include "ZzzCharacter.h"

#include "DSPlaySound.h"
#include "MixMgr.h"

using namespace SEASON3B;
CNewUILuckyItemWnd::CNewUILuckyItemWnd()
{
    memset(m_szSubject, 0, 255);
    m_eType = eLuckyItemType_None;
    m_nMixEffectTimer = 0;
}

CNewUILuckyItemWnd::~CNewUILuckyItemWnd()
{
#ifdef LEM_FIX_LUCKYITEM_UICLASS_SAFEDELETE
    Release();
#endif // LEM_FIX_LUCKYITEM_UICLASS_SAFEDELETE
}

void CNewUILuckyItemWnd::SetFrame(void)
{
    float	fLineY = m_ptPos.y + m_sImgList[eFrame_T].s_fHgt;
    float	fBottomY = fLineY + m_sImgList[eFrame_L].s_fHgt;
    float	fLineX_R = m_ptPos.x + m_fSizeX - m_sImgList[eFrame_L].s_fWid;

    for (int i = 0; i < eFrame_END; i++)
    {
        m_sFrame[i].s_Img = m_sImgList[i];
    }

    m_sFrame[eFrame_BG].s_ptPos = m_ptPos;
    m_sFrame[eFrame_T].s_ptPos = m_ptPos;

    m_sFrame[eFrame_L].s_ptPos.x = m_ptPos.x;
    m_sFrame[eFrame_L].s_ptPos.y = fLineY;

    m_sFrame[eFrame_R].s_ptPos.x = fLineX_R;
    m_sFrame[eFrame_R].s_ptPos.y = fLineY;

    m_sFrame[eFrame_B].s_ptPos.x = m_ptPos.x;
    m_sFrame[eFrame_B].s_ptPos.y = fBottomY;
}

int CNewUILuckyItemWnd::GetLuckyItemRate(int _nType)
{
    if (_nType == eLuckyItemType_Trade)		return 100;
    if (_nType == eLuckyItemType_Refinery)	return 50;

    return 0;
}

void CNewUILuckyItemWnd::Render_Frame(void)
{
    int	i = 0;

    for (i = 0; i < eFrame_END; i++)
    {
        RenderImage(m_sFrame[i].s_Img.s_nImgIndex, m_sFrame[i].s_ptPos.x, m_sFrame[i].s_ptPos.y, m_sFrame[i].s_Img.s_fWid, m_sFrame[i].s_Img.s_fHgt);
    }

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->RenderText(m_ptPos.x, m_ptPos.y + 18.0f, m_szSubject, m_fSizeX, 0, RT3_SORT_CENTER);

    if (m_eEnd != eLuckyItem_End)
    {
        m_BtnMix.Render();
    }
    else
    {
        g_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);
    }

    float fTextY = m_ptPos.y + 18.0f + 190;
    for (i = 0; i < m_nTextMaxLine; i++)
    {
        if (m_sText[i].s_nTextIndex < 0)	break;
        if (m_sText[i].s_nTextIndex == 0)	continue;

        g_pRenderText->SetFont(g_hFont);
        g_pRenderText->SetTextColor(m_sText[i].s_dwColor);
        g_pRenderText->RenderText(m_ptPos.x + 10, fTextY + 11.0f * i, GlobalText[m_sText[i].s_nTextIndex], m_fSizeX - 20, 0, m_sText[i].s_nLine);
    }
}

STORAGE_TYPE CNewUILuckyItemWnd::SetMoveAction()
{
    m_eWndAction = eLuckyItem_Move;
    switch (m_eType)
    {
    case eLuckyItemType_Trade:
        return STORAGE_TYPE::LUCKYITEM_TRADE;
    case eLuckyItemType_Refinery:
        return STORAGE_TYPE::LUCKYITEM_REFINERY;
    }

    return STORAGE_TYPE::UNDEFINED;
}

int CNewUILuckyItemWnd::SetActAction()
{
    m_eWndAction = eLuckyItem_Act;
    switch (m_eType)
    {
        case eLuckyItemType_Trade:
            return 51;
        case eLuckyItemType_Refinery:
            return 52;
        default:
            return -1;
    }
}

void CNewUILuckyItemWnd::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    if (pClass)
    {
        CNewUILuckyItemWnd* pInventory = (CNewUILuckyItemWnd*)pClass;
        pInventory->RenderMixEffect();
    }
}

void CNewUILuckyItemWnd::RenderMixEffect()
{
    if (m_nMixEffectTimer <= 0)
    {
        return;
    }
    else
    {
        --m_nMixEffectTimer;
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

void CNewUILuckyItemWnd::GetResult(BYTE _byResult, int _nIndex, std::span<const BYTE> pbyItemPacket)
{
    int		nDefault = -1;
    int		nMessage = nDefault;
    int		nPlaySound = nDefault;
    int		nAddInven = -1;
    bool	bInitInven = false;

    SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();

    switch (m_eWndAction)
    {
    case eLuckyItem_Move:
        bInitInven = true;
        nAddInven = _nIndex;
        break;
    case eLuckyItem_Act:
        if (_byResult == 1)
        {
            bInitInven = true;
            nAddInven = _nIndex;
            nPlaySound = SOUND_JEWEL01;
            m_nMixEffectTimer = 50;
        }
        else
        {
            if (m_eType == eLuckyItemType_Trade)
            {
                nMessage = 3303;
            }
            else if (m_eType == eLuckyItemType_Refinery)
            {
                bInitInven = true;
            }
        }

        m_eEnd = eLuckyItem_End;
        SetFrame_Text(m_eEnd);
        break;
    }

    if (nMessage > nDefault)	g_pChatListBox->AddText(L"", GlobalText[nMessage], SEASON3B::TYPE_ERROR_MESSAGE);
    if (nPlaySound > nDefault)	PlayBuffer(static_cast<ESound>(nPlaySound));
    if (bInitInven)			g_pLuckyItemWnd->Process_InventoryCtrl_DeleteItem(-1);
    if (nAddInven > nDefault)	Process_InventoryCtrl_InsertItem(nAddInven, pbyItemPacket);

    m_eWndAction = eLuckyItem_None;
}

void CNewUILuckyItemWnd::LoadImg(void)
{
    float	fSizeX = m_fSizeX;
    float	fSizeY = m_fSizeY;
    float	fTop = 64.0f;
    float	fBottom = 45.0f;
    float	fLineX = 21.0f;
    float	fLineY = fSizeY - fTop - fBottom;

    wchar_t* szFileName[] = { L"Interface\\newui_msgbox_back.jpg",
                             L"Interface\\newui_item_back04.tga",
                             L"Interface\\newui_item_back02-L.tga",
                             L"Interface\\newui_item_back02-R.tga",
                             L"Interface\\newui_item_back03.tga",
                             L"Interface\\newui_bt_mix.tga",
    };

    m_sImgList[eFrame_BG].Set(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, fSizeX, fSizeY);
    m_sImgList[eFrame_T].Set(CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2, fSizeX, fTop);
    m_sImgList[eFrame_L].Set(CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT, fLineX, fLineY);
    m_sImgList[eFrame_R].Set(CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT, fLineX, fLineY);
    m_sImgList[eFrame_B].Set(CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM, fSizeX, fBottom);
    m_sImgList[eImgList_MixBtn].Set(BITMAP_INTERFACE_NEW_MIXINVENTORY_BEGIN, 44.0f, 35.0f);

    for (int i = 0; i < eImgList_END; i++)
    {
        LoadBitmap(szFileName[i], m_sImgList[i].s_nImgIndex, GL_LINEAR);
    }
}

bool CNewUILuckyItemWnd::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewUI3DRenderMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_LUCKYITEMWND, this);

    m_pNewInventoryCtrl = new CNewUIInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::LUCKYITEM_TRADE, g_pNewUI3DRenderMng, g_pNewItemMng, this, x + 15, y + 110, 8, 4))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }
    m_pNewInventoryCtrl->GetSquareColorNormal(m_fInvenClr);
    m_pNewInventoryCtrl->GetSquareColorWarning(m_fInvenClrWarning);

    SetPos(x, y);
    SetSize(190.0f, 429.0f);
    LoadImg();
    SetFrame();

    float	fWidth = 0.0f;
    float	fHeight = 0.0f;

    fWidth = m_sImgList[eImgList_MixBtn].s_fWid;
    fHeight = m_sImgList[eImgList_MixBtn].s_fHgt;
    m_BtnMix.ChangeButtonImgState(true, m_sImgList[eImgList_MixBtn].s_nImgIndex, false);
    m_BtnMix.ChangeButtonInfo(m_ptPos.x + (m_fSizeX - fWidth) * 0.5f, m_ptPos.y + 380, fWidth, fHeight);

    for (int i = 0; i < LUCKYITEMMAXLINE; i++)
    {
        m_sText[i].s_nTextIndex = -1;
        m_sText[i].s_dwColor = 0;
        m_sText[i].s_nLine = false;
    }

    Show(false);

    return true;
}

void CNewUILuckyItemWnd::Release()
{
#ifdef LEM_FIX_LUCKYITEM_UICLASS_SAFEDELETE
    SAFE_DELETE(m_pNewInventoryCtrl);
#endif // LEM_FIX_LUCKYITEM_UICLASS_SAFEDELETE
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUILuckyItemWnd::OpeningProcess(void)
{
    for (int i = 0; i < LUCKYITEMMAXLINE; i++)
    {
        m_sText[i].s_nTextIndex = -1;
        m_sText[i].s_dwColor = 0;
        m_sText[i].s_nLine = false;
    }
    m_nTextMaxLine = 0;
    m_eEnd = eLuckyItem_None;
    switch (m_eType)
    {
    case eLuckyItemType_Trade:
        swprintf(m_szSubject, L"%s", GlobalText[3288]);
        AddText(3291, 0xFF0000FF, RT3_SORT_LEFT), AddText(0), AddText(0), AddText(3292), AddText(3293), AddText(3294);
        AddText(0), AddText(0);
        AddText(2223, 0xFF00FFFF);
        AddText(0);
        AddText(3295, 0xFF0000FF), AddText(3296, 0xFF0000FF);
        m_BtnMix.ChangeToolTipText(GlobalText[591], true); // 조합
        break;
    case eLuckyItemType_Refinery:
        swprintf(m_szSubject, L"%s", GlobalText[3289]);
        AddText(2346, 0xFF0000FF, RT3_SORT_LEFT), AddText(0), AddText(0);
        AddText(3300), AddText(3301);
        AddText(0), AddText(0), AddText(0);
        AddText(3302, 0xFF0000FF);
        m_BtnMix.ChangeToolTipText(GlobalText[2061], true); // 제련
        break;
    }
}

void CNewUILuckyItemWnd::SetFrame_Text(eLUCKYITEM _eType)
{
    switch (_eType)
    {
    case eLuckyItem_End:
        for (int i = 0; i < LUCKYITEMMAXLINE; i++)
        {
            m_sText[i].s_nTextIndex = -1;
            m_sText[i].s_dwColor = 0;
            m_sText[i].s_nLine = false;
        }
        m_nTextMaxLine = 0;
        AddText(0);
        if (m_eType == eLuckyItemType_Trade)			AddText(1888);
        else if (m_eType == eLuckyItemType_Refinery) {}
        break;
    }
}

void CNewUILuckyItemWnd::AddText(int _nGlobalTextIndex, DWORD _dwColor, int _nLine)
{
    if (m_nTextMaxLine >= LUCKYITEMMAXLINE)	return;
    m_sText[m_nTextMaxLine].s_nTextIndex = _nGlobalTextIndex;
    m_sText[m_nTextMaxLine].s_dwColor = _dwColor;
    m_sText[m_nTextMaxLine].s_nLine = _nLine;
    m_nTextMaxLine++;
}

bool CNewUILuckyItemWnd::ClosingProcess(void)
{
    if (GetInventoryCtrl()->GetNumberOfItems() > 0 || CNewUIInventoryCtrl::GetPickedItem() != NULL)
    {
        g_pChatListBox->AddText(L"", GlobalText[593], SEASON3B::TYPE_ERROR_MESSAGE);
        return false;
    }

    SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();
    m_eType = eLuckyItemType_None;
    return true;
}

bool CNewUILuckyItemWnd::Process_InventoryCtrl_InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket)
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
    return false;
}

void CNewUILuckyItemWnd::Process_InventoryCtrl_DeleteItem(int iIndex)
{
    if (m_pNewInventoryCtrl)
    {
        if (iIndex == -1)
        {
            m_pNewInventoryCtrl->RemoveAllItems();
            return;
        }
        ITEM* pItem = m_pNewInventoryCtrl->FindItem(iIndex);
        if (pItem != NULL)
            m_pNewInventoryCtrl->RemoveItem(pItem);
    }
}

bool CNewUILuckyItemWnd::Check_LuckyItem_InWnd(void)
{
    if (GetInventoryCtrl()->GetNumberOfItems() > 0)	return true;
    return false;
}

bool CNewUILuckyItemWnd::Check_LuckyItem(ITEM* _pItem)
{
    switch (m_eType)
    {
    case eLuckyItemType_Trade:
        if (Check_LuckyItem_Trade(_pItem))		return true;
        break;
    case eLuckyItemType_Refinery:
        if (Check_LuckyItem_Refinery(_pItem))	return true;
        break;
    }

    return false;
}

bool CNewUILuckyItemWnd::Check_LuckyItem_Trade(ITEM* _pItem)
{
    if (_pItem->Type >= ITEM_HELPER + 135 && _pItem->Type <= ITEM_HELPER + 145)		return true;

    return false;
}

bool CNewUILuckyItemWnd::Check_LuckyItem_Refinery(ITEM* _pItem)
{
    if (_pItem->Type >= ITEM_ARMOR + 62 && _pItem->Type <= ITEM_ARMOR + 72)		return true;
    else if (_pItem->Type >= ITEM_HELM + 62 && _pItem->Type <= ITEM_HELM + 72)		return true;
    else if (_pItem->Type >= ITEM_BOOTS + 62 && _pItem->Type <= ITEM_BOOTS + 72)		return true;
    else if (_pItem->Type >= ITEM_GLOVES + 62 && _pItem->Type <= ITEM_GLOVES + 72)	return true;
    else if (_pItem->Type >= ITEM_PANTS + 62 && _pItem->Type <= ITEM_PANTS + 72)		return true;

    return false;
}

bool CNewUILuckyItemWnd::Process_InventoryCtrl(void)
{
    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    if (!m_pNewInventoryCtrl)	return false;
    if (!pPickedItem)			return false;

    ITEM* pItemObj = pPickedItem->GetItem();
    bool	bAct = Check_LuckyItem(pItemObj);

    if (!bAct || Check_LuckyItem_InWnd())
    {
        m_pNewInventoryCtrl->SetSquareColorNormal(m_fInvenClrWarning[0], m_fInvenClrWarning[1], m_fInvenClrWarning[2]);
        return false;
    }

    if (pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
    {
        if (SEASON3B::IsPress(VK_LBUTTON))
        {
            int iSourceIndex = pPickedItem->GetSourceLinealPos();
            int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
            if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
            {
                auto nMoveIndex = SetMoveAction();
                if (SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSourceIndex, pItemObj, nMoveIndex, iTargetIndex))
                    return true;
            }
        }
    }
    else if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
    {
        if (SEASON3B::IsPress(VK_LBUTTON))
        {
            int iSourceIndex = pPickedItem->GetSourceLinealPos();
            int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
            if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
            {
                auto nMoveIndex = SetMoveAction();
                if (SendRequestEquipmentItem(nMoveIndex, iSourceIndex, pItemObj, nMoveIndex, iTargetIndex))
                {
                    return true;
                }
            }
        }
    }

    // InventoryCtrl Background Color
    m_pNewInventoryCtrl->SetSquareColorNormal(m_fInvenClr[0], m_fInvenClr[1], m_fInvenClr[2]);
    return false;
}

CNewUIInventoryCtrl* CNewUILuckyItemWnd::GetInventoryCtrl() const
{
    return m_pNewInventoryCtrl;
}

bool CNewUILuckyItemWnd::Process_BTN_Action(void)
{
    if (!m_BtnMix.UpdateMouseEvent())
        return false;

    if (m_eEnd == eLuckyItem_End)
        return false;

    if (CNewUIInventoryCtrl::GetPickedItem())
        return false;

    if (!Check_LuckyItem_InWnd())
    {
        g_pChatListBox->AddText(L"", GlobalText[1817], SEASON3B::TYPE_ERROR_MESSAGE);
        return false;
    }
    if (!Check_LuckyItem(m_pNewInventoryCtrl->GetItem(0)))
    {
        g_pChatListBox->AddText(L"", GlobalText[1812], SEASON3B::TYPE_ERROR_MESSAGE);
        return false;
    }
#ifdef LEM_FIX_LUCKYITEM_SLOTCHECK
    if (g_pMyInventory->FindEmptySlot(4, 4) == -1)
#else // LEM_FIX_LUCKYITEM_SLOTCHECK
    if (g_pMyInventory->GetInventoryCtrl()->FindEmptySlot(4, 4) == -1)
#endif // LEM_FIX_LUCKYITEM_SLOTCHECK
    {
        g_pChatListBox->AddText(L"", GlobalText[1815], SEASON3B::TYPE_ERROR_MESSAGE);
        return false;
    }

    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CLuckyItemMsgBoxLayout));
    return true;
}

bool CNewUILuckyItemWnd::UpdateMouseEvent(void)
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->UpdateMouseEvent())
        return false;
    Process_InventoryCtrl();

    POINT ptExitBtn1 = { m_ptPos.x + 169, m_ptPos.y + 7 };

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_LUCKYITEMWND);
    }

    Process_BTN_Action();

    if (CheckMouseIn(m_ptPos.x, m_ptPos.y, m_fSizeX, m_fSizeY))
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

bool CNewUILuckyItemWnd::UpdateKeyEvent(void)
{
    return true;
}

bool CNewUILuckyItemWnd::Update(void)
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
        return false;

    return true;
}

bool CNewUILuckyItemWnd::Render(void)
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    Render_Frame();

    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->Render();

    DisableAlphaBlend();

    return true;
}

float CNewUILuckyItemWnd::GetLayerDepth(void)
{
    return 3.4f;
}
