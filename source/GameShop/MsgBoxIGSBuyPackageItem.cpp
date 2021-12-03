// MsgBoxIGSBuyPackageItem.cpp: implementation of the CMsgBoxIGSBuyPackageItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "MsgBoxIGSBuyPackageItem.h"

#include "UsefulDef.h"
#include "DSPlaySound.h"
#include "NewUISystem.h"
#include "InGameShopSystem.h"
#include "MsgBoxIGSBuyConfirm.h"
#include "MsgBoxIGSSendGift.h"

#ifdef LEM_FIX_WARNINGMSG_BUYITEM	// MsgBoxIGSCommon.h 인클루드 [lem.2010.7.28]
	#include "MsgBoxIGSCommon.h" 
#endif // LEM_FIX_WARNINGMSG_BUYITEM
////////////////////////////////////////////////////////////////////
// 단일/패키지 상품 구매창
////////////////////////////////////////////////////////////////////
CMsgBoxIGSBuyPackageItem::CMsgBoxIGSBuyPackageItem()
{
	m_iPackageSeq	= 0;
	m_iDisplaySeq	= 0;
	m_wItemCode		= -1;

#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	m_iCashType			= -1;
	m_iSelectedCashType	= -1;
	m_bIsSendGift		= false;
#else KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	m_iCashType		= 0;
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	
	m_szPackageName[0]	= '\0';
	m_szPrice[0]		= '\0';
	m_szPeriod[0]		= '\0';

	for(int i=0 ; i<UIMAX_TEXT_LINE ; i++ )
	{
		m_szDescription[i][0] = '\0';
	}
}

CMsgBoxIGSBuyPackageItem::~CMsgBoxIGSBuyPackageItem()
{
	Release();
}

//--------------------------------------------
// Create
bool CMsgBoxIGSBuyPackageItem::Create(float fPriority)
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

	CreateListBox();
	SetButtonInfo();
	
#ifdef PBG_ADD_MSGBACKOPACITY
	SetMsgBackOpacity();
#endif //PBG_ADD_MSGBACKOPACITY
	return true;
}

//--------------------------------------------
// Initialize
void CMsgBoxIGSBuyPackageItem::Initialize(CShopPackage* pPackage)
{
	int iProductSeq;
	int iValue = 0;
	unicode::t_char szText[MAX_TEXT_LENGTH] = {'\0', };

	m_iPackageSeq	= pPackage->PackageProductSeq;
	m_iDisplaySeq	= pPackage->ProductDisplaySeq;


#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
		m_iCashType		= pPackage->CashType;
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL

#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	switch( m_iCashType )
	{
	case 0:		// Select Wcoin C/P
		{
			m_BtnSelectCashType.ChangeFrame(IGS_BUY_SELECT_INDEX_CASHTYPE_PREPAID);
			m_iCashType = IGS_BUY_CASHTYPE_PREPAID;
		}break;
	case 508:	// Wcoin(C)
		{
			m_BtnSelectCashType.ChangeFrame(IGS_BUY_SELECT_INDEX_CASHTYPE_CREDIT);
			m_BtnSelectCashType.LockButtonindex(IGS_BUY_SELECT_INDEX_CASHTYPE_PREPAID);
			m_iCashType = IGS_BUY_CASHTYPE_CREDIT;
		}break;
	case 509:	// Wcion(P)
		{
			m_BtnSelectCashType.ChangeFrame(IGS_BUY_SELECT_INDEX_CASHTYPE_PREPAID);
			m_BtnSelectCashType.LockButtonindex(IGS_BUY_SELECT_INDEX_CASHTYPE_CREDIT);
			m_iCashType = IGS_BUY_CASHTYPE_PREPAID;
		}break;
	default:	// 에러
		{
			m_BtnBuy.SetEnable(false);
			m_BtnSelectCashType.LockButtonindex(IGS_BUY_SELECT_INDEX_CASHTYPE_CREDIT);
			m_BtnSelectCashType.LockButtonindex(IGS_BUY_SELECT_INDEX_CASHTYPE_PREPAID);
		}
	}

	// 선물가능 여부
	if( pPackage->GiftFlag == 184 )
	{
		m_bIsSendGift = true;
		if( m_BtnSelectCashType.GetCurButtonIndex() == IGS_BUY_SELECT_INDEX_CASHTYPE_CREDIT )
		{
			m_BtnPresent.SetEnable(true);	// 선물가능
		}
		else
		{
			m_BtnPresent.SetEnable(false);		// 선물불가능
		}
	}
	else
	{
		m_bIsSendGift = false;
		m_BtnPresent.SetEnable(false);		// 선물불가능
	}
#else // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
#ifdef KJH_MOD_INGAMESHOP_GIFT_FLAG
	// 선물가능 여부
	if( pPackage->GiftFlag == 184)		
	{
		m_BtnPresent.SetEnable(true);	// 선물가능
	}
	else
	{
		m_BtnPresent.SetEnable(false);
	}
#endif // KJH_MOD_INGAMESHOP_GIFT_FLAG
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL

	// Name
	strncpy(m_szPackageName, pPackage->PackageProductName, MAX_TEXT_LENGTH);

	// Price
	ConvertGold(pPackage->Price, szText);
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	strncpy(m_szPrice, szText, MAX_TEXT_LENGTH);
	strncpy(m_szPriceUnit, pPackage->ProductCashName, MAX_TEXT_LENGTH);
#else // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	sprintf(m_szPrice, "%s %s", szText, pPackage->PricUnitName);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL

	// Period
	pPackage->SetProductSeqFirst();
	pPackage->GetProductSeqNext(iProductSeq);
	
	g_InGameShopSystem->GetProductInfoFromProductSeq(iProductSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_USE_LIMIT_PERIOD, iValue, szText);

	if( iValue > 0 )
	{	
		sprintf(m_szPeriod, "%d %s", iValue, szText);
	}
	else 
	{
		sprintf(m_szPeriod, "-");
	}
	
	// ItemCode
	m_wItemCode = atoi(pPackage->InGamePackageID);

#ifdef LEM_FIX_INGAMESHOP_ITEM_CARD
	if( m_iPackageSeq == ITEM_POTION + 91 )	m_BtnPresent.SetEnable(false);
#endif
	
	// Item정보 Text
	ZeroMemory(m_szDescription, sizeof(unicode::t_char)*UIMAX_TEXT_LINE*MAX_TEXT_LENGTH);
	
	g_pRenderText->SetFont(g_hFont);
	int nLine = ::DivideStringByPixel(&m_szDescription[0][0], UIMAX_TEXT_LINE, MAX_TEXT_LENGTH, pPackage->Description, IGS_LISTBOX_WIDTH, false, '#');

	for (int i=0; i < nLine; ++i)
	{
		m_PackageInfo.AddText(m_szDescription[i]);
	}
}

//--------------------------------------------
// Release
void CMsgBoxIGSBuyPackageItem::Release()
{
	CNewUIMessageBoxBase::Release();

	if(g_pNewUI3DRenderMng)
		g_pNewUI3DRenderMng->Remove3DRenderObj(this);

	ReleaseListBox();

	UnloadImages();
}

//--------------------------------------------
// Release
bool CMsgBoxIGSBuyPackageItem::Update()
{
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	int iSelecteCashType = m_BtnSelectCashType.UpdateMouseEvent();

	if( (m_iSelectedCashType != iSelecteCashType) )
	{
		if( iSelecteCashType == IGS_BUY_SELECT_INDEX_CASHTYPE_CREDIT )
		{
			// Wcoin(C)
			m_iCashType = IGS_BUY_CASHTYPE_CREDIT;
			if( m_bIsSendGift == true )
			{
				m_BtnPresent.SetEnable(true);	// 선물가능
			}
		}
		else if( iSelecteCashType == IGS_BUY_SELECT_INDEX_CASHTYPE_PREPAID )
		{
			// WCoin(P)
			m_iCashType = IGS_BUY_CASHTYPE_PREPAID;
			if( m_bIsSendGift == true )
			{
				m_BtnPresent.SetEnable(false);	// 선물불가능
			}
		}
	}

	m_iSelectedCashType = iSelecteCashType;
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL

	m_BtnBuy.Update();
	m_BtnCancel.Update();
	m_BtnPresent.Update();

	ListBoxDoAction();

	return true;
}

//--------------------------------------------
// Render
bool CMsgBoxIGSBuyPackageItem::Render()
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

#ifdef PBG_ADD_MSGBACKOPACITY
	RenderMsgBackColor(true);
#endif //PBG_ADD_MSGBACKOPACITY

	RenderFrame();
	RenderTexts();
	RenderButtons();
	RenderListBox();

	DisableAlphaBlend();
	return true;
}

//--------------------------------------------
// RenderFrame
void CMsgBoxIGSBuyPackageItem::RenderFrame()
{
	// 배경
	RenderImage(IMAGE_IGS_FRAME, GetPos().x, GetPos().y, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT);

#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	// 캐시포인트 선택 UI
	RenderImage(IMAGE_IGS_CASHPOINT_TYPE, GetPos().x+IGS_CASHPOINT_TYPE_POS_X, GetPos().y+IGS_CASHPOINT_TYPE_POS_Y, 
					IMAGE_IGS_CASHPOINT_TYPE_WIDTH, IMAGE_IGS_CASHPOINT_TYPE_HEIGHT);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
}

//--------------------------------------------
// IsVisible
bool CMsgBoxIGSBuyPackageItem::IsVisible() const
{
	return true;
}

//--------------------------------------------
// SetButtonInfo
void CMsgBoxIGSBuyPackageItem::SetButtonInfo()
{
	// 구매 버튼
	m_BtnBuy.SetInfo(IMAGE_IGS_BUTTON, GetPos().x+IGS_BTN_BUY_POS_X, GetPos().y+IGS_BTN_POS_Y, 
		IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
	m_BtnBuy.MoveTextPos(-1, -1);
	m_BtnBuy.SetText(GlobalText[2891]);	 

	// 선물 버튼
	m_BtnPresent.SetInfo(IMAGE_IGS_BUTTON, GetPos().x+IGS_BTN_PRESENT_POS_X, GetPos().y+IGS_BTN_POS_Y, 
						IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
	m_BtnPresent.MoveTextPos(-1, -1);
	m_BtnPresent.SetText(GlobalText[2892]);	
	
	// 취소 버튼
	m_BtnCancel.SetInfo(IMAGE_IGS_BUTTON, GetPos().x+IGS_BTN_CANCEL_POS_X, GetPos().y+IGS_BTN_POS_Y, 
						IMAGE_IGS_BTN_WIDTH, IMAGE_IGS_BTN_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_CUSTOM, true);
	m_BtnCancel.MoveTextPos(-1, -1);
	m_BtnCancel.SetText(GlobalText[229]);	
	
	// C/P 선택 버튼
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	m_BtnSelectCashType.UnRegisterRadioButton();
#ifdef KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
	m_BtnSelectCashType.CreateRadioGroup(2, IMAGE_IGS_CASHPOINT_CHECK_BOX, false, true, true);
#else // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
	m_BtnSelectCashType.CreateRadioGroup(2, IMAGE_IGS_CASHPOINT_CHECK_BOX);
#endif // KJH_MOD_RADIOBTN_MOUSE_OVER_IMAGE
	m_BtnSelectCashType.ChangeRadioButtonInfo(false, GetPos().x+IGS_CASHPOINT_CHECK_BOX_POS_X, GetPos().y+IGS_CASHPOINT_CHECK_BOX_POS_Y,
												IMAGE_IGS_CASHPOINT_CHECK_BOX_WIDTH, IMAGE_IGS_CASHPOINT_CHECK_BOX_WIDTH, IGS_CASHPOINT_CHECK_BOX_DISTANCE_Y);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
}

//--------------------------------------------
// RenderTexts
void CMsgBoxIGSBuyPackageItem::RenderTexts()
{
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetFont(g_hFontBold);

	// Title - "구매창"
	g_pRenderText->RenderText(GetPos().x, GetPos().y+IGS_TEXT_TITLE_POS_Y, 
								GlobalText[2890], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);


	g_pRenderText->SetTextColor(247, 186, 0, 255);

	// Name
	g_pRenderText->RenderText(GetPos().x, GetPos().y+IGS_TEXT_NAME_POS_Y, 
								m_szPackageName, IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);

	g_pRenderText->SetFont(g_hFont);

#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	// 가격
	g_pRenderText->SetTextColor(255, 238, 161, 255);
	g_pRenderText->RenderText(GetPos().x+IGS_TEXT_PRICE_POS_X, GetPos().y+IGS_TEXT_PRICE_POX_Y, 
								m_szPrice, IGS_TEXT_PRICE_WIDTH, 0, RT3_SORT_RIGHT);

	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->RenderText(GetPos().x+IGS_TEXT_PRICE_POS_X, GetPos().y+IGS_TEXT_PRICE_POX_Y+14, 
								m_szPriceUnit, IGS_TEXT_PRICE_WIDTH+4, 0, RT3_SORT_RIGHT);
#else // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	g_pRenderText->SetTextColor(255, 238, 161, 255);
	
	// 가격
	g_pRenderText->RenderText(GetPos().x+IGS_TEXT_PRICE_POS_X, GetPos().y+IGS_TEXT_PRICE_POX_Y, 
								m_szPrice, IGS_TEXT_PRICE_WIDTH, 0, RT3_SORT_RIGHT);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	
#ifdef FOR_WORK
	// debug
	unicode::t_char szText[256] = {'\0', };
	g_pRenderText->SetTextColor(255, 0, 0, 255);
	if( m_wItemCode == 65535 )
	{
		sprintf(szText, "아이템코드가 없습니다.");
	}
	else
	{
		sprintf(szText, "ItemCode : %d (%d, %d)", m_wItemCode, m_wItemCode/MAX_ITEM_INDEX, m_wItemCode%MAX_ITEM_INDEX);
	}
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+10, szText, 200, 0, RT3_SORT_LEFT);
	sprintf(szText, "Package Seq : %d", m_iPackageSeq);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+20, szText, 200, 0, RT3_SORT_LEFT);
	sprintf(szText, "Display Seq : %d", m_iDisplaySeq);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+30, szText, 200, 0, RT3_SORT_LEFT);
	sprintf(szText, "Price Seq : 0");
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+40, szText, 200, 0, RT3_SORT_LEFT);
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
		sprintf(szText, "CashType : %d", m_iCashType);
		g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+50, szText, 200, 0, RT3_SORT_LEFT);
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL



#endif // FOR_WORK
	
}

//--------------------------------------------
// RenderButtons
void CMsgBoxIGSBuyPackageItem::RenderButtons()
{
	m_BtnBuy.Render();
	m_BtnPresent.Render();
	m_BtnCancel.Render();

#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	m_BtnSelectCashType.Render();
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
}

//--------------------------------------------
// Render3D
void CMsgBoxIGSBuyPackageItem::Render3D()
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
void CMsgBoxIGSBuyPackageItem::SetAddCallbackFunc()
{
	AddCallbackFunc(CMsgBoxIGSBuyPackageItem::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
	AddCallbackFunc(CMsgBoxIGSBuyPackageItem::BuyBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
	AddCallbackFunc(CMsgBoxIGSBuyPackageItem::PresentBtnDown, MSGBOX_EVENT_USER_CUSTOM_INGAMESHOP_PRESENT);
	AddCallbackFunc(CMsgBoxIGSBuyPackageItem::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

//--------------------------------------------
// LButtonUp
CALLBACK_RESULT CMsgBoxIGSBuyPackageItem::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	CMsgBoxIGSBuyPackageItem* pOwnMsgBox = dynamic_cast<CMsgBoxIGSBuyPackageItem*>(pOwner);
	if(pOwnMsgBox)
	{
		if(pOwnMsgBox->m_BtnBuy.IsMouseIn() == true)
		{
			g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_COMMON_OK);
			return CALLBACK_BREAK;
		}

		if(pOwnMsgBox->m_BtnPresent.IsMouseIn() == true)
		{
			g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_USER_CUSTOM_INGAMESHOP_PRESENT);
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


//----------------------------------------------------------------------------------------
// Function: 구매요청시 경고가 필요한 아이템에 대하여 추가적으로 경고창을 띄워준다.
// Input   : 아이템 인덱스번호 
// Output  : void
//------------------------------------------------------------------------[lem_2010.7.28]-
#ifdef LEM_FIX_WARNINGMSG_BUYITEM	// Add_WarningMsgBuyItem 함수 [lem.2010.7.28]
bool CMsgBoxIGSBuyPackageItem::Add_WarningMsgBuyItem( int _nItemIndex )
{
	int  nTextSub	= 0;		// 메세지박스 텝 제목
	int	 nTextMsg	= 0;		// 메세지박스 내용
	bool bLimit		= false;
	if( _nItemIndex == ITEM_HELPER+93 || _nItemIndex == ITEM_HELPER+94 )	
	{
		nTextSub = 2902;
		nTextMsg = 3052;
		bLimit	 = !IsMasterLevel(Hero->Class);
	}
	if( !bLimit )	return false;
	// 경고창 생성
	CMsgBoxIGSCommon* pMsgBox	= NULL;
	CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
	pMsgBox->Initialize(GlobalText[nTextSub], GlobalText[nTextMsg]);
	
	return true;
}
#endif // LEM_FIX_WARNINGMSG_BUYITEM

//--------------------------------------------
// OkBtnDown
CALLBACK_RESULT CMsgBoxIGSBuyPackageItem::BuyBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	CMsgBoxIGSBuyPackageItem* pOwnMsgBox = dynamic_cast<CMsgBoxIGSBuyPackageItem*>(pOwner);
#ifdef LEM_FIX_WARNINGMSG_BUYITEM	// Add_WarningMsgBuyItem 호출 [lem.2010.7.28]
	bool	bLimit= false;
	bLimit = pOwnMsgBox->Add_WarningMsgBuyItem( pOwnMsgBox->m_wItemCode );
#endif	// LEM_FIX_WARNINGMSG_BUYITEM

#ifdef LEM_FIX_SEAL_ITEM_BUYLIMIT
	if( bLimit )
	{
		PlayBuffer(SOUND_CLICK01);
		g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
		
		return CALLBACK_BREAK;
	}
#endif // LEM_FIX_SEAL_ITEM_BUYLIMIT

	// 구매확인 MessageBox
	CMsgBoxIGSBuyConfirm* pMsgBox = NULL;
	CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSBuyConfirmLayout), &pMsgBox);


#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	unicode::t_char szPrice[MAX_TEXT_LENGTH] = {'\0', };
	sprintf(szPrice, "%s %s", pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPriceUnit);
	pMsgBox->Initialize(pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, 0, pOwnMsgBox->m_iCashType,
						pOwnMsgBox->m_szPackageName, szPrice, pOwnMsgBox->m_szPeriod);
#else // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	pMsgBox->Initialize(pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, 0, pOwnMsgBox->m_iCashType,
						pOwnMsgBox->m_szPackageName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
#else // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	pMsgBox->Initialize(pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, 0, 
						pOwnMsgBox->m_szPackageName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL

	PlayBuffer(SOUND_CLICK01);
	g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

	return CALLBACK_BREAK;
}

//--------------------------------------------
// PresentBtnDown
CALLBACK_RESULT CMsgBoxIGSBuyPackageItem::PresentBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	CMsgBoxIGSBuyPackageItem* pOwnMsgBox = dynamic_cast<CMsgBoxIGSBuyPackageItem*>(pOwner);
	
	// 선물하기 MessageBox
	CMsgBoxIGSSendGift* pMsgBox = NULL;
	CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSSendGiftLayout), &pMsgBox);

#ifdef KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	unicode::t_char szPrice[MAX_TEXT_LENGTH] = {'\0', };
	sprintf(szPrice, "%s %s", pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPriceUnit);
	pMsgBox->Initialize(pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, 0, pOwnMsgBox->m_iCashType,
						pOwnMsgBox->m_szPackageName, szPrice, pOwnMsgBox->m_szPeriod);
#else // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, 0, pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iCashType,
						pOwnMsgBox->m_szPackageName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);
#endif KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
#else // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, 0, pOwnMsgBox->m_wItemCode, 
						pOwnMsgBox->m_szPackageName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#else // KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, 0, pOwnMsgBox->m_iCashType,
						pOwnMsgBox->m_szPackageName, m_szPrice, pOwnMsgBox->m_szPeriod);
#else // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, 0, pOwnMsgBox->m_iCashType,
						pOwnMsgBox->m_szPackageName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
#else // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, 0, 
						pOwnMsgBox->m_szPackageName, pOwnMsgBox->m_szPrice, pOwnMsgBox->m_szPeriod);
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#endif // KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL

	PlayBuffer(SOUND_CLICK01);
	g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
	
	return CALLBACK_BREAK;
}

//--------------------------------------------
// CancelBtnDown
CALLBACK_RESULT CMsgBoxIGSBuyPackageItem::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	PlayBuffer(SOUND_CLICK01);
	g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

	return CALLBACK_BREAK;
}

