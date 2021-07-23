#pragma once

#include "stdafx.h"

#ifdef NEW_USER_INTERFACE_PROXY

enum eProxyType
{
	eType_Input,
	eType_Shop,
};

//여기다가 SendMessage, ReceiveMessage를 만들어 보자.
struct ServerProxy
{
    virtual ~ServerProxy() = 0{}
};

template< typename T>
struct LocalServerProxy : public util::EventPublisher<T>
{    
    virtual ~LocalServerProxy() = 0{}
    void RegisterMainServerProxy( BoostWeak_Ptr(ServerProxy) proxy );

private:
    BoostWeak_Ptr(ServerProxy)   m_MainServerProxy;
};

template<typename T> inline
void LocalServerProxy<T>::RegisterMainServerProxy( BoostWeak_Ptr(ServerProxy) proxy )
{
    m_MainServerProxy = proxy;
}

#endif //NEW_USER_INTERFACE_PROXY