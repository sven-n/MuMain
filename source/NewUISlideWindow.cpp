// NewUISlideWindow.cpp: implementation of the CNewUISlideWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUISlideWindow.h"
#include "NewUIManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

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
	if(NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_SLIDEWINDOW, this);

	m_pSlideMgr = new CSlideHelpMgr;

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#ifdef USE_SLIDETEST_BMD
	string strFileName = "Data\\Local\\"+g_strSelectedML+"\\Slidetest_"+g_strSelectedML+".bmd";
	m_pSlideMgr->OpenSlideTextFile(strFileName.c_str());
#else // USE_SLIDETEST_BMD
	string strFileName = "Data\\Local\\"+g_strSelectedML+"\\Slide_"+g_strSelectedML+".bmd";
	m_pSlideMgr->OpenSlideTextFile(strFileName.c_str());
#endif // USE_SLIDETEST_BMD
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#ifdef USE_SLIDETEST_BMD
	m_pSlideMgr->OpenSlideTextFile("Data\\Local\\Slidetest.bmd");
#else // USE_SLIDETEST_BMD
	m_pSlideMgr->OpenSlideTextFile("Data\\Local\\Slide.bmd");
#endif // USE_SLIDETEST_BMD
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

	return true;
}

void SEASON3B::CNewUISlideWindow::Release()
{
	SAFE_DELETE(m_pSlideMgr);

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

bool SEASON3B::CNewUISlideWindow::UpdateMouseEvent()
{
// 	extern DWORD g_dwActiveUIID;
// 	extern DWORD g_dwMouseUseUIID;  // 마우스가 윈도우 위에 있는가 (있으면 윈도우 ID)
// 	if(g_dwActiveUIID != 0 || g_dwMouseUseUIID != 0)
// 		return false;

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

