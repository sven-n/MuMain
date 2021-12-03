// MsgBoxIGSStorageItemInfo.cpp: implementation of the MsgBoxIGSUseItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"


#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "MsgBoxIGSStorageItemInfo.h"

#include "wsclientinline.h"
#include "DSPlaySound.h"
#include "MsgBoxIGSUseItemConfirm.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMsgBoxIGSStorageItemInfo::CMsgBoxIGSStorageItemInfo()
{
	m_iStorageSeq	= 0;		// 보관함 순번
	m_iStorageItemSeq = 0;		// 보관함 상품 순번
	m_wItemCode		= -1;		// 아이템 코드
	
	m_szName[0]		= '\0';		// 아이템 이름
	m_szNum[0]		= '\0';
	m_szPeriod[0]	= '\0';
	m_szItemType	= '\0';		// 상품구분 (C : 캐시, P : 상품)
}

CMsgBoxIGSStorageItemInfo::~CMsgBoxIGSStorageItemInfo()
{
	Release();
}

//--------------------------------------------
// Create
bool CMsgBoxIGSStorageItemInfo::Create(float fPriority)
{
	LoadImages();

	SetAddCallbackFunc();

	CNewUIMessageBoxBase::Create((IMAGE_IGS_WINDOW_WIDTH/2)-(IMAGE_IGS_FRAME_WIDTH/2), 
		(IMAGE_IGS_WINDOW_HEIGHT/2)-(IMAGE_IGS_FRAME_HEIGHT/2), 
		IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT, fPriority);
	
	if(g_pNewUI3DRenderMng)
	{
		g_pNewUI3DRenderMng->Add3DRenderObj(this);
	}

	SetButtonInfo();

	SetMsgBackOpacity();
	
	return true;
}

//--------------------------------------------
// IsVisible
bool CMsgBoxIGSStorageItemInfo::IsVisible() const
{
	return true;
}

//--------------------------------------------
// Initialize
void CMsgBoxIGSStorageItemInfo::Initialize(int iStorageSeq, int iStorageItemSeq, WORD wItemCode, unicode::t_char szItemType, 
											unicode::t_char* pszName, unicode::t_char* pszNum, unicode::t_char* pszPeriod)
{
	m_iStorageSeq		= iStorageSeq;
	m_iStorageItemSeq	= iStorageItemSeq;
	m_wItemCode			= wItemCode;
	m_szItemType		= szItemType;
	
	// Name
	strcpy(m_szName, pszName);

	// Num
	sprintf(m_szNum, GlobalText[3040], pszNum);		// "수량 : %s"

	// Period
	sprintf(m_szPeriod, GlobalText[3039], pszPeriod);	// "기간 : %s"
}

//--------------------------------------------
// Release
void CMsgBoxIGSStorageItemInfo::Release()
{
	CNewUIMessageBoxBase::Release();

	if(g_pNewUI3DRenderMng)
		g_pNewUI3DRenderMng->Remove3DRenderObj(this);

	UnloadImages();
}

//--------------------------------------------
// Update
bool CMsgBoxIGSStorageItemInfo::Update()
{
	m_BtnUse.Update();
	m_BtnCancel.Update();

	return true;
}

//--------------------------------------------
// Render
bool CMsgBoxIGSStorageItemInfo::Render()
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

//--------------------------------------------
// Render3D
void CMsgBoxIGSStorageItemInfo::Render3D()
{
#ifdef KJH_MOD_INGAMESHOP_PATCH_091028
	if( m_wItemCode==65535 )
		return;
#endif // KJH_MOD_INGAMESHOP_PATCH_091028

	RenderItem3D(GetPos().x+IGS_3DITEM_POS_X, GetPos().y+IGS_3DITEM_POS_Y, 
					IGS_3DITEM_WIDTH, IGS_3DITEM_HEIGHT, m_wItemCode, 0, 0, 0, true);
}

//--------------------------------------------
// SetAddCallbackFunc
void CMsgBoxIGSStorageItemInfo::SetAddCallbackFunc()
{
	AddCallbackFunc(CMsgBoxIGSStorageItemInfo::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
	AddCallbackFunc(CMsgBoxIGSStorageItemInfo::OKButtonDown, MSGBOX_EVENT_USER_COMMON_OK);
	AddCallbackFunc(CMsgBoxIGSStorageItemInfo::CancelButtonDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

//--------------------------------------------
// LButtonUp
CALLBACK_RESULT CMsgBoxIGSStorageItemInfo::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	CMsgBoxIGSStorageItemInfo* pOwnMsgBox = dynamic_cast<CMsgBoxIGSStorageItemInfo*>(pOwner);

	if(pOwnMsgBox)
	{
		if(pOwnMsgBox->m_BtnUse.IsMouseIn() == true)
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
CALLBACK_RESULT CMsgBoxIGSStorageItemInfo::OKButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	CMsgBoxIGSStorageItemInfo* pOwnMsgBox = dynamic_cast<CMsgBoxIGSStorageItemInfo*>(pOwner);
	
	// 사용하기 확인 창
	CMsgBoxIGSUseItemConfirm* pMsgBox = NULL;
	CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSUseItemConfirmLayout), &pMsgBox);
	pMsgBox->Initialize(pOwnMsgBox->m_iStorageSeq, pOwnMsgBox->m_iStorageItemSeq, 
							pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_szItemType, pOwnMsgBox->m_szName);

	PlayBuffer(SOUND_CLICK01);
	g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

	return CALLBACK_BREAK;
}

//--------------------------------------------
// CancelButtonDown
CALLBACK_RESULT CMsgBoxIGSStorageItemInfo::CancelButtonDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	PlayBuffer(SOUND_CLICK01);
	g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
	
	return CALLBACK_BREAK;
}

//--------------------------------------------
// SetButtonInfo
void CMsgBoxIGSStorageItemInfo::SetButtonInfo()
{
	// 확인 버튼
	m_BtnUse.SetInfo(IMAGE_IGS_BUTTON, GetPos().x+IGS_BTN_OK_POS_X, GetPos().y+IGS_BTN_POS_Y, 
						IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
	m_BtnUse.MoveTextPos(0, -1);
	m_BtnUse.SetText(GlobalText[228]);	

	// 취소 버튼
	m_BtnCancel.SetInfo(IMAGE_IGS_BUTTON, GetPos().x+IGS_BTN_CANCEL_POS_X, GetPos().y+IGS_BTN_POS_Y, 
							IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
	m_BtnCancel.MoveTextPos(0, -1);
	m_BtnCancel.SetText(GlobalText[229]);	
}

//--------------------------------------------
// RenderFrame
void CMsgBoxIGSStorageItemInfo::RenderFrame()
{
	if(m_wItemCode == 65535)
		return;

	RenderImage(IMAGE_IGS_FRAME, GetPos().x, GetPos().y, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT);
}

//--------------------------------------------
// RenderTexts
void CMsgBoxIGSStorageItemInfo::RenderTexts()
{
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetFont(g_hFontBold);

	// Title		"상품 정보창"
	g_pRenderText->RenderText(GetPos().x, GetPos().y+IGS_TEXT_TITLE_POS_Y, GlobalText[3049], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

	// Item Name
	g_pRenderText->SetTextColor(255, 255, 0, 255);
	g_pRenderText->RenderText(GetPos().x, GetPos().y+IGS_TEXT_ITEM_NAME_POS_Y, m_szName, IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	
	// Item Info
 	g_pRenderText->RenderText(GetPos().x+IGS_TEXT_ITEM_INFO_POS_X, GetPos().y+IGS_TEXT_ITEM_INFO_NUM_POS_Y, m_szNum, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);
	g_pRenderText->RenderText(GetPos().x+IGS_TEXT_ITEM_INFO_POS_X, GetPos().y+IGS_TEXT_ITEM_INFO_PERIOD_POS_Y, m_szPeriod, IGS_TEXT_ITEM_INFO_WIDTH, 0, RT3_SORT_LEFT);

#ifdef FOR_WORK
	// debug
	unicode::t_char szText[256] = {0,};
	g_pRenderText->SetTextColor(255, 0, 0, 255);
	if( m_wItemCode == 65535 )
	{
		sprintf(szText, "아이템코드가 없습니다.");
	}
	else
	{
		sprintf(szText, "ItemCode : %d (%d, %d)", m_wItemCode, m_wItemCode/MAX_ITEM_INDEX, m_wItemCode%MAX_ITEM_INDEX);
	}
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+10, szText, 150, 0, RT3_SORT_LEFT);
	sprintf(szText, "Storage Seq : %d", m_iStorageSeq);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+20, szText, 150, 0, RT3_SORT_LEFT);
	sprintf(szText, "Storage ItemSeq : %d", m_iStorageItemSeq);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+30, szText, 150, 0, RT3_SORT_LEFT);
#endif // FOR_WORK
}

//--------------------------------------------
// RenderButtons
void CMsgBoxIGSStorageItemInfo::RenderButtons()
{
	m_BtnUse.Render();
	m_BtnCancel.Render();
}

//--------------------------------------------
// LoadImages
void CMsgBoxIGSStorageItemInfo::LoadImages()
{
	LoadBitmap("Interface\\InGameShop\\Ingame_Bt03.tga",	IMAGE_IGS_BUTTON, GL_LINEAR);
	LoadBitmap("Interface\\InGameShop\\ingame_Box_List_A.tga",		IMAGE_IGS_FRAME, GL_LINEAR);
}

//--------------------------------------------
// UnloadImages
void CMsgBoxIGSStorageItemInfo::UnloadImages()
{
	DeleteBitmap(IMAGE_IGS_BUTTON);
	DeleteBitmap(IMAGE_IGS_FRAME);
}

////////////////////////////////////////////////////////////////////
// LayOut
////////////////////////////////////////////////////////////////////
bool CMsgBoxIGSStorageItemInfoLayout::SetLayout()
{
	CMsgBoxIGSStorageItemInfo* pMsgBox = GetMsgBox();
	if(false == pMsgBox)
		return false;
	
	if(false == pMsgBox->Create())
		return false;
	
	return true;
}

#endif KJH_ADD_INGAMESHOP_UI_SYSTEM
