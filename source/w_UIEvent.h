#pragma once 

#ifdef NEW_USER_INTERFACE_UISYSTEM

namespace ui
{

class UIWnd;
class Event 
{
private:
    struct Base 
	{
        virtual ~Base() {};
        virtual void operator()( UIWnd& wnd ) = 0;
    };

    Base* pBase;

public:
	template <typename CLASS, typename FUNC >
	struct T : public Base 
	{
		BoostWeak_Ptr(CLASS)  m_Handler;
		FUNC    m_Func;

		T( BoostWeak_Ptr(CLASS) handler, FUNC func ) : m_Handler( handler ), m_Func( func )
		{
		}

		virtual void operator()( UIWnd& wnd ) 
		{
			if( !m_Handler.expired() )
			{
				CLASS* pHandler = m_Handler.lock().get();
				(pHandler->*m_Func)( wnd );
			}
		}
	};

public:
    Event( Base* base ) : pBase( base ) 
	{
	}

    ~Event() 
	{
		delete pBase; 
	}

    void operator () ( UIWnd& wnd ) 
	{
        (*pBase)( wnd );
    }
};

};

#endif //NEW_USER_INTERFACE_UISYSTEM