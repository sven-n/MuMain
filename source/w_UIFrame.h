// w_UIFrame.h: interface for the UIFrame class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UISYSTEM

#if !defined(AFX_W_UIFRAME_H__9A9A206D_DBAB_4490_A22F_D2A545619116__INCLUDED_)
#define AFX_W_UIFRAME_H__9A9A206D_DBAB_4490_A22F_D2A545619116__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_UIWnd.h"

namespace ui
{
	BoostSmartPointer( UIFrame );
	class UISystem;
	class UIFrame : public UIWnd
	{
	public:
		static UIFramePtr Make( const string& uiwndName, bool isModal );
		virtual ~UIFrame();

	public:
		UIWnd& QueryChild( vector<string> childname );
		void SetSystem( UISystem& system );
		bool IsModal() const;
		void Focus();

	public:
		virtual void Draw();
		virtual void Process( int delta );

	private:
		void Process( UIWnd& wnd, int delta );
		void Draw( UIWnd& wnd, eDrawType drawtype );
		UIFrame( const string& uiwndName, bool isModal );

	private:
		UISystem*					m_UISystem;
		bool						m_IsModal;
	};

};

#endif // !defined(AFX_W_UIFRAME_H__9A9A206D_DBAB_4490_A22F_D2A545619116__INCLUDED_)

#endif //NEW_USER_INTERFACE_UISYSTEM