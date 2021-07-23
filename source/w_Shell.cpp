// w_Shell.cpp: implementation of the Shell class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "textbitmap.h"
#include "w_Shell.h"

#ifdef NEW_USER_INTERFACE_SHELL

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ShellPtr Shell::Make()
{
	ShellPtr system( new Shell() );
	return system;
}

Shell::Shell()
{

}

Shell::~Shell()
{
	Destroy();
}

void Shell::Initialize( HWND windowHandle, int windowWidth, int windowHeight )
{
	CTextBitmap::Instance().Create();
	m_WindowSize = Dim( windowWidth, windowHeight );

	//local file load
	MakeBuildSystem();
	MakeServerProxySystem();
	MakeInputSystem( windowHandle, m_WindowSize );
	MakeUISystem();
	MakeClientSystem();
}

void Shell::Destroy()
{
	CTextBitmap::Instance().Release();
	m_cClientSystem.reset();
	m_cUISystem.reset();
	m_cInputSystem.reset();
	m_cSerVerProxySystem.reset();
	m_cBuildSystem.reset();
}

void Shell::MakeUISystem()
{
	m_cUISystem = ui::UISystem::MakeUISystem();
	m_cInputSystem->RegisterInputMessageHandler( m_cUISystem );
	
}

void Shell::MakeInputSystem( HWND windowHandle, const Dim& windowsize )
{
	m_cInputSystem = input::InputSystem::Make( windowHandle );
	m_cSerVerProxySystem->GetInputProxy().RegisterHandler( m_cInputSystem );
	RegisterHandler( m_cInputSystem );
	m_cInputSystem->SetWindowSize( windowsize );
}

void Shell::MakeClientSystem()
{
	m_cClientSystem = ClientSystem::Make();
}

void Shell::MakeServerProxySystem()
{
	m_cSerVerProxySystem = ServerProxySystem::Make();
}

void Shell::MakeBuildSystem()
{
	m_cBuildSystem = BuildSystem::Make();
}

void Shell::RegisterHandler( BoostWeak_Ptr(WindowMessageHandler) wndmsghld )
{
	m_lWndMsgHld.push_back( wndmsghld );
}

bool Shell::HandleWindowMessage( UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result )
{
    for( WndMsgHldList::iterator iter = m_lWndMsgHld.begin(); iter != m_lWndMsgHld.end(); )
    {
		WndMsgHldList::iterator curIter = iter;
		++iter;
        BoostWeak_Ptr(WindowMessageHandler) handler = *curIter;
        
        if( handler.expired() )
        {
            m_lWndMsgHld.erase( curIter );
            continue;
        }

        if( handler.lock()->HandleWindowMessage( message, wParam, lParam, result ) == true )
            return true;
    }

	return false;
}

#endif //NEW_USER_INTERFACE_SHELL