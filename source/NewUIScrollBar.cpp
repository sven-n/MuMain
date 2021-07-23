//=============================================================================
//
//	NewUIScrollBar.cpp
//
//	Copyright 2010 Webzen Mu-Studio
//
//=============================================================================
#include "StdAfx.h"

#ifdef KJW_ADD_NEWUI_SCROLLBAR

#include "NewUIScrollBar.h"

using namespace SEASON3B;

//=============================================================================
//  @Name   CNewUIScrollBar()
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  스크롤바 생성자
//=============================================================================
CNewUIScrollBar::CNewUIScrollBar()
{
	memset( &m_ptPos, 0, sizeof(POINT) );
	memset( &m_ptScrollBtnStartPos, 0, sizeof(POINT) );
	memset( &m_ptScrollBtnPos, 0, sizeof(POINT) );
	
	m_iScrollBarPickGap = 0;
	
	m_iScrollBarMovePixel = 1;
	m_iScrollBarHeightPixel = 0;
	m_iScrollBarMiddleNum = 0;
	m_iScrollBarMiddleRemainderPixel = 0;
	
	m_iScrollBtnMouseEvent = SCROLLBAR_MOUSEBTN_NORMAL;				
	m_bScrollBtnActive = false;					
	
	m_fPercentOfSize = 1.0f;

	m_iBeginPos = 0;
	m_iCurPos = 0;
	m_iMaxPos = 1;
}

//=============================================================================
//  @Name   ~CNewUIScrollBar()
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  스크롤바 소멸자
//=============================================================================
CNewUIScrollBar::~CNewUIScrollBar()
{
	Release();
}

//=============================================================================
//  @Name   Create( int iX, int iY, int iHeight )
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  필요한 이미지 로드 및 초기화
//=============================================================================
bool CNewUIScrollBar::Create( int iX, int iY, int iHeight )
{
	m_iHeight = iHeight;
	SetPos( iX, iY );
		
	LoadImages();
	Show( true );
	m_bScrollBtnActive = true;
	
	return true;
}

//=============================================================================
//  @Name   Release()
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  로드된 이미지 해제
//=============================================================================
void CNewUIScrollBar::Release()
{
	UnloadImages();
}

//=============================================================================
//  @Name   GetLayerDepth()
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  Z버퍼 깊이 설정
//=============================================================================
float CNewUIScrollBar::GetLayerDepth()
{
	return 4.4f;
}

//=============================================================================
//  @Name   LoadImages()
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  스크롤바에 필요한 이미지 로드
//=============================================================================
void CNewUIScrollBar::LoadImages()
{
	LoadBitmap( "Interface\\newui_scrollbar_up.tga", IMAGE_SCROLL_TOP );
	LoadBitmap( "Interface\\newui_scrollbar_m.tga", IMAGE_SCROLL_MIDDLE, GL_LINEAR );
	LoadBitmap( "Interface\\newui_scrollbar_down.tga", IMAGE_SCROLL_BOTTOM );
	LoadBitmap( "Interface\\newui_scroll_on.tga", IMAGE_SCROLLBAR_ON, GL_LINEAR );
	LoadBitmap( "Interface\\newui_scroll_off.tga", IMAGE_SCROLLBAR_OFF, GL_LINEAR );
}

//=============================================================================
//  @Name   UnloadImages()
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  로드한 이미지 해제
//=============================================================================
void CNewUIScrollBar::UnloadImages()
{
	DeleteBitmap( IMAGE_SCROLL_TOP );
	DeleteBitmap( IMAGE_SCROLL_MIDDLE );
	DeleteBitmap( IMAGE_SCROLL_BOTTOM );
	DeleteBitmap( IMAGE_SCROLLBAR_ON );
	DeleteBitmap( IMAGE_SCROLLBAR_OFF );
}

//=============================================================================
//  @Name   SetPos(int x, int y)
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  스크롤바의 위치를 토대로 각각 콘트롤의 위치 재설정
//=============================================================================
void CNewUIScrollBar::SetPos(int x, int y)
{
	m_ptPos.x = x;
	m_ptPos.y = y;
	
	m_ptScrollBtnStartPos.x = m_ptPos.x - ( SCROLLBTN_WIDTH / 2 - SCROLLBAR_TOP_WIDTH / 2 );
	m_ptScrollBtnStartPos.y = m_ptPos.y;

	m_ptScrollBtnPos.x = m_ptScrollBtnStartPos.x;
	m_ptScrollBtnPos.y = m_ptScrollBtnStartPos.y;

	m_iScrollBarMovePixel = m_iHeight - SCROLLBTN_HEIGHT;
	if( m_iScrollBarMovePixel < 0 )
		m_iScrollBarMovePixel = 1;

	m_iScrollBarHeightPixel = m_iHeight;

	m_iScrollBarMiddleNum = ( m_iScrollBarHeightPixel - ( SCROLLBAR_TOP_HEIGHT * 2 ) ) / SCROLLBAR_MIDDLE_HEIGHT;
	m_iScrollBarMiddleRemainderPixel = ( m_iScrollBarHeightPixel - ( SCROLLBAR_TOP_HEIGHT * 2 ) ) % SCROLLBAR_MIDDLE_HEIGHT;
}

//=============================================================================
//  @Name   UpdateBtnEvent()
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  버튼 이벤트 처리
//=============================================================================
bool CNewUIScrollBar::UpdateBtnEvent()
{
	// 왼쪽 마우스키를 땠을때
	if( IsRelease( VK_LBUTTON ) )
	{
		m_iScrollBtnMouseEvent = SCROLLBAR_MOUSEBTN_NORMAL;
		m_iScrollBarPickGap = 0;
		return true;
	}

	// 스크롤 버튼을 눌렀을때
	if( CheckMouseIn( m_ptScrollBtnPos.x, m_ptScrollBtnPos.y, SCROLLBTN_WIDTH, SCROLLBTN_HEIGHT ) )
	{	
		if( IsPress( VK_LBUTTON ) && m_bScrollBtnActive == true )
		{
			m_iScrollBarPickGap = MouseY - m_ptScrollBtnPos.y;
			m_iScrollBtnMouseEvent = SCROLLBAR_MOUSEBTN_CLICKED;
			return false;
		}	
	}

	// 스크롤바를 눌렀을때
	if( CheckMouseIn( m_ptPos.x, m_ptPos.y, SCROLLBAR_TOP_WIDTH, m_iScrollBarHeightPixel ) )
	{	
		if( IsPress( VK_LBUTTON ) && m_bScrollBtnActive == true )
		{
			float fPercent = (float)(MouseY - m_ptPos.y ) / (float)m_iScrollBarMovePixel;
			SetPercent( fPercent );
			return true;
		}	
	}

	return true;
}

//=============================================================================
//  @Name   UpdateMouseEvent()
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  마우스 이벤트 처리
//=============================================================================
bool CNewUIScrollBar::UpdateMouseEvent()
{
	
	if( UpdateBtnEvent() == true )	// 처리가 완료 되었다면
		return false;

	return true;
}

//=============================================================================
//  @Name   UpdateKeyEvent()
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  키입력 이벤트 처리
//=============================================================================
bool CNewUIScrollBar::UpdateKeyEvent()
{
	if( !IsVisible() )
	{
		
	}

	return true;
}

//=============================================================================
//  @Name   Update()
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  업데이트
//=============================================================================
bool CNewUIScrollBar::Update()
{
	// 스크롤 버튼이 눌린상태라면
	if( m_iScrollBtnMouseEvent == SCROLLBAR_MOUSEBTN_CLICKED )
	{
		// 마우스의 위치에 따라 스크롤바 버튼을 이동시켜준다.
		float fPercent = (float)(MouseY - m_iScrollBarPickGap - m_ptPos.y ) / (float)m_iScrollBarMovePixel;
		SetPercent( fPercent );
	}

 	return true;
}

//=============================================================================
//  @Name   Render()
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  랜더링
//=============================================================================
bool CNewUIScrollBar::Render()
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// 스크롤바 Frame

	// TOP
	RenderImage(IMAGE_SCROLL_TOP, m_ptPos.x, m_ptPos.y, 
					SCROLLBAR_TOP_WIDTH, SCROLLBAR_TOP_HEIGHT );		

	// MIDDLE
	for( int i=0;i<m_iScrollBarMiddleNum;i++ )
	{
		RenderImage( IMAGE_SCROLL_MIDDLE, m_ptPos.x, 
			m_ptPos.y + SCROLLBAR_TOP_HEIGHT+( i * SCROLLBAR_MIDDLE_HEIGHT ),
			SCROLLBAR_TOP_WIDTH, SCROLLBAR_MIDDLE_HEIGHT );	
	}

	// MIDDLE 나머지
	if( m_iScrollBarMiddleRemainderPixel > 0 )
	{
		RenderImage( IMAGE_SCROLL_MIDDLE, m_ptPos.x, 
			m_ptPos.y + SCROLLBAR_TOP_HEIGHT+( m_iScrollBarMiddleNum * SCROLLBAR_MIDDLE_HEIGHT),
			SCROLLBAR_TOP_WIDTH, m_iScrollBarMiddleRemainderPixel );	
	}

	// BOTTOM
	RenderImage(IMAGE_SCROLL_BOTTOM, m_ptPos.x, m_ptPos.y + m_iHeight - SCROLLBAR_TOP_HEIGHT,
					SCROLLBAR_TOP_WIDTH, SCROLLBAR_TOP_HEIGHT );		
	
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// 스크롤바 버튼
	if( m_bScrollBtnActive == true )
	{
		if (m_iScrollBtnMouseEvent == SCROLLBAR_MOUSEBTN_CLICKED) 
		{
			glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
		}
		RenderImage(IMAGE_SCROLLBAR_ON, m_ptScrollBtnPos.x, m_ptScrollBtnPos.y, 
			SCROLLBTN_WIDTH, SCROLLBTN_HEIGHT);
	}
	else
	{
		RenderImage(IMAGE_SCROLLBAR_OFF, m_ptScrollBtnPos.x, m_ptScrollBtnPos.y, 
			SCROLLBTN_WIDTH, SCROLLBTN_HEIGHT);
	}
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	DisableAlphaBlend();
	return true;
}

//=============================================================================
//  @Name   UpdateScrolling()
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  스크롤 버튼 이미지의 위치를 이동시킨다.
//=============================================================================
void CNewUIScrollBar::UpdateScrolling()
{
	m_ptScrollBtnPos.y = m_ptScrollBtnStartPos.y + m_fPercentOfSize * m_iScrollBarMovePixel;
}

//=============================================================================
//  @Name   ScrollUp( int iMoveValue )
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  스크롤 업 버튼을 눌렀을때
//=============================================================================
void CNewUIScrollBar::ScrollUp( int iMoveValue )
{
	SetCurPos( m_iCurPos + iMoveValue );
}

//=============================================================================
//  @Name   ScrollDown( int iMoveValue )
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  스크롤 다운 버튼을 눌렀을때
//=============================================================================
void CNewUIScrollBar::ScrollDown( int iMoveValue )
{
	SetCurPos( m_iCurPos - iMoveValue );
}

//=============================================================================
//  @Name   SetPercent( float fPercent )
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  스크롤 되어있는 위치가 어디쯤인지 퍼센트 값으로 설정
//=============================================================================
void CNewUIScrollBar::SetPercent( float fPercent )
{
	if( fPercent <= 0.0f )
		m_fPercentOfSize = 0.0f;
	else if( fPercent >= 1.0f )
		m_fPercentOfSize = 1.0f;
	else
		m_fPercentOfSize = fPercent;

	m_iCurPos = m_iMaxPos * m_fPercentOfSize;
	UpdateScrolling();
}

//=============================================================================
//  @Name   SetMaxPos( int iMaxPos )
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  스크롤될 수 있는 최대 수치 설정
//=============================================================================
void CNewUIScrollBar::SetMaxPos( int iMaxPos )
{ 
	if( iMaxPos < 1 )
		iMaxPos = 1;

	m_iMaxPos = iMaxPos; 
}

//=============================================================================
//  @Name   SetCurPos( int iPosValue )
//  @author Kim Jae Woo
//  @date   2010-11-1
//  @brief  현재 스크롤 위치 설정
//=============================================================================
void CNewUIScrollBar::SetCurPos( int iPosValue )
{
	if( m_iBeginPos >= iPosValue )	
		m_iCurPos = m_iBeginPos;
	else if( m_iMaxPos <= iPosValue ) 
		m_iCurPos = m_iMaxPos;
	else 
		m_iCurPos = iPosValue;

	m_fPercentOfSize = (float)m_iCurPos / (float)m_iMaxPos;
	UpdateScrolling();
}

#endif // KJW_ADD_NEWUI_SCROLLBAR