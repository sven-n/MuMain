//=============================================================================
//
//	NewUITextBox.h
//
//	Copyright 2010 Webzen Mu-Studio
//
//=============================================================================
#ifndef _NEWUITEXTBOX_H_
#define _NEWUITEXTBOX_H_

#ifdef KJW_ADD_NEWUI_TEXTBOX
#pragma once
#include "NewUIBase.h"

namespace SEASON3B
{
	//=============================================================================
	//	CLASS   CNewUITextBox 
	//  @brief  텍스트박스 UI
	//=============================================================================
	class CNewUITextBox : public CNewUIObj
	{
		typedef std::vector<unicode::t_string>		type_vector_textbase;
		type_vector_textbase	m_vecText;		
		
	protected:
		int							m_iWidth;						// 텍스트 박스의 넓이
		int							m_iHeight;						// 테스트 박스의 높이
		POINT						m_ptPos;						// 텍스트 박스의 위치
		
		int							m_iTextHeight;					// 문자의 크기
		int							m_iTextLineHeight;				// 문자의 크기 + 공백
		int							m_iLimitLine;					// 한번에 표시될 수 있는 라인수
			
		int							m_iMaxLine;						// 전체 라인수
		int							m_iCurLine;						// 현재 라인

	public:
		CNewUITextBox();
		virtual ~CNewUITextBox();
		
		bool Create( int iX, int iY, int iWidth ,int iHeight );
		void SetPos( int iX, int iY, int iWidth ,int iHeight );
		void Release();
		
		float GetLayerDepth();
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		
		bool Update();
		bool Render();
		
		void ClearText(){ m_vecText.clear(); }							// 텍스트 박스의 내용을 지워준다.
		void AddText( unicode::t_char* strText );						// 텍스트 박스에 내용을 추가한다.
		void AddText( const unicode::t_char* strText );					// 텍스트 박스에 내용을 추가한다.
		
		unicode::t_string GetFullText();								// 텍스트 박스의 전체 문자열을 가져온다.
		unicode::t_string GetLineText( int iLineIndex );				// 텍스트 박스의 특정 라인의 문자열을 가져온다.
		
		int GetMoveableLine();											// 움직일 수 있는 라인수
		int GetLimitLine(){ return m_iLimitLine; }						// 한번에 표시될 수 있는 라인수를 가져온다.
		int GetMaxLine(){ return m_vecText.size(); }					// 텍스트 박스의 전체 라인수를 가져온다.
		int GetCurLine(){ return m_iCurLine; }							// 현재 텍스트 박스에 표시되고 있는 시작줄을 가져온다.
		void SetCurLine( int iLine ){ m_iCurLine = iLine; }				// 현재 텍스트 박스에 표시되고 있는 시작줄을 설정한다.
	};
}

#endif // KJW_ADD_NEWUI_TEXTBOX
#endif _NEWUITEXTBOX_H_

