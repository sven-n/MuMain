// NewUISlideWindow.cpp: implementation of the CSlideWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "UI/Widgets/NewUISlideWindow.h"
#include "UI/Core/NewUIManager.h"

mu::ui::window::CSlideWindow::CSlideWindow()
{
    m_pNewUIMng = NULL;
    m_pSlideMgr = NULL;
}

mu::ui::window::CSlideWindow::~CSlideWindow()
{
    Release();
}

bool mu::ui::window::CSlideWindow::Create(CManager* pNewUIMng)
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

void mu::ui::window::CSlideWindow::Release()
{
    SAFE_DELETE(m_pSlideMgr);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

bool mu::ui::window::CSlideWindow::UpdateMouseEvent()
{
    return true;
}
bool mu::ui::window::CSlideWindow::UpdateKeyEvent()
{
    return true;
}
bool mu::ui::window::CSlideWindow::Update()
{
    m_pSlideMgr->ManageSlide();

    return true;
}
bool mu::ui::window::CSlideWindow::Render()
{
    m_pSlideMgr->Render();

    return true;
}

float mu::ui::window::CSlideWindow::GetLayerDepth()
{
    return 1.91f;
}