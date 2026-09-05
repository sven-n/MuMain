// NewUISlideWindow.cpp: implementation of the CNewUISlideWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "UI/Widgets/NewUISlideWindow.h"
#include "UI/Core/NewUIManager.h"

mu::ui::window::CNewUISlideWindow::CNewUISlideWindow()
{
    m_pNewUIMng = NULL;
    m_pSlideMgr = NULL;
}

mu::ui::window::CNewUISlideWindow::~CNewUISlideWindow()
{
    Release();
}

bool mu::ui::window::CNewUISlideWindow::Create(CNewUIManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_SLIDEWINDOW, this);
    m_pSlideMgr = new CSlideHelpMgr;
    std::wstring strFileName = L"Data\\Local\\" + g_strSelectedML + L"\\Slide_" + g_strSelectedML + L".bmd";
    m_pSlideMgr->OpenSlideTextFile(strFileName.c_str());

    return true;
}

void mu::ui::window::CNewUISlideWindow::Release()
{
    SAFE_DELETE(m_pSlideMgr);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

bool mu::ui::window::CNewUISlideWindow::UpdateMouseEvent()
{
    return true;
}
bool mu::ui::window::CNewUISlideWindow::UpdateKeyEvent()
{
    return true;
}
bool mu::ui::window::CNewUISlideWindow::Update()
{
    m_pSlideMgr->ManageSlide();

    return true;
}
bool mu::ui::window::CNewUISlideWindow::Render()
{
    m_pSlideMgr->Render();

    return true;
}

float mu::ui::window::CNewUISlideWindow::GetLayerDepth()
{
    return 1.91f;
}