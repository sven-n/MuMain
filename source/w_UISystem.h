#ifdef NEW_USER_INTERFACE_UISYSTEM
// w_UISystem.h: interface for the UISystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_W_UISYSTEM_H__9095E81A_1DED_4D11_8922_1DD7AA7AB11E__INCLUDED_)
#define AFX_W_UISYSTEM_H__9095E81A_1DED_4D11_8922_1DD7AA7AB11E__INCLUDED_

#include "w_UIFrame.h"
#include "w_UIWnd.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace ui
{
	BoostSmartPointer( UISystem );
	class UISystem : public InputMessageHandler
	{
	public:
		static UISystemPtr MakeUISystem();
		virtual ~UISystem();

	public:
		BoostSmart_Ptr(UIFrame) QueryFrame( const string& name );
		void SubFrame( const string& frameName );
		void AddFrame( BoostSmart_Ptr(UIFrame) frame );
		void InsertFrame( const string& InsertframeName, BoostSmart_Ptr(UIFrame) frame );
		UIFrame& GetFrame( const string& name );
		bool IsFrame( const string& name );

	public:
		void Focus( UIFrame& frame );
		void Focus( BoostSmart_Ptr(UIWnd) wnd );
		
#ifdef _DEBUG
		bool IsDebugKey() const;
		void SetDebugKey( bool isdebugkey );
#endif //_DEBUG

	public:
		void Process( int delta = 0 );
		void Drew();

	public:
		bool IsLock();
		void ToggleLock();

	protected:
		virtual bool HandleInputMessage( eKeyType type, eKeyStateType state, int value );

	private:
		void HandleMousePicking( eEventType type, BoostSmart_Ptr(UIWnd) wnd );
		void HandleMouseOut( const Coord& mousePos );
		void HandleMouseOver( const Coord& mousePos );
		void HandleMouseOver( const Coord& mousePos, UIWnd& parentWnd );

	private:
		void Initialize();
		void Destroy();
		void InitDefaultFrame();
		UISystem();

	private:
		BoostSmart_Ptr(UIWnd) PickWindow( const Coord& pos, eEventType mask );
		BoostSmart_Ptr(UIWnd) PickWindow( const Coord& pos, UIWnd& parentWnd, eEventType mask );

	private:
		typedef list< BoostSmart_Ptr(UIFrame) >   UIFrame_List;
		UIFrame_List   m_FrameList;

		typedef list< BoostWeak_Ptr(UIWnd) >       UIWnd_List;
		UIWnd_List     m_MouseOverWndList;

	private:
		BoostWeak_Ptr(UIWnd)					m_FocusedWnd;
		BoostSmart_Ptr(UIFrame)					m_DefaultFrame;

		UIWnd*									m_PrevLButtonDownedWnd;
		UIWnd*									m_PrevRButtonDownedWnd;

		UIWnd*									m_PrevLButtonDbClickedWnd;
		UIWnd*									m_PrevRButtonDbClickedWnd;

#ifdef _DEBUG
		bool									m_isDebugKey;
#endif //_DEBUG

	private:
		bool									m_Locked;
	};

	inline
	bool UISystem::IsLock()
	{
		return m_Locked;
	}

	inline
	void UISystem::ToggleLock()
	{
		m_Locked ? false : true;
	}
};



#endif // !defined(AFX_W_UISYSTEM_H__9095E81A_1DED_4D11_8922_1DD7AA7AB11E__INCLUDED_)

#endif //NEW_USER_INTERFACE_UISYSTEM