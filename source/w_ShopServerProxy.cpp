// w_ShopServerProxy.cpp: implementation of the ShopServerProxy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_ShopServerProxy.h"
#ifdef NEW_USER_INTERFACE_PROXY
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzTexture.h"
#include "ZzzAI.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzEffect.h"
#include "ZzzScene.h"
#include "ZzzOpenData.h"
#include "DSPlaySound.h"
#include "WSClient.h"
#include "SideHair.h"
#include "PhysicsManager.h"
#include "GOBoid.h"
#include "CSparts.h"
#include "CSItemOption.h"
#include "CSChaosCastle.h"
#include "GMHellas.h"
#include "wsclientinline.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace
{
	bool IsPartChargeShopOpen()
	{
		if( !Hero->Movement && Hero->SafeZone && (WD_0LORENCIA == World || WD_3NORIA == World 
			|| WD_2DEVIAS == World || WD_51HOME_6TH_CHAR == World
#ifdef LJH_ADD_TO_USE_PARTCHARGESHOP_IN_LOREN_MARKET
			|| WD_79UNITEDMARKETPLACE == World
#endif //LJH_ADD_TO_USE_PARTCHARGESHOP_IN_LOREN_MARKET					
			) ) {
			return true;
		}
		return false;
	}
};

ShopServerProxyPtr ShopServerProxy::Make( BoostWeak_Ptr(ServerProxy) proxy )
{
	ShopServerProxyPtr shopproxy( new ShopServerProxy() );
	shopproxy->Initialize( shopproxy );
	shopproxy->RegisterMainServerProxy( proxy );
	return shopproxy;
}

ShopServerProxy::ShopServerProxy() : m_ShopType(eShop_PartCharge), 
									 m_HighCategoryType(eHighCategory_NewItem), 
									 m_LowCategoryType(eLowCategory_All),
									 m_CategoryPage( 1 ),
									 m_IsShopIn( false ),
									 m_CashPoint( 0 ), m_IsShopInPacket( false ), m_IsDisconnect( false )
{

}

ShopServerProxy::~ShopServerProxy()
{
	Destroy();
}

void ShopServerProxy::Initialize( BoostWeak_Ptr(ShopServerProxy) proxy )
{
	m_handler = proxy;
}

void ShopServerProxy::Destroy()
{

}

void ShopServerProxy::SetShopInit()
{
	m_ShopType = eShop_PartCharge;
	m_HighCategoryType = eHighCategory_NewItem; 
	m_LowCategoryType = eLowCategory_All;
	m_CategoryPage = 1;
	m_IsShopIn = false;
	m_CashPoint = 0; 
	m_IsShopInPacket = false; 
	m_IsDisconnect = false;
}

const bool ShopServerProxy::IsShopMyCartItem( WORD itemcode )
{
	ChashShopMyCartItem_Map::iterator iter = m_ShopMyCartItems.find( itemcode );

	if( iter == m_ShopMyCartItems.end() )
	{
		return false;
	}

	return true;
}

const bool ShopServerProxy::GetShopMyCart( ShopCashShopItem_List& outcategoryitems )
{
	if( m_ShopMyCartItems.size() == 0 )
	{
		return false;
	}
	else
	{
		int count = 0;

		for (ChashShopMyCartItem_Map::iterator iter = m_ShopMyCartItems.begin(); iter != m_ShopMyCartItems.end();)
		{
			ChashShopMyCartItem_Map::iterator curiter = iter;
			++iter;
			CASHSHOP_ITEMLIST& TempItemInfo = (*curiter).second;

			if( TempItemInfo.s_dwItemGuid != 0 )
			{
				if( count >= (m_CategoryPage*MAX_CASHITEM_LIST_PER_PAGE)-MAX_CASHITEM_LIST_PER_PAGE )
				{
					outcategoryitems.push_back( TempItemInfo );
					if( outcategoryitems.size() == MAX_CASHITEM_LIST_PER_PAGE ) break;
				}
				++count;
			}
		}
	}

	return true;
}

const bool ShopServerProxy::InsertShopMyCart( CASHSHOP_ITEMLIST& insertmyitem )
{
	if( !IsShopMyCartItem( insertmyitem.s_dwItemGuid ) )
	{
		m_ShopMyCartItems.insert( make_pair( insertmyitem.s_dwItemGuid, insertmyitem ) );
		return true;
	}

	return false;
}

const bool ShopServerProxy:: DeleteShopMyCart( CASHSHOP_ITEMLIST& deletemyitem )
{
	if( IsShopMyCartItem( deletemyitem.s_dwItemGuid ) )
	{
		m_ShopMyCartItems.erase( deletemyitem.s_dwItemGuid );

		int size = m_ShopMyCartItems.size();

		if( size <= (m_CategoryPage*MAX_CASHITEM_LIST_PER_PAGE)-MAX_CASHITEM_LIST_PER_PAGE )
		{
			m_CategoryPage -= 1;
		}

		return true;
	}

	return false;
}

void ShopServerProxy::ShopReset( bool isMyCartClear )
{
	if( m_ShopItems.size() != 0 ) {
		m_ShopItems.clear();
	}
	
	if( isMyCartClear ) {
		if( m_ShopMyCartItems.size() != 0 ) {
			m_ShopMyCartItems.clear();
		}
	}
	SetShopInit();
}

const bool ShopServerProxy::IsCategoryShopItem()
{
	ShopChashShopItem_Map::iterator iter = m_ShopItems.find( m_LowCategoryType );

	if( iter == m_ShopItems.end() )
	{
		return false;
	}
	else
	{
		ShopChashShopPageItem_Map& shopcategoryitems = (*iter).second;

		ShopChashShopPageItem_Map::iterator pageiter = shopcategoryitems.find( m_CategoryPage );

		if( pageiter == shopcategoryitems.end() )
		{
			return false;
		}
	}

	return true;
}

const bool ShopServerProxy::InsertCategoryShopItem( ShopCashShopItem_List& insertcategoryitems )
{
	ShopChashShopItem_Map::iterator& iter = m_ShopItems.find( m_LowCategoryType );

	if( iter == m_ShopItems.end() )
	{
		ShopChashShopPageItem_Map shoppageitems;
		shoppageitems.insert( make_pair( m_CategoryPage, insertcategoryitems ) );

		m_ShopItems.insert( make_pair( m_LowCategoryType, shoppageitems ) );
	}
	else
	{
		ShopChashShopPageItem_Map& shopcategoryitems = (*iter).second;

		ShopChashShopPageItem_Map::iterator pageiter = shopcategoryitems.find( m_CategoryPage );

		if( pageiter == shopcategoryitems.end() )
		{
			shopcategoryitems.insert( make_pair( m_CategoryPage, insertcategoryitems ) );
			return true;
		}
		else
		{
			return false;
		}
	}

	return true;
}

const bool ShopServerProxy::GetCategoryShopItem( ShopCashShopItem_List& outcategoryitems )
{
	ShopChashShopItem_Map::iterator& iter = m_ShopItems.find( m_LowCategoryType );

	if( iter == m_ShopItems.end() )
	{
		return false;
	}
	else
	{
		ShopChashShopPageItem_Map& shopcategoryitems = (*iter).second;

		ShopChashShopPageItem_Map::iterator pageiter = shopcategoryitems.find( m_CategoryPage );

		if( pageiter == shopcategoryitems.end() )
		{
			return false;
		}
		else
		{
			outcategoryitems = (*pageiter).second;
			return true;
		}
	}

	return false;
}

void ShopServerProxy::SetShopIn( const eShopType shoptype )
{
	if( IsPartChargeShopOpen() == false ) return;

	if( !m_IsShopInPacket && !IsDisconnectServer() )
	{
		SendRequestShopItemlist();
		SendRequestShopCashPoint();
		SendRequestShopInOut( true );
		m_IsShopInPacket = true;
	}
}

void ShopServerProxy::SetShopOut( const eShopType shoptype )
{
	if( !m_IsShopInPacket )
	{
		SendRequestShopInOut( false );
		m_IsShopInPacket = true;
	}
}

void ShopServerProxy::SendRequestShopInOut( bool isOpen )
{
	// isOpen이 false면 닫기, true면 열기
	// already가 false면 한번도 샵에 들어간적이 없고 true면 들어갔었음.
	bool isShopOpenAlready = (m_ShopItems.size() == 0 ? false : true);
	SendRequestCashShopInOut( isOpen, isShopOpenAlready );
}

void ShopServerProxy::RecevieShopInOut( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_CASHSHOPOPEN Data = (LPPMSG_ANS_CASHSHOPOPEN)ReceiveBuffer;

	eCashShopResult result = static_cast<eCashShopResult>(Data->s_btResult);

#ifdef KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI

#ifdef LDK_FIX_OPEN_GOLDBOWMAN_UI_CLICK_SHOP_BTN
	//황금궁수ui 중에 샵버튼 누르면 종료시까지 샵안열림(2008.08.11)
	m_IsShopInPacket = false;
#endif //LDK_FIX_OPEN_GOLDBOWMAN_UI_CLICK_SHOP_BTN

	// 샵 열기/닫기 성공
	if( result == eCashShop_Success )
	{
		if( !m_IsShopIn )
		{
			NotifyToHandler( &ShopServerProxyHandler::OnShopIn, eShop_PartCharge );
			m_IsShopIn = true;
		}
		else
		{
			NotifyToHandler( &ShopServerProxyHandler::OnShopOunt, eShop_PartCharge );
			m_IsShopIn = false;
		}
	}
	// 샵 열기/닫기 실패시 메세지박스 등 메세지가 필요함.
#else // KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
	if( result != eCashShop_Success )
	{
		ServerFailResultMakeMessageBox( result );

#ifdef LDK_FIX_OPEN_GOLDBOWMAN_UI_CLICK_SHOP_BTN
		//황금궁수ui 중에 샵버튼 누르면 종료시까지 샵안열림(2008.08.11)
		m_IsShopInPacket = false;
#endif //LDK_FIX_OPEN_GOLDBOWMAN_UI_CLICK_SHOP_BTN
	}
	else
	{
		if( !m_IsShopIn )
		{
			NotifyToHandler( &ShopServerProxyHandler::OnShopIn, eShop_PartCharge );
			m_IsShopInPacket = false;
			m_IsShopIn = true;
		}
		else
		{
			NotifyToHandler( &ShopServerProxyHandler::OnShopOunt, eShop_PartCharge );
			m_IsShopIn = false;
			m_IsShopInPacket = false;
		}
	}
#endif // KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
}


void ShopServerProxy::SetClientShopOut()
{
	NotifyToHandler( &ShopServerProxyHandler::OnShopOunt, eShop_PartCharge );
	m_IsShopIn = false;
	m_IsShopInPacket = false;
}

void ShopServerProxy::SendRequestShopCashPoint()
{
	SendRequestCashShopCashPoint();
}

void ShopServerProxy::RecevieShopCashPoint( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_CASHPOINT Data = (LPPMSG_ANS_CASHPOINT)ReceiveBuffer;
	m_CashPoint = Data->s_iCashPoint;
	NotifyToHandler( &ShopServerProxyHandler::OnChangeCashPoint, m_CashPoint );
}

void ShopServerProxy::SendRequestShopItemlist()
{
	// 아이템 리스트 카테고리
	// 페이지 인덱스
	// 해당 페이지의 리스트를 받은적이 있는가 (0:있음, 1:없음)
	SendRequestCashShopItemlist( static_cast<BYTE>(m_LowCategoryType-1), static_cast<BYTE>(m_CategoryPage-1), 1 );
}

void ShopServerProxy::RecevieShopItemlist( BYTE* ReceiveBuffer )
{
	ShopCashShopItem_List	receiveItemData;
	
	int dsize = sizeof(ReceiveBuffer); 
	int dsize1 = sizeof(PMSG_ANS_CASHITEMLIST); 

	LPPMSG_ANS_CASHITEMLIST Data = (LPPMSG_ANS_CASHITEMLIST)ReceiveBuffer;
	int Offset                   = sizeof(PBMSG_HEAD2);

	if( ( Data->s_btCategoryIndex + 1 ) != m_LowCategoryType ) return;
	Offset						+= sizeof(Data->s_btCategoryIndex);

	if( ( Data->s_btPageIndex + 1 ) != m_CategoryPage ) return;
	Offset						+= sizeof(Data->s_btPageIndex);

	int itemcount = static_cast<int>(Data->s_btItemCount);
	Offset						+= sizeof(Data->s_btItemCount);

	for( int i = 0;i < itemcount; i++ ) 
	{
		CASHSHOP_ITEMLIST Data2 = *(LPCASHSHOP_ITEMLIST)(ReceiveBuffer+Offset);

		if( Data2.s_dwItemGuid != 0 )
		{
			receiveItemData.push_back( Data2 );
		}

		Offset += sizeof(CASHSHOP_ITEMLIST);
	}
	
	eCashShopResult result = eCashShop_Success;

	if( receiveItemData.size() == 0 )
	{
		if( m_CategoryPage != 1 )
			m_CategoryPage -= 1;
		return;
	}

	if( InsertCategoryShopItem( receiveItemData ) )
	{
		NotifyToHandler( &ShopServerProxyHandler::OnUpDateShopItemInfo, result );
	}
	else
	{
		result = eCashShop_ItemEqual;
		NotifyToHandler( &ShopServerProxyHandler::OnUpDateShopItemInfo, result );
	}
}

void ShopServerProxy::SendRequestShopItemPurchase( int itemcode )
{
	SendRequestCashShopItemPurchase( itemcode, m_LowCategoryType );
}

void ShopServerProxy::RecevieShopItemPurchase( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_CASHITEM_BUY Data = (LPPMSG_ANS_CASHITEM_BUY)ReceiveBuffer;
	eCashShopResult result = static_cast<eCashShopResult>(Data->s_btResult);

	if( result != eCashShop_Success )
	{
		ServerFailResultMakeMessageBox( result );
	}
	else
	{
		SendRequestShopCashPoint();
		NotifyToHandler( &ShopServerProxyHandler::OnCashShopPurchaseResult );
	}
}

void ShopServerProxy::ChangeShopType( const eShopType type )
{
	//여기서 검색 해서 인덴스 데이타를 넣어 주자.
	m_ShopType = type;
	NotifyToHandler( &ShopServerProxyHandler::OnChangeShopType, type );
}

void ShopServerProxy::ChangeHighCategorClk( const eHighCategoryType type )
{
	eCashShopResult result = eCashShop_Success;

	m_HighCategoryType = type;
	m_CategoryPage = 1;
	m_LowCategoryType = eLowCategory_All;

	NotifyToHandler( &ShopServerProxyHandler::OnUpDateShopItemInfo, result );
}

void ShopServerProxy::ChangeLowCategorClk( const eLowCategoryType type )
{
	eCashShopResult result = eCashShop_Success;

	m_LowCategoryType = type;
	m_CategoryPage    = 1;

	if( !IsCategoryShopItem() )
	{
		SendRequestShopItemlist();
	}
	else
	{
		NotifyToHandler( &ShopServerProxyHandler::OnUpDateShopItemInfo, result );
	}
}

void ShopServerProxy::ChangeLowCategorPageClk( WORD pagebutton )
{
	ShopCashShopItem_List outitemdata;
	
	eCashShopResult result = eCashShop_Success;

	DWORD backPage = m_CategoryPage;

	if( pagebutton == 0 )
	{
		if( m_CategoryPage > 1 )
			m_CategoryPage -= 1;
	}
	else
	{
		//맥스 조건문을 넣어 준다
		if( m_HighCategoryType == eHighCategory_NewItem )
			GetCategoryShopItem( outitemdata );
		else
		{
			if( m_ShopMyCartItems.size() <= m_CategoryPage*MAX_CASHITEM_LIST_PER_PAGE )
				return;

			GetShopMyCart( outitemdata );
		}

		int size = outitemdata.size();

		if( size == MAX_CASHITEM_LIST_PER_PAGE ) 
		{
			m_CategoryPage +=1;
		}
		else
		{
			return;
		}
	}

	if( m_HighCategoryType == eHighCategory_NewItem )
	{
		if( backPage == m_CategoryPage ) return;

		if( !IsCategoryShopItem() )
		{
			SendRequestShopItemlist();
		}
		else
		{
			NotifyToHandler( &ShopServerProxyHandler::OnUpDateShopItemInfo, result );
		}
	}
	else
	{
		NotifyToHandler( &ShopServerProxyHandler::OnUpDateShopItemInfo, result );
	}
}

void ShopServerProxy::ChangeCheckBtClk( CASHSHOP_ITEMLIST& item )
{
	eCashShopResult result = eCashShop_Success;

	if( m_HighCategoryType == eHighCategory_NewItem ) // 담기
	{
		if( !InsertShopMyCart( item ) )
		{
			//같은 아이템이 있다.
			result = eCashShop_ItemEqual;
			NotifyToHandler( &ShopServerProxyHandler::OnUpDateShopItemInfo, result );
		}
	}
	else
	{
		if( DeleteShopMyCart( item ) ) // 삭제
		{
			NotifyToHandler( &ShopServerProxyHandler::OnUpDateShopItemInfo, result );
		}
		else
		{
			//아이템이 없다
			result = eCashShop_NotItem;
			NotifyToHandler( &ShopServerProxyHandler::OnUpDateShopItemInfo, result );
		}
	}
}

void ShopServerProxy::ChangeEquipmentItem( const CASHSHOP_ITEMLIST& item )
{
	NotifyToHandler( &ShopServerProxyHandler::OnChangeEquipmentItem, item );
}

void ShopServerProxy::CashShopPurchase( const DWORD itemcode )
{
	SendRequestShopItemPurchase( itemcode );
}

void ShopServerProxy::ServerFailResultMakeMessageBox( eCashShopResult result )
{
	MESSAGEBOXDATA data;

	switch( result )
	{
	case eCashShop_Fail_NotEnoughCash:			data.s_message.push_back( GlobalText[2283] ); break;
	case eCashShop_Fail_NotEmptySpace:			data.s_message.push_back( GlobalText[2284] ); break;
#ifdef ASG_ADD_CASH_SHOP_ALREADY_HAVE_ITEM_MSG
	case eCashShop_Fail_AlreadyHaveItem:	// 중복 아이템입니다.
		data.s_message.push_back(GlobalText[3128]);
		data.s_message.push_back(GlobalText[3129]);
		break;
#endif	// ASG_ADD_CASH_SHOP_ALREADY_HAVE_ITEM_MSG
		//서버 클라 시스템 버그 사항
	case eCashShop_Fail_NotFoundItem:
#ifndef ASG_ADD_CASH_SHOP_ALREADY_HAVE_ITEM_MSG		// 정리시 삭제.
	case eCashShop_Fail_AlreadyHaveItem:
#endif	// ASG_ADD_CASH_SHOP_ALREADY_HAVE_ITEM_MSG
	case eCashShop_Fail_DbError:				
	case eCashShop_Fail_ShopserverDisconnect:	
	case eCashShop_Fail_ShopErrror:				
	case eCashShop_Fail_OpenCashShop:			
	case eCashShop_Fail_ETC:					
	default:
		{
			string Temp = ( format( "%1% %2%" ) % GlobalText[2305] % result ).str();

			data.s_message.push_back( GlobalText[2304] );
			data.s_message.push_back( Temp );
		}
		break;
	}

	data.s_handler = m_handler;
	data.s_id      = 0;
	data.s_type    = eMB_OK;
	MessageBoxMake( data );
}

void ShopServerProxy::SetMessageBox( int id )
{
	MESSAGEBOXDATA data;

	switch( id )
	{
	case MESSAGE_SERVER_LOST:
		{
			data.s_message.push_back( GlobalText[402] );
			data.s_id      = id;
		}
		break;
	}

	data.s_handler = m_handler;
	data.s_type    = eMB_OK;
	MessageBoxMake( data );
}

void ShopServerProxy::OnMessageBox_OK( int id )
{

}

#endif //NEW_USER_INTERFACE_PROXY
