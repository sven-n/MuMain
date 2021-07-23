// NewUICommandWindow.cpp: implementation of the CNewUICommandWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUICommandWindow.h"
#include "NewUISystem.h"
#include "wsclientinline.h"
#include "ZzzInterface.h"
#include "PersonalShopTitleImp.h"
#include "UIGuildInfo.h"
#include "ZzzAI.h"
#include "w_CursedTemple.h"
#ifdef YDG_ADD_NEW_DUEL_SYSTEM
#include "DuelMgr.h"
#endif	// YDG_ADD_NEW_DUEL_SYSTEM

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUICommandWindow::CNewUICommandWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
	m_iCurSelectCommand = COMMAND_NONE;
	m_iCurMouseCursor = CURSOR_NORMAL;
#ifdef BUGFIX_UI_COMMAND_CHAR_ON_MOUSE
	m_bSelectedChar = false;
	m_bCanCommand = false;
#endif // BUGFIX_UI_COMMAND_CHAR_ON_MOUSE
}

SEASON3B::CNewUICommandWindow::~CNewUICommandWindow()
{
	Release();
}


//---------------------------------------------------------------------------------------------
// Create
bool SEASON3B::CNewUICommandWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_COMMAND, this );		// 인터페이스 오브젝트 등록
	
	SetPos(x, y);
	
	LoadImages();
	
	InitButtons();
	
	Show( false );
	
	return true;
}

//---------------------------------------------------------------------------------------------
// Release
void SEASON3B::CNewUICommandWindow::Release()
{
	UnloadImages();
	
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

//---------------------------------------------------------------------------------------------
// InitButtons
void SEASON3B::CNewUICommandWindow::InitButtons( )
{
	unicode::t_char szText[256] = {NULL, };
	unicode::_sprintf( szText, GlobalText[927], "D");
	
	// Exit Button Initialize
	m_BtnExit.ChangeButtonImgState( true, IMAGE_COMMAND_BASE_WINDOW_BTN_EXIT );
	m_BtnExit.ChangeButtonInfo( m_Pos.x+13, m_Pos.y+392, 36, 29 );		
	m_BtnExit.ChangeToolTipText( szText, true );	// "닫기 (D)"
	
	// Command 버튼 초기화
	for(int i=COMMAND_TRADE ; i<COMMAND_END ; i++)
	{
		m_BtnCommand[i].ChangeButtonImgState( true, IMAGE_COMMAND_BTN, true );
		m_BtnCommand[i].ChangeButtonInfo( m_Pos.x+(COMMAND_WINDOW_WIDTH/2-108/2), 
			(m_Pos.y+33)+(i*(29+COMMAND_BTN_INTERVAL_SIZE)), 108, 29 );
	}
	
	// Command 버튼 Text설정
	m_BtnCommand[COMMAND_TRADE].ChangeText( GlobalText[943] );			// "거래"
	m_BtnCommand[COMMAND_PURCHASE].ChangeText(GlobalText[1124] );		// "구매"
	m_BtnCommand[COMMAND_PARTY].ChangeText( GlobalText[944] );			// "파티"
	m_BtnCommand[COMMAND_WHISPER].ChangeText( GlobalText[945] );		// "귓속말"
	m_BtnCommand[COMMAND_GUILD].ChangeText( GlobalText[946] );			// "길드"
	m_BtnCommand[COMMAND_GUILDUNION].ChangeText( GlobalText[1352] );	// "길드연합"
	m_BtnCommand[COMMAND_RIVAL].ChangeText( GlobalText[1321] );			// "적대길드"
	m_BtnCommand[COMMAND_RIVALOFF].ChangeText( GlobalText[1322] );		// "적대길드해제"
	m_BtnCommand[COMMAND_ADD_FRIEND].ChangeText( GlobalText[947] );		// "친구추가"
	m_BtnCommand[COMMAND_FOLLOW].ChangeText( GlobalText[948] );			// "따라가기"
	m_BtnCommand[COMMAND_BATTLE].ChangeText( GlobalText[949] );			// "결투"
#ifndef KJH_DEL_PC_ROOM_SYSTEM			// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM
	m_BtnCommand[COMMAND_PCBANG].ChangeText( GlobalText[2325] );		// "PC방 포인트 상점"
#ifdef ASG_PCROOM_POINT_SYSTEM_MODIFY
	m_BtnCommand[COMMAND_PCBANG].ChangeTextColor(RGBA(255, 128, 255, 255));
#endif	// ASG_PCROOM_POINT_SYSTEM_MODIFY
#endif // ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM
	
}

//---------------------------------------------------------------------------------------------
// OpenningProcess
void SEASON3B::CNewUICommandWindow::OpenningProcess()
{	
	if( m_iCurSelectCommand != COMMAND_NONE )
		SetBtnState( m_iCurSelectCommand, false );		// 버튼 눌림상태 되돌리기
	
	m_iCurSelectCommand = COMMAND_NONE;
	m_iCurMouseCursor = CURSOR_NORMAL;
}

//---------------------------------------------------------------------------------------------
// ClosingProcess
void SEASON3B::CNewUICommandWindow::ClosingProcess()
{
	if( m_iCurSelectCommand != COMMAND_NONE )
		SetBtnState( m_iCurSelectCommand, false );		// 버튼 눌림상태 되돌리기
	
	m_iCurSelectCommand = COMMAND_NONE;
	m_iCurMouseCursor = CURSOR_NORMAL;
}

//---------------------------------------------------------------------------------------------
// BtnProcess
bool SEASON3B::CNewUICommandWindow::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };
	
	// Exit1 버튼 (기본처리)
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
#ifdef KJH_FIX_WOPS_K27165_COMMANDWINDOW_EXIT_BTNPROCESS
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_COMMAND);
#else // KJH_FIX_WOPS_K27165_COMMANDWINDOW_EXIT_BTNPROCESS
#ifdef LDK_FIX_COMMANDWINDOW_EXIT_BTNPROCESS
		//커맨드창 닫기 버튼으로 닫을경우 파티리스트윈도우 위치 이상 (2008.5.15)
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_COMMAND);
#endif //LDK_FIX_COMMANDWINDOW_EXIT_BTNPROCESS
		Show( false );
#endif // KJH_FIX_WOPS_K27165_COMMANDWINDOW_EXIT_BTNPROCESS
		PlayBuffer(SOUND_CLICK01);
		return true;
	}
	
	// Exit2 버튼 
	if(m_BtnExit.UpdateMouseEvent() == true)
	{
#ifdef KJH_FIX_WOPS_K27165_COMMANDWINDOW_EXIT_BTNPROCESS
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_COMMAND);
#else // KJH_FIX_WOPS_K27165_COMMANDWINDOW_EXIT_BTNPROCESS
#ifdef LDK_FIX_COMMANDWINDOW_EXIT_BTNPROCESS
		//커맨드창 닫기 버튼으로 닫을경우 파티리스트윈도우 위치 이상 (2008.5.15)
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_COMMAND);
#endif //LDK_FIX_COMMANDWINDOW_EXIT_BTNPROCESS
		Show( false );
#endif // KJH_FIX_WOPS_K27165_COMMANDWINDOW_EXIT_BTNPROCESS
		PlayBuffer(SOUND_CLICK01);
		return true;
	}
	
	for(int i=COMMAND_TRADE ; i<COMMAND_END ; i++)
	{
		if( m_BtnCommand[i].UpdateMouseEvent() == true )
		{
#ifndef KJH_DEL_PC_ROOM_SYSTEM			// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM
			if( i == COMMAND_PCBANG )
			{
#ifdef ASG_PCROOM_POINT_SYSTEM_MODIFY
				return CommandPCBangPoint();
#else	// ASG_PCROOM_POINT_SYSTEM_MODIFY
				if( CommandPCBangPoint() )
				{
					return true;
				}

				return false;
#endif	// ASG_PCROOM_POINT_SYSTEM_MODIFY
			}
#endif //ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM

			if( m_iCurSelectCommand != COMMAND_NONE )
				SetBtnState( m_iCurSelectCommand, false );		// 버튼 눌림상태 되돌리기

			if( g_CursedTemple->GetInterfaceState( static_cast<int>(SEASON3B::INTERFACE_COMMAND), i ) )
			{
				m_iCurSelectCommand = i;
				SetBtnState( m_iCurSelectCommand, true );		// 버튼 눌림상태 유지
			}

#ifndef KJH_DEL_PC_ROOM_SYSTEM			// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM
			//if( m_iCurSelectCommand != COMMAND_PCBANG )
#endif // ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM
				
		}
	}
	
	return false;
}

//---------------------------------------------------------------------------------------------
// UpdateMouseEvent
bool SEASON3B::CNewUICommandWindow::UpdateMouseEvent()
{
	// 버튼 처리
	if( true == BtnProcess() )	// 처리가 완료 되었다면
		return false;
	
	// 커맨드 창 내 영역 클릭시 하위 UI처리 및 이동 불가
	if( CheckMouseIn(m_Pos.x, m_Pos.y, COMMAND_WINDOW_WIDTH, COMMAND_WINDOW_HEIGHT) )
	{
		SetMouseCursor( CURSOR_NORMAL );
		return false;
	}
	else
	{
		if( m_iCurSelectCommand != COMMAND_NONE )
			SetMouseCursor( CURSOR_IDSELECT );
	}
	
	return true;
}

//---------------------------------------------------------------------------------------------
// UpdateKeyEvent
bool SEASON3B::CNewUICommandWindow::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_COMMAND) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_COMMAND);
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}
	return true;
}

//---------------------------------------------------------------------------------------------
// Update
bool SEASON3B::CNewUICommandWindow::Update()
{
	if( IsVisible() )
	{	
		SelectCommand( );
		RunCommand( );
	}	
	
	return true;
}

//---------------------------------------------------------------------------------------------
// Render
bool SEASON3B::CNewUICommandWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetTextColor( 255, 255, 255, 255 );
	g_pRenderText->SetBgColor( 0, 0, 0, 0 );	
	
	// Base Window
	RenderBaseWindow( );
	
#ifndef KJH_DEL_PC_ROOM_SYSTEM			// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM
#ifndef ASG_PCROOM_POINT_SYSTEM_MODIFY		// 정리시 삭제.
	// PC방 전용 상점 버튼.
	// PC방이 아니거나, 안전지대가 아니면 Button Lock
	if ( !CPCRoomPtSys::Instance().IsPCRoom() || !Hero->SafeZone )
	{
		m_BtnCommand[COMMAND_PCBANG].Lock();
		m_BtnCommand[COMMAND_PCBANG].ChangeTextColor( RGBA(128, 64, 128, 255) );
	}
	else
	{
		m_BtnCommand[COMMAND_PCBANG].UnLock();
		m_BtnCommand[COMMAND_PCBANG].ChangeTextColor( RGBA(255, 128, 255, 255) );
	}
#endif	// ASG_PCROOM_POINT_SYSTEM_MODIFY	// 정리시 삭제.
#endif // ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM
	
	for(int i=COMMAND_TRADE ; i<COMMAND_END ; i++)
	{	
		m_BtnCommand[i].SetFont( g_hFont );	
		if( m_iCurSelectCommand != COMMAND_NONE )
			m_BtnCommand[m_iCurSelectCommand].SetFont( g_hFontBold );
		
		m_BtnCommand[i].Render( );
	}
	
	// UI제목
	g_pRenderText->SetFont( g_hFontBold );
	g_pRenderText->RenderText(m_Pos.x+60, m_Pos.y+12, GlobalText[938], 72, 0, RT3_SORT_CENTER);		// 938 "커맨드 창"
	
	// 캐릭터 아이디 Render
#ifdef BUGFIX_UI_COMMAND_CHAR_ON_MOUSE
	// 마우스커서가 명령상태이고, 캐릭터에 온마우스가 되어있는상태
	if( (m_iCurMouseCursor == CURSOR_IDSELECT) && (m_bSelectedChar == true) )
#else // BUGFIX_UI_COMMAND_CHAR_ON_MOUSE				// 정리할 때 지워야 하는 소스
	if ( SelectedCharacter>=0 && m_iCurMouseCursor == CURSOR_IDSELECT )
#endif // BUGFIX_UI_COMMAND_CHAR_ON_MOUSE				// 정리할 때 지워야 하는 소스
    {
        CHARACTER* c = &CharactersClient[SelectedCharacter];
        if ( c!=NULL && c->Object.Kind==KIND_PLAYER && c!=Hero && (c->Object.Type==MODEL_PLAYER || c->Change ) )
        {
            int Width=128;int Height=32;int x=(float)MouseX+5;int y=(float)MouseY+5;
			RenderBitmap( BITMAP_COMMAND_WINDOW_BEGIN, x, y, Width, Height);
			
			g_pRenderText->SetFont( g_hFontBig );
#ifdef BUGFIX_UI_COMMAND_CHAR_ON_MOUSE
			if( m_bCanCommand )
			{
				// 명령을 사용할 수 있는 거리에 있다. (ID흰색)
				g_pRenderText->SetTextColor(255, 255, 255, 255);
			} // if()
			else
			{
				// 명령을 사용할 수 없는 거리에 있다. (ID빨간색)
				g_pRenderText->SetTextColor(255, 0, 0, 255);
			} // else
#else // BUGFIX_UI_COMMAND_CHAR_ON_MOUSE			// 정리할 때 지워야 하는 소스
			g_pRenderText->SetTextColor(255, 255, 255, 255);
#endif // BUGFIX_UI_COMMAND_CHAR_ON_MOUSE			// 정리할 때 지워야 하는 소스
			g_pRenderText->SetBgColor(20, 20, 20, 0);
			g_pRenderText->RenderText(MouseX+5+64, MouseY+12, c->ID, 0, 0, RT3_WRITE_CENTER);
			
			g_pRenderText->SetFont( g_hFont );
        }
	}

	m_BtnExit.Render( );			// Exit Button
	
	DisableAlphaBlend();
	
	return true;
}

//---------------------------------------------------------------------------------------------
// RenderBaseWindow
void SEASON3B::CNewUICommandWindow::RenderBaseWindow( )
{
	RenderImage(IMAGE_COMMAND_BASE_WINDOW_BACK, m_Pos.x, m_Pos.y, 
		float(COMMAND_WINDOW_WIDTH), float(COMMAND_WINDOW_HEIGHT));
	RenderImage(IMAGE_COMMAND_BASE_WINDOW_TOP, m_Pos.x, m_Pos.y, 
		float(COMMAND_WINDOW_WIDTH), 64.f);
	RenderImage(IMAGE_COMMAND_BASE_WINDOW_LEFT, m_Pos.x, m_Pos.y+64.f, 
		21.f, float(COMMAND_WINDOW_HEIGHT)-64.f-45.f);
	RenderImage(IMAGE_COMMAND_BASE_WINDOW_RIGHT, m_Pos.x+float(COMMAND_WINDOW_WIDTH)-21.f, 
		m_Pos.y+64.f, 21.f, float(COMMAND_WINDOW_HEIGHT)-64.f-45.f);
	RenderImage(IMAGE_COMMAND_BASE_WINDOW_BOTTOM, m_Pos.x, m_Pos.y+float(COMMAND_WINDOW_HEIGHT)-45.f, 
		float(COMMAND_WINDOW_WIDTH), 45.f);
}

//---------------------------------------------------------------------------------------------
// SetPos
void SEASON3B::CNewUICommandWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
	
	m_BtnExit.ChangeButtonInfo( m_Pos.x+13, m_Pos.y+392, 36, 29 );	
	for(int i=COMMAND_TRADE ; i<COMMAND_END ; i++)
	{
		m_BtnCommand[i].ChangeButtonInfo( m_Pos.x+(COMMAND_WINDOW_WIDTH/2-108/2), 
			(m_Pos.y+33)+((i-1)*(29+COMMAND_BTN_INTERVAL_SIZE)), 108, 29 );
	}
}

//---------------------------------------------------------------------------------------------
// GetLayerDepth
float SEASON3B::CNewUICommandWindow::GetLayerDepth()
{
	return 4.6f;
}

//---------------------------------------------------------------------------------------------
// RunCommand
void SEASON3B::CNewUICommandWindow::RunCommand()
{ 
	if( MouseLButtonPush && m_iCurMouseCursor != CURSOR_IDSELECT )
		SetMouseCursor( CURSOR_PUSH );
	
	if ( m_iCurSelectCommand == COMMAND_NONE)
		return;

	int Selectindex = -1;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	CHARACTER* pSelectedCha = NULL;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	CHARACTER* pSelectedCha;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int distX, distY;
#ifdef BUGFIX_UI_COMMAND_CHAR_ON_MOUSE
	m_bSelectedChar = false;
	m_bCanCommand = false;
#endif // BUGFIX_UI_COMMAND_CHAR_ON_MOUSE
	
	if( SelectedCharacter >= 0)
	{
		pSelectedCha = &CharactersClient[SelectedCharacter];
#ifdef BUGFIX_UI_COMMAND_CHAR_ON_MOUSE
		m_bSelectedChar = true;
#endif // BUGFIX_UI_COMMAND_CHAR_ON_MOUSE
		if( pSelectedCha!=NULL ) 
		{
			distX = abs((pSelectedCha->PositionX) - (Hero->PositionX));
			distY = abs((pSelectedCha->PositionY) - (Hero->PositionY));
			if ( pSelectedCha->Object.Kind==KIND_PLAYER && pSelectedCha!=Hero && (pSelectedCha->Object.Type==MODEL_PLAYER||pSelectedCha->Change) && ( distX<=MAX_DISTANCE_TILE && distY<=MAX_DISTANCE_TILE ) )
			{
#ifdef KJH_FIX_WOPS_K22844_CHRISTMAS_TRANSFORM_RING_SUMMON_TO_COMMAND
				// 명령어가 적용되지 말아야 할 캐릭터들
				if( (pSelectedCha->Object.SubType != MODEL_XMAS_EVENT_CHA_DEER)
					&& (pSelectedCha->Object.SubType != MODEL_XMAS_EVENT_CHA_SNOWMAN)
					&& (pSelectedCha->Object.SubType != MODEL_XMAS_EVENT_CHA_SSANTA)
					)
				{
					Selectindex = SelectedCharacter;
#ifdef BUGFIX_UI_COMMAND_CHAR_ON_MOUSE
					m_bCanCommand = true;
#endif // BUGFIX_UI_COMMAND_CHAR_ON_MOUSE
				}
#else // KJH_FIX_WOPS_K22844_CHRISTMAS_TRANSFORM_RING_SUMMON_TO_COMMAND			// 정리할 때 지워야 하는 소스
				Selectindex = SelectedCharacter;
#ifdef BUGFIX_UI_COMMAND_CHAR_ON_MOUSE
				m_bCanCommand = true;
#endif // BUGFIX_UI_COMMAND_CHAR_ON_MOUSE
#endif // KJH_FIX_WOPS_K22844_CHRISTMAS_TRANSFORM_RING_SUMMON_TO_COMMAND		// 정리할 때 지워야 하는 소스
			}
		}
	}
	
	if ( MouseRButtonPush )
    {		
		MouseRButtonPush = false;
        MouseRButton     = false; 
		
		SetMouseCursor( CURSOR_NORMAL );	
		
		if( Selectindex >= 0 )
		{	
			switch( m_iCurSelectCommand )
			{
				
			case COMMAND_TRADE:		// 거래
				{
					CommandTrade( pSelectedCha );
				}break;
				
			case COMMAND_PURCHASE:	// 구매
				{
					CommandPurchase( pSelectedCha );
				}break;
					
			case COMMAND_PARTY:		// 파티
				{
					CommandParty( pSelectedCha->Key );	
				}break;
				
			case COMMAND_WHISPER:   //  귓속말.
				{
					CommandWhisper( pSelectedCha );
				}break;
				
			case COMMAND_GUILD:			// 길드
				{
					CommandGuild( pSelectedCha );
				}break;

			case COMMAND_GUILDUNION:		// 길드연합
				{
					CommandGuildUnion( pSelectedCha );
				}break;
				
			case COMMAND_RIVAL:			// 적대길드
				{
					CommandGuildRival( pSelectedCha );
				}break;
				
			case COMMAND_RIVALOFF:		// 적대길드해제
				{
					CommandCancelGuildRival( pSelectedCha );
				}break;
				
			case COMMAND_ADD_FRIEND:		// 친구추가
				{
					CommandAddFriend( pSelectedCha );
				}break;
				
			case COMMAND_FOLLOW:			//  따라가기
				{	
					CommandFollow(Selectindex);
				}break;
				
			case COMMAND_BATTLE :			//  결투
				{	
					CommandDual( pSelectedCha );
				}break;
				
#ifndef KJH_DEL_PC_ROOM_SYSTEM			// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM
			case COMMAND_PCBANG:
				{	
					CommandPCBangPoint();
				}break;
#endif // ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM
				
			}
		}
		if( m_iCurSelectCommand != COMMAND_NONE )
			SetBtnState( m_iCurSelectCommand, false );
		m_iCurSelectCommand = COMMAND_NONE;
		
	}
}

void SEASON3B::CNewUICommandWindow::SetBtnState(int iBtnType, bool bStateDown)
{
	if(bStateDown)
	{
		m_BtnCommand[iBtnType].UnRegisterButtonState();
		m_BtnCommand[iBtnType].RegisterButtonState(BUTTON_STATE_UP, IMAGE_COMMAND_BTN, 2);
		m_BtnCommand[iBtnType].RegisterButtonState(BUTTON_STATE_OVER, IMAGE_COMMAND_BTN, 2);
		m_BtnCommand[iBtnType].RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_COMMAND_BTN, 2);
		m_BtnCommand[iBtnType].ChangeImgIndex(IMAGE_COMMAND_BTN, 2);	
	}
	else
	{
		m_BtnCommand[iBtnType].UnRegisterButtonState();
		m_BtnCommand[iBtnType].RegisterButtonState(BUTTON_STATE_UP, IMAGE_COMMAND_BTN, 0);
		m_BtnCommand[iBtnType].RegisterButtonState(BUTTON_STATE_OVER, IMAGE_COMMAND_BTN, 1);
		m_BtnCommand[iBtnType].RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_COMMAND_BTN, 2);
		m_BtnCommand[iBtnType].ChangeImgIndex(IMAGE_COMMAND_BTN, 0);
	}	
}

//---------------------------------------------------------------------------------------------
// SelectCommand
void SEASON3B::CNewUICommandWindow::SelectCommand()
{
	
}

int SEASON3B::CNewUICommandWindow::GetCurCommandType()
{
	return m_iCurSelectCommand;
}

void SEASON3B::CNewUICommandWindow::SetMouseCursor( int iCursorType )
{
	m_iCurMouseCursor = iCursorType;
}

int SEASON3B::CNewUICommandWindow::GetMouseCursor()
{
	return m_iCurMouseCursor;
}

//---------------------------------------------------------------------------------------------
// LoadImages
void SEASON3B::CNewUICommandWindow::LoadImages()
{
	// 기본창
	LoadBitmap( "Interface\\newui_msgbox_back.jpg", IMAGE_COMMAND_BASE_WINDOW_BACK, GL_LINEAR );
	LoadBitmap( "Interface\\newui_item_back01.tga", IMAGE_COMMAND_BASE_WINDOW_TOP, GL_LINEAR );
	LoadBitmap( "Interface\\newui_item_back02-L.tga", IMAGE_COMMAND_BASE_WINDOW_LEFT, GL_LINEAR );
	LoadBitmap( "Interface\\newui_item_back02-R.tga", IMAGE_COMMAND_BASE_WINDOW_RIGHT, GL_LINEAR );
	LoadBitmap( "Interface\\newui_item_back03.tga", IMAGE_COMMAND_BASE_WINDOW_BOTTOM, GL_LINEAR );
	LoadBitmap( "Interface\\newui_exit_00.tga", IMAGE_COMMAND_BASE_WINDOW_BTN_EXIT, GL_LINEAR );		// Exit Button
	LoadBitmap( "Interface\\newui_btn_empty.tga", IMAGE_COMMAND_BTN, GL_LINEAR );
	LoadBitmap( "Interface\\newui_cursorid_wnd.jpg", IMAGE_COMMAND_SELECTID_BG, GL_LINEAR );
}

//---------------------------------------------------------------------------------------------
// UnloadImages
void SEASON3B::CNewUICommandWindow::UnloadImages()
{
	// 기본창
	DeleteBitmap( IMAGE_COMMAND_BASE_WINDOW_BACK );
	DeleteBitmap( IMAGE_COMMAND_BASE_WINDOW_TOP );
	DeleteBitmap( IMAGE_COMMAND_BASE_WINDOW_LEFT );
	DeleteBitmap( IMAGE_COMMAND_BASE_WINDOW_RIGHT );
	DeleteBitmap( IMAGE_COMMAND_BASE_WINDOW_BOTTOM );
	DeleteBitmap( IMAGE_COMMAND_BASE_WINDOW_BTN_EXIT );	// Exit Button
	DeleteBitmap( IMAGE_COMMAND_BTN );
	DeleteBitmap( IMAGE_COMMAND_SELECTID_BG );
}


//---------------------------------------------------------------------------------------------
// CommandTrade (거래 명령)
bool SEASON3B::CNewUICommandWindow::CommandTrade(CHARACTER* pSelectedCha)
{
	if(pSelectedCha == NULL)
		return false;

	int level = CharacterAttribute->Level;
	
	//  거래 레벨 제한.
	if( level < TRADELIMITLEVEL )
	{
		//  478 "레벨6부터 거래 신청이 가능합니다."
		g_pChatListBox->AddText("", GlobalText[478], SEASON3B::TYPE_SYSTEM_MESSAGE);
		return false;
	}
	if(IsShopInViewport(pSelectedCha)) 
	{
		// 493 "거래할 수 없는 상황입니다."
		g_pChatListBox->AddText("", GlobalText[493], SEASON3B::TYPE_ERROR_MESSAGE);
		return false;
	}
		
	SendRequestTrade ( pSelectedCha->Key );

	return true;
}

//---------------------------------------------------------------------------------------------
// CommandPurchase (구매 명령)
bool SEASON3B::CNewUICommandWindow::CommandPurchase(CHARACTER* pSelectedCha)
{
	if(pSelectedCha == NULL)
		return false;

	SendRequestOpenPersonalShop(pSelectedCha->Key, pSelectedCha->ID);

	return true;
}

//---------------------------------------------------------------------------------------------
// CommandParty (파티 명령)
bool SEASON3B::CNewUICommandWindow::CommandParty(SHORT iChaKey)
{
	if( PartyNumber>0 && strcmp( Party[0].Name, Hero->ID )!=NULL )
	{
		g_pChatListBox->AddText("", GlobalText[257], SEASON3B::TYPE_ERROR_MESSAGE);
		return false;
	}
	
	SendRequestParty(iChaKey);
	
	return true;
}

//---------------------------------------------------------------------------------------------
// CommandWhisper (귓속말 명령)
bool SEASON3B::CNewUICommandWindow::CommandWhisper(CHARACTER* pSelectedCha)
{
	g_pChatInputBox->SetWhsprID(pSelectedCha->ID);

	return true;
}

//---------------------------------------------------------------------------------------------
// CommandGuild (길드가입 명령)
bool SEASON3B::CNewUICommandWindow::CommandGuild(CHARACTER* pSelectedChar)
{
	// 255 "이미 길드에 속해 있습니다."
	if( Hero->GuildStatus != G_NONE )
	{
		g_pChatListBox->AddText("", GlobalText[255], SEASON3B::TYPE_SYSTEM_MESSAGE);
		return false;
	}
	// 507 "상대방이 길드마스터가 아닙니다."
	if( (pSelectedChar->GuildMarkIndex < 0) || (pSelectedChar->GuildStatus != G_MASTER) )
	{
		g_pChatListBox->AddText("", GlobalText[507], SEASON3B::TYPE_SYSTEM_MESSAGE);
		return false;
	}
	
	SendRequestGuild( pSelectedChar->Key );

	return true;
}

//---------------------------------------------------------------------------------------------
// CommandGuild (길드연합 명령)
bool SEASON3B::CNewUICommandWindow::CommandGuildUnion(CHARACTER* pSelectedCha)
{
	// 1320 "길드마스터가 아닙니다."
	if( Hero->GuildStatus != G_MASTER )
	{
		g_pChatListBox->AddText("", GlobalText[1320], SEASON3B::TYPE_SYSTEM_MESSAGE);
		return false;
	}
	// 1385 "길드에 속한 대상이 아닙니다."
	if( pSelectedCha->GuildStatus == G_NONE )
	{
		g_pChatListBox->AddText("", GlobalText[1385], SEASON3B::TYPE_SYSTEM_MESSAGE);
		return false;
	}
	// 507 "상대방이 길드마스터가 아닙니다."
	if( pSelectedCha->GuildStatus != G_MASTER )
	{
		g_pChatListBox->AddText("", GlobalText[507], SEASON3B::TYPE_SYSTEM_MESSAGE);
		return false;
	}	
	if( pSelectedCha->GuildStatus == G_MASTER )
	{
		SendRequestGuildRelationShip( 0x01, 0x01, HIBYTE(pSelectedCha->Key), LOBYTE(pSelectedCha->Key) );
		return true;
	}

	return false;
}

//---------------------------------------------------------------------------------------------
// CommandGuildRival (적대길드 명령)
bool SEASON3B::CNewUICommandWindow::CommandGuildRival(CHARACTER* pSelectedCha)
{
	// 1320 "길드마스터가 아닙니다."
	if( Hero->GuildStatus != G_MASTER )
	{
		g_pChatListBox->AddText("", GlobalText[1320], SEASON3B::TYPE_SYSTEM_MESSAGE);
		return false;
	}
	
	// 507 "상대방이 길드마스터가 아닙니다."
	if( pSelectedCha->GuildStatus != G_MASTER )
	{
		g_pChatListBox->AddText("", GlobalText[507], SEASON3B::TYPE_SYSTEM_MESSAGE);
		return false;
	}
	
	// 					// ???? "라이벌이 아닙니다." 가 표시된다.
	// 					else if( ???? )
	// 						m_bActiveCmd[COMMAND_RIVALOFF] = 0;
	// 					else
	// 						m_bActiveCmd[COMMAND_RIVALOFF] = 1;
	// 			
	
	SendRequestGuildRelationShip( 0x02, 0x01, HIBYTE(pSelectedCha->Key), LOBYTE(pSelectedCha->Key) );
	
	return true;
}

//---------------------------------------------------------------------------------------------
// CommandCancelGuildRival (적대길드해제 명령)
bool SEASON3B::CNewUICommandWindow::CommandCancelGuildRival(CHARACTER* pSelectedCha)
{
	// 1320 "길드마스터가 아닙니다."
	if( Hero->GuildStatus != G_MASTER )
	{
		g_pChatListBox->AddText("", GlobalText[1320], SEASON3B::TYPE_SYSTEM_MESSAGE);
		return false;
	}
	// 507 "상대방이 길드마스터가 아닙니다."
	if( pSelectedCha->GuildStatus != G_MASTER )
	{
		g_pChatListBox->AddText("", GlobalText[507], SEASON3B::TYPE_SYSTEM_MESSAGE);
		return false;
	}
	// 					// ???? "라이벌이 아닙니다." 가 표시된다.
	// 					else if( ???? )
	// 						m_bActiveCmd[COMMAND_RIVALOFF] = 0;
	// 					else
	// 						m_bActiveCmd[COMMAND_RIVALOFF] = 1;
	// 	
	
	// 무릎꿇기
	SetAction( &Hero->Object, PLAYER_RESPECT1 );
	SendRequestAction( AT_RESPECT1, ((BYTE)((Hero->Object.Angle[2]+22.5f)/360.f*8.f+1.f)%8) );
	SendRequestGuildRelationShip( 0x02, 0x02, HIBYTE(pSelectedCha->Key), LOBYTE(pSelectedCha->Key) );
	
	return true;
}

//---------------------------------------------------------------------------------------------
// CommandAddFriend (친구추가 명령)
bool SEASON3B::CNewUICommandWindow::CommandAddFriend(CHARACTER* pSelectedCha)
{
	if ( g_pWindowMgr->IsServerEnable() == TRUE )
	{
		SendRequestAddFriend(pSelectedCha->ID);
		return true;
	}

	return false;
}

//---------------------------------------------------------------------------------------------
// CommandFollow (따라가기 명령)
bool SEASON3B::CNewUICommandWindow::CommandFollow(int iSelectedChaIndex)
{
	if(iSelectedChaIndex < 0)
	{
		return false;
	}

	g_iFollowCharacter = iSelectedChaIndex;
	
	return true;
}

//---------------------------------------------------------------------------------------------
// CommandCancelGuildRival (결투 명령)
int SEASON3B::CNewUICommandWindow::CommandDual(CHARACTER* pSelectedCha)
{
#ifdef YDG_ADD_NEW_DUEL_UI
	int iLevel = CharacterAttribute->Level;
	if(iLevel < 30)	// 레벨제한
	{
		char szError[48] = "";
		sprintf(szError, GlobalText[2704], 30);	// "%d레벨 이상의 캐릭터만 결투가 가능합니다."
		g_pChatListBox->AddText("", szError, SEASON3B::TYPE_ERROR_MESSAGE);				
		return 3;
	}
#ifdef YDG_ADD_DOPPELGANGER_EVENT
	else if (World >= WD_65DOPPLEGANGER1 && World <= WD_68DOPPLEGANGER4)
	{
		// 도플갱어 맵에서는 결투 신청을 할 수 없다
		g_pChatListBox->AddText("", GlobalText[2866], SEASON3B::TYPE_ERROR_MESSAGE);	// "해당 지역에서 결투는 불가능합니다."
		return 3;
	}
#endif	// YDG_ADD_DOPPELGANGER_EVENT
#ifdef LDS_ADD_UI_UNITEDMARKETPLACE
	else if (World == WD_79UNITEDMARKETPLACE)
	{
		// 통합시장 맵 내에서는 결투 신청 할 수 없다
		g_pChatListBox->AddText("", GlobalText[3063], SEASON3B::TYPE_ERROR_MESSAGE);	// "로랜시장 내에서는 결투 신청을 할 수 없습니다."
		return 3;
	}
#endif // LDS_ADD_UI_UNITEDMARKETPLACE
	else if ( !g_DuelMgr.IsDuelEnabled() )	// 자신이 결투중이 아닐경우 결투가능
	{
		SendRequestDuelStart ( pSelectedCha->Key, pSelectedCha->ID );
		return 1;
	}
	else if ( g_DuelMgr.IsDuelEnabled() && g_DuelMgr.IsDuelPlayer(pSelectedCha, DUEL_ENEMY))
	{
		SendRequestDuelEnd ();
		return 2;
	}
	else    //  본인이 결투중임.
	{
		g_pChatListBox->AddText("", GlobalText[915], SEASON3B::TYPE_ERROR_MESSAGE);
		return 3;
	}
#else	// YDG_ADD_NEW_DUEL_UI
#ifdef DUEL_SYSTEM
	if ( !g_bEnableDuel )	// 자신이 결투중이 아닐경우 결투가능
	{
		SendRequestDuelStart ( pSelectedCha->Key, pSelectedCha->ID );
		return 1;
	}
	else if ( g_bEnableDuel==true && strcmp( pSelectedCha->ID, g_szDuelPlayerID )==NULL )
	{
		SendRequestDuelEnd ();
		return 2;
	}
	else    //  본인이 결투중임.
	{
		g_pChatListBox->AddText("", GlobalText[915], SEASON3B::TYPE_ERROR_MESSAGE);
		return 3;
	}
	
#endif// DUEL_SYSTEM
#endif	// YDG_ADD_NEW_DUEL_UI

	// return 1 : DualStart 성공
	// return 2 : DualEnd 성공
	// return 3 : Dual 중임
	// return 0 : 실패 (Dual System이 아님)
	
	return 0;
}

#ifndef KJH_DEL_PC_ROOM_SYSTEM			// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM
//---------------------------------------------------------------------------------------------
// CommandPCBangPoint (PC방 포인트 상점 명령)
bool SEASON3B::CNewUICommandWindow::CommandPCBangPoint()
{
#ifndef ASG_PCROOM_POINT_SYSTEM_MODIFY		// 정리시 삭제.
	if (Hero->SafeZone)	// 안전지대인가?
	{
#endif	// ASG_PCROOM_POINT_SYSTEM_MODIFY	// 정리시 삭제.
		PlayBuffer(SOUND_CLICK01);
		PlayBuffer(SOUND_INTERFACE01);
		MouseUpdateTime = 0;
		MouseUpdateTimeMax = 6;
		
		SendRequestPCRoomPointShopOpen();	// 서버에 상점 열기 요청.
		return true;
#ifndef ASG_PCROOM_POINT_SYSTEM_MODIFY		// 정리시 삭제.
	}

	return false;
#endif	// ASG_PCROOM_POINT_SYSTEM_MODIFY	// 정리시 삭제.
}
#endif // ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM
