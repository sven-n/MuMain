// w_UISystem.cpp: implementation of the UISystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_UISystem.h"

#ifdef NEW_USER_INTERFACE_UISYSTEM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace ui;

namespace 
{
	eEventType GetEventType( eKeyType type, eKeyStateType state )
	{
		switch( type )
		{
		case eKey_LButton:
			{
				switch( state )
				{
				case eKeyState_Down:     return eEvent_LButtonDown;
				case eKeyState_Up:       return eEvent_LButtonUp;
				case eKeyState_DblClk:   return eEvent_LButtonDbClk;
				}
			}
			break;
		case eKey_RButton:
			{
				switch( state )
				{
				case eKeyState_Down:     return eEvent_RButtonDown;
				case eKeyState_Up:       return eEvent_RButtonUp;
				case eKeyState_DblClk:   return eEvent_RButtonDbClk;
				}
			}
			break;
		}       
		return eEvent_None;
	}

	bool IsMousePickableEvent( eEventType type )
	{
		switch( type )
		{
		case eEvent_LButtonDown:
		case eEvent_LButtonUp:
		case eEvent_LButtonDbClk:
		case eEvent_RButtonDown:
		case eEvent_RButtonUp:
		case eEvent_RButtonDbClk:
			return true;
		}
		return false;
	}
}

UISystemPtr UISystem::MakeUISystem()
{
	UISystemPtr ui( new UISystem );
	return ui;
}

UISystem::UISystem() : m_Locked( false ), m_PrevLButtonDownedWnd( NULL ), m_PrevRButtonDownedWnd( NULL )
, m_PrevLButtonDbClickedWnd( NULL ), m_PrevRButtonDbClickedWnd( NULL )
#ifdef _DEBUG
, m_isDebugKey( false )
#endif //_DEBUG
{
	Initialize();	
}

UISystem::~UISystem()
{
	Destroy();
}

void UISystem::Initialize()
{
	InitDefaultFrame();
}

void UISystem::Destroy()
{
	for( UIFrame_List::iterator iter = m_FrameList.begin(); iter != m_FrameList.end(); )
    {
		UIFrame_List::iterator curiter = iter;
		++iter;
        BoostSmart_Ptr(UIFrame) frame = *curiter;

        frame->Destroy();
		frame.reset();
    }

    m_FrameList.clear();
}

void UISystem::AddFrame( BoostSmart_Ptr(UIFrame) frame )
{
	for( UIFrame_List::iterator iter = m_FrameList.begin()
		; iter != m_FrameList.end(); ++iter )
	{
		if( (**iter).GetName() == frame->GetName() )
		{
			assert( 0 );
		}
	}

    frame->SetSystem( *this );
    m_FrameList.push_front( frame );

	if( frame->IsModal() )
	{
	    for( UIWnd_List::reverse_iterator iter = m_MouseOverWndList.rbegin();
		     iter != m_MouseOverWndList.rend(); )
		{
			UIWnd_List::reverse_iterator curIter = iter;
			++iter;
			BoostWeak_Ptr(UIWnd) wnd = *curIter;

			if( wnd.expired() )
			{
				continue;
			}

			UIWnd& ref = *wnd.lock();

			ref.m_MouseOvered = false;
			// 위험할 수 있다.
			ref.OnMouseOut();
		}

		m_MouseOverWndList.clear();
	}
}

void UISystem::SubFrame( const string& frameName )
{
    for( UIFrame_List::iterator iter = m_FrameList.begin()
        ; iter != m_FrameList.end(); ++iter )
    {
        if( (**iter).GetName() == frameName )
        {
            BoostSmart_Ptr(UIFrame) frame = *iter;
            frame->Destroy();
            m_FrameList.erase( iter );
            return;
        }
    }
}

void UISystem::InsertFrame( const string& InsertframeName, BoostSmart_Ptr(UIFrame) frame )
{
	int i = 0, j = 0;
	bool state = false;

	for( UIFrame_List::iterator iter = m_FrameList.begin()
		; iter != m_FrameList.end(); ++iter )
	{
		if( state )
		{
			m_FrameList.insert( iter, frame );
		}

		if( (**iter).GetName() == InsertframeName )
		{
			state = true;
		}
	}
}

void UISystem::InitDefaultFrame()
{
    m_DefaultFrame = UIFrame::Make("_DEFAULT_FRAME_", false );
    AddFrame( m_DefaultFrame );
}

void UISystem::Focus( UIFrame& frame )
{
    UIFrame_List::iterator frontIter = m_FrameList.begin();

    for( UIFrame_List::iterator iter = m_FrameList.begin()
        ; iter != m_FrameList.end();  )
    {
        UIFrame_List::iterator curIter = iter;
        ++iter;

        if( &frame == (*curIter).get() )
        {
            if( frontIter != curIter )
            {
                BoostSmart_Ptr(UIFrame) curFrame = *curIter;
                (*frontIter)->OnKillFocus();
                (*curIter)->OnFocus();

                m_FrameList.erase( curIter );
                m_FrameList.push_front( curFrame );
                return;
            }
        }
    }
}

void UISystem::Focus( BoostSmart_Ptr(UIWnd) wnd )
{
    if( !m_FocusedWnd.expired() ) 
    {
        if( m_FocusedWnd.lock() != wnd )
        {
            m_FocusedWnd.lock()->m_Focused = false;
            m_FocusedWnd.lock()->OnKillFocus();
        }
    }

    if( wnd )
    {
        if( wnd->m_Focused == false )
        {
            wnd->m_Focused = true;
            wnd->OnFocus();
            
        }
    }

    m_FocusedWnd = wnd;
}

UIFrame& UISystem::GetFrame( const string& name )
{
    for( UIFrame_List::iterator iter = m_FrameList.begin()
       ; iter != m_FrameList.end(); ++iter )
    {
        if( (**iter).GetName() == name )
            return **iter;
    }
    assert( 0 );
    throw;
}

BoostSmart_Ptr(UIFrame) UISystem::QueryFrame( const string& name )
{
	for( UIFrame_List::iterator iter = m_FrameList.begin()
		; iter != m_FrameList.end(); ++iter )
	{
		if( (**iter).GetName() == name )
			return *iter;
	}
	return BoostSmart_Ptr(UIFrame)();
}

bool UISystem::IsFrame( const string& name )
{
    for( UIFrame_List::iterator iter = m_FrameList.begin()
       ; iter != m_FrameList.end(); ++iter )
    {
        if( (**iter).GetName() == name )
            return true;
    }

    return false;
}

BoostSmart_Ptr(UIWnd) UISystem::PickWindow( const Coord& pos, eEventType mask )
{
    for( UIFrame_List::iterator iter = m_FrameList.begin()
        ; iter != m_FrameList.end();  )
    {
        UIFrame_List::iterator curIter = iter;
        ++iter;
        BoostSmart_Ptr(UIFrame) curFrame = *curIter;
        UIWnd& frame = **curIter;
        
        if( frame.m_EnableEvents[mask] == false || frame.IsActivated() == false )
            continue;

        Rect r( frame.GetScreenPosition(), frame.GetScreenPosition() + frame.GetSize() );
        if( r.IsIncluded( pos ) )
        {
            BoostSmart_Ptr(UIWnd) wnd = PickWindow( pos, **curIter, mask );
            if( wnd )
            {
                if( mask == eEvent_LButtonDown )
                {
                    // frame에는 포커스 메세지를 보내준다.

                    UIFrame_List::iterator frontIter = m_FrameList.begin();
                    if( frontIter != curIter )
                    {
                        (*frontIter)->OnKillFocus();
                        (*curIter)->OnFocus();

                        m_FrameList.erase( curIter );
                        m_FrameList.push_front( curFrame );
                    }

                    Focus( wnd );
                }
                return wnd;
            }
        }
        if( curFrame->IsModal() == true )
            break;
    }

    Focus( BoostSmart_Ptr(UIWnd)() );
    return BoostSmart_Ptr(UIWnd)();
}

BoostSmart_Ptr(UIWnd) UISystem::PickWindow( const Coord& pos, UIWnd& parentWnd, eEventType mask )
{
    for( UIWnd::ChildUIWnd_List::reverse_iterator iter = parentWnd.m_Childlist.rbegin()
        ; iter != parentWnd.m_Childlist.rend(); ++iter )
    {
        UIWnd& childWnd = *(*iter);

        if( childWnd.m_EnableEvents[mask] == false || childWnd.IsActivated() == false )
            continue;

        Rect r( childWnd.GetScreenPosition(), childWnd.GetScreenPosition() + childWnd.GetSize() );
        if( r.IsIncluded( pos ) )
        {
            BoostSmart_Ptr(UIWnd) pickedWnd = PickWindow( pos, childWnd, mask );
            if( pickedWnd )
                return pickedWnd;
            else
                return (*iter);
        }
    }
    return BoostSmart_Ptr(UIWnd)();
}

void UISystem::HandleMousePicking( eEventType type, BoostSmart_Ptr(UIWnd) wnd )
{
    switch( type )
    {
    case eEvent_LButtonDown:
	    m_PrevLButtonDownedWnd = wnd.get();
		wnd->OnLButtonDown();
        break;

    case eEvent_LButtonUp:
        wnd->OnLButtonUp();
        if( m_PrevLButtonDbClickedWnd == wnd.get() )
        {
			wnd->OnLButtonDBClk();
        }
        else if( m_PrevLButtonDownedWnd == wnd.get() )
        {
			wnd->OnLButtonClk();
        }
        m_PrevLButtonDownedWnd      = NULL;
        m_PrevLButtonDbClickedWnd   = NULL;
        break;

    case eEvent_LButtonDbClk:
        m_PrevLButtonDbClickedWnd = wnd.get();
        wnd->OnLButtonDown();
        break;

    case eEvent_RButtonDown:
        m_PrevRButtonDownedWnd = wnd.get();
        wnd->OnRButtonDown();
        break;

    case eEvent_RButtonUp:
        wnd->OnRButtonUp();
        if( m_PrevRButtonDbClickedWnd == wnd.get() )
        {
			wnd->OnRButtonDBClk();
        }
        else if( m_PrevRButtonDownedWnd == wnd.get() )
        {
			wnd->OnRButtonClk();
        }
        m_PrevRButtonDownedWnd      = NULL;
        m_PrevRButtonDbClickedWnd   = NULL;
        break;

    case eEvent_RButtonDbClk:
        m_PrevRButtonDbClickedWnd = wnd.get();
        wnd->OnRButtonDown();
        break;
    }
}

void UISystem::HandleMouseOut( const Coord& mousePos )
{
    for( UIWnd_List::reverse_iterator iter = m_MouseOverWndList.rbegin()
       ; iter != m_MouseOverWndList.rend(); )
    {
        UIWnd_List::reverse_iterator curIter = iter;
        ++iter;
        BoostWeak_Ptr(UIWnd) wnd = *curIter;

        if( wnd.expired() )
        {
            continue;
        }

        UIWnd& ref = *wnd.lock();

        Rect r( ref.GetScreenPosition(), ref.GetScreenPosition() + ref.GetSize() );

        if( !r.IsIncluded( mousePos ) )
        {
            ref.m_MouseOvered = false;
            // 위험할 수 있다.
            ref.OnMouseOut();
        }
        else
        {
            ref.OnMouseMove( mousePos );
        }
    }

    m_MouseOverWndList.clear();
}

void UISystem::HandleMouseOver( const Coord& mousePos )
{
    for( UIFrame_List::iterator iter = m_FrameList.begin()
        ; iter != m_FrameList.end(); ++iter )
    {
        UIFrame& frame = **iter;

		if( frame.m_EnableEvents[eEvent_MouseOver] == false || frame.IsActivated() == false )
			continue;

		Rect r( frame.GetScreenPosition(), frame.GetScreenPosition() + frame.GetSize() );
		if( r.IsIncluded( mousePos ) )
		{
			HandleMouseOver( mousePos, frame );           
		}

		if( frame.IsModal() == true )
		{
			break;
		}
    }
}

void UISystem::HandleMouseOver( const Coord& mousePos, UIWnd& parentWnd )
{
    for( UIWnd::ChildUIWnd_List::reverse_iterator iter = parentWnd.m_Childlist.rbegin()
        ; iter != parentWnd.m_Childlist.rend(); ++iter )
    {
        UIWnd& childWnd = *(*iter);

        if( childWnd.m_EnableEvents[eEvent_MouseOver] == false || childWnd.IsActivated() == false )
            continue;

        Rect r( childWnd.GetScreenPosition(), childWnd.GetScreenPosition() + childWnd.GetSize() );
        if( r.IsIncluded( mousePos ) )
        {
            if( childWnd.m_MouseOvered == false )
            {
                childWnd.m_MouseOvered = true;
                // 위험할 수 있다.
                childWnd.OnMouseOver();
            }
            m_MouseOverWndList.push_back( (*iter) );
            HandleMouseOver( mousePos, childWnd );
            return;
        }
    }
}


bool UISystem::HandleInputMessage( eKeyType type, eKeyStateType state, int value )
{
	//Lock일때는 Mouse Message를 받지 않는다.
	if( m_Locked ) return false;

	switch( type )
	{
    case eKey_LButton:
	case eKey_RButton:
		{
			//Mouse Pick Event
			Coord mousePos( value );
            eEventType eventType = GetEventType( type, state );
            if( IsMousePickableEvent( eventType ) )
            {
                BoostSmart_Ptr(UIWnd) pickedWnd = PickWindow( mousePos, eventType );
                if( pickedWnd )
                {
                    HandleMousePicking( eventType, pickedWnd );
                    return true;
                }
            }
		}
		break;
    case eKey_MouseMove:
		{
			//Mouse Over && Out Event
			Coord mousePos( value );
            HandleMouseOut( mousePos );
            HandleMouseOver( mousePos );
		}
		break;
    case eKey_Wheel:
		{
			for( UIWnd_List::reverse_iterator iter = m_MouseOverWndList.rbegin()
                ; iter != m_MouseOverWndList.rend(); )
            {
                UIWnd_List::reverse_iterator curIter = iter;
                ++iter;
                BoostWeak_Ptr(UIWnd) wnd = *curIter;

                if( wnd.expired() )
                {
                    continue;
                }

                UIWnd& ref = *wnd.lock();
				ref.OnWheelScroll( value );
            }
		}
		break;
	case eKey_Enter:
		{
			if( state == eKeyState_Down )
			{
				if( !m_FocusedWnd.expired() )
				{
					m_FocusedWnd.lock()->OnEnter();
				}
			}
		}
		break;
	case eKey_Tab:
		{
			if( state == eKeyState_Down )
			{
				if( !m_FocusedWnd.expired() )
				{
					m_FocusedWnd.lock()->OnTab();
				}
			}
		}
		break;

#ifdef _DEBUG
	case eKey_F8:
		{
			if( state == eKeyState_Down )
			{
				m_isDebugKey = IsDebugKey() ? false : true;
			}
		}
		break;
#endif //_DEBUG
	}

	return false;
}

void UISystem::Process( int delta )
{
	for( UIFrame_List::reverse_iterator iter = m_FrameList.rbegin()
       ; iter != m_FrameList.rend(); ++iter )
    {
        (**iter).Process( delta );
    }
}

void UISystem::Drew()
{
    for( UIFrame_List::reverse_iterator iter = m_FrameList.rbegin()
       ; iter != m_FrameList.rend(); ++iter )
    {
        (**iter).Draw();
    }
}

#ifdef _DEBUG
bool UISystem::IsDebugKey() const
{
	return m_isDebugKey;
}
void UISystem::SetDebugKey( bool isdebugkey )
{
	m_isDebugKey = isdebugkey;
}
#endif //_DEBUG

#endif //NEW_USER_INTERFACE_UISYSTEM