//--------------------------------------------
// LoadImages
void CMsgBoxIGSBuyPackageItem::LoadImages()
{
	LoadBitmap("Interface\\InGameShop\\Ingame_pack_back01.tga", IMAGE_IGS_FRAME, GL_LINEAR);
	LoadBitmap("Interface\\InGameShop\\Ingame_Bt03.tga",		IMAGE_IGS_BUTTON, GL_LINEAR);
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	LoadBitmap("Interface\\InGameShop\\IGS_wcoin_c_p.tga",		IMAGE_IGS_CASHPOINT_TYPE, GL_LINEAR);
	LoadBitmap("Interface\\InGameShop\\IGS_wcoin_check.jpg",	IMAGE_IGS_CASHPOINT_CHECK_BOX, GL_LINEAR);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
}

//--------------------------------------------
// UnloadImages
void CMsgBoxIGSBuyPackageItem::UnloadImages()
{
	DeleteBitmap(IMAGE_IGS_FRAME);
	DeleteBitmap(IMAGE_IGS_BUTTON);
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	DeleteBitmap(IMAGE_IGS_CASHPOINT_TYPE);
	DeleteBitmap(IMAGE_IGS_CASHPOINT_CHECK_BOX);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
}

////////////////////////////////////////////////////////////////////
// 리스트 박스 관련 메소드
////////////////////////////////////////////////////////////////////

//--------------------------------------------
// CreateListBox
void CMsgBoxIGSBuyPackageItem::CreateListBox()
{
	// 외부에서 설정 위치값
	m_PackageInfo.SetPosition(GetPos().x+IGS_LISTBOX_POS_X, GetPos().y+IGS_LISTBOX_POS_Y);

	// 라인 선택시 음영나타내지 않는다
	m_PackageInfo.SetLineColorRender(false);
}

//--------------------------------------------
// RenderListBox
void CMsgBoxIGSBuyPackageItem::RenderListBox()
{
	// 리스트 박스 그리기
	if(m_PackageInfo.GetLineNum() != 0)
		m_PackageInfo.Render();
}

//--------------------------------------------
// ListBoxDoAction
void CMsgBoxIGSBuyPackageItem::ListBoxDoAction()
{
	m_PackageInfo.DoAction();
}

//--------------------------------------------
// ReleaseListBox
void CMsgBoxIGSBuyPackageItem::ReleaseListBox()
{
	m_PackageInfo.Clear();
}

////////////////////////////////////////////////////////////////////
// LayOut
////////////////////////////////////////////////////////////////////
bool CMsgBoxBuyPackageItemLayout::SetLayout()
{
	CMsgBoxIGSBuyPackageItem* pMsgBox = GetMsgBox();
	if(false == pMsgBox)
		return false;
	
	if(false == pMsgBox->Create())
		return false;
	
	return true;
}

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
