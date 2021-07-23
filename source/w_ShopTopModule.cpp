// w_ShopTopModule.cpp: implementation of the ShopTopModule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_Module.h"
#include "w_ShopTopModule.h"

#include "w_UIType.h"
#include "w_UIFrame.h"
#include "w_CompositedPanel.h"
#include "w_Panel.h"
#include "w_RadioButton.h"
#include "w_CustomRadioButton.h"
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
#include "NewUIMyInventory.h"
#include "NewUISystem.h"

#include "DSPlaySound.h"

MAKE_OBJECT_POOL( ShopTopModule );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ShopTopModulePtr ShopTopModule::Make( Module::eType type )
{
	ShopTopModulePtr module( new ShopTopModule( type ) );
	module->Initialize( module );
	TheShopServerProxy().RegisterHandler( module );
	return module;
}

ShopTopModule::ShopTopModule( Module::eType type ) : Module( type )
{

}

ShopTopModule::~ShopTopModule()
{
	Destroy();
}

void ShopTopModule::Initialize( BoostWeak_Ptr(ShopTopModule) handler )
{
	int i = 0;

	Dim WindowSiz = TheShell().GetWindowSize();

	ui::UIFramePtr frame = ui::UIFrame::Make( UISHOPFRAMENAME, false );
	frame->Move( Coord( 0, 0 ) );
	frame->Resize( WindowSiz );
	TheUISystem().AddFrame( frame );

	ui::PanelPtr backtempPanel( new ui::Panel("backtemppanel") );
	backtempPanel->SetColor( 0x00000000 );
	backtempPanel->ChangeOpacityByPercent( 0.9f );
	backtempPanel->Move( Coord( 0, 0 ) );
	backtempPanel->Resize( WindowSiz );
	frame->AddChild( backtempPanel );

	//FrameBack Panel
	if( WindowSiz != Dim( 800, 600 ) )
	{
		int panelspaceX = (WindowSiz == Dim( 1024, 768 )) ? -127 : 1;
		int panelspaceY = (WindowSiz == Dim( 1024, 768 )) ? -120 : 1;
		int panelspace = 219+panelspaceY;

		Coord mainbackpanel[14] =
		{
			Coord( panelspaceX, panelspaceY ), Coord( panelspaceX+256, panelspaceY ), Coord( panelspaceX+(256*2), panelspaceY ), 
			Coord( panelspaceX+(256*3), panelspaceY ), Coord( panelspaceX+(256*4)-2, panelspaceY ), 

			Coord( panelspaceX, panelspace ), Coord( panelspaceX+(256*4)-2, panelspace ),
			Coord( panelspaceX, panelspace+256 ), Coord( panelspaceX+(256*4)-2, panelspace+256 ),
			
			Coord( panelspaceX, panelspace+(256*2)-2 ), Coord( panelspaceX+256, panelspace+(256*2)-2 ), Coord( panelspaceX+(256*2), panelspace+(256*2)-2 ), 
			Coord( panelspaceX+(256*3), panelspace+(256*2)-2 ), Coord( panelspaceX+(256*4)-2, panelspace+(256*2)-2 ),
		};

		for ( i = 0; i < 14; ++i )
		{
			ui::CompositedPanelPtr shopmainbackpanel( new ui::CompositedPanel( ( format( "shopmainbackpanel%1%" ) % i ).str(), ui::ePanel_Piece_One ) );
			shopmainbackpanel->SetImageIndex( BITMAP_NEWUI_SHOP_MAINBACKPANEL+i );
			shopmainbackpanel->Move( mainbackpanel[i] );
			shopmainbackpanel->Resize( Dim( 256, 256 ) );
			frame->AddChild( shopmainbackpanel );
		}
	}

	//Base
	ui::PanelPtr shopframpanel( new ui::Panel("shopframpanel") );
	shopframpanel->SetColor( 0xff2C2C30 );
	shopframpanel->ChangeOpacityByPercent( 0.0f );
	shopframpanel->Move( Coord( 0, 0 ) );
	shopframpanel->Resize( frame->GetSize() );
	frame->AddChild( shopframpanel );

	//ShopBack Panel
	Coord shopleftbackpanelpos[12] =
	{
		Coord( 0, 0 ), Coord( 256, 0 ), Coord( 0, 256 ), Coord( 256, 256 ), 
		Coord( 512, 0 ), Coord( 512, 256 ), Coord( 0, 512 ), Coord( 256, 512 ), Coord( 512, 512 ),
		Coord( 512+84, 0 ), Coord( 512+84, 256 ), Coord( 512+84, 512 ),
	};

	Rect uvrect[12] = 
	{
		Rect( 0, 0, 0, 0 ),
		Rect( 0, 0, 0, 0 ),
		Rect( 0, 0, 0, 0 ),
		Rect( 0, 0, 0, 0 ),
		Rect( 0, 0, 84, 256 ),
		Rect( 84, 0, 168, 256 ),
		Rect( 0, 0, 256, 28 ),
		Rect( 0, 28, 256, 56 ),
		Rect( 0, 56, 84, 84 ),
		Rect( 0, 0, 203, 256 ),
		Rect( 0, 0, 203, 256 ),
		Rect( 0, 84, 203, 112 ),
	};

	Dim shopleftbackpanelsize[12] =
	{
		Dim( 256, 256 ), Dim( 256, 256 ), Dim( 256, 256 ), Dim( 256, 256 ), 
		Dim( uvrect[4].right - uvrect[4].left, uvrect[4].bottom - uvrect[4].top ), 
		Dim( uvrect[5].right - uvrect[5].left, uvrect[5].bottom - uvrect[5].top ), 
		Dim( uvrect[6].right - uvrect[6].left, uvrect[6].bottom - uvrect[6].top ), 
		Dim( uvrect[7].right - uvrect[7].left, uvrect[7].bottom - uvrect[7].top ), 
		Dim( uvrect[8].right - uvrect[8].left, uvrect[8].bottom - uvrect[8].top ), 
		Dim( uvrect[9].right - uvrect[9].left, uvrect[9].bottom - uvrect[9].top ), 
		Dim( uvrect[10].right - uvrect[10].left, uvrect[10].bottom - uvrect[10].top ), 
		Dim( uvrect[11].right - uvrect[11].left, uvrect[11].bottom - uvrect[11].top ), 
	};

	int imageindex[12] = 
	{
		BITMAP_NEWUI_SHOP_SUBBACKPANEL,
		BITMAP_NEWUI_SHOP_SUBBACKPANEL+1,
		BITMAP_NEWUI_SHOP_SUBBACKPANEL+2,
		BITMAP_NEWUI_SHOP_SUBBACKPANEL+3,

		BITMAP_NEWUI_SHOP_SUBBACKPANEL+4,
		BITMAP_NEWUI_SHOP_SUBBACKPANEL+4,
		BITMAP_NEWUI_SHOP_SUBBACKPANEL+5,
		BITMAP_NEWUI_SHOP_SUBBACKPANEL+5,
		BITMAP_NEWUI_SHOP_SUBBACKPANEL+5,

		BITMAP_NEWUI_SHOP_SUBBACKPANEL+6,
		BITMAP_NEWUI_SHOP_SUBBACKPANEL+7,
		BITMAP_NEWUI_SHOP_SUBBACKPANEL+5,
	};

	for ( i = 0; i < 12; ++i )
	{
		ui::CompositedPanelPtr shopsubbackpanel( new ui::CompositedPanel( ( format( "shopsubbackpanel%1%" ) % i ).str(), ui::ePanel_Piece_One ) );
		shopsubbackpanel->SetImageIndex( imageindex[i] );
		shopsubbackpanel->Move( ( frame->GetSize() / 2 ) - ( Coord( 800, 600 ) / 2 ) + shopleftbackpanelpos[i] );
		shopsubbackpanel->Resize( shopleftbackpanelsize[i] );
		shopsubbackpanel->SetUVRect( uvrect[i] );
		shopframpanel->AddChild( shopsubbackpanel );
	}

	//Left, Middle Module
	ui::PanelPtr shopleftbackpanel( new ui::Panel("shopleftbackpanel") );
	shopleftbackpanel->SetColor( 0xff2C2C30 );
	shopleftbackpanel->ChangeOpacityByPercent( 0.0f );
	shopleftbackpanel->Move( ( frame->GetSize() / 2 ) - ( Coord( 800, 600 ) / 2 ) );
	shopleftbackpanel->Resize( Dim( 596, 540 ) );
	shopleftbackpanel->EnableLButtonEvent( true );
	shopframpanel->AddChild( shopleftbackpanel );

	//Right Module
	ui::PanelPtr shoprightbackpanel( new ui::Panel( "shoprightbackpanel" ) );
	shoprightbackpanel->SetColor( 0xff2C2C30 );
	shoprightbackpanel->ChangeOpacityByPercent( 0.0f );
	shoprightbackpanel->Move( Coord( shopleftbackpanel->GetPosition().x + shopleftbackpanel->GetSize().x,
									 shopleftbackpanel->GetPosition().y ) );
	shoprightbackpanel->Resize( Dim( 800 - shopleftbackpanel->GetSize().x, 540 ) );
	shoprightbackpanel->EnableLButtonEvent( true );
	shopframpanel->AddChild( shoprightbackpanel );

	//HighCategory TapButton info
	int                tapcount = 0;
	ShopCategory_List infodata;
	TheGameInfoBuilder().QueryShopUIInfo().QueryHighShopCategory( TheShopServerProxy().GetShopType(), infodata );

	//HighCategory TapButton Group
	ui::RadioButton::GroupPtr highcategory( new ui::RadioButton::Group );

	for( ShopCategory_List::iterator iter = infodata.begin(); iter != infodata.end(); )
	{
		ShopCategory_List::iterator curiter = iter;
		++iter;
		SHOPCATEGORY& Tempdata = *curiter;

		ui::CustomRadioButtonPtr highcategorytapbutton( new ui::CustomRadioButton( ( format( "highcategorytapbutton%1%" ) % tapcount ).str() ) );
		highcategorytapbutton->SetID( Tempdata.s_HighCategory );
		highcategorytapbutton->RegisterGroup( highcategory, highcategorytapbutton );
		highcategorytapbutton->MakeImage( BITMAP_NEWUI_SHOP_LEFTBT+7 );
		highcategorytapbutton->Move( Coord( 111 + ( tapcount * 84 ), 8 ) );
		highcategorytapbutton->Resize( Dim( 86, 27 ) );
		highcategorytapbutton->RegisterHandler( ui::eEvent_LButtonClick, handler, &ShopTopModule::OnHighCategoryClk );
		highcategorytapbutton->SetRadioButtonName( Tempdata.s_ButtonName, Coord( 0, 3 ), TA_LEFT );
		shopleftbackpanel->AddChild( highcategorytapbutton );
		if( Tempdata.s_HighCategory == eHighCategory_NewItem && Tempdata.s_LowCategory == eLowCategory_None ) 
		{
			highcategorytapbutton->BasicChangeFrame();
			TheShopServerProxy().ChangeHighCategorClk( static_cast<eHighCategoryType>(Tempdata.s_HighCategory) );
		}
		++tapcount;
	}
}

void ShopTopModule::Destroy()
{

}

void ShopTopModule::OnHighCategoryClk( ui::UIWnd& wnd )
{
	SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	
	int CategoryIndex = wnd.GetID();

	TheShopServerProxy().ChangeHighCategorClk( static_cast<eHighCategoryType>(CategoryIndex) );
	PlayBuffer(SOUND_CLICK01);
}

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM
