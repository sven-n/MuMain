// NewUIPurchaseShopInventory.cpp: implementation of the CNewUIPurchaseShopInventory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include "NewUIPurchaseShopInventory.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "wsclientinline.h"
#include "PersonalShopTitleImp.h"



namespace
{
	void RenderText( const char* text, int x, int y, int sx, int sy, DWORD color, DWORD backcolor, int sort, HFONT hFont = g_hFont )
	{
		g_pRenderText->SetFont(hFont);

		DWORD backuptextcolor = g_pRenderText->GetTextColor();
		DWORD backuptextbackcolor = g_pRenderText->GetBgColor();

		g_pRenderText->SetTextColor(color);
		g_pRenderText->SetBgColor(backcolor);
		g_pRenderText->RenderText(x, y, text, sx, sy, sort);

		g_pRenderText->SetTextColor(backuptextcolor);
		g_pRenderText->SetBgColor(backuptextbackcolor);
	}
};

using namespace SEASON3B;

SEASON3B::CNewUIPurchaseShopInventory::CNewUIPurchaseShopInventory() : m_pNewUIMng( NULL ), m_pNewInventoryCtrl( NULL )
{
	m_Pos.x = m_Pos.y = 0;
	m_ShopCharacterIndex = -1;
}

SEASON3B::CNewUIPurchaseShopInventory::~CNewUIPurchaseShopInventory()
{
	Release();
}

bool SEASON3B::CNewUIPurchaseShopInventory::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if(NULL == pNewUIMng || NULL == g_pNewUI3DRenderMng || NULL == g_pNewItemMng)
		return false;

	LoadImages();

	SetPos(x, y);

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY, this);

	m_pNewInventoryCtrl = new CNewUIInventoryCtrl;
	if(false == m_pNewInventoryCtrl->Create(g_pNewUI3DRenderMng, g_pNewItemMng, this, m_Pos.x+16, m_Pos.y+90, 8, 4))
	{
		SAFE_DELETE(m_pNewInventoryCtrl);
		return false;
	}

	m_pNewInventoryCtrl->SetToolTipType( TOOLTIP_TYPE_PURCHASE_SHOP );
	m_pNewInventoryCtrl->LockInventory();

	m_Button = new CNewUIButton;
	m_Button->ChangeButtonImgState( true, IMAGE_INVENTORY_EXIT_BTN, false );
	m_Button->ChangeButtonInfo( m_Pos.x+13, m_Pos.y+391, 36, 29 );

	Show(false);

	return true;
}

void SEASON3B::CNewUIPurchaseShopInventory::Release()
{
	SAFE_DELETE(m_Button);

	SAFE_DELETE(m_pNewInventoryCtrl);

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}

	UnloadImages();
}

bool SEASON3B::CNewUIPurchaseShopInventory::InsertItem(int iIndex, BYTE* pbyItemPacket)
{
	if(m_pNewInventoryCtrl)
	{
		return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
	}

	return false;
}

void SEASON3B::CNewUIPurchaseShopInventory::DeleteItem(int iIndex)
{
	if(m_pNewInventoryCtrl)
	{
		ITEM* pItem = m_pNewInventoryCtrl->FindItem(iIndex);

		if(pItem != NULL)
		{
			m_pNewInventoryCtrl->RemoveItem(pItem);
		}
	}
}

ITEM* SEASON3B::CNewUIPurchaseShopInventory::FindItem(int iLinealPos)
{
	if(m_pNewInventoryCtrl)
	{
		return m_pNewInventoryCtrl->FindItem(iLinealPos);
	}

	return NULL;
}

void SEASON3B::CNewUIPurchaseShopInventory::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_MSGBOX_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_INVENTORY_BACK_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_INVENTORY_BACK_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_INVENTORY_BACK_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_INVENTORY_BACK_BOTTOM, GL_LINEAR);
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_INVENTORY_EXIT_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_Box_openTitle.tga", IMAGE_MYSHOPINVENTORY_EDIT, GL_LINEAR);
}

void SEASON3B::CNewUIPurchaseShopInventory::UnloadImages()
{
	DeleteBitmap(IMAGE_MYSHOPINVENTORY_EDIT);	
	DeleteBitmap(IMAGE_INVENTORY_EXIT_BTN);
	DeleteBitmap(IMAGE_INVENTORY_BACK_BOTTOM);
	DeleteBitmap(IMAGE_INVENTORY_BACK_RIGHT);
	DeleteBitmap(IMAGE_INVENTORY_BACK_LEFT);
	DeleteBitmap(IMAGE_INVENTORY_BACK_TOP);
	DeleteBitmap(IMAGE_MSGBOX_BACK);
}

bool SEASON3B::CNewUIPurchaseShopInventory::UpdateMouseEvent()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };
	//. Exit1 버튼 (기본처리)
	if(SEASON3B::IsRelease(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12)) 
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY);
		return false;
	}
	if(m_Button->UpdateMouseEvent())
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY);
		return false;
	}

	if(m_pNewInventoryCtrl)
	{
		if(false == m_pNewInventoryCtrl->UpdateMouseEvent())
		{
			return false;
		}

		if( PurchaseShopInventoryProcess() )
		{
			return false;
		}
	}

	//. 인벤토리 내의 영역 클릭시 하위 UI처리 및 이동 불가
	if(CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT))
	{
		if(SEASON3B::IsPress(VK_RBUTTON))
		{
			MouseRButton = false;
			MouseRButtonPop = false;
			MouseRButtonPush = false;
			return false;
		}

		if(SEASON3B::IsNone(VK_LBUTTON) == false)
		{
			return false;
		}
	}

	return true;
}

bool SEASON3B::CNewUIPurchaseShopInventory::UpdateKeyEvent()
{
	return true;
}

bool SEASON3B::CNewUIPurchaseShopInventory::PurchaseShopInventoryProcess()
{
	if(m_pNewInventoryCtrl && IsPress(VK_LBUTTON))
	{
		int iCurSquareIndex = m_pNewInventoryCtrl->FindItemptIndex(MouseX, MouseY);
		if( iCurSquareIndex != -1 )
		{
			ChangeSourceIndex(iCurSquareIndex);
			CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CPersonalShopItemBuyMsgBoxLayout));
		}

		return true;
	}

	return false;
}

bool SEASON3B::CNewUIPurchaseShopInventory::Update()
{
	if(m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
	{
		return false;
	}
	return true;
}

void SEASON3B::CNewUIPurchaseShopInventory::RenderFrame()
{
	RenderImage(IMAGE_MSGBOX_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
	RenderImage(IMAGE_INVENTORY_BACK_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
	RenderImage(IMAGE_INVENTORY_BACK_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_INVENTORY_BACK_RIGHT, m_Pos.x+190-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_INVENTORY_BACK_BOTTOM, m_Pos.x, m_Pos.y+429-45, 190.f, 45.f);
	RenderImage(IMAGE_MYSHOPINVENTORY_EDIT, m_Pos.x+12, m_Pos.y+49, 169.f, 26.f);
}

void SEASON3B::CNewUIPurchaseShopInventory::RenderTextInfo()
{
	// 1102 "개인상점"
	RenderText(GlobalText[1102], m_Pos.x, m_Pos.y+15, 190, 0, 0xFF49B0FF, 0x00000000, RT3_SORT_CENTER );
	// 상점이름
	RenderText( m_TitleText.c_str(), m_Pos.x, m_Pos.y+58, 190, 0, RGBA(0, 255, 0, 255), 0x00000000, RT3_SORT_CENTER, g_hFontBold );

	unicode::t_char Text[100];

	memset(&Text, 0, sizeof(unicode::t_char)*100);
	// 370 "주의!"
	sprintf(Text, GlobalText[370]); 
	RenderText( Text, m_Pos.x+30, m_Pos.y+230, 0, 0, RGBA(255, 45, 47, 255), 0x00000000, RT3_SORT_LEFT, g_hFontBold );

	memset(&Text, 0, sizeof(unicode::t_char)*100);
	// 1109 "상점 등록시 판매 가격을"
	sprintf(Text, GlobalText[1109]); 
	RenderText( Text, m_Pos.x+30, m_Pos.y+250, 0, 0, RGBA(247, 206, 77, 255), 0x00000000, RT3_SORT_LEFT );

	memset(&Text, 0, sizeof(unicode::t_char)*100);
	// 1111 "꼭! 확인하시기 바랍니다."
	sprintf(Text, GlobalText[1111]); 
	RenderText( Text, m_Pos.x+30, m_Pos.y+262, 0, 0, RGBA(247, 206, 77, 255), 0x00000000, RT3_SORT_LEFT );

	memset(&Text, 0, sizeof(unicode::t_char)*100);
	// 1112 "개인상점에서 이미"
	sprintf(Text, GlobalText[1112]); 
	RenderText( Text, m_Pos.x+30, m_Pos.y+274, 0, 0, RGBA(247, 206, 77, 255), 0x00000000, RT3_SORT_LEFT );

	memset(&Text, 0, sizeof(unicode::t_char)*100);
	// 1114 "구매한 아이템은 취소하여"
	sprintf(Text, GlobalText[1114]); 
	RenderText( Text, m_Pos.x+30, m_Pos.y+286, 0, 0, RGBA(247, 206, 77, 255), 0x00000000, RT3_SORT_LEFT );

	memset(&Text, 0, sizeof(unicode::t_char)*100);
	// 1115 "돌려 받을 수 없습니다."
	sprintf(Text, GlobalText[1115]); 
	RenderText( Text, m_Pos.x+30, m_Pos.y+298, 0, 0, RGBA(247, 206, 77, 255), 0x00000000, RT3_SORT_LEFT );

	memset(&Text, 0, sizeof(unicode::t_char)*100);
	// 1134 "아이템 거래는 모두"
	sprintf(Text, GlobalText[1134]); 
	RenderText( Text, m_Pos.x+30, m_Pos.y+320, 0, 0, RGBA(255, 45, 47, 255), 0x00000000, RT3_SORT_LEFT, g_hFontBold );

	memset(&Text, 0, sizeof(unicode::t_char)*100);
	// 1135 "젠으로만 거래 가능합니다."
	sprintf(Text, GlobalText[1135]); 
	RenderText( Text, m_Pos.x+30, m_Pos.y+332, 0, 0, RGBA(255, 45, 47, 255), 0x00000000, RT3_SORT_LEFT, g_hFontBold );
}

bool SEASON3B::CNewUIPurchaseShopInventory::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);

	RenderFrame();

	RenderTextInfo();

	if(m_pNewInventoryCtrl)
	{
		m_pNewInventoryCtrl->Render();
	}

	m_Button->Render();

	DisableAlphaBlend();

	return true;
}

void SEASON3B::CNewUIPurchaseShopInventory::ClosingProcess()
{
#ifdef LDS_FIX_MEMORYLEAK_WHERE_NEWUI_DEINITIALIZE
 	if( m_pNewInventoryCtrl )
 	{
 		m_pNewInventoryCtrl->RemoveAllItems();
#ifdef LDS_ADD_OUTPUTERRORLOG_WHEN_RECEIVEREFRESHPERSONALSHOPITEM
		g_ErrorReport.Write("@ [Notice] CNewUIPurchaseShopInventory::ClosingProcess():m_pNewInventoryCtrl->RemoveAllItems(); )\n" );
#endif // LDS_ADD_OUTPUTERRORLOG_WHEN_RECEIVEREFRESHPERSONALSHOPITEM
 	}
#endif // LDS_FIX_MEMORYLEAK_WHERE_NEWUI_DEINITIALIZE

	m_ShopCharacterIndex = -1;

	g_pMyInventory->ChangeMyShopButtonStateOpen();
}

int SEASON3B::CNewUIPurchaseShopInventory::GetPointedItemIndex()
{
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
	int iPointedItemIndex = m_pNewInventoryCtrl->GetPointedSquareIndex();
	
	if(iPointedItemIndex != -1)
	{
		iPointedItemIndex += MAX_MY_INVENTORY_INDEX;
	}
	
	return iPointedItemIndex;
#else // KJH_FIX_DARKLOAD_PET_SYSTEM
	return m_pNewInventoryCtrl->GetPointedSquareIndex();
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM
}

