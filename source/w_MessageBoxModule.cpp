// w_MessageBoxModule.cpp: implementation of the MessageBoxModule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_Module.h"
#include "w_MessageBoxModule.h"

#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

#include "w_UIType.h"
#include "w_UIWnd.h"
#include "w_CompositedPanel.h"
#include "w_CustomButton.h"
#include "w_CustomCheckButton.h"
#include "w_Panel.h"
#include "w_Text.h"

#include "DSPlaySound.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MessageBoxModulePtr MessageBoxModule::Make( const MESSAGEBOXDATA& messageboxdata )
{
	MessageBoxModulePtr module( new MessageBoxModule );
	module->Initialize( messageboxdata, module );
	return module;
}

MessageBoxModule::MessageBoxModule() : Module( Module::eMessageBox )
{

}

MessageBoxModule::~MessageBoxModule()
{
	Destroy();
}

void MessageBoxModule::Initialize( const MESSAGEBOXDATA& messageboxdata, weak_ptr<MessageBoxModule> uiHandler )
{
	m_Handler = messageboxdata.s_handler;
	m_ID      = messageboxdata.s_id;

	Dim WindowSiz = TheShell().GetWindowSize();

	ui::UIFramePtr frame = ui::UIFrame::Make( UIMESSAGEBOXFRAME_NAME, true );
	frame->Move( Coord( 0, 0 ) );
	frame->Resize( WindowSiz );
	TheUISystem().AddFrame( frame );

	ui::PanelPtr messageframepanel( new ui::Panel("shopframpanel") );
	messageframepanel->SetColor( 0xff2C2C30 );
	messageframepanel->ChangeOpacityByPercent( 0.7f );
	messageframepanel->Move( Coord( 0, 0 ) );
	messageframepanel->Resize( frame->GetSize() );
	frame->AddChild( messageframepanel );

	ui::CompositedPanelPtr messageboxbackpanel( new ui::CompositedPanel( "messageboxback", ui::ePanel_Piece_One ) );
	messageboxbackpanel->SetImageIndex( BITMAP_MESSAGE_WIN );
	messageboxbackpanel->Move( Coord( (messageframepanel->GetSize().x/2)-(352/2), (messageframepanel->GetSize().y/2)-(113/2) ) );
	messageboxbackpanel->Resize( Dim( 352, 113 ) );
	messageframepanel->AddChild( messageboxbackpanel );
	
	int messagecount = 0, pos_y = 44;

	if( messageboxdata.s_message.size() != 1 )
	{
		pos_y -= messageboxdata.s_message.size()*6;
	}

	for ( list<string>::const_iterator iter = messageboxdata.s_message.begin(); iter != messageboxdata.s_message.end(); )
	{
		list<string>::const_iterator curiter = iter;
		++iter;
		const string& message = *curiter;

		ui::TextPtr messagetext( new ui::Text( "messagetext" ) );
		messagetext->Make( FONTSIZE_LANGUAGE, message.length() );
		messagetext->SetAlign( TA_LEFT );
		messagetext->SetContent( message );
		Coord textpos = Coord( (messageboxbackpanel->GetSize().x/2)-(messagetext->GetSize().x/2), pos_y+(17*messagecount));
		messagetext->Move( textpos );
		messageboxbackpanel->AddChild( messagetext );

		++messagecount;
	}

	if( messageboxdata.s_type == eMB_OK )
	{
		ui::CustomButtonPtr okbutton( new ui::CustomButton( "okbutton" ) );
		okbutton->MakeImage( BITMAP_NEWUI_SHOP_LEFTDISPLAYBT );
		okbutton->Move( Coord( 138, 75 ) );
		okbutton->Resize( Dim( 72, 30 ) );
		okbutton->SetCustomButtonName( "Ok", Coord( 0, 8 ), TA_LEFT );
		okbutton->RegisterHandler( ui::eEvent_LButtonClick, uiHandler, &MessageBoxModule::OnOkButtonClick );
		messageboxbackpanel->AddChild( okbutton );
	}
	else
	{
		ui::CustomButtonPtr okbutton( new ui::CustomButton( "okbutton" ) );
		okbutton->MakeImage( BITMAP_NEWUI_SHOP_LEFTDISPLAYBT );
		okbutton->Move( Coord( 96, 75 ) );
		okbutton->Resize( Dim( 72, 30 ) );
		if( messageboxdata.s_type == eMB_OKCANCEL )
		{
			okbutton->SetCustomButtonName( "Ok", Coord( 0, 8 ), TA_LEFT );
			okbutton->RegisterHandler( ui::eEvent_LButtonClick, uiHandler, &MessageBoxModule::OnOkButtonClick );
		}
		else
		{
			okbutton->SetCustomButtonName( "Yes", Coord( 0, 8 ), TA_LEFT );
			okbutton->RegisterHandler( ui::eEvent_LButtonClick, uiHandler, &MessageBoxModule::OnYesButtonClick );
		}
		messageboxbackpanel->AddChild( okbutton );

		ui::CustomButtonPtr cancelbutton( new ui::CustomButton( "cancelbutton" ) );
		cancelbutton->MakeImage( BITMAP_NEWUI_SHOP_LEFTDISPLAYBT );
		cancelbutton->Move( Coord( 187, 75 ) );
		cancelbutton->Resize( Dim( 72, 30 ) );
		if( messageboxdata.s_type == eMB_OKCANCEL )
		{
			cancelbutton->SetCustomButtonName( "Cancel", Coord( 0, 8 ), TA_LEFT );
			cancelbutton->RegisterHandler( ui::eEvent_LButtonClick, uiHandler, &MessageBoxModule::OnCloseButtonClick );
		}
		else
		{
			cancelbutton->SetCustomButtonName( "No", Coord( 0, 8 ), TA_LEFT );
			cancelbutton->RegisterHandler( ui::eEvent_LButtonClick, uiHandler, &MessageBoxModule::OnNoButtonClick );
		}
		messageboxbackpanel->AddChild( cancelbutton );
	}
}

void MessageBoxModule::Destroy()
{
}

void MessageBoxModule::OnOkButtonClick( ui::UIWnd& wnd )
{
	if( !m_Handler.expired() ) m_Handler.lock()->OnMessageBox_OK( m_ID );
	PlayBuffer(SOUND_CLICK01);
	MessageBoxClear;
}

void MessageBoxModule::OnCloseButtonClick( ui::UIWnd& wnd )
{
	if( !m_Handler.expired() ) m_Handler.lock()->OnMessageBox_CANCEL( m_ID );
	PlayBuffer(SOUND_CLICK01);
	MessageBoxClear;
}

void MessageBoxModule::OnYesButtonClick( ui::UIWnd& wnd )
{
	if( !m_Handler.expired() ) m_Handler.lock()->OnMessageBox_YES( m_ID );
	PlayBuffer(SOUND_CLICK01);
	MessageBoxClear;
}

void MessageBoxModule::OnNoButtonClick( ui::UIWnd& wnd )
{
	if( !m_Handler.expired() ) m_Handler.lock()->OnMessageBox_NO( m_ID );
	PlayBuffer(SOUND_CLICK01);
	MessageBoxClear;
}

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM