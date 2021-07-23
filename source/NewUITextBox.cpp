//=============================================================================
//
//	NewUITextBox.cpp
//
//	Copyright 2010 Webzen Mu-Studio
//
//=============================================================================
#pragma once
#include "StdAfx.h"

#ifdef KJW_ADD_NEWUI_TEXTBOX

#include "NewUITextBox.h"
#include "UIControls.h"
#include "wsclientinline.h"

using namespace SEASON3B;

const int iMAX_TEXT_LINE = 512;
const int iLINE_INTERVAL = 2;

//=============================================================================
//  @Name   CNewUITextBox()
//  @author Kim Jae Woo
//  @date   2010-10-22
//  @brief  생성자
//=============================================================================
CNewUITextBox::CNewUITextBox()
{
	m_iWidth = 0;
	m_iHeight = 0;
	memset( &m_ptPos, 0, sizeof(POINT) );
	
	m_iTextHeight = 0;
	m_iTextLineHeight = 1;
	m_iLimitLine = 0;
		
	m_iMaxLine = 0;
	m_iCurLine = 0;
}

//=============================================================================
//  @Name   ~CNewUITextBox()
//  @author Kim Jae Woo
//  @date   2010-10-22
//  @brief  소멸자
//=============================================================================
CNewUITextBox::~CNewUITextBox()
{
	Release();
}

//=============================================================================
//  @Name   Create( int iX, int iY, int iWidth ,int iHeight )
//  @author Kim Jae Woo
//  @date   2010-10-25
//  @brief  좌표정보 및 초기화
//=============================================================================
bool CNewUITextBox::Create( int iX, int iY, int iWidth ,int iHeight )
{
	SetPos( iX, iY, iWidth, iHeight );
	Show( true );
	
	return true;
}

//=============================================================================
//  @Name   SetPos( int iX, int iY, int iWidth ,int iHeight )
//  @author Kim Jae Woo
//  @date   2010-10-25
//  @brief  좌표정보 설정
//=============================================================================
void CNewUITextBox::SetPos( int iX, int iY, int iWidth ,int iHeight )
{
	m_ptPos.x = iX;
	m_ptPos.y = iY;
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	SIZE Fontsize;
	g_pRenderText->SetFont(g_hFont);

	unicode::t_string strTemp = _T("A");

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		g_pMultiLanguage->_GetTextExtentPoint32( g_pRenderText->GetFontDC(), strTemp.c_str(), strTemp.size(), &Fontsize );
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		unicode::_GetTextExtentPoint( g_pRenderText->GetFontDC(), strTemp.c_str(), strTemp.size(), &Fontsize );
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

	m_iTextHeight = Fontsize.cy;
	m_iTextLineHeight = m_iTextHeight + iLINE_INTERVAL;

	m_iLimitLine = m_iHeight / m_iTextLineHeight;

	m_iMaxLine = 0;
	m_iCurLine = 0;
}

//=============================================================================
//  @Name   Release()
//  @author Kim Jae Woo
//  @date   2010-10-25
//  @brief  해제
//=============================================================================
void CNewUITextBox::Release()
{

}

//=============================================================================
//  @Name   GetLayerDepth()
//  @author Kim Jae Woo
//  @date   2010-10-25
//  @brief  UI깊이(Z-Buffer)
//=============================================================================
float CNewUITextBox::GetLayerDepth()
{
	return 4.4f;
}

//=============================================================================
//  @Name   UpdateMouseEvent()
//  @author Kim Jae Woo
//  @date   2010-10-25
//  @brief  마우스 이벤트 처리
//=============================================================================
bool CNewUITextBox::UpdateMouseEvent()
{
	return true;
}

//=============================================================================
//  @Name   UpdateKeyEvent()
//  @author Kim Jae Woo
//  @date   2010-10-25
//  @brief  키입력 이벤트 처리
//=============================================================================
bool CNewUITextBox::UpdateKeyEvent()
{
	return true;
}

//=============================================================================
//  @Name   Update()
//  @author Kim Jae Woo
//  @date   2010-10-25
//  @brief  업데이트
//=============================================================================
bool CNewUITextBox::Update()
{
 	return true;
}

//=============================================================================
//  @Name   Render()
//  @author Kim Jae Woo
//  @date   2010-10-25
//  @brief  랜더링
//=============================================================================
bool CNewUITextBox::Render()
{
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	for( int iIndex = 0; iIndex < m_iLimitLine; iIndex++ )
	{
		int iLineIndex = m_iCurLine + iIndex;
		
		if( GetLineText( iLineIndex ).empty() == false  )
		{
			g_pRenderText->SetFont( g_hFont );
			//g_pRenderText->SetBgColor( 0, 0, 0, 0 );
			g_pRenderText->SetTextColor( 255, 255, 255, 255 );
			g_pRenderText->RenderText(	m_ptPos.x,	m_ptPos.y + iIndex * m_iTextLineHeight,	GetLineText( iLineIndex ).c_str(),
										m_iWidth,	0,		RT3_SORT_LEFT );
		}
	}

	return true;
}

//=============================================================================
//  @Name   AddText( unicode::t_char* strText )
//  @author Kim Jae Woo
//  @date   2010-10-25
//  @brief  문자열을 추가시켜준다. 만약 텍스트박스보다 길다면 박스크기에 맞게 잘라준다.
//=============================================================================
void CNewUITextBox::AddText( unicode::t_char* strText )
{
	unicode::t_char strTemp[ iMAX_TEXT_LINE ][ iMAX_TEXT_LINE ];
	::memset( strTemp[0], 0, sizeof( char ) * iMAX_TEXT_LINE * iMAX_TEXT_LINE );

	// strText를 m_iWidth길이에 맞게 잘라서 strTemp에 결과를 집어 넣는다. 잘려진 라인의 길이는 iTextLine이다.
	int iTextLine = ::DivideStringByPixel( &strTemp[0][0], iMAX_TEXT_LINE, iMAX_TEXT_LINE, strText, m_iWidth, true, '#' );
	
	// 나쥐어진 라인 수만큼 텍스트 추가
	for( int iIndex=0;iIndex<iTextLine;iIndex++ )
	{
		m_vecText.push_back( strTemp[ iIndex ] );
	}
}	

//=============================================================================
//  @Name   AddText( const unicode::t_char* strText )
//  @author Kim Jae Woo
//  @date   2010-10-25
//  @brief  정적 문자열을 받기 위한 함수 AddText( unicode::t_char* strText )와 동일한 작동
//=============================================================================
void CNewUITextBox::AddText( const unicode::t_char* strText )
{
	unicode::t_char strTempText[iMAX_TEXT_LINE] = {0,};
	sprintf( strTempText, strText );

	AddText( strTempText );
}

//=============================================================================
//  @Name   GetFullText()
//  @author Kim Jae Woo
//  @date   2010-10-25
//  @brief  현재 텍스트박스에 입력되어있는 전체 문자열을 가져온다.
//=============================================================================
unicode::t_string CNewUITextBox::GetFullText()
{
	unicode::t_string strTemp;
	
	type_vector_textbase::iterator vi = m_vecText.begin();
	for(; vi != m_vecText.end(); vi++)
	{
		strTemp += (*vi);
	}

	return strTemp;
}

//=============================================================================
//  @Name   GetLineText( int iLineIndex )
//  @author Kim Jae Woo
//  @date   2010-10-25
//  @brief  iLineIndex에 해당하는 인덱스(라인)에 있는 문자열을 읽어온다.
//=============================================================================
unicode::t_string CNewUITextBox::GetLineText( int iLineIndex )
{
	if( 0 > iLineIndex || m_vecText.size() <= iLineIndex )
		return "";

	return m_vecText[ iLineIndex ];
}

//=============================================================================
//  @Name   GetMoveableLine()
//  @author Kim Jae Woo
//  @date   2010-10-25
//  @brief  움직일 수 있는 라인수 스크롤바 및 텍스트 박스 시작위치 설정시 사용
//=============================================================================
int CNewUITextBox::GetMoveableLine()		
{
	int iMoveableLine = m_vecText.size() - m_iLimitLine;
	if( iMoveableLine <= 0 )
		iMoveableLine = 0;

	return iMoveableLine;
}

#endif // KJW_ADD_NEWUI_TEXTBOX