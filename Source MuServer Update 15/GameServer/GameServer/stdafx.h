#pragma once

#define WIN32_LEAN_AND_MEAN

#define _WIN32_WINNT _WIN32_WINNT_WINXP


#define GAMESERVER_CLIENT "WWW.CREANDOMU.COM"

#define GAMESERVER_CLIENTE_UPDATE 15
#define GAMESERVER_CLIENTE_PREMIUM 0


#if(GAMESERVER_TYPE==0)
#define GAMESERVER_VERSION "GS"
#else
#define GAMESERVER_VERSION "GSCS"
#endif

#ifndef GAMESERVER_TYPE
#define GAMESERVER_TYPE 0
#endif

#ifndef GAMESERVER_EXTRA
#define GAMESERVER_EXTRA 0
#endif

#ifndef GAMESERVER_UPDATE
#define GAMESERVER_UPDATE 803
#endif

#if(GAMESERVER_UPDATE==401)
#define GAMESERVER_SEASON "SEASON 4"
#endif

#if(GAMESERVER_UPDATE==502)
#define GAMESERVER_SEASON "SEASON 5"
#endif

#if(GAMESERVER_UPDATE==603)
#define GAMESERVER_SEASON "SEASON 6"
#endif

#if(GAMESERVER_UPDATE==803)
#define GAMESERVER_SEASON "SEASON 8"
#endif

#define NEW_PROTOCOL_SYSTEM 1

//#ifndef GAMESERVER_LANGUAGE
#define GAMESERVER_LANGUAGE 1
//#endif

#ifndef PROTECT_STATE
#define PROTECT_STATE 1 
#endif

#ifndef ENCRYPT_STATE
#define ENCRYPT_STATE 0
#endif

#if(PROTECT_STATE==0)
#define GAMESERVER_NAME "Free"
#else
#define GAMESERVER_NAME "Premium"
#endif

//MC bot

//----------------------------
//	 ZG-Defines			     -
//	 1:ON  (CPP AND HEADER!) -
//	 2:OFF (CPP AND HEADER!) -
//----------------------------

#ifndef POINT_CUSTOM 
#define POINT_CUSTOM 1 // ShopPointEx
#endif

#define PC_POINT			 2 //Agregar algun npc o bot con PCP
#define HAPPY_HOURS			 2 //[80%]->Falta Arreglar Los Vip!
#define MASTER_HOURS		 2
#define ACHERON_GUARDIAN	 2

#define BOT_BUFFER			 1

#define ARCA_WAR			 2	// Mix nomas!!
#define GOLDEN_ARCHER		 2	// 


//-- Need All Types
#define PG_CUSTOM			 1
#define	ALLBOTSSTRUC		 1

//MC bot

// System Include
#include <windows.h>
#include <winsock2.h>
#include <mswSock.h>
#include <commctrl.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <map>
#include <vector>
#include <queue>
#include <random>
#include <Rpc.h>
#include <algorithm>
#include <string>
#include <atltime.h>
#include <dbghelp.h>
#include <Psapi.h>
#include <csignal>
#include <thread>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Rpcrt4.lib")
#pragma comment(lib,"dbghelp.lib")
#pragma comment(lib,"Psapi.lib")

#if(GAMESERVER_UPDATE>=701)
#if(NDEBUG==0)
#pragma comment(lib,"Util\\cryptopp\\Debug\\cryptlib.lib")
#else
#pragma comment(lib,"Util\\cryptopp\\Release\\cryptlib.lib")
#endif
#pragma comment(lib,"Util\\mapm\\mapm.lib")
#endif

typedef char chr;

typedef float flt;

typedef short shrt;

typedef unsigned __int64 QWORD;

__pragma(warning(disable:28159))
__pragma(warning(disable:26495))
