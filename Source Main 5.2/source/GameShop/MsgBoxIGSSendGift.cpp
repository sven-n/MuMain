// MsgBoxIGSSendGift.cpp: implementation of the CMsgBoxIGSSendGift class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ZzzCharacter.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "MsgBoxIGSSendGift.h"
#include "DSPlaySound.h"

#include "MsgBoxIGSCommon.h"
#include "MsgBoxIGSSendGiftConfirm.h"

CMsgBoxIGSSendGift::CMsgBoxIGSSendGift()
{
    m_iPackageSeq = 0;
    m_iDisplaySeq = 0;
    m_iPriceSeq = 0;
    m_wItemCode = -1;
    m_iCashType = 0;

    m_szID[0] = '\0';
    m_szMessage[0] = '\0';

    m_szName[0] = '\0';
    m_szPrice[0] = '\0';
    m_szPeriod[0] = '\0';

    for (int i = 0; i < NUM_LINE_CMB; i++)
    {
        m_szNotice[i][0] = '\0';
    }

    m_iNumNoticeLine = 0;
}

CMsgBoxIGSSendGift::~CMsgBoxIGSSendGift()
{
    Release();
}

bool CMsgBoxIGSSendGift::Create(float fPriority)
{
    LoadImages();
    SetAddCallbackFunc();

    CNewUIMessageBoxBase::Create((IMAGE_IGS_WINDOW_WIDTH / 2) - (IMAGE_IGS_FRAME_WIDTH / 2),
        (IMAGE_IGS_WINDOW_HEIGHT / 2) - (IMAGE_IGS_FRAME_HEIGHT / 2),
        IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT, fPriority);

    SetButtonInfo();
    InitInputBox();

    SetMsgBackOpacity();

    return true;
}

void CMsgBoxIGSSendGift::InitInputBox()
{
    m_IDInputBox.Init(g_hWnd, IGS_ID_INPUT_TEXT_WIDTH, IGS_ID_INPUT_TEXT_HEIGHT, MAX_ID_SIZE);
    m_IDInputBox.SetPosition(GetPos().x + IGS_ID_INPUT_TEXT_POS_X, GetPos().y + IGS_ID_INPUT_TEXT_POS_Y);
    m_IDInputBox.SetTextColor(255, 0, 0, 0);
    m_IDInputBox.SetBackColor(255, 255, 255, 255);
    m_IDInputBox.SetFont(g_hFont);
    m_IDInputBox.SetTextLimit(MAX_ID_SIZE);
    m_IDInputBox.SetState(UISTATE_NORMAL);

    m_MessageInputBox.SetMultiline(TRUE);
    m_MessageInputBox.Init(g_hWnd, IGS_MESSAGE_INPUT_TEXT_WIDTH, IGS_MESSAGE_INPUT_TEXT_HEIGHT, IGS_MESSAGE_INPUT_TEXT_LINE_HEIGHT);
    m_MessageInputBox.SetPosition(GetPos().x + IGS_MESSAGE_INPUT_TEXT_POS_X, GetPos().y + IGS_MESSAGE_INPUT_TEXT_POS_Y);
    m_MessageInputBox.SetUseScrollbar(FALSE);
    m_MessageInputBox.SetTextLimit(MAX_GIFT_MESSAGE_SIZE);
    m_MessageInputBox.SetFont(g_hFont);
    m_MessageInputBox.SetBackColor(0, 0, 0, 0);
    m_MessageInputBox.SetState(UISTATE_NORMAL);
    m_MessageInputBox.SetTextColor(255, 0, 0, 0);

    m_IDInputBox.GiveFocus();
}

void CMsgBoxIGSSendGift::Initialize(int iPackageSeq, int iDisplaySeq, int iPriceSeq, DWORD wItemCode, int iCashType, wchar_t* pszName, wchar_t* pszPrice, wchar_t* pszPeriod)
{
    m_iPackageSeq = iPackageSeq;
    m_iDisplaySeq = iDisplaySeq;
    m_iPriceSeq = iPriceSeq;
    m_wItemCode = wItemCode;
    m_iCashType = iCashType;

    swprintf(m_szName, GlobalText[3037], pszName);
    swprintf(m_szPrice, GlobalText[3038], pszPrice);
    swprintf(m_szPeriod, GlobalText[3039], pszPeriod);

    m_iNumNoticeLine = ::DivideStringByPixel(&m_szNotice[0][0], NUM_LINE_CMB, MAX_TEXT_LENGTH, GlobalText[2920], IGS_TEXT_NOTICE_WIDTH);
}

void CMsgBoxIGSSendGift::Release()
{
    CNewUIMessageBoxBase::Release();
    UnloadImages();
}

bool CMsgBoxIGSSendGift::Update()
{
    m_BtnOk.Update();
    m_BtnCancel.Update();

    m_IDInputBox.DoAction();
    m_MessageInputBox.DoAction();

    if (m_IDInputBox.HaveFocus() == TRUE)
        m_IDInputBox.GetText(m_szID, MAX_ID_SIZE + 1);

    if (m_MessageInputBox.HaveFocus() == TRUE)
        m_MessageInputBox.GetText(m_szMessage, MAX_GIFT_MESSAGE_SIZE);

    if (SEASON3B::IsPress(VK_TAB) == true)
    {
        ChangeInputBoxFocus();
    }

    return true;
}

bool CMsgBoxIGSSendGift::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderMsgBackColor(true);

    RenderFrame();
    RenderTexts();
    RenderButtons();

    m_IDInputBox.Render();
    m_MessageInputBox.Render();

    DisableAlphaBlend();
    return true;
}

void CMsgBoxIGSSendGift::SetAddCallbackFunc()
{
    AddCallbackFunc(CMsgBoxIGSSendGift::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(CMsgBoxIGSSendGift::OKButtonDown, MSGBOX_EVENT_USER_COMMON_OK);
    AddCallbackFunc(CMsgBoxIGSSendGift::CancelButtonDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

CALLBACK_RESULT CMsgBoxIGSSendGift::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSSendGift*>(pOwner);

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

CALLBACK_RESULT CMsgBoxIGSSendGift::OKButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSSendGift*>(pOwner);

    if (pOwnMsgBox->m_szID[0] == '\0')
    {
        CMsgBoxIGSCommon* pMsgBox = NULL;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[3028], GlobalText[3031]);
    }
    else if (wcscmp(pOwnMsgBox->m_szID, Hero->ID) == 0)
    {
        CMsgBoxIGSCommon* pMsgBox = NULL;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[3028], GlobalText[3032]);
    }
    else
    {
        CMsgBoxIGSSendGiftConfirm* pMsgBox = NULL;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSSendGiftConfirmLayout), &pMsgBox);
        pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pOwnMsgBox->m_iPriceSeq, pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iCashType, pOwnMsgBox->m_szID, pOwnMsgBox->m_szMessage, pOwnMsgBox->m_szName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

CALLBACK_RESULT CMsgBoxIGSSendGift::CancelButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

void CMsgBoxIGSSendGift::SetButtonInfo()
{
    m_BtnOk.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + IGS_BTN_OK_POS_X, GetPos().y + IGS_BTN_POS_Y, IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
    m_BtnOk.MoveTextPos(0, -1);
    m_BtnOk.SetText(GlobalText[228]);

    m_BtnCancel.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + IGS_BTN_CANCEL_POS_X, GetPos().y + IGS_BTN_POS_Y, IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
    m_BtnCancel.MoveTextPos(0, -1);
    m_BtnCancel.SetText(GlobalText[229]);
}

void CMsgBoxIGSSendGift::RenderFrame()
{
    RenderImage(IMAGE_IGS_FRAME, GetPos().x, GetPos().y, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT);
    RenderImage(IMAGE_IGS_DECO, GetPos().x + IMAGE_IGS_DECO_POS_X, GetPos().y + IMAGE_IGS_DECO_POS_Y, IMAGE_IGS_DECO_WIDTH, IMAGE_IGS_DECO_HEIGHT);
    RenderImage(IMAGE_IGS_INPUTTEXT, GetPos().x + IMAGE_IGS_ID_INPUT_BOX_POS_X, GetPos().y + IMAGE_IGS_ID_INPUT_BOX_POS_Y, IMAGE_IGS_ID_INPUT_BOX_WIDTH, IMAGE_IGS_ID_INPUT_BOX_HEIGHT);
}

void CMsgBoxIGSSendGift::RenderTexts()
{
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);

    g_pRenderText->RenderText(GetPos().x, GetPos().y + IGS_TEXT_TITLE_POS_Y, GlobalText[2916], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->RenderText(GetPos().x + IGS_TEXT_ID_TITLE_POS_X, GetPos().y + IGS_TEXT_ID_TITLE_POS_Y, GlobalText[2918], IGS_TEXT_ID_TITLE_WIDTH, 0, RT3_SORT_LEFT);

    g_pRenderText->SetTextColor(0, 0, 0, 255);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + IGS_TEXT_MESSAGE_TITLE_POS_Y, GlobalText[2919], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);

    g_pRenderText->SetTextColor(247, 186, 0, 255);
    g_pRenderText->RenderText(GetPos().x + IGS_TEXT_ITEM_INFO_POS_X, GetPos().y + IGS_TEXT_ITEM_INFO_NAME_POS_Y, m_szName, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);
    g_pRenderText->RenderText(GetPos().x + IGS_TEXT_ITEM_INFO_POS_X, GetPos().y + IGS_TEXT_ITEM_INFO_PRICE_POS_Y, m_szPrice, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);
    g_pRenderText->RenderText(GetPos().x + IGS_TEXT_ITEM_INFO_POS_X, GetPos().y + IGS_TEXT_ITEM_INFO_PERIOD_POS_Y, m_szPeriod, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);

    g_pRenderText->SetTextColor(255, 255, 255, 255);
    for (int i = 0; i < m_iNumNoticeLine; i++)
    {
        g_pRenderText->RenderText(GetPos().x, GetPos().y + IGS_TEXT_NOTICE_POS_Y + i * 10, m_szNotice[i], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);
    }

#ifdef FOR_WORK
    wchar_t szText[256] = { 0, };
    g_pRenderText->SetTextColor(255, 0, 0, 255);
    swprintf(szText, L"Package Seq : %d", m_iPackageSeq);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 10, szText, 200, 0, RT3_SORT_LEFT);
    swprintf(szText, L"Display Seq : %d", m_iDisplaySeq);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 20, szText, 200, 0, RT3_SORT_LEFT);
    swprintf(szText, L"Price Seq : %d", m_iPriceSeq);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 30, szText, 200, 0, RT3_SORT_LEFT);
    swprintf(szText, L"ItemCode : %d", m_wItemCode);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 40, szText, 200, 0, RT3_SORT_LEFT);
    swprintf(szText, L"CashType : %d", m_iCashType);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 50, szText, 200, 0, RT3_SORT_LEFT);
#endif // FOR_WORK
}

void CMsgBoxIGSSendGift::RenderButtons()
{
    m_BtnOk.Render();
    m_BtnCancel.Render();
}

void CMsgBoxIGSSendGift::ChangeInputBoxFocus()
{
    if (m_IDInputBox.HaveFocus() == TRUE)
    {
        m_MessageInputBox.GiveFocus();
    }
    else if (m_MessageInputBox.HaveFocus() == TRUE)
    {
        m_IDInputBox.GiveFocus();
    }
    else
    {
        m_IDInputBox.GiveFocus();
    }
}

void CMsgBoxIGSSendGift::LoadImages()
{
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt03.tga", IMAGE_IGS_BUTTON, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\ingame_gift_back01.tga", IMAGE_IGS_FRAME, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\ingame_gift_icon.tga", IMAGE_IGS_DECO, GL_LINEAR);
    LoadBitmap(L"Interface\\InGameShop\\ingame_gift_namebox.tga", IMAGE_IGS_INPUTTEXT, GL_LINEAR);
}

void CMsgBoxIGSSendGift::UnloadImages()
{
    DeleteBitmap(IMAGE_IGS_BUTTON);
    DeleteBitmap(IMAGE_IGS_FRAME);
    DeleteBitmap(IMAGE_IGS_DECO);
    DeleteBitmap(IMAGE_IGS_INPUTTEXT);
}

bool CMsgBoxIGSSendGiftLayout::SetLayout()
{
    CMsgBoxIGSSendGift* pMsgBox = GetMsgBox();
    if (false == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

#endif KJH_ADD_INGAMESHOP_UI_SYSTEM