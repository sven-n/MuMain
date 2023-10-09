// MsgBoxIGSDeleteItemConfirm.cpp: implementation of the CMsgBoxIGSDeleteItemConfirm class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "MsgBoxIGSDeleteItemConfirm.h"


#include "DSPlaySound.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMsgBoxIGSDeleteItemConfirm::CMsgBoxIGSDeleteItemConfirm()
{
    m_iMiddleCount = 4;

    m_iStorageSeq = 0;			// 보관함 순번
    m_iStorageItemSeq = 0;		// 보관함 상품 순번
    m_szItemType = '\0';		// 상품구분 (C : 캐시, P : 상품)

    for (int i = 0; i < UIMAX_TEXT_LINE; i++)
    {
        m_szDescription[i][0] = '\0';
    }

    m_iDesciptionLine = 0;
}

CMsgBoxIGSDeleteItemConfirm::~CMsgBoxIGSDeleteItemConfirm()
{
    Release();
}

//--------------------------------------------
// Create
bool CMsgBoxIGSDeleteItemConfirm::Create(float fPriority)
{
    LoadImages();

    SetAddCallbackFunc();

    CNewUIMessageBoxBase::Create((IMAGE_IGS_WINDOW_WIDTH / 2) - (IMAGE_IGS_FRAME_WIDTH / 2),
        (IMAGE_IGS_WINDOW_HEIGHT / 2) - (IMAGE_IGS_FRAME_HEIGHT / 2),
        IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT, fPriority);

    SetButtonInfo();

    SetMsgBackOpacity();

    return true;
}

//--------------------------------------------
// Initialize
void CMsgBoxIGSDeleteItemConfirm::Initialize(int iStorageSeq, int iStorageItemSeq, wchar_t szItemType)
{
    m_iStorageSeq = iStorageSeq;
    m_iStorageItemSeq = iStorageItemSeq;
    m_szItemType = szItemType;

    m_iDesciptionLine = ::DivideStringByPixel(&m_szDescription[0][0], UIMAX_TEXT_LINE, MAX_TEXT_LENGTH, GlobalText[2931], IGS_TEXT_DIVIDE_WIDTH, false, '#');
}

//--------------------------------------------
// Release
void CMsgBoxIGSDeleteItemConfirm::Release()
{
    CNewUIMessageBoxBase::Release();

    UnloadImages();
}

//--------------------------------------------
// Update
bool CMsgBoxIGSDeleteItemConfirm::Update()
{
    m_BtnDelete.Update();
    m_BtnCancel.Update();

    return true;
}

//--------------------------------------------
// Render
bool CMsgBoxIGSDeleteItemConfirm::Render()
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

//--------------------------------------------
// LButtonUp
CALLBACK_RESULT CMsgBoxIGSDeleteItemConfirm::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSDeleteItemConfirm*>(pOwner);

    if (pOwnMsgBox)
    {
        if (pOwnMsgBox->m_BtnDelete.IsMouseIn() == true)
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

//--------------------------------------------
// OKButtonDown
CALLBACK_RESULT CMsgBoxIGSDeleteItemConfirm::OKButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSDeleteItemConfirm*>(pOwner);

    SocketClient->ToGameServer()->SendCashShopDeleteStorageItemRequest(pOwnMsgBox->m_iStorageSeq, pOwnMsgBox->m_iStorageItemSeq, pOwnMsgBox->m_szItemType);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

//--------------------------------------------
// CancelButtonDown
CALLBACK_RESULT CMsgBoxIGSDeleteItemConfirm::CancelButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

//--------------------------------------------
// SetAddCallbackFunc
void CMsgBoxIGSDeleteItemConfirm::SetAddCallbackFunc()
{
    AddCallbackFunc(CMsgBoxIGSDeleteItemConfirm::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(CMsgBoxIGSDeleteItemConfirm::OKButtonDown, MSGBOX_EVENT_USER_COMMON_OK);
    AddCallbackFunc(CMsgBoxIGSDeleteItemConfirm::CancelButtonDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

//--------------------------------------------
// SetButtonInfo
void CMsgBoxIGSDeleteItemConfirm::SetButtonInfo()
{
    // 확인 버튼
    m_BtnDelete.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + IGS_BTN_DEL_POS_X, GetPos().y + IGS_BTN_POS_Y,
        IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT);
    m_BtnDelete.SetText(GlobalText[2932]);

    // 취소 버튼
    m_BtnCancel.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + IGS_BTN_CANCEL_POS_X, GetPos().y + IGS_BTN_POS_Y,
        IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT);
    m_BtnCancel.SetText(GlobalText[229]);
}

//--------------------------------------------
// RenderFrame
void CMsgBoxIGSDeleteItemConfirm::RenderFrame()
{
    int iY;

    RenderImage(IMAGE_IGS_BACK, GetPos().x, GetPos().y, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT);
    RenderImage(IMAGE_IGS_UP, GetPos().x, GetPos().y, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_UP_HEIGHT);

    iY = GetPos().y + IMAGE_IGS_UP_HEIGHT;

    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(IMAGE_IGS_LEFTLINE, GetPos().x, iY, IMAGE_IGS_LINE_WIDTH, IMAGE_IGS_LINE_HEIGHT);
        RenderImage(IMAGE_IGS_RIGHTLINE, GetPos().x + IMAGE_IGS_FRAME_WIDTH - IMAGE_IGS_LINE_WIDTH, iY,
            IMAGE_IGS_LINE_WIDTH, IMAGE_IGS_LINE_HEIGHT);
        iY += IMAGE_IGS_LINE_HEIGHT;
    }
    RenderImage(IMAGE_IGS_DOWN, GetPos().x, iY, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_DOWN_HEIGHT);
}

//--------------------------------------------
// RenderTexts
void CMsgBoxIGSDeleteItemConfirm::RenderTexts()
{
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);

    // Title - "아이템 삭제"
    g_pRenderText->RenderText(GetPos().x, GetPos().y + IGS_TEXT_TITLE_Y, GlobalText[2930], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);

    // Decription
    for (int i = 0; i < m_iDesciptionLine; ++i)
    {
        g_pRenderText->RenderText(GetPos().x + IGS_TEXT_DESCRIPTION_POS_X, GetPos().y + IGS_TEXT_DESCRIPTION_POS_Y + (i * IGS_TEXT_DESCRIPTION_INTERVAL),
            m_szDescription[i], IGS_TEXT_DESCRIPTION_WIDTH, 0, RT3_SORT_LEFT);
    }

#ifdef FOR_WORK
    wchar_t szText[256] = { 0, };
    g_pRenderText->SetTextColor(255, 0, 0, 255);
    swprintf(szText, L"m_iStorageSeq : %d", m_iStorageSeq);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 10, szText, 150, 0, RT3_SORT_LEFT);
    swprintf(szText, L"m_iStorageItemSeq : %d", m_iStorageItemSeq);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 20, szText, 150, 0, RT3_SORT_LEFT);
    swprintf(szText, L"m_szItemType : %c", m_szItemType);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 30, szText, 150, 0, RT3_SORT_LEFT);
#endif // FOR_WORK
}

//--------------------------------------------
// RenderButtons
void CMsgBoxIGSDeleteItemConfirm::RenderButtons()
{
    m_BtnDelete.Render();
    m_BtnCancel.Render();
}

//--------------------------------------------
// LoadImages
void CMsgBoxIGSDeleteItemConfirm::LoadImages()
{
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt03.tga", IMAGE_IGS_BUTTON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_IGS_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_IGS_DOWN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_top.tga", IMAGE_IGS_UP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_back06(L).tga", IMAGE_IGS_LEFTLINE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_back06(R).tga", IMAGE_IGS_RIGHTLINE, GL_LINEAR);
}

//--------------------------------------------
// UnloadImages
void CMsgBoxIGSDeleteItemConfirm::UnloadImages()
{
    DeleteBitmap(IMAGE_IGS_BUTTON);
    DeleteBitmap(IMAGE_IGS_BACK);
    DeleteBitmap(IMAGE_IGS_DOWN);
    DeleteBitmap(IMAGE_IGS_UP);
    DeleteBitmap(IMAGE_IGS_LEFTLINE);
    DeleteBitmap(IMAGE_IGS_RIGHTLINE);
}

////////////////////////////////////////////////////////////////////
// LayOut
////////////////////////////////////////////////////////////////////
bool CMsgBoxIGSDeleteItemConfirmLayout::SetLayout()
{
    CMsgBoxIGSDeleteItemConfirm* pMsgBox = GetMsgBox();
    if (false == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

#endif KJH_ADD_INGAMESHOP_UI_SYSTEM