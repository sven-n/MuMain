// w_ItemModuleView.h: interface for the ItemModuleView class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UISYSTEM

#if !defined(AFX_W_ITEMMODULEVIEW_H__E8469CB5_5FEB_49FE_8538_DC5ACA95C956__INCLUDED_)
#define AFX_W_ITEMMODULEVIEW_H__E8469CB5_5FEB_49FE_8538_DC5ACA95C956__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_ImageUnit.h"
#include "w_UIWnd.h"
#include "w_Text.h"
#include "w_ToolTipPanel.h"

/*
View를 잡아서 아이템을 랜더링 하는게 아니라 화면 전체를 다시 뷰를 잡아 그린다...
개당 하나의 아이템이라고 생각 하지 말자. 요것도 죄송..ㅜ..ㅜ
*/

namespace ui
{
	BoostSmartPointer( ItemModuleView );
	class ItemModuleView : public UIWnd
	{
	public:
		ItemModuleView( const string& name );
		virtual ~ItemModuleView();
		void SetToolTipItem( const CASHSHOP_ITEMLIST& tooltipiteminfo );

	protected:
		virtual void OnMove( const Coord& prevPos );
		virtual void OnResize( const Dim& prevSize );

		virtual void OnMouseOver();
		virtual void OnMouseOut();

	protected:
		virtual void DrawMesh();
		virtual void Process( int delta );

	private:
		void Initialize();
		void Destroy();

	private:
		ToolTipPanelPtr					m_ToolTipPanel;
		int								m_ToolTipInfoSize;
		int								m_Type;
		int								m_Level;
		int								m_AddOption;
		int								m_ExOption;
		bool							m_AngleStat;
	};
};

#endif // !defined(AFX_W_ITEMMODULEVIEW_H__E8469CB5_5FEB_49FE_8538_DC5ACA95C956__INCLUDED_)

#endif //NEW_USER_INTERFACE_UISYSTEM