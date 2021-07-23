// w_WorldServerProxy.h: interface for the WorldServerProxy class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_PROXY 

#if !defined(AFX_W_WORLDSERVERPROXY_H__0B29E2D6_83C8_4BB7_AC9F_FE8E704A1539__INCLUDED_)
#define AFX_W_WORLDSERVERPROXY_H__0B29E2D6_83C8_4BB7_AC9F_FE8E704A1539__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_InputProxy.h"
#include "w_ShopServerProxy.h"
#include "w_GameServerProxy.h"
#include "w_ServerProxy.h"

BoostSmartPointer( ServerProxySystem );
class ServerProxySystem : public ServerProxy
{
public:
	static BoostSmart_Ptr(ServerProxySystem) Make();
	virtual ~ServerProxySystem();	

public:
	InputProxy&	GetInputProxy();
	GameServerProxy& GetGameServerProxy();
	ShopServerProxy& GetShopServerProxy();

private:
	void Initialize( BoostWeak_Ptr(ServerProxySystem) proxy );
	void Destroy();
	ServerProxySystem();

private:
	InputProxyPtr					m_InputProxy;

	//GameTask에 두개를 붙여주자..
	ShopServerProxyPtr				m_ShopServerProxy;
	GameServerProxyPtr				m_GameServerProxy;
};

inline
InputProxy& ServerProxySystem::GetInputProxy()
{
	assert( m_InputProxy );
    return *m_InputProxy;
}

inline
ShopServerProxy& ServerProxySystem::GetShopServerProxy()
{
	assert( m_ShopServerProxy );
    return *m_ShopServerProxy;
}

inline
GameServerProxy& ServerProxySystem::GetGameServerProxy()
{
	assert( m_GameServerProxy );
    return *m_GameServerProxy;
}

#endif // !defined(AFX_W_WORLDSERVERPROXY_H__0B29E2D6_83C8_4BB7_AC9F_FE8E704A1539__INCLUDED_)

#endif //NEW_USER_INTERFACE_PROXY