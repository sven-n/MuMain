// MsgBoxIGSSendGift.cpp: implementation of the CMsgBoxIGSSendGift class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "MsgBoxIGSSendGift.h"

#include "DSPlaySound.h"
#include "wsclientinline.h"
#include "MsgBoxIGSCommon.h"
#include "MsgBoxIGSSendGiftConfirm.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMsgBoxIGSSendGift::CMsgBoxIGSSendGift()
{
	m_iPackageSeq	= 0;
	m_iDisplaySeq	= 0;
	m_iPriceSeq		= 0;
#ifdef KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL
	m_wItemCode		= -1;
#endif // KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	m_iCashType		= 0;
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	
	m_szID[0]		= '\0';			// 보낼 유저 ID
	m_szMessage[0]	= '\0';			// 보낼 Meaasge
	
	m_szName[0]		= '\0';
	m_szPrice[0]	= '\0';
	m_szPeriod[0]	= '\0';

	for(int i=0 ; i<NUM_LINE_CMB ; i++)
	{
		m_szNotice[i][0] = '\0';
	}

	m_iNumNoticeLine = 0;

}

CMsgBoxIGSSendGift::~CMsgBoxIGSSendGift()
{
	Release();
}

//--------------------------------------------
// Create
bool CMsgBoxIGSSendGift::Create(float fPriority)
{
	LoadImages();
	SetAddCallbackFunc();

	CNewUIMessageBoxBase::Create((IMAGE_IGS_WINDOW_WIDTH/2)-(IMAGE_IGS_FRAME_WIDTH/2), 
		(IMAGE_IGS_WINDOW_HEIGHT/2)-(IMAGE_IGS_FRAME_HEIGHT/2), 
		IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT, fPriority);
	
	SetButtonInfo();
	InitInputBox();

	SetMsgBackOpacity();
	
	return true;
}

//--------------------------------------------
// InitInputBox
void CMsgBoxIGSSendGift::InitInputBox()
{
	// 선물할 캐릭터ID Input Box
	m_IDInputBox.Init(g_hWnd, IGS_ID_INPUT_TEXT_WIDTH, IGS_ID_INPUT_TEXT_HEIGHT, MAX_ID_SIZE);
	m_IDInputBox.SetPosition(GetPos().x+IGS_ID_INPUT_TEXT_POS_X, GetPos().y+IGS_ID_INPUT_TEXT_POS_Y);
	m_IDInputBox.SetTextColor(255, 0, 0, 0);
	m_IDInputBox.SetBackColor(255, 255, 255, 255);
	m_IDInputBox.SetFont(g_hFont);
	m_IDInputBox.SetTextLimit(MAX_ID_SIZE);
	m_IDInputBox.SetState(UISTATE_NORMAL);
	
	// 메세지 Input Box
	m_MessageInputBox.SetMultiline(TRUE);
	m_MessageInputBox.Init(g_hWnd, IGS_MESSAGE_INPUT_TEXT_WIDTH, IGS_MESSAGE_INPUT_TEXT_HEIGHT, IGS_MESSAGE_INPUT_TEXT_LINE_HEIGHT);
	m_MessageInputBox.SetPosition(GetPos().x+IGS_MESSAGE_INPUT_TEXT_POS_X, GetPos().y+IGS_MESSAGE_INPUT_TEXT_POS_Y);
 	m_MessageInputBox.SetUseScrollbar(FALSE);
	m_MessageInputBox.SetTextLimit(MAX_GIFT_MESSAGE_SIZE);
	m_MessageInputBox.SetFont(g_hFont);
	//m_MessageInputBox.SetOption(UIOPTION_NULL);
	m_MessageInputBox.SetBackColor(0, 0, 0, 0);
	m_MessageInputBox.SetState(UISTATE_NORMAL);
	m_MessageInputBox.SetTextColor(255, 0, 0, 0);
	
	m_IDInputBox.GiveFocus();		// 기본 포커스
}

//--------------------------------------------
// Initialize
#ifdef KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL
		#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
		void CMsgBoxIGSSendGift::Initialize(int iPackageSeq, int iDisplaySeq, int iPriceSeq, DWORD wItemCode, int iCashType,
											unicode::t_char* pszName, unicode::t_char* pszPrice, unicode::t_char* pszPeriod)
		#else // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
											void CMsgBoxIGSSendGift::Initialize(int iPackageSeq, int iDisplaySeq, int iPriceSeq, DWORD wItemCode, 
											unicode::t_char* pszName, unicode::t_char* pszPrice, unicode::t_char* pszPeriod)
		#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#else // KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL
	#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	void CMsgBoxIGSSendGift::Initialize(int iPackageSeq, int iDisplaySeq, int iPriceSeq, int iCashType,
										unicode::t_char* pszName, unicode::t_char* pszPrice, unicode::t_char* pszPeriod)
	#else // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	void CMsgBoxIGSSendGift::Initialize(int iPackageSeq, int iDisplaySeq, int iPriceSeq,
										unicode::t_char* pszName, unicode::t_char* pszPrice, unicode::t_char* pszPeriod)
	#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#endif // KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL
{
	m_iPackageSeq	= iPackageSeq;
	m_iDisplaySeq	= iDisplaySeq;
	m_iPriceSeq		= iPriceSeq;
#ifdef KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL
	m_wItemCode	= wItemCode;
#endif // KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	m_iCashType		= iCashType;
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL

	sprintf(m_szName, GlobalText[3037], pszName);		// "상품명 : %s"
	sprintf(m_szPrice, GlobalText[3038], pszPrice);		// "가격 : %s"
	sprintf(m_szPeriod, GlobalText[3039], pszPeriod);	// "기간 : %s"

	//int m_iNumNoticeLine = SeparateTextIntoLines( GlobalText[2898], txtline[0], NUM_LINE_CMB, MAX_LENGTH_CMB);
	m_iNumNoticeLine = ::DivideStringByPixel(&m_szNotice[0][0], NUM_LINE_CMB, MAX_TEXT_LENGTH, GlobalText[2920], IGS_TEXT_NOTICE_WIDTH);
}

//--------------------------------------------
// Release
void CMsgBoxIGSSendGift::Release()
{
	CNewUIMessageBoxBase::Release();

	UnloadImages();
}

//--------------------------------------------
// Update
bool CMsgBoxIGSSendGift::Update()
{
	m_BtnOk.Update();
	m_BtnCancel.Update();

	m_IDInputBox.DoAction();
	m_MessageInputBox.DoAction();

	if(m_IDInputBox.HaveFocus() == TRUE)
		m_IDInputBox.GetText(m_szID, MAX_ID_SIZE+1); 


	if(m_MessageInputBox.HaveFocus() == TRUE)
		m_MessageInputBox.GetText(m_szMessage, MAX_GIFT_MESSAGE_SIZE);

	if(SEASON3B::IsPress(VK_TAB) == true)
	{
		ChangeInputBoxFocus();
	}

	return true;
}

//--------------------------------------------
// Render
bool CMsgBoxIGSSendGift::Render()
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	RenderMsgBackColor(true);

	RenderFrame();
	RenderTexts();
	RenderButtons();

	m_IDInputBox.Render();
	m_MessageInputBox.Render();

	DisableAlphaBlend();
	return true;
}

//--------------------------------------------
// SetAddCallbackFuncSetButtonInfo
void CMsgBoxIGSSendGift::SetAddCallbackFunc()
{
	AddCallbackFunc(CMsgBoxIGSSendGift::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
	AddCallbackFunc(CMsgBoxIGSSendGift::OKButtonDown, MSGBOX_EVENT_USER_COMMON_OK);
	AddCallbackFunc(CMsgBoxIGSSendGift::CancelButtonDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

//--------------------------------------------
// LButtonUp
CALLBACK_RESULT CMsgBoxIGSSendGift::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	CMsgBoxIGSSendGift* pOwnMsgBox = dynamic_cast<CMsgBoxIGSSendGift*>(pOwner);

	if(pOwnMsgBox)
	{
		if(pOwnMsgBox->m_BtnOk.IsMouseIn() == true)
		{
			g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
			return CALLBACK_BREAK;
		}

		if(pOwnMsgBox->m_BtnCancel.IsMouseIn() == true)
		{
			g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_CANCEL);
			return CALLBACK_BREAK;
		}
	}

	return CALLBACK_CONTINUE;
}

//--------------------------------------------
// OKButtonDown
CALLBACK_RESULT CMsgBoxIGSSendGift::OKButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	CMsgBoxIGSSendGift* pOwnMsgBox = dynamic_cast<CMsgBoxIGSSendGift*>(pOwner);

	if( pOwnMsgBox->m_szID[0] == '\0' )
	{
		CMsgBoxIGSCommon* pMsgBox = NULL;
		CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
		// "에러", "선물 받을 사람의 ID가 없습니다."
		pMsgBox->Initialize(GlobalText[3028], GlobalText[3031]);
	}
#ifdef KJH_FIX_BTS206_INGAMESHOP_SEND_GIFT_MSGBOX_BLANK_ID
	else if( strcmp(pOwnMsgBox->m_szID, Hero->ID) == 0 )
#else // KJH_FIX_BTS206_INGAMESHOP_SEND_GIFT_MSGBOX_BLANK_ID
	if( strcmp(pOwnMsgBox->m_szID, Hero->ID) == 0 )
#endif // KJH_FIX_BTS206_INGAMESHOP_SEND_GIFT_MSGBOX_BLANK_ID
	{
		CMsgBoxIGSCommon* pMsgBox = NULL;
		CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
		// "에러", "자신에게 선물할 수 없습니다."
		pMsgBox->Initialize(GlobalText[3028], GlobalText[3032]);
	}
	else
	{
		CMsgBoxIGSSendGiftConfirm* pMsgBox = NULL;
		CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSSendGiftConfirmLayout), &pMsgBox);
#ifdef KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL

		#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
				pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pOwnMsgBox->m_iPriceSeq, 
					pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iCashType, pOwnMsgBox->m_szID, pOwnMsgBox->m_szMessage, 
					pOwnMsgBox->m_szName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);
		#else // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
				pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pOwnMsgBox->m_iPriceSeq, 
					pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_szID, pOwnMsgBox->m_szMessage, 
					pOwnMsgBox->m_szName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);
		#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL

#else // KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL
	#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
			pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pOwnMsgBox->m_iPriceSeq, 
									pOwnMsgBox->m_iCashType, pOwnMsgBox->m_szID, pOwnMsgBox->m_szMessage, 
									pOwnMsgBox->m_szName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);
	#else // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
			pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pOwnMsgBox->m_iPriceSeq, 
									pOwnMsgBox->m_szID, pOwnMsgBox->m_szMessage, 
									pOwnMsgBox->m_szName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);
	#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#endif // KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL
	}
	
	PlayBuffer(SOUND_CLICK01);
	g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

	return CALLBACK_BREAK;
}

//--------------------------------------------
// CancelButtonDown
CALLBACK_RESULT CMsgBoxIGSSendGift::CancelButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	PlayBuffer(SOUND_CLICK01);
	g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

	return CALLBACK_BREAK;
}

//--------------------------------------------
// SetButtonInfo
void CMsgBoxIGSSendGift::SetButtonInfo()
{
	// 확인 버튼
	m_BtnOk.SetInfo(IMAGE_IGS_BUTTON, GetPos().x+IGS_BTN_OK_POS_X, GetPos().y+IGS_BTN_POS_Y, 
						IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
	m_BtnOk.MoveTextPos(0, -1);
	m_BtnOk.SetText(GlobalText[228]);	
	
	// 취소 버튼
	m_BtnCancel.SetInfo(IMAGE_IGS_BUTTON, GetPos().x+IGS_BTN_CANCEL_POS_X, GetPos().y+IGS_BTN_POS_Y, 
						IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
	m_BtnCancel.MoveTextPos(0, -1);
	m_BtnCancel.SetText(GlobalText[229]);	
}

//--------------------------------------------
// RenderFrame
void CMsgBoxIGSSendGift::RenderFrame()
{
	// Frame
	RenderImage(IMAGE_IGS_FRAME, GetPos().x, GetPos().y, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT);

	// Deco
	RenderImage(IMAGE_IGS_DECO, GetPos().x+IMAGE_IGS_DECO_POS_X, GetPos().y+IMAGE_IGS_DECO_POS_Y, 
					IMAGE_IGS_DECO_WIDTH, IMAGE_IGS_DECO_HEIGHT);

	// Input Textbox
	RenderImage(IMAGE_IGS_INPUTTEXT, GetPos().x+IMAGE_IGS_ID_INPUT_BOX_POS_X, GetPos().y+IMAGE_IGS_ID_INPUT_BOX_POS_Y, 
					IMAGE_IGS_ID_INPUT_BOX_WIDTH, IMAGE_IGS_ID_INPUT_BOX_HEIGHT);

}

//--------------------------------------------
// RenderTexts
void CMsgBoxIGSSendGift::RenderTexts()
{
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetFont(g_hFontBold);

	// Title - "아이템 선물하기"
	g_pRenderText->RenderText(GetPos().x, GetPos().y+IGS_TEXT_TITLE_POS_Y, GlobalText[2916], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

	// "선물할 캐릭터 명"
	g_pRenderText->RenderText(GetPos().x+IGS_TEXT_ID_TITLE_POS_X, GetPos().y+IGS_TEXT_ID_TITLE_POS_Y, 
								GlobalText[2918], IGS_TEXT_ID_TITLE_WIDTH, 0, RT3_SORT_LEFT);

	// "<선물할 메세지>"
	g_pRenderText->SetTextColor(0, 0, 0, 255);
	g_pRenderText->RenderText(GetPos().x, GetPos().y+IGS_TEXT_MESSAGE_TITLE_POS_Y, GlobalText[2919], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);

	// 아이템 정보
	g_pRenderText->SetTextColor(247, 186, 0, 255);
	g_pRenderText->RenderText(GetPos().x+IGS_TEXT_ITEM_INFO_POS_X, GetPos().y+IGS_TEXT_ITEM_INFO_NAME_POS_Y, 
								m_szName, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);
	g_pRenderText->RenderText(GetPos().x+IGS_TEXT_ITEM_INFO_POS_X, GetPos().y+IGS_TEXT_ITEM_INFO_PRICE_POS_Y, 
								m_szPrice, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);
	g_pRenderText->RenderText(GetPos().x+IGS_TEXT_ITEM_INFO_POS_X, GetPos().y+IGS_TEXT_ITEM_INFO_PERIOD_POS_Y, 
								m_szPeriod, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);

	// 주의사항
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	for ( int i=0 ; i < m_iNumNoticeLine; i++)
	{
		g_pRenderText->RenderText(GetPos().x, GetPos().y+IGS_TEXT_NOTICE_POS_Y+i*10, m_szNotice[i], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);
	}
	
#ifdef FOR_WORK
	unicode::t_char szText[256] = { 0, };
	g_pRenderText->SetTextColor(255, 0, 0, 255);
	sprintf(szText, "Package Seq : %d", m_iPackageSeq);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+10, szText, 200, 0, RT3_SORT_LEFT);
	sprintf(szText, "Display Seq : %d", m_iDisplaySeq);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+20, szText, 200, 0, RT3_SORT_LEFT);
	sprintf(szText, "Price Seq : %d", m_iPriceSeq);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+30, szText, 200, 0, RT3_SORT_LEFT);
#ifdef KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL
	sprintf(szText, "ItemCode : %d", m_wItemCode);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+40, szText, 200, 0, RT3_SORT_LEFT);
