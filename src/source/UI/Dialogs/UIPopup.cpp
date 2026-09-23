//  UIPopup.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/Core/UIManager.h"
#include "UIPopup.h"
#include "UI/Core/WindowSystem.h"

CUIPopup::CUIPopup()
{
    Init();
}

CUIPopup::~CUIPopup() {}

void CUIPopup::Init()
{
    PopupResultFuncPointer = NULL;
    PopupUpdateInputFuncPointer = NULL;
    PopupRenderFuncPointer = NULL;
    m_dwPopupID = 0;
}

DWORD CUIPopup::SetPopup(int (*ResultFunc)(POPUP_RESULT Result))
{
    if (m_dwPopupID != 0)
    {
        __TraceF(TEXT("CUIPopup::SetPopup\n"));
        g_pSystemLogBox->AddText(L"SetPopup", mu::ui::window::TYPE_SYSTEM_MESSAGE);
        return 0;
    }

    Init();
    PopupResultFuncPointer = ResultFunc;
    m_dwPopupID = GetTickCount();

    return m_dwPopupID;
}

void CUIPopup::SetPopupExtraFunc(void (*InputFunc)(), void (*RenderFunc)())
{
    PopupUpdateInputFuncPointer = InputFunc;
    PopupRenderFuncPointer = RenderFunc;
}

DWORD CUIPopup::GetPopupID()
{
    return m_dwPopupID;
}

void CUIPopup::Close()
{
    m_dwPopupID = 0;
}

void CUIPopup::CancelPopup()
{
    if (m_dwPopupID == 0)
        return;

    if (PopupResultFuncPointer)
    {
        PopupResultFuncPointer(POPUP_RESULT_NONE);
        PopupResultFuncPointer = NULL;
    }

    PopupUpdateInputFuncPointer = NULL;
    PopupRenderFuncPointer = NULL;
    Close();
}

bool CUIPopup::PressKey(int nKey)
{
    if (m_dwPopupID == 0)
        return false;

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

    return false;
}

void CUIPopup::UpdateInput()
{
    if (m_dwPopupID == 0)
        return;

    if (PopupUpdateInputFuncPointer)
        PopupUpdateInputFuncPointer();
}

void CUIPopup::Render()
{
    if (m_dwPopupID == 0)
        return;

    if (PopupRenderFuncPointer)
        PopupRenderFuncPointer();
}
