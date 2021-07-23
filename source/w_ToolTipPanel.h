// w_ToolTipPanel.h: interface for the ToolTipPanel class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UISYSTEM

#if !defined(AFX_W_TOOLTIPPANEL_H__34A747D2_EE7C_48F4_9B2A_AD58630BF8DB__INCLUDED_)
#define AFX_W_TOOLTIPPANEL_H__34A747D2_EE7C_48F4_9B2A_AD58630BF8DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_ImageUnit.h"
#include "w_UIWnd.h"
#include "w_Text.h"

/*
vector<string>	data;
vector<DWORD>	colordata;

ToolTipPanelPtr tooltip( new ToolTipPanel( "tooltip" ) ); 
tooltip->SetInfo( data, colordata );
tooltip->Move( Coord( iTargetX, iTargetY ) );
tooltip->SetAngleStat( true );
tooltip->DrawRect();
*/

namespace ui
{
	BoostSmartPointer(ToolTipPanel);
	class ToolTipPanel : public UIWnd
	{
	public:
		ToolTipPanel( const string& name );
		virtual ~ToolTipPanel();

	public:
		void SetInfo( const CASHSHOP_ITEMLIST& data );
		void SetInfo( const vector<string>& data );
		void SetInfo( const vector<string>& data, const vector<DWORD>& colordata );
		void SetAngleStat( bool state );

	protected:
		virtual void Process( int delta );

	public:
		virtual void DrawRect();

	private:
		void ClearInfo();

	private:
		//Draw
		image::ImageUnitPtr				m_Image;
		vector<image::ImageUnitPtr>     m_TextBack;
		vector<TextPtr>					m_ToolTipInfoText;
		//info
		vector<string>					m_ToolTipInfos;
		vector<DWORD>					m_ToolTipInfoColors;
		CASHSHOP_ITEMLIST				m_ToolTipItemInfo;
		bool							m_AngleStat;
		int								m_TempPosY;
	};

	inline
	void ToolTipPanel::SetAngleStat( bool state )
	{
		m_AngleStat = state;
	}
};

#endif // !defined(AFX_W_TOOLTIPPANEL_H__34A747D2_EE7C_48F4_9B2A_AD58630BF8DB__INCLUDED_)

#endif //NEW_USER_INTERFACE_UISYSTEM
