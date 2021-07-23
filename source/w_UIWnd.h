// w_UIWnd.h: interface for the UIWnd class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UISYSTEM

#if !defined(AFX_W_UIWND_H__27DA28D8_254C_4870_8F5D_3FC5F37617F1__INCLUDED_)
#define AFX_W_UIWND_H__27DA28D8_254C_4870_8F5D_3FC5F37617F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_UIEvent.h"

//덩치가 커지면 부분을 나눠서 관리 하자.
//논리적인 부분이 상속 받을때와, 물리적인 부분이 상속 받을때를 나눈다.

namespace ui
{
	BoostSmartPointer( UIWnd );
	class UIWnd  
	{
		friend class UISystem;
		friend class UIFrame;

	public:
		UIWnd( const string& uiName );
		virtual ~UIWnd();

	public:
		void AddChild( BoostSmart_Ptr(UIWnd) childWnd );
		void SudChild( const string& wndName );
		void SubChild( int id );
		void ClearChild();
		void SetID( int id );

	public:
		UIWnd& GetChild( const string& name );
		UIWnd& GetChild( int id );

		template<typename T> 
		T& GetChild( const string& name ) {
			return static_cast<T&>( GetChild( name ) );
		}
		
		template<typename T>
		T& GetChild( int id ) {
			return static_cast<T&>( GetChild( id ) );
		}

		const string& GetName() const;
		int GetID() const;
		int GetChildCount() const;
		void Activate( bool b );
		
		void EnableEvent( eEventType type, bool b );
		void EnableEvents( vector<eEventType> types, bool b );
		void EnableLButtonEvent( bool b );
		void EnableRButtonEvent( bool b );

		void ChangeOpacity( BYTE b, bool bRecursive = false );
		void ChangeOpacityByPercent( float f, bool bRecursive = false );
		BYTE GetOpacity() const;
		void LockOpacity( bool b );
    
		bool IsActivated() const;
		bool IsFocused() const;
		bool IsMouseOvered() const;
		bool IsChild( const string& name ) const;
		bool IsChild( int id ) const;
		bool IsDrawEvent( eDrawType drawtype );

	public:
		void Move( const Coord& pos );
		void Resize( const Dim& dim );

		const Coord& GetScreenPosition() const;
		const Coord& GetPosition() const;
		const Dim& GetSize() const;

	private:
		void Destroy();
		void Initialize();

	protected://virtual Event FUN Class로 빼는 방법도 생각 해보자.
		   //다중 상속이 되더라도 코드라 깔끔해진다.
		virtual void OnInitialize();
		virtual void OnDestory();

		//Focus Event
		virtual void OnFocus();
		virtual void OnKillFocus();

		//Mouse Point Event
		virtual void OnMouseMove( const Coord& pos );
		virtual void OnMouseOver();
		virtual void OnMouseOut();

		//Lbutton Event
		virtual void OnLButtonDown();
		virtual void OnLButtonUp();
		virtual void OnLButtonClk();
		virtual void OnLButtonDBClk();

		//Rbutton Event
		virtual void OnRButtonDown();
		virtual void OnRButtonUp();
		virtual void OnRButtonClk();
		virtual void OnRButtonDBClk();

		//Wheel Event
		virtual void OnWheelScroll( int delta );
		virtual void OnWheelButtonDown( int delta );
		virtual void OnWheelButtonUp( int delta );
		virtual void OnWheelButtonClk( int delta );
		virtual void OnWheelButtonDBClk( int delta );

		//Keyboard Event
		virtual void OnTab();
		virtual void OnEnter();
		virtual void OnEscape();

		//Child Event
		virtual void OnAddChild( const string& childName );
		virtual void OnSubChild( const string& childName );
		virtual void OnSubChild( int id );
		virtual void OnClearChild();

		//Opacity Event
		virtual void OnChangeOpacity( BYTE prevOp );

		//Move, Size Event
		virtual void OnMove( const Coord& pos );
		virtual void OnResize( const Dim& dim );

		virtual void DrawImage();
		virtual void DrawMesh();
		virtual void DrawViewPortMesh();
		virtual void DrawRect();

		virtual void Process( int delta );
		
	public:
		template<typename CLASS, typename FUNC> 
		inline
		void RegisterHandler( eEventType type, BoostWeak_Ptr(CLASS) handler, FUNC func )
		{
			m_Eventsarray[type] = BoostSmart_Ptr(Event)( new Event( new Event::T<CLASS,FUNC>( handler, func ) ) );
		}

		inline
		void UnregisterHandler( eEventType type )
		{
			m_Eventsarray[type].reset();
		}

	public:
		void RegisterDrawEvent( eDrawType drawtype );
		void UnRegisterDrawEvent( eDrawType drawtype );
		void ClearDrawEvent();

	protected:
		void NotifyToHandler( eEventType type );
		void CalculateScreenPosition();

	protected:
		typedef list< eDrawType >											DrawEvent_List;
		typedef list< BoostSmart_Ptr(UIWnd) >								ChildUIWnd_List;
		typedef array< BoostSmart_Ptr(Event), eEventTypeMax >				Event_Array;
		
	protected:
		struct sScreenPosition
		{
			Coord									pos;
			BoostWeak_Ptr(sScreenPosition)			parentPos;
		};

		//event
		array<bool, eEventTypeCount+1>								m_EnableEvents;
		//Point 절대좌표
		BoostSmart_Ptr(sScreenPosition)								m_ScreenPosition;
		//Discern
		string														m_Name;
		int															m_ID;
		//Point 상대좌표
		Coord														m_Position;
		Dim															m_Size;
		//Child Wnd
		ChildUIWnd_List												m_Childlist;
		//Event
		Event_Array													m_Eventsarray;
		//DrawEvent
		DrawEvent_List											    m_DrawEventslist;

	private:
		//Opacity
		BYTE														m_Opacity;
		bool														m_LockedOpacity;

		bool														m_Activate;
		bool														m_MouseOvered;
		bool														m_Focused;
	};

	inline
	const Coord& UIWnd::GetScreenPosition() const
	{
		return m_ScreenPosition->pos;
	}

	inline
	const Coord& UIWnd::GetPosition() const
	{
		return m_Position;
	}

	inline
	const Dim& UIWnd::GetSize() const
	{
		return m_Size;
	}

	inline
	void UIWnd::SetID( int id )
	{
		m_ID = id;
	}

	inline
	int UIWnd::GetID() const
	{
		return m_ID;
	}

	inline
	const string& UIWnd::GetName() const
	{
		return m_Name;
	}

	inline
	int UIWnd::GetChildCount() const
	{
		return static_cast<int>( m_Childlist.size() );
	}

	inline
	void UIWnd::RegisterDrawEvent( eDrawType drawtype )
	{
		m_DrawEventslist.push_back( drawtype );
	}

	inline
	void UIWnd::ClearDrawEvent()
	{
		m_DrawEventslist.clear();
	}
};


#endif // !defined(AFX_W_UIWND_H__27DA28D8_254C_4870_8F5D_3FC5F37617F1__INCLUDED_)

#endif //NEW_USER_INTERFACE_UISYSTEM
