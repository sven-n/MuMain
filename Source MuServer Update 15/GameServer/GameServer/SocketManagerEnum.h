#pragma once

enum class ProtocolHead : uint16_t
{
	CLIENT_ACCEPT,			//Enviado pelo cliente
	CLIENT_LIVE_CLIENT,

	SERVER_CONNECT,			//Enviado pelo servidor
	SERVER_DISCONNECT,

	BOTH_CONNECT_LOGIN,			//Enviado pelo cliente e servidor
	BOTH_CONNECT_CHARACTER,
	BOTH_POSITION,
	BOTH_MOVE,
	BOTH_ATTACK1,
	BOTH_ATTACK2,
	BOTH_ATTACK3,

	BOTH_MESSAGE,
};