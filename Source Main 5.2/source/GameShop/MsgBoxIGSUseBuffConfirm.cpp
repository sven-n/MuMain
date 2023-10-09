// MsgBoxIGSUseBuffConfirm.cpp: implementation of the CMsgBoxIGSUseBuffConfirm class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "MsgBoxIGSUseBuffConfirm.h"

#include "DSPlaySound.h"

CMsgBoxIGSUseBuffConfirm::CMsgBoxIGSUseBuffConfirm()
{
    m_iMiddleCount = 5;

    for (int i = 0; i < UIMAX_TEXT_LINE; i++)
    {
        m_szDescription[i][0] = '\0';
    }

    m_iStorageSeq = 0;
    m_iStorageItemSeq = 0;
    m_wItemCode = -1;
    m_szItemType = '\0';
}

CMsgBoxIGSUseBuffConfirm::~CMsgBoxIGSUseBuffConfirm()
{
    Release();
}

bool CMsgBoxIGSUseBuffConfirm::Create(float fPriority)
{
    LoadImages();

    SetAddCallbackFunc();

    CNewUIMessageBoxBase::Create((IMAGE_IGS_WINDOW_WIDTH / 2) - (IMAGE_IGS_FRAME_WIDTH / 2), (IMAGE_IGS_WINDOW_HEIGHT / 2) - (IMAGE_IGS_FRAME_HEIGHT / 2), IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT, fPriority);

    SetButtonInfo();

    SetMsgBackOpacity();

    return true;
}

void CMsgBoxIGSUseBuffConfirm::Initialize(int iStorageSeq, int iStorageItemSeq, WORD wItemCode, wchar_t szItemType, wchar_t* pszItemName, wchar_t* pszBuffName)
{
    wchar_t szText[256] = { 0, };

    m_iStorageSeq = iStorageSeq;
    m_iStorageItemSeq = iStorageItemSeq;
    m_wItemCode = wItemCode;
    m_szItemType = szItemType;
    wcscpy(m_szCurrentBuffName, pszBuffName);

    swprintf(szText, GlobalText[3047], pszItemName, pszBuffName, pszItemName);
    m_iDesciptionLine = ::DivideStringByPixel(&m_szDescription[0][0], UIMAX_TEXT_LINE, MAX_TEXT_LENGTH, szText, IGS_TEXT_DIVIDE_WIDTH, false, '#');
}

void CMsgBoxIGSUseBuffConfirm::Release()
{
    CNewUIMessageBoxBase::Release();

    UnloadImages();
}

bool CMsgBoxIGSUseBuffConfirm::Update()
{
    m_BtnOk.Update();
    m_BtnCancel.Update();

    return true;
}

bool CMsgBoxIGSUseBuffConfirm::Render()
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

CALLBACK_RESULT CMsgBoxIGSUseBuffConfirm::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSUseBuffConfirm*>(pOwner);

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

