// MsgBoxIGSUseItemConfirm.cpp: implementation of the CMsgBoxIGSUseItemConfirm class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ZzzCharacter.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "MsgBoxIGSUseItemConfirm.h"


#include "DSPlaySound.h"
#include "MsgBoxIGSUseBuffConfirm.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMsgBoxIGSUseItemConfirm::CMsgBoxIGSUseItemConfirm()
{
    m_iMiddleCount = 7;

    for (int i = 0; i < UIMAX_TEXT_LINE; i++)
    {
        m_szDescription[i][0] = '\0';
    }

    m_iStorageSeq = 0;		// 보관함 순번
    m_iStorageItemSeq = 0;		// 보관함 상품 순번
    m_wItemCode = -1;		// 아이템 코드
    m_szItemType = '\0';		// 상품구분 (C : 캐시, P : 상품)
}

CMsgBoxIGSUseItemConfirm::~CMsgBoxIGSUseItemConfirm()
{
    Release();
}

//--------------------------------------------
// Create
bool CMsgBoxIGSUseItemConfirm::Create(float fPriority)
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
void CMsgBoxIGSUseItemConfirm::Initialize(int iStorageSeq, int iStorageItemSeq, WORD wItemCode,
    wchar_t szItemType, wchar_t* pszItemName)
{
    wchar_t szText[256] = { 0, };

    m_iStorageSeq = iStorageSeq;
    m_iStorageItemSeq = iStorageItemSeq;
    m_wItemCode = wItemCode;
    m_szItemType = szItemType;

    wcscpy(m_szItemName, pszItemName);

    // Description
    swprintf(szText, GlobalText[2923], pszItemName);
    m_iDesciptionLine = ::DivideStringByPixel(&m_szDescription[0][0], UIMAX_TEXT_LINE, MAX_TEXT_LENGTH, szText, IGS_TEXT_DIVIDE_WIDTH, false, '#');
}

//--------------------------------------------
// Release
void CMsgBoxIGSUseItemConfirm::Release()
{
    CNewUIMessageBoxBase::Release();

    UnloadImages();
}

//--------------------------------------------
// Update
bool CMsgBoxIGSUseItemConfirm::Update()
{
    m_BtnOk.Update();
    m_BtnCancel.Update();

    return true;
}

//--------------------------------------------
// Render
bool CMsgBoxIGSUseItemConfirm::Render()
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
CALLBACK_RESULT CMsgBoxIGSUseItemConfirm::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSUseItemConfirm*>(pOwner);

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

//--------------------------------------------
// OKButtonDown
CALLBACK_RESULT CMsgBoxIGSUseItemConfirm::OKButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSUseItemConfirm*>(pOwner);

    // 현재버프증 사용하려는 버프타입이 같으면 경고 메세지 처리
    BuffScriptLoader& pBuffInfo = TheBuffInfo();
    int iBuffType = pBuffInfo.GetBuffType(pOwnMsgBox->m_wItemCode);
    wchar_t szBuffName[MAX_TEXT_LENGTH] = { '\0', };
    bool bEqualBuff = Hero->Object.m_BuffMap.IsEqualBuffType(iBuffType, szBuffName);

#ifdef LEM_FIX_WARNINNGMSG_DELETE
    bEqualBuff = false;
#endif	// LEM_FIX_WARNINNGMSG_DELETE [lem_2010.8.18]

    if (bEqualBuff)
    {
        //  버프 경고창
        CMsgBoxIGSUseBuffConfirm* pMsgBox = NULL;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSUseBuffConfirmLayout), &pMsgBox);
        pMsgBox->Initialize(pOwnMsgBox->m_iStorageSeq, pOwnMsgBox->m_iStorageItemSeq,
            pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_szItemType, pOwnMsgBox->m_szItemName, szBuffName);
    }
    else
    {
        SocketClient->ToGameServer()->SendCashShopStorageItemConsumeRequest(pOwnMsgBox->m_iStorageSeq, pOwnMsgBox->m_iStorageItemSeq, pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_szItemType);
        SocketClient->ToGameServer()->SendCashShopPointInfoRequest();
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

//--------------------------------------------
// CancelButtonDown
CALLBACK_RESULT CMsgBoxIGSUseItemConfirm::CancelButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

//--------------------------------------------
// SetAddCallbackFunc
void CMsgBoxIGSUseItemConfirm::SetAddCallbackFunc()
{
    AddCallbackFunc(CMsgBoxIGSUseItemConfirm::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(CMsgBoxIGSUseItemConfirm::OKButtonDown, MSGBOX_EVENT_USER_COMMON_OK);
    AddCallbackFunc(CMsgBoxIGSUseItemConfirm::CancelButtonDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

//--------------------------------------------
// SetButtonInfo
void CMsgBoxIGSUseItemConfirm::SetButtonInfo()
{
    // 확인 버튼
    m_BtnOk.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + IGS_BTN_OK_POS_X, GetPos().y + IGS_BTN_POS_Y,
        IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
    m_BtnOk.MoveTextPos(0, -1);
    m_BtnOk.SetText(GlobalText[228]);

    // 취소 버튼
    m_BtnCancel.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + IGS_BTN_CANCEL_POS_X, GetPos().y + IGS_BTN_POS_Y,
        IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
    m_BtnCancel.MoveTextPos(0, -1);
    m_BtnCancel.SetText(GlobalText[229]);
}

//--------------------------------------------
// RenderFrame
void CMsgBoxIGSUseItemConfirm::RenderFrame()
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
void CMsgBoxIGSUseItemConfirm::RenderTexts()
{
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);

    // Title - "사용 확인"
    g_pRenderText->RenderText(GetPos().x, GetPos().y + IGS_TEXT_TITLE_Y, GlobalText[2922], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

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
    swprintf(szText, L"m_wItemCode : %d", m_wItemCode);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 30, szText, 150, 0, RT3_SORT_LEFT);
    swprintf(szText, L"m_szItemType : %c", m_szItemType);
    g_pRenderText->RenderText(GetPos().x + IMAGE_IGS_FRAME_WIDTH, GetPos().y + 40, szText, 150, 0, RT3_SORT_LEFT);
#endif // FOR_WORK
}

//--------------------------------------------
// RenderButtons
void CMsgBoxIGSUseItemConfirm::RenderButtons()
{
    m_BtnOk.Render();
    m_BtnCancel.Render();
}

//--------------------------------------------
// LoadImages
void CMsgBoxIGSUseItemConfirm::LoadImages()
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
void CMsgBoxIGSUseItemConfirm::UnloadImages()
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
bool CMsgBoxIGSUseItemConfirmLayout::SetLayout()
{
    CMsgBoxIGSUseItemConfirm* pMsgBox = GetMsgBox();
    if (false == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

#endif KJH_ADD_INGAMESHOP_UI_SYSTEM