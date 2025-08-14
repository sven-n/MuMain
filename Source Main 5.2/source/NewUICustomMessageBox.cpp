#include "stdafx.h"
#include "NewUICustomMessageBox.h"
#include "DSPlaySound.h"
#include "UIControls.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"

#include "PersonalShopTitleImp.h"
#include "CComGem.h"
#include "MatchEvent.h"
#include "MixMgr.h"
#include "npcBreeder.h"
#include "ZzzOpenData.h"
#include "DuelMgr.h"
#include "NewUISystem.h"
#include "w_CursedTemple.h"

extern int DeleteIndex;
extern int AppointStatus;
extern bool LogOut;

char AppointType;

#define SUBGUILDMASTER	64
#define BATTLEMASTER	32

using namespace SEASON3B;

SEASON3B::CNewUITextInputMsgBox::CNewUITextInputMsgBox()
{
    m_pInputBox = NULL;
}

SEASON3B::CNewUITextInputMsgBox::~CNewUITextInputMsgBox()
{
    Release();
}

bool SEASON3B::CNewUITextInputMsgBox::Create(DWORD dwMsgBoxType, DWORD dwInputType, int iInputBoxWidth, int iInputBoxHeight, int iTextLimit, bool bIsPassword)
{
    m_dwMsgBoxType = dwMsgBoxType;
    m_dwInputType = dwInputType;

    AddCallbackFunc(SEASON3B::CNewUITextInputMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);

    int x, y, width, height;

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_MIDDLE_HEIGHT + MSGBOX_BOTTOM_HEIGHT;
    if (CNewUIMessageBoxBase::Create(x, y, width, height) == false)
    {
        return false;
    }

    m_pInputBox = new CUITextInputBox;

    if (m_pInputBox)
    {
        m_pInputBox->Init(g_hWnd, iInputBoxWidth, iInputBoxHeight, iTextLimit, bIsPassword);
        x = GetPos().x + (MSGBOX_WIDTH / 2) - (iInputBoxWidth / 2);
        y = GetPos().y + MSGBOX_TOP_HEIGHT - INPUTBOX_TOP_BLANK;
        m_pInputBox->SetPosition(x, y);
        m_pInputBox->SetTextColor(255, 255, 230, 210);
        m_pInputBox->SetBackColor(255, 0, 0, 0);
        m_pInputBox->SetFont(g_hFont);
        m_pInputBox->SetState(UISTATE_NORMAL);
        m_pInputBox->GiveFocus();
    }

    SetButtonInfo();

    return true;
}

void SEASON3B::CNewUITextInputMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();

    auto vi = m_MsgTextList.begin();
    for (; vi != m_MsgTextList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgTextList.clear();

    SAFE_DELETE(m_pInputBox);

    g_MessageBox->SetRelatedWnd(g_hWnd);
    SetFocus(g_hWnd);
}

void SEASON3B::CNewUITextInputMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    switch (m_dwMsgBoxType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        x = GetPos().x + (GetSize().cx / 2) - (MSGBOX_BTN_WIDTH / 2);
        y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
        width = MSGBOX_BTN_WIDTH;
        height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        x = GetPos().x + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
        y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
        width = MSGBOX_BTN_WIDTH;
        height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

        x = GetPos().x + (GetSize().cx / 2) + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
        y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
        width = MSGBOX_BTN_WIDTH;
        height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        break;
    }
}

CALLBACK_RESULT SEASON3B::CNewUITextInputMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUITextInputMsgBox*>(pOwner);
    if (pMsgBox)
    {
        switch (pMsgBox->GetMsgBoxType())
        {
        case MSGBOX_COMMON_TYPE_OK:
            if (pMsgBox->m_BtnOk.IsMouseIn() == true)
            {
                g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
                return CALLBACK_BREAK;
            }
            break;
        case MSGBOX_COMMON_TYPE_OKCANCEL:
            if (pMsgBox->m_BtnOk.IsMouseIn() == true)
            {
                g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
                return CALLBACK_BREAK;
            }
            if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
            {
                g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
                return CALLBACK_BREAK;
            }
            break;
        }
    }

    return CALLBACK_CONTINUE;
}

void SEASON3B::CNewUITextInputMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    int iOrigSize = m_MsgTextList.size();
    int iLine = SeparateText(strMsg, dwColor, byFontType);
    int iSize = m_MsgTextList.size();

    if (iSize > 1)
    {
        float height = GetSize().cy;

        if (iOrigSize < 1)
        {
            iLine = iLine + iOrigSize - 1;
        }

        height += (MSGBOX_MIDDLE_HEIGHT * iLine);
        SetSize(GetSize().cx, height);
        AddButtonBlank(iLine);

        if (m_pInputBox)
        {
            m_pInputBox->SetPosition(m_pInputBox->GetPosition_x(), m_pInputBox->GetPosition_y() + (iLine * MSGBOX_MIDDLE_HEIGHT));
        }
    }
}

void SEASON3B::CNewUITextInputMsgBox::AddButtonBlank(int iAddLine)
{
    switch (m_dwMsgBoxType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        m_BtnOk.AddBlank(iAddLine * MSGBOX_MIDDLE_HEIGHT);
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        m_BtnOk.AddBlank(iAddLine * MSGBOX_MIDDLE_HEIGHT);
        m_BtnCancel.AddBlank(iAddLine * MSGBOX_MIDDLE_HEIGHT);
        break;
    }
}

int SEASON3B::CNewUITextInputMsgBox::SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    

    SIZE TextSize;
    size_t TextExtentWidth;
    int iLine = 0;

    GetTextExtentPoint32(g_pRenderText->GetFontDC(), strMsg.c_str(), strMsg.size(), &TextSize);
    TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);

    if (TextExtentWidth <= MSGBOX_TEXT_MAXWIDTH)
    {
        auto* pMsg = new MSGBOX_TEXTDATA;
        pMsg->strMsg = strMsg;
        pMsg->dwColor = dwColor;
        pMsg->byFontType = byFontType;
        m_MsgTextList.push_back(pMsg);

        iLine = 1;
        return iLine;
    }

    type_string strCutText, strRemainText;
    strRemainText = strMsg;

    bool bLoop = true;
    while (bLoop)
    {
        int prev_offset = 0;
        for (int cur_offset = 0; cur_offset < (int)strRemainText.size(); )
        {
            prev_offset = cur_offset;
            size_t offset = _mbclen((const unsigned char*)(strRemainText.c_str() + cur_offset));
            cur_offset += offset;

            type_string strTemp(strRemainText, 0, cur_offset/* size */);
            GetTextExtentPoint32(g_pRenderText->GetFontDC(), strTemp.c_str(), strTemp.size(), &TextSize);
            TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);

            if (TextExtentWidth > MSGBOX_TEXT_MAXWIDTH && cur_offset != 0)
            {
                strCutText = type_string(strRemainText, 0, prev_offset/* size */);
                strRemainText = type_string(strRemainText, prev_offset, strRemainText.size() - prev_offset/* size */);

                auto* pMsg = new MSGBOX_TEXTDATA;
                pMsg->strMsg = strCutText;
                pMsg->dwColor = dwColor;
                pMsg->byFontType = byFontType;
                m_MsgTextList.push_back(pMsg);
                iLine++;

                GetTextExtentPoint32(g_pRenderText->GetFontDC(), strRemainText.c_str(), strRemainText.size(), &TextSize);
                TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);

                if (TextExtentWidth <= MSGBOX_TEXT_MAXWIDTH)
                {
                    auto* pMsg = new MSGBOX_TEXTDATA;
                    pMsg->strMsg = strRemainText;
                    pMsg->dwColor = dwColor;
                    pMsg->byFontType = byFontType;
                    m_MsgTextList.push_back(pMsg);
                    iLine++;

                    bLoop = false;
                    break;
                }

                break;
            }
        }
    }

    return iLine;
}

DWORD SEASON3B::CNewUITextInputMsgBox::GetMsgBoxType()
{
    return m_dwMsgBoxType;
}

bool SEASON3B::CNewUITextInputMsgBox::Update()
{
    switch (m_dwMsgBoxType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        m_BtnOk.Update();
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        m_BtnOk.Update();
        m_BtnCancel.Update();
        break;
    }

    if (m_pInputBox)
    {
        m_pInputBox->DoAction();

        if (m_pInputBox->HaveFocus() && g_MessageBox->GetRelatedWnd() != m_pInputBox->GetHandle())
        {
            g_MessageBox->SetRelatedWnd(m_pInputBox->GetHandle());
        }
        if (false == m_pInputBox->HaveFocus() && g_MessageBox->GetRelatedWnd() != g_hWnd)
        {
            g_MessageBox->SetRelatedWnd(g_hWnd);
        }
    }

    return true;
}

bool SEASON3B::CNewUITextInputMsgBox::Render()
{
    float x, y, width, height;

    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    int iCount = m_MsgTextList.size();
    for (int i = 0; i < iCount; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);

    RenderTexts();

    if (m_pInputBox)
    {
        m_pInputBox->Render();
    }

    RenderButtons();

    DisableAlphaBlend();
    return true;
}

void SEASON3B::CNewUITextInputMsgBox::RenderTexts()
{
    
    

    float x, y;
    x = GetPos().x; y = GetPos().y + MSGBOX_TEXT_TOP_BLANK;
    auto vi = m_MsgTextList.begin();
    for (; vi != m_MsgTextList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        SIZE TextSize;
        size_t TextExtentWidth, TextExtentHeight;

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), (*vi)->strMsg.c_str(), (*vi)->strMsg.size(), &TextSize);
        TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);
        TextExtentHeight = (size_t)(TextSize.cy / g_fScreenRate_y);

        x = GetPos().x + (MSGBOX_WIDTH / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void SEASON3B::CNewUITextInputMsgBox::RenderButtons()
{
    switch (m_dwMsgBoxType)
    {
    case MSGBOX_COMMON_TYPE_OK:
        m_BtnOk.Render();
        break;
    case MSGBOX_COMMON_TYPE_OKCANCEL:
        m_BtnOk.Render();
        m_BtnCancel.Render();
        break;
    }
}

void SEASON3B::CNewUITextInputMsgBox::GetInputBoxText(wchar_t* strText)
{
    if (m_pInputBox)
    {
        m_pInputBox->GetText(strText);
    }
}

void SEASON3B::CNewUITextInputMsgBox::SetInputBoxOption(int iOption)
{
    if (m_pInputBox)
    {
        m_pInputBox->SetOption(iOption);
    }
}

void SEASON3B::CNewUITextInputMsgBox::SetInputBoxPosition(int x, int y)
{
    if (m_pInputBox)
    {
        m_pInputBox->SetPosition(x, y);
    }
}

void SEASON3B::CNewUITextInputMsgBox::SetInputBoxSize(int width, int height)
{
    if (m_pInputBox)
    {
        m_pInputBox->SetSize(width, height);
    }
}

void SEASON3B::CNewUIKeyPadButton::Render()
{
    if (GetEventState() == EVENT_BTN_HOVER)
    {
        RenderImage(BITMAP_INVENTORY + 17, GetPosX(), GetPosY(), GetWidth(), GetHeight());
    }
    else if (GetEventState() == EVENT_BTN_DOWN)
    {
        RenderImage(BITMAP_INVENTORY + 18, GetPosX(), GetPosY(), GetWidth(), GetHeight());
    }
    else
    {
        glColor3f(0.80f, 0.80f, 0.80f);
        RenderImage(BITMAP_INVENTORY + 17, GetPosX(), GetPosY(), GetWidth(), GetHeight());
        glColor3f(1.f, 1.f, 1.f);
    }
}

void SEASON3B::CNewUIDeleteKeyPadButton::Render()
{
    if (GetEventState() == EVENT_BTN_HOVER)
    {
        RenderImage(BITMAP_INTERFACE + 25, GetPosX(), GetPosY(), GetWidth(), GetHeight());
    }
    else if (GetEventState() == EVENT_BTN_DOWN)
    {
        RenderImage(BITMAP_INTERFACE + 26, GetPosX(), GetPosY(), GetWidth(), GetHeight());
    }
    else
    {
        RenderImage(BITMAP_INTERFACE + 24, GetPosX(), GetPosY(), GetWidth(), GetHeight());
    }
}

//////////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIKeyPadMsgBox::CNewUIKeyPadMsgBox()
{
    ClearInput();

    ZeroMemory(m_strCheckKeyPadInput, sizeof(m_strCheckKeyPadInput));

    m_iInputLimit = 0;
}

SEASON3B::CNewUIKeyPadMsgBox::~CNewUIKeyPadMsgBox()
{
}

bool SEASON3B::CNewUIKeyPadMsgBox::Create(DWORD dwType, int iInputLimit)
{
    m_iInputLimit = iInputLimit;

    AddCallbackFunc(SEASON3B::CNewUIKeyPadMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CNewUIKeyPadMsgBox::KeyPadBtnDown, MSGBOX_EVENT_USER_CUSTOM_KEYPAD_INPUT);
    AddCallbackFunc(SEASON3B::CNewUIKeyPadMsgBox::DeleteBtnDown, MSGBOX_EVENT_USER_CUSTOM_KEYPAD_DELETE);

    int x, y, width, height;

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_MIDDLE_HEIGHT * MSGBOX_MIDDLE_FRAME_NUM + MSGBOX_BOTTOM_HEIGHT;
    if (CNewUIMessageBoxBase::Create(x, y, width, height) == false)
    {
        return false;
    }

    m_dwType = dwType;

    for (int i = 0; i < MAX_KEYPADINPUT; ++i)
    {
        m_iKeyPadMapping[i] = i;
    }

    for (int i = 0; i < 20; ++i)
    {
        int iFirst = rand() % 10;
        int iSecond = rand() % 10;
        if (iFirst != iSecond)
        {
            swaps(m_iKeyPadMapping[iFirst], m_iKeyPadMapping[iSecond]);
        }
    }

    ZeroMemory(m_strKeyPadInput, sizeof(m_strKeyPadInput));

    SetButtonInfo();

    return true;
}

void SEASON3B::CNewUIKeyPadMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();

    auto vi = m_MsgTextList.begin();
    for (; vi != m_MsgTextList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgTextList.clear();
}

CALLBACK_RESULT SEASON3B::CNewUIKeyPadMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUIKeyPadMsgBox*>(pOwner);
    if (pMsgBox)
    {
        for (int i = 0; i < MAX_KEYPADINPUT; ++i)
        {
            if (pMsgBox->m_BtnKeyPad[i].IsMouseIn() == true)
            {
                leaf::xstreambuf buf;
                int iKeyPadInputNumber = 0;
                iKeyPadInputNumber = pMsgBox->m_iKeyPadMapping[i];
                buf << iKeyPadInputNumber;
                g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_KEYPAD_INPUT, buf);
                return CALLBACK_BREAK;
            }
        }

        if (pMsgBox->m_BtnDeleteKeyPad.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_KEYPAD_DELETE);
            return CALLBACK_BREAK;
        }

        if (pMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CNewUIKeyPadMsgBox::KeyPadBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUIKeyPadMsgBox*>(pOwner);

    if (pMsgBox)
    {
        if (pMsgBox->GetInputSize() < pMsgBox->GetInputLimit())
        {
            int* pInputNumber = (int*)xParam.data();
            pMsgBox->KeyPadInput(*pInputNumber);
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CNewUIKeyPadMsgBox::DeleteBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUIKeyPadMsgBox*>(pOwner);

    if (pMsgBox)
    {
        pMsgBox->DeleteKeyPadInput();
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CNewUIKeyPadMsgBox::Close(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    return CALLBACK_CONTINUE;
}

void SEASON3B::CNewUIKeyPadMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    auto* pMsg = new MSGBOX_TEXTDATA;
    pMsg->strMsg = strMsg;
    pMsg->dwColor = dwColor;
    pMsg->byFontType = byFontType;
    m_MsgTextList.push_back(pMsg);
}

int SEASON3B::CNewUIKeyPadMsgBox::GetInputLimit()
{
    return m_iInputLimit;
}

int SEASON3B::CNewUIKeyPadMsgBox::GetInputSize()
{
    return wcslen(m_strKeyPadInput);
}

void SEASON3B::CNewUIKeyPadMsgBox::ClearInput()
{
    ZeroMemory(m_strKeyPadInput, sizeof(m_strKeyPadInput));
}

const wchar_t* SEASON3B::CNewUIKeyPadMsgBox::GetInputText()
{
    return m_strKeyPadInput;
}

void SEASON3B::CNewUIKeyPadMsgBox::SetCheckInputText(const wchar_t* strInput)
{
    memcpy(m_strCheckKeyPadInput, strInput, m_iInputLimit);
}

bool SEASON3B::CNewUIKeyPadMsgBox::IsCheckInput()
{
    return (0 == memcmp(m_strCheckKeyPadInput, m_strKeyPadInput, m_iInputLimit));
}

void SEASON3B::CNewUIKeyPadMsgBox::SetStoragePassword(WORD wPassword)
{
    m_wStoragePassword = wPassword;
}

WORD SEASON3B::CNewUIKeyPadMsgBox::GetStoragePassword()
{
    return m_wStoragePassword;
}

bool SEASON3B::CNewUIKeyPadMsgBox::IsAllSameNumber()
{
    for (int i = 0; i < m_iInputLimit - 1; ++i)
    {
        if (m_strKeyPadInput[i] != m_strKeyPadInput[i + 1])
        {
            return false;
        }
    }

    return true;
}

void SEASON3B::CNewUIKeyPadMsgBox::KeyPadInput(int iInput)
{
    wchar_t strInput[4] = { 0, };
    swprintf(strInput, L"%d", iInput);
    wcscat(m_strKeyPadInput, strInput);
}

void SEASON3B::CNewUIKeyPadMsgBox::DeleteKeyPadInput()
{
    int iSize = wcslen(m_strKeyPadInput);
    if (iSize > 0)
    {
        m_strKeyPadInput[iSize - 1] = '\0';
    }
}

void SEASON3B::CNewUIKeyPadMsgBox::SetButtonInfo()
{
    int x, y, width, height;

    width = KEYPAD_WIDTH; height = KEYPAD_HEIGHT;
    x = GetPos().x + (GetSize().cx / 2) - ((width + 5) * 5) / 2;
    y = GetPos().y + 80;
    for (int i = 0; i < MAX_KEYPADINPUT; ++i)
    {
        int xPos = i % 5;
        int yPos = i / 5;

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnKeyPad[i].SetInfo(0, x + ((width + 5) * xPos), y + ((height + 5) * yPos), width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
        m_BtnKeyPad[i].SetInfo(0, x + ((width + 5) * xPos), y + ((height + 5) * yPos), width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
    }

    y += (height + 5) * 2;

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnDeleteKeyPad.SetInfo(0, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnDeleteKeyPad.SetInfo(0, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    width = MSGBOX_BTN_WIDTH;
    height = MSGBOX_BTN_HEIGHT;
    x = GetPos().x + (((GetSize().cx / 2) - width) / 2);
    y = GetPos().y + GetSize().cy - (height + MSGBOX_BTN_BOTTOM_BLANK);
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    width = MSGBOX_BTN_WIDTH;
    height = MSGBOX_BTN_HEIGHT;
    x = GetPos().x + (GetSize().cx / 2) + (((GetSize().cx / 2) - width) / 2);
    y = GetPos().y + GetSize().cy - (height + MSGBOX_BTN_BOTTOM_BLANK);
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
}

bool SEASON3B::CNewUIKeyPadMsgBox::Update()
{
    for (int i = 0; i < MAX_KEYPADINPUT; ++i)
    {
        m_BtnKeyPad[i].Update();
    }

    m_BtnDeleteKeyPad.Update();

    m_BtnOk.Update();

    m_BtnCancel.Update();

    return true;
}

bool SEASON3B::CNewUIKeyPadMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderFrame();
    RenderTexts();
    RenderKeyPadInput();
    RenderButtons();

    DisableAlphaBlend();
    return true;
}

void SEASON3B::CNewUIKeyPadMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < MSGBOX_MIDDLE_FRAME_NUM; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CNewUIKeyPadMsgBox::RenderKeyPadInput()
{
    float x, y, width, height;
    glColor3f(0.3f, 0.3f, 0.3f);
    width = 10.f * m_iInputLimit + 12.f;
    height = 18.f;
    x = GetPos().x + (MSGBOX_WIDTH / 2) - (width / 2);
    y = GetPos().y + 55;

    RenderBitmap(BITMAP_INTERFACE + 23, x, y, width, height, 0.f, 0.f, 40 / 64.f, 18 / 32.f);
    glColor3f(1.0f, 1.0f, 1.0f);

    std::wstring strInput = L"";
    for (int i = 0; i < GetInputSize(); ++i)
    {
        strInput += L"*";
    }
    x += 5;
    y += 0;
    g_pRenderText->SetFont(g_hFontBig);
    g_pRenderText->SetTextColor(128, 128, 196, 255);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->RenderText(x, y, strInput.c_str());
}

void SEASON3B::CNewUIKeyPadMsgBox::RenderTexts()
{
    
    

    float x, y;
    x = GetPos().x; y = GetPos().y + 25;
    auto vi = m_MsgTextList.begin();
    for (; vi != m_MsgTextList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        SIZE TextSize;
        size_t TextExtentWidth, TextExtentHeight;

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), (*vi)->strMsg.c_str(), (*vi)->strMsg.size(), &TextSize);
        TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);
        TextExtentHeight = (size_t)(TextSize.cy / g_fScreenRate_y);

        x = GetPos().x + (MSGBOX_WIDTH / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void SEASON3B::CNewUIKeyPadMsgBox::RenderButtons()
{
    for (int i = 0; i < MAX_KEYPADINPUT; ++i)
    {
        m_BtnKeyPad[i].Render();
        SEASON3B::RenderNumber(m_BtnKeyPad[i].GetPosX() + 15, m_BtnKeyPad[i].GetPosY() + 5, m_iKeyPadMapping[i], 1.5f);
    }

    m_BtnDeleteKeyPad.Render();
    m_BtnOk.Render();
    m_BtnCancel.Render();
}

//////////////////////////////////////////////////////////////////////////

SEASON3B::CUseFruitCheckMsgBox::CUseFruitCheckMsgBox()
{
    ZeroMemory(&m_Item, sizeof(m_Item));
}

SEASON3B::CUseFruitCheckMsgBox::~CUseFruitCheckMsgBox()
{
    Release();
}

bool SEASON3B::CUseFruitCheckMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_MIDDLE_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    if (g_pNewUI3DRenderMng)
    {
        g_pNewUI3DRenderMng->Add3DRenderObj(this);
    }

    ITEM* pItem = g_pMyInventory->GetStandbyItem();
    if (pItem == NULL)
    {
        return false;
    }

    Set3DItem(pItem);

    wchar_t strName[50] = { 0, };
    if (pItem->Type == ITEM_FRUITS)
    {
        switch (pItem->Level)
        {
        case 0:
            swprintf(strName, L"%s", GlobalText[168]);
            break;
        case 1:
            swprintf(strName, L"%s", GlobalText[169]);
            break;
        case 2:
            swprintf(strName, L"%s", GlobalText[167]);
            break;
        case 3:
            swprintf(strName, L"%s", GlobalText[166]);
            break;
        case 4:
            swprintf(strName, L"%s", GlobalText[1900]);
            break;
        }
    }

    wchar_t strText[128] = { 0, };
    swprintf(strText, L"( %s%s )", strName, GlobalText[1901]);
    AddMsg(strText, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
    AddMsg(GlobalText[1902], RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);

    return true;
}

void SEASON3B::CUseFruitCheckMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();

    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->Remove3DRenderObj(this);

    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();
}

void SEASON3B::CUseFruitCheckMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    auto* pMsg = new MSGBOX_TEXTDATA;
    pMsg->strMsg = strMsg;
    pMsg->dwColor = dwColor;
    pMsg->byFontType = byFontType;
    m_MsgDataList.push_back(pMsg);
}

void SEASON3B::CUseFruitCheckMsgBox::Set3DItem(ITEM* pItem)
{
    if (pItem)
    {
        memcpy(&m_Item, pItem, sizeof(ITEM));
    }
}

bool SEASON3B::CUseFruitCheckMsgBox::Update()
{
    m_BtnAdd.Update();
    m_BtnMinus.Update();
    m_BtnCancel.Update();

    return true;
}

bool SEASON3B::CUseFruitCheckMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderFrame();
    RenderTexts();
    RenderButtons();

    DisableAlphaBlend();
    return true;
}

void SEASON3B::CUseFruitCheckMsgBox::Render3D()
{
    float x, y, width, height;

    x = GetPos().x + 30;
    y = GetPos().y + 30;
    width = MSGBOX_3DITEM_WIDTH;
    height = MSGBOX_3DITEM_HEIGHT;

    RenderItem3D(x, y, width, height,
        m_Item.Type,
        m_Item.Level,
        m_Item.ExcellentFlags,
        m_Item.AncientDiscriminator,
        true);		// PickUp
}

bool SEASON3B::CUseFruitCheckMsgBox::IsVisible() const
{
    return true;
}

CALLBACK_RESULT SEASON3B::CUseFruitCheckMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CUseFruitCheckMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnAdd.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_USE_FRUIT_ADD);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnMinus.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_USE_FRUIT_MINUS);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CUseFruitCheckMsgBox::AddBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_byItemUseType = 0x00;
    BYTE byIndex = g_pMyInventory->GetStandbyItemIndex();
    SendRequestUse(byIndex, 0);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CUseFruitCheckMsgBox::MinusBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_byItemUseType = 0x01;
    BYTE byIndex = g_pMyInventory->GetStandbyItemIndex();
    SendRequestUse(byIndex, 0);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CUseFruitCheckMsgBox::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void SEASON3B::CUseFruitCheckMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CUseFruitCheckMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CUseFruitCheckMsgBox::AddBtnDown, MSGBOX_EVENT_USER_CUSTOM_USE_FRUIT_ADD);
    AddCallbackFunc(SEASON3B::CUseFruitCheckMsgBox::MinusBtnDown, MSGBOX_EVENT_USER_CUSTOM_USE_FRUIT_MINUS);
    AddCallbackFunc(SEASON3B::CUseFruitCheckMsgBox::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void SEASON3B::CUseFruitCheckMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float triwidth = (GetSize().cx / 3.f);
    float btnhalf = (MSGBOX_BTN_EMPTY_SMALL_WIDTH) / 2.f;

    x = GetPos().x + (triwidth / 2) - btnhalf;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    m_BtnAdd.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnAdd.SetText(GlobalText[1412]);

    x = GetPos().x + triwidth + (triwidth / 2) - btnhalf;
    m_BtnMinus.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnMinus.SetText(GlobalText[1903]);

    x = GetPos().x + (triwidth * 2) + (triwidth / 2) - btnhalf;
    m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnCancel.SetText(GlobalText[229]);
}

void SEASON3B::CUseFruitCheckMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < 1; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CUseFruitCheckMsgBox::RenderTexts()
{
    
    

    float x, y;

    x = GetPos().x + MSGBOX_TEXT_LEFT_BLANK_3DITEM; y = GetPos().y + MSGBOX_TEXT_TOP_BLANK;
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        SIZE TextSize;
        size_t TextExtentWidth, TextExtentHeight;

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), (*vi)->strMsg.c_str(), (*vi)->strMsg.size(), &TextSize);
        TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);
        TextExtentHeight = (size_t)(TextSize.cy / g_fScreenRate_y);

        x = GetPos().x + 60 + ((GetSize().cx - 60) / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void SEASON3B::CUseFruitCheckMsgBox::RenderButtons()
{
    m_BtnAdd.Render();
    m_BtnMinus.Render();
    m_BtnCancel.Render();
}

//////////////////////////////////////////////////////////////////////////

SEASON3B::CGemIntegrationMsgBox::CGemIntegrationMsgBox()
{
};

SEASON3B::CGemIntegrationMsgBox::~CGemIntegrationMsgBox()
{
    Release();
};

bool SEASON3B::CGemIntegrationMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    AddMsg(GlobalText[1801], RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);

    AddMsg(GlobalText[3307]);
    AddMsg(GlobalText[3308]);

    SetButtonInfo();

    return true;
}

void SEASON3B::CGemIntegrationMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();

    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();
}

bool SEASON3B::CGemIntegrationMsgBox::Update()
{
    m_BtnUnity.Update();
    m_BtnDisjoint.Update();
    m_BtnCancel.Update();

    return true;
}

bool SEASON3B::CGemIntegrationMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CGemIntegrationMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    auto* pMsg = new MSGBOX_TEXTDATA;
    pMsg->strMsg = strMsg;
    pMsg->dwColor = dwColor;
    pMsg->byFontType = byFontType;
    m_MsgDataList.push_back(pMsg);
}

void SEASON3B::CGemIntegrationMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CGemIntegrationMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CGemIntegrationMsgBox::UnityBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY);
    AddCallbackFunc(SEASON3B::CGemIntegrationMsgBox::DisjointBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT);
    AddCallbackFunc(SEASON3B::CGemIntegrationMsgBox::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

CALLBACK_RESULT SEASON3B::CGemIntegrationMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CGemIntegrationMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnUnity.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnDisjoint.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CGemIntegrationMsgBox::UnityBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetMode(COMGEM::ATTACH);

    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(CGemIntegrationUnityMsgBoxLayout));

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CGemIntegrationMsgBox::DisjointBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetMode(COMGEM::DETACH);

    if (!COMGEM::FindWantedList())
    {
        g_pSystemLogBox->AddText(GlobalText[1818], SEASON3B::TYPE_ERROR_MESSAGE);
        return CALLBACK_BREAK;
    }

    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(CGemIntegrationDisjointMsgBoxLayout));

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CGemIntegrationMsgBox::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::Exit();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void SEASON3B::CGemIntegrationMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalf = (MSGBOX_BTN_EMPTY_WIDTH) / 2.f;

    x = GetPos().x + msgboxhalfwidth - btnhalf;
    y = GetPos().y + BTN_TOP_BLANK;
    width = MSGBOX_BTN_EMPTY_WIDTH;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    m_BtnUnity.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);

    m_BtnUnity.SetText(GlobalText[1801]);

    x = GetPos().x + msgboxhalfwidth - btnhalf;
    y += BTN_GAP;
    m_BtnDisjoint.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);

    m_BtnDisjoint.SetText(GlobalText[1800]);

    btnhalf = MSGBOX_BTN_EMPTY_SMALL_WIDTH / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalf;
    y += BTN_GAP;
    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);

    m_BtnCancel.SetText(GlobalText[1002]);
}

void SEASON3B::CGemIntegrationMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < MIDDLE_COUNT; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CGemIntegrationMsgBox::RenderTexts()
{
    
    

    float x, y;

    x = GetPos().x; y = GetPos().y + (MSGBOX_TEXT_TOP_BLANK / 2);
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        SIZE TextSize;
        size_t TextExtentWidth, TextExtentHeight;

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), (*vi)->strMsg.c_str(), (*vi)->strMsg.size(), &TextSize);
        TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);
        TextExtentHeight = (size_t)(TextSize.cy / g_fScreenRate_y);

        x = GetPos().x + (GetSize().cx / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void SEASON3B::CGemIntegrationMsgBox::RenderButtons()
{
    m_BtnUnity.Render();
    m_BtnDisjoint.Render();
    m_BtnCancel.Render();
}

SEASON3B::CGemIntegrationUnityMsgBox::CGemIntegrationUnityMsgBox()
{
}

SEASON3B::CGemIntegrationUnityMsgBox::~CGemIntegrationUnityMsgBox()
{
    Release();
}

bool SEASON3B::CGemIntegrationUnityMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    AddMsg(GlobalText[1801], RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
    SetText();
    SetButtonInfo();

    return true;
}

void SEASON3B::CGemIntegrationUnityMsgBox::SetText(void)
{
    m_MsgDataList.clear();
    if (COMGEM::m_cGemType == COMGEM::NOGEM)
    {
        AddMsg(GlobalText[1801], RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
        AddMsg(GlobalText[3309]);
    }
    else
    {
        AddMsg(GlobalText[1801], RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
        AddMsg(GlobalText[3310]);
    }
}

void SEASON3B::CGemIntegrationUnityMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();

    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();
}

bool SEASON3B::CGemIntegrationUnityMsgBox::Update()
{
    int i;
    switch (COMGEM::m_cGemType)
    {
    case COMGEM::eNOGEM:
        for (i = 0; i < COMGEM::eGEMTYPE_END; i++)
            m_cJewelButton[i].Update();
        break;
    default:
        for (i = 0; i < COMGEM::eCOMTYPE_END; i++)
            m_cMixButton[i].Update();
        break;
    }
    m_BtnCancel.Update();

    return true;
}

bool SEASON3B::CGemIntegrationUnityMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderFrame();

    RenderTexts();

    RenderButtons();

    DisableAlphaBlend();
    return true;
}

void SEASON3B::CGemIntegrationUnityMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    auto* pMsg = new MSGBOX_TEXTDATA;
    pMsg->strMsg = strMsg;
    pMsg->dwColor = dwColor;
    pMsg->byFontType = byFontType;
    m_MsgDataList.push_back(pMsg);
}

void SEASON3B::CGemIntegrationUnityMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CGemIntegrationUnityMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CGemIntegrationUnityMsgBox::BlessingBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_BLESSING);
    AddCallbackFunc(SEASON3B::CGemIntegrationUnityMsgBox::SoulBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_SOUL);
    AddCallbackFunc(SEASON3B::CGemIntegrationUnityMsgBox::TenBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_TEN);
    AddCallbackFunc(SEASON3B::CGemIntegrationUnityMsgBox::TwentyBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_TWENTY);
    AddCallbackFunc(SEASON3B::CGemIntegrationUnityMsgBox::ThirtyBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_THIRTY);
    AddCallbackFunc(SEASON3B::CGemIntegrationUnityMsgBox::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    AddCallbackFunc(SEASON3B::CGemIntegrationUnityMsgBox::SelectMixBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_SELECTMIX);
}

void SEASON3B::CGemIntegrationUnityMsgBox::ResetWndSize(int _nType)
{
    int	height;

    if (_nType)
    {
        m_nMiddleCount = 7;
    }
    else
    {
        m_nMiddleCount = MIDDLE_COUNT;
    }

    height = MSGBOX_TOP_HEIGHT + (m_nMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;
    SetSize(GetSize().cx, height);
    m_BtnCancel.SetPos(m_BtnCancel.GetPosX(), GetPos().y + GetSize().cy - 50);
}

void SEASON3B::CGemIntegrationUnityMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_SMALL_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;

    int	  nNum = 0;
    int	  nBtnIndex[COMGEM::eGEMTYPE_END] = { 1806, 1807, 3312, 3313, 3314, 2081, 3315, 3316, 3317, 3318 };
    wchar_t szTemp[256] = { 0, };
    CNewUIMessageBoxButton	cButton;
    x = GetPos().x;
    y = GetPos().y + 50;
    for (int i = 0; i < (int)COMGEM::eGEMTYPE_END; i++)
    {
        cButton.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x + 20.0f + (i % 2) * (20 + width), y + (height + 5.0f) * int(i / 2), width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
        cButton.SetText(GlobalText[nBtnIndex[i]]);
        m_cJewelButton.push_back(cButton);
    }

    for (int k = 0; k < (int)COMGEM::eCOMTYPE_END; k++)
    {
        cButton.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x + 50.0f, y + (height + 10.0f) * k, MSGBOX_BTN_EMPTY_WIDTH + 20, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
        // 1808 "%d개 조합(%d젠 소요)"
        swprintf(szTemp, GlobalText[1808], 10 * (k + 1), 500000 * (k + 1));
        cButton.SetText(szTemp);
        m_cMixButton.push_back(cButton);
    }

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y += 15.0f + (height + 10.0f) * (int)COMGEM::eCOMTYPE_END;
    m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnCancel.SetText(GlobalText[1002]);

    ResetWndSize(0);
}

void SEASON3B::CGemIntegrationUnityMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;

    for (int i = 0; i < m_nMiddleCount; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CGemIntegrationUnityMsgBox::RenderTexts()
{
    
    

    float x, y;

    x = GetPos().x; y = GetPos().y + (MSGBOX_TEXT_TOP_BLANK / 2);
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        SIZE TextSize;
        size_t TextExtentWidth, TextExtentHeight;

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), (*vi)->strMsg.c_str(), (*vi)->strMsg.size(), &TextSize);
        TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);
        TextExtentHeight = (size_t)(TextSize.cy / g_fScreenRate_y);

        x = GetPos().x + (GetSize().cx / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void SEASON3B::CGemIntegrationUnityMsgBox::RenderButtons()
{
    int i;
    switch (COMGEM::m_cGemType)
    {
    case COMGEM::eNOGEM:
        for (i = 0; i < COMGEM::eGEMTYPE_END; i++)
            m_cJewelButton[i].Render();
        break;
    default:
        for (i = 0; i < COMGEM::eCOMTYPE_END; i++)
            m_cMixButton[i].Render();
        break;
    }
    m_BtnCancel.Render();
}

CALLBACK_RESULT SEASON3B::CGemIntegrationUnityMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CGemIntegrationUnityMsgBox*>(pOwner);
    if (pMsgBox)
    {
        int i;
        switch (COMGEM::m_cGemType)
        {
        case COMGEM::eNOGEM:
            for (i = 0; i < COMGEM::eGEMTYPE_END; i++)
            {
                if (!pMsgBox->m_cJewelButton[i].IsMouseIn())	continue;
                COMGEM::SetGem(i * 2);
                pMsgBox->ResetWndSize(1);
                //	SetText();
                break;
            }
            break;
        default:
            for (i = 0; i < COMGEM::eCOMTYPE_END; i++)
            {
                if (!pMsgBox->m_cMixButton[i].IsMouseIn())	continue;
                COMGEM::m_cComType = COMGEM::GetJewelRequireCount(i);
                pMsgBox->ResetWndSize(0);
                g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_SELECTMIX);
                return CALLBACK_BREAK;
            }
            break;
        }

        if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CGemIntegrationUnityMsgBox::BlessingBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetGem(COMGEM::CELE);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CGemIntegrationUnityMsgBox::SoulBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetGem(COMGEM::SOUL);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CGemIntegrationUnityMsgBox::SelectMixBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    if (!COMGEM::CheckInv())
    {
        return CALLBACK_BREAK;
    }

    SEASON3B::CNewUICommonMessageBox* pMsgBox = NULL;
    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGemIntegrationUnityCheckMsgBoxLayout), &pMsgBox);
    if (pMsgBox)
    {
        wchar_t strText[256] = { 0, };
        swprintf(strText, GlobalText[COMGEM::GetJewelIndex(COMGEM::m_cGemType, 0)], GlobalText[1807], COMGEM::m_cCount);
        pMsgBox->AddMsg(strText, CLRDW_YELLOW, MSGBOX_FONT_BOLD);

        swprintf(strText, GlobalText[1810], COMGEM::m_iValue);
        pMsgBox->AddMsg(strText, CLRDW_YELLOW, MSGBOX_FONT_BOLD);
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CGemIntegrationUnityMsgBox::TenBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetComType(COMGEM::FIRST);

    if (COMGEM::CheckInv())
    {
        SEASON3B::CNewUICommonMessageBox* pMsgBox = NULL;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGemIntegrationUnityCheckMsgBoxLayout), &pMsgBox);
        if (pMsgBox)
        {
            wchar_t strText[256] = { 0, };
            if (COMGEM::m_cGemType == COMGEM::CELE)
            {
                swprintf(strText, GlobalText[1809], GlobalText[1806], COMGEM::m_cCount);
            }
            else if (COMGEM::m_cGemType == COMGEM::SOUL)
            {
                swprintf(strText, GlobalText[1809], GlobalText[1807], COMGEM::m_cCount);
            }
            pMsgBox->AddMsg(strText, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
            swprintf(strText, GlobalText[1810], COMGEM::m_iValue);
            pMsgBox->AddMsg(strText, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
        }

        PlayBuffer(SOUND_CLICK01);
        g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    }
    else
    {
        auto* pMsgBox = dynamic_cast<CGemIntegrationUnityMsgBox*>(pOwner);
        if (pMsgBox)
        {
            pMsgBox->m_BtnTen.ClearEventState();
        }
        PlayBuffer(SOUND_CLICK01);
    }

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CGemIntegrationUnityMsgBox::TwentyBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetComType(COMGEM::SECOND);

    if (COMGEM::CheckInv())
    {
        SEASON3B::CNewUICommonMessageBox* pMsgBox = NULL;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGemIntegrationUnityCheckMsgBoxLayout), &pMsgBox);
        if (pMsgBox)
        {
            wchar_t strText[256] = { 0, };
            if (COMGEM::m_cGemType == COMGEM::CELE)
            {
                swprintf(strText, GlobalText[1809], GlobalText[1806], COMGEM::m_cCount);
            }
            else if (COMGEM::m_cGemType == COMGEM::SOUL)
            {
                swprintf(strText, GlobalText[1809], GlobalText[1807], COMGEM::m_cCount);
            }
            pMsgBox->AddMsg(strText, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
            swprintf(strText, GlobalText[1810], COMGEM::m_iValue);
            pMsgBox->AddMsg(strText, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
        }

        PlayBuffer(SOUND_CLICK01);
        g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    }
    else
    {
        auto* pMsgBox = dynamic_cast<CGemIntegrationUnityMsgBox*>(pOwner);
        if (pMsgBox)
        {
            pMsgBox->m_BtnTwenty.ClearEventState();
        }
        PlayBuffer(SOUND_CLICK01);
    }

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CGemIntegrationUnityMsgBox::ThirtyBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetComType(COMGEM::THIRD);

    if (COMGEM::CheckInv())
    {
        SEASON3B::CNewUICommonMessageBox* pMsgBox = NULL;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGemIntegrationUnityCheckMsgBoxLayout), &pMsgBox);
        if (pMsgBox)
        {
            wchar_t strText[256] = { 0, };
            if (COMGEM::m_cGemType == COMGEM::CELE)
            {
                swprintf(strText, GlobalText[1809], GlobalText[1806], COMGEM::m_cCount);
            }
            else if (COMGEM::m_cGemType == COMGEM::SOUL)
            {
                swprintf(strText, GlobalText[1809], GlobalText[1807], COMGEM::m_cCount);
            }
            pMsgBox->AddMsg(strText, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
            swprintf(strText, GlobalText[1810], COMGEM::m_iValue);
            pMsgBox->AddMsg(strText, RGBA(255, 255, 0, 255), MSGBOX_FONT_BOLD);
        }

        PlayBuffer(SOUND_CLICK01);
        g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    }
    else
    {
        auto* pMsgBox = dynamic_cast<CGemIntegrationUnityMsgBox*>(pOwner);
        if (pMsgBox)
        {
            pMsgBox->m_BtnThirty.ClearEventState();
        }
        PlayBuffer(SOUND_CLICK01);
    }

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CGemIntegrationUnityMsgBox::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::Exit();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

SEASON3B::CGemIntegrationDisjointMsgBox::CGemIntegrationDisjointMsgBox()
{
    m_iMiddleFrameCount = 0;
}

SEASON3B::CGemIntegrationDisjointMsgBox::~CGemIntegrationDisjointMsgBox()
{
    Release();
}

bool SEASON3B::CGemIntegrationDisjointMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    m_iMiddleFrameCount = 1;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleFrameCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    AddMsg(GlobalText[1800], RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
    SetButtonInfo();
    ChangeMiddleFrameBig();
    AddMsg(L" ", RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
    AddMsg(GlobalText[3311], CLRDW_YELLOW, MSGBOX_FONT_BOLD);

    return true;
}

void SEASON3B::CGemIntegrationDisjointMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();

    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();
}

bool SEASON3B::CGemIntegrationDisjointMsgBox::Update()
{
    m_BtnCancel.Update();

    if (true)
    {
        COMGEM::MoveUnMixList();

        UNMIX_TEXT* pUT = COMGEM::m_UnmixTarList.GetSelectedText();
        if (pUT)
        {
            m_BtnDisjoint.SetEnable(true);
            m_BtnDisjoint.Update();
        }
    }

    return true;
}

bool SEASON3B::CGemIntegrationDisjointMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButtons();
    RenderGemList();
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CGemIntegrationDisjointMsgBox::ChangeMiddleFrameSmall()
{
    int height = 0;

    m_iMiddleFrameCount = 1;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleFrameCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    SetSize(GetSize().cx, height);

    m_BtnCancel.SetPos(m_BtnCancel.GetPosX(), GetPos().y + 80);

    m_BtnDisjoint.SetEnable(false);
}

void SEASON3B::CGemIntegrationDisjointMsgBox::ChangeMiddleFrameBig()
{
    int height = 0;

    m_iMiddleFrameCount = 10;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleFrameCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    SetSize(GetSize().cx, height);

    m_BtnCancel.SetPos(m_BtnCancel.GetPosX(), GetPos().y + GetSize().cy - 50);

    m_BtnDisjoint.SetPos(m_BtnDisjoint.GetPosX(), GetPos().y + GetSize().cy - 85);
}

void SEASON3B::CGemIntegrationDisjointMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    auto* pMsg = new MSGBOX_TEXTDATA;
    pMsg->strMsg = strMsg;
    pMsg->dwColor = dwColor;
    pMsg->byFontType = byFontType;
    m_MsgDataList.push_back(pMsg);
}

void SEASON3B::CGemIntegrationDisjointMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CGemIntegrationDisjointMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CGemIntegrationDisjointMsgBox::BlessingBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_BLESSING);
    AddCallbackFunc(SEASON3B::CGemIntegrationDisjointMsgBox::SoulBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_SOUL);
    AddCallbackFunc(SEASON3B::CGemIntegrationDisjointMsgBox::DisjointBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_DISJOINT);
    AddCallbackFunc(SEASON3B::CGemIntegrationDisjointMsgBox::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

CALLBACK_RESULT SEASON3B::CGemIntegrationDisjointMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CGemIntegrationDisjointMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnBlessing.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_BLESSING);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnSoul.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_SOUL);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnDisjoint.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_DISJOINT);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CGemIntegrationDisjointMsgBox::BlessingBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CGemIntegrationDisjointMsgBox*>(pOwner);
    if (pMsgBox == false)
    {
        return CALLBACK_CONTINUE;
    }

    COMGEM::SetGem(COMGEM::CELE);

    COMGEM::ResetWantedList();
    COMGEM::FindWantedList();

    if (COMGEM::m_UnmixTarList.IsEmpty() == true)
    {
        g_pSystemLogBox->AddText(GlobalText[1818], SEASON3B::TYPE_ERROR_MESSAGE);
        COMGEM::GetBack();
        pMsgBox->ChangeMiddleFrameSmall();
    }
    else
    {
        pMsgBox->ChangeMiddleFrameBig();
    }

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CGemIntegrationDisjointMsgBox::SoulBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CGemIntegrationDisjointMsgBox*>(pOwner);
    if (pMsgBox == false)
    {
        return CALLBACK_CONTINUE;
    }

    COMGEM::SetGem(COMGEM::SOUL);

    COMGEM::ResetWantedList();
    COMGEM::FindWantedList();

    if (COMGEM::m_UnmixTarList.IsEmpty() == true)
    {
        g_pSystemLogBox->AddText(GlobalText[1818], SEASON3B::TYPE_ERROR_MESSAGE);
        COMGEM::GetBack();
        pMsgBox->ChangeMiddleFrameSmall();
    }
    else
    {
        pMsgBox->ChangeMiddleFrameBig();
    }

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CGemIntegrationDisjointMsgBox::DisjointBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    UNMIX_TEXT* pUT = COMGEM::m_UnmixTarList.GetSelectedText();
    if (pUT)
    {
        COMGEM::SelectFromList(pUT->m_iInvenIdx, pUT->m_cLevel);

        SEASON3B::CNewUICommonMessageBox* pMsgBox = NULL;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGemIntegrationDisjointCheckMsgBoxLayout), &pMsgBox);

        if (pMsgBox)
        {
            wchar_t strText[256] = { 0, };
            int	iGemLevel = COMGEM::GetUnMixGemLevel() + 1;
            ITEM* pItem = g_pMyInventory->GetInventoryCtrl()->FindItem(pUT->m_iInvenIdx);
            int	  nIdx = COMGEM::Check_Jewel(pItem->Type);
            COMGEM::SetGem(nIdx);
            swprintf(strText, GlobalText[1813], GlobalText[COMGEM::GetJewelIndex(nIdx, COMGEM::eGEM_NAME)], iGemLevel);

            pMsgBox->AddMsg(strText, CLRDW_DARKYELLOW, MSGBOX_FONT_BOLD);
            swprintf(strText, GlobalText[1814], COMGEM::m_iValue);
            pMsgBox->AddMsg(strText, CLRDW_DARKYELLOW, MSGBOX_FONT_BOLD);
        }
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CGemIntegrationDisjointMsgBox::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::Exit();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void SEASON3B::CGemIntegrationDisjointMsgBox::SetButtonInfo()
{
    float x, y, width, height;
    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_SMALL_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 40;
    m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnCancel.SetText(GlobalText[1002]);

    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    m_BtnDisjoint.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnDisjoint.SetText(GlobalText[188]);
    m_BtnDisjoint.SetEnable(false);
}

void SEASON3B::CGemIntegrationDisjointMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleFrameCount; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CGemIntegrationDisjointMsgBox::RenderTexts()
{
    
    

    float x, y;

    x = GetPos().x; y = GetPos().y + (MSGBOX_TEXT_TOP_BLANK / 2);
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        SIZE TextSize;
        size_t TextExtentWidth, TextExtentHeight;

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), (*vi)->strMsg.c_str(), (*vi)->strMsg.size(), &TextSize);
        TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);
        TextExtentHeight = (size_t)(TextSize.cy / g_fScreenRate_y);

        x = GetPos().x + (GetSize().cx / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void SEASON3B::CGemIntegrationDisjointMsgBox::RenderGemList()
{
    
    

    int x, y;
    y = GetPos().y + 80;

    m_BtnDisjoint.Render();

    x = GetPos().x + (GetSize().cx / 2) - (COMGEM::m_UnmixTarList.GetWidth() / 2);

    COMGEM::m_UnmixTarList.SetPosition(x, y + 40 + COMGEM::m_UnmixTarList.GetHeight() / 2.0f);
    COMGEM::RenderUnMixList();
}

void SEASON3B::CGemIntegrationDisjointMsgBox::RenderButtons()
{
    m_BtnCancel.Render();
}

//////////////////////////////////////////////////////////////////////////

SEASON3B::CSystemMenuMsgBox::CSystemMenuMsgBox()
{
}

SEASON3B::CSystemMenuMsgBox::~CSystemMenuMsgBox()
{
    Release();
}

bool SEASON3B::CSystemMenuMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (5 * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    return true;
}

void SEASON3B::CSystemMenuMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();
}

bool SEASON3B::CSystemMenuMsgBox::Update()
{
    m_BtnGameOver.Update();
    m_BtnChooseServer.Update();
    m_BtnChooseCharacter.Update();
    m_BtnOption.Update();
    m_BtnCancel.Update();
    return true;
}

bool SEASON3B::CSystemMenuMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CSystemMenuMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < 5; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CSystemMenuMsgBox::RenderButtons()
{
    m_BtnGameOver.Render();
    m_BtnChooseServer.Render();
    m_BtnChooseCharacter.Render();
    m_BtnOption.Render();
    m_BtnCancel.Render();
}

void SEASON3B::CSystemMenuMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CSystemMenuMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CSystemMenuMsgBox::GameOverBtnDown, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_GAMEOVER);
    AddCallbackFunc(SEASON3B::CSystemMenuMsgBox::ChooseServerBtnDown, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_CHOOSESERVER);
    AddCallbackFunc(SEASON3B::CSystemMenuMsgBox::ChooseCharacterBtnDown, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_CHOOSECHARACTER);
    AddCallbackFunc(SEASON3B::CSystemMenuMsgBox::OptionBtnDown, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_OPTION);
    AddCallbackFunc(SEASON3B::CSystemMenuMsgBox::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    AddCallbackFunc(SEASON3B::CSystemMenuMsgBox::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
}

void SEASON3B::CSystemMenuMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 23;
    m_BtnGameOver.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnGameOver.SetText(GlobalText[381]);

    y += 30.f;
    m_BtnChooseServer.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnChooseServer.SetText(GlobalText[382]);

    y += 30.f;
    m_BtnChooseCharacter.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnChooseCharacter.SetText(GlobalText[383]);

    y += 30.f;
    m_BtnOption.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnOption.SetText(GlobalText[385]);

    y += 30.f;
    m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnCancel.SetText(GlobalText[384]);
}

CALLBACK_RESULT SEASON3B::CSystemMenuMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CSystemMenuMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnGameOver.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_GAMEOVER);

            if (Hero->PK) return CALLBACK_BREAK;

            g_pNewUIHotKey->SetStateGameOver(true);

            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnChooseServer.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_CHOOSESERVER);

            if (Hero->PK) return CALLBACK_BREAK;

            g_pNewUIHotKey->SetStateGameOver(true);

            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnChooseCharacter.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_CHOOSECHARACTER);

            if (Hero->PK) return CALLBACK_BREAK;

            g_pNewUIHotKey->SetStateGameOver(true);

            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnOption.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SYSTEMMENU_OPTION);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CSystemMenuMsgBox::GameOverBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_ErrorReport.Write(L"> Menu - Exit game. ");
    g_ErrorReport.WriteCurrentTime();

    SaveOptions();
    SaveMacro(L"Data\\Macro.txt");

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY))
    {
        g_pSystemLogBox->AddText(GlobalText[592], SEASON3B::TYPE_ERROR_MESSAGE);
    }
    else
    {
        MUHelper::g_MuHelper.TriggerStop();
        LogOut = true;
        SocketClient->ToGameServer()->SendLogOut(0);
        g_ConsoleDebug->Write(MCD_SEND, L"0xF1 [SendRequestLogOut] 0");
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CSystemMenuMsgBox::ChooseServerBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    View_End_Result = false;
    Suc_Or_Fail = -1;
    M34CryWolf1st::CryWolfMVPInit();

    g_ErrorReport.Write(L"> Menu - Join another server. ");
    g_ErrorReport.WriteCurrentTime();

    SaveOptions();
    SaveMacro(L"Data\\Macro.txt");

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY))
    {
        g_pSystemLogBox->AddText(GlobalText[592], SEASON3B::TYPE_ERROR_MESSAGE);
    }
    else
    {
        MUHelper::g_MuHelper.TriggerStop();
        g_pNewUIMng->ResetActiveUIObj();
        LogOut = true;
        SocketClient->ToGameServer()->SendLogOut(2);
        g_ConsoleDebug->Write(MCD_SEND, L"0xF1 [SendRequestLogOut] 2");
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CSystemMenuMsgBox::ChooseCharacterBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    View_End_Result = false;
    Suc_Or_Fail = -1;
    M34CryWolf1st::CryWolfMVPInit();

    g_ErrorReport.Write(L"> Menu - Join with another character. ");
    g_ErrorReport.WriteCurrentTime();

    //  게임내에서 설정한 데이터 저장.
    SaveOptions();
    SaveMacro(L"Data\\Macro.txt");

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY))
    {
        g_pSystemLogBox->AddText(GlobalText[592], SEASON3B::TYPE_SYSTEM_MESSAGE);
    }
    else
    {
        MUHelper::g_MuHelper.TriggerStop();
        g_pNewUIMng->ResetActiveUIObj();
        LogOut = true;
        SocketClient->ToGameServer()->SendLogOut(1);
        g_ConsoleDebug->Write(MCD_SEND, L"0xF1 [SendRequestLogOut] 1");
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CSystemMenuMsgBox::OptionBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_pNewUISystem->Show(SEASON3B::INTERFACE_OPTION);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CSystemMenuMsgBox::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

SEASON3B::CBloodCastleResultMsgBox::CBloodCastleResultMsgBox()
{
}

SEASON3B::CBloodCastleResultMsgBox::~CBloodCastleResultMsgBox()
{
}

bool SEASON3B::CBloodCastleResultMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    AddCallbackFunc(SEASON3B::CBloodCastleResultMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CBloodCastleResultMsgBox::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    x = GetPos().x + (GetSize().cx / 2) - (MSGBOX_BTN_WIDTH / 2);
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    width = MSGBOX_BTN_WIDTH;
    height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    return true;
}

bool SEASON3B::CBloodCastleResultMsgBox::Update()
{
    m_BtnOk.Update();

    return true;
}

bool SEASON3B::CBloodCastleResultMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    m_BtnOk.Render();
    EnableAlphaBlend();
    matchEvent::RenderResult();
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CBloodCastleResultMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < MIDDLE_COUNT; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

CALLBACK_RESULT SEASON3B::CBloodCastleResultMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CBloodCastleResultMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CBloodCastleResultMsgBox::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

SEASON3B::CDevilSquareRankMsgBox::CDevilSquareRankMsgBox()
{
}

SEASON3B::CDevilSquareRankMsgBox::~CDevilSquareRankMsgBox()
{
}

bool SEASON3B::CDevilSquareRankMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    AddCallbackFunc(SEASON3B::CDevilSquareRankMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CDevilSquareRankMsgBox::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT1 * MSGBOX_MIDDLE_HEIGHT)
        + (MIDDLE_COUNT2 * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_LINE_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    x = GetPos().x + (GetSize().cx / 2) - (MSGBOX_BTN_WIDTH / 2);
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    width = MSGBOX_BTN_WIDTH;
    height = MSGBOX_BTN_HEIGHT;
    m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);

    return true;
}

bool SEASON3B::CDevilSquareRankMsgBox::Update()
{
    m_BtnOk.Update();
    matchEvent::SetPosition(GetPos().x, GetPos().y);

    return true;
}

bool SEASON3B::CDevilSquareRankMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    m_BtnOk.Render();
    EnableAlphaBlend();
    matchEvent::RenderResult();
    DisableAlphaBlend();

    return true;
}

CALLBACK_RESULT SEASON3B::CDevilSquareRankMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CDevilSquareRankMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CDevilSquareRankMsgBox::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void SEASON3B::CDevilSquareRankMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < MIDDLE_COUNT1; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_LINE_WIDTH; height = MSGBOX_LINE_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_LINE, x, y, width, height);
    y += height;

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;

    for (int i = 0; i < MIDDLE_COUNT2; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);

    x = GetPos().x + 13; y = GetPos().y + 75; width = MSGBOX_SEPARATE_LINE_WIDTH; height = MSGBOX_SEPARATE_LINE_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_SEPARATE_LINE, x, y, width, height);

    x = GetPos().x + 13; y = GetPos().y + 93; width = MSGBOX_SEPARATE_LINE_WIDTH; height = MSGBOX_SEPARATE_LINE_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_SEPARATE_LINE, x, y, width, height);

    x = GetPos().x + 13; y = GetPos().y + 255; width = MSGBOX_SEPARATE_LINE_WIDTH; height = MSGBOX_SEPARATE_LINE_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_SEPARATE_LINE, x, y, width, height);

    x = GetPos().x + 13; y = GetPos().y + 273; width = MSGBOX_SEPARATE_LINE_WIDTH; height = MSGBOX_SEPARATE_LINE_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_SEPARATE_LINE, x, y, width, height);
}

SEASON3B::CChaosCastleResultMsgBox::CChaosCastleResultMsgBox()
{
}

SEASON3B::CChaosCastleResultMsgBox::~CChaosCastleResultMsgBox()
{
}

bool SEASON3B::CChaosCastleResultMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    AddCallbackFunc(SEASON3B::CChaosCastleResultMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CChaosCastleResultMsgBox::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    x = GetPos().x + (GetSize().cx / 2) - (MSGBOX_BTN_WIDTH / 2);
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    width = MSGBOX_BTN_WIDTH;
    height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    return true;
}

bool SEASON3B::CChaosCastleResultMsgBox::Update()
{
    m_BtnOk.Update();

    return true;
}

bool SEASON3B::CChaosCastleResultMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    m_BtnOk.Render();
    EnableAlphaBlend();
    matchEvent::RenderResult();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT SEASON3B::CChaosCastleResultMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CChaosCastleResultMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CChaosCastleResultMsgBox::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

void SEASON3B::CChaosCastleResultMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < MIDDLE_COUNT; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

//////////////////////////////////////////////////////////////////////////

SEASON3B::CChaosMixMenuMsgBox::CChaosMixMenuMsgBox()
{
}

SEASON3B::CChaosMixMenuMsgBox::~CChaosMixMenuMsgBox()
{
    Release();
}

bool SEASON3B::CChaosMixMenuMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    return true;
}

void SEASON3B::CChaosMixMenuMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();
}

bool SEASON3B::CChaosMixMenuMsgBox::Update()
{
    m_BtnGeneralMix.Update();
    m_BtnChaosMix.Update();
    m_BtnMix380.Update();
    m_BtnCancel.Update();

    return true;
}

bool SEASON3B::CChaosMixMenuMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT SEASON3B::CChaosMixMenuMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CChaosMixMenuMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnGeneralMix.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_MIXMENU_GENERALMIX);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnChaosMix.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_MIXMENU_CHAOSMIX);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnMix380.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_MIXMENU_MIX380);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CChaosMixMenuMsgBox::GeneralMixBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(0);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CChaosMixMenuMsgBox::ChaosMixBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(1);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CChaosMixMenuMsgBox::Mix380BtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(2);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CChaosMixMenuMsgBox::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.ClearCheckRecipeResult();
    g_pNewUISystem->Hide(SEASON3B::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void SEASON3B::CChaosMixMenuMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CChaosMixMenuMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CChaosMixMenuMsgBox::GeneralMixBtnDown, MSGBOX_EVENT_USER_CUSTOM_MIXMENU_GENERALMIX);
    AddCallbackFunc(SEASON3B::CChaosMixMenuMsgBox::ChaosMixBtnDown, MSGBOX_EVENT_USER_CUSTOM_MIXMENU_CHAOSMIX);
    AddCallbackFunc(SEASON3B::CChaosMixMenuMsgBox::Mix380BtnDown, MSGBOX_EVENT_USER_CUSTOM_MIXMENU_MIX380);
    AddCallbackFunc(SEASON3B::CChaosMixMenuMsgBox::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void SEASON3B::CChaosMixMenuMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 85;
    m_BtnGeneralMix.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnGeneralMix.SetText(GlobalText[735]);

    y = GetPos().y + 155;
    m_BtnChaosMix.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnChaosMix.SetText(GlobalText[736]);

    y = GetPos().y + 225;
    m_BtnMix380.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnMix380.SetText(GlobalText[2193]);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnCancel.SetText(GlobalText[1002]);
}

void SEASON3B::CChaosMixMenuMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < MIDDLE_COUNT; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CChaosMixMenuMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 128, 0, 255);
    g_pRenderText->SetFont(g_hFontBold);
    swprintf(szText, GlobalText[734]);
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 15;
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFont);

    swprintf(szText, GlobalText[872]);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 15, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    swprintf(szText, GlobalText[873], Hero->ID);
    g_pRenderText->RenderText(fPos_x, fPos_y + 2 * 15, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    swprintf(szText, GlobalText[1680], Hero->ID);
    g_pRenderText->RenderText(fPos_x, fPos_y + 3 * 15, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 100;
    swprintf(szText, GlobalText[870], Hero->ID);
    g_pRenderText->RenderText(fPos_x, fPos_y + 0 * 15, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    swprintf(szText, GlobalText[871], Hero->ID);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 15, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 85;
    swprintf(szText, GlobalText[2194], Hero->ID);
    g_pRenderText->RenderText(fPos_x, fPos_y + 0 * 15, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void SEASON3B::CChaosMixMenuMsgBox::RenderButtons()
{
    m_BtnGeneralMix.Render();
    m_BtnChaosMix.Render();
    m_BtnMix380.Render();
    m_BtnCancel.Render();
}

SEASON3B::CDialogMsgBox::CDialogMsgBox()
{
}

SEASON3B::CDialogMsgBox::~CDialogMsgBox()
{
    Release();
}

bool SEASON3B::CDialogMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    return true;
}

void SEASON3B::CDialogMsgBox::Release()
{
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();
}

void SEASON3B::CDialogMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    int iOrigSize = m_MsgDataList.size();
    int iLine = SeparateText(strMsg, dwColor, byFontType);
    int iSize = m_MsgDataList.size();

    if (iSize > 2)
    {
        float height = GetSize().cy;

        if (iOrigSize < 2)
        {
            iLine = iLine + iOrigSize - 2;
        }

        height += (MSGBOX_MIDDLE_HEIGHT * iLine);
        SetSize(GetSize().cx, height);
        AddButtonBlank(iLine);
    }
}

CALLBACK_RESULT SEASON3B::CDialogMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CDialogMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnEnd.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_DIALOG_END);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CDialogMsgBox::EndBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool SEASON3B::CDialogMsgBox::Update()
{
    m_BtnEnd.Update();

    return true;
}

bool SEASON3B::CDialogMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CDialogMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CDialogMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CDialogMsgBox::EndBtnDown, MSGBOX_EVENT_USER_CUSTOM_DIALOG_END);
}

int SEASON3B::CDialogMsgBox::SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    

    SIZE TextSize;
    size_t TextExtentWidth;
    int iLine = 0;

    GetTextExtentPoint32(g_pRenderText->GetFontDC(), strMsg.c_str(), strMsg.size(), &TextSize);
    TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);

    if (TextExtentWidth <= MSGBOX_TEXT_MAXWIDTH)
    {
        auto* pMsg = new MSGBOX_TEXTDATA;
        pMsg->strMsg = strMsg;
        pMsg->dwColor = dwColor;
        pMsg->byFontType = byFontType;
        m_MsgDataList.push_back(pMsg);

        iLine = 1;
        return iLine;
    }

    type_string strCutText, strRemainText;
    strRemainText = strMsg;

    bool bLoop = true;
    while (bLoop)
    {
        int prev_offset = 0;
        for (int cur_offset = 0; cur_offset < (int)strRemainText.size(); )
        {
            prev_offset = cur_offset;
            size_t offset = _mbclen((const unsigned char*)(strRemainText.c_str() + cur_offset));
            cur_offset += offset;

            type_string strTemp(strRemainText, 0, cur_offset/* size */);
            GetTextExtentPoint32(g_pRenderText->GetFontDC(), strTemp.c_str(), strTemp.size(), &TextSize);
            TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);

            if (TextExtentWidth > MSGBOX_TEXT_MAXWIDTH && cur_offset != 0)
            {
                strCutText = type_string(strRemainText, 0, prev_offset/* size */);
                strRemainText = type_string(strRemainText, prev_offset, strRemainText.size() - prev_offset/* size */);

                auto* pMsg = new MSGBOX_TEXTDATA;
                pMsg->strMsg = strCutText;
                pMsg->dwColor = dwColor;
                pMsg->byFontType = byFontType;
                m_MsgDataList.push_back(pMsg);
                iLine++;

                GetTextExtentPoint32(g_pRenderText->GetFontDC(), strRemainText.c_str(), strRemainText.size(), &TextSize);
                TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);

                if (TextExtentWidth <= MSGBOX_TEXT_MAXWIDTH)
                {
                    auto* pMsg = new MSGBOX_TEXTDATA;
                    pMsg->strMsg = strRemainText;
                    pMsg->dwColor = dwColor;
                    pMsg->byFontType = byFontType;
                    m_MsgDataList.push_back(pMsg);
                    iLine++;

                    bLoop = false;
                    break;
                }

                break;
            }
        }
    }

    return iLine;
}

void SEASON3B::CDialogMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_SMALL_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnEnd.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnEnd.SetText(GlobalText[609]);
}

void SEASON3B::CDialogMsgBox::AddButtonBlank(int iAddLine)
{
    m_BtnEnd.AddBlank(iAddLine * MSGBOX_MIDDLE_HEIGHT);
}

void SEASON3B::CDialogMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    if (m_MsgDataList.size() > 2)
    {
        int iCount = m_MsgDataList.size() - 2;
        for (int i = 0; i < iCount; ++i)
        {
            RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
            y += height;
        }
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CDialogMsgBox::RenderTexts()
{
    

    float x, y;

    x = GetPos().x; y = GetPos().y + MSGBOX_TEXT_TOP_BLANK;
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        SIZE TextSize;
        size_t TextExtentWidth, TextExtentHeight;

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), (*vi)->strMsg.c_str(), (*vi)->strMsg.size(), &TextSize);
        TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);
        TextExtentHeight = (size_t)(TextSize.cy / g_fScreenRate_y);

        x = GetPos().x + (MSGBOX_WIDTH / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void SEASON3B::CDialogMsgBox::RenderButtons()
{
    m_BtnEnd.Render();
}

//////////////////////////////////////////////////////////////////////////

SEASON3B::CProgressMsgBox::CProgressMsgBox()
{
    m_dwStartTime = 0;
    m_dwEndTime = 0;
    m_dwElapseTime = 0;
}

SEASON3B::CProgressMsgBox::~CProgressMsgBox()
{
    Release();
}

bool SEASON3B::CProgressMsgBox::Create(DWORD dwElapseTime, float fPriority)
{
    int x, y, width, height;

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    SetAddCallbackFunc();

    m_dwElapseTime = dwElapseTime;
    m_dwStartTime = timeGetTime();
    m_dwEndTime = m_dwStartTime + m_dwElapseTime;

    SetCanMove(true);

    return true;
}

void SEASON3B::CProgressMsgBox::Release()
{
}

void SEASON3B::CProgressMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CProgressMsgBox::ClosingProcess, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_CLOSINGPROCESS);
}

void SEASON3B::CProgressMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    int iOrigSize = m_MsgDataList.size();
    int iLine = SeparateText(strMsg, dwColor, byFontType);
    int iSize = m_MsgDataList.size();

    if (iSize > 2)
    {
        float height = GetSize().cy;

        if (iOrigSize < 2)
        {
            iLine = iLine + iOrigSize - 2;
        }

        height += (MSGBOX_MIDDLE_HEIGHT * iLine);
        SetSize(GetSize().cx, height);
    }
}

void SEASON3B::CProgressMsgBox::SetElapseTime(DWORD dwElapseTime)
{
    m_dwElapseTime = dwElapseTime;
    m_dwStartTime = timeGetTime();
    m_dwEndTime = m_dwStartTime + m_dwElapseTime;
}

int SEASON3B::CProgressMsgBox::SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    

    SIZE TextSize;
    size_t TextExtentWidth;
    int iLine = 0;

    GetTextExtentPoint32(g_pRenderText->GetFontDC(), strMsg.c_str(), strMsg.size(), &TextSize);
    TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);

    if (TextExtentWidth <= MSGBOX_TEXT_MAXWIDTH)
    {
        auto* pMsg = new MSGBOX_TEXTDATA;
        pMsg->strMsg = strMsg;
        pMsg->dwColor = dwColor;
        pMsg->byFontType = byFontType;
        m_MsgDataList.push_back(pMsg);

        iLine = 1;
        return iLine;
    }

    type_string strCutText, strRemainText;
    strRemainText = strMsg;

    bool bLoop = true;
    while (bLoop)
    {
        int prev_offset = 0;

        for (int cur_offset = 0; cur_offset < (int)strRemainText.size(); )
        {
            prev_offset = cur_offset;
            size_t offset = _mbclen((const unsigned char*)(strRemainText.c_str() + cur_offset));
            cur_offset += offset;

            type_string strTemp(strRemainText, 0, cur_offset/* size */);
            GetTextExtentPoint32(g_pRenderText->GetFontDC(), strTemp.c_str(), strTemp.size(), &TextSize);
            TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);

            if (TextExtentWidth > MSGBOX_TEXT_MAXWIDTH && cur_offset != 0)
            {
                strCutText = type_string(strRemainText, 0, prev_offset/* size */);
                strRemainText = type_string(strRemainText, prev_offset, strRemainText.size() - prev_offset/* size */);

                auto* pMsg = new MSGBOX_TEXTDATA;
                pMsg->strMsg = strCutText;
                pMsg->dwColor = dwColor;
                pMsg->byFontType = byFontType;
                m_MsgDataList.push_back(pMsg);
                iLine++;

                GetTextExtentPoint32(g_pRenderText->GetFontDC(), strRemainText.c_str(), strRemainText.size(), &TextSize);
                TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);

                if (TextExtentWidth <= MSGBOX_TEXT_MAXWIDTH)
                {
                    auto* pMsg = new MSGBOX_TEXTDATA;
                    pMsg->strMsg = strRemainText;
                    pMsg->dwColor = dwColor;
                    pMsg->byFontType = byFontType;
                    m_MsgDataList.push_back(pMsg);
                    iLine++;

                    bLoop = false;
                    break;
                }

                break;
            }
        }

        int i = 0;
    }

    return iLine;
}

bool SEASON3B::CProgressMsgBox::Update()
{
    g_pMainFrame->UpdateItemHotKey();

    DWORD dwTime = timeGetTime();
    if (dwTime >= m_dwEndTime)
    {
        g_MessageBox->SendEvent(this, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_CLOSINGPROCESS);
    }

    return true;
}

bool SEASON3B::CProgressMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderFrame();
    RenderTexts();
    RenderProgress();

    DisableAlphaBlend();

    return true;
}

void SEASON3B::CProgressMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    if (m_MsgDataList.size() > 2)
    {
        int iCount = m_MsgDataList.size() - 2;
        for (int i = 0; i < iCount; ++i)
        {
            RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
            y += height;
        }
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CProgressMsgBox::RenderTexts()
{
    

    float x, y;

    x = GetPos().x; y = GetPos().y + MSGBOX_TEXT_TOP_BLANK;
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        SIZE TextSize;
        size_t TextExtentWidth, TextExtentHeight;

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), (*vi)->strMsg.c_str(), (*vi)->strMsg.size(), &TextSize);
        TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);
        TextExtentHeight = (size_t)(TextSize.cy / g_fScreenRate_y);

        x = GetPos().x + (MSGBOX_WIDTH / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void SEASON3B::CProgressMsgBox::RenderProgress()
{
    DWORD dwTime = timeGetTime();
    float fProgress = (float)(dwTime - m_dwStartTime) / m_dwElapseTime;

    float x, y;
    x = GetPos().x + MSGBOX_WIDTH / 2 - 160.f / 2;
    y = GetPos().y + GetSize().cy - 50.f;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_PROGRESS_BG, x, y, 160.f, 18.f);
    x += 5.f;
    y += 5.f;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_PROGRESS_BAR, x, y, 150.f * fProgress, 8.f);
}

CALLBACK_RESULT SEASON3B::CProgressMsgBox::ClosingProcess(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_CONTINUE;
}

SEASON3B::CCursedTempleProgressMsgBox::CCursedTempleProgressMsgBox()
{
    m_dwStartTime = 0;
    m_dwEndTime = 0;
    m_dwElapseTime = 0;
}

SEASON3B::CCursedTempleProgressMsgBox::~CCursedTempleProgressMsgBox()
{
    Release();
}

