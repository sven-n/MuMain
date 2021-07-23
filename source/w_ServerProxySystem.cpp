// w_WorldServerProxy.cpp: implementation of the WorldServerProxy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_ServerProxySystem.h"

#ifdef NEW_USER_INTERFACE_PROXY

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ServerProxySystemPtr ServerProxySystem::Make()
{
	ServerProxySystemPtr system( new ServerProxySystem() );
	system->Initialize( system );
	return system;
}

ServerProxySystem::ServerProxySystem()
{
}

ServerProxySystem::~ServerProxySystem()
{
	Destroy();
}

void ServerProxySystem::Initialize( BoostWeak_Ptr(ServerProxySystem) proxy )
{
	//Proxy들을 만들자.
	m_InputProxy = InputProxy::Make( proxy );
	m_GameServerProxy = GameServerProxy::Make( proxy );
	m_ShopServerProxy = ShopServerProxy::Make( proxy );
}

void ServerProxySystem::Destroy()
{

}

#endif //NEW_USER_INTERFACE_PROXY