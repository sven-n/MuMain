// w_ShopMiddleModule.cpp: implementation of the ShopMiddleModule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_Module.h"
#include "w_ShopMiddleModule.h"


#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"

#include "NewUIInventoryCtrl.h"
#include "NewUIMyInventory.h"
#include "NewUISystem.h"

#include "DSPlaySound.h"

MAKE_OBJECT_POOL( ShopMiddleModule );
MAKE_OBJECT_POOL( ShopMiddleModule::ShopItemList );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ShopMiddleModule::ShopItemListPtr ShopMiddleModule::ShopItemList::Make( ShopItemListData& data )
{
	ShopItemListPtr itemlist( new ShopItemList() );
	itemlist->Initialize( data );
	return itemlist;
}

ShopMiddleModule::ShopItemList::ShopItemList()
{

}

ShopMiddleModule::ShopItemList::~ShopItemList()
{
	Destroy();
}

void ShopMiddleModule::ShopItemList::Initialize( ShopMiddleModule::ShopItemListData& data )
{
	int x = 0, y = 0, count = 0, i = 0;

	Dim size = Dim( 151, 154 );

	for( ShopCashShopItem_List::iterator iter = data.s_ItemDatas.begin(); iter != data.s_ItemDatas.end(); )
	{
		ShopCashShopItem_List::iterator curiter = iter;
		++iter;
		CASHSHOP_ITEMLIST& itemdata = *curiter;

		ITEM ip;
		ChangeCodeItem( &ip, (BYTE*)itemdata.s_btItemInfo );
		ITEM_ATTRIBUTE* ipattribute = &ItemAttribute[ip.Type];
		ip.Durability = itemdata.s_btItemDuration;

		int Level = (ip.Level>>3)&15;
		int minitem = (1*MAX_CASHITEM_LIST_PER_PAGE)-MAX_CASHITEM_LIST_PER_PAGE;

		if( i >= minitem )
		{
			m_ShopPageItems.insert( make_pair( itemdata.s_dwItemGuid, itemdata ) );

			y = ( (i-minitem) / 3 ) * 159;

			ui::PanelPtr girthshopmiddlebackpanel( new ui::Panel( ( format( "girthshopmiddlebackpanel%1%" ) % i ).str() ) );
			girthshopmiddlebackpanel->SetColor( 0xffffffff );
			girthshopmiddlebackpanel->ChangeOpacityByPercent( 0.0f );
			girthshopmiddlebackpanel->Move( Coord( x + ( count * ( size.x + data.s_IntervalSize.x ) ), y ) );
			girthshopmiddlebackpanel->Resize( size );
			data.s_BackPanel.lock()->AddChild( girthshopmiddlebackpanel );

			ui::CompositedPanelPtr girthshopmiddlepanel( new ui::CompositedPanel( "girthshopmiddlepanel", ui::ePanel_Piece_One ) );
			girthshopmiddlepanel->SetImageIndex( BITMAP_NEWUI_SHOP_LEFTDISPLAYBT+6 );
			girthshopmiddlepanel->SetID( ip.Type );
			girthshopmiddlepanel->Move( Coord( 0, 0 ) );
			girthshopmiddlepanel->Resize( Dim( 151, 124 ) );
			girthshopmiddlebackpanel->AddChild( girthshopmiddlepanel );

			string		itemName;
#ifdef LDK_FIX_CS5_HIDE_SEEDLEVEL
			if(ip.Type >= ITEM_WING+60 && ip.Type <= ITEM_WING+65)
			{
				itemName = ( format( "%1%" ) % ipattribute->Name ).str();
			}
			else
#endif //LDK_FIX_CS5_HIDE_SEEDLEVEL
			{
				if( Level == 0 )
					itemName = ( format( "%1%" ) % ipattribute->Name ).str();
				else
					itemName = ( format( "%1% +%2%" ) % ipattribute->Name % Level ).str();
			}

			ui::TextPtr girthshopmiddleitemname( new ui::Text( ( format( "girthshopmiddleitemname%1%" ) % i ).str() ) );
			girthshopmiddleitemname->Make( FONTSIZE_LANGUAGE, itemName.length() );
			girthshopmiddleitemname->SetAlign( TA_LEFT );
			girthshopmiddleitemname->SetContent( itemName );
			girthshopmiddleitemname->SetColor( 0xffC5A6FF );
			Coord textpos = Coord( (girthshopmiddlepanel->GetSize().x/2)-(girthshopmiddleitemname->GetSize().x/2), 8);
			girthshopmiddleitemname->Move( textpos );
			girthshopmiddlepanel->AddChild( girthshopmiddleitemname );

			itemName = ( format( "%1%%2%" ) % GlobalText[2291] % (itemdata.s_wItemPrice) ).str();
			ui::TextPtr cashtext( new ui::Text( ( format( "cashtext%1%" ) % i ).str() ) );
			cashtext->Make( FONTSIZE_LANGUAGE, itemName.length() );
			cashtext->SetAlign( TA_LEFT );
			cashtext->SetContent( itemName );
			cashtext->SetColor( 0xffFFEE9E );
			textpos = Coord( (girthshopmiddlepanel->GetSize().x/2)-(cashtext->GetSize().x/2), 106);
			cashtext->Move( textpos );
			girthshopmiddlepanel->AddChild( cashtext );

			ui::CustomButtonPtr shopmiddlebuybt( new ui::CustomButton( "shopmiddlebuybt" ) );
			shopmiddlebuybt->MakeImage( BITMAP_NEWUI_SHOP_LEFTDISPLAYBT );
			shopmiddlebuybt->Move( Coord( 3, 124 ) );
			shopmiddlebuybt->Resize( Dim( 72, 30 ) );
			shopmiddlebuybt->SetID( itemdata.s_dwItemGuid );
			shopmiddlebuybt->SetCustomButtonName( GlobalText[2293], Coord( 0, 8 ), TA_LEFT );
			shopmiddlebuybt->RegisterHandler( ui::eEvent_LButtonClick, data.s_handler, &ShopMiddleModule::OnBuyClk );
			girthshopmiddlebackpanel->AddChild( shopmiddlebuybt );

			ui::CustomButtonPtr shopmiddlepresentbt( new ui::CustomButton( "shopmiddlepresentbt" ) );
			shopmiddlepresentbt->MakeImage( BITMAP_NEWUI_SHOP_LEFTDISPLAYBT );
			shopmiddlepresentbt->Move( Coord( 78, 124 ) );
			shopmiddlepresentbt->Resize( Dim( 72, 30 ) );
			shopmiddlepresentbt->SetID( itemdata.s_dwItemGuid );
			shopmiddlepresentbt->SetCustomButtonName( GlobalText[2294], Coord( 0, 8 ), TA_LEFT );
			shopmiddlepresentbt->RegisterHandler( ui::eEvent_LButtonClick, data.s_handler, &ShopMiddleModule::OnPresentClk );
			girthshopmiddlebackpanel->AddChild( shopmiddlepresentbt );

			ui::ItemModuleViewPtr shopitemmodel( new ui::ItemModuleView( "shopitemmodel" ) );
			shopitemmodel->Move( Coord( 0, 30 ) );
			shopitemmodel->Resize( Dim( girthshopmiddlepanel->GetSize().x, 99 - 30 ) );
			shopitemmodel->SetToolTipItem( itemdata );
			shopitemmodel->SetID( itemdata.s_dwItemGuid );
			shopitemmodel->RegisterHandler( ui::eEvent_RButtonClick, data.s_handler, &ShopMiddleModule::OnCheckboxClk );
			shopitemmodel->RegisterHandler( ui::eEvent_LButtonClick, data.s_handler, &ShopMiddleModule::OnEquipmentItemClk );
			girthshopmiddlepanel->AddChild( shopitemmodel );
			
			if( itemdata.s_btSpecialOption != 0 )
			{
				Dim imgsize[3] = {
					Dim(39, 33), //new
					Dim(39, 33), //hot
					Dim(60, 47), //sale
				};
				ui::CompositedPanelPtr specialoption( new ui::CompositedPanel( "girthshopmiddlepanel", ui::ePanel_Piece_One ) );
				specialoption->SetImageIndex( (BITMAP_NEWUI_SHOP_LEFTDISPLAYBT+14)+(itemdata.s_btSpecialOption-1) );
				specialoption->SetID( ip.Type );
				specialoption->Move( Coord( 10, 0 ) );
				specialoption->Resize( imgsize[itemdata.s_btSpecialOption-1] );
				specialoption->EnableLButtonEvent( false );
				specialoption->EnableRButtonEvent( false );
				shopitemmodel->AddChild( specialoption );
			}

			Dim viewsize  = shopitemmodel->GetSize();
			int miniCount = 0;

			for( int y = 0; y < ipattribute->Height; ++y )
			{
				for( int x = 0; x < ipattribute->Width; ++x )
				{
					Coord miniinventoryPos = Coord( 13+(x*5), (viewsize.y-8)-(y*5) );

					ui::CompositedPanelPtr miniinventory( new ui::CompositedPanel( ( format( "miniinventory%1%" ) % miniCount ).str(), ui::ePanel_Piece_One ) );
					miniinventory->SetImageIndex( BITMAP_NEWUI_SHOP_LEFTDISPLAYBT+7 );
					miniinventory->Move( miniinventoryPos );
					miniinventory->Resize( Dim( 4, 4 ) );
					miniinventory->EnableLButtonEvent( false );
					shopitemmodel->AddChild( miniinventory );

					++miniCount;
				}
			}
		}

		if( ++count == 3 ) count = 0;
		
		if( ++i >= (1*MAX_CASHITEM_LIST_PER_PAGE) ) break;
	}
}

void ShopMiddleModule::ShopItemList::Destroy()
{
	m_ShopPageItems.clear();
}

CASHSHOP_ITEMLIST& ShopMiddleModule::ShopItemList::GetPageItem( CASHSHOP_ITEMLIST& csitem, const WORD itemcode )
{
	ChashShopMyCartItem_Map::iterator iter = m_ShopPageItems.find( itemcode );

	if( iter ==  m_ShopPageItems.end() )
	{
		assert( 0 );
		return csitem;
	}
	else
	{
		csitem = (*iter).second;
		return csitem;
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ShopMiddleModulePtr ShopMiddleModule::Make( Module::eType type )
{
	ShopMiddleModulePtr module( new ShopMiddleModule( type ) );
	module->Initialize( module );
	TheShopServerProxy().RegisterHandler( module );
	return module;
}

ShopMiddleModule::ShopMiddleModule( Module::eType type ) : Module( type ), m_BackItemCode( -1 )
{	

}

ShopMiddleModule::~ShopMiddleModule()
{
	Destroy();
}

void ShopMiddleModule::Initialize( BoostWeak_Ptr(ShopMiddleModule) handler )
{
	ui::UIWnd& shopleftFrame = TheUISystem().QueryFrame( UISHOPFRAMENAME )->GetChild( "shopframpanel" ).GetChild( "shopleftbackpanel" );

	m_FrameSize = shopleftFrame.GetSize();

	ui::PanelPtr shopmiddlemainbackpanel( new ui::Panel( "shopmiddlemainbackpanel" ) );
	shopmiddlemainbackpanel->SetColor( 0xffffffff );
	shopmiddlemainbackpanel->ChangeOpacityByPercent( 0.0f );
	shopmiddlemainbackpanel->Move( Coord( 118, 9 + 30 ) );
	shopmiddlemainbackpanel->Resize( Dim( 458 + 15, 472 ) );
	shopleftFrame.AddChild( shopmiddlemainbackpanel );

	ui::CustomButtonPtr shopleftpagebotton( new ui::CustomButton( "shopleftpagebotton" ) );
	shopleftpagebotton->MakeImage( BITMAP_NEWUI_SHOP_LEFTDISPLAYBT+8 );
	shopleftpagebotton->SetID( 0 );
	shopleftpagebotton->Move( Coord( 111 + ( 479 / 2 ) - 40, 511 ) );
	shopleftpagebotton->Resize( Dim( 23, 23 ) );
	shopleftpagebotton->RegisterHandler( ui::eEvent_LButtonClick, handler, &ShopMiddleModule::OnPageClk );
	shopleftFrame.AddChild( shopleftpagebotton );

	ui::CustomButtonPtr shoprightpagebotton( new ui::CustomButton( "shoprightpagebotton" ) );
	shoprightpagebotton->MakeImage( BITMAP_NEWUI_SHOP_LEFTDISPLAYBT+11 );
	shoprightpagebotton->SetID( 1 );
	shoprightpagebotton->Move( Coord( 111 + ( 479 / 2 ) + 23, 511 ) );
	shoprightpagebotton->Resize( Dim( 23, 23 ) );
	shoprightpagebotton->RegisterHandler( ui::eEvent_LButtonClick, handler, &ShopMiddleModule::OnPageClk );
	shopleftFrame.AddChild( shoprightpagebotton );

	string textpage = ( format( "%1%" ) % TheShopServerProxy().GetLowCategoryPageCount() ).str();
	ui::TextPtr shoppagetext( new ui::Text( "shoppagetext" ) );
	shoppagetext->Make( FONTSIZE_LANGUAGE, textpage.size() );
	shoppagetext->SetAlign( TA_LEFT );
	shoppagetext->SetContent( textpage );
	Coord textpos = Coord( 111 + ( 479 / 2 ), 516 );
	shoppagetext->Move( textpos );
	shoppagetext->EnableLButtonEvent( false );
	shopleftFrame.AddChild( shoppagetext );

	m_handler		= handler;
	m_ItemBackPanel = shopmiddlemainbackpanel;
	m_CurPageText   = shoppagetext;

	UpdateItemPanel();
}

void ShopMiddleModule::Destroy()
{

}

void ShopMiddleModule::OnBuyClk( ui::UIWnd& wnd )
{
	SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	
	MESSAGEBOXDATA data;
	m_BackItemCode = -1;
	
	CASHSHOP_ITEMLIST item = m_ShoopItems->GetPageItem( item, wnd.GetID() );
	int Type                = ConvertItemType( (BYTE*)item.s_btItemInfo );

	if( Type == 0x1FFF )
	{
		data.s_message.push_back( GlobalText[1518] );
		data.s_type    = eMB_OK;
	}
	else if( TheShopServerProxy().GetCashPoint() < item.s_wItemPrice )
	{
		data.s_message.push_back( GlobalText[2283] );
		data.s_type    = eMB_OK;
	}
	else if( (Type >= ITEM_HELPER+43 && Type <= ITEM_HELPER+45)
#ifdef PSW_SEAL_ITEM
			|| (Type == ITEM_HELPER+59)
#endif //PSW_SEAL_ITEM
#ifdef PSW_SEAL_ITEM
			|| (Type >= ITEM_HELPER+62 && Type <= ITEM_HELPER+63)
#endif //PSW_SEAL_ITEM
#ifdef ASG_ADD_CS6_ASCENSION_SEAL_MASTER
			|| Type == ITEM_HELPER+93	// 상승의인장마스터
#endif	// ASG_ADD_CS6_ASCENSION_SEAL_MASTER
#ifdef ASG_ADD_CS6_WEALTH_SEAL_MASTER
			|| Type == ITEM_HELPER+94	// 풍요의인장마스터
#endif	// ASG_ADD_CS6_WEALTH_SEAL_MASTER
#ifdef YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
			|| Type == ITEM_HELPER+103
#endif	// YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM			
		)
	{
		 if( g_isCharacterBuff((&Hero->Object), eBuff_Seal1)
		  || g_isCharacterBuff((&Hero->Object), eBuff_Seal2)   
		  || g_isCharacterBuff((&Hero->Object), eBuff_Seal3) 
#ifdef PSW_SEAL_ITEM
		  || g_isCharacterBuff((&Hero->Object), eBuff_Seal4)
#endif //PSW_SEAL_ITEM
#ifdef PSW_ADD_PC4_SEALITEM
		  || g_isCharacterBuff((&Hero->Object), eBuff_Seal_HpRecovery)
		  || g_isCharacterBuff((&Hero->Object), eBuff_Seal_MpRecovery)
#endif //PSW_ADD_PC4_SEALITEM
#ifdef ASG_ADD_CS6_ASCENSION_SEAL_MASTER
		  || g_isCharacterBuff((&Hero->Object), eBuff_AscensionSealMaster)	// 상승의인장마스터
#endif	// ASG_ADD_CS6_ASCENSION_SEAL_MASTER
#ifdef ASG_ADD_CS6_WEALTH_SEAL_MASTER
		  || g_isCharacterBuff((&Hero->Object), eBuff_WealthSealMaster)		// 풍요의인장마스터
#endif	// ASG_ADD_CS6_WEALTH_SEAL_MASTER
#ifdef YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
			|| g_isCharacterBuff((&Hero->Object), eBuff_PartyExpBonus)
#endif	// YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM			
		  )
		 {
#ifdef PSW_PARTCHARGE_ITEM2
			 data.s_message.push_back( GlobalText[2526] );
#else //PSW_PARTCHARGE_ITEM2
			 data.s_message.push_back( GlobalText[2249] );
#endif //PSW_PARTCHARGE_ITEM2			 
			
			data.s_type    = eMB_OK;
		 }
		 else
		 {
#ifdef PBG_MOD_MASTERLEVEL_MESSAGEBOX
			if((IsMasterLevel( Hero->Class ) == false)				//마스터 레벨이 아니고
				&& (Type == ITEM_HELPER+93
				|| Type == ITEM_HELPER+94))	
			{
				data.s_message.push_back( GlobalText[2531] );
				data.s_type = eMB_OK;
			}
			else if(IsMasterLevel( Hero->Class ) == true				//마스터 레벨이고
				&& (Type == ITEM_HELPER+43
				|| Type == ITEM_HELPER+44
#ifdef YDG_FIX_MASTERLEVEL_PARTY_EXP_BONUS_BLOCK
				|| Type == ITEM_HELPER+103			// 파티경험치 상승 구입 불가
#endif	// YDG_FIX_MASTERLEVEL_PARTY_EXP_BONUS_BLOCK
				)
				)
			{
				data.s_message.push_back( GlobalText[2531] );
				data.s_type = eMB_OK;
			}
			else
			{
#endif //PBG_MOD_MASTERLEVEL_MESSAGEBOX
			 m_BackItemCode = wnd.GetID();
			 data.s_message.push_back( GlobalText[2280] );
			 data.s_message.push_back( GlobalText[2281] );
			 data.s_type    = eMB_OKCANCEL;
#ifdef PBG_MOD_MASTERLEVEL_MESSAGEBOX
			}
#endif //PBG_MOD_MASTERLEVEL_MESSAGEBOX
		 }
	}

	else if ((Type >= ITEM_POTION+72 && Type <= ITEM_POTION+77)
#ifdef PSW_ADD_PC4_SCROLLITEM
		|| (Type >= ITEM_POTION+97 && Type <= ITEM_POTION+98)
#endif //PSW_ADD_PC4_SCROLLITEM
#ifdef YDG_ADD_HEALING_SCROLL
		|| Type == ITEM_POTION+140	// 치유의 스크롤
#endif	// YDG_ADD_HEALING_SCROLL
		)
	{
		if(g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll1)
			|| g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll2)
			|| g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll3)
			|| g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll4)
			|| g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll5)
			|| g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll6)
#ifdef PSW_ADD_PC4_SCROLLITEM
			|| g_isCharacterBuff((&Hero->Object), eBuff_Scroll_Battle)
			|| g_isCharacterBuff((&Hero->Object), eBuff_Scroll_Strengthen)
#endif //PSW_ADD_PC4_SCROLLITEM			
#ifdef YDG_ADD_HEALING_SCROLL
			|| g_isCharacterBuff((&Hero->Object), eBuff_Scroll_Healing)	// 치유의 스크롤
#endif	// YDG_ADD_HEALING_SCROLL
			)
		{
			data.s_message.push_back( GlobalText[2527] );
			data.s_type    = eMB_OK;			
		}
		else if( Type == ITEM_POTION+75 )
		{
			int Class = GetBaseClass ( Hero->Class );
			int ExClass = IsSecondClass(Hero->Class);
			
			if( Class==CLASS_DARK || Class==CLASS_WIZARD 
				|| (Class==CLASS_WIZARD && ExClass) 
				|| (Class==CLASS_SUMMONER)
				)
			{
				m_BackItemCode = wnd.GetID();
				data.s_message.push_back( GlobalText[2280] );
				data.s_message.push_back( GlobalText[2281] );
				data.s_type    = eMB_OKCANCEL;
			}
			else
			{
				data.s_message.push_back( GlobalText[2531] );
				data.s_type    = eMB_OK;
			}
		}
		else
		{
			m_BackItemCode = wnd.GetID();
			data.s_message.push_back( GlobalText[2280] );
			data.s_message.push_back( GlobalText[2281] );
			data.s_type    = eMB_OKCANCEL;
		}
	}
#ifdef PSW_CHARACTER_CARD
	else if( Type == ITEM_POTION+91 )
	{
		if( TheGameServerProxy().IsCharacterCard() )
		{
			data.s_message.push_back( GlobalText[2555] );
			data.s_type    = eMB_OK;
		}
		else
		{
			m_BackItemCode = wnd.GetID();
			data.s_message.push_back( GlobalText[2280] );
			data.s_message.push_back( GlobalText[2281] );
			data.s_type    = eMB_OKCANCEL;
		}
	}
#endif //PSW_CHARACTER_CARD
#ifdef YDG_MOD_AURA_ITEM_EXCLUSIVELY_BUY
	else if( Type == ITEM_HELPER+104 || Type == ITEM_HELPER+105 )		// 오라 중복구입 금지
	{
		if (g_isCharacterBuff((&Hero->Object), eBuff_AG_Addition)
#ifdef LJH_FIX_BUG_BUYING_AG_AURA_WHEN_USING_SD_AURA			
			|| g_isCharacterBuff((&Hero->Object), eBuff_SD_Addition))
#else	//LJH_FIX_BUG_BUYING_AG_AURA_WHEN_USING_SD_AURA			
			|| g_isCharacterBuff((&Hero->Object), eBuff_AG_Addition))
#endif	//LJH_FIX_BUG_BUYING_AG_AURA_WHEN_USING_SD_AURA			
		{
			data.s_message.push_back( GlobalText[3056] );
			data.s_type    = eMB_OK;			
		}
		else
		{
			m_BackItemCode = wnd.GetID();
			data.s_message.push_back( GlobalText[2280] );
			data.s_message.push_back( GlobalText[2281] );
			data.s_type    = eMB_OKCANCEL;
		}
	}
#endif	// YDG_MOD_AURA_ITEM_EXCLUSIVELY_BUY
	else
	{
		m_BackItemCode = wnd.GetID();
		data.s_message.push_back( GlobalText[2280] );
		data.s_message.push_back( GlobalText[2281] );
		data.s_type    = eMB_OKCANCEL;
	}

	data.s_handler = m_handler;
	data.s_id      = 0;
	MessageBoxMake( data );
	PlayBuffer(SOUND_CLICK01);
}

void ShopMiddleModule::OnEquipmentItemClk( ui::UIWnd& wnd )
{
	SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();

	int ItemIndex = wnd.GetID();
	
	CASHSHOP_ITEMLIST item = m_ShoopItems->GetPageItem( item, ItemIndex );

	if( item.s_dwItemGuid == 0 ) 
	{
		assert( 0 );
		return;
	}

	TheShopServerProxy().ChangeEquipmentItem( item );
	PlayBuffer(SOUND_CLICK01);
}

void ShopMiddleModule::OnPresentClk( ui::UIWnd& wnd )
{
	SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	
	int itemcode = wnd.GetID();

	MESSAGEBOXDATA data;
	data.s_handler = m_handler;
	data.s_message.push_back( GlobalText[2303] );
	data.s_id      = 1;
	data.s_type    = eMB_OK;
	MessageBoxMake( data );
	PlayBuffer(SOUND_CLICK01);
}

void ShopMiddleModule::OnCheckboxClk( ui::UIWnd& wnd )
{
	SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	
	m_BackItemCode = wnd.GetID();

	MESSAGEBOXDATA data;

	data.s_handler = m_handler;

	if( TheShopServerProxy().GetHighCategoryType() == eHighCategory_NewItem )
	{
		data.s_message.push_back( GlobalText[2286] );
	}
	else
	{
		data.s_message.push_back( GlobalText[2287] );
	}

	data.s_id      = 1;
	data.s_type    = eMB_OKCANCEL;

	MessageBoxMake( data );
	PlayBuffer(SOUND_CLICK01);
}

void ShopMiddleModule::OnPageClk( ui::UIWnd& wnd )
{
	SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	
	TheShopServerProxy().ChangeLowCategorPageClk( wnd.GetID() );
	PlayBuffer(SOUND_CLICK01);
}

void ShopMiddleModule::OnUpDateShopItemInfo( const eCashShopResult result )
{
	if( result == eCashShop_Success )
	{
		DWORD page = TheShopServerProxy().GetLowCategoryPageCount();

		string textpage = ( format( "%1%" ) % page ).str();
		Coord textpos = Coord( 111 + ( 479 / 2 ), 516 );

		m_CurPageText->Make( FONTSIZE_LANGUAGE, textpage.size() );
		m_CurPageText->SetContent( textpage );
		m_CurPageText->Move( textpos );
		
		UpdateItemPanel( TheShopServerProxy().GetHighCategoryType() );
	}
	else
	{
		//에러 메세지 출력 할것
	}
}

void ShopMiddleModule::OnCashShopPurchaseResult()
{
	if( m_BackItemCode == -1 ) return;

	CASHSHOP_ITEMLIST item = m_ShoopItems->GetPageItem( item, m_BackItemCode );

	if( item.s_dwItemGuid == 0 )
	{
		assert( 0 );
		return;
	}

	ITEM ip;
	ChangeCodeItem( &ip, (BYTE*)item.s_btItemInfo );
	ITEM_ATTRIBUTE* ipattribute = &ItemAttribute[ip.Type];


	//삭제 하기 위함
	if( TheShopServerProxy().GetHighCategoryType() != eHighCategory_NewItem )
	{
		TheShopServerProxy().ChangeCheckBtClk( item );
	}

	//중국은 문구가 이상합니다....수정 요청..
	//GlobalText[2282] 이안에 %s가 있으면 안 됩니다.
	MESSAGEBOXDATA data;
	data.s_handler = m_handler;
	data.s_message.push_back( ( format( "%1% %2%" ) % ipattribute->Name % GlobalText[2282]).str() );
	data.s_id      = 1;
	data.s_type    = eMB_OK;

	MessageBoxMake( data );

	m_BackItemCode = -1;
}

void ShopMiddleModule::UpdateItemPanel( eHighCategoryType type )
{
	if( m_ShoopItems ) 
	{
		//여기부분 주의 순서를 잘 해줄것...!!!
		m_ShoopItems.reset();
	}

	m_ItemBackPanel->ClearChild();

	ShopCashShopItem_List		OutData;

	switch( type )
	{
	case eHighCategory_NewItem:
		{
			TheShopServerProxy().GetCategoryShopItem( OutData );
		}
		break;
	case eHighCategory_OldItem:
		{
			TheShopServerProxy().GetShopMyCart( OutData );
		}
		break;
	}

	if( OutData.size() == 0 ) return;
	
	ShopMiddleModule::ShopItemListData			data;

	data.s_BackPanel    = m_ItemBackPanel;
	data.s_handler      = m_handler;
	data.s_ItemDatas    = OutData;
	data.s_IntervalSize = Dim( 8, 8 );
	data.s_page         = TheShopServerProxy().GetLowCategoryPageCount();

	m_ShoopItems = ShopMiddleModule::ShopItemList::Make( data );
}

void ShopMiddleModule::OnMessageBox_OK( int id )
{
	if( m_BackItemCode == -1 ) return;

	if( id == 0 )
	{
		TheShopServerProxy().CashShopPurchase( m_BackItemCode );
	}
	else
	{
		CASHSHOP_ITEMLIST item = m_ShoopItems->GetPageItem( item, m_BackItemCode );

		if( item.s_dwItemGuid == 0 )
		{
			assert( 0 );
			return;
		}

		TheShopServerProxy().ChangeCheckBtClk( item );

		m_BackItemCode = -1;
	}
}

void ShopMiddleModule::OnMessageBox_CANCEL( int id )
{

}

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM

/*
	UISCRIPT checkbutton;
	checkbutton.s_ParentFrameName = "ShopTopModule";
	checkbutton.s_ParentWndName.push_back( "shopframpanel" );
	checkbutton.s_ParentWndName.push_back( "shopleftbackpanel" );
	checkbutton.s_ParentWndName.push_back( "girthshopmiddlepanel" );
	checkbutton.s_UiName          = "shopmiddlepreviewcheckbt";
	checkbutton.s_UiID            = ItemIndex;
	checkbutton.s_UIImgIndex = BITMAP_NEWUI_SHOP_LEFTDISPLAYBT+6;
	checkbutton.s_Position = Coord( 126, 4 );
	checkbutton.s_Size = Dim( 14, 14 );

	ui::UIWnd& parentuiwnd = TheUISystem().QueryFrame( checkbutton.s_ParentFrameName )->QueryChild( checkbutton.s_ParentWndName );
	UIMAKER( ui::CustomCheckButton, checkbutton, parentuiwnd );
	parentuiwnd.GetChild( checkbutton.s_UiName ).RegisterHandler( ui::eEvent_LButtonClick, data.s_handler, &ShopMiddleModule::OnCheckboxClk );
*/