CALLBACK_RESULT CMsgBoxIGSUseBuffConfirm::OKButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSUseBuffConfirm*>(pOwner);

    SocketClient->ToGameServer()->SendCashShopStorageItemConsumeRequest(pOwnMsgBox->m_iStorageSeq, pOwnMsgBox->m_iStorageItemSeq, pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_szItemType);
    SocketClient->ToGameServer()->SendCashShopPointInfoRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT CMsgBoxIGSUseBuffConfirm::CancelButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void CMsgBoxIGSUseBuffConfirm::SetAddCallbackFunc()
{
    AddCallbackFunc(CMsgBoxIGSUseBuffConfirm::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(CMsgBoxIGSUseBuffConfirm::OKButtonDown, MSGBOX_EVENT_USER_COMMON_OK);
    AddCallbackFunc(CMsgBoxIGSUseBuffConfirm::CancelButtonDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void CMsgBoxIGSUseBuffConfirm::SetButtonInfo()
{
    m_BtnOk.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + IGS_BTN_OK_POS_X, GetPos().y + IGS_BTN_POS_Y, IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
    m_BtnOk.MoveTextPos(0, -1);
    m_BtnOk.SetText(GlobalText[228]);

    m_BtnCancel.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + IGS_BTN_CANCEL_POS_X, GetPos().y + IGS_BTN_POS_Y, IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
    m_BtnCancel.MoveTextPos(0, -1);
    m_BtnCancel.SetText(GlobalText[229]);
}

void CMsgBoxIGSUseBuffConfirm::RenderFrame()
{
    int iY;

    RenderImage(IMAGE_IGS_BACK, GetPos().x, GetPos().y, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT);
    RenderImage(IMAGE_IGS_UP, GetPos().x, GetPos().y, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_UP_HEIGHT);

    iY = GetPos().y + IMAGE_IGS_UP_HEIGHT;

    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(IMAGE_IGS_LEFTLINE, GetPos().x, iY, IMAGE_IGS_LINE_WIDTH, IMAGE_IGS_LINE_HEIGHT);
        RenderImage(IMAGE_IGS_RIGHTLINE, GetPos().x + IMAGE_IGS_FRAME_WIDTH - IMAGE_IGS_LINE_WIDTH, iY, IMAGE_IGS_LINE_WIDTH, IMAGE_IGS_LINE_HEIGHT);
        iY += IMAGE_IGS_LINE_HEIGHT;
    }

    RenderImage(IMAGE_IGS_DOWN, GetPos().x, iY, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_DOWN_HEIGHT);
}

void CMsgBoxIGSUseBuffConfirm::RenderTexts()
{
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);

    g_pRenderText->RenderText(GetPos().x, GetPos().y + IGS_TEXT_TITLE_Y, GlobalText[3046], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);

    // Decription
    for (int i = 0; i < m_iDesciptionLine; ++i)
    {
        g_pRenderText->RenderText(GetPos().x + IGS_TEXT_DESCRIPTION_POS_X, GetPos().y + IGS_TEXT_DESCRIPTION_POS_Y + (i * IGS_TEXT_DESCRIPTION_INTERVAL), m_szDescription[i], IGS_TEXT_DESCRIPTION_WIDTH, 0, RT3_SORT_LEFT);
    }

#ifdef FOR_WORK
    wchar_t szText[256] = { 0, };
    g_pRenderText->SetTextColor(255, 0, 0, 255);
    swprintf(szText, L"m_iStorageSeq : %d", m_iStorageSeq);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 10, szText, 150, 0, RT3_SORT_LEFT);
    swprintf(szText, L"m_iStorageItemSeq : %d", m_iStorageItemSeq);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 20, szText, 150, 0, RT3_SORT_LEFT);
    swprintf(szText, L"m_wItemCode : %d", m_wItemCode);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 30, szText, 150, 0, RT3_SORT_LEFT);
    swprintf(szText, L"m_szItemType : %c", m_szItemType);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 40, szText, 150, 0, RT3_SORT_LEFT);
#endif // FOR_WORK
}

void CMsgBoxIGSUseBuffConfirm::RenderButtons()
{
    m_BtnOk.Render();
    m_BtnCancel.Render();
}

void CMsgBoxIGSUseBuffConfirm::LoadImages()
{
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt03.tga", IMAGE_IGS_BUTTON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_IGS_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_IGS_DOWN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_top.tga", IMAGE_IGS_UP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_back06(L).tga", IMAGE_IGS_LEFTLINE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_back06(R).tga", IMAGE_IGS_RIGHTLINE, GL_LINEAR);
}

void CMsgBoxIGSUseBuffConfirm::UnloadImages()
{
    DeleteBitmap(IMAGE_IGS_BUTTON);
    DeleteBitmap(IMAGE_IGS_BACK);
    DeleteBitmap(IMAGE_IGS_DOWN);
    DeleteBitmap(IMAGE_IGS_UP);
    DeleteBitmap(IMAGE_IGS_LEFTLINE);
    DeleteBitmap(IMAGE_IGS_RIGHTLINE);
}

bool CMsgBoxIGSUseBuffConfirmLayout::SetLayout()
{
    CMsgBoxIGSUseBuffConfirm* pMsgBox = GetMsgBox();
    if (false == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

#endif KJH_ADD_INGAMESHOP_UI_SYSTEM