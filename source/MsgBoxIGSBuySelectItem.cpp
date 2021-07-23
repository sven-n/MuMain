// MsgBoxIGSBuySelectItem.cpp: implementation of the CMsgBoxIGSBuySelectItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "MsgBoxIGSBuySelectItem.h"

#include "DSPlaySound.h"
#include "NewUISystem.h"
#include "MsgBoxIGSBuyConfirm.h"
#include "MsgBoxIGSSendGift.h"


////////////////////////////////////////////////////////////////////
// 선택 상품 구매창
////////////////////////////////////////////////////////////////////
CMsgBoxIGSBuySelectItem::CMsgBoxIGSBuySelectItem()
{
	m_iPackageSeq = 0;
	m_iDisplaySeq = 0;
	m_wItemCode = -1;

#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	m_iCashType			= -1;
	m_iSelectedCashType	= -1;
	m_bIsSendGift		= false;
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL

	m_iDescriptionLine = 0;
	
	m_szPackageName[0] = '\0';
	m_szPrice[0] = '\0';

	for(int i=0 ; i<UIMAX_TEXT_LINE ; i++)
	{
		m_szDescription[i][0] = '\0';
	}
}

CMsgBoxIGSBuySelectItem::~CMsgBoxIGSBuySelectItem()
{
	Release();
}

//--------------------------------------------
// Create
bool CMsgBoxIGSBuySelectItem::Create(float fPriority)
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
void CMsgBoxIGSBuySelectItem::Initialize(CShopPackage* pPackage)
{
	int iProductSeq, iPriceSeq;

	m_wItemCode = atoi(pPackage->InGamePackageID);
	
	m_iPackageSeq = pPackage->PackageProductSeq;
	m_iDisplaySeq = pPackage->ProductDisplaySeq;
	
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	m_iCashType		= pPackage->CashType;

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

	// 상세설명
	ZeroMemory(m_szDescription, sizeof(unicode::t_char)*UIMAX_TEXT_LINE*MAX_TEXT_LENGTH);
	
	g_pRenderText->SetFont(g_hFont);
	m_iDescriptionLine = ::DivideStringByPixel(&m_szDescription[0][0], UIMAX_TEXT_LINE, MAX_TEXT_LENGTH, 
												pPackage->Description, IGS_TEXT_DISCRIPTION_WIDTH, false, '#');

	pPackage->SetProductSeqFirst();
	if( pPackage->GetProductSeqNext(iProductSeq) == false )
	{
		// 에러처리
	}

	pPackage->SetPriceSeqFirst();
	while( pPackage->GetPriceSeqNext(iPriceSeq) )
	{
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
		AddData(pPackage->PackageProductSeq, pPackage->ProductDisplaySeq, iPriceSeq, iProductSeq, pPackage->ProductCashName);
#else // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
		AddData(pPackage->PackageProductSeq, pPackage->ProductDisplaySeq, iPriceSeq, iProductSeq, pPackage->PricUnitName, pPackage->CashType);
#else // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
		AddData(pPackage->PackageProductSeq, pPackage->ProductDisplaySeq, iPriceSeq, iProductSeq, pPackage->PricUnitName);
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	}
}

//--------------------------------------------
// Release
void CMsgBoxIGSBuySelectItem::Release()
{
	CNewUIMessageBoxBase::Release();

	if(g_pNewUI3DRenderMng)
		g_pNewUI3DRenderMng->Remove3DRenderObj(this);

	ReleaseListBox();

	UnloadImages();
}

//--------------------------------------------
// Update
bool CMsgBoxIGSBuySelectItem::Update()
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
bool CMsgBoxIGSBuySelectItem::Render()
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
// Render
void CMsgBoxIGSBuySelectItem::RenderFrame()
{
	// MsgBox 배경
	RenderImage(IMAGE_IGS_MGSBOX_BACK, GetPos().x, GetPos().y, IMAGE_IGS_FRAME_WIDTH, IMAGE_IGS_FRAME_HEIGHT);

#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	// 캐시포인트 선택 UI
	RenderImage(IMAGE_IGS_CASHPOINT_TYPE, GetPos().x+IGS_CASHPOINT_TYPE_POS_X, GetPos().y+IGS_CASHPOINT_TYPE_POS_Y, 
					IMAGE_IGS_CASHPOINT_TYPE_WIDTH, IMAGE_IGS_CASHPOINT_TYPE_HEIGHT);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
}

//--------------------------------------------
// RenderTexts
void CMsgBoxIGSBuySelectItem::RenderTexts()
{
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetFont(g_hFontBold);
	
	// Title - "구매창"
	g_pRenderText->RenderText(GetPos().x, GetPos().y+IGS_TEXT_TITLE_POS_Y, 
								GlobalText[2890], IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);	// "구매창"
	
	
	g_pRenderText->SetTextColor(255, 255, 0, 255);
	
	// Item 제목
	g_pRenderText->RenderText(GetPos().x, GetPos().y+IGS_TEXT_NAME_POS_Y, 
								m_szPackageName, IMAGE_IGS_FRAME_WIDTH, 0, RT3_SORT_CENTER);
	
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	
	// Item 상세설명
	for(int i=0 ; i<m_iDescriptionLine ; i++)
	{
		g_pRenderText->RenderText(GetPos().x+IGS_TEXT_ATTR_POS_X, GetPos().y+IGS_TEXT_ATTR_POS_Y+(i*10), 
								m_szDescription[i], IGS_TEXT_ATTR_WIDTH, 0, RT3_SORT_LEFT);
	}
	
	// 가격
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	g_pRenderText->SetTextColor(255, 255, 0, 255);
	g_pRenderText->RenderText(GetPos().x+IGS_TEXT_PRICE_POS_X, GetPos().y+IGS_TEXT_PRICE_POX_Y, 
								m_szPrice, IGS_TEXT_PRICE_WIDTH, 0, RT3_SORT_RIGHT);

	g_pRenderText->SetTextColor(255, 255, 255, 255);	
	g_pRenderText->RenderText(GetPos().x+IGS_TEXT_PRICE_POS_X, GetPos().y+IGS_TEXT_PRICE_POX_Y+14, 
								m_szPriceUnit, IGS_TEXT_PRICE_WIDTH+2, 0, RT3_SORT_RIGHT);
#else // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	g_pRenderText->RenderText(GetPos().x+IGS_TEXT_PRICE_POS_X, GetPos().y+IGS_TEXT_PRICE_POX_Y, 
								m_szPrice, IGS_TEXT_PRICE_WIDTH, 0, RT3_SORT_RIGHT);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL

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
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+10, szText, 200, 0, RT3_SORT_LEFT);
	sprintf(szText, "Package Seq : %d", m_iPackageSeq);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+20, szText, 200, 0, RT3_SORT_LEFT);
	sprintf(szText, "Display Seq : %d", m_iDisplaySeq);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+30, szText, 200, 0, RT3_SORT_LEFT);
	sprintf(szText, "Price Seq : %d", m_SelectBuyListBox.GetSelectedText()->m_iPriceSeq);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+40, szText, 200, 0, RT3_SORT_LEFT);
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	sprintf(szText, "Cash Type : %d", m_iCashType);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+50, szText, 200, 0, RT3_SORT_LEFT);
#else // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	sprintf(szText, "Cash Type : %d", m_SelectBuyListBox.GetSelectedText()->m_iCashType);
	g_pRenderText->RenderText(GetPos().x+IMAGE_IGS_FRAME_WIDTH, GetPos().y+50, szText, 200, 0, RT3_SORT_LEFT);
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
#endif // FOR_WORK

}

//--------------------------------------------
// RenderButtons
void CMsgBoxIGSBuySelectItem::RenderButtons()
{
	m_BtnPresent.Render();
	m_BtnBuy.Render();
	m_BtnCancel.Render();

#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	m_BtnSelectCashType.Render();
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
}

//--------------------------------------------
// Render3D
void CMsgBoxIGSBuySelectItem::Render3D()
{
#ifdef KJH_MOD_INGAMESHOP_PATCH_091028
	if( m_wItemCode==65535 )
		return;
#endif // KJH_MOD_INGAMESHOP_PATCH_091028

	RenderItem3D(GetPos().x+IGS_3DITEM_POS_X, GetPos().y+IGS_3DITEM_POS_Y, 
					IGS_3DITEM_WIDTH, IGS_3DITEM_HEIGHT, m_wItemCode, 0, 0, 0, true);
}

//--------------------------------------------
// IsVisible
bool CMsgBoxIGSBuySelectItem::IsVisible() const
{
	return true;
}

//--------------------------------------------
// SetAddCallbackFunc
void CMsgBoxIGSBuySelectItem::SetAddCallbackFunc()
{
	AddCallbackFunc(CMsgBoxIGSBuySelectItem::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
	AddCallbackFunc(CMsgBoxIGSBuySelectItem::BuyBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
	AddCallbackFunc(CMsgBoxIGSBuySelectItem::PresentBtnDown, MSGBOX_EVENT_USER_CUSTOM_INGAMESHOP_PRESENT);
	AddCallbackFunc(CMsgBoxIGSBuySelectItem::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

//--------------------------------------------
// LButtonUp
CALLBACK_RESULT CMsgBoxIGSBuySelectItem::LButtonUp(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	CMsgBoxIGSBuySelectItem* pOwnMsgBox = dynamic_cast<CMsgBoxIGSBuySelectItem*>(pOwner);

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

//--------------------------------------------
// LButtonUp
CALLBACK_RESULT CMsgBoxIGSBuySelectItem::BuyBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	CMsgBoxIGSBuySelectItem* pOwnMsgBox = dynamic_cast<CMsgBoxIGSBuySelectItem*>(pOwner);
	IGS_SelectBuyItem* pItem = pOwnMsgBox->m_SelectBuyListBox.GetSelectedText();
	
	// 구매하기 MessageBox
	CMsgBoxIGSBuyConfirm* pMsgBox = NULL;
	CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSBuyConfirmLayout), &pMsgBox);
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	pMsgBox->Initialize(pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pItem->m_iPriceSeq, 
							pOwnMsgBox->m_iCashType, pItem->m_szItemName, pItem->m_szItemPrice, pItem->m_szItemPeriod);
#else // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	pMsgBox->Initialize(pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pItem->m_iPriceSeq, 
							pItem->m_iCashType, pItem->m_szItemName, pItem->m_szItemPrice, pItem->m_szItemPeriod);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
#else // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
		pMsgBox->Initialize(pOwnMsgBox->m_wItemCode, pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pItem->m_iPriceSeq, 
								pItem->m_szItemName, pItem->m_szItemPrice, pItem->m_szItemPeriod);
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL

	PlayBuffer(SOUND_CLICK01);
	g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
	
	return CALLBACK_BREAK;
}

//--------------------------------------------
// PresentBtnDown
CALLBACK_RESULT CMsgBoxIGSBuySelectItem::PresentBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	CMsgBoxIGSBuySelectItem* pOwnMsgBox = dynamic_cast<CMsgBoxIGSBuySelectItem*>(pOwner);
	IGS_SelectBuyItem* pItem = pOwnMsgBox->m_SelectBuyListBox.GetSelectedText();
	
	// 선물하기 MessageBox
	CMsgBoxIGSSendGift* pMsgBox = NULL;
	CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSSendGiftLayout), &pMsgBox);
#ifdef KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pItem->m_iPriceSeq, pItem->m_wItemCode, 
						pOwnMsgBox->m_iCashType, pOwnMsgBox->m_szPackageName, pItem->m_szItemPrice, pItem->m_szItemPeriod);
#else // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pItem->m_iPriceSeq, pItem->m_wItemCode, 
						pItem->m_iCashType, pOwnMsgBox->m_szPackageName, pItem->m_szItemPrice, pItem->m_szItemPeriod);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
#else // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pItem->m_iPriceSeq, 
						pItem->m_wItemCode, pOwnMsgBox->m_szPackageName, pItem->m_szItemPrice, pItem->m_szItemPeriod);
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#else // KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pItem->m_iPriceSeq, pOwnMsgBox->m_iCashType,
						pOwnMsgBox->m_szPackageName, pItem->m_szItemPrice, pItem->m_szItemPeriod);
#else // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pItem->m_iPriceSeq, pItem->m_iCashType,
						pOwnMsgBox->m_szPackageName, pItem->m_szItemPrice, pItem->m_szItemPeriod);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
#else // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	pMsgBox->Initialize(pOwnMsgBox->m_iPackageSeq, pOwnMsgBox->m_iDisplaySeq, pItem->m_iPriceSeq,
						pOwnMsgBox->m_szPackageName, pItem->m_szItemPrice, pItem->m_szItemPeriod);
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#endif // KJH_FIX_INGAMESHOP_SENDGIFT_ELIXIROFCONTROL

	PlayBuffer(SOUND_CLICK01);
	g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
	
	return CALLBACK_BREAK;
}

//--------------------------------------------
// CancelBtnDown
CALLBACK_RESULT CMsgBoxIGSBuySelectItem::CancelBtnDown(class CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
	PlayBuffer(SOUND_CLICK01);
	g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

	return CALLBACK_BREAK;
}

//--------------------------------------------
// CancelBtnDown
void CMsgBoxIGSBuySelectItem::SetButtonInfo()
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
// LoadImages
void CMsgBoxIGSBuySelectItem::LoadImages()
{
	LoadBitmap("Interface\\InGameShop\\Ingame_pack_back03.tga", IMAGE_IGS_MGSBOX_BACK, GL_LINEAR);
	LoadBitmap("Interface\\InGameShop\\Ingame_Bt03.tga",		IMAGE_IGS_BUTTON, GL_LINEAR);
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	LoadBitmap("Interface\\InGameShop\\IGS_wcoin_c_p.tga",		IMAGE_IGS_CASHPOINT_TYPE, GL_LINEAR);
	LoadBitmap("Interface\\InGameShop\\IGS_wcoin_check.jpg",	IMAGE_IGS_CASHPOINT_CHECK_BOX, GL_LINEAR);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
}

//--------------------------------------------
// UnloadImages
void CMsgBoxIGSBuySelectItem::UnloadImages()
{
	DeleteBitmap(IMAGE_IGS_MGSBOX_BACK);
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
// CreateListBoxAddData
void CMsgBoxIGSBuySelectItem::CreateListBox()
{
	m_SelectBuyListBox.SetPosition(GetPos().x+IGS_LISTBOX_POS_X, GetPos().y+IGS_LISTBOX_POS_Y);
}

//--------------------------------------------
// RenderListBox
void CMsgBoxIGSBuySelectItem::RenderListBox()
{
	// 리스트 박스 그리기
	if(m_SelectBuyListBox.GetLineNum() != 0)
		m_SelectBuyListBox.Render();
}

//--------------------------------------------
// ReleaseListBox
void CMsgBoxIGSBuySelectItem::ReleaseListBox()
{
	m_SelectBuyListBox.Clear();
}

//--------------------------------------------
// ListBoxDoAction
void CMsgBoxIGSBuySelectItem::ListBoxDoAction()
{
	m_SelectBuyListBox.DoAction();
	
	// Data 선택이 변경되면
	if( m_SelectBuyListBox.IsChangeLine() == TRUE )
	{
		IGS_SelectBuyItem* pItem = m_SelectBuyListBox.GetSelectedText();

#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
		strncpy( m_szPrice, pItem->m_szItemPrice, MAX_TEXT_LENGTH );
		strncpy( m_szPriceUnit, pItem->m_szItemPriceUnit, MAX_TEXT_LENGTH );
#else // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
		strcpy( m_szPrice, pItem->m_szItemPrice);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
		m_wItemCode = pItem->m_wItemCode;
	}
}

//--------------------------------------------
// AddData
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
void CMsgBoxIGSBuySelectItem::AddData(int iPackageSeq, int iDisplaySeq, int iPriceSeq, int iProductSeq, unicode::t_char* pszPriceUnit)
#else // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
void CMsgBoxIGSBuySelectItem::AddData(int iPackageSeq, int iDisplaySeq, int iPriceSeq, int iProductSeq, unicode::t_char* pszPriceUnit, int iCashType)
#else // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	void CMsgBoxIGSBuySelectItem::AddData(int iPackageSeq, int iDisplaySeq, int iPriceSeq, int iProductSeq, unicode::t_char* pszPriceUnit)
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
{
	int iValue;
	unicode::t_char szText[MAX_TEXT_LENGTH] = {'\0', };
	
	IGS_SelectBuyItem Item;
	memset(&Item, 0, sizeof(IGS_SelectBuyItem));

	Item.m_bIsSelected = FALSE;
	Item.m_iPackageSeq = iPackageSeq;
	Item.m_iDisplaySeq = iDisplaySeq;
	Item.m_iPriceSeq = iPriceSeq;
#ifndef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL			// #ifndef
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	Item.m_iCashType = iCashType;
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL

	// 아이템코드
	g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_ITEMCODE, iValue, szText);
	Item.m_wItemCode = iValue;
	
	// 아이템명
	g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_ITEMNAME, iValue, szText);
	strcpy(Item.m_szItemName, szText);
	
	// 가격
	g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_PRICE, iValue, szText);
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	strncpy(Item.m_szItemPrice, szText, MAX_TEXT_LENGTH );
	strncpy(Item.m_szItemPriceUnit, pszPriceUnit, MAX_TEXT_LENGTH );
#else // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	sprintf(Item.m_szItemPrice, "%s %s", szText, pszPriceUnit);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	
	// 기간
	g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_USE_LIMIT_PERIOD, iValue, szText);
	if( iValue > 0 )
	{
		sprintf(Item.m_szItemPeriod, "%d %s", iValue, szText);
	}
	else
	{
		sprintf(Item.m_szItemPeriod, "-");
	}
	
	// 수량
	g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_NUM, iValue, szText);
	if( iValue > 0 )
	{
		// 속성
		// "수량 : %d 개 / 기간 : %s"
		sprintf(Item.m_szAttribute, GlobalText[3045], iValue, Item.m_szItemPeriod);
	}
	else
	{
		// 속성
		sprintf(Item.m_szAttribute, GlobalText[3039], Item.m_szItemPeriod);		// "기간 : %s"
	}
	
	m_SelectBuyListBox.AddText(Item);
}


////////////////////////////////////////////////////////////////////
// LayOut
////////////////////////////////////////////////////////////////////
bool CMsgBoxIGSBuySelectItemLayout::SetLayout()
{
	CMsgBoxIGSBuySelectItem* pMsgBox = GetMsgBox();
	if(false == pMsgBox)
		return false;
	
	if(false == pMsgBox->Create())
		return false;
	
	return true;
}

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
