// w_Text.h: interface for the Text class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UISYSTEM

#if !defined(AFX_W_TEXT_H__4CF15885_03B5_4A15_AB4F_63FA7D07A021__INCLUDED_)
#define AFX_W_TEXT_H__4CF15885_03B5_4A15_AB4F_63FA7D07A021__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TextBitmap.h"
#include "Text.h"
#include "w_ImageUnit.h"
#include "w_UIWnd.h"
#include "Text.h"

/*
ui::TextPtr cashtext( new ui::Text("cashtext") );
cashtext->Make( FONTSIZE_LANGUAGE, pricename.length() );
cashtext->SetAlign( TA_LEFT );
cashtext->SetContent( ( format( "¹Ùº¸%1%" ) % "¸Þ·Õ") ).str() );
cashtext->SetColor( 0xff00ED1B );
textpos = Coord( 84+((107/2)-(cashtext->GetSize().x/2)), 444);
cashtext->Move( textpos );
shoprightFrame.AddChild( cashtext );
*/

namespace ui
{
	BoostSmartPointer(Text);
	class Text : public UIWnd
	{
	public:
		Text(const string& rstrName);
		virtual ~Text();
		
		void Make(int nFontHeight, int nTextMaxLen,
			TEXT_STYLE eStyle = TS_SHADOW, TEXT_TYPE eType = TT_NORMAL);
		void SetContent(const string& rstrContent);
		void SetColor(const Color& rColor);
		void SetAlign(BYTE byAlign);

	public:
		virtual void Process(int nDelta);
		virtual void DrawImage();

	protected:
		virtual void OnMove(const Coord& rPrevPos);
		virtual void OnResize(const Dim& rPrevSize);
		virtual void OnMouseOver();
		virtual void OnMouseOut();
		virtual void OnChangeOpacity(BYTE byPrevOpacity);

	private:
		void Initialize();
		void Destroy();

	private:
		BoostSmart_Ptr(CText)	m_pTextBase;
	};
};

#endif // !defined(AFX_W_TEXT_H__4CF15885_03B5_4A15_AB4F_63FA7D07A021__INCLUDED_)

#endif //NEW_USER_INTERFACE_UISYSTEM