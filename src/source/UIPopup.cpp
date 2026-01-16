//  UIPopup.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzInterface.h"
#include "UIManager.h"
#include "UIPopup.h"
#include "NewUISystem.h"

extern CUITextInputBox* g_pSingleTextInputBox;
extern int				g_iChatInputType;



CUIPopup::CUIPopup()
{
    int nButtonID = 0;
    m_OkButton.Init(nButtonID++, GlobalText[228]);
    m_OkButton.SetParentUIID(0);
    m_OkButton.SetSize(50, 18);
    m_CancelButton.Init(nButtonID++, GlobalText[229]);
    m_CancelButton.SetParentUIID(0);
    m_CancelButton.SetSize(50, 18);
    m_YesButton.Init(nButtonID++, GlobalText[1037]);
    m_YesButton.SetParentUIID(0);
    m_YesButton.SetSize(50, 18);
    m_NoButton.Init(nButtonID++, GlobalText[1038]);
    m_NoButton.SetParentUIID(0);
    m_NoButton.SetSize(50, 18);

    m_dwPopupStartTime = 0;
    m_dwPopupEndTime = 0;
    m_dwPopupElapseTime = 5000;

    Init();
}

CUIPopup::~CUIPopup()
{
}

void CUIPopup::Init()
{
    PopupResultFuncPointer = NULL;
    PopupUpdateInputFuncPointer = NULL;
    PopupRenderFuncPointer = NULL;

    m_dwPopupID = 0;
    m_nPopupTextCount = 0;
    m_PopupType = 0;
    ZeroMemory(m_szPopupText, sizeof(char) * MAX_POPUP_TEXTLINE * MAX_POPUP_TEXTLENGTH);
    ZeroMemory(m_szInputText, sizeof(char) * 1024);
}

DWORD CUIPopup::SetPopup(const wchar_t* pszText, int nLineCount, int nBufferSize, int Type, int (*ResultFunc)(POPUP_RESULT Result), POPUP_ALIGN Align)
{
    if (nLineCount > MAX_POPUP_TEXTLINE)
    {
        __TraceF(TEXT("CUIPopup::SetPopup\n"));
        return 0;
    }
    if (m_dwPopupID != 0)
    {
        __TraceF(TEXT("CUIPopup::SetPopup\n"));
        g_pSystemLogBox->AddText(L"SetPopup", SEASON3B::TYPE_SYSTEM_MESSAGE);
        return 0;
    }

    Init();
    m_Align = Align;
    m_sizePopup.cy = 0;
    SIZE sizeText;
    for (int i = 0; i < nLineCount; ++i)
    {
        if (pszText[i * nBufferSize])
        {
            wcscpy(m_szPopupText[i], &pszText[i * nBufferSize]);
            GetTextExtentPoint32(g_pRenderText->GetFontDC(), m_szPopupText[i], wcslen(m_szPopupText[i]), &sizeText);
            m_sizePopup.cy += sizeText.cy + 2;
        }
    }
    m_nPopupTextCount = nLineCount;
    m_PopupType = Type;
    PopupResultFuncPointer = ResultFunc;

    m_sizePopup.cx = 213;
    m_sizePopup.cy += 43;
    if (m_PopupType & POPUP_OK || m_PopupType & POPUP_OKCANCEL || m_PopupType & POPUP_YESNO)
        m_sizePopup.cy += 20;
    if (m_PopupType & POPUP_INPUT)
        m_sizePopup.cy += 17;
    if (m_PopupType & POPUP_TIMEOUT)
        m_sizePopup.cy += 17;

    m_dwPopupStartTime = GetTickCount();
    m_dwPopupEndTime = m_dwPopupStartTime + m_dwPopupElapseTime;
    m_dwPopupID = m_dwPopupStartTime;

    return m_dwPopupID;
}

void CUIPopup::SetInputMode(int nInputSize, int nTextLength, UIOPTIONS Options)
{
    m_nInputSize = nInputSize;
    m_nInputTextLength = nTextLength;
    m_InputOptions = Options;
}

bool CUIPopup::IsInputEnable()
{
    if (m_dwPopupID != 0 && m_PopupType & POPUP_INPUT)
        return true;
    else
        return false;
}

void CUIPopup::SetTimeOut(DWORD dwElapseTime)
{
    m_dwPopupElapseTime = dwElapseTime;
}

void CUIPopup::SetPopupExtraFunc(void (*InputFunc)(), void (*RenderFunc)())
{
    PopupUpdateInputFuncPointer = InputFunc;
    PopupRenderFuncPointer = RenderFunc;
}

wchar_t* CUIPopup::GetInputText()
{
    if (g_iChatInputType == 1)
        g_pSingleTextInputBox->GetText(m_szInputText, 1024);
    else
    {
        if (InputText[0])
            wcscpy(m_szInputText, InputText[0]);
        else
            m_szInputText[0] = NULL;
    }

    return m_szInputText;
}

DWORD CUIPopup::GetPopupID()
{
    return m_dwPopupID;
}

void CUIPopup::Close()
{
    if (m_dwPopupID == 0)	return;

    if (m_PopupType & POPUP_INPUT)
    {
        if (g_iChatInputType == 1)
        {
            g_pSingleTextInputBox->SetText(NULL);
            SaveIMEStatus();
            g_pSingleTextInputBox->SetState(UISTATE_HIDE);
        }
        else
            memset(InputText[0], 0, MAX_ID_SIZE);

        InputLength[0] = 0;
        InputTextMax[0] = MAX_ID_SIZE;
    }

    m_dwPopupID = 0;
}

void CUIPopup::CancelPopup()
{
    if (m_dwPopupID == 0)	return;

    if (PopupResultFuncPointer)
    {
        if (m_PopupType & POPUP_CUSTOM)
        {
        }
        else if (m_PopupType & POPUP_OK)
        {
            PopupResultFuncPointer(POPUP_RESULT_OK);
            PopupResultFuncPointer = NULL;
        }
        else if (m_PopupType & POPUP_OKCANCEL)
        {
            PopupResultFuncPointer(POPUP_RESULT_CANCEL);
            PopupResultFuncPointer = NULL;
        }
        else if (m_PopupType & POPUP_YESNO)
        {
            PopupResultFuncPointer(POPUP_RESULT_NO);
            PopupResultFuncPointer = NULL;
        }
        else
        {
            PopupResultFuncPointer(POPUP_RESULT_NONE);
            PopupResultFuncPointer = NULL;
        }
    }

    PopupUpdateInputFuncPointer = NULL;
    PopupRenderFuncPointer = NULL;
    Close();
}

bool CUIPopup::PressKey(int nKey)
{
    if (m_dwPopupID == 0)	return false;

    if (m_PopupType & POPUP_CUSTOM)
    {
        if (nKey == VK_ESCAPE)
        {
            Close();
            return true;
        }
    }
    else if (m_PopupType & POPUP_OK)
    {
        if (nKey == VK_ESCAPE)
        {
            if (PopupResultFuncPointer)
            {
                if (PopupResultFuncPointer(POPUP_RESULT_OK | POPUP_RESULT_ESC) == 0)
                    return true;
                PopupResultFuncPointer = NULL;
            }
            Close();
            return true;
        }
    }
    else if (m_PopupType & POPUP_OKCANCEL)
    {
        if (nKey == VK_ESCAPE)
        {
            if (PopupResultFuncPointer)
            {
                if (PopupResultFuncPointer(POPUP_RESULT_CANCEL | POPUP_RESULT_ESC) == 0)
                    return true;
                PopupResultFuncPointer = NULL;
            }
            Close();
            return true;
        }
    }
    else if (m_PopupType & POPUP_YESNO)
    {
        if (nKey == VK_ESCAPE)
        {
            if (PopupResultFuncPointer)
            {
                if (PopupResultFuncPointer(POPUP_RESULT_NO | POPUP_RESULT_ESC) == 0)
                    return true;
                PopupResultFuncPointer = NULL;
            }
            Close();
            return true;
        }
    }
    else
    {
        if (nKey == VK_ESCAPE)
        {
            if (PopupResultFuncPointer)
            {
                if (PopupResultFuncPointer(POPUP_RESULT_NONE | POPUP_RESULT_ESC) == 0)
                    return true;
                PopupResultFuncPointer = NULL;
            }
            Close();
            return true;
        }
    }

    if (nKey == VK_RETURN && m_PopupType & POPUP_INPUT)
    {
        if (m_PopupType & POPUP_OK)
        {
            if (PopupResultFuncPointer)
            {
                if (PopupResultFuncPointer(POPUP_RESULT_OK) == 0)
                    return true;
                PopupResultFuncPointer = NULL;
            }
            Close();
            return true;
        }
        else if (m_PopupType & POPUP_OKCANCEL)
        {
            if (PopupResultFuncPointer)
            {
                if (PopupResultFuncPointer(POPUP_RESULT_OK) == 0)
                    return true;
                PopupResultFuncPointer = NULL;
            }
            Close();
            return true;
        }
        else if (m_PopupType & POPUP_YESNO)
        {
            if (PopupResultFuncPointer)
            {
                if (PopupResultFuncPointer(POPUP_RESULT_YES) == 0)
                    return true;
                PopupResultFuncPointer = NULL;
            }
            Close();
            return true;
        }
        else
        {
            if (PopupResultFuncPointer)
            {
                if (PopupResultFuncPointer(POPUP_RESULT_NONE) == 0)
                    return true;
                PopupResultFuncPointer = NULL;
            }
            Close();
            return true;
        }
    }

    return false;
}

bool CUIPopup::CheckTimeOut()
{
    if (!(m_PopupType & POPUP_TIMEOUT))
        return false;

    DWORD dwCurrTime = GetTickCount();
    if (dwCurrTime >= m_dwPopupEndTime)
        return true;
    else
        return false;
}

void CUIPopup::UpdateInput()
{
    if (m_dwPopupID == 0)	return;

    float fSubWinPos_x = 320 - m_sizePopup.cx / 2;
    float fSubWinPos_y = 130 - m_sizePopup.cy / 2;
    if (CheckMouseIn(fSubWinPos_x, fSubWinPos_y, m_sizePopup.cx, m_sizePopup.cy))
        MouseOnWindow = TRUE;

    if (PopupUpdateInputFuncPointer)
    {
        PopupUpdateInputFuncPointer();
    }
    else
    {
        bool bTimeOut = CheckTimeOut();

        if (m_PopupType & POPUP_OK)
        {
            if (bTimeOut)
            {
                if (PopupResultFuncPointer)
                {
                    if (PopupResultFuncPointer(POPUP_RESULT_TIMEOUT) == 0)
                        return;
                    PopupResultFuncPointer = NULL;
                }
                Close();
            }
            else if (m_OkButton.DoMouseAction())
            {
                if (PopupResultFuncPointer)
                {
                    if (PopupResultFuncPointer(POPUP_RESULT_OK) == 0)
                        return;
                    PopupResultFuncPointer = NULL;
                }
                Close();
            }
        }
        else if (m_PopupType & POPUP_OKCANCEL)
        {
            if (bTimeOut)
            {
                if (PopupResultFuncPointer)
                {
                    if (PopupResultFuncPointer(POPUP_RESULT_TIMEOUT) == 0)
                        return;
                    PopupResultFuncPointer = NULL;
                }
                Close();
            }
            else if (m_OkButton.DoMouseAction())
            {
                if (PopupResultFuncPointer)
                {
                    if (PopupResultFuncPointer(POPUP_RESULT_OK) == 0)
                        return;
                    PopupResultFuncPointer = NULL;
                }
                Close();
            }
            else if (m_CancelButton.DoMouseAction())
            {
                if (PopupResultFuncPointer)
                {
                    if (PopupResultFuncPointer(POPUP_RESULT_CANCEL) == 0)
                        return;
                    PopupResultFuncPointer = NULL;
                }
                Close();
            }
        }
        else if (m_PopupType & POPUP_YESNO)
        {
            if (bTimeOut)
            {
                if (PopupResultFuncPointer)
                {
                    if (PopupResultFuncPointer(POPUP_RESULT_TIMEOUT) == 0)
                        return;
                    PopupResultFuncPointer = NULL;
                }
                Close();
            }
            else if (m_YesButton.DoMouseAction())
            {
                if (PopupResultFuncPointer)
                {
                    if (PopupResultFuncPointer(POPUP_RESULT_YES) == 0)
                        return;
                    PopupResultFuncPointer = NULL;
                }
                Close();
            }
            else if (m_NoButton.DoMouseAction())
            {
                if (PopupResultFuncPointer)
                {
                    if (PopupResultFuncPointer(POPUP_RESULT_NO) == 0)
                        return;
                    PopupResultFuncPointer = NULL;
                }
                Close();
            }
        }
        else
        {
            if (bTimeOut)
            {
                if (PopupResultFuncPointer)
                {
                    if (PopupResultFuncPointer(POPUP_RESULT_TIMEOUT) == 0)
                        return;
                    PopupResultFuncPointer = NULL;
                }
                Close();
            }
        }
    }
}

void CUIPopup::Render()
{
    if (m_dwPopupID == 0)
    {
        return;
    }

    if (PopupRenderFuncPointer)
    {
        PopupRenderFuncPointer();
    }
    else
    {
        POINT pt;
        if (m_Align == PA_CENTER)
            pt = g_pUIManager->RenderWindowBase(m_sizePopup.cy);
        else
            pt = g_pUIManager->RenderWindowBase(m_sizePopup.cy, 10);

        g_pRenderText->SetFont(g_hFont);
        EnableAlphaTest();

        g_pRenderText->SetBgColor(0x00000000);
        g_pRenderText->SetTextColor(0xFFFFFFFF);

        float fPosY = pt.y + 20;
        for (int i = 0; i < m_nPopupTextCount; ++i)
        {
            SIZE size;
            GetTextExtentPoint32(g_pRenderText->GetFontDC(), m_szPopupText[i], wcslen(m_szPopupText[i]), &size);

            size.cx /= g_fScreenRate_x;
            g_pRenderText->RenderText(320 - (size.cx / 2), fPosY, m_szPopupText[i], 0, 0, RT3_SORT_LEFT);
            fPosY += 15;
        }

        if (m_PopupType & POPUP_INPUT)
        {
            RenderBitmap(BITMAP_INVENTORY + 11, 320 - m_nInputSize / 2, fPosY, m_nInputSize, 18, 0.f, 0.f, 113.f / 128.f, 18.f / 32.f);

            fPosY += 7;
            if (g_iChatInputType == 1)
            {
                g_pSingleTextInputBox->SetState(UISTATE_NORMAL);
                g_pSingleTextInputBox->SetOption(m_InputOptions);
                g_pSingleTextInputBox->SetBackColor(0, 0, 0, 0);
                g_pSingleTextInputBox->SetTextLimit(m_nInputTextLength);
                g_pSingleTextInputBox->SetSize(m_nInputSize, 14);
                g_pSingleTextInputBox->SetPosition(320 - m_nInputSize / 2 + 5, fPosY - 2);
                g_pSingleTextInputBox->GiveFocus();
                g_pSingleTextInputBox->Render();
            }
            else if (g_iChatInputType == 0)
            {
                InputTextWidth = 100;
                RenderInputText(pt.x + 213 / 2 - m_nInputSize + 5, fPosY - 2, 0);
                InputTextWidth = 255;
            }
            fPosY += 10;
        }
        if (m_PopupType & POPUP_TIMEOUT)
        {
            fPosY += 7;
            DWORD dwCurrTime = GetTickCount();
            float fProgress = (float)(dwCurrTime - m_dwPopupStartTime) / m_dwPopupElapseTime;

            RenderBitmap(BITMAP_INTERFACE_EX + 42, 320 - 75, fPosY, 150.0f, 12.0f, 0.f, 0.f, 200.0f / 256.0f, 16.0f / 16.0f);
            EnableAlphaBlend();
            RenderBitmap(BITMAP_INTERFACE_EX + 43, 320 - 75 - 4, fPosY, 150.0f * fProgress, 12.0f, 0.f, 0.f, (150.0f * fProgress) / 256.0f, 16.0f / 16.0f);
            DisableAlphaBlend();
            fPosY += 10;
        }
        fPosY += 9;
        if (m_PopupType & POPUP_OK)
        {
            m_OkButton.SetPosition(320 - 25, fPosY);
            m_OkButton.Render();
        }
        else if (m_PopupType & POPUP_OKCANCEL)
        {
            m_OkButton.SetPosition(320 - 50 - 10, fPosY);
            m_OkButton.Render();
            m_CancelButton.SetPosition(320 + 10, fPosY);
            m_CancelButton.Render();
        }
        else if (m_PopupType & POPUP_YESNO)
        {
            m_YesButton.SetPosition(320 - 50 - 10, fPosY);
            m_YesButton.Render();
            m_NoButton.SetPosition(320 + 10, fPosY);
            m_NoButton.Render();
        }
    }
}