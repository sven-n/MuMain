// MsgBoxIGSBuyPackageItem.cpp: implementation of the CMsgBoxIGSBuyPackageItem class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "MsgBoxIGSBuyPackageItem.h"

#include "UsefulDef.h"
#include "DSPlaySound.h"
#include "NewUISystem.h"
#include "InGameShopSystem.h"
#include "MsgBoxIGSBuyConfirm.h"
#include "MsgBoxIGSSendGift.h"

CMsgBoxIGSBuyPackageItem::CMsgBoxIGSBuyPackageItem()
{
    m_iPackageSeq = 0;
    m_iDisplaySeq = 0;
    m_wItemCode = -1;
    m_iCashType = 0;
    m_szPackageName[0] = '\0';
    m_szPrice[0] = '\0';
    m_szPeriod[0] = '\0';

    for (int i = 0; i < UIMAX_TEXT_LINE; i++)
    {
        m_szDescription[i][0] = '\0';
    }
}

CMsgBoxIGSBuyPackageItem::~CMsgBoxIGSBuyPackageItem()
{
    Release();
}

bool CMsgBoxIGSBuyPackageItem::Create(float fPriority)
{
    LoadImages();

    SetAddCallbackFunc();

    CNewUIMessageBoxBase::Create((IMAGE_IGS_WINDOW_WIDTH / 2) - (IMAGE_IGS_FRAME_WIDTH / 2), (IMAGE_IGS_WINDOW_HEIGHT / 2) - (IMAGE_IGS_FRAME_HEIGHT / 2), IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT, fPriority);

    if (g_pNewUI3DRenderMng)
    {
        g_pNewUI3DRenderMng->Add3DRenderObj(this);
    }

    CreateListBox();
    SetButtonInfo();
    SetMsgBackOpacity();
    return true;
}

void CMsgBoxIGSBuyPackageItem::Initialize(CShopPackage* pPackage)
{
    int iProductSeq;
    int iValue = 0;
    wchar_t szText[MAX_TEXT_LENGTH] = { '\0', };

    m_iPackageSeq = pPackage->PackageProductSeq;
    m_iDisplaySeq = pPackage->ProductDisplaySeq;
    m_iCashType = pPackage->CashType;

    if (pPackage->GiftFlag == 184)
    {
        m_BtnPresent.SetEnable(true);
    }
    else
    {
        m_BtnPresent.SetEnable(false);
    }

    wcsncpy(m_szPackageName, pPackage->PackageProductName, MAX_TEXT_LENGTH);
    ConvertGold(pPackage->Price, szText);
    swprintf(m_szPrice, L"%s %s", szText, pPackage->PricUnitName);

    // Period
    pPackage->SetProductSeqFirst();
    pPackage->GetProductSeqNext(iProductSeq);

    g_InGameShopSystem->GetProductInfoFromProductSeq(iProductSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_USE_LIMIT_PERIOD, iValue, szText);

    if (iValue > 0)
    {
        swprintf(m_szPeriod, L"%d %s", iValue, szText);
    }
    else
    {
        swprintf(m_szPeriod, L"-");
    }

    m_wItemCode = _wtoi(pPackage->InGamePackageID);

    ZeroMemory(m_szDescription, sizeof(wchar_t) * UIMAX_TEXT_LINE * MAX_TEXT_LENGTH);

    g_pRenderText->SetFont(g_hFont);
    int nLine = ::DivideStringByPixel(&m_szDescription[0][0], UIMAX_TEXT_LINE, MAX_TEXT_LENGTH, pPackage->Description, IGS_LISTBOX_WIDTH, false, '#');

    for (int i = 0; i < nLine; ++i)
    {
        m_PackageInfo.AddText(m_szDescription[i]);
    }
}

void CMsgBoxIGSBuyPackageItem::Release()
{
    CNewUIMessageBoxBase::Release();

    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->Remove3DRenderObj(this);

    ReleaseListBox();

    UnloadImages();
}

bool CMsgBoxIGSBuyPackageItem::Update()
{
    m_BtnBuy.Update();
    m_BtnCancel.Update();
    m_BtnPresent.Update();
    ListBoxDoAction();
    return true;
}

bool CMsgBoxIGSBuyPackageItem::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderMsgBackColor(true);
    RenderFrame();
    RenderTexts();
    RenderButtons();
    RenderListBox();
    DisableAlphaBlend();
    return true;
}

void CMsgBoxIGSBuyPackageItem::RenderFrame()
{
    RenderImage(IMAGE_IGS_FRAME, GetPos().x, GetPos().y, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT);
}

bool CMsgBoxIGSBuyPackageItem::IsVisible() const
{
    return true;
}

void CMsgBoxIGSBuyPackageItem::SetButtonInfo()
{
    m_BtnBuy.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + IGS_BTN_BUY_POS_X, GetPos().y + IGS_BTN_POS_Y, IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
    m_BtnBuy.MoveTextPos(-1, -1);
    m_BtnBuy.SetText(GlobalText[2891]);

    m_BtnPresent.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + IGS_BTN_PRESENT_POS_X, GetPos().y + IGS_BTN_POS_Y, IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
    m_BtnPresent.MoveTextPos(-1, -1);
    m_BtnPresent.SetText(GlobalText[2892]);

    m_BtnCancel.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + IGS_BTN_CANCEL_POS_X, GetPos().y + IGS_BTN_POS_Y, IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
    m_BtnCancel.MoveTextPos(-1, -1);
    m_BtnCancel.SetText(GlobalText[229]);
}

