// w_GameServerProxy.h: interface for the GameServerProxy class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_PROXY

#if !defined(AFX_W_GAMESERVERPROXY_H__DE948B9E_9163_4D4C_9B57_A9B95DF555C6__INCLUDED_)
#define AFX_W_GAMESERVERPROXY_H__DE948B9E_9163_4D4C_9B57_A9B95DF555C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_ServerProxy.h"

//Handler에 Virtual 함수만 추가 하자.
//순수 가장 함수로 만들지 말자.--;;;
struct GameServerProxyHandler 
{
	virtual ~GameServerProxyHandler() = 0 {}
};

BoostSmartPointer( GameServerProxy );
class GameServerProxy : public MESSAGEBOXHANDLER, public LocalServerProxy<GameServerProxyHandler>
{
public:
	static GameServerProxyPtr Make( BoostWeak_Ptr(ServerProxy) proxy );
	virtual ~GameServerProxy();

private:
	void Initialize( BoostWeak_Ptr(GameServerProxy) proxy );
	void Destroy();
	GameServerProxy();

protected:
	virtual void OnMessageBox_OK( int id );
	
#ifdef PSW_CHARACTER_CARD
public:
	void ReceiveBuyCharacterCard( BYTE* ReceiveBuffer );
	void ReceiveCharacterCard( BYTE* ReceiveBuffer );

public:
	const bool IsCharacterCard() { return m_IsCharacterCard; }

private:
	BoostWeak_Ptr(GameServerProxy)      m_handler;
	bool								m_IsCharacterCard;
#endif //PSW_CHARACTER_CARD
};

#endif // !defined(AFX_W_GAMESERVERPROXY_H__DE948B9E_9163_4D4C_9B57_A9B95DF555C6__INCLUDED_)

#endif //NEW_USER_INTERFACE_PROXY