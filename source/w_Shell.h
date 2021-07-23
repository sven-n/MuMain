// w_Shell.h: interface for the Shell class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_SHELL 

#if !defined(AFX_W_SHELL_H__2ECDDA4C_D3F9_4B28_B277_786B1998587A__INCLUDED_)
#define AFX_W_SHELL_H__2ECDDA4C_D3F9_4B28_B277_786B1998587A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//새로운 스스템 관리자
#include "w_BuildSystem.h"
#include "w_ServerProxySystem.h"
#include "w_InputSystem.h"
#include "w_UISystem.h"
#include "w_ClientSystem.h"

BoostSmartPointer( Shell );
class Shell  
{
public:	
	static ShellPtr Make();
	void Initialize( HWND windowHandle, int windowWidth, int windowHeight );
	virtual ~Shell();
	Shell();

public:
	ui::UISystem&			GetUISystem();
	input::InputSystem&		GetInputSystem();
	ServerProxySystem&		GetServerProxySystem();
	ClientSystem&			GetClientSystem();
	BuildSystem&			GetBuildSystem();
	
public:
	const Dim&				GetWindowSize() const;
	
public:
	bool HandleWindowMessage( UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result );
	void RegisterHandler( BoostWeak_Ptr(WindowMessageHandler) wndmsghld );
	void UnregisterHandler();

private:
	void MakeBuildSystem();
	void MakeUISystem();
	void MakeInputSystem( HWND windowHandle, const Dim& windowsize );
	void MakeServerProxySystem();
	void MakeClientSystem();
	void Destroy();

private:
	typedef	list< BoostWeak_Ptr(WindowMessageHandler) >	WndMsgHldList;

private:
	WndMsgHldList							m_lWndMsgHld;

private:
	//system
	ui::UISystemPtr							m_cUISystem;
	input::InputSystemPtr					m_cInputSystem;
	ServerProxySystemPtr					m_cSerVerProxySystem;
	ClientSystemPtr							m_cClientSystem;
	BuildSystemPtr							m_cBuildSystem;
	Dim										m_WindowSize;
};

inline
BuildSystem& Shell::GetBuildSystem()
{
	assert( m_cBuildSystem );
	return *m_cBuildSystem;
}

inline
ui::UISystem& Shell::GetUISystem()
{
	assert( m_cUISystem );
	return *m_cUISystem;
}

inline
input::InputSystem&	Shell::GetInputSystem()
{
	assert( m_cInputSystem );
	return *m_cInputSystem;
}

inline
ServerProxySystem& Shell::GetServerProxySystem()
{
	assert( m_cSerVerProxySystem );
	return *m_cSerVerProxySystem;
}

inline
ClientSystem& Shell::GetClientSystem()
{
	assert( m_cClientSystem );
	return *m_cClientSystem;
}

inline
const Dim& Shell::GetWindowSize() const
{
	return m_WindowSize;
}

#endif // !defined(AFX_W_SHELL_H__2ECDDA4C_D3F9_4B28_B277_786B1998587A__INCLUDED_)

#endif //NEW_USER_INTERFACE_SHELL