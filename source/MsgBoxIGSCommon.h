// MsgBoxIGSCommon.h: interface for the CMsgBoxIGSCommon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSGBOXIGSCOMMON_H__5E2E9B11_C6F5_411E_BC68_1CF1D1DAE3C0__INCLUDED_)
#define AFX_MSGBOXIGSCOMMON_H__5E2E9B11_C6F5_411E_BC68_1CF1D1DAE3C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "UIControls.h"
#include "NewUIMessageBox.h"
#include "NewUICommonMessageBox.h"
#include "NewUIOptionWindow.h"

using namespace SEASON3B;

class CMsgBoxIGSCommon : public CNewUIMessageBoxBase
{
public:
	enum IMAGE_IGS_COMMON
	{
		IMAGE_IGS_BUTTON	= BITMAP_IGS_MSGBOX_BUTTON,				// 인게임샵 버튼
		IMAGE_IGS_BACK		= CNewUIOptionWindow::IMAGE_OPTION_FRAME_BACK,
		IMAGE_IGS_UP		= CNewUIOptionWindow::IMAGE_OPTION_FRAME_UP,
		IMAGE_IGS_DOWN		= CNewUIOptionWindow::IMAGE_OPTION_FRAME_DOWN,
		IMAGE_IGS_LEFTLINE	= CNewUIOptionWindow::IMAGE_OPTION_FRAME_LEFT,
		IMAGE_IGS_RIGHTLINE = CNewUIOptionWindow::IMAGE_OPTION_FRAME_RIGHT,
	};
	
	enum IMAGESIZE_IGS_COMMON
	{
		IMAGE_IGS_WINDOW_WIDTH	= 640,	// 인게임샵 배경 사이즈
		IMAGE_IGS_WINDOW_HEIGHT = 429,
		IMAGE_IGS_FRAME_WIDTH	= 190,	// 메세지박스 Size
		IMAGE_IGS_FRAME_HEIGHT	= 109,
		IMAGE_IGS_UP_HEIGHT		= 64,
		IMAGE_IGS_DOWN_HEIGHT	= 45,
		IMAGE_IGS_LINE_WIDTH	= 21,
		IMAGE_IGS_LINE_HEIGHT	= 10,
		IMAGE_IGS_BTN_WIDTH		= 52,	// 버튼 Size
		IMAGE_IGS_BTN_HEIGHT	= 26,	
	};
	
	// 메세지박스상의 상대좌표
	enum IGS_COMMON_POS
	{
		IGS_TEXT_TITLE_POS_Y		= 10,	// Title Text
		IGS_TEXT_ITEM_INFO_POS_X	= 10,	// 아이템정보 Textbox
		IGS_TEXT_ITEM_INFO_POS_Y	= 40,
		IGS_TEXT_ITEM_INFO_WIDTH	= 170,
		IGS_BTN_POS_Y				= 5,	// Button
	};

	// 기타
	enum IGS_COMMON_ETC
	{
		IGS_NUM_TEXT_LIMIT_RENDER_MIDDLE_LINE = 3,		// 중간 라인 프레임 하나 추가에 따른 Text수
	};
	
public:
	CMsgBoxIGSCommon();
	~CMsgBoxIGSCommon();
	
	bool Create(float fPriority = 3.f);
	void Release();
	
	bool Update();
	bool Render();
	
	void Initialize(const unicode::t_char* pszTitle, const unicode::t_char* pszText);
	
	static CALLBACK_RESULT LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	static CALLBACK_RESULT OKButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	static CALLBACK_RESULT CancelButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
	
private:
	void SetAddCallbackFunc();
	void SetButtonInfo();
	
	void RenderFrame();
	void RenderTexts();
	void RenderButtons();
	
	void LoadImages();
	void UnloadImages();
	
private:
	// buttons
	CNewUIMessageBoxButton m_BtnOk;

	unicode::t_char		m_szTitle[MAX_TEXT_LENGTH];
#ifdef PBG_FIX_MSGBUFFERSIZE
	unicode::t_char		m_szText[NUM_LINE_CMB][MAX_TEXT_LENGTH];
#else //PBG_FIX_MSGBUFFERSIZE
	unicode::t_char		m_szText[NUM_LINE_CMB][MAX_LENGTH_CMB];
#endif //PBG_FIX_MSGBUFFERSIZE

	int	m_iMsgBoxWidth;
	int m_iMsgBoxHeight;
	int m_iMiddleCount;
	int	m_iNumTextLine;
	
public:
};

////////////////////////////////////////////////////////////////////
// LayOut
////////////////////////////////////////////////////////////////////
class CMsgBoxIGSCommonLayout : public TMsgBoxLayout<CMsgBoxIGSCommon>
{
public:
	bool SetLayout();
};


#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

#endif // !defined(AFX_MSGBOXIGSCOMMON_H__5E2E9B11_C6F5_411E_BC68_1CF1D1DAE3C0__INCLUDED_)
