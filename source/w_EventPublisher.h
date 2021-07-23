#pragma once

#include "stdafx.h"

#ifdef NEW_USER_INTERFACE_UTIL

namespace util
{

template<typename T>
struct EventPublisher
{
    virtual ~EventPublisher(void) = 0{}

	inline
    void RegisterHandler( BoostWeak_Ptr(T) handler )
	{
		m_HandlerList.push_back( handler );
	}

	inline
	void UnregisterHandler( T* handler )
	{
		for( HandlerList::iterator iter = m_HandlerList.begin()
		   ; iter != m_HandlerList.end(); )
		{
			HandlerList::iterator curIter = iter;
			++iter;
			BoostWeak_Ptr(T) wHandler = *curIter;
			if( wHandler.expired() )
			{
				m_HandlerList.erase( curIter );
			}
			else if( wHandler.lock().get() == handler )
			{
				m_HandlerList.erase( curIter );
			}
		}
	}

    template<typename FUNC>
	inline
    void    NotifyToHandler( FUNC func )
	{
		for( HandlerList::iterator iter = m_HandlerList.begin()
			; iter != m_HandlerList.end(); )
		{
			HandlerList::iterator curIter = iter;
			++iter;
			BoostWeak_Ptr(T) handler = *curIter;
			if( handler.expired() )
			{
				m_HandlerList.erase( curIter );
			}
			else
			{
				(handler.lock().get()->*func)();
			}
		}
	}

    template<typename ARG1, typename FUNC>
	inline
    void    NotifyToHandler( FUNC func, ARG1 arg1 )
	{
		for( HandlerList::iterator iter = m_HandlerList.begin()
			; iter != m_HandlerList.end(); )
		{
			HandlerList::iterator curIter = iter;
			++iter;
			BoostWeak_Ptr(T) handler = *curIter;
			if( handler.expired() )
			{
				m_HandlerList.erase( curIter );
			}
			else
			{
				(handler.lock().get()->*func)( arg1 );
			}
		}
	}

    template<typename ARG1, typename ARG2, typename FUNC>
	inline
    void    NotifyToHandler( FUNC func, ARG1 arg1, ARG2 arg2 )
	{
		for( HandlerList::iterator iter = m_HandlerList.begin()
			; iter != m_HandlerList.end(); )
		{
			HandlerList::iterator curIter = iter;
			++iter;
			BoostWeak_Ptr(T) handler = *curIter;
			if( handler.expired() )
			{
				m_HandlerList.erase( curIter );
			}
			else
			{
				(handler.lock().get()->*func)( arg1, arg2 );
			}
		}
	}

    template<typename ARG1, typename ARG2, typename ARG3, typename FUNC>
	inline
    void    NotifyToHandler( FUNC func, ARG1 arg1, ARG2 arg2, ARG3 arg3 )
	{
		for( HandlerList::iterator iter = m_HandlerList.begin()
			; iter != m_HandlerList.end(); )
		{
			HandlerList::iterator curIter = iter;
			++iter;
			BoostWeak_Ptr(T) handler = *curIter;
			if( handler.expired() )
			{
				m_HandlerList.erase( curIter );
			}
			else
			{
				(handler.lock().get()->*func)( arg1, arg2, arg3 );
			}
		}
	}

    template<typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename FUNC> 
	inline
    void    NotifyToHandler( FUNC func, ARG1 arg1, ARG2 arg2, ARG3 arg3, ARG4 arg4 )
	{
		for( HandlerList::iterator iter = m_HandlerList.begin()
			; iter != m_HandlerList.end(); )
		{
			HandlerList::iterator curIter = iter;
			++iter;
			BoostWeak_Ptr(T) handler = *curIter;
			if( handler.expired() )
			{
				m_HandlerList.erase( curIter );
			}
			else
			{
				(handler.lock().get()->*func)( arg1, arg2, arg3, arg4 );
			}
		}
	}

    template<typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename FUNC>
	inline
    void    NotifyToHandler( FUNC func, ARG1 arg1, ARG2 arg2, ARG3 arg3, ARG4 arg4, ARG5 arg5 )
	{
		for( HandlerList::iterator iter = m_HandlerList.begin()
			; iter != m_HandlerList.end(); )
		{
			HandlerList::iterator curIter = iter;
			++iter;
			BoostWeak_Ptr(T) handler = *curIter;
			if( handler.expired() )
			{
				m_HandlerList.erase( curIter );
			}
			else
			{
				(handler.lock().get()->*func)( arg1, arg2, arg3, arg4, arg5 );
			}
		}
	}

protected:
    typedef list< BoostWeak_Ptr(T) > HandlerList;

protected:
    HandlerList						m_HandlerList;
};

};

#endif //NEW_USER_INTERFACE_UTIL