#endif // KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	sprintf(szText, "CashType : %d", m_iCashType);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+50, szText, 200, 0, RT3_SORT_LEFT);
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#endif // FOR_WORK
}

//--------------------------------------------
// RenderButtons
void CMsgBoxIGSSendGift::RenderButtons()
{
	m_BtnOk.Render();
	m_BtnCancel.Render();
}

//--------------------------------------------
// ChangeInputBoxFocus
void CMsgBoxIGSSendGift::ChangeInputBoxFocus()
{	
	if( m_IDInputBox.HaveFocus() == TRUE )
	{
		m_MessageInputBox.GiveFocus();
	}
	else if( m_MessageInputBox.HaveFocus() == TRUE )
	{
		m_IDInputBox.GiveFocus(); 
	}
	else 
	{
		m_IDInputBox.GiveFocus();
	}
}

//--------------------------------------------
// LoadImages
void CMsgBoxIGSSendGift::LoadImages()
{
	LoadBitmap("Interface\\InGameShop\\Ingame_Bt03.tga",	IMAGE_IGS_BUTTON, GL_LINEAR);
	LoadBitmap("Interface\\InGameShop\\ingame_gift_back01.tga",	IMAGE_IGS_FRAME, GL_LINEAR);
	LoadBitmap("Interface\\InGameShop\\ingame_gift_icon.tga",	IMAGE_IGS_DECO, GL_LINEAR);
	LoadBitmap("Interface\\InGameShop\\ingame_gift_namebox.tga",	IMAGE_IGS_INPUTTEXT, GL_LINEAR);
}

//--------------------------------------------
// UnloadImages
void CMsgBoxIGSSendGift::UnloadImages()
{
	DeleteBitmap(IMAGE_IGS_BUTTON);
	DeleteBitmap(IMAGE_IGS_FRAME);
	DeleteBitmap(IMAGE_IGS_DECO);	
	DeleteBitmap(IMAGE_IGS_INPUTTEXT);	
}

////////////////////////////////////////////////////////////////////
// LayOut
////////////////////////////////////////////////////////////////////
bool CMsgBoxIGSSendGiftLayout::SetLayout()
{
	CMsgBoxIGSSendGift* pMsgBox = GetMsgBox();
	if(false == pMsgBox)
		return false;
	
	if(false == pMsgBox->Create())
		return false;
	
	return true;
}

#endif KJH_ADD_INGAMESHOP_UI_SYSTEM