void CMsgBoxIGSBuyPackageItem::RenderTexts()
{
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);

    g_pRenderText->RenderText(GetPos().x, GetPos().y + IGS_TEXT_TITLE_POS_Y, GlobalText[2890], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetTextColor(247, 186, 0, 255);

    g_pRenderText->RenderText(GetPos().x, GetPos().y + IGS_TEXT_NAME_POS_Y, m_szPackageName, IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);

    g_pRenderText->SetTextColor(255, 238, 161, 255);

    g_pRenderText->RenderText(GetPos().x + IGS_TEXT_PRICE_POS_X, GetPos().y + IGS_TEXT_PRICE_POX_Y, m_szPrice, IGS_TEXT_PRICE_WIDTH, 0, RT3_SORT_RIGHT);

#ifdef FOR_WORK
    wchar_t szText[256] = { '\0', };
    g_pRenderText->SetTextColor(255, 0, 0, 255);
    if (m_wItemCode == 65535)
    {
        swprintf(szText, L"아이템코드가 없습니다.");
    }
    else
    {
        swprintf(szText, L"ItemCode : %d (%d, %d)", m_wItemCode, m_wItemCode / MAX_ITEM_INDEX, m_wItemCode % MAX_ITEM_INDEX);
    }
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 10, szText, 200, 0, RT3_SORT_LEFT);
    swprintf(szText, L"Package Seq : %d", m_iPackageSeq);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 20, szText, 200, 0, RT3_SORT_LEFT);
    swprintf(szText, L"Display Seq : %d", m_iDisplaySeq);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 30, szText, 200, 0, RT3_SORT_LEFT);
    swprintf(szText, L"Price Seq : 0");
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 40, szText, 200, 0, RT3_SORT_LEFT);
    swprintf(szText, L"CashType : %d", m_iCashType);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 50, szText, 200, 0, RT3_SORT_LEFT);
#endif // FOR_WORK
}

void CMsgBoxIGSBuyPackageItem::RenderButtons()
{
    m_BtnBuy.Render();
    m_BtnPresent.Render();
    m_BtnCancel.Render();
}

void CMsgBoxIGSBuyPackageItem::Render3D()
{
    if (m_wItemCode == 65535)
        return;

    RenderItem3D(GetPos().x + IGS_3DITEM_POS_X, GetPos().y + IGS_3DITEM_POS_Y, IGS_3DITEM_WIDTH, IGS_3DITEM_HEIGHT, m_wItemCode, 0, 0, 0, true);
}

void CMsgBoxIGSBuyPackageItem::SetAddCallbackFunc()
{
    AddCallbackFunc(CMsgBoxIGSBuyPackageItem::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(CMsgBoxIGSBuyPackageItem::BuyBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    AddCallbackFunc(CMsgBoxIGSBuyPackageItem::PresentBtnDown, MSGBOX_EVENT_USER_CUSTOM_INGAMESHOP_PRESENT);
    AddCallbackFunc(CMsgBoxIGSBuyPackageItem::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

CALLBACK_RESULT CMsgBoxIGSBuyPackageItem::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSBuyPackageItem*>(pOwner);
    if (pOwnMsgBox)
    {
        if (pOwnMsgBox->m_BtnBuy.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
            return CALLBACK_BREAK;
        }

        if (pOwnMsgBox->m_BtnPresent.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_INGAMESHOP_PRESENT);
            return CALLBACK_BREAK;
        }

        if (pOwnMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }
    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT CMsgBoxIGSBuyPackageItem::BuyBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSBuyPackageItem*>(pOwner);
    CMsgBoxIGSBuyConfirm* pMsgBox = NULL;
    CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSBuyConfirmLayout), &pMsgBox);

    pMsgBox->Initialize(pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, 0, pOwnMsgBox->m_iCashType, pOwnMsgBox->m_szPackageName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT CMsgBoxIGSBuyPackageItem::PresentBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSBuyPackageItem*>(pOwner);

    CMsgBoxIGSSendGift* pMsgBox = NULL;
    CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSSendGiftLayout), &pMsgBox);

    pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, 0, pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iCashType, pOwnMsgBox->m_szPackageName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

CALLBACK_RESULT CMsgBoxIGSBuyPackageItem::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void CMsgBoxIGSBuyPackageItem::LoadImages()
{
    LoadBitmap(L"Interface\\InGameShop\\Ingame_pack_back01.tga", IMAGE_IGS_FRAME, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt03.tga", IMAGE_IGS_BUTTON, GL_LINEAR);
}

void CMsgBoxIGSBuyPackageItem::UnloadImages()
{
    DeleteBitmap(IMAGE_IGS_FRAME);
    DeleteBitmap(IMAGE_IGS_BUTTON);
}

void CMsgBoxIGSBuyPackageItem::CreateListBox()
{
    m_PackageInfo.SetPosition(GetPos().x + IGS_LISTBOX_POS_X, GetPos().y + IGS_LISTBOX_POS_Y);
    m_PackageInfo.SetLineColorRender(false);
}

void CMsgBoxIGSBuyPackageItem::RenderListBox()
{
    if (m_PackageInfo.GetLineNum() != 0)
        m_PackageInfo.Render();
}

void CMsgBoxIGSBuyPackageItem::ListBoxDoAction()
{
    m_PackageInfo.DoAction();
}

void CMsgBoxIGSBuyPackageItem::ReleaseListBox()
{
    m_PackageInfo.Clear();
}

bool CMsgBoxBuyPackageItemLayout::SetLayout()
{
    CMsgBoxIGSBuyPackageItem* pMsgBox = GetMsgBox();
    if (false == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM