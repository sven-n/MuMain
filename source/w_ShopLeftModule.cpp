// w_ShopLeftModule.cpp: implementation of the ShopLeftModule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_Module.h"
#include "w_ShopLeftModule.h"

#include "w_Text.h"
#include "w_ShopUIInfo.h"

#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "NewUIInventoryCtrl.h"

#include "DSPlaySound.h"

MAKE_OBJECT_POOL( ShopLeftModule );
MAKE_OBJECT_POOL( ShopLeftModule::ShopCategoryList );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ShopLeftModule::ShopCategoryListPtr ShopLeftModule::ShopCategoryList::Make( ShopLeftModule::ShopCategoryData& data )
{
	ShopLeftModule::ShopCategoryListPtr categorylist( new ShopLeftModule::ShopCategoryList( data ) );
	return categorylist;
}

ShopLeftModule::ShopCategoryList::ShopCategoryList( ShopLeftModule::ShopCategoryData& data )
{
	Initialize( data );
}

ShopLeftModule::ShopCategoryList::~ShopCategoryList()
{
	Destroy();
}

void ShopLeftModule::ShopCategoryList::Initialize( ShopLeftModule::ShopCategoryData& data )
{
	int i = 0;

	ui::CompositedPanelPtr decopanel1( new ui::CompositedPanel( "decopanel1", ui::ePanel_Piece_Height_Three ) );
	decopanel1->SetImageIndex( BITMAP_NEWUI_SHOP_LEFTBT+1 );
	decopanel1->Move( Coord( (0 + data.s_IntervalSize.x) + 41, 18 ) );
	decopanel1->Resize( Dim( 4, (data.s_ItemDatas.size()*data.s_IntervalSize.y)-24 ) );
	data.s_BackPanel.lock()->AddChild( decopanel1 );

	ui::CompositedPanelPtr decopanel0( new ui::CompositedPanel( "decopanel0", ui::ePanel_Piece_One ) );
	decopanel0->SetImageIndex( BITMAP_NEWUI_SHOP_LEFTBT );
	decopanel0->Move( Coord( -6, -16 ) );
	decopanel0->Resize( Dim( 58, 70 ) );
	data.s_BackPanel.lock()->AddChild( decopanel0 );

	ui::CompositedPanelPtr decopanel2( new ui::CompositedPanel( "decopanel2", ui::ePanel_Piece_One ) );
	decopanel2->SetImageIndex( BITMAP_NEWUI_SHOP_LEFTBT+4 );
	decopanel2->Move( Coord( -6, (data.s_ItemDatas.size()*data.s_IntervalSize.y) - 43 ) );
	decopanel2->Resize( Dim( 58, 125 ) );
	data.s_BackPanel.lock()->AddChild( decopanel2 );

	ui::RadioButton::GroupPtr lowcategory( new ui::RadioButton::Group );

	for( list<SHOPCATEGORY>::iterator iter = data.s_ItemDatas.begin(); iter != data.s_ItemDatas.end(); )
	{
		list<SHOPCATEGORY>::iterator curiter = iter;
		++iter;
		SHOPCATEGORY& Tempdata = *curiter;

		ui::CustomRadioButtonPtr lowcategorybt( new ui::CustomRadioButton( ( format( "lowcategorybt%1%" ) % i ).str() ) );
		lowcategorybt->SetID( Tempdata.s_LowCategory );
		lowcategorybt->RegisterGroup( lowcategory, lowcategorybt );
		lowcategorybt->MakeImage( BITMAP_NEWUI_SHOP_LEFTBT+5 );
		lowcategorybt->Move( Coord( 8 + data.s_IntervalSize.x, 9 + ( data.s_IntervalSize.y * i ) ) );
		lowcategorybt->Resize( Dim( 88, 30 ) );
		lowcategorybt->SetRadioButtonName( Tempdata.s_ButtonName, Coord( 0, 9 ), TA_LEFT );
		lowcategorybt->RegisterHandler( ui::eEvent_LButtonClick, data.s_handler, &ShopLeftModule::OnLowCategoryClk );

		if( Tempdata.s_LowCategory == TheShopServerProxy().GetLowCategoryType() )
		{
			lowcategorybt->BasicChangeFrame();
		}

		data.s_BackPanel.lock()->AddChild( lowcategorybt );

		++i;
	}
}

void ShopLeftModule::ShopCategoryList::Destroy()
{

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ShopLeftModulePtr ShopLeftModule::Make( Module::eType type )
{
	ShopLeftModulePtr module( new ShopLeftModule( type ) );
	module->Initialize( module );
	TheShopServerProxy().RegisterHandler( module );
	return module;
}

ShopLeftModule::ShopLeftModule( Module::eType type ) : Module( type )
{

}

ShopLeftModule::~ShopLeftModule()
{
	Destroy();
}

void ShopLeftModule::Initialize( BoostWeak_Ptr(ShopLeftModule) handler )
{
	int i = 0;

	ui::UIWnd& shopleftFrame = TheUISystem().QueryFrame( UISHOPFRAMENAME )->GetChild( "shopframpanel" ).GetChild( "shopleftbackpanel" );

	ui::PanelPtr shopmiddlemainbackpanel( new ui::Panel( "shopmiddlemainbackpanel" ) );
	shopmiddlemainbackpanel->SetColor( 0xffffffff );
	shopmiddlemainbackpanel->ChangeOpacityByPercent( 0.0f );
	shopmiddlemainbackpanel->Move( Coord( 6, 32 ) );
	shopmiddlemainbackpanel->Resize( Dim( 108 - 6, 532 - 32 ) );
	shopleftFrame.AddChild( shopmiddlemainbackpanel );

	m_handler		    = handler;
	m_CategoryBackPanel = shopmiddlemainbackpanel;

	ShopCategory_List shopcategorydata;

	TheGameInfoBuilder().QueryShopUIInfo().QueryLowShopCategory( TheShopServerProxy().GetShopType(), 
																 TheShopServerProxy().GetHighCategoryType(),
																 shopcategorydata 
															   );
	UpdateItemPanel( shopcategorydata );
}

void ShopLeftModule::Destroy()
{

}

void ShopLeftModule::OnUpDateShopItemInfo( const eCashShopResult result )
{
	if( result == eCashShop_Success )
	{
		ShopCategory_List shopcategorydata;

		TheGameInfoBuilder().QueryShopUIInfo().QueryLowShopCategory( TheShopServerProxy().GetShopType(), 
																	 TheShopServerProxy().GetHighCategoryType(),
																	 shopcategorydata 
																   );
		UpdateItemPanel( shopcategorydata );
	}
	else
	{
		//에러 메세지 출력 할것.
	}
}

void ShopLeftModule::UpdateItemPanel( list<SHOPCATEGORY>& categorydata )
{
	//여기부분 주의 순서를 잘 해줄것...!!!
	if( m_ShopCategorys ) m_ShopCategorys.reset();

	m_CategoryBackPanel->ClearChild();

	ShopLeftModule::ShopCategoryData			data;

	data.s_BackPanel    = m_CategoryBackPanel;
	data.s_handler      = m_handler;
	data.s_ItemDatas    = categorydata;
	data.s_IntervalSize = Dim( 0, 37 );

	m_ShopCategorys = ShopLeftModule::ShopCategoryList::Make( data );
}

void ShopLeftModule::OnLowCategoryClk( ui::UIWnd& wnd )
{
	if(SEASON3B::CNewUIInventoryCtrl::GetPickedItem()) 
	{
		SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	}
	
	int CategoryIndex = wnd.GetID();
	TheShopServerProxy().ChangeLowCategorClk( static_cast<eLowCategoryType>(CategoryIndex) );
	PlayBuffer(SOUND_CLICK01);
}

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM
