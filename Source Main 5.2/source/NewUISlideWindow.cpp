// NewUISlideWindow.cpp: implementation of the CNewUISlideWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUISlideWindow.h"
#include "NewUIManager.h"

SEASON3B::CNewUISlideWindow::CNewUISlideWindow()
{
    m_pNewUIMng = NULL;
    m_pSlideMgr = NULL;
}

SEASON3B::CNewUISlideWindow::~CNewUISlideWindow()
{
    Release();
}

bool SEASON3B::CNewUISlideWindow::Create(CNewUIManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_SLIDEWINDOW, this);
    m_pSlideMgr = new CSlideHelpMgr;
    std::wstring strFileName = L"Data\\Local\\" + g_strSelectedML + L"\\Slide_" + g_strSelectedML + L".bmd";
    m_pSlideMgr->OpenSlideTextFile(strFileName.c_str());

    return true;
}

void SEASON3B::CNewUISlideWindow::Release()
{
    SAFE_DELETE(m_pSlideMgr);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

bool SEASON3B::CNewUISlideWindow::UpdateMouseEvent()
{
    return true;
}
bool SEASON3B::CNewUISlideWindow::UpdateKeyEvent()
{
    return true;
}
bool SEASON3B::CNewUISlideWindow::Update()
{
    m_pSlideMgr->ManageSlide();

    return true;
}
bool SEASON3B::CNewUISlideWindow::Render()
{
    m_pSlideMgr->Render();

    return true;
}

float SEASON3B::CNewUISlideWindow::GetLayerDepth()
{
    return 1.91f;
}