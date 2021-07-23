// w_ShopRightModule.cpp: implementation of the ShopRightModule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_Module.h"

#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"

#include "w_UIType.h"
#include "w_UIWnd.h"
#include "w_CustomButton.h"
#include "w_InventoryBox.h"

#include "w_Panel.h"
#include "w_ShopRightModule.h"

#include "iexplorer.h"

#include "NewUIInventoryCtrl.h"
#include "NewUIMyInventory.h"
#include "NewUISystem.h"

#include "DSPlaySound.h"

#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

MAKE_OBJECT_POOL( ShopRightModule );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ShopRightModulePtr ShopRightModule::Make( Module::eType type )
{
	ShopRightModulePtr module( new ShopRightModule( type ) );
	module->Initialize( module );
	TheShopServerProxy().RegisterHandler( module );
	return module;
}

ShopRightModule::ShopRightModule( Module::eType type ) : Module( type )
{

}

ShopRightModule::~ShopRightModule()
{
	Destroy();
}

void ShopRightModule::Initialize( BoostWeak_Ptr(ShopRightModule) handler )
{
	ui::UIWnd& shoprightFrame = TheUISystem().QueryFrame( UISHOPFRAMENAME )->GetChild( "shopframpanel" ).GetChild( "shoprightbackpanel" );

	ui::InventoryBoxPtr shoprightmyinvenbox( new ui::InventoryBox( "shoprightmyinvenbox" ) );
	shoprightmyinvenbox->MakeImage( BITMAP_NEWUI_SHOP_RIGHTBT, Dim( 8, 8 ) );
	shoprightmyinvenbox->SetItem( 0 );
	shoprightmyinvenbox->Move( Coord( 14+8, 257+8 ) );
	shoprightmyinvenbox->Resize( Dim( 22, 22 ) );
	shoprightFrame.AddChild( shoprightmyinvenbox );

	ui::CustomButtonPtr shoprightresetbt( new ui::CustomButton( "shoprightresetbt" ) );
	shoprightresetbt->MakeImage( BITMAP_NEWUI_SHOP_LEFTDISPLAYBT );
	shoprightresetbt->Move( Coord( (shoprightFrame.GetSize().x/2) - (72/2) , 224 ) );
	shoprightresetbt->Resize( Dim( 72, 30 ) );
	shoprightresetbt->SetCustomButtonName( GlobalText[2292], Coord( 0, 9 ), TA_LEFT );
	shoprightresetbt->RegisterHandler( ui::eEvent_LButtonClick, handler, &ShopRightModule::OnResetClk );
	shoprightFrame.AddChild( shoprightresetbt );

	ui::PanelPtr previewPanel( new ui::Panel("previewPanel") );
	previewPanel->SetColor( 0xff2C2C30 );
	previewPanel->ChangeOpacityByPercent( 0.0f );
	previewPanel->Move( Coord( 17, 18 ) );
	previewPanel->Resize( Dim( 170, 198 ) );
	shoprightFrame.AddChild( previewPanel );
	
	ui::ModelViewPtr shoppreviewmodel( new ui::ModelView( "shoppreviewmodel" ) );
	shoppreviewmodel->CopyPlayer();
	shoppreviewmodel->Move( Coord( 0, 0 ) );
	shoppreviewmodel->Resize( Dim( 170, 198 ) );
	previewPanel->AddChild( shoppreviewmodel );

	ui::CustomButtonPtr cashbutton( new ui::CustomButton( "cashbutton" ) );
	cashbutton->MakeImage( BITMAP_NEWUI_SHOP_LEFTDISPLAYBT );
	cashbutton->Move( Coord( 14, 224 + 240 ) );
	cashbutton->Resize( Dim( 72, 30 ) );
	cashbutton->SetCustomButtonName( GlobalText[2290], Coord( 0, 9 ), TA_LEFT );
	cashbutton->RegisterHandler( ui::eEvent_LButtonClick, handler, &ShopRightModule::OnCashClk );
	shoprightFrame.AddChild( cashbutton );

	ui::CustomButtonPtr exitbutton( new ui::CustomButton( "exitbutton" ) );
	exitbutton->MakeImage( BITMAP_NEWUI_SHOP_RIGHTBT+2 );
	exitbutton->Move( Coord( 14, 224 + 240 + 34 ) );
	exitbutton->Resize( Dim( 34, 30 ) );
	exitbutton->RegisterHandler( ui::eEvent_LButtonClick, handler, &ShopRightModule::OnExitClk );
	shoprightFrame.AddChild( exitbutton );

	string pricename = GlobalText[2289];
	ui::TextPtr cashpricenametext( new ui::Text("cashpricenametext") );
	cashpricenametext->Make( FONTSIZE_LANGUAGE, pricename.length() );
	cashpricenametext->SetAlign( TA_LEFT );
	cashpricenametext->SetContent( pricename );
	cashpricenametext->SetColor( 0xffFFEE9E );
	Coord textpos = Coord( 14+((70/2)-(cashpricenametext->GetSize().x/2)), 444 );
	cashpricenametext->Move( textpos );
	shoprightFrame.AddChild( cashpricenametext );

	pricename = ( format( "%1%" ) % (TheShopServerProxy().GetCashPoint()) ).str();
	ui::TextPtr cashtext( new ui::Text("cashtext") );
	cashtext->Make( FONTSIZE_LANGUAGE, pricename.length() );
	cashtext->SetAlign( TA_LEFT );
	cashtext->SetContent( pricename );
	cashtext->SetColor( 0xff00ED1B );
	textpos = Coord( 84+((107/2)-(cashtext->GetSize().x/2)), 444);
	cashtext->Move( textpos );
	shoprightFrame.AddChild( cashtext );

	m_handler	   = handler;
	m_ModelView    = shoppreviewmodel; 
	m_PreviewPanel = previewPanel;
	m_Price		   = cashtext;
}

void ShopRightModule::Destroy()
{
	if( m_ModelView ) 
	{
		m_ModelView.reset();
	}
}

void ShopRightModule::OnChangeEquipmentItem( const CASHSHOP_ITEMLIST& item )
{
	if( m_ModelView ) 
	{
		if( !m_ModelView->SetEquipmentItem( item ) )
		{
			//에러 메세지 출력 
			//착용 할 수 없는 아이템

			MESSAGEBOXDATA data;
			data.s_handler = m_handler;
			data.s_message.push_back( GlobalText[2285] );
			data.s_id      = 0;
			data.s_type    = eMB_OK;

			MessageBoxMake( data );
		}
	}
}

void ShopRightModule::OnResetClk( ui::UIWnd& wnd )
{
	SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	
	if( m_PreviewPanel ) 
	{
		if( m_ModelView ) 
		{
			m_ModelView.reset();
		}

		m_PreviewPanel->ClearChild();
		
		ui::ModelViewPtr shoppreviewmodel( new ui::ModelView( "shoppreviewmodel" ) );
		shoppreviewmodel->CopyPlayer();
		shoppreviewmodel->Move( Coord( 0, 0 ) );
		shoppreviewmodel->Resize( Dim( 170, 198 ) );
		m_PreviewPanel->AddChild( shoppreviewmodel );

		m_ModelView = shoppreviewmodel;
	}
	PlayBuffer(SOUND_CLICK01);
}

#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
extern BOOL g_bUseWindowMode;
#endif

void ShopRightModule::OnCashClk( ui::UIWnd& wnd )
{
	SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	
	//웹에다가 연결 한다.
	if( g_bUseWindowMode )
	{
		//::ShellExecute(NULL, _T("open"), "http://www.muonline.co.kr", NULL,NULL, SW_SHOW);
		// 2295 "http://www.muonline.co.kr"
		if( leaf::OpenExplorer( GlobalText[2295] ) == false ) {
			assert(0);
		}
	}
	else
	{
		MESSAGEBOXDATA data;
		data.s_handler = m_handler;
		data.s_message.push_back( GlobalText[2288] );
		data.s_id      = 0;
		data.s_type    = eMB_OK;

		MessageBoxMake( data );
	}

	PlayBuffer(SOUND_CLICK01);
}

void ShopRightModule::OnExitClk( ui::UIWnd& wnd )
{
	SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	
	if( !TheUISystem().IsFrame( UIMESSAGEBOXFRAME_NAME ) )
	{
		TheShopServerProxy().SetShopOut();
	}

	PlayBuffer(SOUND_CLICK01);
}

void ShopRightModule::OnChangeCashPoint( const DWORD cashpoint )
{
	if( m_Price )
	{
		string pricename = ( format( "%1%" ) % cashpoint  ).str();
		m_Price->Make( FONTSIZE_LANGUAGE, pricename.length() );
		m_Price->SetContent( pricename );
		m_Price->SetColor( 0xff00ED1B );
		m_Price->Move( Coord( 84+((107/2)-(m_Price->GetSize().x/2)), 444) );
	}
}

void ShopRightModule::OnMessageBox_OK( int id )
{

}

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM