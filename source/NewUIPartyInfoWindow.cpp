// NewUIPartyInfo.cpp: implementation of the CNewUIPartyInfo class.
//
//////////////////////////////////////////////////////////////////////

// 차후에 수정
// 1. GetMapName() - 다른곳에서도 쓸 수 있도록 공통으로 호출할수 있는 곳에 있는것이 좋겠다.

#include "stdafx.h"

#include "NewUIPartyInfoWindow.h"
#include "NewUISystem.h"
#include "wsclientinline.h"
#include "GMBattleCastle.h"
#include "CSChaosCastle.h"
#include "GMHellas.h"
#include "GM3rdChangeUp.h"
#include "w_CursedTemple.h"


using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIPartyInfoWindow::CNewUIPartyInfoWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
	m_bParty = false;
}

CNewUIPartyInfoWindow::~CNewUIPartyInfoWindow()
{
	Release();
}

//---------------------------------------------------------------------------------------------
// Create
bool CNewUIPartyInfoWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_PARTY, this );		// 인터페이스 오브젝트 등록
	
	SetPos(x, y);
	
	LoadImages();
	
	InitButtons();
	
	Show( false );

	return true;
}

//---------------------------------------------------------------------------------------------
// Release
void CNewUIPartyInfoWindow::Release()
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
void CNewUIPartyInfoWindow::InitButtons( )
{
	// Exit Button Initialize
	m_BtnExit.ChangeButtonImgState( true, IMAGE_PARTY_BASE_WINDOW_BTN_EXIT );
	m_BtnExit.ChangeButtonInfo( m_Pos.x+13, m_Pos.y+392, 36, 29 );		
	m_BtnExit.ChangeToolTipText( GlobalText[221], true );	// "파티창 닫기(P)"
	
	// Exit Party Button Initialize
	for( int i=0 ; i<MAX_PARTYS ; i++ )
	{	
		int iVal = i*71;		// 인덱스에 따른 편차(y의 위치)
		m_BtnPartyExit[i].ChangeButtonImgState( true, IMAGE_PARTY_EXIT );
		m_BtnPartyExit[i].ChangeButtonInfo( m_Pos.x+159, m_Pos.y+63+iVal, 13, 13 );
	}
}

//---------------------------------------------------------------------------------------------
// OpenningProcess
void CNewUIPartyInfoWindow::OpenningProcess()
{
	SendRequestPartyList();
}

//---------------------------------------------------------------------------------------------
// ClosingProcess
void CNewUIPartyInfoWindow::ClosingProcess()
{

}

//---------------------------------------------------------------------------------------------
// BtnProcess
bool CNewUIPartyInfoWindow::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };
	
	// Exit1 버튼 (기본처리)
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_PARTY);
		return true;
	}
	
	// Exit2 버튼 
	if(m_BtnExit.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_PARTY);
		return true;
	}
	
	// 파티창 
	if( IsVisible() )		
	{
		for( int i=0 ; i<PartyNumber ; i++ )
		{	
			if( !strcmp( Party[0].Name, Hero->ID ) || !strcmp( Party[i].Name, Hero->ID ) )		// 자신이 파티장일때, 자신과 타인 강퇴
			{
				if( m_BtnPartyExit[i].UpdateMouseEvent() )
				{
					LeaveParty( i );
					return true;
				}
			}
		}
	}


	return false;
}

//---------------------------------------------------------------------------------------------
// UpdateMouseEvent
bool CNewUIPartyInfoWindow::UpdateMouseEvent()
{
	// 버튼 처리
	if( true == BtnProcess() )	// 처리가 완료 되었다면
		return false;
	
	// 파티 창 내 영역 클릭시 하위 UI처리 및 이동 불가
	if( CheckMouseIn(m_Pos.x, m_Pos.y, PARTY_INFO_WINDOW_WIDTH, PARTY_INFO_WINDOW_HEIGHT) )
		return false;

	return true;
}

//---------------------------------------------------------------------------------------------
// UpdateKeyEvent
bool CNewUIPartyInfoWindow::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PARTY) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_PARTY);
			PlayBuffer(SOUND_CLICK01);

			return false;
		}
	}

	return true;
}

//---------------------------------------------------------------------------------------------
// Update
bool CNewUIPartyInfoWindow::Update()
{
	if( IsVisible() )		// 파티중일때
	{
		// 정보 갱신
		/* 
			. 프레임마다 정보갱신은 무리임. 
			. 실시간으로 정보갱신 하려면, 서버팀과 상의 (예를들어 몇초당 한번 정보요청 하는방법)
			. 현재는 파티창을 열떄에만 정보요청
		*/
		//SendRequestPartyList();
		if( PartyNumber > 0)
			SetParty( true );
		else
			SetParty( false );
	}	


	return true;
}

//---------------------------------------------------------------------------------------------
// Render
bool CNewUIPartyInfoWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetTextColor(0xFFFFFFFF);

	DWORD dwPreBGColor = g_pRenderText->GetBgColor();
	g_pRenderText->SetBgColor( RGBA(0, 0, 0, 0) );
	
	// Base Window
	RenderImage(IMAGE_PARTY_BASE_WINDOW_BACK, m_Pos.x, m_Pos.y, 
						float(PARTY_INFO_WINDOW_WIDTH), float(PARTY_INFO_WINDOW_HEIGHT));
	RenderImage(IMAGE_PARTY_BASE_WINDOW_TOP, m_Pos.x, m_Pos.y, 
						float(PARTY_INFO_WINDOW_WIDTH), 64.f);
	RenderImage(IMAGE_PARTY_BASE_WINDOW_LEFT, m_Pos.x, m_Pos.y+64.f, 
						21.f, float(PARTY_INFO_WINDOW_HEIGHT)-64.f-45.f);
	RenderImage(IMAGE_PARTY_BASE_WINDOW_RIGHT, m_Pos.x+float(PARTY_INFO_WINDOW_WIDTH)-21.f, 
						m_Pos.y+64.f, 21.f, float(PARTY_INFO_WINDOW_HEIGHT)-64.f-45.f);
	RenderImage(IMAGE_PARTY_BASE_WINDOW_BOTTOM, m_Pos.x, m_Pos.y+float(PARTY_INFO_WINDOW_HEIGHT)-45.f, 
						float(PARTY_INFO_WINDOW_WIDTH), 45.f);

	m_BtnExit.Render();			// Exit Button

	// UI제목
	g_pRenderText->SetFont( g_hFontBold );
	g_pRenderText->RenderText(m_Pos.x+60, m_Pos.y+12, GlobalText[190], 72, 0, RT3_SORT_CENTER);		// "파  티"

	g_pRenderText->SetFont( g_hFont );
	
	if( m_bParty )			// 파티상태일때
	{
		for( int i=0 ; i<PartyNumber ; i++ )
		{
			PARTY_t *pMember = &Party[i];
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			int iVal = i*71;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			bool bExitBtnRender =false;
			
			// 기타 인터페이스
			// 파티에서 나가기 버튼( 파티장일때만, 파티장이 아닐때는 자신만 렌더)
			if( !strcmp( Party[0].Name, Hero->ID ) || !strcmp( Party[i].Name, Hero->ID ) )
			{
				bExitBtnRender = true;
			}

			RenderMemberStatue( i, pMember, bExitBtnRender );
		}
	}
	else						// 파티상태가 아닐때
	{
		int iStartHeight = 60;
		g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+iStartHeight, GlobalText[191], 0, 0, RT3_SORT_CENTER);		// "파티를 맺고 싶은 사람과"
		g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+iStartHeight+15, GlobalText[192], 0, 0, RT3_SORT_CENTER);		// "마주본 상태에서 /파티 라고"
		g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+iStartHeight+30, GlobalText[193], 0, 0, RT3_SORT_CENTER);		// "입력하시면 파티가 맺어짐."
		g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+iStartHeight+50, GlobalText[194], 0, 0, RT3_SORT_CENTER);		// "파티를 맺은 후 몹을 사냥"
		g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+iStartHeight+65, GlobalText[195], 0, 0, RT3_SORT_CENTER);		// "하면 더 많은 경험치를 파티원"
		g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+iStartHeight+80, GlobalText[196], 0, 0, RT3_SORT_CENTER);		// "들의 레벨에 따라 평등하게"
		g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+iStartHeight+95, GlobalText[197], 0, 0, RT3_SORT_CENTER);		// "나누어 가집니다."
	}
	
	g_pRenderText->SetBgColor( dwPreBGColor );

	DisableAlphaBlend();

	return true;
}

//---------------------------------------------------------------------------------------------
// RenderGroupBox()
void CNewUIPartyInfoWindow::RenderGroupBox( int iPosX, int iPosY, int iWidth, int iHeight, int iTitleWidth/* =60 */, int iTitleHeight/* =20  */ )
{
	// 배경
	EnableAlphaTest();

	glColor4f ( 0.f, 0.f, 0.f, 0.9f );
	RenderColor( float(iPosX+3), float(iPosY+2), float(iTitleWidth-8), float(iTitleHeight) );
 	glColor4f ( 0.f, 0.f, 0.f, 0.6f );							
 	RenderColor( float(iPosX+3), float(iPosY+2+iTitleHeight), float(iWidth-7), float(iHeight-iTitleHeight-7) );

	EndRenderColor();

	// 모서리
	RenderImage(IMAGE_PARTY_TABLE_TOP_LEFT, iPosX, iPosY, 14, 14);
	RenderImage(IMAGE_PARTY_TABLE_TOP_RIGHT, iPosX+iTitleWidth-14, iPosY, 14, 14);
	RenderImage(IMAGE_PARTY_TABLE_TOP_RIGHT, iPosX+iWidth-14, iPosY+iTitleHeight, 14, 14);
	RenderImage(IMAGE_PARTY_TABLE_BOTTOM_LEFT, iPosX, iPosY+iHeight-14, 14, 14);
	RenderImage(IMAGE_PARTY_TABLE_BOTTOM_RIGHT, iPosX+iWidth-14, iPosY+iHeight-14, 14, 14);

	// 선
	RenderImage(IMAGE_PARTY_TABLE_TOP_PIXEL, iPosX+6, iPosY, iTitleWidth-12, 14);
	RenderImage(IMAGE_PARTY_TABLE_RIGHT_PIXEL, iPosX+iTitleWidth-14, iPosY+6, 14, iTitleHeight-6);
	RenderImage(IMAGE_PARTY_TABLE_TOP_PIXEL, iPosX+iTitleWidth-5, iPosY+iTitleHeight, iWidth-iTitleWidth-6, 14);
	RenderImage(IMAGE_PARTY_TABLE_RIGHT_PIXEL, iPosX+iWidth-14, iPosY+iTitleHeight+6, 14, iHeight-iTitleHeight-14);
	RenderImage(IMAGE_PARTY_TABLE_BOTTOM_PIXEL, iPosX+6, iPosY+iHeight-14, iWidth-12, 14);
	RenderImage(IMAGE_PARTY_TABLE_LEFT_PIXEL, iPosX, iPosY+6, 14, iHeight-14);
}

//---------------------------------------------------------------------------------------------
// RenderMemberStatue
void CNewUIPartyInfoWindow::RenderMemberStatue( int iIndex, PARTY_t* pMember, bool bExitBtnRender /*= false*/)
{
	unicode::t_char szText[256] = {0, };
	
	int iVal = iIndex*71;		// 인덱스에 따른 편차(y의 위치)
	int iPosX = m_Pos.x+10;
	int iPosY = m_Pos.y+40+iVal;
	
	RenderGroupBox( iPosX, iPosY, 170, 70, 70, 20 );

	// Character Information
	g_pRenderText->SetFont( g_hFontBold );
	if( iIndex == 0 )	
	{	
		// 길드장
		g_pRenderText->SetFont( g_hFontBold );
		g_pRenderText->SetTextColor( 0, 255, 0, 255 );
		// ID
		g_pRenderText->RenderText( iPosX, iPosY+8, pMember->Name, 70, 0, RT3_SORT_CENTER );

		// 길드장 표시 깃발
		RenderImage(IMAGE_PARTY_FLAG, m_Pos.x+146, iPosY+23, 10, 12);	
	}
	else
	{
		// 길드장 아님
		g_pRenderText->SetFont( g_hFontBold );
		g_pRenderText->SetTextColor( 255, 255, 255, 255 );
		// ID
		g_pRenderText->RenderText( iPosX, iPosY+8, pMember->Name, 70, 0, RT3_SORT_CENTER );
	}
	
	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetTextColor( RGBA( 194, 194, 194, 255 ) );

	// 현재위치의 맵이름
	g_pRenderText->RenderText( iPosX+10, iPosY+26, GetMapName( pMember->Map ), 70, 0, RT3_SORT_LEFT );
	
	// 좌표
	unicode::_sprintf( szText, "(%d,%d)", pMember->x, pMember->y );
	g_pRenderText->RenderText( iPosX+85, iPosY+26, szText, 60, 0, RT3_SORT_LEFT);

	// HPBar
	// HP length = (남아있는HP * 전체길이(151-4) / 최대HP(255)
	int iHP = (pMember->currHP*147)/pMember->maxHP;	
	RenderImage(IMAGE_PARTY_HPBAR_BACK, iPosX+8, iPosY+39, 151, 8);					
	RenderImage(IMAGE_PARTY_HPBAR, iPosX+10, iPosY+41, iHP, 4);

	// HP
	unicode::_sprintf( szText, "%d %s %d", pMember->currHP, GlobalText[2374], pMember->maxHP );
	g_pRenderText->RenderText( iPosX+88, iPosY+51, szText, 70, 0, RT3_SORT_RIGHT );

	if( bExitBtnRender )
	{
		m_BtnPartyExit[iIndex].Render();
	}
}

//---------------------------------------------------------------------------------------------
// LeaveParty
bool CNewUIPartyInfoWindow::LeaveParty( const int iIndex )
{
	// 저주받은 사원(?) 에 들어갔을때 는 파티 탈퇴 예외 처리
	if( !g_CursedTemple->IsCursedTemple() )
	{
		PlayBuffer(SOUND_CLICK01);
		SendRequestPartyLeave(Party[iIndex].Number);
	}

	SetParty( false );

	return true;
}


//---------------------------------------------------------------------------------------------
// SetPos
void CNewUIPartyInfoWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;

	m_BtnExit.ChangeButtonInfo( m_Pos.x+13, m_Pos.y+392, 36, 29 );

	// Exit Party Button Initialize
	for( int i=0 ; i<MAX_PARTYS ; i++ )
	{	
		int iVal = i*71;
		m_BtnPartyExit[i].ChangeButtonInfo( m_Pos.x+159, m_Pos.y+63+iVal, 13, 13 );
	}	
}

//---------------------------------------------------------------------------------------------
// SetParty
void CNewUIPartyInfoWindow::SetParty( bool bParty )
{
	m_bParty = bParty;	
}


//---------------------------------------------------------------------------------------------
// GetLayerDepth
float CNewUIPartyInfoWindow::GetLayerDepth()
{
	return 2.4f;
}

//---------------------------------------------------------------------------------------------
// LoadImages
void CNewUIPartyInfoWindow::LoadImages()
{
	// 기본창
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_PARTY_BASE_WINDOW_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_PARTY_BASE_WINDOW_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_PARTY_BASE_WINDOW_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_PARTY_BASE_WINDOW_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_PARTY_BASE_WINDOW_BOTTOM, GL_LINEAR);
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_PARTY_BASE_WINDOW_BTN_EXIT, GL_LINEAR);		// Exit Button

	// GroupBox
	LoadBitmap("Interface\\newui_item_table01(L).tga", IMAGE_PARTY_TABLE_TOP_LEFT);
	LoadBitmap("Interface\\newui_item_table01(R).tga", IMAGE_PARTY_TABLE_TOP_RIGHT);
	LoadBitmap("Interface\\newui_item_table02(L).tga", IMAGE_PARTY_TABLE_BOTTOM_LEFT);
	LoadBitmap("Interface\\newui_item_table02(R).tga", IMAGE_PARTY_TABLE_BOTTOM_RIGHT);
	LoadBitmap("Interface\\newui_item_table03(Up).tga", IMAGE_PARTY_TABLE_TOP_PIXEL);
	LoadBitmap("Interface\\newui_item_table03(Dw).tga", IMAGE_PARTY_TABLE_BOTTOM_PIXEL);
	LoadBitmap("Interface\\newui_item_table03(L).tga", IMAGE_PARTY_TABLE_LEFT_PIXEL);
	LoadBitmap("Interface\\newui_item_table03(R).tga", IMAGE_PARTY_TABLE_RIGHT_PIXEL);

	// 파티창
	LoadBitmap("Interface\\newui_party_lifebar01.jpg", IMAGE_PARTY_HPBAR_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_party_lifebar02.jpg", IMAGE_PARTY_HPBAR, GL_LINEAR);
	LoadBitmap("Interface\\newui_party_flag.tga", IMAGE_PARTY_FLAG, GL_LINEAR);
	LoadBitmap( "Interface\\newui_party_x.tga", IMAGE_PARTY_EXIT, GL_LINEAR );
}

//---------------------------------------------------------------------------------------------
// UnloadImages
void CNewUIPartyInfoWindow::UnloadImages()
{
	// 기본창
	DeleteBitmap( IMAGE_PARTY_BASE_WINDOW_BACK );
	DeleteBitmap( IMAGE_PARTY_BASE_WINDOW_TOP );
	DeleteBitmap( IMAGE_PARTY_BASE_WINDOW_LEFT );
	DeleteBitmap( IMAGE_PARTY_BASE_WINDOW_RIGHT );
	DeleteBitmap( IMAGE_PARTY_BASE_WINDOW_BOTTOM );
	DeleteBitmap( IMAGE_PARTY_BASE_WINDOW_BTN_EXIT );	// Exit Button

	// GroupBox
	DeleteBitmap(IMAGE_PARTY_TABLE_RIGHT_PIXEL);
	DeleteBitmap(IMAGE_PARTY_TABLE_LEFT_PIXEL);
	DeleteBitmap(IMAGE_PARTY_TABLE_BOTTOM_PIXEL);
	DeleteBitmap(IMAGE_PARTY_TABLE_TOP_PIXEL);
	DeleteBitmap(IMAGE_PARTY_TABLE_BOTTOM_RIGHT);
	DeleteBitmap(IMAGE_PARTY_TABLE_BOTTOM_LEFT);
	DeleteBitmap(IMAGE_PARTY_TABLE_TOP_RIGHT);
	DeleteBitmap(IMAGE_PARTY_TABLE_TOP_LEFT);

	// 파티창
	DeleteBitmap(IMAGE_PARTY_HPBAR_BACK);
	DeleteBitmap(IMAGE_PARTY_HPBAR);
	DeleteBitmap(IMAGE_PARTY_FLAG);
	DeleteBitmap(IMAGE_PARTY_EXIT);
}
