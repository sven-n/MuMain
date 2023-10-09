// MsgBoxIGSBuyConfirm.cpp: implementation of the CMsgBoxIGSBuyConfirm class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "MsgBoxIGSBuyConfirm.h"
#include "DSPlaySound.h"

#include <strsafe.h>

CMsgBoxIGSBuyConfirm::CMsgBoxIGSBuyConfirm()
{
    m_iMiddleCount = 7;
    m_wItemCode = 0;
    m_iPackageSeq = 0;
    m_iDisplaySeq = 0;
    m_iPriceSeq = 0;
    m_iCashType = 0;
    m_szItemName[0] = '\0';
    m_szItemPrice[0] = '\0';
    m_szItemPeriod[0] = '\0';

    for (int i = 0; i < NUM_LINE_CMB; i++)
    {
        m_szNotice[i][0] = '\0';
    }

    m_iNumNoticeLine = 0;
}

CMsgBoxIGSBuyConfirm::~CMsgBoxIGSBuyConfirm()
{
    Release();
}

bool CMsgBoxIGSBuyConfirm::Create(float fPriority)
{
    LoadImages();
    SetAddCallbackFunc();
    CNewUIMessageBoxBase::Create((IMAGE_IGS_WINDOW_WIDTH / 2) - (IMAGE_IGS_FRAME_WIDTH / 2), (IMAGE_IGS_WINDOW_HEIGHT / 2) - (IMAGE_IGS_FRAME_HEIGHT / 2), IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT, fPriority);
    SetButtonInfo();
    SetMsgBackOpacity();
    return true;
}

void CMsgBoxIGSBuyConfirm::Initialize(WORD wItemCode, int iPackageSeq, int iDisplaySeq, int iPriceSeq, int iCashType, wchar_t* pszName, wchar_t* pszPrice, wchar_t* pszPeriod)
{
    m_wItemCode = wItemCode;
    m_iPackageSeq = iPackageSeq;
    m_iDisplaySeq = iDisplaySeq;
    m_iPriceSeq = iPriceSeq;
    m_iCashType = iCashType;

    swprintf(m_szItemName, GlobalText[3037], pszName);
    swprintf(m_szItemPrice, GlobalText[3038], pszPrice);
    swprintf(m_szItemPeriod, GlobalText[3039], pszPeriod);

    //int m_iNumNoticeLine = SeparateTextIntoLines( GlobalText[2898], txtline[0], NUM_LINE_CMB, MAX_LENGTH_CMB);
    m_iNumNoticeLine = ::DivideStringByPixel(&m_szNotice[0][0], NUM_LINE_CMB, MAX_TEXT_LENGTH, GlobalText[2898], IGS_TEXT_NOTICE_WIDTH);
}

void CMsgBoxIGSBuyConfirm::Release()
{
    CNewUIMessageBoxBase::Release();

    UnloadImages();
}

bool CMsgBoxIGSBuyConfirm::Update()
{
    m_BtnOk.Update();
    m_BtnCancel.Update();

    return true;
}

bool CMsgBoxIGSBuyConfirm::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderMsgBackColor(true);

    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();

    return true;
}

void CMsgBoxIGSBuyConfirm::SetAddCallbackFunc()
{
    AddCallbackFunc(CMsgBoxIGSBuyConfirm::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(CMsgBoxIGSBuyConfirm::OKButtonDown, MSGBOX_EVENT_USER_COMMON_OK);
    AddCallbackFunc(CMsgBoxIGSBuyConfirm::CancelButtonDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

CALLBACK_RESULT CMsgBoxIGSBuyConfirm::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSBuyConfirm*>(pOwner);
    if (pOwnMsgBox)
    {
        if (pOwnMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
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

CALLBACK_RESULT CMsgBoxIGSBuyConfirm::OKButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSBuyConfirm*>(pOwner);
    SocketClient->ToGameServer()->SendCashShopItemBuyRequest(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pOwnMsgBox->m_iPriceSeq, pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iCashType, 0);
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

CALLBACK_RESULT CMsgBoxIGSBuyConfirm::CancelButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

void CMsgBoxIGSBuyConfirm::SetButtonInfo()
{
    m_BtnOk.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + IGS_BTN_OK_POS_X, GetPos().y + IGS_BTN_POS_Y, IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
    m_BtnOk.MoveTextPos(0, -1);
    m_BtnOk.SetText(GlobalText[228]);
    m_BtnCancel.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + IGS_BTN_CANCEL_POS_X, GetPos().y + IGS_BTN_POS_Y, IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
    m_BtnCancel.MoveTextPos(0, -1);
    m_BtnCancel.SetText(GlobalText[229]);
}

void CMsgBoxIGSBuyConfirm::RenderFrame()
{
    int iY = GetPos().y;

    RenderImage(IMAGE_IGS_BACK, GetPos().x, iY, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT);
    RenderImage(IMAGE_IGS_UP, GetPos().x, iY, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_UP_HEIGHT);
    iY += IMAGE_IGS_UP_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(IMAGE_IGS_LEFTLINE, GetPos().x, iY, IMAGE_IGS_LINE_WIDTH, IMAGE_IGS_LINE_HEIGHT);
        RenderImage(IMAGE_IGS_RIGHTLINE, GetPos().x + IMAGE_IGS_FRAME_WIDTH - IMAGE_IGS_LINE_WIDTH, iY, IMAGE_IGS_LINE_WIDTH, IMAGE_IGS_LINE_HEIGHT);
        iY += IMAGE_IGS_LINE_HEIGHT;
    }
    RenderImage(IMAGE_IGS_DOWN, GetPos().x, iY, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_DOWN_HEIGHT);
    RenderImage(IMAGE_IGS_TEXTBOX, GetPos().x + IGS_TEXTBOX_POS_X, GetPos().y + IGS_TEXTBOX_POS_Y, IMAGE_IGS_TEXTBOX_WIDTH, IMAGE_IGS_TEXTBOX_HEIGHT);
}

void CMsgBoxIGSBuyConfirm::RenderTexts()
{
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + IGS_TEXT_TITLE_POS_Y, GlobalText[2896], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + IGS_TEXT_QUESTION_POS_Y, GlobalText[2897], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);
    g_pRenderText->SetTextColor(247, 186, 0, 255);
    g_pRenderText->RenderText(GetPos().x + IGS_TEXT_ITEM_INFO_POS_X, GetPos().y + IGS_TEXT_ITEM_INFO_NAME_POS_Y, m_szItemName, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);
    g_pRenderText->RenderText(GetPos().x + IGS_TEXT_ITEM_INFO_POS_X, GetPos().y + IGS_TEXT_ITEM_INFO_PRICE_POS_Y, m_szItemPrice, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);
    g_pRenderText->RenderText(GetPos().x + IGS_TEXT_ITEM_INFO_POS_X, GetPos().y + IGS_TEXT_ITEM_INFO_PERIOD_POS_Y, m_szItemPeriod, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    for (int i = 0; i < m_iNumNoticeLine; i++)
    {
        g_pRenderText->RenderText(GetPos().x, GetPos().y + IGS_TEXT_NOTICE_POS_Y + (i * 10), m_szNotice[i], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);
    }

#ifdef FOR_WORK
    wchar_t szText[256] = { 0, };
    g_pRenderText->SetTextColor(255, 0, 0, 255);
    if (m_wItemCode == 65535)
    {
        swprintf(szText, L"Bad Item index");
    }
    else
    {
        swprintf(szText, L"ItemCode : %d (%d, %d)", m_wItemCode, m_wItemCode / MAX_ITEM_INDEX, m_wItemCode % MAX_ITEM_INDEX);
    }
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 10, szText, 200, 0, RT3_SORT_LEFT);
    swprintf(szText, L"Package Seq : %d", m_iPackageSeq);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 20, szText, 100, 0, RT3_SORT_LEFT);
    swprintf(szText, L"Display Seq : %d", m_iDisplaySeq);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 30, szText, 100, 0, RT3_SORT_LEFT);
    swprintf(szText, L"Price Seq : %d", m_iPriceSeq);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 40, szText, 100, 0, RT3_SORT_LEFT);
    swprintf(szText, L"CashType : %d", m_iCashType);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 50, szText, 100, 0, RT3_SORT_LEFT);
#endif // FOR_WORK
}

void CMsgBoxIGSBuyConfirm::RenderButtons()
{
    m_BtnOk.Render();
    m_BtnCancel.Render();
}

void CMsgBoxIGSBuyConfirm::LoadImages()
{
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt03.tga", IMAGE_IGS_BUTTON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_IGS_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_IGS_DOWN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_top.tga", IMAGE_IGS_UP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_back06(L).tga", IMAGE_IGS_LEFTLINE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_back06(R).tga", IMAGE_IGS_RIGHTLINE, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\ingame_box.tga", IMAGE_IGS_TEXTBOX, GL_LINEAR);
}

void CMsgBoxIGSBuyConfirm::UnloadImages()
{
    DeleteBitmap(IMAGE_IGS_BUTTON);
    DeleteBitmap(IMAGE_IGS_BACK);
    DeleteBitmap(IMAGE_IGS_DOWN);
    DeleteBitmap(IMAGE_IGS_UP);
    DeleteBitmap(IMAGE_IGS_LEFTLINE);
    DeleteBitmap(IMAGE_IGS_RIGHTLINE);
    DeleteBitmap(IMAGE_IGS_TEXTBOX);
}

bool CMsgBoxIGSBuyConfirmLayout::SetLayout()
{
    CMsgBoxIGSBuyConfirm* pMsgBox = GetMsgBox();
    if (false == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

#endif KJH_ADD_INGAMESHOP_UI_SYSTEM