bool SEASON3B::CCursedTempleProgressMsgBox::Create(DWORD dwElapseTime, float fPriority)
{
    int x, y, width, height;

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    SetAddCallbackFunc();

    m_dwElapseTime = dwElapseTime;
    m_dwStartTime = timeGetTime();
    m_dwEndTime = m_dwStartTime + m_dwElapseTime;

    SetCanMove(true);

    return true;
}

void SEASON3B::CCursedTempleProgressMsgBox::Release()
{
}

void SEASON3B::CCursedTempleProgressMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CCursedTempleProgressMsgBox::ClosingProcess, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_CLOSINGPROCESS);
    AddCallbackFunc(SEASON3B::CCursedTempleProgressMsgBox::CompleteProcess, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_COMPLETEPROCESS);
}

void SEASON3B::CCursedTempleProgressMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    int iOrigSize = m_MsgDataList.size();
    int iLine = SeparateText(strMsg, dwColor, byFontType);
    int iSize = m_MsgDataList.size();

    if (iSize > 2)
    {
        float height = GetSize().cy;

        if (iOrigSize < 2)
        {
            iLine = iLine + iOrigSize - 2;
        }

        height += (MSGBOX_MIDDLE_HEIGHT * iLine);
        SetSize(GetSize().cx, height);
    }
}

int SEASON3B::CCursedTempleProgressMsgBox::SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    

    SIZE TextSize;
    size_t TextExtentWidth;
    int iLine = 0;

    GetTextExtentPoint32(g_pRenderText->GetFontDC(), strMsg.c_str(), strMsg.size(), &TextSize);
    TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);

    if (TextExtentWidth <= MSGBOX_TEXT_MAXWIDTH)
    {
        auto* pMsg = new MSGBOX_TEXTDATA;
        pMsg->strMsg = strMsg;
        pMsg->dwColor = dwColor;
        pMsg->byFontType = byFontType;
        m_MsgDataList.push_back(pMsg);

        iLine = 1;
        return iLine;
    }

    type_string strCutText, strRemainText;
    strRemainText = strMsg;

    bool bLoop = true;
    while (bLoop)
    {
        int prev_offset = 0;
        for (int cur_offset = 0; cur_offset < (int)strRemainText.size(); )
        {
            prev_offset = cur_offset;
            size_t offset = _mbclen((const unsigned char*)(strRemainText.c_str() + cur_offset));
            cur_offset += offset;

            type_string strTemp(strRemainText, 0, cur_offset/* size */);

            GetTextExtentPoint32(g_pRenderText->GetFontDC(), strTemp.c_str(), strTemp.size(), &TextSize);
            TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);

            if (TextExtentWidth > MSGBOX_TEXT_MAXWIDTH && cur_offset != 0)
            {
                strCutText = type_string(strRemainText, 0, prev_offset/* size */);
                strRemainText = type_string(strRemainText, prev_offset, strRemainText.size() - prev_offset/* size */);

                auto* pMsg = new MSGBOX_TEXTDATA;
                pMsg->strMsg = strCutText;
                pMsg->dwColor = dwColor;
                pMsg->byFontType = byFontType;
                m_MsgDataList.push_back(pMsg);
                iLine++;

                GetTextExtentPoint32(g_pRenderText->GetFontDC(), strRemainText.c_str(), strRemainText.size(), &TextSize);
                TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);

                if (TextExtentWidth <= MSGBOX_TEXT_MAXWIDTH)
                {
                    auto* pMsg = new MSGBOX_TEXTDATA;
                    pMsg->strMsg = strRemainText;
                    pMsg->dwColor = dwColor;
                    pMsg->byFontType = byFontType;
                    m_MsgDataList.push_back(pMsg);
                    iLine++;

                    bLoop = false;
                    break;
                }

                break;
            }
        }
    }

    return iLine;
}

bool SEASON3B::CCursedTempleProgressMsgBox::Update()
{
    g_pMainFrame->UpdateItemHotKey();

    DWORD dwTime = timeGetTime();
    if (dwTime >= m_dwEndTime)
    {
        g_MessageBox->SendEvent(this, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_COMPLETEPROCESS);
    }

    if (CheckHeroAction() == false || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM) == false)
    {
        g_MessageBox->SendEvent(this, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_CLOSINGPROCESS);
    }

    return true;
}

void SEASON3B::CCursedTempleProgressMsgBox::SetNpcIndex(DWORD dwIndex)
{
    m_dwNpcIndex = dwIndex;
}

DWORD SEASON3B::CCursedTempleProgressMsgBox::GetNpcIndex()
{
    return m_dwNpcIndex;
}

CALLBACK_RESULT SEASON3B::CCursedTempleProgressMsgBox::ClosingProcess(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    g_CursedTemple->SetGaugebarEnabled(false);
    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CCursedTempleProgressMsgBox::CompleteProcess(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CCursedTempleProgressMsgBox*>(pOwner);
    if (pMsgBox == NULL)
    {
        return CALLBACK_CONTINUE;
    }

    SocketClient->ToGameServer()->SendTalkToNpcRequest(pMsgBox->GetNpcIndex());

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    g_CursedTemple->SetGaugebarCloseTimer();
    return CALLBACK_CONTINUE;
}

bool SEASON3B::CCursedTempleProgressMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderProgress();
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CCursedTempleProgressMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    if (m_MsgDataList.size() > 2)
    {
        int iCount = m_MsgDataList.size() - 2;
        for (int i = 0; i < iCount; ++i)
        {
            RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
            y += height;
        }
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CCursedTempleProgressMsgBox::RenderTexts()
{
    

    float x, y;

    x = GetPos().x; y = GetPos().y + MSGBOX_TEXT_TOP_BLANK;
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        SIZE TextSize;
        size_t TextExtentWidth, TextExtentHeight;

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), (*vi)->strMsg.c_str(), (*vi)->strMsg.size(), &TextSize);
        TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);
        TextExtentHeight = (size_t)(TextSize.cy / g_fScreenRate_y);

        x = GetPos().x + (MSGBOX_WIDTH / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void SEASON3B::CCursedTempleProgressMsgBox::RenderProgress()
{
    DWORD dwTime = timeGetTime();
    float fProgress = (float)(dwTime - m_dwStartTime) / m_dwElapseTime;

    float x, y;
    x = GetPos().x + MSGBOX_WIDTH / 2 - 160.f / 2;
    y = GetPos().y + GetSize().cy - 50.f;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_PROGRESS_BG, x, y, 160.f, 18.f);
    x += 5.f;
    y += 5.f;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_PROGRESS_BAR, x, y, 150.f * fProgress, 8.f);
}

bool SEASON3B::CCursedTempleProgressMsgBox::CheckHeroAction()
{
    if (g_isCharacterBuff((&Hero->Object), eDeBuff_Harden)
        || g_isCharacterBuff((&Hero->Object), eDeBuff_Stun)
        || g_isCharacterBuff((&Hero->Object), eDeBuff_CursedTempleRestraint)
        || g_isCharacterBuff((&Hero->Object), eDeBuff_Sleep)
        )
    {
        return false;
    }

    int action = Hero->Object.CurrentAction;

    if (!(action >= PLAYER_SET && action <= PLAYER_STOP_RIDE_WEAPON)
        && !(action == PLAYER_SHOCK)
        && !(action == PLAYER_FENRIR_STAND)
        && !(action == PLAYER_FENRIR_STAND_TWO_SWORD)
        && !(action == PLAYER_FENRIR_STAND_ONE_RIGHT)
        && !(action == PLAYER_FENRIR_STAND_ONE_LEFT)
        && !(action == PLAYER_DARKLORD_STAND)
        && !(action == PLAYER_STOP_RIDE_HORSE)
        && !(action == PLAYER_ATTACK_STRIKE)
        && !(action == PLAYER_STOP_TWO_HAND_SWORD_TWO)
        && !(action >= PLAYER_RAGE_FENRIR_STAND && action <= PLAYER_RAGE_FENRIR_STAND_ONE_LEFT)
        && !(action == PLAYER_RAGE_UNI_STOP_ONE_RIGHT))
    {
        return false;
    }

    return true;
}

SEASON3B::CDuelMsgBox::CDuelMsgBox()
{
}

SEASON3B::CDuelMsgBox::~CDuelMsgBox()
{
}

bool SEASON3B::CDuelMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 70;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_MIDDLE_HEIGHT * 7 + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    return true;
}

void SEASON3B::CDuelMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CDuelMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CDuelMsgBox::OkBtnDown, MSGBOX_EVENT_USER_CUSTOM_DUEL_OK);
    AddCallbackFunc(SEASON3B::CDuelMsgBox::CancelBtnDown, MSGBOX_EVENT_USER_CUSTOM_DUEL_CANCEL);
    AddCallbackFunc(SEASON3B::CDuelMsgBox::CancelBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
}

void SEASON3B::CDuelMsgBox::SetButtonInfo()
{
    float x, y, width, height;
    width = MSGBOX_BTN_WIDTH;
    height = MSGBOX_BTN_HEIGHT;

    x = GetPos().x + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    x = GetPos().x + (GetSize().cx / 2) + (((GetSize().cx / 2) - MSGBOX_BTN_WIDTH) / 2);
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_CANCEL, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
}

void SEASON3B::CDuelMsgBox::Release()
{
}

bool SEASON3B::CDuelMsgBox::Update()
{
    m_BtnOk.Update();
    m_BtnCancel.Update();

    return true;
}

bool SEASON3B::CDuelMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButton();
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CDuelMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x + (GetSize().cx / 2) - 74;
    y = GetPos().y + 15;
    width = 148;
    height = 138;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_DUEL_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < 7; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CDuelMsgBox::RenderTexts()
{
    wchar_t strDuelID[256];
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 0, 255);
    g_pRenderText->SetFont(g_hFontBold);
    swprintf(strDuelID, L"[%s]", g_DuelMgr.GetDuelPlayerID(DUEL_ENEMY));
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 115, strDuelID, MSGBOX_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 135, GlobalText[910], MSGBOX_WIDTH, 0, RT3_SORT_CENTER);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 151, GlobalText[911], MSGBOX_WIDTH, 0, RT3_SORT_CENTER);
}

void SEASON3B::CDuelMsgBox::RenderButton()
{
    m_BtnOk.Render();
    m_BtnCancel.Render();
}

CALLBACK_RESULT SEASON3B::CDuelMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CDuelMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_DUEL_OK);
            return CALLBACK_BREAK;
        }
        else if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_DUEL_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CDuelMsgBox::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_DuelMgr.SendDuelRequestAnswer(DUEL_ENEMY, TRUE);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    PlayBuffer(SOUND_CLICK01);
    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CDuelMsgBox::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_DuelMgr.SendDuelRequestAnswer(DUEL_ENEMY, FALSE);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    PlayBuffer(SOUND_CLICK01);
    return CALLBACK_CONTINUE;
}

SEASON3B::CDuelResultMsgBox::CDuelResultMsgBox()
{
    m_szWinnerID[0] = '\0';
    m_szLoserID[0] = '\0';
}

SEASON3B::CDuelResultMsgBox::~CDuelResultMsgBox()
{
}

bool SEASON3B::CDuelResultMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 70;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_MIDDLE_HEIGHT * 7 + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    return true;
}

void SEASON3B::CDuelResultMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CDuelResultMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CDuelResultMsgBox::OkBtnDown, MSGBOX_EVENT_USER_CUSTOM_DUEL_OK);
    AddCallbackFunc(SEASON3B::CDuelResultMsgBox::OkBtnDown, MSGBOX_EVENT_PRESSKEY_ESC);
}

void SEASON3B::CDuelResultMsgBox::SetButtonInfo()
{
    float x, y, width, height;
    width = MSGBOX_BTN_WIDTH;
    height = MSGBOX_BTN_HEIGHT;

    x = GetPos().x + (GetSize().cx / 2) - MSGBOX_BTN_WIDTH / 2;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
}

void SEASON3B::CDuelResultMsgBox::Release()
{
}

bool SEASON3B::CDuelResultMsgBox::Update()
{
    m_BtnOk.Update();

    return true;
}

bool SEASON3B::CDuelResultMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButton();
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CDuelResultMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x + (GetSize().cx / 2) - 74;
    y = GetPos().y + 15;
    width = 148;
    height = 138;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_DUEL_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < 7; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CDuelResultMsgBox::RenderTexts()
{
    wchar_t strDuelID[256];
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 0, 255);
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 100, GlobalText[2694], MSGBOX_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFont);
    swprintf(strDuelID, GlobalText[2695], m_szWinnerID);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 120, strDuelID, MSGBOX_WIDTH, 0, RT3_SORT_CENTER);
    swprintf(strDuelID, GlobalText[2696], m_szLoserID);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 135, strDuelID, MSGBOX_WIDTH, 0, RT3_SORT_CENTER);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 151, GlobalText[2697], MSGBOX_WIDTH, 0, RT3_SORT_CENTER);
}

void SEASON3B::CDuelResultMsgBox::RenderButton()
{
    m_BtnOk.Render();
}

CALLBACK_RESULT SEASON3B::CDuelResultMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CDuelResultMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_DUEL_OK);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CDuelResultMsgBox::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    // 	SendRequestDuelOk(1, g_iDuelPlayerIndex, g_szDuelPlayerID);

    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    PlayBuffer(SOUND_CLICK01);

    return CALLBACK_CONTINUE;
}

void CDuelResultMsgBox::SetIDs(wchar_t* pszWinnerID, wchar_t* pszLoserID)
{
    wcsncpy(m_szWinnerID, pszWinnerID, MAX_ID_SIZE);
    m_szWinnerID[MAX_ID_SIZE] = '\0';
    wcsncpy(m_szLoserID, pszLoserID, MAX_ID_SIZE);
    m_szLoserID[MAX_ID_SIZE] = '\0';
}

CCherryBlossomMsgBox::CCherryBlossomMsgBox()
{
    m_iMiddleCount = 8;
}

CCherryBlossomMsgBox::~CCherryBlossomMsgBox()
{
    Release();
}

bool CCherryBlossomMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    return true;
}

void CCherryBlossomMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();
}

bool CCherryBlossomMsgBox::Update()
{
    m_BtnWhiteCB.Update();
    m_BtnRedCB.Update();
    m_BtnGoldCB.Update();
    m_BtnExit.Update();

    return true;
}

bool CCherryBlossomMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT CCherryBlossomMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CCherryBlossomMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnWhiteCB.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_CB_WHITE);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnRedCB.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_CB_RED);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnGoldCB.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_CB_GOLD);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT CCherryBlossomMsgBox::WhiteCBBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    //	g_pNewUISystem->Show(SEASON3B::INTERFACE_CHERRYBLOSSOM_WINDOW);
    //	g_pCherryBlossom->SetType(SEASON3B::CNewUICherryBlossom::CB_WHITE);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT CCherryBlossomMsgBox::RedCBBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    //	g_pNewUISystem->Show(SEASON3B::INTERFACE_CHERRYBLOSSOM_WINDOW);
    //	g_pCherryBlossom->SetType(SEASON3B::CNewUICherryBlossom::CB_RED);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT CCherryBlossomMsgBox::GodCBBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    //	g_pNewUISystem->Show(SEASON3B::INTERFACE_CHERRYBLOSSOM_WINDOW);
    //	g_pCherryBlossom->SetType(SEASON3B::CNewUICherryBlossom::CB_GOLD);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT CCherryBlossomMsgBox::ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    //	SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_CONTINUE;
}

void CCherryBlossomMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CCherryBlossomMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CCherryBlossomMsgBox::WhiteCBBtnDown, MSGBOX_EVENT_USER_CUSTOM_CB_WHITE);
    AddCallbackFunc(SEASON3B::CCherryBlossomMsgBox::RedCBBtnDown, MSGBOX_EVENT_USER_CUSTOM_CB_RED);
    AddCallbackFunc(SEASON3B::CCherryBlossomMsgBox::GodCBBtnDown, MSGBOX_EVENT_USER_CUSTOM_CB_GOLD);
    AddCallbackFunc(SEASON3B::CCherryBlossomMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void CCherryBlossomMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 50;
    m_BtnWhiteCB.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnWhiteCB.SetText(GlobalText[2542]);

    y = GetPos().y + 100;
    m_BtnRedCB.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnRedCB.SetText(GlobalText[2543]);

    y = GetPos().y + 150;
    m_BtnGoldCB.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnGoldCB.SetText(GlobalText[2544]);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    // 1002 "닫기"
    m_BtnExit.SetText(GlobalText[1002]);
}

void CCherryBlossomMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void CCherryBlossomMsgBox::RenderTexts()
{
    wchar_t title[256];

    MONSTER_SCRIPT* m = &MonsterScript[450];
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 0, 255);
    g_pRenderText->SetFont(g_hFontBold);
    swprintf(title, L"%s", m->Name);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 10, title, MSGBOX_WIDTH, 0, RT3_SORT_CENTER);

    wchar_t titleinfo[256];
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 0, 255);
    g_pRenderText->SetFont(g_hFontBold);
    swprintf(titleinfo, L"%s", GlobalText[2544]);
    g_pRenderText->RenderText(GetPos().x, GetPos().y + 70, titleinfo, MSGBOX_WIDTH, 0, RT3_SORT_CENTER);
}

void CCherryBlossomMsgBox::RenderButtons()
{
    m_BtnWhiteCB.Render();
    m_BtnRedCB.Render();
    m_BtnGoldCB.Render();
    m_BtnExit.Render();
}

bool SEASON3B::CTradeZenMsgBoxLayout::SetLayout()
{
    CNewUITextInputMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL, INPUTBOX_TYPE_NUMBER, INPUTBOX_WIDTH, INPUTBOX_HEIGHT, INPUTBOX_TEXTLIMIT))
        return false;

    pMsgBox->SetInputBoxOption(UIOPTION_NUMBERONLY | UIOPTION_PAINTBACK);
    pMsgBox->AddMsg(GlobalText[422]);
    pMsgBox->AddCallbackFunc(CTradeZenMsgBoxLayout::ReturnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CTradeZenMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CTradeZenMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    return true;
}

CALLBACK_RESULT SEASON3B::CTradeZenMsgBoxLayout::ProcessOk(class CNewUIMessageBoxBase* pOwner)
{
    auto* pMsgBox = dynamic_cast<CNewUITextInputMsgBox*>(pOwner);
    wchar_t strText[MAX_TEXT_LENGTH] = { 0, };
    pMsgBox->GetInputBoxText(strText);
    if (wcslen(strText) == 0)
        return CALLBACK_CONTINUE;

    int iInputZen = _wtoi(strText);
    if (iInputZen == 0)
        return CALLBACK_CONTINUE;

    g_pTrade->SendRequestMyGoldInput(iInputZen);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CTradeZenMsgBoxLayout::ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner);
}

CALLBACK_RESULT SEASON3B::CTradeZenMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner);
}

CALLBACK_RESULT SEASON3B::CTradeZenMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool SEASON3B::CZenReceiptMsgBoxLayout::SetLayout()
{
    CNewUITextInputMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL, INPUTBOX_TYPE_NUMBER, INPUTBOX_WIDTH, INPUTBOX_HEIGHT, INPUTBOX_TEXTLIMIT))
        return false;

    pMsgBox->SetInputBoxOption(UIOPTION_NUMBERONLY | UIOPTION_PAINTBACK);
    pMsgBox->AddMsg(GlobalText[420]);
    pMsgBox->AddCallbackFunc(CZenReceiptMsgBoxLayout::ReturnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CZenReceiptMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CZenReceiptMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    return true;
}

CALLBACK_RESULT SEASON3B::CZenReceiptMsgBoxLayout::ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner, xParam);
}

CALLBACK_RESULT SEASON3B::CZenReceiptMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner, xParam);
}

CALLBACK_RESULT SEASON3B::CZenReceiptMsgBoxLayout::ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUITextInputMsgBox*>(pOwner);
    wchar_t strText[MAX_TEXT_LENGTH] = { 0, };
    pMsgBox->GetInputBoxText(strText);
    if (wcslen(strText) == 0)
    {
        return CALLBACK_CONTINUE;
    }

    int iInputZen = _wtoi(strText);
    if (iInputZen == 0)
    {
        return CALLBACK_CONTINUE;
    }

    if (iInputZen <= (int)CharacterMachine->Gold)
    {
        SocketClient->ToGameServer()->SendVaultMoveMoneyRequest(0, iInputZen);
    }
    else
    {
        SEASON3B::CreateOkMessageBox(GlobalText[423]);
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CZenReceiptMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool SEASON3B::CZenPaymentMsgBoxLayout::SetLayout()
{
    CNewUITextInputMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL, INPUTBOX_TYPE_NUMBER, INPUTBOX_WIDTH, INPUTBOX_HEIGHT, INPUTBOX_TEXTLIMIT))
        return false;

    pMsgBox->SetInputBoxOption(UIOPTION_NUMBERONLY | UIOPTION_PAINTBACK);
    pMsgBox->AddMsg(GlobalText[421]);
    pMsgBox->AddCallbackFunc(CZenPaymentMsgBoxLayout::ReturnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CZenPaymentMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CZenPaymentMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    return true;
}

CALLBACK_RESULT SEASON3B::CZenPaymentMsgBoxLayout::ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner, xParam);
}

CALLBACK_RESULT SEASON3B::CZenPaymentMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner, xParam);
}

CALLBACK_RESULT SEASON3B::CZenPaymentMsgBoxLayout::ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUITextInputMsgBox*>(pOwner);
    wchar_t strText[MAX_TEXT_LENGTH] = { 0, };
    pMsgBox->GetInputBoxText(strText);
    if (wcslen(strText) == 0)
    {
        return CALLBACK_CONTINUE;
    }

    int iInputZen = _wtoi(strText);
    if (iInputZen == 0)
    {
        return CALLBACK_CONTINUE;
    }

    if (iInputZen <= CharacterMachine->StorageGold && CharacterMachine->Gold + iInputZen <= 2000000000
        )
    {
        if (!g_pStorageInventory->IsStorageLocked()
            || g_pStorageInventory->IsCorrectPassword())
        {
            SocketClient->ToGameServer()->SendVaultMoveMoneyRequest(1, iInputZen);
        }
        else
        {
            g_pStorageInventory->SetBackupTakeZen(iInputZen);
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CPasswordKeyPadMsgBoxLayout));
        }
    }
    else if (CharacterMachine->Gold + iInputZen > 2000000000)
    {
    }
    else
    {
        SEASON3B::CreateOkMessageBox(GlobalText[423]);
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CZenPaymentMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool SEASON3B::CPersonalShopItemValueMsgBoxLayout::SetLayout()
{
    CNewUITextInputMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL, INPUTBOX_TYPE_NUMBER, INPUTBOX_WIDTH, INPUTBOX_HEIGHT, INPUTBOX_TEXTLIMIT))
        return false;

    pMsgBox->SetInputBoxOption(UIOPTION_NUMBERONLY | UIOPTION_PAINTBACK);
    pMsgBox->AddMsg(GlobalText[1129]);
    pMsgBox->AddCallbackFunc(CPersonalShopItemValueMsgBoxLayout::ReturnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CPersonalShopItemValueMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CPersonalShopItemValueMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    return true;
}

CALLBACK_RESULT SEASON3B::CPersonalShopItemValueMsgBoxLayout::ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUITextInputMsgBox*>(pOwner);

    wchar_t strText[MAX_TEXT_LENGTH] = { 0, };

    pMsgBox->GetInputBoxText(strText);

    if (wcslen(strText) == 0)
    {
        return CALLBACK_CONTINUE;
    }

    int iInputZen = _wtoi(strText);
    if (iInputZen == 0)
    {
        return CALLBACK_CONTINUE;
    }

    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
    ITEM* pItem = NULL;
    if (pPickedItem)
    {
        pItem = pPickedItem->GetItem();
    }
    else
    {
        int iSourceIndex = g_pMyShopInventory->GetSourceIndex();
        pItem = g_pMyShopInventory->FindItem(iSourceIndex);
    }

    bool bResult = false;
    if (pItem)
    {
        DWORD dwItemValue = ItemValue(pItem, 2);

        if (iInputZen < (int)dwItemValue)
        {
            bResult = true;
        }
    }

    if (bResult == true)
    {
        SEASON3B::CNewUI3DItemCommonMsgBox* lpMsgBox = NULL;

        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CPersonalShopItemValueCheckMsgBoxLayout), &lpMsgBox);
        if (lpMsgBox)
        {
            wchar_t strText2[MAX_TEXT_LENGTH] = { 0, };
            swprintf(strText2, GlobalText[1132], strText);
            lpMsgBox->AddMsg(strText2, RGBA(255, 0, 0, 255), MSGBOX_FONT_BOLD);
            lpMsgBox->AddMsg(GlobalText[1133]);
            lpMsgBox->SetItemValue(iInputZen);
        }
    }
    else
    {
        if (g_pMyShopInventory->IsEnablePersonalShop() == true)
        {
            SocketClient->ToGameServer()->SendPlayerShopClose();
        }

        CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

        int iSourceIndex = -1, iTargetIndex = -1;
        int shopWndType = 0;

        if (pPickedItem)
        {
            ITEM* pItemObj = pPickedItem->GetItem();
            iSourceIndex = pPickedItem->GetSourceLinealPos();
            iTargetIndex = g_pMyShopInventory->GetTargetIndex();

            if (pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
            {
                SocketClient->ToGameServer()->SendPlayerShopSetItemPrice(iSourceIndex, iInputZen);

                SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSourceIndex, pItemObj, STORAGE_TYPE::MYSHOP, iTargetIndex);
            }
            else if (pPickedItem->GetOwnerInventory() == nullptr)
            {
                SocketClient->ToGameServer()->SendPlayerShopSetItemPrice(iSourceIndex, iInputZen);

                SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSourceIndex, pItemObj, STORAGE_TYPE::MYSHOP, iTargetIndex);
            }
            else if (pPickedItem->GetOwnerInventory() == g_pMyShopInventory->GetInventoryCtrl())
            {
                SocketClient->ToGameServer()->SendPlayerShopSetItemPrice(iSourceIndex, iInputZen);

                SendRequestEquipmentItem(STORAGE_TYPE::MYSHOP, iSourceIndex, pItemObj, STORAGE_TYPE::MYSHOP, iTargetIndex);
            }

            AddPersonalItemPrice(iTargetIndex, iInputZen, g_IsPurchaseShop);
        }
        else
        {
            iSourceIndex = g_pMyShopInventory->GetSourceIndex();
            SocketClient->ToGameServer()->SendPlayerShopSetItemPrice(iSourceIndex, iInputZen);
            AddPersonalItemPrice(iSourceIndex, iInputZen, g_IsPurchaseShop);
        }
    }

    g_pMyShopInventory->SetInputValueTextBox(false);
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CPersonalShopItemValueMsgBoxLayout::ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner, xParam);
}

CALLBACK_RESULT SEASON3B::CPersonalShopItemValueMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner, xParam);
}

CALLBACK_RESULT SEASON3B::CPersonalShopItemValueMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
    g_pMyShopInventory->SetInputValueTextBox(false);
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

bool SEASON3B::CPersonalShopNameMsgBoxLayout::SetLayout()
{
    CNewUITextInputMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL, INPUTBOX_TYPE_TEXT, INPUT_WIDTH, INPUT_HEIGHT, INPUT_TEXTLIMIT))
        return false;

    pMsgBox->SetInputBoxOption(UIOPTION_PAINTBACK);
    pMsgBox->AddMsg(GlobalText[1128]);
    pMsgBox->AddCallbackFunc(CPersonalShopNameMsgBoxLayout::ReturnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CPersonalShopNameMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CPersonalShopNameMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    return true;
}

CALLBACK_RESULT SEASON3B::CPersonalShopNameMsgBoxLayout::ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUITextInputMsgBox*>(pOwner);
    wchar_t strText[MAX_TEXT_LENGTH] = { 0, };
    pMsgBox->GetInputBoxText(strText);
    if (wcslen(strText) == 0)
    {
        return CALLBACK_CONTINUE;
    }

    if (IsCorrectShopTitle(strText))
    {
        wcscpy(g_szPersonalShopTitle, strText);
    }
    else
    {
        g_pSystemLogBox->AddText(GlobalText[1130], SEASON3B::TYPE_SYSTEM_MESSAGE);
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CPersonalShopNameMsgBoxLayout::ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner, xParam);
}

CALLBACK_RESULT SEASON3B::CPersonalShopNameMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner, xParam);
}

CALLBACK_RESULT SEASON3B::CPersonalShopNameMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool SEASON3B::CCastleWithdrawMsgBoxLayout::SetLayout()
{
    CNewUITextInputMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL, INPUTBOX_TYPE_NUMBER, INPUTBOX_WIDTH, INPUTBOX_HEIGHT, INPUTBOX_TEXTLIMIT))
        return false;

    pMsgBox->SetInputBoxOption(UIOPTION_NUMBERONLY | UIOPTION_PAINTBACK);

    pMsgBox->AddMsg(GlobalText[1571]);
    pMsgBox->AddMsg(GlobalText[1570]);

    pMsgBox->AddCallbackFunc(CCastleWithdrawMsgBoxLayout::ReturnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CCastleWithdrawMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CCastleWithdrawMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    return true;
}

CALLBACK_RESULT SEASON3B::CCastleWithdrawMsgBoxLayout::ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUITextInputMsgBox*>(pOwner);
    wchar_t strText[MAX_TEXT_LENGTH] = { 0, };
    pMsgBox->GetInputBoxText(strText);
    if (wcslen(strText) == 0)
    {
        return CALLBACK_CONTINUE;
    }

    DWORD dwInputZen = _wtoi(strText);
    if (dwInputZen == 0)
    {
        return CALLBACK_CONTINUE;
    }

    g_SenatusInfo.DoWithdrawAction(dwInputZen);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CCastleWithdrawMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUITextInputMsgBox*>(pOwner);
    wchar_t strText[MAX_TEXT_LENGTH] = { 0, };
    pMsgBox->GetInputBoxText(strText);
    if (wcslen(strText) == 0)
    {
        return CALLBACK_CONTINUE;
    }

    DWORD dwInputZen = _wtoi(strText);
    if (dwInputZen == 0)
    {
        return CALLBACK_CONTINUE;
    }

    g_SenatusInfo.DoWithdrawAction(dwInputZen);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CCastleWithdrawMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool SEASON3B::CPasswordKeyPadMsgBoxLayout::SetLayout()
{
    CNewUIKeyPadMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(KEYPAD_TYPE_MOVE, 4))
        return false;

    pMsgBox->AddMsg(GlobalText[690]);
    pMsgBox->AddMsg(GlobalText[695]);
    pMsgBox->AddCallbackFunc(CPasswordKeyPadMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CPasswordKeyPadMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    return true;
}

CALLBACK_RESULT SEASON3B::CPasswordKeyPadMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUIKeyPadMsgBox*>(pOwner);

    if (pMsgBox == false)
    {
        return CALLBACK_CONTINUE;
    }

    if (pMsgBox->GetInputSize() == pMsgBox->GetInputLimit())
    {
        WORD wInputNumber = (WORD)_wtoi(pMsgBox->GetInputText());
        SocketClient->ToGameServer()->SendUnlockVault(wInputNumber);
    }
    else
    {
        return CALLBACK_CONTINUE;
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CPasswordKeyPadMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    if (g_pPickedItem)
        g_pPickedItem->ShowPickedItem();

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE))
        g_pStorageInventory->SetItemAutoMove(false);

    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool SEASON3B::CStorageLockKeyPadMsgBoxLayout::SetLayout()
{
    CNewUIKeyPadMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(KEYPAD_TYPE_LOCK_FIRST, 4))
        return false;

    pMsgBox->AddMsg(GlobalText[693]);
    pMsgBox->AddMsg(GlobalText[695]);
    pMsgBox->AddCallbackFunc(CStorageLockKeyPadMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CStorageLockKeyPadMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    return true;
}

CALLBACK_RESULT SEASON3B::CStorageLockKeyPadMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUIKeyPadMsgBox*>(pOwner);

    if (pMsgBox == false)
    {
        return CALLBACK_CONTINUE;
    }

    PlayBuffer(SOUND_CLICK01);

    if (pMsgBox->GetInputSize() == pMsgBox->GetInputLimit())
    {
        if (pMsgBox->IsAllSameNumber() == true)
        {
            pMsgBox->ClearInput();
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
            SEASON3B::CreateOkMessageBox(GlobalText[442]);
            return CALLBACK_BREAK;
        }

        CNewUIKeyPadMsgBox* pKeyPadMsgBox = NULL;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CStorageLockCheckKeyPadMsgBoxLayout), &pKeyPadMsgBox);
        if (pKeyPadMsgBox)
        {
            pKeyPadMsgBox->SetCheckInputText(pMsgBox->GetInputText());
        }
        g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
        return CALLBACK_BREAK;
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CStorageLockKeyPadMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool SEASON3B::CStorageLockCheckKeyPadMsgBoxLayout::SetLayout()
{
    CNewUIKeyPadMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(KEYPAD_TYPE_LOCK_SECOND, 4))
        return false;

    pMsgBox->AddMsg(GlobalText[694]);
    pMsgBox->AddMsg(GlobalText[696]);
    pMsgBox->AddCallbackFunc(CStorageLockCheckKeyPadMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CStorageLockCheckKeyPadMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    return true;
}

CALLBACK_RESULT SEASON3B::CStorageLockCheckKeyPadMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUIKeyPadMsgBox*>(pOwner);

    if (pMsgBox == false)
    {
        return CALLBACK_CONTINUE;
    }

    PlayBuffer(SOUND_CLICK01);

    if (pMsgBox->GetInputSize() == pMsgBox->GetInputLimit())
    {
        if (pMsgBox->IsAllSameNumber() == true)
        {
            pMsgBox->ClearInput();
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
            SEASON3B::CreateOkMessageBox(GlobalText[442]);
            return CALLBACK_BREAK;
        }

        if (pMsgBox->IsCheckInput() == true)
        {
            WORD wInputNumber = (WORD)_wtoi(pMsgBox->GetInputText());

            CNewUITextInputMsgBox* pPassword = NULL;
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CStorageLockMsgBoxLayout), &pPassword);
            if (pPassword)
            {
                pPassword->SetPassword(wInputNumber);
            }

            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
            return CALLBACK_BREAK;
        }
        else
        {
            pMsgBox->ClearInput();
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
            SEASON3B::CreateOkMessageBox(GlobalText[445]);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CStorageLockCheckKeyPadMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool SEASON3B::CStorageLockMsgBoxLayout::SetLayout()
{
    CNewUITextInputMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox) return false;

    int _temp = (g_iLengthAuthorityCode / 10) <= 0 ? 1 : (g_iLengthAuthorityCode / 10);
    int _width = INPUTBOX_WIDTH * _temp;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL, INPUTBOX_TYPE_NUMBER, _width, INPUTBOX_HEIGHT, g_iLengthAuthorityCode, true))
    {
        return false;
    }

    pMsgBox->SetInputBoxOption(UIOPTION_PAINTBACK);
    pMsgBox->AddMsg(GlobalText[691]);
    pMsgBox->AddMsg(GlobalText[697]);
    pMsgBox->AddCallbackFunc(SEASON3B::CStorageLockMsgBoxLayout::ReturnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(SEASON3B::CStorageLockMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(SEASON3B::CStorageLockMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
    return true;
}

CALLBACK_RESULT SEASON3B::CStorageLockMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner, xParam);
}

CALLBACK_RESULT SEASON3B::CStorageLockMsgBoxLayout::ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner, xParam);
}

CALLBACK_RESULT SEASON3B::CStorageLockMsgBoxLayout::ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUITextInputMsgBox*>(pOwner);

    if (pMsgBox == false)
    {
        return CALLBACK_CONTINUE;
    }

    wchar_t strText[20] = { 0, };

    pMsgBox->GetInputBoxText(strText);
    int iInputTextSize = wcslen(strText);

    if (iInputTextSize > 0)
    {
        SocketClient->ToGameServer()->SendSetVaultPin(pMsgBox->GetPassword(), strText);
    }
    else
    {
        return CALLBACK_CONTINUE;
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CStorageLockMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool SEASON3B::CStorageLockFinalKeyPadMsgBoxLayout::SetLayout()
{
    CNewUIKeyPadMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(KEYPAD_TYPE_LOCK_FINAL, g_iLengthAuthorityCode))
        return false;

    pMsgBox->AddMsg(GlobalText[691]);
    pMsgBox->AddMsg(GlobalText[697]);
    pMsgBox->AddCallbackFunc(CStorageLockFinalKeyPadMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CStorageLockFinalKeyPadMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    return true;
}

CALLBACK_RESULT SEASON3B::CStorageLockFinalKeyPadMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUIKeyPadMsgBox*>(pOwner);

    if (pMsgBox == false)
    {
        return CALLBACK_CONTINUE;
    }

    PlayBuffer(SOUND_CLICK01);

    if (pMsgBox->GetInputSize() == pMsgBox->GetInputLimit())
    {
        if (pMsgBox->GetStoragePassword() != 0)
        {
            SocketClient->ToGameServer()->SendSetVaultPin(pMsgBox->GetStoragePassword(), pMsgBox->GetInputText());
        }
        g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

        return CALLBACK_BREAK;
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CStorageLockFinalKeyPadMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

bool SEASON3B::CStorageUnlockMsgBoxLayout::SetLayout()
{
    CNewUITextInputMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox) return false;

    int _temp = (g_iLengthAuthorityCode / 10) <= 0 ? 1 : (g_iLengthAuthorityCode / 10);
    int _width = INPUTBOX_WIDTH * _temp;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL, INPUTBOX_TYPE_NUMBER, _width, INPUTBOX_HEIGHT, g_iLengthAuthorityCode, true))
    {
        return false;
    }

    pMsgBox->SetInputBoxOption(UIOPTION_PAINTBACK);

    pMsgBox->AddMsg(GlobalText[242]);
    pMsgBox->AddMsg(GlobalText[697]);

    pMsgBox->AddCallbackFunc(CStorageUnlockMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CStorageUnlockMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    return true;
}

CALLBACK_RESULT SEASON3B::CStorageUnlockMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUITextInputMsgBox*>(pOwner);

    if (pMsgBox == false)
    {
        return CALLBACK_CONTINUE;
    }

    wchar_t strText[20] = { 0, };

    pMsgBox->GetInputBoxText(strText);
    int iInputTextSize = wcslen(strText);

    if (iInputTextSize > 0)
    {
        SocketClient->ToGameServer()->SendRemoveVaultPin(strText);
    }
    else
    {
        return CALLBACK_CONTINUE;
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CStorageUnlockMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool SEASON3B::CStorageUnlockKeyPadMsgBoxLayout::SetLayout()
{
    CNewUIKeyPadMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;
    if (false == pMsgBox->Create(KEYPAD_TYPE_UNLOCK, g_iLengthAuthorityCode))
        return false;

    pMsgBox->AddMsg(GlobalText[242]);
    pMsgBox->AddMsg(GlobalText[697]);

    pMsgBox->AddCallbackFunc(CStorageUnlockKeyPadMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CStorageUnlockKeyPadMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    return true;
}

CALLBACK_RESULT SEASON3B::CStorageUnlockKeyPadMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUIKeyPadMsgBox*>(pOwner);

    if (pMsgBox == false)
    {
        return CALLBACK_CONTINUE;
    }

    if (pMsgBox->GetInputSize() == pMsgBox->GetInputLimit())
    {
        SocketClient->ToGameServer()->SendRemoveVaultPin(pMsgBox->GetInputText());
    }
    else
    {
        return CALLBACK_CONTINUE;
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CStorageUnlockKeyPadMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool SEASON3B::CUseFruitCheckMsgBoxLayout::SetLayout()
{
    CUseFruitCheckMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CGemIntegrationMsgBoxLayout::SetLayout()
{
    CGemIntegrationMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CGemIntegrationUnityMsgBoxLayout::SetLayout()
{
    CGemIntegrationUnityMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CGemIntegrationDisjointMsgBoxLayout::SetLayout()
{
    CGemIntegrationDisjointMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CBloodCastleResultMsgBoxLayout::SetLayout()
{
    CBloodCastleResultMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CDevilSquareRankMsgBoxLayout::SetLayout()
{
    CDevilSquareRankMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CChaosCastleResultMsgBoxLayout::SetLayout()
{
    CChaosCastleResultMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CChaosMixMenuMsgBoxLayout::SetLayout()
{
    CChaosMixMenuMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CDialogMsgBoxLayout::SetLayout()
{
    CDialogMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CCrownSwitchPopLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(GlobalText[1484]);

    return true;
}

bool SEASON3B::CCrownSwitchPushLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(GlobalText[1485]);

    return true;
}

bool SEASON3B::CCrownSwitchOtherPushLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    return true;
}

bool SEASON3B::CSealRegisterStartLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CSealRegisterSuccessLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(GlobalText[1490]);

    return true;
}

bool SEASON3B::CSealRegisterFailLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    return true;
}

bool SEASON3B::CSealRegisterOtherLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(GlobalText[1492]);

    return true;
}

bool SEASON3B::CSealRegisterOtherCampLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(GlobalText[1982]);

    return true;
}

bool SEASON3B::CCrownDefenseRemoveLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(GlobalText[1493]);

    return true;
}

bool SEASON3B::CCrownDefenseCreateLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(GlobalText[1494]);

    return true;
}

bool SEASON3B::CCursedTempleHolicItemGetLayout::SetLayout()
{
    CCursedTempleProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(10000))
        return false;

    pMsgBox->AddMsg(GlobalText[2417]);

    return true;
}

bool SEASON3B::CCursedTempleHolicItemSaveLayout::SetLayout()
{
    CCursedTempleProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(10000))
        return false;

    pMsgBox->AddMsg(GlobalText[2418]);

    return true;
}

bool SEASON3B::CTrainerMenuMsgBoxLayout::SetLayout()
{
    CTrainerMenuMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CTrainerRecoverMsgBoxLayout::SetLayout()
{
    CTrainerRecoverMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CElpisMsgBoxLayout::SetLayout()
{
    CElpisMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CSystemMenuMsgBoxLayout::SetLayout()
{
    CSystemMenuMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CDuelMsgBoxLayout::SetLayout()
{
    CDuelMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CDuelResultMsgBoxLayout::SetLayout()
{
    CDuelResultMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool CCherryBlossomMsgBoxLayout::SetLayout()
{
    CCherryBlossomMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CSeedMasterMenuMsgBoxLayout::SetLayout()
{
    CSeedMasterMenuMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CSeedInvestigatorMenuMsgBoxLayout::SetLayout()
{
    CSeedInvestigatorMenuMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CResetCharacterPointMsgBoxLayout::SetLayout()
{
    CResetCharacterPointMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CDelgardoMainMenuMsgBoxLayout::SetLayout()
{
    CDelgardoMainMenuMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool SEASON3B::CLuckyTradeMenuMsgBoxLayout::SetLayout()
{
    CLuckyTradeMenuMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

SEASON3B::CLuckyTradeMenuMsgBox::CLuckyTradeMenuMsgBox()
{
    m_iMiddleCount = 7;
}

SEASON3B::CLuckyTradeMenuMsgBox::~CLuckyTradeMenuMsgBox()
{
    Release();
}

void SEASON3B::CLuckyTradeMenuMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();
}

bool SEASON3B::CLuckyTradeMenuMsgBox::Update()
{
    m_BtnTrade.Update();
    m_BtnRefinery.Update();
    m_BtnExit.Update();

    return true;
}

bool SEASON3B::CLuckyTradeMenuMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();

    return true;
}

bool SEASON3B::CLuckyTradeMenuMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT SEASON3B::CLuckyTradeMenuMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    CLuckyTradeMenuMsgBox* pMsgBox = dynamic_cast<CLuckyTradeMenuMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnTrade.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_LUCKYITEM_TRADE);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnRefinery.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_LUCKYITEM_REFINERY);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CLuckyTradeMenuMsgBox::LuckyItemTradeBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_pLuckyItemWnd->SetAct(eLuckyItemType_Trade);
    g_pNewUISystem->Show(SEASON3B::INTERFACE_LUCKYITEMWND);
    //g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CLuckyTradeMenuMsgBox::LuckyItemRefineryBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_pLuckyItemWnd->SetAct(eLuckyItemType_Refinery);
    g_pNewUISystem->Show(SEASON3B::INTERFACE_LUCKYITEMWND);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CLuckyTradeMenuMsgBox::ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void SEASON3B::CLuckyTradeMenuMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CLuckyTradeMenuMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CLuckyTradeMenuMsgBox::LuckyItemTradeBtnDown, MSGBOX_EVENT_USER_CUSTOM_LUCKYITEM_TRADE);
    AddCallbackFunc(SEASON3B::CLuckyTradeMenuMsgBox::LuckyItemRefineryBtnDown, MSGBOX_EVENT_USER_CUSTOM_LUCKYITEM_REFINERY);
    AddCallbackFunc(SEASON3B::CLuckyTradeMenuMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void SEASON3B::CLuckyTradeMenuMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 85;
    m_BtnTrade.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnTrade.SetText(L"럭키아이템 교환");	// "GlobalText"

    y = GetPos().y + 120;
    m_BtnRefinery.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnRefinery.SetText(L"럭키아이템 제련");	// "GlobalText"

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(GlobalText[1002]);
}

void SEASON3B::CLuckyTradeMenuMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CLuckyTradeMenuMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    swprintf(szText, L"럭키아이템 교환NPC");	// "LuckyItem Trade NPC"
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 15;
    g_pRenderText->SetFont(g_hFont);
    swprintf(szText, L"럭키아이템으로 교환하거나 제련할 수 있습니?");
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void SEASON3B::CLuckyTradeMenuMsgBox::RenderButtons()
{
    m_BtnTrade.Render();
    m_BtnRefinery.Render();
    m_BtnExit.Render();
}

//////////////////////////////////////////////////////////////////////////

SEASON3B::CTrainerMenuMsgBox::CTrainerMenuMsgBox()
{
    m_iMiddleCount = 7;
}

SEASON3B::CTrainerMenuMsgBox::~CTrainerMenuMsgBox()
{
    Release();
}

bool SEASON3B::CTrainerMenuMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();

    return true;
}

void SEASON3B::CTrainerMenuMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();
}

bool SEASON3B::CTrainerMenuMsgBox::Update()
{
    m_BtnRecover.Update();
    m_BtnRevive.Update();
    m_BtnExit.Update();

    return true;
}

bool SEASON3B::CTrainerMenuMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT SEASON3B::CTrainerMenuMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CTrainerMenuMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnRecover.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_RECOVER);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnRevive.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_REVIVE);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CTrainerMenuMsgBox::RecoverBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CTrainerRecoverMsgBoxLayout));

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CTrainerMenuMsgBox::ReviveBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_TRAINER);
    g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CTrainerMenuMsgBox::ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void SEASON3B::CTrainerMenuMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CTrainerMenuMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CTrainerMenuMsgBox::RecoverBtnDown, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_RECOVER);
    AddCallbackFunc(SEASON3B::CTrainerMenuMsgBox::ReviveBtnDown, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_REVIVE);
    AddCallbackFunc(SEASON3B::CTrainerMenuMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void SEASON3B::CTrainerMenuMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 85;
    m_BtnRecover.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnRecover.SetText(GlobalText[1204]);

    y = GetPos().y + 120;
    m_BtnRevive.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnRevive.SetText(GlobalText[1205]);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(GlobalText[1002]);
}

void SEASON3B::CTrainerMenuMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CTrainerMenuMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    swprintf(szText, GlobalText[1227]);
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 15;
    g_pRenderText->SetFont(g_hFont);
    swprintf(szText, GlobalText[1202]);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    swprintf(szText, GlobalText[1203], Hero->ID);
    g_pRenderText->RenderText(fPos_x, fPos_y + 2 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void SEASON3B::CTrainerMenuMsgBox::RenderButtons()
{
    m_BtnRecover.Render();
    m_BtnRevive.Render();
    m_BtnExit.Render();
}

SEASON3B::CTrainerRecoverMsgBox::CTrainerRecoverMsgBox()
{
    m_iMiddleCount = 7;
}

SEASON3B::CTrainerRecoverMsgBox::~CTrainerRecoverMsgBox()
{
    Release();
}

bool SEASON3B::CTrainerRecoverMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();

    return true;
}

void SEASON3B::CTrainerRecoverMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();
}

bool SEASON3B::CTrainerRecoverMsgBox::Update()
{
    m_BtnRecoverDarkSpirit.Update();
    m_BtnRecoverDarkHorse.Update();
    m_BtnExit.Update();

    return true;
}

bool SEASON3B::CTrainerRecoverMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT SEASON3B::CTrainerRecoverMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CTrainerRecoverMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnRecoverDarkSpirit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_RECOVER_DARKSPRIT);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnRecoverDarkHorse.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_RECOVER_DARKHORSE);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CTrainerRecoverMsgBox::RecoverDarkSpiritrBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    npcBreeder::RecoverPet(REVIVAL_DARKSPIRIT);
    SocketClient->ToGameServer()->SendCloseNpcRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CTrainerRecoverMsgBox::RecoverDarkHorseBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    npcBreeder::RecoverPet(REVIVAL_DARKHORSE);
    SocketClient->ToGameServer()->SendCloseNpcRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CTrainerRecoverMsgBox::ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void SEASON3B::CTrainerRecoverMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CTrainerRecoverMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CTrainerRecoverMsgBox::RecoverDarkSpiritrBtnDown, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_RECOVER_DARKSPRIT);
    AddCallbackFunc(SEASON3B::CTrainerRecoverMsgBox::RecoverDarkHorseBtnDown, MSGBOX_EVENT_USER_CUSTOM_TRAINER_MENU_RECOVER_DARKHORSE);
    AddCallbackFunc(SEASON3B::CTrainerRecoverMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void SEASON3B::CTrainerRecoverMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;

    y = GetPos().y + 65;
    m_BtnRecoverDarkHorse.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnRecoverDarkHorse.SetText(GlobalText[1187]);

    y = GetPos().y + 115;
    m_BtnRecoverDarkSpirit.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnRecoverDarkSpirit.SetText(GlobalText[1214]);

    btnhalfwidth = MSGBOX_BTN_EMPTY_SMALL_WIDTH / 2.f;
    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(GlobalText[1002]);
}

void SEASON3B::CTrainerRecoverMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CTrainerRecoverMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    swprintf(szText, GlobalText[1227]);
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 15;
    g_pRenderText->SetFont(g_hFont);
    swprintf(szText, GlobalText[1228]);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    g_pRenderText->SetTextColor(206, 192, 146, 255);
    npcBreeder::CalcRecoveryZen(REVIVAL_DARKHORSE, szText);
    g_pRenderText->RenderText(fPos_x, fPos_y + 75, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    npcBreeder::CalcRecoveryZen(REVIVAL_DARKSPIRIT, szText);
    g_pRenderText->RenderText(fPos_x, fPos_y + 125, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void SEASON3B::CTrainerRecoverMsgBox::RenderButtons()
{
    m_BtnRecoverDarkSpirit.Render();
    m_BtnRecoverDarkHorse.Render();
    m_BtnExit.Render();
}

SEASON3B::CElpisMsgBox::CElpisMsgBox()
{
    m_iMiddleCount = 12;
    m_iMessageType = 0;
}

SEASON3B::CElpisMsgBox::~CElpisMsgBox()
{
    Release();
}

bool SEASON3B::CElpisMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();

    return true;
}

void SEASON3B::CElpisMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();
}

bool SEASON3B::CElpisMsgBox::Update()
{
    m_BtnAboutRefinary.Update();
    m_BtnAboutJewelOfHarmony.Update();
    m_BtnRefine.Update();
    m_BtnExit.Update();

    return true;
}

bool SEASON3B::CElpisMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT SEASON3B::CElpisMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CElpisMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnAboutRefinary.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_REFINARY);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnAboutJewelOfHarmony.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_JEWELOFHARMONY);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnRefine.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_ELPIS_REFINE);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CElpisMsgBox::AboutRefinaryBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CElpisMsgBox*>(pOwner);
    if (pMsgBox)
    {
        pMsgBox->SetMessageType(MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_REFINARY);
    }

    PlayBuffer(SOUND_CLICK01);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CElpisMsgBox::AboutJewelOfHarmonyBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CElpisMsgBox*>(pOwner);
    if (pMsgBox)
    {
        pMsgBox->SetMessageType(MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_JEWELOFHARMONY);
    }

    PlayBuffer(SOUND_CLICK01);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CElpisMsgBox::RefineBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_ELPIS);
    g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CElpisMsgBox::ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void SEASON3B::CElpisMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CElpisMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CElpisMsgBox::AboutRefinaryBtnDown, MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_REFINARY);
    AddCallbackFunc(SEASON3B::CElpisMsgBox::AboutJewelOfHarmonyBtnDown, MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_JEWELOFHARMONY);
    AddCallbackFunc(SEASON3B::CElpisMsgBox::RefineBtnDown, MSGBOX_EVENT_USER_CUSTOM_ELPIS_REFINE);
    AddCallbackFunc(SEASON3B::CElpisMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void SEASON3B::CElpisMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 145;
    m_BtnAboutRefinary.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnAboutRefinary.SetText(GlobalText[2201]);

    y = GetPos().y + 175;
    m_BtnAboutJewelOfHarmony.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnAboutJewelOfHarmony.SetText(GlobalText[2202]);

    y = GetPos().y + 205;
    m_BtnRefine.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnRefine.SetText(GlobalText[2203]);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(GlobalText[1002]);
}

void SEASON3B::CElpisMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);

    x = GetPos().x; y = GetPos().y + 120, width = MSGBOX_LINE_WIDTH; height = MSGBOX_LINE_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_LINE, x, y, width, height);
}

void SEASON3B::CElpisMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    swprintf(szText, GlobalText[2206]);
    g_pRenderText->RenderText(fPos_x, fPos_y + 0 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 15;
    g_pRenderText->SetTextColor(220, 183, 131, 255);	// 황금색

    switch (m_iMessageType)
    {
    case 0:
        swprintf(szText, GlobalText[2074]);
        g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
        break;
    case MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_REFINARY:
    {
        wchar_t Textlist[7][100];
        int lineSize = 0;
        lineSize = CutText3(GlobalText[2198], Textlist[0], MSGBOX_WIDTH - 60.0f, 7, 100);
        for (int i = 0; i < lineSize; ++i)
        {
            g_pRenderText->RenderText(fPos_x + 20, fPos_y + (i + 1) * 18, Textlist[i]);
        }
    }
    break;
    case MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_JEWELOFHARMONY:
    {
        wchar_t Textlist[7][100];
        int lineSize = 0;
        lineSize = CutText3(GlobalText[2199], Textlist[0], MSGBOX_WIDTH - 60.0f, 7, 100);
        for (int i = 0; i < lineSize; ++i)
        {
            g_pRenderText->RenderText(fPos_x + 20, fPos_y + (i + 1) * 18, Textlist[i]);
        }
    }
    break;
    }
}

void SEASON3B::CElpisMsgBox::RenderButtons()
{
    m_BtnAboutRefinary.Render();
    m_BtnAboutJewelOfHarmony.Render();
    m_BtnRefine.Render();
    m_BtnExit.Render();
}

SEASON3B::CSeedMasterMenuMsgBox::CSeedMasterMenuMsgBox()
{
    m_iMiddleCount = 7;
}

SEASON3B::CSeedMasterMenuMsgBox::~CSeedMasterMenuMsgBox()
{
    Release();
}

bool SEASON3B::CSeedMasterMenuMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();

    return true;
}

void SEASON3B::CSeedMasterMenuMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();
}

bool SEASON3B::CSeedMasterMenuMsgBox::Update()
{
    m_BtnExtractSeed.Update();
    m_BtnSeedSphere.Update();
    m_BtnExit.Update();

    return true;
}

bool SEASON3B::CSeedMasterMenuMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT SEASON3B::CSeedMasterMenuMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CSeedMasterMenuMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnExtractSeed.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SEED_MASTER_MENU_EXTRACT_SEED);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnSeedSphere.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SEED_MASTER_MENU_SEED_SPHERE);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CSeedMasterMenuMsgBox::ExtractSeedBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_EXTRACT_SEED);
    g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CSeedMasterMenuMsgBox::SeedSphereBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_SEED_SPHERE);
    g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CSeedMasterMenuMsgBox::ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void SEASON3B::CSeedMasterMenuMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CSeedMasterMenuMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CSeedMasterMenuMsgBox::ExtractSeedBtnDown, MSGBOX_EVENT_USER_CUSTOM_SEED_MASTER_MENU_EXTRACT_SEED);
    AddCallbackFunc(SEASON3B::CSeedMasterMenuMsgBox::SeedSphereBtnDown, MSGBOX_EVENT_USER_CUSTOM_SEED_MASTER_MENU_SEED_SPHERE);
    AddCallbackFunc(SEASON3B::CSeedMasterMenuMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void SEASON3B::CSeedMasterMenuMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 85;
    m_BtnExtractSeed.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnExtractSeed.SetText(GlobalText[2664]);

    y = GetPos().y + 120;
    m_BtnSeedSphere.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnSeedSphere.SetText(GlobalText[2665]);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(GlobalText[1002]);
}

void SEASON3B::CSeedMasterMenuMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CSeedMasterMenuMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    swprintf(szText, GlobalText[2666]);
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 15;
    g_pRenderText->SetFont(g_hFont);
    swprintf(szText, GlobalText[2667]);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    swprintf(szText, GlobalText[2668]);
    g_pRenderText->RenderText(fPos_x, fPos_y + 2 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void SEASON3B::CSeedMasterMenuMsgBox::RenderButtons()
{
    m_BtnExtractSeed.Render();
    m_BtnSeedSphere.Render();
    m_BtnExit.Render();
}

SEASON3B::CSeedInvestigatorMenuMsgBox::CSeedInvestigatorMenuMsgBox()
{
    m_iMiddleCount = 7;
}

SEASON3B::CSeedInvestigatorMenuMsgBox::~CSeedInvestigatorMenuMsgBox()
{
    Release();
}

bool SEASON3B::CSeedInvestigatorMenuMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();

    return true;
}

void SEASON3B::CSeedInvestigatorMenuMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();
}

bool SEASON3B::CSeedInvestigatorMenuMsgBox::Update()
{
    m_BtnAttachSocket.Update();
    m_BtnDetachSocket.Update();
    m_BtnExit.Update();

    return true;
}

bool SEASON3B::CSeedInvestigatorMenuMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT SEASON3B::CSeedInvestigatorMenuMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CSeedInvestigatorMenuMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnAttachSocket.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SEED_INVESTIGATOR_MENU_ATTACH_SOCKET);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnDetachSocket.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_SEED_INVESTIGATOR_MENU_DETACH_SOCKET);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CSeedInvestigatorMenuMsgBox::AttachSocketBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_ATTACH_SOCKET);
    g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CSeedInvestigatorMenuMsgBox::DetachSocketBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_DETACH_SOCKET);
    g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CSeedInvestigatorMenuMsgBox::ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void SEASON3B::CSeedInvestigatorMenuMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CSeedInvestigatorMenuMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CSeedInvestigatorMenuMsgBox::AttachSocketBtnDown, MSGBOX_EVENT_USER_CUSTOM_SEED_INVESTIGATOR_MENU_ATTACH_SOCKET);
    AddCallbackFunc(SEASON3B::CSeedInvestigatorMenuMsgBox::DetachSocketBtnDown, MSGBOX_EVENT_USER_CUSTOM_SEED_INVESTIGATOR_MENU_DETACH_SOCKET);
    AddCallbackFunc(SEASON3B::CSeedInvestigatorMenuMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void SEASON3B::CSeedInvestigatorMenuMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 85;
    m_BtnAttachSocket.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnAttachSocket.SetText(GlobalText[2669]);

    y = GetPos().y + 120;
    m_BtnDetachSocket.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnDetachSocket.SetText(GlobalText[2670]);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(GlobalText[1002]);
}

void SEASON3B::CSeedInvestigatorMenuMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CSeedInvestigatorMenuMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    swprintf(szText, GlobalText[2671]);
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 15;
    g_pRenderText->SetFont(g_hFont);
    swprintf(szText, GlobalText[2672]);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    swprintf(szText, GlobalText[2673]);
    g_pRenderText->RenderText(fPos_x, fPos_y + 2 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void SEASON3B::CSeedInvestigatorMenuMsgBox::RenderButtons()
{
    m_BtnAttachSocket.Render();
    m_BtnDetachSocket.Render();
    m_BtnExit.Render();
}

SEASON3B::CResetCharacterPointMsgBox::CResetCharacterPointMsgBox()
{
    m_iMiddleCount = 7;
}

SEASON3B::CResetCharacterPointMsgBox::~CResetCharacterPointMsgBox()
{
    Release();
}

bool SEASON3B::CResetCharacterPointMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x = 0, y = 0, width = 0, height = 0;

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();
    return true;
}

void SEASON3B::CResetCharacterPointMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 105;
    m_ResetCharacterPointBtn.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_ResetCharacterPointBtn.SetText(GlobalText[1884]); // "스탯 초기화"

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(GlobalText[1002]);
}

void SEASON3B::CResetCharacterPointMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();
}

void SEASON3B::CResetCharacterPointMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CResetCharacterPointMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CResetCharacterPointMsgBox::ResetCharacterPointBtnDown, MSGBOX_EVENT_USER_CUSTOM_RESET_CHARACTER_POINT);
    AddCallbackFunc(SEASON3B::CResetCharacterPointMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

CALLBACK_RESULT SEASON3B::CResetCharacterPointMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CResetCharacterPointMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_ResetCharacterPointBtn.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_RESET_CHARACTER_POINT);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

bool SEASON3B::CResetCharacterPointMsgBox::Update()
{
    m_ResetCharacterPointBtn.Update();
    m_BtnExit.Update();

    return true;
}

bool SEASON3B::CResetCharacterPointMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CResetCharacterPointMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CResetCharacterPointMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    swprintf(szText, GlobalText[1885]);
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 25;
    g_pRenderText->SetFont(g_hFont);
    swprintf(szText, GlobalText[1886]);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 18, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void SEASON3B::CResetCharacterPointMsgBox::RenderButtons()
{
    m_ResetCharacterPointBtn.Render();
    m_BtnExit.Render();
}

bool SEASON3B::CResetCharacterPointMsgBox::isCharacterEquipmentItem()
{
    for (int i = 0; i < MAX_EQUIPMENT; i++) {
        if (CharacterMachine->Equipment[i].Type != -1) {
            return true;
        }
    }
    return false;
}

CALLBACK_RESULT SEASON3B::CResetCharacterPointMsgBox::ResetCharacterPointBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);

    for (int i = 0; i < MAX_EQUIPMENT; i++) {
        if (CharacterMachine->Equipment[i].Type != -1)
        {
            g_pSystemLogBox->AddText(GlobalText[1883], SEASON3B::TYPE_ERROR_MESSAGE);
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
            return CALLBACK_BREAK;
        }
    }

    SocketClient->ToGameServer()->SendResetCharacterPointRequest();

    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CResetCharacterPointMsgBox::ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

bool SEASON3B::CGuildBreakPasswordMsgBoxLayout::SetLayout()
{
    CNewUITextInputMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    int _temp = (g_iLengthAuthorityCode / 10) <= 0 ? 1 : (g_iLengthAuthorityCode / 10);
    int _width = INPUTBOX_WIDTH * _temp;
    if (false == pMsgBox->Create(MSGBOX_COMMON_TYPE_OKCANCEL, INPUTBOX_TYPE_NUMBER, _width, INPUTBOX_HEIGHT, g_iLengthAuthorityCode, true))
        return false;

    pMsgBox->SetInputBoxOption(UIOPTION_PAINTBACK);
    pMsgBox->AddMsg(GlobalText[427]);
    pMsgBox->AddMsg(GlobalText[428]);

    pMsgBox->AddCallbackFunc(SEASON3B::CGuildBreakPasswordMsgBoxLayout::ReturnDown, MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(SEASON3B::CGuildBreakPasswordMsgBoxLayout::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(SEASON3B::CGuildBreakPasswordMsgBoxLayout::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);

    return true;
}

CALLBACK_RESULT SEASON3B::CGuildBreakPasswordMsgBoxLayout::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner, xParam);
}

CALLBACK_RESULT SEASON3B::CGuildBreakPasswordMsgBoxLayout::ReturnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    return ProcessOk(pOwner, xParam);
}

CALLBACK_RESULT SEASON3B::CGuildBreakPasswordMsgBoxLayout::ProcessOk(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CNewUITextInputMsgBox*>(pOwner);

    if (pMsgBox == false)
    {
        return CALLBACK_CONTINUE;
    }

    wchar_t strText[20] = { 0, };

    pMsgBox->GetInputBoxText(strText);
    int iInputTextSize = wcslen(strText);

    if (iInputTextSize > 0)
    {
        SocketClient->ToGameServer()->SendGuildKickPlayerRequest(GuildList[DeleteIndex].Name, strText);
    }
    else
    {
        g_pSystemLogBox->AddText(GlobalText[401], SEASON3B::TYPE_ERROR_MESSAGE);
    }

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CGuildBreakPasswordMsgBoxLayout::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

SEASON3B::CGuild_ToPerson_Position::CGuild_ToPerson_Position()
{
    COMGEM::m_cGemType = COMGEM::CELE;
    AppointType = SUBGUILDMASTER;
}

SEASON3B::CGuild_ToPerson_Position::~CGuild_ToPerson_Position()
{
    Release();
}

bool SEASON3B::CGuild_ToPerson_Position::Create(float fPriority)
{
    int x, y, width, height;

    SetAddCallbackFunc();

    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 100;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);

    SetButtonInfo();

    return true;
}

void SEASON3B::CGuild_ToPerson_Position::Release()
{
    CNewUIMessageBoxBase::Release();

    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();
}

bool SEASON3B::CGuild_ToPerson_Position::Update()
{
    m_BtnBlessing.Update();
    m_BtnSoul.Update();
    m_BtnOk.Update();
    m_BtnCancel.Update();

    return true;
}

bool SEASON3B::CGuild_ToPerson_Position::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CGuild_ToPerson_Position::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    auto* pMsg = new MSGBOX_TEXTDATA;
    pMsg->strMsg = strMsg;
    pMsg->dwColor = dwColor;
    pMsg->byFontType = byFontType;
    m_MsgDataList.push_back(pMsg);
}

void SEASON3B::CGuild_ToPerson_Position::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CGuild_ToPerson_Position::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CGuild_ToPerson_Position::BlessingBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_BLESSING);
    AddCallbackFunc(SEASON3B::CGuild_ToPerson_Position::SoulBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_SOUL);
    AddCallbackFunc(SEASON3B::CGuild_ToPerson_Position::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    AddCallbackFunc(SEASON3B::CGuild_ToPerson_Position::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void SEASON3B::CGuild_ToPerson_Position::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_SMALL_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH + 50;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + 57;//(GetPos().x + (msgboxhalfwidth / 2) - btnhalfwidth) + 60;
    y = GetPos().y + 30;
    m_BtnBlessing.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnBlessing.SetText(GlobalText[1311]);

    y += 27;
    m_BtnSoul.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnSoul.SetText(GlobalText[1312]);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x -= 9;
    y += 70;
    m_BtnOk.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnOk.SetText(GlobalText[228]);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x += 64;
    m_BtnCancel.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnCancel.SetText(GlobalText[1002]);
}

void SEASON3B::CGuild_ToPerson_Position::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x;
    y = GetPos().y + 2.f;
    width = (GetSize().cx - MSGBOX_BACK_BLANK_WIDTH);
    height = (GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT) - 75;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    int iCount = 5;
    for (int i = 0; i < iCount; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CGuild_ToPerson_Position::RenderTexts()
{
    
    

    float x, y;

    x = GetPos().x; y = (GetPos().y + (MSGBOX_TEXT_TOP_BLANK / 2)) + 80;

    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        g_pRenderText->SetTextColor((*vi)->dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        switch ((*vi)->byFontType)
        {
        case MSGBOX_FONT_NORMAL:
            g_pRenderText->SetFont(g_hFont);
            break;
        case MSGBOX_FONT_BOLD:
            g_pRenderText->SetFont(g_hFontBold);
            break;
        }

        SIZE TextSize;
        size_t TextExtentWidth, TextExtentHeight;

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), (*vi)->strMsg.c_str(), (*vi)->strMsg.size(), &TextSize);
        TextExtentWidth = (size_t)(TextSize.cx / g_fScreenRate_x);
        TextExtentHeight = (size_t)(TextSize.cy / g_fScreenRate_y);

        x = GetPos().x + (GetSize().cx / 2) - (TextExtentWidth / 2);
        g_pRenderText->RenderText((int)x, (int)y, (*vi)->strMsg.c_str());
        y += (TextExtentHeight + 4);
    }
}

void SEASON3B::CGuild_ToPerson_Position::RenderButtons()
{
    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();

    wchar_t strText[256];
    if (COMGEM::m_cGemType == COMGEM::CELE)
    {
        swprintf(strText, GlobalText[1314], GuildList[DeleteIndex].Name, GlobalText[1301]);
        AppointType = SUBGUILDMASTER;
        AddMsg(strText, RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
        glColor4f(1.0f, 1.0f, 0.2f, 1.0f);
        m_BtnBlessing.Render();
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
        m_BtnBlessing.Render();
    }

    if (COMGEM::m_cGemType == COMGEM::SOUL)
    {
        swprintf(strText, GlobalText[1314], GuildList[DeleteIndex].Name, GlobalText[1302]);
        AppointType = BATTLEMASTER;
        AddMsg(strText, RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
        glColor4f(1.0f, 1.0f, 0.2f, 1.0f);
        m_BtnSoul.Render();
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
        m_BtnSoul.Render();
    }

    m_BtnOk.Render();
    m_BtnCancel.Render();
    AddMsg(GlobalText[1315], RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
}

CALLBACK_RESULT SEASON3B::CGuild_ToPerson_Position::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CGuild_ToPerson_Position*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnBlessing.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_BLESSING);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnSoul.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_SOUL);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnOk.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnCancel.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CGuild_ToPerson_Position::BlessingBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetGem(COMGEM::CELE);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CGuild_ToPerson_Position::SoulBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetGem(COMGEM::SOUL);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CGuild_ToPerson_Position::OkBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::Exit();
    SocketClient->ToGameServer()->SendGuildRoleAssignRequest(
        AppointType,
        GuildList[DeleteIndex].Name,
        AppointType == G_PERSON ? 0x01 : 0x02);

    SocketClient->ToGameServer()->SendGuildListRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CGuild_ToPerson_Position::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::Exit();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool SEASON3B::CGuild_ToPerson_PositionLayout::SetLayout()
{
    CGuild_ToPerson_Position* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

SEASON3B::CDelgardoMainMenuMsgBox::CDelgardoMainMenuMsgBox()
{
    m_iMiddleCount = 7;
}

SEASON3B::CDelgardoMainMenuMsgBox::~CDelgardoMainMenuMsgBox()
{
    Release();
}

bool SEASON3B::CDelgardoMainMenuMsgBox::Create(float fPriority)
{
    SetAddCallbackFunc();

    int x, y, width, height;
    x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    y = 60;
    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CNewUIMessageBoxBase::Create(x, y, width, height, fPriority);
    SetButtonInfo();

    return true;
}

void SEASON3B::CDelgardoMainMenuMsgBox::Release()
{
    CNewUIMessageBoxBase::Release();
}

bool SEASON3B::CDelgardoMainMenuMsgBox::Update()
{
    m_BtnReg.Update();
    m_BtnExchange.Update();
    m_BtnExit.Update();

    return true;
}

bool SEASON3B::CDelgardoMainMenuMsgBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

CALLBACK_RESULT SEASON3B::CDelgardoMainMenuMsgBox::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CDelgardoMainMenuMsgBox*>(pOwner);
    if (pMsgBox)
    {
        if (pMsgBox->m_BtnReg.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_DELGARDO_REGISTRATION_LUCKY_COIN);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExchange.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_DELGARDO_EXCHANGE_LUCKY_COIN);
            return CALLBACK_BREAK;
        }
        if (pMsgBox->m_BtnExit.IsMouseIn() == true)
        {
            g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
            return CALLBACK_BREAK;
        }
    }

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT SEASON3B::CDelgardoMainMenuMsgBox::RegBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_pNewUISystem->Show(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION);

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CDelgardoMainMenuMsgBox::ExchangeBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    g_pNewUISystem->Show(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT SEASON3B::CDelgardoMainMenuMsgBox::ExitBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void SEASON3B::CDelgardoMainMenuMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(SEASON3B::CDelgardoMainMenuMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(SEASON3B::CDelgardoMainMenuMsgBox::RegBtnDown, MSGBOX_EVENT_USER_CUSTOM_DELGARDO_REGISTRATION_LUCKY_COIN);
    AddCallbackFunc(SEASON3B::CDelgardoMainMenuMsgBox::ExchangeBtnDown, MSGBOX_EVENT_USER_CUSTOM_DELGARDO_EXCHANGE_LUCKY_COIN);
    AddCallbackFunc(SEASON3B::CDelgardoMainMenuMsgBox::ExitBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

void SEASON3B::CDelgardoMainMenuMsgBox::SetButtonInfo()
{
    float x, y, width, height;

    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + 85;
    m_BtnReg.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnReg.SetText(GlobalText[1891]);

    y = GetPos().y + 120;
    m_BtnExchange.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY);
    m_BtnExchange.SetText(GlobalText[1892]);

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = GetPos().x + msgboxhalfwidth - btnhalfwidth;
    y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    m_BtnExit.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnExit.SetText(GlobalText[1002]);
}

void SEASON3B::CDelgardoMainMenuMsgBox::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = GetSize().cx - MSGBOX_BACK_BLANK_WIDTH; height = GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < m_iMiddleCount; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);
}

void SEASON3B::CDelgardoMainMenuMsgBox::RenderTexts()
{
    wchar_t szText[256] = { 0, };
    float fPos_x = GetPos().x + 10;
    float fPos_y = GetPos().y + 10;

    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetFont(g_hFontBold);
    swprintf(szText, GlobalText[1890]);
    g_pRenderText->RenderText(fPos_x, fPos_y, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);

    fPos_y += 26;
    g_pRenderText->SetFont(g_hFont);
    swprintf(szText, GlobalText[1932]);
    g_pRenderText->RenderText(fPos_x, fPos_y + 1 * 12, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    swprintf(szText, GlobalText[1933]);
    g_pRenderText->RenderText(fPos_x, fPos_y + 2 * 12, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
    swprintf(szText, GlobalText[1934]);
    g_pRenderText->RenderText(fPos_x, fPos_y + 3 * 12, szText, MSGBOX_WIDTH - 20.0f, 0, RT3_SORT_CENTER);
}

void SEASON3B::CDelgardoMainMenuMsgBox::RenderButtons()
{
    m_BtnReg.Render();
    m_BtnExchange.Render();
    m_BtnExit.Render();
}