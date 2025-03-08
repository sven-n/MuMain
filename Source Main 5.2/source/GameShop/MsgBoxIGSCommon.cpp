// MsgBoxIGSCommon.cpp: implementation of the CMsgBoxIGSCommon class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "MsgBoxIGSCommon.h"

#include "DSPlaySound.h"

CMsgBoxIGSCommon::CMsgBoxIGSCommon()
{
    memset(m_szTitle, 0, sizeof(m_szTitle));
    memset(m_szText, 0, sizeof(m_szText));
    m_iMiddleCount = 0;

    m_iMsgBoxWidth = IMAGE_IGS_FRAME_WIDTH;
    m_iMsgBoxHeight = IMAGE_IGS_FRAME_HEIGHT;
}

CMsgBoxIGSCommon::~CMsgBoxIGSCommon()
{
    Release();
}

bool CMsgBoxIGSCommon::Create(float fPriority)
{
    LoadImages();

    SetAddCallbackFunc();

    CNewUIMessageBoxBase::Create((IMAGE_IGS_WINDOW_WIDTH / 2) - (IMAGE_IGS_FRAME_WIDTH / 2),
        (IMAGE_IGS_WINDOW_HEIGHT / 2) - (IMAGE_IGS_FRAME_HEIGHT / 2),
        m_iMsgBoxWidth, m_iMsgBoxHeight, fPriority);

    SetButtonInfo();

    SetMsgBackOpacity();

    return true;
}

void CMsgBoxIGSCommon::Initialize(const wchar_t* pszTitle, const wchar_t* pszText)
{
    wcscpy(m_szTitle, pszTitle);

    m_iNumTextLine = ::DivideStringByPixel(&m_szText[0][0], NUM_LINE_CMB, MAX_TEXT_LENGTH, pszText, IGS_TEXT_ITEM_INFO_WIDTH, true, '#');

    if (m_iNumTextLine > IGS_NUM_TEXT_LIMIT_RENDER_MIDDLE_LINE)
    {
        m_iMiddleCount = m_iNumTextLine - IGS_NUM_TEXT_LIMIT_RENDER_MIDDLE_LINE;
    }

    m_iMsgBoxWidth = IMAGE_IGS_FRAME_WIDTH;
    m_iMsgBoxHeight = IMAGE_IGS_FRAME_HEIGHT + (m_iMiddleCount * IMAGE_IGS_LINE_HEIGHT);

    CNewUIMessageBoxBase::SetSize(m_iMsgBoxWidth, m_iMsgBoxHeight);
    SetButtonInfo();
}

void CMsgBoxIGSCommon::Release()
{
    CNewUIMessageBoxBase::Release();

    UnloadImages();
}

bool CMsgBoxIGSCommon::Update()
{
    m_BtnOk.Update();

    return true;
}

bool CMsgBoxIGSCommon::Render()
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

CALLBACK_RESULT CMsgBoxIGSCommon::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pOwnMsgBox = dynamic_cast<CMsgBoxIGSCommon*>(pOwner);

    if (pOwnMsgBox)
    {
        if (pOwnMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT CMsgBoxIGSCommon::OKButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void CMsgBoxIGSCommon::SetAddCallbackFunc()
{
    AddCallbackFunc(CMsgBoxIGSCommon::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(CMsgBoxIGSCommon::OKButtonDown, MSGBOX_EVENT_USER_COMMON_OK);
}

void CMsgBoxIGSCommon::SetButtonInfo()
{
    m_BtnOk.SetInfo(IMAGE_IGS_BUTTON, GetPos().x + (IMAGE_IGS_FRAME_WIDTH / 2) - (IMAGE_IGS_BTN_WIDTH / 2), (GetPos().y + m_iMsgBoxHeight) - (IMAGE_IGS_BTN_HEIGHT + IGS_BTN_POS_Y), IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
    m_BtnOk.MoveTextPos(0, -1);
    m_BtnOk.SetText(GlobalText[228]);
}

void CMsgBoxIGSCommon::RenderFrame()
{
    int iY = GetPos().y;

    RenderImage(IMAGE_IGS_BACK, GetPos().x, GetPos().y, m_iMsgBoxWidth, m_iMsgBoxHeight);
    RenderImage(IMAGE_IGS_UP, GetPos().x, GetPos().y, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_UP_HEIGHT);
    iY += IMAGE_IGS_UP_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(IMAGE_IGS_LEFTLINE, GetPos().x, iY, IMAGE_IGS_LINE_WIDTH, IMAGE_IGS_LINE_HEIGHT);
        RenderImage(IMAGE_IGS_RIGHTLINE, GetPos().x + IMAGE_IGS_FRAME_WIDTH - IMAGE_IGS_LINE_WIDTH, iY,
            IMAGE_IGS_LINE_WIDTH, IMAGE_IGS_LINE_HEIGHT);
        iY += IMAGE_IGS_LINE_HEIGHT;
    }
    RenderImage(IMAGE_IGS_DOWN, GetPos().x, iY, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_DOWN_HEIGHT);
}

void CMsgBoxIGSCommon::RenderTexts()
{
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);

    // Title
    g_pRenderText->RenderText(GetPos().x, GetPos().y + IGS_TEXT_TITLE_POS_Y, m_szTitle, IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);

    // Text
    int iY = IGS_TEXT_ITEM_INFO_POS_Y;
    if (m_iNumTextLine <= IGS_NUM_TEXT_LIMIT_RENDER_MIDDLE_LINE)
    {
        iY = iY + ((IGS_NUM_TEXT_LIMIT_RENDER_MIDDLE_LINE - m_iNumTextLine) * 5);
    }

    for (int j = 0; j < m_iNumTextLine; ++j)
    {
        g_pRenderText->RenderText(GetPos().x + IGS_TEXT_ITEM_INFO_POS_X, GetPos().y + iY + j * 12, m_szText[j], IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_CENTER);
    }
}

void CMsgBoxIGSCommon::RenderButtons()
{
    m_BtnOk.Render();
}

void CMsgBoxIGSCommon::LoadImages()
{
    LoadBitmap(L"Interface\\InGameShop\\Ingame_Bt03.tga", IMAGE_IGS_BUTTON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_IGS_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_IGS_DOWN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_top.tga", IMAGE_IGS_UP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_back06(L).tga", IMAGE_IGS_LEFTLINE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_back06(R).tga", IMAGE_IGS_RIGHTLINE, GL_LINEAR);
}

void CMsgBoxIGSCommon::UnloadImages()
{
    DeleteBitmap(IMAGE_IGS_BUTTON);
    DeleteBitmap(IMAGE_IGS_BACK);
    DeleteBitmap(IMAGE_IGS_DOWN);
    DeleteBitmap(IMAGE_IGS_UP);
    DeleteBitmap(IMAGE_IGS_LEFTLINE);
    DeleteBitmap(IMAGE_IGS_RIGHTLINE);
}

bool CMsgBoxIGSCommonLayout::SetLayout()
{
    CMsgBoxIGSCommon* pMsgBox = GetMsgBox();
    if (false == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

#endif KJH_ADD_INGAMESHOP_UI_SYSTEM