// w_GameServerProxy.cpp: implementation of the GameServerProxy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_GameServerProxy.h"

#ifdef NEW_USER_INTERFACE_PROXY

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GameServerProxyPtr GameServerProxy::Make( BoostWeak_Ptr(ServerProxy) proxy )
{
	BoostSmart_Ptr(GameServerProxy) serverproxy( new GameServerProxy() );
	serverproxy->Initialize( serverproxy );
	serverproxy->RegisterMainServerProxy( proxy );
	return serverproxy;
}

GameServerProxy::GameServerProxy()
{
#ifdef PSW_CHARACTER_CARD
	m_IsCharacterCard = false;
#endif //PSW_CHARACTER_CARD
}

GameServerProxy::~GameServerProxy()
{
	Destroy();
}

void GameServerProxy::Initialize( BoostWeak_Ptr(GameServerProxy) proxy )
{
#ifdef PSW_CHARACTER_CARD
	m_handler = proxy;
#endif //PSW_CHARACTER_CARD
}

void GameServerProxy::Destroy()
{
	
}

void GameServerProxy::OnMessageBox_OK( int id )
{
	
}

#ifdef PSW_CHARACTER_CARD
void GameServerProxy::ReceiveBuyCharacterCard( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_SETACCOUNTINFO Data = (LPPMSG_ANS_SETACCOUNTINFO)ReceiveBuffer;
	m_IsCharacterCard = Data->s_bResult;

	MESSAGEBOXDATA data;
	if( m_IsCharacterCard ) {
		data.s_message.push_back( GlobalText[2551] );
	}
	else{
		data.s_message.push_back( GlobalText[2554] );
	}

	//data.s_id      = id;
	data.s_handler = m_handler;
	data.s_type    = eMB_OK;
	MessageBoxMake( data );
}

void GameServerProxy::ReceiveCharacterCard( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_ACCOUNTINFO Data = (LPPMSG_ANS_ACCOUNTINFO)ReceiveBuffer;
	m_IsCharacterCard = Data->s_bSummoner;
}
#endif //PSW_CHARACTER_CARD

#endif //NEW_USER_INTERFACE_PROXY