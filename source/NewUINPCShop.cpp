// NewUINPCShop.cpp: implementation of the CNewUINPCShop class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUINPCShop.h"
#include "NewUISystem.h"
#include "NewUICommonMessageBox.h"
#include "PCRoomPoint.h"
#include "ZzzInventory.h"
#include "wsclientinline.h"
#ifdef LDK_ADD_GAMBLE_SYSTEM
#include "GambleSystem.h"
#endif //LDK_ADD_GAMBLE_SYSTEM

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUINPCShop::CNewUINPCShop() 
{ 
	Init(); 
}

SEASON3B::CNewUINPCShop::~CNewUINPCShop() 
{ 
	Release(); 
}

void SEASON3B::CNewUINPCShop::Init()
{
	m_pNewUIMng = NULL;
	m_pNewInventoryCtrl = NULL;
	m_Pos.x = m_Pos.y = 0;

	m_dwShopState = SHOP_STATE_BUYNSELL;
	m_iTaxRate = 0;
	m_bRepairShop = false;

#ifdef KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM
	m_bIsNPCShopOpen = false;
#else // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM			// 디파인 정리할때 지워야 하는 부분
#ifdef KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
	m_bFirstOpen = false;
#endif // KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
#endif // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM		// 디파인 정리할때 지워야 하는 부분

	m_dwStandbyItemKey = 0;

#ifdef CSK_FIX_HIGHVALUE_MESSAGEBOX
	m_bSellingItem = false;
#endif // CSK_FIX_HIGHVALUE_MESSAGEBOX
}

bool SEASON3B::CNewUINPCShop::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if(NULL == pNewUIMng || NULL == g_pNewItemMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_NPCSHOP, this);

	m_pNewInventoryCtrl = new CNewUIInventoryCtrl;
	if(false == m_pNewInventoryCtrl->Create(g_pNewUI3DRenderMng, g_pNewItemMng, this, x+15, y+50, 8, 15))
	{
		SAFE_DELETE(m_pNewInventoryCtrl);
		return false;
	}

	if(m_pNewInventoryCtrl)
	{
		// 상점은 Lock 걸어놓고 사용해야 한다.
		m_pNewInventoryCtrl->LockInventory();
		// 툴팁 타입은 상점으로 해야한다.
		m_pNewInventoryCtrl->SetToolTipType(TOOLTIP_TYPE_NPC_SHOP);
	}

	SetPos(x, y);

	LoadImages();

	SetButtonInfo();

	Show(false);

	return true;
}

void SEASON3B::CNewUINPCShop::Release()
{
	UnloadImages();

	SAFE_DELETE(m_pNewInventoryCtrl);

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

void SEASON3B::CNewUINPCShop::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool SEASON3B::CNewUINPCShop::UpdateMouseEvent()
{
	if(m_pNewInventoryCtrl)
	{	
		if(false == m_pNewInventoryCtrl->UpdateMouseEvent())
		{
			return false;
		}
		
		// 상점 인벤토리 마우스이벤트 처리
		if(InventoryProcess() == true)	// 처리가 완료 되었다면
		{
			return false;
		}

		if(m_pNewInventoryCtrl->CheckPtInRect(MouseX, MouseY) == true)
		{
			ITEM* pItem = m_pNewInventoryCtrl->FindItemAtPt(MouseX, MouseY);

			// 아이템이 선택되어있고, 마우스 왼쪽버튼 Release
#ifdef KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM
			if((m_bIsNPCShopOpen == true) && (pItem) && (SEASON3B::IsRelease(VK_LBUTTON)))
#else // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM
			if(pItem && SEASON3B::IsRelease(VK_LBUTTON))
#endif // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM
			{
				int iIndex = (pItem->y * m_pNewInventoryCtrl->GetNumberOfColumn()) + pItem->x;
#ifdef LDK_ADD_GAMBLE_SYSTEM
				GambleSystem& _gambleSys = GambleSystem::Instance();
#endif //LDK_ADD_GAMBLE_SYSTEM

#ifndef KJH_DEL_PC_ROOM_SYSTEM			// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM				
				// PC방 포인트 상점에서 구입시는 확인창을 뛰움.
				CPCRoomPtSys& rPCRoomPtSys = CPCRoomPtSys::Instance();
				if(rPCRoomPtSys.IsPCRoomPointShopMode())
				{
					// 아이템 위치 저장.
					rPCRoomPtSys.SetBuyItemPos(iIndex);
					g_pNPCShop->SetStandbyItemKey(pItem->Key);
					
					SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CPCRoomPointItemBuyMsgBoxLayout));

					return false;
				}
				else
#endif //ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM
#ifdef LDK_ADD_GAMBLE_SYSTEM
				// 겜블 상점 구입 확인창.	
				if( _gambleSys.IsGambleShop() )
				{
					_gambleSys.SetBuyItemInfo(iIndex, ItemValue(pItem, 0));
					g_pNPCShop->SetStandbyItemKey(pItem->Key);
					
					SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGambleBuyMsgBoxLayout));

					return false;
				}
				else
#endif //LDK_ADD_GAMBLE_SYSTEM
				{
					// 상점 물품 사기 	
					if(BuyCost == 0)	// BuyCost 전역변수
					{
#ifdef KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM
						SendRequestBuy(iIndex, ItemValue(pItem, 0));
#else // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM					// 디파인 정리할때 지워야 하는 부분
#ifdef KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
						// 상점이 처음 열리고 마우스 왼쪽버튼을 Release 하고 나서야 NPC상점을 이용할 수 있다. 
#ifdef KJH_FIX_DOUBLECLICK_BUY_ITEM
						if( m_bFirstOpen == false )
						{
							SendRequestBuy(iIndex, ItemValue(pItem, 0));
						}

						if( SEASON3B::IsRelease(VK_LBUTTON) )
						{
							m_bFirstOpen = false;
						}
#else // KJH_FIX_DOUBLECLICK_BUY_ITEM
						if( m_bFirstOpen == false )
						{
							SendRequestBuy(iIndex, ItemValue(pItem, 0));
						}
						else
						{
							if( SEASON3B::IsRelease(VK_LBUTTON) )
							{
								m_bFirstOpen = false;
								return false;
							}
						}
#endif // KJH_FIX_DOUBLECLICK_BUY_ITEM
#else // KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
						SendRequestBuy(iIndex, ItemValue(pItem, 0));
#endif // KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
#endif // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM				// 디파인 정리할때 지워야 하는 부분
					}	

					return false;
				}
			}
#ifdef KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM
			else if(SEASON3B::IsRelease(VK_LBUTTON))
			{
				m_bIsNPCShopOpen = true;
				return false;
			}
#endif // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM
			else if(SEASON3B::IsPress(VK_LBUTTON))
			{
				return false;
			}
		}
	}

	// 버튼 마우스이벤트 처리
	if(BtnProcess() == true)	// 처리가 완료 되었다면
	{
		return false;
	}

	if(CheckMouseIn(m_Pos.x, m_Pos.y, NPCSHOP_WIDTH, NPCSHOP_HEIGHT))
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

bool SEASON3B::CNewUINPCShop::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) == false)
	{
		return true;
	}

	if(SEASON3B::IsRepeat(VK_SHIFT) && SEASON3B::IsPress('L'))
	{
		SendRequestRepair(255, 0);
		return false;
	}

	else if(SEASON3B::IsPress('L'))	// 수리 단축키.
    {
        //  상점에서 수리 단축키.
		// 수리가능한 상점이면서 픽크된 아이템이 없을 시
		if(m_bRepairShop && CNewUIInventoryCtrl::GetPickedItem() == NULL )	
        {
			ToggleState();
			return false;
        }
    }

	
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true
#ifdef YDG_FIX_NPCSHOP_SELLING_LOCK
			&& m_bSellingItem == false
#endif	// YDG_FIX_NPCSHOP_SELLING_LOCK
			)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPCSHOP);
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}
	

	return true;
}

bool SEASON3B::CNewUINPCShop::Update()
{
	if(m_bRepairShop)
	{
		RepairAllGold();
	}
	if(m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
	{
		return false;
	}
	return true;
}

bool SEASON3B::CNewUINPCShop::Render()
{
	EnableAlphaTest();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// 프레임 그린다.
	RenderFrame();

	// 텍스트를 그린다.
	RenderTexts();

	// 버튼을 그린다.
	RenderButton();

	RenderRepairMoney();

	if(m_pNewInventoryCtrl)
	{
		m_pNewInventoryCtrl->Render();
	}

	DisableAlphaBlend();
	
	return true;
}

void SEASON3B::CNewUINPCShop::RenderFrame()
{
	RenderImage(IMAGE_NPCSHOP_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
	RenderImage(IMAGE_NPCSHOP_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
	RenderImage(IMAGE_NPCSHOP_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_NPCSHOP_RIGHT, m_Pos.x+190-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_NPCSHOP_BOTTOM, m_Pos.x, m_Pos.y+429-45, 190.f, 45.f);
}

void SEASON3B::CNewUINPCShop::RenderTexts()
{
#ifndef KJH_DEL_PC_ROOM_SYSTEM			// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM
	if (CPCRoomPtSys::Instance().IsPCRoomPointShopMode())	// PC포인트 상점인가?
	{
		// 주의 사항 텍스트 렌더.
		g_pRenderText->SetFont(g_hFontBold);
		g_pRenderText->SetBgColor(0);
		g_pRenderText->SetTextColor(255, 128, 255, 255);

		// 2325 "PC방 포인트 상점"
		g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 12, GlobalText[2325], NPCSHOP_WIDTH, 0, RT3_SORT_CENTER);

		const short c_nLineMax = 2;
		const short c_nLowMax = 36;
		unicode::t_char aszNotice[c_nLineMax][c_nLowMax];

		const short c_nNoticeMax = 3;	// 문장 개수
		// 2327 "PC방 포인트 상점에서는 물건의 구입만 가능합니다."
		// 2328 "인장은 구입 후 바로 효과가 적용됩니다."
		// 2332 "캐릭터 종료 및 로랜협곡 이동시 효과가 사라집니다."
		const unicode::t_char* c_apszGlobalText[c_nNoticeMax]
			= { GlobalText[2327], GlobalText[2328], GlobalText[2332] };

		int i, j;
		for (i = 0; i < c_nNoticeMax; ++i)
		{
			::memset(aszNotice[0], 0, sizeof(unicode::t_char) * c_nLineMax * c_nLowMax);
			::SeparateTextIntoLines(
				c_apszGlobalText[i], aszNotice[0], c_nLineMax, c_nLowMax);
			for (j = 0; j < c_nLineMax; ++j)
			{
#ifdef LDK_FIX_PCROOM_INFOMATION
				if(0 == unicode::_strlen(aszNotice[j]) ) continue;
#endif //LDK_FIX_PCROOM_INFOMATION

				g_pRenderText->RenderText(m_Pos.x,
					m_Pos.y+356+(i*22)+(j*11),	// 문장 간격 26, 줄 간격 12.
					aszNotice[j], 190, 0, RT3_SORT_CENTER);
			}
		}

		g_pRenderText->SetFont(g_hFont);

		return;	// 사기, 팔기, 수리 등은 렌더할 필요 없음.
	}
#endif //ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(220, 220, 220, 255);

	// 230 "상  인"	
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 12, GlobalText[230], NPCSHOP_WIDTH, 0, RT3_SORT_CENTER);
	
	// 1623 "세율: %d%% (실시간변동)"
	unicode::t_char strText[256];
	unicode::_sprintf(strText, GlobalText[1623], m_iTaxRate);
	g_pRenderText->RenderText(m_Pos.x,m_Pos.y + 27, strText, NPCSHOP_WIDTH, 0, RT3_SORT_CENTER);
}

void SEASON3B::CNewUINPCShop::RenderButton()
{
	if(m_bRepairShop)
	{
		m_BtnRepair.Render();
		m_BtnRepairAll.Render();
	}
}

void SEASON3B::CNewUINPCShop::RenderRepairMoney()
{
	if(m_bRepairShop)
	{
		RenderImage(IMAGE_NPCSHOP_REPAIR_MONEY, m_Pos.x+10, m_Pos.y+355, 170.f, 24.f);

		g_pRenderText->SetBgColor(255, 255, 255, 0);
		g_pRenderText->SetTextColor(255, 220, 150, 255);
		
		unicode::t_char strText[256];
        ConvertGold(AllRepairGold, strText);

		g_pRenderText->SetFont(g_hFontBold);
		// 239 "전체수리"
		g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+362, GlobalText[239]);
        g_pRenderText->SetTextColor(getGoldColor(AllRepairGold));
		g_pRenderText->RenderText(m_Pos.x+100, m_Pos.y+362, strText);
	}
}

float SEASON3B::CNewUINPCShop::GetLayerDepth()
{ 
#ifdef KJH_FIX_WOPS_K27500_POTION_NUM_RENDER
	// MyInventory보다 나중에 Render되어야 한다.
	return 4.55;	
#else // KJH_FIX_WOPS_K27500_POTION_NUM_RENDER
	return 2.5f; 
#endif // KJH_FIX_WOPS_K27500_POTION_NUM_RENDER
}

void SEASON3B::CNewUINPCShop::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_NPCSHOP_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back04.tga", IMAGE_NPCSHOP_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_NPCSHOP_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_NPCSHOP_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_NPCSHOP_BOTTOM, GL_LINEAR);
	LoadBitmap("Interface\\newui_repair_00.tga", IMAGE_NPCSHOP_BTN_REPAIR, GL_LINEAR);

	LoadBitmap("Interface\\newui_item_money2.tga", IMAGE_NPCSHOP_REPAIR_MONEY, GL_LINEAR);
}

void SEASON3B::CNewUINPCShop::UnloadImages()
{
	DeleteBitmap(IMAGE_NPCSHOP_BACK);
	DeleteBitmap(IMAGE_NPCSHOP_TOP);
	DeleteBitmap(IMAGE_NPCSHOP_LEFT);
	DeleteBitmap(IMAGE_NPCSHOP_LEFT);
	DeleteBitmap(IMAGE_NPCSHOP_BOTTOM);
	DeleteBitmap(IMAGE_NPCSHOP_BTN_REPAIR);

	DeleteBitmap(IMAGE_NPCSHOP_REPAIR_MONEY);
}

void SEASON3B::CNewUINPCShop::SetTaxRate(int iTaxRate)
{
	m_iTaxRate = iTaxRate;
}

int SEASON3B::CNewUINPCShop::GetTaxRate()
{	
	return m_iTaxRate;
}

bool SEASON3B::CNewUINPCShop::InsertItem(int iIndex, BYTE* pbyItemPacket)
{
	if(m_pNewInventoryCtrl)
	{
		return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
	}
	
	return false;
}

bool SEASON3B::CNewUINPCShop::InventoryProcess()
{
	CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

	if( !m_pNewInventoryCtrl )	return false;
	if( !pPickedItem )			return false;
	ITEM* pItem = pPickedItem->GetItem();

#ifndef KJH_FIX_SELL_LUCKYITEM				// #ifndef
#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 상인 판매할 때에 인벤 Warning칼라 [lem_2010.9.8]
	if( IsSellingBan( pItem ) )	m_pNewInventoryCtrl->SetSquareColorNormal(1.0f, 0.0f, 0.0f );
	else	m_pNewInventoryCtrl->SetSquareColorNormal(0.1f, 0.4f, 0.8f );
#endif // LEM_ADD_LUCKYITEM
#endif // KJH_FIX_SELL_LUCKYITEM
	if(	SEASON3B::IsRelease(VK_LBUTTON) == true
		&& m_pNewInventoryCtrl->CheckPtInRect(MouseX, MouseY) == true
#ifdef CSK_FIX_HIGHVALUE_MESSAGEBOX
		&& m_bSellingItem == false	// 아이템 팔고 있는 중이 아니면
#endif // CSK_FIX_HIGHVALUE_MESSAGEBOX
		)
	{
#ifndef KJH_DEL_PC_ROOM_SYSTEM			// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM
		// PC방 포인트 상점에 팔 수 없다. 
		if (CPCRoomPtSys::Instance().IsPCRoomPointShopMode())
		{
			// 2329 "아이템을 판매 할 수 없습니다."
			g_pChatListBox->AddText("", GlobalText[2329], SEASON3B::TYPE_ERROR_MESSAGE);
			
			return true;
		}
#endif //ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM

#ifdef LJH_FIX_BUG_SELLING_ITEM_CAUSING_OVER_LIMIT_OF_ZEN
		// 현재 소지하고 있는 금액과 아이템의 판매금액의 합이 20억(제한)을 넘을 경우 판매 금지
		if (CharacterMachine->Gold + ItemValue(pItem) > 2000000000)
		{
			g_pChatListBox->AddText("", GlobalText[3148], SEASON3B::TYPE_SYSTEM_MESSAGE);
			
			return true;
		}
#endif //LJH_FIX_BUG_SELLING_ITEM_CAUSING_OVER_LIMIT_OF_ZEN

		if( pItem && pItem->Jewel_Of_Harmony_Option != 0 )
		{
			// 2211 "강화된 아이템은 판매할 수 없습니다."
			g_pChatListBox->AddText("", GlobalText[2211], SEASON3B::TYPE_ERROR_MESSAGE);

			return true;
		}
#ifndef KJH_FIX_SELL_LUCKYITEM				// #ifndef
#ifdef LEM_ADD_LUCKYITEM		// 럭키아이템 판매 금지
		else if( Check_ItemAction(pItem, eITEM_SELL) && pItem->Durability > 0)
		{
			// 668 "거래가 불가능한 아이템입니다."
			g_pChatListBox->AddText("", GlobalText[2329], SEASON3B::TYPE_ERROR_MESSAGE);
			m_pNewInventoryCtrl->BackupPickedItem();

			return true;
		}
#endif // LEM_ADD_LUCKYITEM
#endif // KJH_FIX_SELL_LUCKYITEM

#ifdef LJH_FIX_NOT_POP_UP_HIGHVALUE_MSGBOX_FOR_BANNED_TO_TRADE
		else if(pItem && IsSellingBan(pItem) == true)	// 판매금지 아이템
		{
			// 668 "거래가 불가능한 아이템입니다."
			g_pChatListBox->AddText("", GlobalText[668], SEASON3B::TYPE_ERROR_MESSAGE);
			m_pNewInventoryCtrl->BackupPickedItem();
			
			return true;
		}
		else if(pItem && IsHighValueItem(pItem) == true)
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CHighValueItemCheckMsgBoxLayout));
			pPickedItem->HidePickedItem();
			
			return true;
		}
#else  //LJH_FIX_NOT_POP_UP_HIGHVALUE_MSGBOX_FOR_BANNED_TO_TRADE
		else if(pItem && IsHighValueItem(pItem) == true)
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CHighValueItemCheckMsgBoxLayout));
			pPickedItem->HidePickedItem();

			return true;
		}
		else if(pItem && IsSellingBan(pItem) == true)	// 판매금지 아이템
		{
			// 668 "거래가 불가능한 아이템입니다."
			g_pChatListBox->AddText("", GlobalText[668], SEASON3B::TYPE_ERROR_MESSAGE);
			m_pNewInventoryCtrl->BackupPickedItem();

			return true;
		}
#endif //LJH_FIX_NOT_POP_UP_HIGHVALUE_MSGBOX_FOR_BANNED_TO_TRADE

		// 인벤토리에서 상점으로
		if (pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
		{
			int iSourceIndex = pPickedItem->GetSourceLinealPos() + MAX_EQUIPMENT_INDEX;
			SendRequestSell(iSourceIndex);

			return true;
		}
		// 장비창에서 상점으로
		else if(pPickedItem->GetOwnerInventory() == NULL)
		{
			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			SendRequestSell(iSourceIndex);
			
			return true;
		}
	}

	return false;
}

bool SEASON3B::CNewUINPCShop::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };

	//. Exit1 버튼 (기본처리)
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12)
#ifdef YDG_FIX_NPCSHOP_SELLING_LOCK
		&& m_bSellingItem == false
#endif	// YDG_FIX_NPCSHOP_SELLING_LOCK
		)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPCSHOP);
		
		return true;
	}

	if(m_bRepairShop)
	{
		if(m_BtnRepair.UpdateMouseEvent() == true)
		{
			ToggleState();
			
			return true;
		}
		if(m_BtnRepairAll.UpdateMouseEvent() == true)
		{
			SendRequestRepair(255, 0);
			
			return true;
		}
	}

	return false;
}

#ifdef KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM_EX
// 상점을 열 때 프로세스
void SEASON3B::CNewUINPCShop::OpenningProcess()
{
	if( SEASON3B::IsRepeat(VK_LBUTTON))
	{
		m_bIsNPCShopOpen = false;
	}
	else
	{
		m_bIsNPCShopOpen = true;		// 초기화(상점이 열리고 나서 true)
	}
}
#else KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM_EX
#ifdef KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM
// 상점을 열 때 프로세스
void SEASON3B::CNewUINPCShop::OpenningProcess()
{
	m_bIsNPCShopOpen = false;		// 초기화(상점이 열리고 나서 true)
}
#else // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM			// 디파인 정리할때 지워야 하는 부분
#ifdef KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
// 상점을 열 때 프로세스
void SEASON3B::CNewUINPCShop::OpenningProcess()
{
	m_bFirstOpen = true;
}
#endif // KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
#endif // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM		// 디파인 정리할때 지워야 하는 부분
#endif // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM_EX

void SEASON3B::CNewUINPCShop::ClosingProcess()
{
	SendExitInventory();

	// 각 정보 초기화
	m_dwShopState = SHOP_STATE_BUYNSELL;
	m_iTaxRate = 0;
	m_bRepairShop = false;
	m_dwStandbyItemKey = 0;

#ifdef KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM
	m_bIsNPCShopOpen = false;
#else // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM		// 디파인 정리할때 지워야 하는 부분
#ifdef KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
	m_bFirstOpen = false;
#endif // KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
#endif // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM		// 디파인 정리할때 지워야 하는 부분

	if(m_pNewInventoryCtrl)
	{
		m_pNewInventoryCtrl->RemoveAllItems();
	}
#ifndef KJH_DEL_PC_ROOM_SYSTEM		// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM
	// PC방 포인트 상점 모드 초기화.
	CPCRoomPtSys::Instance().SetPCRoomPointShopMode(false);
#endif //ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM

#ifdef LDK_ADD_GAMBLE_SYSTEM
	// 겜블 상점 모드 초기화
	GambleSystem::Instance().SetGambleShop(false);
#endif //LDK_ADD_GAMBLE_SYSTEM

#ifdef CSK_FIX_HIGHVALUE_MESSAGEBOX
	m_bSellingItem = false;
#endif // CSK_FIX_HIGHVALUE_MESSAGEBOX
}

void SEASON3B::CNewUINPCShop::SetButtonInfo()
{
	m_BtnRepair.ChangeButtonImgState(true, IMAGE_NPCSHOP_BTN_REPAIR, false);
	m_BtnRepair.ChangeButtonInfo(m_Pos.x + 54, m_Pos.y + 390, 36, 29);
	// 233 "수리(R)"
	m_BtnRepair.ChangeToolTipText(GlobalText[233], true);

	m_BtnRepairAll.ChangeButtonImgState(true, IMAGE_NPCSHOP_BTN_REPAIR, false);
	m_BtnRepairAll.ChangeButtonInfo(m_Pos.x + 98, m_Pos.y + 390, 36, 29);
	// 237 "전체수리(A)"
	m_BtnRepairAll.ChangeToolTipText(GlobalText[237], true);
}

void SEASON3B::CNewUINPCShop::SetRepairShop(bool bRepair)
{
	m_bRepairShop = bRepair;
}

bool SEASON3B::CNewUINPCShop::IsRepairShop()
{
	return m_bRepairShop;
}

void SEASON3B::CNewUINPCShop::ToggleState()
{
	if(m_dwShopState == SHOP_STATE_BUYNSELL)
	{
		m_dwShopState = SHOP_STATE_REPAIR;
		
		g_pMyInventory->SetRepairMode(true);
	}
	else
	{
		m_dwShopState = SHOP_STATE_BUYNSELL;
		g_pMyInventory->SetRepairMode(false);
	}
}

DWORD SEASON3B::CNewUINPCShop::GetShopState()
{	
	return m_dwShopState;
}


int SEASON3B::CNewUINPCShop::GetPointedItemIndex()
{
	return m_pNewInventoryCtrl->GetPointedSquareIndex();
}

void SEASON3B::CNewUINPCShop::SetStandbyItemKey(DWORD dwItemKey)
{ 
	m_dwStandbyItemKey = dwItemKey; 
}

DWORD SEASON3B::CNewUINPCShop::GetStandbyItemKey() const
{ 
	return m_dwStandbyItemKey; 
}

int SEASON3B::CNewUINPCShop::GetStandbyItemIndex()
{
	ITEM* pItem = GetStandbyItem();
	if(pItem)
		return pItem->y*m_pNewInventoryCtrl->GetNumberOfColumn()+pItem->x;
	return -1;
}

ITEM* SEASON3B::CNewUINPCShop::GetStandbyItem()
{
	if(m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->FindItemByKey(m_dwStandbyItemKey);
	return NULL;
}

#ifdef CSK_FIX_HIGHVALUE_MESSAGEBOX
void SEASON3B::CNewUINPCShop::SetSellingItem(bool bFlag)
{
	m_bSellingItem = bFlag;
}

bool SEASON3B::CNewUINPCShop::IsSellingItem()
{
	return m_bSellingItem;
}
#endif // CSK_FIX_HIGHVALUE_MESSAGEBOX
