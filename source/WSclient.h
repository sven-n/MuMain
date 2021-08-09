#ifndef __SOCKETCLIENT_H__
#define __SOCKETCLIENT_H__

#pragma once

#include "wsctlc.h"
#include "SimpleModulus.h"
#ifdef PKD_ADD_ENHANCED_ENCRYPTION
#include "SessionCryptor.h"
#endif // PKD_ADD_ENHANCED_ENCRYPTION
#include "CSMapServer.h"
//#include "../ProtocolSend.h"

#define WM_ASYNCSELECTMSG (WM_USER+0)

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#define MAX_CHAT_SIZE 90
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#define MAX_CHAT_SIZE 60
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#define SIZE_PROTOCOLVERSION	( 5)
#define SIZE_PROTOCOLSERIAL		( 16)
#define MAX_GUILDNAME			8

#define	REQUEST_JOIN_SERVER      		        0
#define RECEIVE_JOIN_SERVER_WAITING				1	// 인증서버가 바쁘니 대기하라.
#define	RECEIVE_JOIN_SERVER_SUCCESS		        2
#define	RECEIVE_JOIN_SERVER_FAIL_VERSION        3
#define	REQUEST_CREATE_ACCOUNT			        10	// 사용안함.
#define	RECEIVE_CREATE_ACCOUNT_SUCCESS		    11	// 사용안함.
#define	RECEIVE_CREATE_ACCOUNT_FAIL_ID          12	// 사용안함.
#define	RECEIVE_CREATE_ACCOUNT_FAIL_RESIDENT    13	// 사용안함.

#define	REQUEST_LOG_IN					        19
#define	RECEIVE_LOG_IN_SUCCESS			        20
#define	RECEIVE_LOG_IN_FAIL_PASSWORD	        21
#define	RECEIVE_LOG_IN_FAIL_ID			        22
#define RECEIVE_LOG_IN_FAIL_ID_CONNECTED        23
#define RECEIVE_LOG_IN_FAIL_SERVER_BUSY         24
#define RECEIVE_LOG_IN_FAIL_ID_BLOCK            25
#define RECEIVE_LOG_IN_FAIL_VERSION				26
#define RECEIVE_LOG_IN_FAIL_CONNECT				27
#define RECEIVE_LOG_IN_FAIL_ERROR				28
#define RECEIVE_LOG_IN_FAIL_USER_TIME1          29
#define RECEIVE_LOG_IN_FAIL_USER_TIME2          30
#define RECEIVE_LOG_IN_FAIL_PC_TIME1            31
#define RECEIVE_LOG_IN_FAIL_PC_TIME2            32
#define RECEIVE_LOG_IN_FAIL_DATE				33	// 사용 안함.
#define RECEIVE_LOG_IN_FAIL_POINT_DATE			34	// 대만 전용
#define RECEIVE_LOG_IN_FAIL_POINT_HOUR			35	// 대만 전용
#define RECEIVE_LOG_IN_FAIL_INVALID_IP			36	// 대만 전용
#define RECEIVE_LOG_IN_FAIL_NO_PAYMENT_INFO		37
#define RECEIVE_LOG_IN_FAIL_ONLY_OVER_15		38
#ifdef ASG_ADD_CHARGED_CHANNEL_TICKET
#define RECEIVE_LOG_IN_FAIL_CHARGED_CHANNEL		39	// 글로벌 전용.
#endif	// ASG_ADD_CHARGED_CHANNEL_TICKET

#define	REQUEST_CHARACTERS_LIST			        50
#define	RECEIVE_CHARACTERS_LIST			        51
#define	REQUEST_CREATE_CHARACTER		        52
#define	RECEIVE_CREATE_CHARACTER_SUCCESS        53
#define	RECEIVE_CREATE_CHARACTER_FAIL           54
#define	RECEIVE_CREATE_CHARACTER_FAIL2          55
#define	REQUEST_DELETE_CHARACTER	            56
#define	RECEIVE_DELETE_CHARACTER_SUCCESS        57
#define REQUEST_JOIN_MAP_SERVER			        60
#define RECEIVE_JOIN_MAP_SERVER			        61
#define	RECEIVE_CONFIRM_PASSWORD_SUCCESS        62	// 사용안함.
#define	RECEIVE_CONFIRM_PASSWORD_FAIL_ID        63	// 사용안함.
#define	RECEIVE_CONFIRM_PASSWORD2_SUCCESS       64	// 사용안함.
#define	RECEIVE_CONFIRM_PASSWORD2_FAIL_ID       65	// 사용안함.
#define	RECEIVE_CONFIRM_PASSWORD2_FAIL_ANSWER   66	// 사용안함.
#define	RECEIVE_CONFIRM_PASSWORD2_FAIL_RESIDENT 67	// 사용안함.
#define RECEIVE_CHANGE_PASSWORD_SUCCESS         68	// 사용안함.
#define RECEIVE_CHANGE_PASSWORD_FAIL_ID         69	// 사용안함.
#define RECEIVE_CHANGE_PASSWORD_FAIL_RESIDENT   70	// 사용안함.
#define RECEIVE_CHANGE_PASSWORD_FAIL_PASSWORD   71	// 사용안함.


#ifdef PJH_CHARACTER_RENAME
#define REQUEST_CHECK_CHANGE_NAME				72
#define RECEIVE_CHECK_CHANGE_NAME				73
#define REQUEST_CHANGE_NAME						74
#define RECEIVE_CHANGE_NAME_RESULT				75
#endif //PJH_CHARACTER_RENAME

#ifdef MODIFY_SOCKET_PROTOCOL
//2008. 5. 22 소켓 아이템 으로 인해 인덱스 확장후 이 숫자로 고정
#define PACKET_ITEM_LENGTH  12
#else	// MODIFY_SOCKET_PROTOCOL
//2005. 7. 11 아이템 인덱스 확장후 이 숫자로 고정
#define PACKET_ITEM_LENGTH  7
#endif	// MODIFY_SOCKET_PROTOCOL

#define EQUIPMENT_LENGTH    17
#define MAX_SPE_BUFFERSIZE_	( 2048)

#ifdef YDG_MOD_PROTECT_AUTO_V5
#define MAX_HACKTOOL_FILENAME_LENGTH 64
#endif	// YDG_MOD_PROTECT_AUTO_V5

extern int CurrentProtocolState;

typedef struct {
	BYTE Code;
	BYTE Size;
	BYTE HeadCode;
} PBMSG_HEADER, * LPPBMSG_HEADER;

typedef struct {
	BYTE Code;
	BYTE SizeH;
	BYTE SizeL;
	BYTE HeadCode;
} PWMSG_HEADER, * LPPWMSG_HEADER;

// 암호화 패킷
typedef struct {
	BYTE Code;
	BYTE Size;
	BYTE byBuffer[255];
} PBMSG_ENCRYPTED, * LPPBMSG_ENCRYPTED;

typedef struct {
	BYTE Code;
	BYTE SizeH;
	BYTE SizeL;
	BYTE byBuffer[MAX_SPE_BUFFERSIZE_];
} PWMSG_ENCRYPTED, * LPWBMSG_ENCRYPTED;

//request default SubCode
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
} PREQUEST_DEFAULT_SUBCODE, * LPPREQUEST_DEFAULT_SUBCODE;

//receive default
typedef struct {
	PBMSG_HEADER Header;
	BYTE         Value;
} PHEADER_DEFAULT, * LPPHEADER_DEFAULT;

// 마이너스 열매 관련
typedef struct {
	PBMSG_HEADER	Header;
	BYTE			result;
#ifdef PSW_FRUIT_ITEM
	WORD			btStatValue;	// 증 / 감 스탯값
#else //PSW_FRUIT_ITEM
	BYTE			btStatValue;	// 증 / 감 스탯값
#endif //PSW_FRUIT_ITEM	
	BYTE			btFruitType;	// 열매 종류
} PMSG_USE_STAT_FRUIT, * LPPMSG_USE_STAT_FRUIT;

//receive default subcode
typedef struct 
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         Value;
} PHEADER_DEFAULT_SUBCODE, * LPPHEADER_DEFAULT_SUBCODE;

//receive Character List 
typedef struct 
{
    BYTE         MaxClass;
	BYTE		 MoveCount;
	BYTE         Value;
} PHEADER_DEFAULT_CHARACTER_LIST, * LPPHEADER_DEFAULT_CHARACTER_LIST;

#ifdef PBG_ADD_CHARACTERCARD
//////////////////////////////////////////////////////////////////////////
// 0xDE, 0x00	해외 소환술사 카드와 같은 구조
#define CLASS_SUMMONER_CARD		0x01
#define CLASS_DARK_LORD_CARD	0x02
#define CLASS_DARK_CARD			0x04
#define CLASS_CHARACTERCARD_TOTALCNT	3

typedef struct {
	PWMSG_HEADER	header;
	BYTE			CharacterCard;	// 다크로드 마검사 캐릭터 카드
} PHEADER_CHARACTERCARD, *LPPHEADER_CHARACTERCARD;

typedef struct {
	bool bCharacterEnable[CLASS_CHARACTERCARD_TOTALCNT];		// 활성화 [0]마검 [1]다크
}CHARACTER_ENABLE, *LPCHARACTER_ENABLE;

extern CHARACTER_ENABLE g_CharCardEnable;
#endif //PBG_ADD_CHARACTERCARD

//receive default key
typedef struct {
	PBMSG_HEADER Header;
	BYTE         KeyH;
	BYTE         KeyL;
} PHEADER_DEFAULT_KEY, * LPPHEADER_DEFAULT_KEY;

//receive default key
typedef struct {
	PBMSG_HEADER Header;
	BYTE         Value;
	BYTE         KeyH;
	BYTE         KeyL;
} PHEADER_DEFAULT_VALUE_KEY, * LPPHEADER_DEFAULT_VALUE_KEY;

//(이벤트)오픈시간 요청.
typedef struct {
    PBMSG_HEADER Header;
    BYTE         Value;
    BYTE         KeyH;
    BYTE         KeyM;
    BYTE         KeyL;
} PHEADER_MATCH_OPEN_VALUE, * LPPHEADER_MATCH_OPEN_VALUE;

// 마법 걸린 상태 바꾸기 패킷
typedef struct
{
	PBMSG_HEADER Header;
	BYTE		State;				// 0 : 해제, 1 : 상태적용
	BYTE		KeyH;
	BYTE		KeyL;
	BYTE		BuffIndex;
} PMSG_VIEWSKILLSTATE, *LPPMSG_VIEWSKILLSTATE;

//receive default(word)
typedef struct {
	PWMSG_HEADER Header;
	BYTE         Value;
} PWHEADER_DEFAULT_WORD, * LPPWHEADER_DEFAULT_WORD;

//receive default(word)
typedef struct {
	PWMSG_HEADER Header;
	INT          Value;
} PWHEADER_DEFAULT_WORD2, * LPPWHEADER_DEFAULT_WORD2;



//receive default subcode(word)
typedef struct {
	PWMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         Value;
	
} PHEADER_DEFAULT_SUBCODE_WORD, * LPPHEADER_DEFAULT_SUBCODE_WORD;

typedef struct {
	PBMSG_HEADER Header;
	WORD        Value;
} PHEADER_DEFAULT_WORD, * LPPHEADER_DEFAULT_WORD;

typedef struct {
	PBMSG_HEADER Header;
	DWORD        Value;
} PHEADER_DEFAULT_DWORD, * LPPHEADER_DEFAULT_DWORD;

typedef struct {
	PBMSG_HEADER Header;
	BYTE        byBuffer[1024];
} PHEADER_DEFAULT_CUSTOM, * LPPHEADER_DEFAULT_CUSTOM;


typedef struct {
	PBMSG_HEADER  Header;
	BYTE          Index;
	BYTE          Item[PACKET_ITEM_LENGTH];
} PHEADER_DEFAULT_ITEM, * LPPHEADER_DEFAULT_ITEM;

typedef struct {
	PBMSG_HEADER  Header;
	BYTE          SubCode;
	BYTE          Index;
	BYTE          Item[PACKET_ITEM_LENGTH];
} PHEADER_DEFAULT_SUBCODE_ITEM, * LPPHEADER_DEFAULT_SUBCODE_ITEM;

//send trade
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          SrcFlag;
	BYTE          SrcIndex;
	BYTE          Item[PACKET_ITEM_LENGTH];
	BYTE          DstFlag;
	BYTE          DstIndex;
} PSEND_TRADE, * LPSEND_TRADE;

///////////////////////////////////////////////////////////////////////////////
// log in
///////////////////////////////////////////////////////////////////////////////

typedef struct {
	WORD         Index;	// 서버 개수 늘어나면서 바뀜
	BYTE         Percent;
#ifdef _PVP_DYNAMIC_SERVER_TYPE
	BYTE		 IsPvPServer;
#endif	// _PVP_DYNAMIC_SERVER_TYPE
} PRECEIVE_SERVER_LIST, * LPPRECEIVE_SERVER_LIST;

//receive join server
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         Result;
	BYTE         NumberH;
	BYTE         NumberL;
	BYTE         Version[SIZE_PROTOCOLVERSION];
} PRECEIVE_JOIN_SERVER, * LPPRECEIVE_JOIN_SERVER;

//request create acoount
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         ID[MAX_ID_SIZE];
	
#ifdef LDS_MODIFY_CHAR_LENGTH_USERPASSWORD // 비밀번호 자릿수 10->12로 변경 사항
	BYTE         Password[MAX_PASSWORD_SIZE];
#else // LDS_MODIFY_CHAR_LENGTH_USERPASSWORD 
	BYTE         Password[MAX_ID_SIZE];
#endif // LDS_MODIFY_CHAR_LENGTH_USERPASSWORD 

	BYTE         Name[10];
	BYTE         ResidentNumber[13];
	BYTE         Quiz[30];
	BYTE         Answer[30];
	BYTE         PhoneNumber[20];
	BYTE         Email[50];
} PREQUEST_CREATE_ACCOUNT, * LPPREQUEST_CREATE_ACCOUNT;

//request log in
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         ID[MAX_ID_SIZE];
	
#ifdef LDS_MODIFY_CHAR_LENGTH_USERPASSWORD  // 비밀번호 자릿수 10->12로 변경 사항
	BYTE         Password[MAX_PASSWORD_SIZE];
#else // LDS_MODIFY_CHAR_LENGTH_USERPASSWORD 
	BYTE         Password[MAX_ID_SIZE];
#endif // LDS_MODIFY_CHAR_LENGTH_USERPASSWORD 
	
	DWORD        Time;
	BYTE         Version[SIZE_PROTOCOLVERSION];
	BYTE         Serial[SIZE_PROTOCOLSERIAL];
} PREQUEST_LOG_IN, * LPPREQUEST_LOG_IN;

//request confirm password
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         ID[MAX_ID_SIZE];
} PREQUEST_CONFIRM_PASSWORD, * LPPREQUEST_CONFIRM_PASSWORD;

//receive confirm password
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         Result;
	BYTE         Question[30];
} PRECEIVE_CONFIRM_PASSWORD, * LPPRECEIVE_CONFIRM_PASSWORD;

//request confirm password
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         ID[MAX_ID_SIZE];
	BYTE         ResidentNumber[13];
	BYTE         Answer[30];
} PREQUEST_CONFIRM_PASSWORD2, * LPPREQUEST_CONFIRM_PASSWORD2;

//receive confirm password
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         Result;
	BYTE         Password[MAX_ID_SIZE];
} PRECEIVE_CONFIRM_PASSWORD2, * LPPRECEIVE_CONFIRM_PASSWORD2;

//request change password
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         ID[MAX_ID_SIZE];
	BYTE         OldPassword[MAX_ID_SIZE];
	BYTE         NewPassword[MAX_ID_SIZE];
	BYTE         ResidentNumber[13];
} PREQUEST_CHANGE_PASSWORD, * LPPREQUEST_CHANGE_PASSWORD;

//receive characters list
typedef struct {
	BYTE         Index;
	BYTE         ID[MAX_ID_SIZE];
	WORD         Level;
	BYTE		 CtlCode;
	BYTE         Class;
	BYTE         Equipment[EQUIPMENT_LENGTH];
    BYTE         byGuildStatus;
} PRECEIVE_CHARACTER_LIST, * LPPRECEIVE_CHARACTER_LIST;

//request create character
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         ID[MAX_ID_SIZE];
	BYTE         Class;
} PREQUEST_CREATE_CHARACTER, * LPREQUEST_CREATE_CHARACTER;

//receive create character
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         Result;
	BYTE         ID[MAX_ID_SIZE];
	BYTE         Index;
    WORD         Level;
	BYTE		 Class;
	//BYTE         Equipment[24];
} PRECEIVE_CREATE_CHARACTER, * LPPRECEIVE_CREATE_CHARACTER;

//request delete character
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         ID[MAX_ID_SIZE];
	BYTE         Resident[10];
} PREQUEST_DELETE_CHARACTER, * LPREQUEST_DELETE_CHARACTER;

//request join map server
typedef struct {
	PBMSG_HEADER Header;
	BYTE		 SubCode;
	BYTE         ID[MAX_ID_SIZE];
} PREQUEST_JOIN_MAP_SERVER, * LPPREQUEST_JOIN_MAP_SERVER;

//receive join map server
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         PositionX;
	BYTE         PositionY;
	BYTE         Map;
	BYTE         Angle;

	BYTE		btMExp1;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 1)
	BYTE		btMExp2;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 2)
	BYTE		btMExp3;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 3)
	BYTE		btMExp4;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 4)
	BYTE		btMExp5;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 5)
	BYTE		btMExp6;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 6)
	BYTE		btMExp7;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 7)
	BYTE		btMExp8;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 8)


	BYTE		btMNextExp1;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 1)
	BYTE		btMNextExp2;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 2)
	BYTE		btMNextExp3;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 3)
	BYTE		btMNextExp4;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 4)
	BYTE		btMNextExp5;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 5)
	BYTE		btMNextExp6;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 6)
	BYTE		btMNextExp7;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 7)
	BYTE		btMNextExp8;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 8)

	WORD         LevelUpPoint;
	//BYTE         LevelUpPoint;
	WORD         Strength;
	WORD         Dexterity;
	WORD         Vitality;
	WORD         Energy;
	WORD         Life;
	WORD         LifeMax;
	WORD         Mana;
	WORD         ManaMax;
	WORD		 Shield;
	WORD		 ShieldMax;
	WORD		 SkillMana;
	WORD		 SkillManaMax;
	DWORD        Gold;
	BYTE         PK;
	BYTE		 CtlCode;
    short        AddPoint;
    short        MaxAddPoint;
    WORD         Charisma;  //  통솔.
	WORD		 wMinusPoint;		// 현재 감소 포인트
	WORD		 wMaxMinusPoint;	// 최대 감소 포인트
} PRECEIVE_JOIN_MAP_SERVER, * LPPRECEIVE_JOIN_MAP_SERVER;

//receive revival
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         PositionX;
	BYTE         PositionY;
	BYTE         Map;
	BYTE         Angle;
	WORD         Life;
	WORD         Mana;
	WORD		 Shield;
	WORD		 SkillMana;
	BYTE		btMExp1;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 1)
	BYTE		btMExp2;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 2)
	BYTE		btMExp3;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 3)
	BYTE		btMExp4;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 4)
	BYTE		btMExp5;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 5)
	BYTE		btMExp6;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 6)
	BYTE		btMExp7;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 7)
	BYTE		btMExp8;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 8)

	DWORD        Gold;
} PRECEIVE_REVIVAL, * LPPRECEIVE_REVIVAL;

//inventory
typedef struct {
	BYTE          Index;
	BYTE          Item[PACKET_ITEM_LENGTH];
} PRECEIVE_INVENTORY, * LPPRECEIVE_INVENTORY;


///////////////////////////////////////////////////////////////////////////////
// trade
///////////////////////////////////////////////////////////////////////////////
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         ID[MAX_ID_SIZE];
    WORD         Level;
    DWORD        GuildKey;
} PTRADE, * LPPTRADE;


///////////////////////////////////////////////////////////////////////////////
// game
///////////////////////////////////////////////////////////////////////////////

//request chat
typedef struct {
	PBMSG_HEADER Header;
	BYTE         ID[MAX_ID_SIZE];
	BYTE         ChatText[MAX_CHAT_SIZE];
} PCHATING, * LPPCHATING;

typedef struct {
	PBMSG_HEADER Header;
	BYTE         KeyH;
	BYTE         KeyL;
	BYTE         ChatText[MAX_CHAT_SIZE];
} PCHATING_KEY, * LPPCHATING_KEY;

typedef struct {
	PBMSG_HEADER Header;
	BYTE         Result;
	BYTE		 Count;
	WORD		 Delay;
	DWORD		 Color;
	BYTE		 Speed;
	BYTE         Notice[256];
} PRECEIVE_NOTICE, * LPPRECEIVE_NOTICE;

//receive equipment
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         KeyH;
	BYTE         KeyL;
	BYTE         Class;
	BYTE         Equipment[EQUIPMENT_LENGTH];
} PRECEIVE_EQUIPMENT, * LPPRECEIVE_EQUIPMENT;

//receive other map character
typedef struct {
	BYTE         KeyH;
	BYTE         KeyL;
	BYTE         PositionX;
	BYTE         PositionY;
	BYTE         Class;
	BYTE         Equipment[EQUIPMENT_LENGTH];
	BYTE         ID[MAX_ID_SIZE];
	BYTE         TargetX;
	BYTE         TargetY;
	BYTE         Path;
	BYTE         s_BuffCount;
	BYTE		 s_BuffEffectState[MAX_BUFF_SLOT_INDEX];
} PCREATE_CHARACTER, * LPPCREATE_CHARACTER;

//receive other map character
typedef struct 
{
	BYTE         KeyH;
	BYTE         KeyL;
	BYTE         PositionX;
	BYTE         PositionY;
	BYTE         TypeH;
	BYTE         TypeL;
	BYTE         ID[MAX_ID_SIZE];
	BYTE         TargetX;
	BYTE         TargetY;
	BYTE         Path;
	BYTE         Class;
	BYTE         Equipment[EQUIPMENT_LENGTH];
	BYTE         s_BuffCount;
	BYTE		 s_BuffEffectState[MAX_BUFF_SLOT_INDEX];
} PCREATE_TRANSFORM, * LPPCREATE_TRANSFORM;

//receive other map character
typedef struct {
	BYTE         KeyH;
	BYTE         KeyL;
	BYTE         TypeH;
	BYTE         TypeL;
	BYTE         PositionX;
	BYTE         PositionY;
	BYTE         TargetX;
	BYTE         TargetY;
	BYTE         Path;
	BYTE         ID[MAX_ID_SIZE];
	BYTE         s_BuffCount;
	BYTE		 s_BuffEffectState[MAX_BUFF_SLOT_INDEX];
} PCREATE_SUMMON, * LPPCREATE_SUMMON;

//receive other map character
typedef struct {
	BYTE         KeyH;
	BYTE         KeyL;
	BYTE         TypeH;
	BYTE         TypeL;
	BYTE         PositionX;
	BYTE         PositionY;
	BYTE         TargetX;
	BYTE         TargetY;
	BYTE         Path;
	BYTE         s_BuffCount;
	BYTE		 s_BuffEffectState[MAX_BUFF_SLOT_INDEX];
} PCREATE_MONSTER, * LPPCREATE_MONSTER;

//send move my character
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          PositionX;
	BYTE          PositionY;
	BYTE          Path[8];
} PMOVE_MY_CHARACTER, * LPPMOVE_MY_CHARACTER;

//receive move character
typedef struct {
	BYTE          KeyH;
	BYTE          KeyL;
	BYTE          PositionX;
	BYTE          PositionY;
	BYTE          Path[1];
} PMOVE_CHARACTER, * LPPMOVE_CHARACTER;

//delete character and item
typedef struct {
	BYTE         KeyH;
	BYTE         KeyL;
} PDELETE_CHARACTER, * LPPDELETE_CHARACTER;

//create item
typedef struct {
	BYTE          KeyH;
	BYTE          KeyL;
	BYTE          PositionX;
	BYTE          PositionY;
	BYTE          Item[PACKET_ITEM_LENGTH];
} PCREATE_ITEM, * LPPCREATE_ITEM;

//request drop item
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          PositionX;
	BYTE          PositionY;
	BYTE          InventoryIndex;
} PREQUEST_DROP_ITEM, * LPPREQUEST_DROP_ITEM;

//request drop item
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          SrcIndex;
	BYTE          DstIndex;
} PREQUEST_EQUIPMENT_ITEM, * LPPREQUEST_EQUIPMENT_ITEM;

//change character
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          KeyH;
	BYTE          KeyL;
	BYTE          Item[PACKET_ITEM_LENGTH];
} PCHANGE_CHARACTER, * LPPCHANGE_CHARACTER;

//request attack
#ifdef KJH_MOD_ATTACK_PROTOCOL_FOR_PROTECT_HACK
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          Action;
	BYTE          Dir;
	BYTE          KeyH;
	BYTE          KeyL;
} PREQUEST_ATTACK, * LPPREQUEST_ATTACK;
#else // KJH_MOD_ATTACK_PROTOCOL_FOR_PROTECT_HACK
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          KeyH;
	BYTE          KeyL;
	BYTE          Action;
	BYTE          Dir;
} PREQUEST_ATTACK, * LPPREQUEST_ATTACK;
#endif // KJH_MOD_ATTACK_PROTOCOL_FOR_PROTECT_HACK

//receive get item
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          Result;
	BYTE          Item[PACKET_ITEM_LENGTH];
} PRECEIVE_GET_ITEM, * LPPRECEIVE_GET_ITEM;

//receive attack
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          KeyH;
	BYTE          KeyL;
	BYTE          DamageH;
	BYTE          DamageL;
	BYTE		  DamageType;
	BYTE		  ShieldDamageH;
	BYTE		  ShieldDamageL;			
} PRECEIVE_ATTACK, * LPPRECEIVE_ATTACK;

//receive die
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          KeyH;
	BYTE          KeyL;
	BYTE          ExpH;
	BYTE          ExpL;
	BYTE          DamageH;
	BYTE          DamageL;
} PRECEIVE_DIE, * LPPRECEIVE_DIE;
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          KeyH;
	BYTE          KeyL;
	WORD          ExpH;
	WORD          ExpL;
	BYTE          DamageH;
	BYTE          DamageL;
} PRECEIVE_DIE2, * LPPRECEIVE_DIE2;

//receive default key
typedef struct {
	PBMSG_HEADER Header;
	BYTE         KeyH;
	BYTE         KeyL;
	BYTE          MagicH;
	BYTE          MagicL;
	BYTE		 TKeyH;
	BYTE		 TKeyL;
} PHEADER_DEFAULT_DIE, * LPPHEADER_DEFAULT_DIE;

//request action
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          Angle;
	BYTE          Action;
	BYTE		  TargetKeyH;
	BYTE		  TargetKeyL;
} PREQUEST_ACTION, * LPPREQUEST_ACTION;

//receive action
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          KeyH;
	BYTE          KeyL;
	BYTE          Angle;
	BYTE          Action;
	BYTE		  TargetKeyH;
	BYTE		  TargetKeyL;
} PRECEIVE_ACTION, * LPPRECEIVE_ACTION;

//send magic
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          MagicH;
	BYTE          MagicL;
	BYTE          KeyH;
	BYTE          KeyL;
} PSEND_MAGIC, * LPPSEND_MAGIC;

//send magic continue
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          MagicH;
	BYTE          MagicL;
	BYTE          PositionX;
	BYTE          PositionY;
	BYTE          Angle;
} PSEND_MAGIC_CONTINUE, * LPPSEND_MAGIC_CONTINUE;

//receive magic continue
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          MagicH;
	BYTE          MagicL;
	BYTE          KeyH;
	BYTE          KeyL;
	BYTE          PositionX;
	BYTE          PositionY;
	BYTE          Angle;
} PRECEIVE_MAGIC_CONTINUE, * LPPRECEIVE_MAGIC_CONTINUE;

//receive magic
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          MagicH;
	BYTE          MagicL;
	BYTE          SourceKeyH;
	BYTE          SourceKeyL;
	BYTE          TargetKeyH;
	BYTE          TargetKeyL;
} PRECEIVE_MAGIC, * LPPRECEIVE_MAGIC;

//receive MonsterSkill
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          MagicH;
	BYTE          MagicL;
	WORD		  SourceKey;
	WORD		  TargetKey;
} PRECEIVE_MONSTERSKILL, * LPPRECEIVE_MONSTERSKILL;

//receive magic target
typedef struct {
	BYTE          KeyH;
	BYTE          KeyL;
} PRECEIVE_MAGIC_POSITION, * LPPRECEIVE_MAGIC_POSITION;

//send magic continue
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          MagicH;
	BYTE          MagicL;
	BYTE          PositionX;
	BYTE          PositionY;
	BYTE          Count;
	BYTE          Key[10];
} PSEND_MAGIC_CONTINUES, * LPPSEND_MAGIC_CONTINUES;

//receive magic target
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          KeyH;
	BYTE          KeyL;

	BYTE          MagicH;
	BYTE          MagicL;

	BYTE          PositionX;
	BYTE          PositionY;
	BYTE          Count;
} PRECEIVE_MAGIC_POSITIONS, * LPPRECEIVE_MAGIC_POSITIONS;

//receive magic list count
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         Value;
    BYTE         ListType;  //  스킬 리스트 종류. ( 0:일반 전체 스킬 리스트, 1:추가적인 스킬 리스트, 2:삭제 스킬 리스트 ).
} PHEADER_MAGIC_LIST_COUNT, * LPPHEADER_MAGIC_LIST_COUNT;

#pragma pack(push, 1)
//receive magic target
typedef struct {
	BYTE          Index;

	WORD		  Type;
	BYTE          Level;
} PRECEIVE_MAGIC_LIST, * LPPRECEIVE_MAGIC_LIST;
#pragma pack(pop)

//receive skill count.
typedef	struct {
	PBMSG_HEADER  Header;
	BYTE          KeyH;
	BYTE          KeyL;
	BYTE		  m_byType;		//	스킬 번호.
	BYTE		  m_byCount;	//	단계 카운트.
} PRECEIVE_EX_SKILL_COUNT, * LPPRECEIVE_EX_SKILL_COUNT;

//receive gold
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          Flag;
	DWORD         Gold;
} PRECEIVE_GOLD, * LPPRECEIVE_GOLD;

//receive repair gold
typedef struct {
	PBMSG_HEADER  Header;
	DWORD         Gold;
} PRECEIVE_REPAIR_GOLD, * LPPRECEIVE_REPAIR_GOLD;

//receive level up
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	WORD         Level;
	WORD         LevelUpPoint;
	//BYTE         Level;
	//BYTE         LevelUpPoint;
	WORD         MaxLife;
	WORD         MaxMana;
	WORD		 MaxShield;
	WORD		 SkillManaMax;
    short        AddPoint;
    short        MaxAddPoint;
	WORD		 wMinusPoint;		// 현재 감소 포인트
	WORD		 wMaxMinusPoint;	// 최대 감소 포인트
} PRECEIVE_LEVEL_UP, * LPPRECEIVE_LEVEL_UP;

typedef struct {
	PBMSG_HEADER  Header;
	BYTE          Index;
	BYTE		  Life[5];
} PRECEIVE_LIFE, * LPPRECEIVE_LIFE;

//receive add point
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         Result;
	WORD         Max;
	WORD		 ShieldMax;
	WORD		 SkillManaMax;
} PRECEIVE_ADD_POINT, * LPPRECEIVE_ADD_POINT;

typedef struct {
	PBMSG_HEADER Header;
	BYTE         KeyH;
	BYTE         KeyL;
	BYTE         PositionX;
	BYTE         PositionY;
} PRECEIVE_MOVE_POSITION, * LPPRECEIVE_MOVE_POSITION;

typedef struct {
	PBMSG_HEADER Header;
	WORD		 Flag;
	BYTE         Map;
	BYTE         PositionX;
	BYTE         PositionY;
	BYTE         Angle;
} PRECEIVE_TELEPORT_POSITION, * LPPRECEIVE_TELEPORT_POSITION;

//receive damage
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         DamageH;
	BYTE         DamageL;
	BYTE		 ShieldDamageH;
	BYTE		 ShieldDamageL;
} PRECEIVE_DAMAGE, * LPPRECEIVE_DAMAGE;

//receive create guild master
typedef struct {
	PBMSG_HEADER Header;
	BYTE         Name[8];
	BYTE         Mark[32];
} PSEND_CREATE_GUILD, * LPPSEND_CREATE_GUILD;

//receive party info
typedef struct {
    BYTE         value;
} PRECEIVE_PARTY_INFO, * LPPRECEIVE_PARTY_INFO;

//receive party infos
typedef struct {
	PBMSG_HEADER Header;
    BYTE         Count;
} PRECEIVE_PARTY_INFOS, * LPPRECEIVE_PARTY_INFOS;

//receive party list
typedef struct {
	BYTE         ID[MAX_ID_SIZE];
	BYTE         Number;
	BYTE         Map;
	BYTE         x;
	BYTE         y;
    int          currHP;
    int          maxHP;
#ifdef LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER	
	char		 PKPartyLevel;
#endif //LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER	
} PRECEIVE_PARTY_LIST, * LPPRECEIVE_PARTY_LIST;

//receive party list
typedef struct {
	PBMSG_HEADER Header;
	BYTE         Result;
	BYTE         Count;
} PRECEIVE_PARTY_LISTS, * LPPRECEIVE_PARTY_LISTS;

typedef struct {
    PBMSG_HEADER Header;
    BYTE         KeyH;
    BYTE         KeyL;
	int			 ItemInfo;
	BYTE		 ItemLevel;
}PRECEIVE_GETITEMINFO_FOR_PARTY, * LPPRECEIVE_GETITEMINFO_FOR_PARTY;

//receive pk
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         KeyH;
	BYTE         KeyL;
	BYTE         PK;
} PRECEIVE_PK, * LPPRECEIVE_PK;

typedef struct {
	PBMSG_HEADER Header;
	BYTE         Index;
	WORD         Time;
} PRECEIVE_HELPER_ITEM, * LPPRECEIVE_HELPER_ITEM;

typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         IP[15];
	WORD         Port;
} PRECEIVE_SERVER_ADDRESS, * LPPRECEIVE_SERVER_ADDRESS;

typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		 Server;
} PRECEIVE_SERVER_BUSY, * LPPRECEIVE_SERVER_BUSY;

typedef struct {
	BYTE         KeyH;
	BYTE         KeyL;
	BYTE         Name[8];
	BYTE         Mark[32];
} PRECEIVE_GUILD, * LPPRECEIVE_GUILD;

typedef struct {
	PBMSG_HEADER Header;
	BYTE         KeyH;
	BYTE         KeyL;
	BYTE         Name[8];
	BYTE         Mark[32];
} PRECEIVE_GUILD_MARK, * LPPRECEIVE_GUILD_MARK;

typedef struct {
	BYTE         KeyH;
	BYTE         KeyL;
	BYTE         GuildKeyH;
	BYTE         GuildKeyL;
} PRECEIVE_GUILD_PLAYER, * LPPRECEIVE_GUILD_PLAYER;

// 길드원 목록
typedef struct {
	BYTE         ID[MAX_ID_SIZE];
	BYTE         Number;
	BYTE		 CurrentServer;
	BYTE		 GuildStatus;		// 길드직위
} PRECEIVE_GUILD_LIST, * LPPRECEIVE_GUILD_LIST;

// 길드원 목록 리스트
typedef struct {
	PWMSG_HEADER Header;
	BYTE         Result;
	BYTE         Count;
	DWORD        TotalScore;
	BYTE         Score;
	char		szRivalGuildName[MAX_GUILDNAME];
} PRECEIVE_GUILD_LISTS, * LPPRECEIVE_GUILD_LISTS;

//send guild leave
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          ID[MAX_ID_SIZE];
	BYTE          ResidentNumber[10];
} PSEND_GUILD_LEAVE, * LPPSEND_GUILD_LEAVE;

typedef struct {
	PBMSG_HEADER Header;
	BYTE         Name[8];
} PSEND_GUILD_WAR, * LPPSEND_GUILD_WAR;

//receive guild war
typedef struct {
	PBMSG_HEADER Header;
	BYTE         Name[8];
	BYTE         Type;
	BYTE         Team;
} PRECEIVE_WAR, * LPPRECEIVE_WAR;

typedef struct {
	PBMSG_HEADER Header;
	BYTE         Score1;
	BYTE         Score2;
	BYTE         Type;
} PRECEIVE_WAR_SCORE, * LPPRECEIVE_WAR_SCORE;

// 뷰포트의 길드 ID 목록
typedef struct
{
	int			 GuildKey;
	BYTE		 GuildStatus;		// 길드직위 -1 : NONE, 0 : PERSON, 32 : BATTLE_MASTER, 64 : SUB_MASTER, 128 : MASTER
	BYTE		 GuildType;			// 길드종류 일반(0), 수호길드(1)
	BYTE		 GuildRelationShip;	// 길드관계 (0x00 : 관계없음, 0x01 : Union, 0x04 Union Master, 0x02 : Rival, 0x08: Rival Union )
	BYTE		 KeyH;
	BYTE		 KeyL;
#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
	BYTE btCastleOwner;		// 수성측길드나 연합이면 1, 아니면 0
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
} PRECEIVE_GUILD_ID, *LPPRECEIVE_GUILD_ID;

// 길드 정보 요청 결과
typedef struct
{
	PBMSG_HEADER Header;
	int			GuildKey;
	BYTE		GuildType;			// 길드종류 일반(0), 용병(1), 클랜(2)
	BYTE		UnionName[MAX_GUILDNAME];
	BYTE		GuildName[MAX_GUILDNAME];
	BYTE		Mark[32];
} PPRECEIVE_GUILDINFO, *LPPPRECEIVE_GUILDINFO;

enum GUILD_REQ_COMMON_RESULT
{
	// 길드가 없음
	GUILD_ANS_NOTEXIST_GUILD			= 0x10,
	// 연합관계 실패 (공성전관계때문에)
	GUILD_ANS_UNIONFAIL_BY_CASTLE		= 0x10,
	// 권한 없음
	GUILD_ANS_NOTEXIST_PERMISSION		= 0x11,

	// 임명 할 수 있는 계급이 없음
	GUILD_ANS_NOTEXIST_EXTRA_STATUS		= 0x12,
	// 임명 할 수 있는 타입이 없음
	GUILD_ANS_NOTEXIST_EXTRA_TYPE		= 0x13,

	// 관계 유 & 무
	GUILD_ANS_EXIST_RELATIONSHIP_UNION	= 0x15,
	GUILD_ANS_EXIST_RELATIONSHIP_RIVAL	= 0x16,

	// 연합 유 & 무
	GUILD_ANS_EXIST_UNION				= 0x17,
	GUILD_ANS_EXIST_RIVAL				= 0x18,
	GUILD_ANS_NOTEXIST_UNION			= 0x19,
	GUILD_ANS_NOTEXIST_RIVAL			= 0x1A,

	// 연합 마스터가 아님
	GUILD_ANS_NOT_UNION_MASTER			= 0x1B,

	// 적대 길드가 아님
	GUILD_ANS_NOT_GUILD_RIVAL			= 0x1C,

	// 연합길드생성조건미달 (길드원20이상)
	GUILD_ANS_CANNOT_BE_UNION_MASTER_GUILD = 0x1D,
	
	// 최대 길드 연합 개수 초과함
	GUILD_ANS_EXCEED_MAX_UNION_MEMBER	= 0x1E,

	// 신청이 취소되었습니다.
	GUILD_ANS_CANCEL_REQUEST			= 0x20,

#ifdef ASG_ADD_GENS_SYSTEM
	// 연합 길마가 겐스에 가입이 안되어 있다.
	GUILD_ANS_UNION_MASTER_NOT_GENS		= 0xA1,
	// 길마가 겐스에 가입이 안되어 있다.
	GUILD_ANS_GUILD_MASTER_NOT_GENS		= 0xA2,
	// 연합 길마와 겐스가 서로 다르다.
	GUILD_ANS_UNION_MASTER_DISAGREE_GENS = 0xA3,
#endif	// ASG_ADD_GENS_SYSTEM
};

// 길드생성 결과
typedef struct {
	PBMSG_HEADER	Header;
	BYTE			Value;
	BYTE			GuildType;
} PMSG_GUILD_CREATE_RESULT, *LPPMSG_GUILD_CREATE_RESULT;

// 길드종류를 변경 결과
typedef struct
{
	PBMSG_HEADER	Header;
	BYTE			byGuildType;				// 일반(0), 용병(1), 클랜(2)
	BYTE			byResult;					// 실패(0x00), 성공(0x01), 길드없음(0x10), 권한없음(0x11), 길드종류없음(0x13)
} PMSG_GUILD_ASSIGN_TYPE_RESULT, *LPPMSG_GUILD_ASSIGN_TYPE_RESULT;

// 직책을 임명 / 변경 / 해제 결과
typedef struct {
	PBMSG_HEADER	Header;
	BYTE			byType;						// 임명(1), 변경(2), 해제(3)
	BYTE			byResult;					// 결과
	char			szTargetName[MAX_ID_SIZE];	// 대상유저이름
} PRECEIVE_GUILD_ASSIGN, *LPPRECEIVE_GUILD_ASSIGN;

// 연합길드 리스트 개수
typedef struct {
	PWMSG_HEADER	Header;
	BYTE			byCount;
	BYTE			byResult;
	BYTE			byRivalCount;
	BYTE			byUnionCount;
} PMSG_UNIONLIST_COUNT, *LPPMSG_UNIONLIST_COUNT;
// 연합길드 정보
typedef struct {
	BYTE			byMemberCount;
	BYTE			GuildMark[32];
	char			szGuildName[MAX_GUILDNAME];
} PMSG_UNIONLIST, *LPPMSG_UNIONLIST;

// 연합이나 적대길드 요청  
typedef struct {
	PBMSG_HEADER	Header;
	BYTE			byRelationShipType;			// 관계종류 ( 0x01 : Union, 0x02 : Rival )
	BYTE			byRequestType;				// 요청종류 ( 0x01 : Join, 0x02 : BreakOff )
	BYTE			byTargetUserIndexH;			// 목표유저인덱스H
	BYTE			byTargetUserIndexL;			// 목표유저인덱스L
} PMSG_GUILD_RELATIONSHIP, *LPPMSG_GUILD_RELATIONSHIP;

// 연합이나 적대길드 요청결과  
typedef struct {
	PBMSG_HEADER	Header;
	BYTE			byRelationShipType;			// 관계종류 ( 0x01 : Union, 0x02 : Rival )
	BYTE			byRequestType;				// 요청종류 ( 0x01 : Join, 0x02 : BreakOff )
	BYTE			byResult;					// 결과 ( 0x00 : 거부, 0x01 : 수락, 0x10 : 길드없음, 0x11 : 권한없음 )
	BYTE			byTargetUserIndexH;			// 목표유저인덱스H
	BYTE			byTargetUserIndexL;			// 목표유저인덱스L
} PMSG_GUILD_RELATIONSHIP_RESULT, *LPPMSG_GUILD_RELATIONSHIP_RESULT;

typedef struct {
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE			byResult;
	BYTE			byRequestType;
	BYTE			byRelationShipType;
} PMSG_BAN_UNIONGUILD, *LPPMSG_BAN_UNIONGUILD;

// 뷰포트내의 연합정보 변화를  
typedef struct {
	PWMSG_HEADER	Header;
	BYTE			byCount;
} PMSG_UNION_VIEWPORT_NOTIFY_COUNT, *LPPMSG_UNION_VIEWPORT_NOTIFY_COUNT;
typedef struct {
	BYTE			byKeyH;
	BYTE			byKeyL;
	int				nGuildKey;
	BYTE			byGuildRelationShip;
	char			szUnionName[MAX_GUILDNAME];
} PMSG_UNION_VIEWPORT_NOTIFY, *LPPMSG_UNION_VIEWPORT_NOTIFY;

//receive gold
typedef struct {
	PBMSG_HEADER Header;
	BYTE         Result;
	DWORD        StorageGold;
	DWORD        Gold;
} PRECEIVE_STORAGE_GOLD, * LPPRECEIVE_STORAGE_GOLD;

//receive soccer time
typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	WORD         Time;
} PRECEIVE_SOCCER_TIME, * LPPRECEIVE_SOCCER_TIME;

typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         Name1[8];
	BYTE         Score1;
	BYTE         Name2[8];
	BYTE         Score2;
} PRECEIVE_SOCCER_SCORE, * LPPRECEIVE_SOCCER_SCORE;

#ifdef CSK_FIX_QWER
#pragma pack(push, 1)
#endif // CSK_FIX_QWER
typedef struct {
    PBMSG_HEADER Header;
    BYTE         SubCode;
#ifdef CSK_FIX_SKILLHOTKEY_PACKET
	BYTE         HotKey[20];
#else // CSK_FIX_SKILLHOTKEY_PACKET
	BYTE         HotKey[10];
#endif // CSK_FIX_SKILLHOTKEY_PACKET
    BYTE         GameOption;
    BYTE         KeyQWE[3];
	BYTE		 ChatLogBox;
	BYTE		 KeyR;
	int			 QWERLevel;		// 4바이트
} PRECEIVE_OPTION, * LPPRECEIVE_OPTION;
#ifdef CSK_FIX_QWER
#pragma pack(pop)
#endif // CSK_FIX_QWER

typedef struct {
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		 Cmd1;
	BYTE		 Cmd2;
	BYTE		 Cmd3;
} PRECEIVE_SERVER_COMMAND, * LPPRECEIVE_SERVER_COMMAND;


typedef struct {
	PBMSG_HEADER Header;
	BYTE		 m_MyRank;
	BYTE         m_Count;
	BYTE		 m_byRank;
} PDEVILRANK, * LPPDEVILRANK;

typedef struct {    //  이벤트칩 정보들. .
    PBMSG_HEADER Header;
    BYTE        m_byType;       //  ( 0:레나, 1:스톤, 2:상용화 2주년 복권 ... )
#ifdef ASG_FIX_LENA_REGISTRATION
	int			m_nChipCount;  //  이벤트칩 수.
#else	// ASG_FIX_LENA_REGISTRATION
	short       m_shChipCount;  //  이벤트칩 수.
#endif	// ASG_FIX_LENA_REGISTRATION
    short       m_shMutoNum[3]; //  뮤토 번호.
} PRECEIVE_EVENT_CHIP_INFO, * LPPRECEIVE_EVENT_CHIP_INFO;

typedef struct {    //  이벤트칩수.
    PBMSG_HEADER Header;
    BYTE        m_byType;       //  ( 0:레나, 1:스톤, 2:상용화 2주년 복권 ... )
#ifdef ASG_FIX_LENA_REGISTRATION
	unsigned int  m_unChipCount;
#else	// ASG_FIX_LENA_REGISTRATION
    unsigned short  m_shChipCount;
#endif	// ASG_FIX_LENA_REGISTRATION
} PRECEIVE_ENVET_CHIP, * LPPRECEIVE_EVENT_CHIP;

typedef struct {    //  뮤토번호.
    PBMSG_HEADER Header;
    short        m_shMutoNum[3];
} PRECEIVE_MUTONUMBER, * LPPRECEIVE_MUTONUMBER;

#ifdef SCRATCH_TICKET
typedef struct {	//	복권.
	PBMSG_HEADER Header;
	BYTE		 m_byIsRegistered;	//	복권이 등록되 있는지 검사. ( 0:미등록, 1:등록, 2:등록횟순 넘김 ).
	char		 m_strGiftName[64];	//	경품이름.
} PRECEIVE_SCRATCH_TICKET_EVENT, *LPPRECEIVE_SCRATCH_TICKET_EVENT;
#endif

typedef struct {	//	효과음 재생
	PBMSG_HEADER Header;
	WORD		 wEffectNum;	//	효과음 번호
} PRECEIVE_PLAY_SOUND_EFFECT, *LPPRECEIVE_PLAY_SOUND_EFFECT;

typedef struct {
	PBMSG_HEADER Header;
	BYTE         m_byValue;     //  이벤트 ( 0:비활성, 1:활성 ).
    BYTE         m_byNumber;    //  이벤트 번호.
} PHEADER_EVENT, * LPPHEADER_EVENT;

typedef struct {
    PBMSG_HEADER    Header;
    BYTE            m_wEventType;       //  이벤트 종류.
    BYTE            m_wLeftEnterCount;  //  남은 횟수.
}PRECEIVE_EVENT_COUNT, *LPPRECEIVE_EVENT_COUNT;

typedef struct {
    PBMSG_HEADER Header;
    BYTE         m_byCount;
    BYTE         m_byQuest[50];
} PRECEIVE_QUEST_HISTORY, * LPPRECEIVE_QUEST_HISTORY;

typedef struct {
    PBMSG_HEADER Header;
    BYTE         m_byQuestIndex;
    BYTE         m_byState;
} PRECEIVE_QUEST_STATE, * LPPRECEIVE_QUEST_STATE;

typedef struct {
    PBMSG_HEADER Header;
    BYTE         m_byQuestIndex;
    BYTE         m_byResult;
    BYTE         m_byState;
} PRECEIVE_QUEST_RESULT, * LPPRECEIVE_QUEST_RESULT;

typedef struct {
    PBMSG_HEADER Header;
    BYTE         m_byKeyH;
    BYTE         m_byKeyL;
    BYTE         m_byReparation;
    BYTE         m_byNumber;
} PRECEIVE_QUEST_REPARATION, * LPPRECEIVE_QUEST_REPARATION;

#ifdef ASG_ADD_NEW_QUEST_SYSTEM

// GC[0xF6][0x0A]
typedef struct 
{	
	PBMSG_HEADER	Header;
	BYTE			SubCode;

	WORD			m_wNPCIndex;
	WORD			m_wQuestCount;
} PMSG_NPCTALK_QUESTLIST, *LPPMSG_NPCTALK_QUESTLIST;

//0xF6, 0x0B 대사 계속 진행
//0xF6, 0x0C 퀘스트 조건 보상(퀘스트 진행창에서 받을 때)
//0xF6, 0x1B 퀘스트 조건 보상(진행 중인 퀘스트 리스트에서 받을 때)
#pragma pack(push, 1)
typedef struct 
{	
	PBMSG_HEADER	Header;
	BYTE			SubCode;

	DWORD			m_dwQuestIndex;
	BYTE			m_byRequestCount;
	BYTE			m_byRewardCount;
#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	BYTE			m_byRandRewardCount;	// 랜덤 보상 실재 개수(A개 중에 B개 의 B를 말함)
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
} PMSG_NPC_QUESTEXP_INFO, *LPPMSG_NPC_QUESTEXP_INFO;
#pragma pack(pop)

// 요구 사항
enum QUEST_REQUEST_TYPE
{
	QUEST_REQUEST_NONE		= 0x0000,
	QUEST_REQUEST_MONSTER	= 0x0001,	// 몬스터 잡아와라.
	QUEST_REQUEST_SKILL		= 0x0002,	// 스킬 배워와라
	QUEST_REQUEST_ITEM		= 0x0004,	// 아이템 구해와라
	QUEST_REQUEST_LEVEL		= 0x0008,	// 레벨 올려와라
	QUEST_REQUEST_TUTORIAL	= 0x0010,	// 튜토리얼
	QUEST_REQUEST_BUFF		= 0x0020,	// 버프를 받아라.
#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	QUEST_REQUEST_EVENT_MAP_USER_KILL		= 0x0040,	// 유저 킬 포인트
	QUEST_REQUEST_EVENT_MAP_MON_KILL		= 0x0041,	// 몬스터 킬 포인트
	QUEST_REQUEST_EVENT_MAP_BLOOD_GATE		= 0x0042,	// 블러드 캐슬 성문 파괴
	QUEST_REQUEST_EVENT_MAP_CLEAR_BLOOD		= 0x0100,	// 블러드 캐슬 클리어
	QUEST_REQUEST_EVENT_MAP_CLEAR_CHAOS		= 0x0101,	// 카오스 캐슬 클리어
	QUEST_REQUEST_EVENT_MAP_CLEAR_DEVIL		= 0x0102,	// 악마의 광장 클리어
	QUEST_REQUEST_EVENT_MAP_CLEAR_ILLUSION	= 0x0103,	// 환영 사원 클리어
	QUEST_REQUEST_EVENT_MAP_DEVIL_POINT		= 0x0104,	// 악마의 광장 포인트 획득
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
#ifdef ASG_ADD_TIME_LIMIT_QUEST
	QUEST_REQUEST_ZEN		= 0x0105,	// 젠
	QUEST_REQUEST_PVP_POINT	= 0x0106,	// PVP 포인트
#endif	// ASG_ADD_TIME_LIMIT_QUEST
#ifdef ASG_ADD_QUEST_REQUEST_NPC_SEARCH
	QUEST_REQUEST_NPC_TALK	= 0x0107,	// NPC를 찾아라.
#endif	// ASG_ADD_QUEST_REQUEST_NPC_SEARCH

	QUEST_REQUEST_TYPE_MAX
};

#pragma pack(push, 1)
typedef struct 
{	
	DWORD	m_dwType;
	WORD	m_wIndex;
	DWORD	m_dwValue;
#ifdef ASG_ADD_TIME_LIMIT_QUEST
	DWORD	m_dwCurValue;
#else	// ASG_ADD_TIME_LIMIT_QUEST
	WORD	m_wCurValue;
#endif	// ASG_ADD_TIME_LIMIT_QUEST
	BYTE	m_byItemInfo[PACKET_ITEM_LENGTH];
} NPC_QUESTEXP_REQUEST_INFO, *LPNPC_QUESTEXP_REQUEST_INFO;
#pragma pack(pop)

// 보상
enum QUEST_REWARD_TYPE
{
	QUEST_REWARD_NONE		= 0x0000,
	QUEST_REWARD_EXP		= 0x0001,	// 경험치 준다.
	QUEST_REWARD_ZEN		= 0x0002,	// 젠 준다.
	QUEST_REWARD_ITEM		= 0x0004,	// 아이템 준다.
	QUEST_REWARD_BUFF		= 0x0008,	// 버프 준다.
#ifdef ASG_ADD_GENS_SYSTEM
	QUEST_REWARD_CONTRIBUTE	= 0x0010,	// 기여도를 준다.
#endif	// ASG_ADD_GENS_SYSTEM
#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	QUEST_REWARD_RANDOM		= 0x0020,	// 랜덤설정
#endif //ASG_ADD_QUEST_REQUEST_REWARD_TYPE

	QUEST_REWARD_TYPE_MAX
};

#pragma pack(push, 1)
typedef struct 
{	
	DWORD	m_dwType;
	WORD	m_wIndex;
	DWORD	m_dwValue;
	BYTE	m_byItemInfo[PACKET_ITEM_LENGTH];
} NPC_QUESTEXP_REWARD_INFO, *LPNPC_QUESTEXP_REWARD_INFO;
#pragma pack(pop)

// 요구 사항, 보상 정보
typedef struct 
{	
	NPC_QUESTEXP_REQUEST_INFO	NpcQuestRequestInfo[5];
	NPC_QUESTEXP_REWARD_INFO	NpcQuestRewardInfo[5];
} NPC_QUESTEXP_INFO, *LPNPC_QUESTEXP_INFO;

//----------------------------------------------------------------------------
// GC[0xF6][0x0D] 퀘스트 완료 결과
//----------------------------------------------------------------------------
typedef struct 
{	
	PBMSG_HEADER	Header;
	BYTE			SubCode;

	DWORD			m_dwQuestIndex;
	BYTE			m_byResult;	// 1 : 보상 정상 지급, 2 : 젠이 최대치가 넘었다. 3 : 인벤에 아이템 들어갈 자리 없다.
} PMSG_ANS_QUESTEXP_COMPLETE, *LPPMSG_ANS_QUESTEXP_COMPLETE;

//----------------------------------------------------------------------------
// GC[0xF6][0x0F] 퀘스트 포기
//----------------------------------------------------------------------------
typedef struct 
{	
	PBMSG_HEADER	Header;
	BYTE			SubCode;
	
	DWORD			m_dwQuestGiveUpIndex;
} PMSG_ANS_QUESTEXP_GIVEUP, *LPPMSG_ANS_QUESTEXP_GIVEUP;

//----------------------------------------------------------------------------
// GC[0xF6][0x1A] 진행중인 퀘스트 리스트 응답
//----------------------------------------------------------------------------
typedef struct 
{	
	PBMSG_HEADER	Header;
	BYTE			SubCode;

	BYTE			m_byQuestCount;
} PMSG_ANS_QUESTEXP_PROGRESS_LIST, *LPPMSG_ANS_QUESTEXP_PROGRESS_LIST;

#endif	// ASG_ADD_NEW_QUEST_SYSTEM

#ifdef ASG_ADD_TIME_LIMIT_QUEST

const enum QUESTEXP_RESULT
{
	QUEST_RESULT_TIME_LIMIT	= 0x00,		// 시간 제한
	QUEST_RESULT_CNT_LIMIT	= 0x01,		// 퀘스트 개수 제한.
		
	QUEST_RESULT_MAX
};

//----------------------------------------------------------------------------
// CG[0xF6][0x00] 퀘스트 관련 결과
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER	Header;
	BYTE			SubCode;

	BYTE			m_byResult;			// enum QUESTEXP_RESULT 값
} PMSG_ANS_QUESTEXP_RESULT, *LPPMSG_ANS_QUESTEXP_RESULT;
#endif	// ASG_ADD_TIME_LIMIT_QUEST

#ifdef ASG_ADD_GENS_SYSTEM
//----------------------------------------------------------------------------
// GC[0xF8][0x02] 겐스 가입 응답
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER	Header;
	BYTE			SubCode;

	BYTE			m_byResult;				// 가입 요청 결과.
	BYTE			m_byInfluence;			// 겐스 소속.
} PMSG_ANS_REG_GENS_MEMBER, *LPPMSG_ANS_REG_GENS_MEMBER;

//----------------------------------------------------------------------------
// GC[0xF8][0x04] 겐스 탈퇴 응답
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER	Header;
	BYTE			SubCode;
	
	BYTE			m_byResult;				// 탈퇴 요청 결과.
	
} PMSG_ANS_SECEDE_GENS_MEMBER, *LPPMSG_ANS_SECEDE_GENS_MEMBER;

//----------------------------------------------------------------------------
// GC[0xF8][0x07] 겐스 세력 정보  .
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER	Header;
	BYTE			SubCode;
	
	BYTE			m_byInfluence;			// 겐스 소속.
#ifdef PBG_ADD_GENSRANKING
	int				m_nRanking;				// 개인 랭킹
	int				m_nGensClass;			// 겐스 계급
	int				m_nContributionPoint;	// 기여도점수
	int				m_nNextContributionPoint;	// 다음 계급 기여도점수
#endif //PBG_ADD_GENSRANKING
} PMSG_MSG_SEND_GENS_INFO, *LPPMSG_MSG_SEND_GENS_INFO;

//----------------------------------------------------------------------------
// GC[0xF8][0x05] 뷰포트 안의 다른 플래이어의 겐스 세력 정보  
//----------------------------------------------------------------------------
typedef struct
{
	PWMSG_HEADER    Header;
	BYTE			SubCode;

	BYTE			m_byCount;	
} PMSG_SEND_GENS_MEMBER_VIEWPORT, *LPPMSG_SEND_GENS_MEMBER_VIEWPORT;

typedef struct
{
	BYTE			m_byInfluence;			// 겐스 소속.
	BYTE			m_byNumberH;
	BYTE			m_byNumberL;
#ifdef PBG_ADD_GENSRANKING
	int				m_nRanking;				// 개인 랭킹	//클라 현재 사용안함
	int				m_nGensClass;			// 겐스 계급
	int				m_nContributionPoint;	// 기여도점수	//클라 현재 사용안함
#endif //PBG_ADD_GENSRANKING
} PMSG_GENS_MEMBER_VIEWPORT_INFO, *LPPMSG_GENS_MEMBER_VIEWPORT_INFO;
#endif	// ASG_ADD_GENS_SYSTEM

#ifdef PBG_ADD_GENSRANKING
//----------------------------------------------------------------------------
// GC[0xF8][0x08] 겐스 보상
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER	Header;
	BYTE			SubCode;
	BYTE			m_byRewardResult;				// 보상요청결과
}PMSG_GENS_REWARD_CODE, *LPPMSG_GENS_REWARD_CODE;
#endif //PBG_ADD_GENSRANKING

#ifdef ASG_ADD_UI_NPC_DIALOGUE
//----------------------------------------------------------------------------
// GC[0xF9][0x01] NPC UI가 대화로 진행되는 NPC 클릭 때 응답
//----------------------------------------------------------------------------
typedef struct 
{	
	PBMSG_HEADER	Header;
	BYTE			SubCode;
	
	WORD			m_wNPCIndex;
	DWORD			m_dwContributePoint;	//기여도 포인트
} PMSG_ANS_NPC_CLICK, *LPPMSG_ANS_NPC_CLICK;
#endif	// ASG_ADD_UI_NPC_DIALOGUE

//  맵 속성 위치.
typedef struct {
    BYTE        m_byX;
    BYTE        m_byY;
}PRECEIVE_MAP_ATTRIBUTE, *LPPRECEIVE_MAP_ATTRIBUTE;

//  맵 속성 변경 정보.
typedef struct {
    PBMSG_HEADER    Header;
    BYTE            m_byType;
    BYTE            m_byMapAttr;
    BYTE            m_byMapSetType;
    BYTE            m_byCount;

    PRECEIVE_MAP_ATTRIBUTE  m_vAttribute[128*128];
}PRECEIVE_SET_MAPATTRIBUTE, *LPPRECEIVE_SET_MAPATTRIBUTE;

//  게임 진행 정보. ( 블러드 캐슬 )
typedef struct  {
    PBMSG_HEADER    Header;
    BYTE            m_byPlayState;      //  경기 상태 (블러드 캐슬--> 0:경기시작, 1:경기진행, 2:경기종료 )
    WORD            m_wRemainSec;       //  남은 시간 ( 초 )
    WORD            m_wMaxKillMonster;  //  목표 몬스터량.
    WORD            m_wCurKillMonster;  //  사냥된 몬스터량.
    WORD            m_wIndex;           //  절대 아이템을 얻는 캐릭터 인덱스
    BYTE            m_byItemType;       //  절대 아이템 타입.
}PRECEIVE_MATCH_GAME_STATE, *LPPRECEIVE_MATCH_GAME_STATE;


#ifdef YDG_ADD_NEW_DUEL_PROTOCOL
// 결투요청시 상대의 요청 결과 (1)
typedef struct		// SC4
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		nResult;
	BYTE		bIndexH;				// 상대방 사용자의 인덱스 (상위 바이트)
	BYTE		bIndexL;				// 상대방 사용자의 인덱스 (하위 바이트)
	CHAR		szID[MAX_ID_SIZE];	// 상대방 케릭이름
} PMSG_ANS_DUEL_INVITE, *LPPMSG_ANS_DUEL_INVITE;

// 결투요청받은 유저에게 응답 요청 (2)
typedef struct _tagPMSG_REQ_DUEL_ANSWER		// SC2
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		bIndexH;				// 상대방 사용자의 인덱스 (상위 바이트)
	BYTE		bIndexL;				// 상대방 사용자의 인덱스 (하위 바이트)
	CHAR		szID[MAX_ID_SIZE];	// 상대방 케릭이름
} PMSG_REQ_DUEL_ANSWER, *LPPMSG_REQ_DUEL_ANSWER;

// 결투종료 (3)
typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		nResult;
	BYTE		bIndexH;		// 상대방 사용자의 인덱스 (상위 바이트)
	BYTE		bIndexL;		// 상대방 사용자의 인덱스 (하위 바이트)
	CHAR		szID[MAX_ID_SIZE];	// 케릭이름
} PMSG_ANS_DUEL_EXIT, *LPPMSG_ANS_DUEL_EXIT;

// 결투자 점수 정보 (4)
typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		bIndexH1;		// 상대방 사용자의 인덱스 (상위 바이트) - 1번
	BYTE		bIndexL1;		// 상대방 사용자의 인덱스 (하위 바이트) - 1번
	BYTE		bIndexH2;		// 상대방 사용자의 인덱스 (상위 바이트) - 2번
	BYTE		bIndexL2;		// 상대방 사용자의 인덱스 (하위 바이트) - 2번
	BYTE		btDuelScore1;		// 결투점수 - 1번
	BYTE		btDuelScore2;		// 결투점수 - 2번
} PMSG_DUEL_SCORE_BROADCAST, *LPPMSG_DUEL_SCORE_BROADCAST;

// 결투자 생명 정보 (5)
typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		bIndexH1;		// 상대방 사용자의 인덱스 (상위 바이트) - 1번
	BYTE		bIndexL1;		// 상대방 사용자의 인덱스 (하위 바이트) - 1번
	BYTE		bIndexH2;		// 상대방 사용자의 인덱스 (상위 바이트) - 2번
	BYTE		bIndexL2;		// 상대방 사용자의 인덱스 (하위 바이트) - 2번
	BYTE		btHP1;			// 생명 - 1번(%)
	BYTE		btHP2;			// 생명 - 2번(%)
	BYTE		btShield1;		// 쉴드 - 1번(%)
	BYTE		btShield2;		// 쉴드 - 2번(%)
} PMSG_DUEL_HP_BROADCAST, *LPPMSG_DUEL_HP_BROADCAST;

// 결투 채널(관전) 리스트 (6)
typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	struct {
		CHAR	szID1[MAX_ID_SIZE];	// 케릭이름
		CHAR	szID2[MAX_ID_SIZE];	// 케릭이름
		BYTE	bStart;			// PK 시작 유/무
		BYTE	bWatch;			// 관전가능 유/무
	} channel[4];
} PMSG_ANS_DUEL_CHANNELLIST, *LPPMSG_ANS_DUEL_CHANNELLIST;

// 채널(관전) 요청 (7)
typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		nResult;
	BYTE		nChannelId;				// 채널 아이디
	CHAR		szID1[MAX_ID_SIZE];	// 케릭이름
	CHAR		szID2[MAX_ID_SIZE];	// 케릭이름
	BYTE		bIndexH1;		// 사용자의 인덱스 (상위 바이트) - 1번
	BYTE		bIndexL1;		// 사용자의 인덱스 (하위 바이트) - 1번
	BYTE		bIndexH2;		// 사용자의 인덱스 (상위 바이트) - 2번
	BYTE		bIndexL2;		// 사용자의 인덱스 (하위 바이트) - 2번
} PMSG_ANS_DUEL_JOINCNANNEL, *LPPMSG_ANS_DUEL_JOINCNANNEL;

// 채널(관전) 입장한 관전자 (8)
typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	CHAR		szID[MAX_ID_SIZE];	// 케릭이름
} PMSG_DUEL_JOINCNANNEL_BROADCAST, *LPPMSG_DUEL_JOINCNANNEL_BROADCAST;

// 채널(관전) 종료 (9)
typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		nResult;
} PMSG_ANS_DUEL_LEAVECNANNEL, *LPPMSG_ANS_DUEL_LEAVECNANNEL;

// 채널(관전) 떠난 관전자 (10)
typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	CHAR		szID[MAX_ID_SIZE];	// 케릭이름
} PMSG_DUEL_LEAVECNANNEL_BROADCAST, *LPPMSG_DUEL_LEAVECNANNEL_BROADCAST;

// 관전자 리스트(All) (11)
typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		nCount;
	struct {
		CHAR	szID[MAX_ID_SIZE];	// 케릭이름
	} user[10];
} PMSG_DUEL_OBSERVERLIST_BROADCAST, *LPPMSG_DUEL_OBSERVERLIST_BROADCAST;

// 결투결과 정보 (12)
typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	CHAR		szWinner[MAX_ID_SIZE];	// 승자 케릭이름
	CHAR		szLoser[MAX_ID_SIZE];	// 패자 케릭이름
} PMSG_DUEL_RESULT_BROADCAST, *LPPMSG_DUEL_RESULT_BROADCAST;

typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		nFlag;					// 0: 라운드 시작, 1: 라운드 종료
} PMSG_DUEL_ROUNDSTART_BROADCAST, *LPPMSG_DUEL_ROUNDSTART_BROADCAST;

#else	// YDG_ADD_NEW_DUEL_PROTOCOL
#ifdef DUEL_SYSTEM

typedef struct {
	PBMSG_HEADER    Header;
	BYTE			bStart;
	BYTE			bIndexH;
	BYTE			bIndexL;
	BYTE			ID[MAX_ID_SIZE];
}PRECEIVE_DUELSTART, *LPPRECEIVE_DUELSTART;

typedef struct {
	PBMSG_HEADER    Header;
	BYTE			bIndexH;
	BYTE			bIndexL;
	BYTE			ID[MAX_ID_SIZE];
}PRECEIVE_DUELEND, *LPPRECEIVE_DUELEND;

typedef struct {
	PBMSG_HEADER    Header;
	BYTE			bIndexH;
	BYTE			bIndexL;
	BYTE			ID[MAX_ID_SIZE];
}PNOTIFY_DUELSTART, *LPPNOTIFY_DUELSTART;

typedef struct
{
	PBMSG_HEADER	h;
	BYTE			NumberH1;		// 결투 중이었던 상대방 사용자의 인덱스 (상위 바이트) - 1번
	BYTE			NumberL1;		// 결투 중이었던 상대방 사용자의 인덱스 (하위 바이트) - 1번
	BYTE			NumberH2;		// 결투 중이었던 상대방 사용자의 인덱스 (상위 바이트) - 2번
	BYTE			NumberL2;		// 결투 중이었던 상대방 사용자의 인덱스 (하위 바이트) - 2번
	BYTE			btDuelScore1;	// 결투점수 - 1번
	BYTE			btDuelScore2;	// 결투점수 - 1번
}PNOTIFY_DUELSCORE, *LPPNOTIFY_DUELSCORE;

#endif // DUEL_SYSTEM
#endif	// YDG_ADD_NEW_DUEL_PROTOCOL

typedef struct tagPSHOPTITLE_HEADER {
	PWMSG_HEADER    Header;
	BYTE			bySubcode;
	BYTE			byCount;
} PSHOPTITLE_HEADERINFO, *LPPSHOPTITLE_HEADERINFO;
typedef struct tagPSHOPTITLE_DATA {
	BYTE	byIndexH;
	BYTE	byIndexL;
	BYTE	szTitle[36];	//. MAX_SHOPTITLE
} PSHOPTITLE_DATAINFO, *LPPSHOPTITLE_DATAINFO;
typedef struct tagPSHOPTITLE_CHANGE {
	PBMSG_HEADER	Header;
	BYTE			bySubcode;
	BYTE			byIndexH;
	BYTE			byIndexL;
	BYTE			szTitle[36];	//. MAX_SHOPTITLE
	BYTE			szId[MAX_ID_SIZE];
} PSHOPTITLE_CHANGEINFO, *LPPSHOPTITLE_CHANGEINFO;

typedef struct tagPSHOPSETPRICE_RESULT {
	PBMSG_HEADER	Header;
	BYTE			bySubcode;
	BYTE			byResult;
	BYTE			byItemPos;
} PSHOPSETPRICE_RESULTINFO, *LPPSHOPSETPRICE_RESULTINFO;

typedef struct tagCREATEPSHOP_RESULT {
	PBMSG_HEADER	Header;
	BYTE			bySubcode;
	BYTE			byResult;
} CREATEPSHOP_RESULTINFO, *LPCREATEPSHOP_RESULSTINFO;
typedef struct tagDESTROYPSHOP_RESULT {
	PBMSG_HEADER	Header;
	BYTE			bySubcode;
	BYTE			byResult;
	BYTE			byIndexH;
	BYTE			byIndexL;
} DESTROYPSHOP_RESULTINFO, *LPDESTROYPSHOP_RESULTINFO;

typedef struct tagGETPSHOPITEMLIST_HEADER {
	PWMSG_HEADER    Header;
	BYTE			bySubcode;
	BYTE			byResult;
	BYTE			byIndexH;
	BYTE			byIndexL;
	BYTE			szId[MAX_ID_SIZE];
	BYTE			szShopTitle[36];	//. MAX_SHOPTITLE
	BYTE			byCount;
} GETPSHOPITEMLIST_HEADERINFO, *LPGETPSHOPITEMLIST_HEADERINFO;
typedef struct tagGETPSHOPITEM_DATA {
	BYTE	byPos;
	BYTE	byItemInfo[PACKET_ITEM_LENGTH];
	INT		iItemPrice;
} GETPSHOPITEM_DATAINFO, *LPGETPSHOPITEM_DATAINFO;
typedef struct tagCLOSEPSHOP_RESULT {
	PWMSG_HEADER    Header;
	BYTE			bySubcode;
	BYTE			byIndexH;
	BYTE			byIndexL;
} CLOSEPSHOP_RESULTINFO, *LPCLOSEPSHOP_RESULTINFO;

typedef struct tagPURCHASEITEM_RESULT {
	PBMSG_HEADER	Header;
	BYTE			bySubcode;
	BYTE			byResult;
	BYTE			byIndexH;
	BYTE			byIndexL;
	BYTE			byItemInfo[PACKET_ITEM_LENGTH];
	BYTE			byPos;
} PURCHASEITEM_RESULTINFO, *LPPURCHASEITEM_RESULTINFO;
typedef struct tagSOLDITEM_RESULT {
	PBMSG_HEADER	Header;
	BYTE			bySubcode;
	BYTE			byPos;
	BYTE			szId[MAX_ID_SIZE];
} SOLDITEM_RESULTINFO, *LPSOLDITEM_RESULTINFO;

typedef struct tagDISPLAYEFFECT_NOTIFY {
	PBMSG_HEADER	Header;
	BYTE			byIndexH;
	BYTE			byIndexL;
	BYTE			byType;			// 포션 정보(효과타입)
} DISPLAYEREFFECT_NOTIFYINFO, *LPDISPLAYEREFFECT_NOTIFYINFO;

typedef struct {
	PWMSG_HEADER    Header;
	BYTE			MemoCount;
	BYTE			MaxMemo;
	BYTE			Count;
} FS_FRIEND_LIST_HEADER, * LPFS_FRIEND_LIST_HEADER;

typedef struct {
	BYTE			Name[MAX_ID_SIZE];
	BYTE			Server;
} FS_FRIEND_LIST_DATA, * LPFS_FRIEND_LIST_DATA;

typedef struct {
	PBMSG_HEADER    Header;
	BYTE			Result;
	BYTE			Name[MAX_ID_SIZE];
	BYTE			Server;
} FS_FRIEND_RESULT, * LPFS_FRIEND_RESULT;

typedef struct {
	PBMSG_HEADER    Header;
	BYTE			Name[MAX_ID_SIZE];
} FS_ACCEPT_ADD_FRIEND_RESULT, * LPFS_ACCEPT_ADD_FRIEND_RESULT;

typedef struct {
	PBMSG_HEADER    Header;
	BYTE			Name[MAX_ID_SIZE];
	BYTE			Server;
} FS_FRIEND_STATE_CHANGE, * LPFS_FRIEND_STATE_CHANGE;

typedef struct {
	PBMSG_HEADER    Header;
	BYTE			Result;
	DWORD			WindowGuid;
} FS_SEND_LETTER_RESULT, * LPFS_SEND_LETTER_RESULT;

typedef struct {
	PBMSG_HEADER    Header;
	WORD			Index;
	BYTE			Name[MAX_ID_SIZE];
	BYTE			Date[30];
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	BYTE			Subject[MAX_LETTER_TITLE_LENGTH];
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	BYTE			Subject[32];
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	BYTE			Read;
} FS_LETTER_ALERT, * LPFS_LETTER_ALERT;

typedef struct {
	PWMSG_HEADER    Header;
	WORD			Index;
	WORD			MemoSize;
	BYTE			Class;
	BYTE			Equipment[EQUIPMENT_LENGTH];
	BYTE			PhotoDir;
	BYTE			PhotoAction;
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	BYTE			Memo[MAX_LETTERTEXT_LENGTH];
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	BYTE			Memo[1000];
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
} FS_LETTER_TEXT, * LPFS_LETTER_TEXT;

typedef struct {
	PBMSG_HEADER    Header;
	BYTE			Result;
	WORD			Index;
} FS_LETTER_RESULT, * LPFS_LETTER_RESULT;

typedef struct {
	PBMSG_HEADER    Header;
	BYTE			IP[15];
	WORD			RoomNumber;
	DWORD			Ticket;
	BYTE			Type;
	BYTE			ID[10];
	BYTE			Result;
} FS_CHAT_CREATE_RESULT, * LPFS_CHAT_CREATE_RESULT;

typedef struct {
	PBMSG_HEADER    Header;
	BYTE			Result;
} FS_CHAT_JOIN_RESULT, * LPFS_CHAT_JOIN_RESULT;

typedef struct {
	PBMSG_HEADER    Header;
	BYTE			Type;
	BYTE			Index;
	BYTE			Name[MAX_ID_SIZE];
} FS_CHAT_CHANGE_STATE, * LPFS_CHAT_CHANGE_STATE;

typedef struct {
	PWMSG_HEADER    Header;
	WORD			RoomNumber;
	BYTE			Count;
} FS_CHAT_USERLIST_HEADER, * LPFS_CHAT_USERLIST_HEADER;

typedef struct {
	BYTE			Index;
	BYTE			Name[MAX_ID_SIZE];
} FS_CHAT_USERLIST_DATA, * LPFS_CHAT_USERLIST_DATA;

typedef struct {
	PBMSG_HEADER    Header;
	BYTE			Result;
	DWORD			WindowGuid;
} FS_CHAT_INVITE_RESULT, * LPFS_CHAT_INVITE_RESULT;

typedef struct {
	PBMSG_HEADER    Header;
	BYTE			Index;
	BYTE			MsgSize;
	BYTE			Msg[100];
} FS_CHAT_TEXT, * LPFS_CHAT_TEXT;

//----------------------------------------------------------------------------
// GC [0x2D] 아이템 효과 해제
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER	h;

	WORD		wOptionType;		// 아이템 옵션 카테고리 (아이템 옵션의 종류)
	WORD		wEffectType;		// 아이템 옵션 효과 인덱스 (wOptionType에 하위)
	BYTE		byEffectOption;		// 효과 옵션 (0 : 사용, 1 : 사용 시간 종료, 2 : 취소)
	int			wEffectTime;
	BYTE		byBuffType;
} PMSG_ITEMEFFECTCANCEL, *LPPMSG_ITEMEFFECTCANCEL;

//////////////////////////////////////////////////////////////////////////
//  
//////////////////////////////////////////////////////////////////////////
#ifdef PET_SYSTEM

typedef struct
{
    PBMSG_HEADER    m_Header;
    BYTE            m_byPetType;    //  Pet Type ( 0:다크스피릿. ... )
    BYTE            m_byCommand;    //  명령. ( 0:일반(대기), 1:랜덤 공격, 2:주인과 같이 공격, 3:타켓공격.
    BYTE            m_byKeyH;       //  대상 Key.
    BYTE            m_byKeyL;
}PRECEIVE_PET_COMMAND, *LPPRECEIVE_PET_COMMAND;


typedef struct 
{
    PBMSG_HEADER    m_Header;
    BYTE            m_byPetType;    //  Pet Type ( 0:다크스피릿. ... )
    BYTE            m_bySkillType;  //  공격 타입. ( 0:기본, etc:스킬 ).
    BYTE            m_byKeyH;       //  대상 Key;
    BYTE            m_byKeyL;
    BYTE            m_byTKeyH;      //  타켓 Key;
    BYTE            m_byTKeyL;
}PRECEIVE_PET_ATTACK, *LPPRECEIVE_PET_ATTACK;


typedef struct 
{
    PBMSG_HEADER    m_Header;
    BYTE            m_byPetType;    //  팻아이템 타입 ( 0:다크스피릿. etc ).
    BYTE            m_byInvType;    //  인벤 종류 ( 0:인벤토리, 1:창고, 2:거래창... )
    BYTE            m_byPos;        //  위치.
    BYTE            m_byLevel;      //  아이템 레벨.
    int             m_iExp;         //  경험치.
#ifdef KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
	BYTE			m_byLife;		//	생명력
#endif // KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
}PRECEIVE_PET_INFO, *LPPRECEIVE_PET_INFO;

#endif// PET_SYSTEM


//////////////////////////////////////////////////////////////////////////
//	
// From 10. 11 , 안성민
//
//////////////////////////////////////////////////////////////////////////

//Type은 0: 배틀사커, 1:길드전, 2:결투
typedef struct
{
	PBMSG_HEADER    m_Header;
	BYTE			m_subCode;
	BYTE			m_Type;	
	WORD			m_Time;		// 경기시간 : 단위(초)- 시작값은 1부터 증가
}PMSG_MATCH_TIMEVIEW, *LPPMSG_MATCH_TIMEVIEW;

typedef struct
{
	PBMSG_HEADER	m_Header;
	BYTE			m_subCode;
	BYTE			m_Type;

	char			m_MatchTeamName1[MAX_ID_SIZE];	//결투의 경우만 캐릭터이름
	WORD			m_Score1;

	char			m_MatchTeamName2[MAX_ID_SIZE];
	WORD			m_Score2;
	void			Clear()
	{ 
		memset(&m_MatchTeamName1, NULL, MAX_ID_SIZE); 
		memset(&m_MatchTeamName2, NULL, MAX_ID_SIZE);
		m_Score1 = 0;
		m_Score2 = 0;
		m_Type = 0;
	}
}PMSG_MATCH_RESULT, *LPPMSG_MATCH_RESULT;

typedef struct 
{
	PBMSG_HEADER	m_Header;
	BYTE			m_subCode;
	BYTE			m_x;
	BYTE			m_y;
}PMSG_SOCCER_GOALIN, *LPPMSG_SOCCER_GOALIN;

extern PMSG_MATCH_RESULT		g_wtMatchResult;
extern PMSG_MATCH_TIMEVIEW		g_wtMatchTimeLeft;
extern int g_iGoalEffect;

//////////////////////////////////////////////////////////////////////////

typedef struct      //  맵서버 이동을 위한 서버 정보.
{
    PBMSG_HEADER    m_Header;
    MServerInfo     m_vSvrInfo;
}PHEADER_MAP_CHANGESERVER_INFO, *LPPHEADER_MAP_CHANGESERVER_INFO;


enum CASTLESIEGE_STATE
{
	CASTLESIEGE_STATE_NONE					= -1,		// 상태없음
	CASTLESIEGE_STATE_IDLE_1				= 0,		// 유휴기간 - 1
	CASTLESIEGE_STATE_REGSIEGE				= 1,		// 신청기간 (공성주기 시작)
	CASTLESIEGE_STATE_IDLE_2				= 2,		// 유휴기간 - 1
	CASTLESIEGE_STATE_REGMARK				= 3,		// 문장 등록기간
	CASTLESIEGE_STATE_IDLE_3				= 4,		// 유휴기간 - 2
	CASTLESIEGE_STATE_NOTIFY				= 5,		// 발표기간
	CASTLESIEGE_STATE_READYSIEGE			= 6,		// 준비기간
	CASTLESIEGE_STATE_STARTSIEGE			= 7,		// 공성시작
	CASTLESIEGE_STATE_ENDSIEGE				= 8,		// 휴전기간
	CASTLESIEGE_STATE_ENDCYCLE				= 9,		// 공성주기 종료
};

//----------------------------------------------------------------------------
// GC [0xB2][0x00] 클라이언트가 공성전 상태정보를 요청 응답
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;					// 요청 결과 (0:실패/1:성공/2:성주인없음/3:성정보비정상/4:성길드없어짐) -> 성공이면 길드정보는 확실히  
	CHAR		cCastleSiegeState;			// 현재 공성전 상태
	BYTE		btStartYearH;				// 상태 시작 - 년 (상위 바이트)
	BYTE		btStartYearL;				// 상태 시작 - 년 (하위 바이트)
	BYTE		btStartMonth;				// 상태 시작 - 달
	BYTE		btStartDay;					// 상태 시작 - 일
	BYTE		btStartHour;				// 상태 시작 - 시
	BYTE		btStartMinute;				// 상태 시작 - 분
	BYTE		btEndYearH;					// 상태 종료 - 년 (상위 바이트)
	BYTE		btEndYearL;					// 상태 종료 - 년 (하위 바이트)
	BYTE		btEndMonth;					// 상태 종료 - 달
	BYTE		btEndDay;					// 상태 종료 - 일
	BYTE		btEndHour;					// 상태 종료 - 시
	BYTE		btEndMinute;				// 상태 종료 - 분
	BYTE		btSiegeStartYearH;			// 공성전 시작 - 년 (상위 바이트)
	BYTE		btSiegeStartYearL;			// 공성전 시작 - 년 (하위 바이트)
	BYTE		btSiegeStartMonth;			// 공성전 시작 - 달
	BYTE		btSiegeStartDay;			// 공성전 시작 - 일
	BYTE		btSiegeStartHour;			// 공성전 시작 - 시
	BYTE		btSiegeStartMinute;			// 공성전 시작 - 분
	CHAR		cOwnerGuild[8];				// 수성측 길드
	CHAR		cOwnerGuildMaster[10];		// 수성측 길드 마스터

	CHAR		btStateLeftSec1;			// 다음 상태까지 남은 시간 - 상위 1번째
	CHAR		btStateLeftSec2;			// 다음 상태까지 남은 시간 - 상위 2번째
	CHAR		btStateLeftSec3;			// 다음 상태까지 남은 시간 - 상위 3번째
	CHAR		btStateLeftSec4;			// 다음 상태까지 남은 시간 - 상위 4번째
} PMSG_ANS_CASTLESIEGESTATE, *LPPMSG_ANS_CASTLESIEGESTATE;

//----------------------------------------------------------------------------
// GC [0xB2][0x01] 공성전 등록을 요청 응답
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;					// 요청 결과 (0:실패/1:성공/2:이미등록됨/3:수성측길드/4:길드없음/5:길마레벨부족/6:길드에속하지않음/7:시간이지났음/8:길드원수부족)
	CHAR		szGuildName[8];				// 길드이름
} PMSG_ANS_REGCASTLESIEGE, *LPPMSG_ANS_REGCASTLESIEGE;

//----------------------------------------------------------------------------
// GC [0xB2][0x02] 공성전 포기를 요청 응답
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;					// 요청 결과 (0:실패/1:성공/2:등록된길드아님/3:시간이지났음)
	BYTE		btIsGiveUp;					// 포기여부 (0:포기않함/1:포기)
	CHAR		szGuildName[8];				// 길드이름
} PMSG_ANS_GIVEUPCASTLESIEGE, *LPPMSG_ANS_GIVEUPCASTLESIEGE;

//----------------------------------------------------------------------------
// GC [0xB2][0x03] 자신의 길드의 공성등록에 대한 정보를 요청 응답
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;					// 요청 결과 (0:실패/1:성공/2:등록된길드아님)
	CHAR		szGuildName[8];				// 길드이름
	BYTE		btGuildMark1;				// 성주의 표식 - DWORD를 최상위 바이트 부터 4단계로 나눔
	BYTE		btGuildMark2;
	BYTE		btGuildMark3;
	BYTE		btGuildMark4;
	BYTE		btIsGiveUp;					// 포기여부
	BYTE		btRegRank;					// 등록순위
} PMSG_ANS_GUILDREGINFO, *LPPMSG_ANS_GUILDREGINFO;

//----------------------------------------------------------------------------
// GC [0xB2][0x04] 자신의 길드의 성주의 표식 등록 요청 응답
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;					// 요청 결과 (0:실패/1:성공/2:등록된길드아님/3:잘못된아이템등록)
	CHAR		szGuildName[8];				// 길드이름
	BYTE		btGuildMark1;				// 성주의 표식 - DWORD를 최상위 바이트 부터 4단계로 나눔
	BYTE		btGuildMark2;
	BYTE		btGuildMark3;
	BYTE		btGuildMark4;
} PMSG_ANS_REGGUILDMARK, *LPPMSG_ANS_REGGUILDMARK;

//----------------------------------------------------------------------------
// GC [0xB2][0x05] 수성측 NPC 구매 요청 응답 -> 권한체크, 수성측만 가능 (0xC1)
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;					// 요청 결과 (0:실패/1:성공/2:권한이없음/3:돈이모자람/4:이미존재)
	INT			iNpcNumber;					// NPC번호
	INT			iNpcIndex;					// NPC인덱스
} PMSG_ANS_NPCBUY, *LPPMSG_ANS_NPCBUY;

//----------------------------------------------------------------------------
// GC [0xB2][0x06] 수성측 NPC 수리 요청 응답 -> 권한체크, 수성측만 가능 (0xC1)
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;					// 요청 결과 (0:실패/1:성공/2:권한이없음/3:돈이모자람)
	INT			iNpcNumber;					// NPC번호
	INT			iNpcIndex;					// NPC인덱스
	INT			iNpcHP;						// 체력
	INT			iNpcMaxHP;					// 최대체력
} PMSG_ANS_NPCREPAIR, *LPPMSG_ANS_NPCREPAIR;

//----------------------------------------------------------------------------
// GC [0xB2][0x07] 수성측 NPC 업그레이드 요청 응답 -> 권한체크, 수성측만 가능 (0xC1)
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;					// 요청 결과 (0:실패/1:성공/2:권한이없음/3:돈이모자람/4:보석이모자람/5:타입이잘못됨/6:요청값이잘못됨/7:NPC가존재하지않음)
	INT			iNpcNumber;					// NPC번호
	INT			iNpcIndex;					// NPC인덱스
	INT			iNpcUpType;					// 업그레이드 타입 (1:방어력/2:회복력/3:내구도(최대생명))
	INT			iNpcUpValue;				// 업그레이드 수치 (3번 내구도의 경우 실제로 올라갈 내구도 값)
} PMSG_ANS_NPCUPGRADE, *LPPMSG_ANS_NPCUPGRADE;

//----------------------------------------------------------------------------
// CG [0xB2][0x08] 현재 성의 자금, 세금 정보 요청 응답 (0xC1)
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;					// 요청 결과 (0:실패/1:성공/2:권한이없음)
	BYTE		btTaxRateChaos;				// 현재 성의 세율 - 카오스조합
	BYTE		btTaxRateStore;				// 현재 성의 세율 - 상점
	BYTE		btMoney1;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 1)
	BYTE		btMoney2;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 2)
	BYTE		btMoney3;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 3)
	BYTE		btMoney4;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 4)
	BYTE		btMoney5;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 5)
	BYTE		btMoney6;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 6)
	BYTE		btMoney7;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 7)
	BYTE		btMoney8;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 8)
} PMSG_ANS_TAXMONEYINFO, *LPPMSG_ANS_TAXMONEYINFO;

//----------------------------------------------------------------------------
// CG [0xB2][0x09] 현재 성의 세율 변경 요청 응답 (0xC1)
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;					// 요청 결과 (0:실패/1:성공/2:권한이없음)
	BYTE		btTaxType;					// 세율 종류 (1:카오스조합/2:상점)
	BYTE		btTaxRate1;					// 변경된 현재 세율
	BYTE		btTaxRate2;					// 변경된 현재 세율
	BYTE		btTaxRate3;					// 변경된 현재 세율
	BYTE		btTaxRate4;					// 변경된 현재 세율
} PMSG_ANS_TAXRATECHANGE, *LPPMSG_ANS_TAXRATECHANGE;

//----------------------------------------------------------------------------
// CG [0xB2][0x10] 현재 성의 자금 출금 요청 응답 (0xC1)
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;					// 요청 결과 (0:실패/1:성공/2:권한이없음)
	BYTE		btMoney1;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 1)
	BYTE		btMoney2;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 2)
	BYTE		btMoney3;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 3)
	BYTE		btMoney4;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 4)
	BYTE		btMoney5;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 5)
	BYTE		btMoney6;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 6)
	BYTE		btMoney7;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 7)
	BYTE		btMoney8;					// 현재 성의 자금 (INT64의 최상위 바이트 부터 8)
} PMSG_ANS_MONEYDRAWOUT, *LPPMSG_ANS_MONEYDRAWOUT;

//----------------------------------------------------------------------------
// CG [0xB2][0x1A] 세율 변경 (0xC1)
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btTaxType;					// 세율 형태 (1:카오스조합/2:상점)
	BYTE		btTaxRate;					// 세율
} PMSG_ANS_MAPSVRTAXINFO, *LPPMSG_ANS_MAPSVRTAXINFO;

//----------------------------------------------------------------------------
// GC [0xB3] 공성관련 DB로 관리되는 특정 NPC의 목록을 요청 응답 (0xC2)
//----------------------------------------------------------------------------
typedef struct
{
    PREQUEST_DEFAULT_SUBCODE    m_Header;
	BYTE		btResult;					// 요청 결과 (0:실패/1:성공/2:권한이없음)
	INT			iCount;
} PMSG_ANS_NPCDBLIST, *LPPMSG_ANS_NPCDBLIST;
typedef struct
{
	INT			iNpcNumber;					// NPC번호
	INT			iNpcIndex;					// NPC인덱스
	INT			iNpcDfLevel;				// 방어력 레벨
	INT			iNpcRgLevel;				// 회복력 레벨
	INT			iNpcMaxHp;					// 최대 HP
	INT			iNpcHp;						// HP
	BYTE		btNpcX;						// 좌표 - X
	BYTE		btNpcY;						// 좌표 - Y
	BYTE		btNpcLive;					// NPC가 현재 존재하는지 여부
} PMSG_NPCDBLIST, *LPPMSG_NPCDBLIST;


//----------------------------------------------------------------------------
// GC [0xB4] 공성 등록 길드목록 요청 응답 (0xC2)
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	h;
	BYTE		btResult;					// 요청 결과 (0:실패/1:성공)
	INT			iCount;
} PMSG_ANS_CSREGGUILDLIST, *LPPMSG_ANS_CSREGGUILDLIST;
typedef struct
{
	CHAR		szGuildName[8];				// 길드이름
	BYTE		btRegMarks1;				// 성주의 표식의 개수 (INT의 상위 바이트 부터 1)
	BYTE		btRegMarks2;				// 성주의 표식의 개수 (INT의 상위 바이트 부터 2)
	BYTE		btRegMarks3;				// 성주의 표식의 개수 (INT의 상위 바이트 부터 3)
	BYTE		btRegMarks4;				// 성주의 표식의 개수 (INT의 상위 바이트 부터 4)
	BYTE		btIsGiveUp;					// 포기여부 (0:포기안함/1:포기함)
	BYTE		btSeqNum;					// 등록순서
} PMSG_CSREGGUILDLIST, *LPPMSG_CSREGGUILDLIST;

//----------------------------------------------------------------------------
// GC [0xB5] 공성 확정 길드목록 요청 응답 (0xC2)
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	h;
	BYTE		btResult;					// 요청 결과 (0:실패/1:성공/2:기간아님/3:데이터준비않됨)
	INT			iCount;
} PMSG_ANS_CSATTKGUILDLIST, *LPPMSG_ANS_CSATTKGUILDLIST;
typedef struct
{
	BYTE		btCsJoinSide;				// 참여측 정보 (1:수성측/2이상:공성측)
	BYTE		btGuildInvolved;			// 길드가 연합의 주측 길드인지 여부 (0:아님/1:주축임)
	CHAR		szGuildName[8];				// 길드이름
	INT			iGuildScore;				// 길드 선정 당시의 점수 <<--- 추가됨
} PMSG_CSATTKGUILDLIST, *LPPMSG_CSATTKGUILDLIST;


typedef struct 
{
	PBMSG_HEADER    m_Header;
	BYTE            m_bySubCode;
}PBMSG_HEADER2;

//----------------------------------------------------------------------------
// GC [0xA4][0x00] 유저의 몬스터 킬 카운트 정보를 되돌려준다.
//----------------------------------------------------------------------------
typedef struct
{ 
	PBMSG_HEADER2	h;
	BYTE	byResult;		// 결과 0x00:실패(에러)
							//		0x01:성공 
							//		0x02:이미 완료한 퀘스트
							//		0x03:진행중인 퀘스트 아님
							//		0x04:퀘스트가 없음(퀘스트 번호가 잘못왔음)
							//		0x05:완료조건이 킬카운트가 아님(클라이언트에서 요청하면 안됨)
		
	BYTE	byQuestIndex;		// 퀘스트 인덱스
	int		anKillCountInfo[10];// 몬스터 킬카운트 정보
} PMSG_ANS_QUEST_MONKILL_INFO, *LPPMSG_ANS_QUEST_MONKILL_INFO;

    //////////////////////////////////////////////////////////////////////////
    //  GC [0xB2][0x1F] 성 사냥터 입장 가능 여부 지정 요청 응답 ( 0xC1 )
    //////////////////////////////////////////////////////////////////////////
    typedef struct 
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byResult;
        BYTE                m_byHuntZoneEnter;
    }PMSG_CSHUNTZONEENTER, *LPPMSG_CSHUNTZONEENTER;

    //////////////////////////////////////////////////////////////////////////
    //  GC [0xB9][0x03] 성 사냥터 문지기가 그 사냥터의 여러 정보를 알려준다.
    //////////////////////////////////////////////////////////////////////////
    typedef struct 
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byResult;         //  0:일반인, 1:일반인, 2:연합(길드)원, 3:연합(길드)마스터.
        BYTE                m_byEnable;         //  0:일반인 입장 불가, 1:일반인 입장 가능.
        INT                 m_iCurrPrice;       //  현재 설정된 입장료.
        INT                 m_iMaxPrice;        //  최대 받을수 있는 입장료.
        INT                 m_iUnitPrice;       //  증가 단위.
    }PRECEIVE_CASTLE_HUNTZONE_INFO, *LPPRECEIVE_CASTLE_HUNTZONE_INFO; 

    //////////////////////////////////////////////////////////////////////////
    //  GC [0xB9][0x05] 성 사냥터 입장 결과.
    //////////////////////////////////////////////////////////////////////////
    typedef struct 
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byResult;     //  0:입장불가, 1:입장가능
    }PRECEVIE_CASTLE_HUNTZONE_RESULT, *LPPRECEVIE_CASTLE_HUNTZONE_RESULT;

    //////////////////////////////////////////////////////////////////////////
    //  GC [0xB9][0x02] 성을 차지한 길드 마크.
    //////////////////////////////////////////////////////////////////////////
    typedef struct 
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byGuildMark[32];
    }PRECEIVE_CASTLE_FLAG, *LPPRECEIVE_CASTLE_FLAG;

    //////////////////////////////////////////////////////////////////////////
    //  성문.
    //////////////////////////////////////////////////////////////////////////
    typedef struct  //  성문 정보.
    {
        PBMSG_HEADER2        m_Header;           //  헤더.
        BYTE                m_byResult;         //  성문 정보 요청 결과값. ( 0:실패, 1:성공, 2:성문없음, 3:레버없음, 4:권한없음 )
        BYTE                m_byKeyH;           //  
        BYTE                m_byKeyL;
    }PRECEIVE_GATE_STATE, *LPPRECEIVE_GATE_STATE;


    typedef struct  //  성문에 대한 명령 결과.
    {
        PBMSG_HEADER2        m_Header;
        BYTE                m_byResult;         //  성문 정보 요청 결과값. ( 0:실패, 1:성공, 2:성문없음, 3:레버없음, 4:권한없음 )
        BYTE                m_byOperator;       //  작동 결과.
        BYTE                m_byKeyH;
        BYTE                m_byKeyL;
    }PRECEIVE_GATE_OPERATOR, *LPPRECEIVE_GATE_OPERATOR;


    typedef struct  //  성문의 상태.
    {
        PBMSG_HEADER2        m_Header;
        BYTE                m_byOperator;       //  작동 상황 ( 0:닫기, 1:열기 )
        BYTE                m_byKeyH;
        BYTE                m_byKeyL;
    }PRECEIVE_GATE_CURRENT_STATE, *LPPRECEIVE_GATE_CURRENT_STATE;


    typedef struct  //  스위치 작동 여부.
    {
        PBMSG_HEADER2       m_Header;   
        BYTE                m_byIndexH;         //  왕관 스위치 인덱스.
        BYTE                m_byIndexL;         //  
        BYTE                m_byKeyH;           //  사용자 스위치 인덱스.
        BYTE                m_byKeyL;           //  
        BYTE                m_byState;          //  상태. ( 0:스위치땜, 1:스위치 작동, 2:다른이가 사용중 )
    }PRECEIVE_SWITCH_PROC, *LPPRECEIVE_SWITCH_PROC;


    typedef struct  //  왕관의 상태.
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byCrownState;     //  왕관의 상태 ( 0:직인등록, 1:직인등록성공, 2:직인등록실패, 3:다른사람이 사용중 )

		DWORD				m_dwCrownAccessTime;

    }PRECEIVE_CROWN_STATE, *LPPRECEIVE_CROWN_STATE;


	typedef struct
	{
		PBMSG_HEADER2		m_Header;
		BYTE				m_byIndex1;
		BYTE				m_byIndex2;
		BYTE				m_bySwitchState;
		BYTE				m_JoinSide;

		BYTE				m_szGuildName[8];
		BYTE				m_szUserName[MAX_ID_SIZE+1];
	}PRECEIVE_CROWN_SWITCH_INFO, *LPRECEIVE_CROWN_SWITCH_INFO;


    typedef struct  //  공성전 진행 상태 알림.
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byBasttleCastleState; //  0:왕의 직인 등록, 1:중간 승리.
        BYTE                m_szGuildName[8];       //  등록, 승리한 길드 이름.
    }PRECEIVE_BC_PROCESS, *LPPRECEIVE_BC_PROCESS;


    //////////////////////////////////////////////////////////////////////////
    //  
    //////////////////////////////////////////////////////////////////////////
    typedef struct //   몬스터 생성 시간.
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byKeyH;
        BYTE                m_byKeyL;

        BYTE                m_byBuildTime;
    }PRECEIVE_MONSTER_BUILD_TIME, *LPPRECEIVE_MONSTER_BUILD_TIME;


    //////////////////////////////////////////////////////////////////////////
    //  공성무기
    //////////////////////////////////////////////////////////////////////////
    typedef struct  //  Npc선택시 선택한 Npc의 상태값.
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byResult;         //  NPC요청 결과. ( 0:실패, 1:성공, 2... ).

        BYTE                m_byWeaponType;     //  무기 종류.

        BYTE                m_byKeyH;           //  NPC Key값.
        BYTE                m_byKeyL;
    }PRECEIVE_CATAPULT_STATE, *LPPRECEIVE_CATAPULT_STATE;


    typedef struct  //  공성 무기 발사 요청 결과.
    {
        PBMSG_HEADER2        m_Header;
        BYTE                m_byResult;         //  발사 요청 결과.
        BYTE                m_byKeyH;           //  발사하는 무기의 키값.
        BYTE                m_byKeyL;           

        BYTE                m_byWeaponType;     //  무기 종류 ( 0: 공성, 1:수성 )

        BYTE                m_byTargetX;        //  공격 위치 X.
        BYTE                m_byTargetY;        //  공격 위치 Y.
    }PRECEIVE_WEAPON_FIRE, *LPPRECEIVE_WEAPON_FIRE;


    typedef struct  //  투석기 공습 경보.
    {
        PBMSG_HEADER2        m_Header;
        BYTE                m_byWeaponType;     //  무기 종류 ( 0:공성, 1:수성 )

        BYTE                m_byTargetX;        //  공격 위치 X.
        BYTE                m_byTargetY;        //  공격 위치 Y.
    }PRECEIVE_BOMBING_ALERT, *LPPRECEIVE_BOMBING_ALERT;


    typedef struct //   공격을 받는 대상 정보.
    {
        PBMSG_HEADER2        m_Header;
        BYTE                m_byKeyH;           //  공격 당하는 Key값.
        BYTE                m_byKeyL;

        BYTE                m_byWeaponType;     //  무기 종류 ( 0:공성, 1:수성 ).
    }PRECEIVE_BOMBING_TARGET, *LPPRECEIVE_BOMBING_TARGET;


    typedef struct //   미리 임시적인 위치의 뷰포트를 받는다.
    {
        BYTE                m_byObjType;        //  오브젝트 종류. ( 0:캐릭터, 1:몬스터, 2:NPC )
        BYTE                m_byTypeH;
        BYTE                m_byTypeL;
        BYTE                m_byKeyH;
        BYTE                m_byKeyL;
        BYTE                m_byPosX;
        BYTE                m_byPosY;
    	BYTE                m_byEquipment[EQUIPMENT_LENGTH];
		BYTE				s_BuffCount;
		BYTE				s_BuffEffectState[MAX_BUFF_SLOT_INDEX];     
    }PRECEIVE_PREVIEW_PORT, *LPPRECEIVE_PREVIEW_PORT;


    typedef struct 
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byTeam;
        BYTE                m_byX;
        BYTE                m_byY;
        BYTE                m_byCmd;
    }PRECEIVE_GUILD_COMMAND, *LPPRECEIVE_GUILD_COMMAND;


    typedef struct 
    {
        BYTE                m_byX;
        BYTE                m_byY;
    }PRECEIVE_MEMBER_LOCATION, *LPPRECEIVE_MEMBER_LOCATION;


    typedef struct 
    {
        BYTE                m_byType;           //  오브젝트 종류.
        BYTE                m_byX;
        BYTE                m_byY;
    }PRECEIVE_NPC_LOCATION, *LPPRECEIVE_NPC_LOCATION;


    typedef struct
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byResult;         //  데이터 요청 결과.
    }PRECEIVE_MAP_INFO_RESULT, *LPPRECEIVE_MAP_INFO_RESULT;

    typedef struct  //  공성전 중 남은 시간을 사용자들에게 주기적으로 알림. 
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byHour;
        BYTE                m_byMinute;
    }PRECEIVE_MATCH_TIMER, *LPPRECEIVE_MATCH_TIMER;
	
	typedef struct  
	{
		PBMSG_HEADER2		m_Header;	
		BYTE				m_iJewelType;		//보석 타입 ( 0: 축석 / 1: 영석)
		BYTE				m_iJewelMix;		//조합 타입 ( 0, 1, 2순으로 10개, 20개, 30개)
	}PMSG_REQ_JEWEL_MIX, *LPPMSG_REQ_JEWEL_MIX;

	typedef struct 
	{
		PBMSG_HEADER2		m_Header;
		BYTE				m_iResult;			//0 : 오류, 1: 성공
												//2 : 보석 타입 오류, 3: 조합타입 오류, 4: 보석 부족, 5: 조합금 부족
	}PMSG_ANS_JEWEL_MIX, *LPPMSG_ANS_JEWEL_MIX;

	typedef struct  
	{
		PBMSG_HEADER2		m_Header;			
		BYTE				m_iJewelType;		//보석 타입
		BYTE				m_iJewelLevel;		//조합된 레벨
		BYTE				m_iJewelPos;		//인벤토리 내 인덱스
	}PMSG_REQ_JEWEL_UNMIX, *LPPMSG_REQ_JEWEL_UNMIX;

	typedef struct  
	{
		PBMSG_HEADER2		m_Header;
		
		int		iMasterLevelSkill;	// 습득하려는 스킬 번호
	}PMSG_REQ_MASTERLEVEL_SKILL, *LPPMSG_REQ_MASTERLEVEL_SKILL;

	typedef struct 
	{
		PBMSG_HEADER2		m_Header;
		BYTE				m_iResult;			//0: 오류, 1: 성공
												//2: 보석 타입 오류, 3: 보석 레벨 오류 4: 아이템 없음
												//5: 인벤토리 범위 오류, 6: 해체할 아이템 오류
												//7: 인벤토리 공간 부족, 8: 조합금이 부족
	}PMSG_ANS_JEWEL_UNMIX, *LPPMSG_ANS_JEWEL_UNMIX;

////////////////////////////////////////////////////////////////////////////
//  크라이울프 1차 MVP 프로토콜 ( 0xBD 사용 )
////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------
// GC [0xBD][0x00] 클라이언트가 크라이울프 점령 및 상태정보 요청에 대한 응답
//--------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;

	BYTE			btOccupationState;	// 점령 정보 (0:평화, 1:점령, 2:전쟁)
	BYTE			btCrywolfState;		// 크라이울프 진행 상태

} PMSG_ANS_CRYWOLF_INFO, *LPPMSG_ANS_CRYWOLF_INFO;

//--------------------------------------------------------------------------
// GC [0xBD][0x02] 방어막, 제단 상태 정보
//--------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;

	INT				iCrywolfStatueHP;	// 방어막 HP
	BYTE			btAltarState1;		// 제단 1 상태
	BYTE			btAltarState2;		// 제단 2 상태
	BYTE			btAltarState3;		// 제단 3 상태
	BYTE			btAltarState4;		// 제단 4 상태
	BYTE			btAltarState5;		// 제단 5 상태

} PMSG_ANS_CRYWOLF_STATE_ALTAR_INFO, *LPPMSG_ANS_CRYWOLF_STATE_ALTAR_INFO;

//--------------------------------------------------------------------------
// GC [0xBD][0x03] 클라이언트가 특정 제단과 사제 계약 요청에 대한 응답
//--------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;

	BYTE			bResult;			// 계약 결과 (0:실패, 1:성공)
	BYTE			btAltarState;		// 제단 상태
	BYTE			btObjIndexH;		// 제단 인덱스
	BYTE			btObjIndexL;		// 제단 인덱스

} PMSG_ANS_CRYWOLF_ALTAR_CONTRACT, *LPPPMSG_ANS_CRYWOLF_ALTAR_CONTRACT;

//--------------------------------------------------------------------------
// GC [0xBD][0x04] 크라이울프 진행 시간
//--------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;

	BYTE			btHour;				// 남은 시간
	BYTE			btMinute;			// 남은 분

} PMSG_ANS_CRYWOLF_LEFTTIME, *LPPPMSG_ANS_CRYWOLF_LEFTTIME;

//--------------------------------------------------------------------------
// GC [0xBD][0x0C] 크라이울프 MVP 탱커 공격 영역 알림
//--------------------------------------------------------------------------
typedef struct
{
    PBMSG_HEADER2       h;

    BYTE                btObjClassH;        //  몬스터 종류
    BYTE                btObjClassL;           

	BYTE                btSourceX;        //  발사 위치 X.
    BYTE                btSourceY;        //  발사 위치 Y.

    BYTE                btPointX;        //  폭발 위치 X.
    BYTE                btPointY;        //  폴발 위치 Y.
} PMSG_NOTIFY_REGION_MONSTER_ATTACK, *LPPMSG_NOTIFY_REGION_MONSTER_ATTACK;

typedef struct
{
	PBMSG_HEADER2	h;

	int				btBossHP;			//발가스 HP
	BYTE			btMonster2;			//다크엘프 마리수
	
} PMSG_ANS_CRYWOLF_BOSSMONSTER_INFO, *LPPMSG_ANS_CRYWOLF_BOSSMONSTER_INFO;


typedef struct 
{
	PBMSG_HEADER2	h;

	BYTE			btPlusChaosRate;	// 크라이울프 혜택
} PMSG_ANS_CRYWOLF_BENEFIT_PLUS_CHAOSRATE, *LPPMSG_ANS_CRYWOLF_BENEFIT_PLUS_CHAOSRATE;


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

typedef struct
{
	PBMSG_HEADER2	h;
	BYTE			btRank;			//0 : D    1 : C   2 : B   3 : A   4 : S
	int				iGettingExp;	//획득경험치
} PMSG_ANS_CRYWOLF_PERSONAL_RANK, *LPPMSG_ANS_CRYWOLF_PERSONAL_RANK;

typedef struct
{
	PBMSG_HEADER2	h;
	BYTE			btCount;
} PMSG_ANS_CRYWOLF_HERO_LIST_INFO_COUNT, *LPPMSG_ANS_CRYWOLF_HERO_LIST_INFO_COUNT;

typedef struct
{
	BYTE iRank;
	char szHeroName[MAX_ID_SIZE];
	int iHeroScore;
	BYTE btHeroClass;
} PMSG_ANS_CRYWOLF_HERO_LIST_INFO, *LPPMSG_ANS_CRYWOLF_HERO_LIST_INFO;
//////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
//  칸투르 관련  프로토콜 ( 0xD1 사용 )
////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// CG [0xD1][0x00] 클라이언트가 칸투르 보스전의 현재 상태 정보
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;

	BYTE	btState;			// 칸투르 보스전 상태
	BYTE	btDetailState;		// 칸투르 보스전 세부 상태

	BYTE	btEnter;			// 입장 가능 여부 ( 0:불가 1:가능 )
	BYTE	btUserCount;		// 입장이 가능 한 경우 현재 입장 인원
	int		iRemainTime;		// 입장이 가능한 경우 입장 종료까지 남은 시간(Sec)
	
} PMSG_ANS_KANTURU_STATE_INFO, *LPPMSG_ANS_KANTURU_STATE_INFO;

//----------------------------------------------------------------------------
// GC [0xD1][0x01] 클라이언트의 칸투르 보스전 전투맵 입장 요청에 대한 응답
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;
	
	BYTE		btResult;	// 0:입장가능
							// 1:입장 가능한 인원 수 초과
							// 2:입장에 필요한 아이템이 없음
							// 3:입장가능한 상태가 아님

} PMSG_ANS_ENTER_KANTURU_BOSS_MAP, *LPPMSG_ANS_ENTER_KANTURU_BOSS_MAP;

//----------------------------------------------------------------------------
// GC [0xD1][0x02] 유저가 칸투르 보스전 전투맵에 입장했을 때 현재 상태를 보낸다
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;

	BYTE		btCurrentState;			// 칸투르의 현재 메인 상태
	BYTE		btCurrentDetailState;	// 그에 대한 세부 상태

} PMSG_ANS_KANTURU_CURRENT_STATE, *LPPMSG_ANS_KANTURU_CURRENT_STATE;

//----------------------------------------------------------------------------
// GC [0xD1][0x03] 칸투르 보스전 도중 상태가 변했을 때 상태값을 알린다
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;

	BYTE		btState;			// 칸투르의 메인 상태
	BYTE		btDetailState;		// 그에 대한 세부 상태
	
} PMSG_ANS_KANTURU_STATE_CHANGE, *LPPMSG_ANS_KANTURU_STATE_CHANGE;

//----------------------------------------------------------------------------
// GC [0xD1][0x04] 칸투르 보스전 전투 성공 / 실패
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;

	BYTE		btResult;	// 0:실패 1:성공
	
} PMSG_ANS_KANTURU_BATTLE_RESULT, *LPPMSG_ANS_KANTURU_BATTLE_RESULT;

//----------------------------------------------------------------------------
// GC [0xD1][0x05] 칸투르 보스전 전투 연출마다 제한 시간
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;

	int			btTimeLimit;	// 제한 시간(Sec)
	
} PMSG_ANS_KANTURU_BATTLE_SCENE_TIMELIMIT, *LPPMSG_ANS_KANTURU_BATTLE_SCENE_TIMELIMIT;

//----------------------------------------------------------------------------
// GC [0xD1][0x06] 칸투르 보스전 광역 공격( 마야 스킬 사용 )
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;

	BYTE		btObjClassH;	// 몬스터 종류
	BYTE		btObjClassL;
	
	BYTE		btType;			// 0 : 아이스스톰  1:브로큰샤워

} PMSG_NOTIFY_KANTURU_WIDE_AREA_ATTACK, *LPPMSG_NOTIFY_KANTURU_WIDE_AREA_ATTACK;

//----------------------------------------------------------------------------
// GC [0xD1][0x07] 칸투르 입장 유저&몬스터 수 디스플레이
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;

	BYTE		bMonsterCount;		// 유저에게 보내 줄 몬스터 수
	BYTE		btUserCount;		// 유저에게 보내 줄 유저 수

} PMSG_NOTIFY_KANTURU_USER_MONSTER_COUNT, *LPPMSG_NOTIFY_KANTURU_USER_MONSTER_COUNT;


#ifdef ADD_PCROOM_POINT_SYSTEM
//----------------------------------------------------------------------------
// PC방 포인트 시스템 : 0xD0 사용
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// GC [0xD0][0x04] PC방 포인트 정보 요청 응답
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;	
	short		nPoint;			// 현재 포인트
	short		nMaxPoint;		// 최대 포인트
#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
	BYTE		bPCBang;		// PC방인가 아닌가?	
#endif // CSK_MOD_MOVE_COMMAND_WINDOW	
} PMSG_ANS_GC_PCROOM_POINT_INFO, *LPPMSG_ANS_GC_PCROOM_POINT_INFO;

//----------------------------------------------------------------------------
// GC [0xD0][0x05] PC방 포인트 사용 요청 응답
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		byResult;	// 사용 결과 0x00:성공 
							//			0x01:안전지대 아님 
							//			0x02:해당위치에 아이템이 없음 
							//			0x03:인벤토리 공간 부족
							//			0x04:기본 조건 포인트부족
							//			0x05:PC방 포인트 시스템 대상자가 아님
							//			0x06:에러
							//			0x07:상점 사용 가능 시간 초과
							//			0x08:PC방 포인트 전용 아이템이 아님
							//			0x09:구입 가능한 포인트 부족
	BYTE		byInvenPos;		// 인벤토리 위치	0xff일 경우 버프.
						// *인장은 성공시 바로 적용하고 아이템은 성공시 인벤에 아이템 추가한다.
	BYTE		ItemInfo[PACKET_ITEM_LENGTH];
} PMSG_ANS_GC_PCROOM_POINT_USE, *LPPMSG_ANS_GC_PCROOM_POINT_USE;

//----------------------------------------------------------------------------
// CG GC [0xD0][0x06] PC방 포인트 샾 열림/닫힘
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		byResult;	// 결과	0x00:오픈 
							//		0x01:안전지대 아님
							//		0x02:상점에 아이템 없음 
							//		0x03:다른 창 사용 중 
							//		0x04:PC방 포인트 시스템 대상자가 아님							
							//		0x05:에러
} PMSG_REQ_PCROOM_POINT_SHOP_OPEN, *LPPMSG_REQ_PCROOM_POINT_SHOP_OPEN;

#endif	// ADD_PCROOM_POINT_SYSTEM

//----------------------------------------------------------------------------
// CG [0xBF][0x00] 입장 요청
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE			btCursedTempleIdx;// 저주받은 사원 번호
	BYTE			iItemPos;// 피의 두루마리 위치
} PMSG_REQ_ENTER_CURSED_TEMPLE, *LPPMSG_REQ_ENTER_CURSED_TEMPLE;

//----------------------------------------------------------------------------
// GC [0xBF][0x00] 입장 요청 결과
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE			Result;		/* 요청결과 (	0 : 성공
											1 : 알맞은 피의 두루마리 없음
											2 : 들어갈수 있는 시간이 아님
											3 : 피의 두루마리는 맞으나 자신의 레벨과 맞지 않음
											4 : 인원제한이 넘었음
											5 : 1일 입장제한 횟수를 넘었음
											6 : 입장료가 모자란다.
											7 : 카오유저는 입장할 수 없다.
											8 : 변신반지를 착용하고 입장할 수 없다.
										)
							    */
} PMSG_RESULT_ENTER_CURSED_TEMPLE, *LPPMSG_RESULT_ENTER_CURSED_TEMPLE;

//----------------------------------------------------------------------------
// GC [0xBF][0x01] 진행 상태 / 시간 정보 / 파티 위치정보
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	WORD		wRemainSec;			// 게임시작 후 종료까지 남은 초
	WORD		btUserIndex;		// 성물을 가지고 있는 사용자 인덱스
	BYTE		btX;				// 성물 위치 X
	BYTE		btY;				// 성물 위치 Y
	BYTE		btAlliedPoint;		// 연합군측 점수
	BYTE		btIllusionPoint;	// 환영교단측 점수
	BYTE		btMyTeam;		    // 0:연합군, 1: 환영교단

	BYTE		btPartyCount;
} PMSG_CURSED_TAMPLE_STATE, *LPPMSG_CURSED_TAMPLE_STATE;

typedef struct 
{
	WORD		wPartyUserIndex;
	BYTE		byMapNumber;
	BYTE		btX;
	BYTE		btY;	
} PMSG_CURSED_TAMPLE_PARTY_POS, *LPPMSG_CURSED_TAMPLE_PARTY_POS;

//----------------------------------------------------------------------------
// CG [0xBF][0x02]	저주받은 사원 전용 마법사용
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;

	BYTE          MagicH;
	BYTE          MagicL;
	WORD		wTargetObjIndex;	// 타겟 인덱스
	BYTE		Dis;				// 거리
} PMSG_CURSED_TEMPLE_USE_MAGIC, *LPPMSG_CURSED_TEMPLE_USE_MAGIC;

//----------------------------------------------------------------------------
// GC [0xBF][0x03]	저주받은 사원에 입장한 인원수를 전송
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		btUserCount[6];		// 각 저주받은 사원의 인원수
} PMSG_CURSED_TEMPLE_USER_COUNT, *LPPMSG_CURSED_TEMPLE_USER_COUNT;

//----------------------------------------------------------------------------
// GC [0xBF][0x02]	저주받은 사원 전용 마법사용 결과
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE        MagicResult;        // 0 실패, 1 성공

	BYTE          MagicH;
	BYTE          MagicL;

	WORD		wSourceObjIndex;	// 소스 인덱스
	WORD		wTargetObjIndex;	// 타겟 인덱스
} PMSG_CURSED_TEMPLE_USE_MAGIC_RESULT, *LPPMSG_CURSED_TEMPLE_USE_MAGIC_RESULT;

//----------------------------------------------------------------------------
// GC [0xBF][0x04] 이벤트 경기 결과
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		btAlliedPoint;		// 연합군측 점수
	BYTE		btIllusionPoint;	// 환영교단측 점수

	BYTE		btUserCount;		// 캐릭터 수
} PMSG_CURSED_TEMPLE_RESULT, *LPPMSG_CURSED_TEMPLE_RESULT;

typedef struct 
{
	char		GameId[MAX_ID_SIZE];		// 캐릭터이름
	BYTE		byMapNumber;				// 맵번호
	BYTE		btTeam;						// 팀
	BYTE		btClass;					// 클래스
	int			nAddExp;					// 보상 경험치
} PMSG_CURSED_TEMPLE_USER_ADD_EXP, *LPPMSG_CURSED_TEMPLE_USER_ADD_EXP;


//----------------------------------------------------------------------------
// GC [0xBF][0x06] 저주받은 사원에서 몬스터를 죽여서 얻는 스킬 포인트
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
	BYTE		btSkillPoint;		// 현재의 스킬 포인트
} PMSG_CURSED_TEMPLE_SKILL_POINT, *LPPMSG_CURSED_TEMPLE_SKILL_POINT;

//----------------------------------------------------------------------------
// GC [0xBF][0x07] 저주받은 사원에서 전용스킬 종료
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;

	BYTE          MagicH;
	BYTE          MagicL;

	WORD		wObjIndex;
} PMSG_CURSED_TEMPLE_SKILL_END, *LPPMSG_CURSED_TEMPLE_SKILL_END;

//----------------------------------------------------------------------------
// GC [0xBF][0x08] 환영 사원에서 성물을 획득
//----------------------------------------------------------------------------
typedef struct  
{
	PBMSG_HEADER2 h;
	WORD		wUserIndex;
	char		Name[MAX_ID_SIZE];	
} PMSG_RELICS_GET_USER, *LPPMSG_RELICS_GET_USER;

//----------------------------------------------------------------------------
// GC [0xBF][0x09] 환영 사원의 상태 전송
//----------------------------------------------------------------------------
typedef struct  
{
	PBMSG_HEADER2 h;
	BYTE		btTempleNumber;
	BYTE		btIllusionTempleState;	// 0: wait, 1: wait->ready, 2: ready->play, 3: play->end, 
} PMSG_ILLUSION_TEMPLE_STATE, *LPPMSG_ILLUSION_TEMPLE_STATE;

//----------------------------------------------------------------------------
// GC [0xBF][0x0a] 체인마법 전송
//----------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////
// 엘리스 체인라이트닝 시전시 서버로부터 받는 패킷
// 보낼때는 SendRequestMagicContinue() 로  
/////////////////////////////////////////////////////////////////////
// Receive Chain Magic			
typedef struct {
	PBMSG_HEADER2	Header;

	BYTE			MagicH;
	BYTE			MagicL;

	WORD			wUserIndex;
	BYTE			byCount;
} PRECEIVE_CHAIN_MAGIC, *LPPRECEIVE_CHAIN_MAGIC;

	/////////////////////////////////////////////////////////////////////
	// ReceiveChainMagic()의 wCount 갯수대로 서버로부터 패킷을 받는다.
	/////////////////////////////////////////////////////////////////////
	// Receive Chain Magic Object
	typedef struct {
		WORD			wTargetIndex;
	} PRECEIVE_CHAIN_MAGIC_OBJECT, *LPPRECEIVE_CHAIN_MAGIC_OBJECT;

typedef struct 
{
	PBMSG_HEADER  h;	
	BYTE		subcode;	
	
	short		nMLevel;		// 마스터레벨

	BYTE		btMExp1;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 1)
	BYTE		btMExp2;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 2)
	BYTE		btMExp3;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 3)
	BYTE		btMExp4;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 4)
	BYTE		btMExp5;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 5)
	BYTE		btMExp6;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 6)
	BYTE		btMExp7;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 7)
	BYTE		btMExp8;					// 현재 마스터레벨 경험치 (INT64의 최상위 바이트 부터 8)

	BYTE		btMNextExp1;				// 다음 마스터레벨 경험치 (INT64의 최상위 바이트 부터 1)
	BYTE		btMNextExp2;				// 다음 마스터레벨 경험치 (INT64의 최상위 바이트 부터 2)
	BYTE		btMNextExp3;				// 다음 마스터레벨 경험치 (INT64의 최상위 바이트 부터 3)
	BYTE		btMNextExp4;				// 다음 마스터레벨 경험치 (INT64의 최상위 바이트 부터 4)
	BYTE		btMNextExp5;				// 다음 마스터레벨 경험치 (INT64의 최상위 바이트 부터 5)
	BYTE		btMNextExp6;				// 다음 마스터레벨 경험치 (INT64의 최상위 바이트 부터 6)
	BYTE		btMNextExp7;				// 다음 마스터레벨 경험치 (INT64의 최상위 바이트 부터 7)
	BYTE		btMNextExp8;				// 다음 마스터레벨 경험치 (INT64의 최상위 바이트 부터 8)

	// 마스터레벨 포인트(미정)
	// 마스터스킬 및 마스터스킬 트리 정보(미정)

	short		nMLPoint;					// 마스터레벨 포인트

	// 마스터레벨에 의해 정보가 재설정된다.
	WORD		wMaxLife;
	WORD		wMaxMana;
	WORD		wMaxShield;
	WORD		wMaxSkillMana;
} PMSG_MASTERLEVEL_INFO, *LPPMSG_MASTERLEVEL_INFO;

typedef struct 
{
	PBMSG_HEADER	h;
	BYTE		subcode;

	short		nMLevel;			// 현재 마스터레벨

	short		nAddMPoint;			// 추가된 마스터레벨 포인트
	short       nMLevelUpMPoint;	// 사용 가능한 마스터레벨 포인트
//	short		nTotalMPoint;		// 내가 가진 총 마스터레벨 포인트
	short		nMaxPoint;			// 최대 마스터레벨 포인트

	WORD		wMaxLife;
	WORD		wMaxMana;
	WORD		wMaxShield;
	WORD		wMaxBP;
} PMSG_MASTERLEVEL_UP, *LPPMSG_MASTERLEVEL_UP;

typedef struct  
{
	PBMSG_HEADER	h;
	BYTE		subcode;

	BYTE		btResult;	// 결과값	0:에러 1:습득했다. 2:습득 가능한 사용자 아님 3:습득 가능한 스킬번호 아님
							//			4:마스터레벨 포인트 부족 5:그 외

	short       nMLPoint;	// 현재 마스터레벨 포인트

	int			nSkillNum;
	int			nSkillLevel;
} PMSG_ANS_MASTERLEVEL_SKILL, *LPPMSG_ANS_MASTERLEVEL_SKILL;

#ifdef PJH_CHARACTER_RENAME
//----------------------------------------------------------------------------
// GC [0xF3][0x15] 캐릭터명 변경 확인요청 결과
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
	char		Name[MAX_ID_SIZE];
	BYTE		btResult;		// 0: 불필요, 1: 필요
} PMSG_ANS_CHECK_CHANGE_NAME, *LPPMSG_ANS_CHECK_CHANGE_NAME;

//----------------------------------------------------------------------------
// GC [0xF3][0x16] 캐릭터명 변경 요청 결과
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
	char		Name[MAX_ID_SIZE];
	char		NewName[MAX_ID_SIZE];
	BYTE		btResult;		// 0: 성공, 1: 동일 캐릭터명 존재
} PMSG_CHANGE_NAME_RESULT, *LPPMSG_CHANGE_NAME_RESULT;
#endif //PJH_CHARACTER_RENAME

#ifdef CSK_RAKLION_BOSS
//----------------------------------------------------------------------------
// GC [0xD1][0x10] 유저의 라클리온 보스전의 현재 상태 정보 요청에 대한 응답
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;
	
	BYTE			btState;		// 라클리온 보스전 상태
	BYTE			btDetailState;	// 라클리온 보스전 세부 상태
	
	BYTE			btEnter;		// 입장 가능 여부 ( 0:불가 1:가능 )
	int				iRemainTime;	// 입장이 가능한 경우 입장 종료까지 남은 시간(Sec)
} PMSG_ANS_RAKLION_STATE_INFO, *LPPMSG_ANS_RAKLION_STATE_INFO;

//----------------------------------------------------------------------------
// GC [0xD1][0x11] 유저가 라클리온 보스전 전투맵에 입장했을 때 현재 상태를 보낸다.
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;
	
	BYTE			btCurrentState;				// 라클리온의 현재 메인 상태
	BYTE			btCurrentDetailState;		// 그에 대한 세부 상태
} PMSG_ANS_RAKLION_CURRENT_STATE, *LPPMSG_ANS_RAKLION_CURRENT_STATE;

//----------------------------------------------------------------------------
// GC [0xD1][0x12] 라클리온 보스전 도중 상태가 변했을 때 상태값을 알린다.
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;	
	
	BYTE			btState;			// 라클리온의 메인 상태
	BYTE			btDetailState;		// 그에 대한 세부 상태	
} PMSG_ANS_RAKLION_STATE_CHANGE, *LPPMSG_ANS_RAKLION_STATE_CHANGE;

//----------------------------------------------------------------------------
// GC [0xD1][0x13] 라클리온 보스전 전투 성공 / 실패
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;
	
	BYTE			btResult;			// 0:실패 1:성공
} PMSG_ANS_RAKLION_BATTLE_RESULT, *LPPMSG_ANS_RAKLION_BATTLE_RESULT;

//----------------------------------------------------------------------------
// GC [0xD1][0x14] 라클리온 보스전 광역 공격( 세루판 스킬 사용 )
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	
	BYTE			btObjClassH;		// 몬스터 종류
	BYTE			btObjClassL;
	
	BYTE			btType;				// 스킬 종류
	
} PMSG_NOTIFY_RAKLION_WIDE_AREA_ATTACK, *LPPMSG_NOTIFY_RAKLION_WIDE_AREA_ATTACK;

//----------------------------------------------------------------------------
// GC [0xD1][0x15] 라클리온 입장 유저&몬스터 수 디스플레이
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	
	BYTE			btMonsterCount;		// 유저에게 보내 줄 몬스터 수
	BYTE			btUserCount;		// 유저에게 보내 줄 유저 수
} PMSG_NOTIFY_RAKLION_USER_MONSTER_COUNT, *LPPMSG_NOTIFY_RAKLION_USER_MONSTER_COUNT;

#endif // CSK_RAKLION_BOSS

#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
//----------------------------------------------------------------------------
// CG[0xBF][0x0b] 등록된 동전 개수 요청
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
	int			nRegCoinCnt;	// 등록할 동전 개수
} PMSG_REQ_GET_COIN_COUNT, *LPPMSG_REQ_GET_COIN_COUNT;

//----------------------------------------------------------------------------
// GC[0xBF][0x0b] 등록된 동전 개수 응답 
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
	int			nCoinCnt;					// 등록된 동전 개수
}PMSG_ANS_GET_COIN_COUNT, *LPPMSG_ANS_GET_COIN_COUNT;

//----------------------------------------------------------------------------
// CG[0xBF][0x0c] 동전등록 요청
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
}PMSG_REQ_REGEIST_COIN, *LPPMSG_REQ_REGEIST_COIN;

//----------------------------------------------------------------------------
// GC0xBF][0x0c] 동전 등록 응답
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
	BYTE		btResult;						// 0:실패, 1:성공
	int			nCurCoinCnt;					// 현재 등록된 동전 개수
}PMSG_ANS_REGEIST_COIN, *LPPMSG_ANS_REGEIST_COIN;

//----------------------------------------------------------------------------
// CG[0xBF][0x0d] 동전 교환 요청
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
	int			nCoinCnt;	// 교환 개수
}PMSG_REQ_TRADE_COIN, *LPPMSG_REG_TREADE_COIN;

//----------------------------------------------------------------------------
// GC[0xBF][0x0d] 동전 교환 응답
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
	BYTE		btResult;	// 0: 동전 개수 부족, 1: 성공
}PMSG_ANS_TRADE_COIN, *LPPMSG_ANS_TREADE_COIN;

#endif //KJH_PBG_ADD_SEVEN_EVENT_2008

#ifdef YDG_ADD_DOPPELGANGER_PROTOCOLS
//----------------------------------------------------------------------------
// GC [0xBF][0x0E] 도플갱어맵 입장 요청 결과
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		btResult;		/* 요청결과 (	0 : 성공
											1 : 차원의마경 없음
											2 : 이미 다른 파티가 사용중
											3 : 카오유저는 입장할 수 없다.
											4 : 변신반지를 착용하고 입장할 수 없다.
										)
							*/
} PMSG_RESULT_ENTER_DOPPELGANGER, *LPPMSG_RESULT_ENTER_DOPPELGANGER;
//----------------------------------------------------------------------------
// GC [0xBF][0x0F] 도플갱어 선두 몬스터 위치
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		btPosIndex;	// 위치번호(0~9) 몬스터 리젠지점이 0
} PMSG_DOPPELGANGER_MONSTER_POSITION, *LPPMSG_DOPPELGANGER_MONSTER_POSITION;
//----------------------------------------------------------------------------
// GC [0xBF][0x10] 도플갱어의 상태 전송
//----------------------------------------------------------------------------
typedef struct  
{
	PBMSG_HEADER2 h;
	BYTE		btDoppelgangerState;	// 0: wait, 1: wait->ready, 2: ready->play, 3: play->end, 
} PMSG_DOPPELGANGER_STATE, *LPPMSG_DOPPELGANGER_STATE;
//----------------------------------------------------------------------------
// GC [0xBF][0x11] 도플갱어의 아이스워커의 상태 전송
//----------------------------------------------------------------------------
typedef struct  
{
	PBMSG_HEADER2 h;
	BYTE		btIceworkerState;	// 0: 출현, 1: 사라짐
	BYTE		btPosIndex;	// 위치번호(0~9) 몬스터 리젠지점이 0
} PMSG_DOPPELGANGER_ICEWORKER_STATE, *LPPMSG_DOPPELGANGER_ICEWORKER_STATE;
//----------------------------------------------------------------------------
// GC [0xBF][0x12] 도플갱어의 진행 상태 / 시간 정보 / 파티 위치정보
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	WORD		wRemainSec;			// 게임시작 후 종료까지 남은 초	
	BYTE		btUserCount;		// 유저수
	BYTE		btDummy;
	BYTE		UserPosData;
} PMSG_DOPPELGANGER_PLAY_INFO, *LPPMSG_DOPPELGANGER_PLAY_INFO;

typedef struct 
{
	WORD		wUserIndex;			// 사용자 인덱스
	BYTE		byMapNumber;		// 맵번호
	BYTE		btPosIndex;	// 위치번호(0~9) 몬스터 리젠지점이 0
} PMSG_DOPPELGANGER_USER_POS, *LPPMSG_DOPPELGANGER_USER_POS;
//----------------------------------------------------------------------------
// GC [0xBF][0x13] 도플갱어의 결과
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		btResult;		// 0: 성공, 1: 실패(죽어서실패_일반), 2: 실패(방어실패)
	DWORD		dwRewardExp;	// 보상경험치
}PMSG_DOPPELGANGER_RESULT, *LPPMSG_DOPPELGANGER_RESULT;
//----------------------------------------------------------------------------
// GC [0xBF][0x14] 도플갱어의 결과
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2 h;
	BYTE		btMaxGoalCnt;
	BYTE		btGoalCnt;	
}PMSG_DOPPELGANGER_MONSTER_GOAL, * LPPMSG_DOPPELGANGER_MONSTER_GOAL;

#endif	// YDG_ADD_DOPPELGANGER_PROTOCOLS

#ifdef PBG_ADD_SECRETBUFF
//----------------------------------------------------------------------------
// GC [0xBF][0x15] 피로도의 퍼센트
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2 h;
	BYTE		btFatiguePercentage;
}PMSG_FATIGUEPERCENTAGE, * LPPMSG_FATIGUEPERCENTAGE;
#endif //PBG_ADD_SECRETBUFF


#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
//----------------------------------------------------------------------------
// GC [0xBF][0x20] 인벤장착 아이템 결과
// btResult: 1-장착, 2-해제
//----------------------------------------------------------------------------
typedef struct _tagPMSG_ANS_INVENTORY_EQUIPMENT_ITEM
{
	PBMSG_HEADER2	h;
	
	BYTE	btItemPos;		// 아이템 위치
	BYTE	btResult;		// 아이템 상태 결과값
} PMSG_ANS_INVENTORY_EQUIPMENT_ITEM, *LPPMSG_ANS_INVENTORY_EQUIPMENT_ITEM;
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY

#ifdef YDG_MOD_PROTECT_AUTO_V5
//----------------------------------------------------------------------------
// GC[0x8A] 전체 접속자에게 해당 프로그램을 사용중인지 확인 요첟
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER	h;
	DWORD		dwKey;
	char		Filename[MAX_HACKTOOL_FILENAME_LENGTH];
}PMSG_REQ_HACKTOOL_STATISTICS, *LPPMSG_REQ_HACKTOOL_STATISTICS;
#endif	// YDG_MOD_PROTECT_AUTO_V5

#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL
//----------------------------------------------------------------------------
// GC[0x8E][0x01] 맵 이동 채크섬 데이터를 전송한다.
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;
	DWORD		dwKeyValue;
}PMSG_MAPMOVE_CHECKSUM, *LPPMSG_MAPMOVE_CHECKSUM;

//----------------------------------------------------------------------------
// GC[0x8E][0x03] 클라이언트의 맵 이동 결과
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;
	BYTE		btResult;		
}PMSG_ANS_MAPMOVE, *LPPMSG_ANS_MAPMOVE;
#endif	// YDG_ADD_MOVE_COMMAND_PROTOCOL

#ifdef LDK_ADD_EMPIREGUARDIAN_PROTOCOLS
//----------------------------------------------------------------------------
// GC [0xF7][0x02] 제국 수호군맵 입장 요청 결과
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		Result;		/* 요청결과 (0 : 성공, 1 : 이미 게임 진행중, 2 : 퀘스트 아이템이 부족하다.)*/
	BYTE		Day;			/* 요일맵 (1:일, 2:월, 3:화, 4:수, 5:목, 6:금, 7:토)*/
	BYTE		Zone;			/* 존번호 (존 1, 2, 3)*/
	BYTE		Wheather;		/* 날씨 (0:맑음, 1:비, 2:안개, 3:비+번개)*/
	DWORD		RemainTick;		// Result값이 1일경우 대기 시간값이 들어있음
} PMSG_RESULT_ENTER_EMPIREGUARDIAN, *LPPMSG_RESULT_ENTER_EMPIREGUARDIAN;

//----------------------------------------------------------------------------
// GC [0xF7][0x04] 제국 수호군맵 시간 정보 갱신
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		Type;			/* 요청결과 (0 : 대기, 1 : 루팅, 2 : 타임어택)*/
	DWORD		RemainTick;		// 대기, 남은 타임 어택 시간을 알려줌.
	BYTE		MonsterCount;	// 남은 몬스터 수
} PMSG_REMAINTICK_EMPIREGUARDIAN, *LPPMSG_REMAINTICK_EMPIREGUARDIAN;

//----------------------------------------------------------------------------
// GC [0xF7][0x06] 제국 수호군맵 타임어택 결과
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		Result;			/* 요청결과 (0 : 실패, 1 : 현재 존 성공, 2 : 모든 존 성공)*/
	DWORD		Exp;			// result값이 2일경우 경험치 량이 들어있음
} PMSG_CLEAR_RESULT_EMPIREGUARDIAN, *LPPMSG_CLEAR_RESULT_EMPIREGUARDIAN;
#endif //LDK_ADD_EMPIREGUARDIAN_PROTOCOLS

// 인게임샵
//////////////////////////////////////////////////////////////////////////
// 클라이언트 - 서버간의 캐쉬샵 연동 프로토콜
//////////////////////////////////////////////////////////////////////////
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#pragma pack(push, 1)

//----------------------------------------------------------------------------
// 사용자의 캐쉬 포인트 정보 요청 (0xD2)(0x01)
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2		h;
}PMSG_CASHSHOP_CASHPOINT_REQ, *LPPMSG_CASHSHOP_CASHPOINT_REQ;

//----------------------------------------------------------------------------
// 사용자의 캐쉬 포인트 정보 전달 (0xD2)(0x01)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	BYTE				btViewType;

	double				dTotalCash;
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	double				dCashCredit;		// C (CreditCard)
	double				dCashPrepaid;		// P (PrepaidCard)
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	double				dTotalPoint;
	double				dTotalMileage;
}PMSG_CASHSHOP_CASHPOINT_ANS, *LPPMSG_CASHSHOP_CASHPOINT_ANS;

//----------------------------------------------------------------------------
// 캐쉬샵 Open/Close 요청 (0xD2)(0x02)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	BYTE				byShopOpenType;				// 샵 인터페이스 오픈 타입 ( 0 : 열기, 1 : 닫기 )
}PMSG_CASHSHOP_SHOPOPEN_REQ, *LPPMSG_CASHSHOP_SHOPOPEN_REQ;

//----------------------------------------------------------------------------
// 캐쉬샵 Open 결과 (0xD2)(0x02)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	BYTE				byShopOpenResult;		// 샵 인터페이스 결과 ( 0 : 실패, 1 : 성공 )
}PMSG_CASHSHOP_SHOPOPEN_ANS, *LPPMSG_CASHSHOP_SHOPOPEN_ANS;

//----------------------------------------------------------------------------
// 아이템 구매 요청 (0xD2)(0x03)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;
	
	long				lBuyItemPackageSeq;		// 포장 상품 순번 (패키지 인덱스)
	long				lBuyItemDisplaySeq;		// 전시 정보 순번 (전시상품 인덱스)
	long				lBuyItemPriceSeq;		// 가격 순번
	WORD				wItemCode;				// 구매할 아이템 코드
}PMSG_CASHSHOP_BUYITEM_REQ, *LPPMSG_CASHSHOP_BUYITEM_REQ;

//----------------------------------------------------------------------------
// 아이템 구매 결과 (0xD2)(0x03)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	BYTE				byResultCode;			// 아이템 구매 결과 (구매결과코드 참조)
	long				lItemLeftCount;			// 아이템 판매 잔여 개수 (업데이트)
}PMSG_CASHSHOP_BUYITEM_ANS, *LPPMSG_CASHSHOP_BUYITEM_ANS;

//----------------------------------------------------------------------------
// 아이템 선물 요청 (0xD2)(0x04)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	long				lGiftItemPackageSeq;				// 선물 아이템 포장 상품 순번
	long				lDiftItemDisplaySeq;				// 전시 정보 순번
	long				lGiftItemPriceSeq;					// 선물 아이템 가격 순번
	long				lSaleZone;							// 판매 영역
	// 사용자 아이디 필요
	char				chReceiveUserID[MAX_ID_SIZE+1];
	char				chMessage[MAX_GIFT_MESSAGE_SIZE];
}PMSG_CASHSHOP_GIFTSEND_REQ, *LPPMSG_CASHSHOP_GIFTSEND_REQ;

//----------------------------------------------------------------------------
// 아이템 선물 결과 (0xD2)(0x04)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	BYTE				byResultCode;
	long				lItemLeftCount;
	double				dLimitedCash;
}PMSG_CASHSHOP_GIFTSEND_ANS, *LPPMSG_CASHSHOP_GIFTSEND_ANS;

//----------------------------------------------------------------------------
// 보관함 리스트 요청 (0xD2)(0x05)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;
#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
	int					iPageIndex;						// 요청 페이지 인덱스
	char				chStorageType;					// 보관함 타입 ( S : 본인 보관함, G : 선물함, A : 전체)
#endif// KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
}PMSG_CASHSHOP_STORAGELIST_REQ, *LPPMSG_CASHSHOP_STORAGELIST_REQ;

//----------------------------------------------------------------------------
// 보관함 리스트 카운트 전달 (0xD2)(0x06)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
	WORD				wTotalItemCount;				// 보관함 전체 아이템 카운트
	WORD				wCurrentItemCount;				// 현재 페이지 아이템 개수
	WORD				wPageIndex;						// 현재 페이지 인덱스
	WORD				wTotalPage;						// 전체 페이지 카운트
#else // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
	WORD				wItemCount;					// 보관함/선물함 카운트
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
}PMSG_CASHSHOP_STORAGECOUNT, *LPPMSG_CASHSHOP_STORAGECOUNT;

//----------------------------------------------------------------------------
// 보관함 리스트 전달 (0xD2)(0x0D)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	long				lStorageIndex;
	long				lItemSeq;
	long				lStorageGroupCode;
	long				lProductSeq;
	long				lPriceSeq;
	double				dCashPoint;
	char				chItemType;
}PMSG_CASHSHOP_STORAGELIST, *LPPMSG_CASHSHOP_STORAGELIST;

//----------------------------------------------------------------------------
// 선물함 아이템 리스트 전달 (0xD2)(0x0E)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;
	
	long				lStorageIndex;
	long				lItemSeq;
	long				lStorageGroupCode;
	long				lProductSeq;
	long				lPriceSeq;
	double				dCashPoint;
	char				chItemType;

	char				chSendUserName[MAX_ID_SIZE+1];
	char				chMessage[MAX_GIFT_MESSAGE_SIZE];
}PMSG_CASHSHOP_GIFTSTORAGELIST, *LPPMSG_CASHSHOP_GIFTSTORAGELIST;

//----------------------------------------------------------------------------
// 캐쉬선물 요청 (0xD2)(0x07)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	double				dCashValue;

	char				chReceiveUserID[MAX_ID_SIZE+1];
	char				chMessage[MAX_GIFT_MESSAGE_SIZE];
}PMSG_CASHSHOP_CASHSEND_REQ, *LPPMSG_CASHSHOP_CASHSEND_REQ;

//----------------------------------------------------------------------------
// 캐쉬선물 결과 (0xD2)(0x07)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	double				dGiftCashLimit;
	BYTE				byResultCode;
}PMSG_CASHSHOP_CASHSEND_ANS, *LPPMSG_CASHSHOP_CASHSEND_ANS;

//----------------------------------------------------------------------------
// 해당 사용자가 상품의 구매/선물이 가능한지 확인 요청 (0xD2)(0x08)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;
}PMSG_CASHSHOP_ITEMBUY_CONFIRM_REQ, *LPPMSG_CASHSHOP_ITEMBUY_CONFIRM_REQ;

//----------------------------------------------------------------------------
// 해당 사용자가 상품의 구매/선물이 가능한지 확인 결과 (0xD2)(0x08)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	double				dPresentedCash;					// 한달 동안 선물한 캐쉬 합계
	double				dPresenteLimitCash;				// 한달 동안 선물 가능한 캐쉬 합계

	BYTE				byResult;
	BYTE				byItemBuyPossible;				// 아이템 구매 가능 여부 ( 0 : 불가, 1 : 가능 )
	BYTE				byPresendPossible;				// 아이템 선물 가능 여부 ( 0 : 불가, 1 : 가능 )
}PMSG_CASHSHOP_ITEMBUY_CONFIRM_ANS, *LPPMSG_CASHSHOP_ITEMBUY_CONFIRM_ANS;

//----------------------------------------------------------------------------
// 상품 잔여 개수 조회 요청 (0xD2)(0x09)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	long				lPackageSeq;					// 전시상품 코드
}PMSG_CASHSHOP_ITEMBUY_LEFT_COUNT_REQ, *LPPMSG_CASHSHOP_ITEMBUY_LEFT_COUNT_REQ;

//----------------------------------------------------------------------------
// 상품 잔여 개수 조회 결과 (0xD2)(0x09)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	long				lPackageSeq;					// 전시상품 순번
	long				lLeftCount;						// 잔여 개수
}PMSG_CASHSHOP_ITEMBUY_LEFT_COUNT_ANS, *LPPMSG_CASHSHOP_ITEMBUY_LEFT_COUNT_ANS;

//----------------------------------------------------------------------------
// 보관함 아이템 버리기 요청 (0xD2)(0x0A)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	long				lStorageSeq;					// 보관함 순번
	long				lStorageItemSeq;				// 보관함 상품 순번
	char				chStorageItemType;				// 상품 구분 ( C : 캐쉬, P : 상품 )
}PMSG_CASHSHOP_STORAGE_ITEM_THROW_REQ, *LPPMSG_CASHSHOP_STORAGE_ITEM_THROW_REQ;

//----------------------------------------------------------------------------
// 보관함 아이템 버리기 결과 (0xD2)(0x0A)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	BYTE				byResult;						// 결과 코드 ( 0 : 성공, 1 : 보관함 버리기 항목이 없음, -1 : 오류 발생, -2 : DB 접근 오류 )
}PMSG_CASHSHOP_STORAGE_ITEM_THROW_ANS, *LPPMSG_CASHSHOP_STORAGE_ITEM_THROW_ANS;

//----------------------------------------------------------------------------
// 보관함 아이템 사용 요청 (0xD2)(0x0B)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	long				lStorageSeq;					// 보관함 순번
	long				lStorageItemSeq;				// 보관함 상품 순번

	WORD				wItemCode;						// 사용할 아이템의 아이템 코드
	char				chStorageItemType;				// 상품 구분 ( C : 캐쉬, P : 상품 )
}PMSG_CASHSHOP_STORAGE_ITEM_USE_REQ, *LPPMSG_CASHSHOP_STORAGE_ITEM_USE_REQ;

//----------------------------------------------------------------------------
// 보관함 아이템 사용 결과 (0xD2)(0x0B)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	BYTE				byResult;						// 결과 코드
}PMSG_CASHSHOP_STORAGE_ITEM_USE_ANS, *LPPMSG_CASHSHOP_STORAGE_ITEM_USE_ANS;

//----------------------------------------------------------------------------
// 아이템 리스트 버전 업데이트 (0xD2)(0x0C)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;
	
	WORD				wSaleZone;
	WORD				wYear;
	WORD				wYearIdentify;
}PMSG_CASHSHOP_VERSION_UPDATE, *LPPMSG_CASHSHOP_VERSION_UPDATE;

//----------------------------------------------------------------------------
// 이벤트 아이템 리스트 요청 (0xD2)(0x13)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2			h;
	
	long				lEventCategorySeq;			// 하위 이벤트 카테고리 Sequence
}PMSG_CASHSHOP_EVENTITEM_REQ, *LPPMSG_CASHSHOP_EVENTITEM_REQ;


//----------------------------------------------------------------------------
// 이벤트 아이템 리스트 카운트 (0xD2)(0x13)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2			h;
	
	WORD				wEventItemListCount;						// 전체 아이템 리스트 카운트
}PMSG_CASHSHOP_EVENTITEM_COUNT, *LPPMSG_CASHSHOP_EVENTITEM_COUNT;


//----------------------------------------------------------------------------
// 이벤트 아이템 리스트 (0xD2)(0x14)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2			h;
	
	long				lPackageSeq[INGAMESHOP_DISPLAY_ITEMLIST_SIZE];			// 이벤트 아이템 리스트를 9개 단위로 보낸다.
}PMSG_CASHSHOP_EVENTITEM_LIST, *LPPMSG_CASHSHOP_EVENTITEM_LIST;

//----------------------------------------------------------------------------
// 배너 버전 업데이트 (0xD2)(0x15)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2			h;
	
	WORD				wBannerZone;
	WORD				wYear;
	WORD				wYearIdentify;
}PMSG_CASHSHOP_BANNER_UPDATE, *LPPMSG_CASHSHOP_BANNER_UPDATE;

#pragma pack(pop)

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM


#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM

//----------------------------------------------------------------------------
// 기간제 아이템 리스트 카운트 (0xD2)(0x11)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;
	
	BYTE				byPeriodItemCount;				// 기간제 아이템 카운트
}PMSG_PERIODITEMEX_ITEMCOUNT, *LPPMSG_PERIODITEMEX_ITEMCOUNT;


//----------------------------------------------------------------------------
// 기간제 아이템 리스트 (0xD2)(0x12)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;
	
	WORD				wItemCode;						// 아이템 코드
	WORD				wItemSlotIndex;					// 아이템 슬롯 인덱스
	long				lExpireDate;					// 만료시간 (CTime으로 사용)
}PMSG_PERIODITEMEX_ITEMLIST, *LPPMSG_PERIODITEMEX_ITEMLIST;

#endif // KJH_ADD_PERIOD_ITEM_SYSTEM

#ifdef PBG_MOD_GAMEGUARD_HANDLE
//----------------------------------------------------------------------------
// ?????????
//----------------------------------------------------------------------------
typedef struct  
{
	PBMSG_HEADER2		h;
	WORD				wCharInfo;
}PMSG_GAMEGUARD_HADLE, *LPPMSG_GAMEGUARD_HADLE;
#endif //PBG_MOD_GAMEGUARD_HANDLE

#ifdef KJH_ADD_DUMMY_SKILL_PROTOCOL
//----------------------------------------------------------------------------
// 더미 프로토콜 초기화 (0x8D)(0x05)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;
	DWORD				dwDummySeedValue;
#ifdef KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	DWORD				dwAttackSerialSeedValue;
#endif // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
}PMSG_SET_ATTACK_PROTOCOL_DUMMY_VALUE, *LPPMSG_SET_ATTACK_PROTOCOL_DUMMY_VALUE;
#endif // KJH_ADD_DUMMY_SKILL_PROTOCOL

#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
//----------------------------------------------------------------------------
// 레이지파이터 스킬0x4A
//----------------------------------------------------------------------------
typedef struct {
	PBMSG_HEADER  Header;
	BYTE          AttackH;
	BYTE          AttackL;
	BYTE          SourceKeyH;
	BYTE          SourceKeyL;
	BYTE          TargetKeyH;
	BYTE          TargetKeyL;
} PRECEIVE_STRAIGHTATTACK, * LPPRECEIVE_STRAIGHTATTACK;
//----------------------------------------------------------------------------
// 타겟인덱스요청0x4B
//----------------------------------------------------------------------------
typedef struct {
	PBMSG_HEADER Header;
	WORD		 MagicNumber;
	WORD		 TargerIndex[DARKSIDE_TARGET_MAX];
} PRECEIVE_DARKSIDE_INDEX, * LPPRECEIVE_DARKSIDE_INDEX;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
//////////////////////////////////////////////////////////////////////////
// ?????????????????????????????????????
//////////////////////////////////////////////////////////////////////////
extern CWsctlc SocketClient;
extern int HeroKey;

extern int SummonLife;
extern int SendGetItem;
extern int SendDropItem;

extern bool EnableGuildWar;
extern int  GuildWarIndex;
extern char GuildWarName[8+1];
extern int  GuildWarScore[2];

extern bool EnableSoccer;
extern BYTE HeroSoccerTeam;
extern int  SoccerTime;
extern char SoccerTeamName[2][8+1];
extern bool SoccerObserver;

#ifdef PKD_ADD_ENHANCED_ENCRYPTION
extern CSessionCryptor g_SessionCryptorCS;
extern CSessionCryptor g_SessionCryptorSC;
#else
extern CSimpleModulus g_SimpleModulusCS;
extern CSimpleModulus g_SimpleModulusSC;
#endif // PKD_ADD_ENHANCED_ENCRYPTION

#ifdef ACC_PACKETSIZE
extern int g_iTotalPacketRecv;
extern int g_iTotalPacketSend;
extern DWORD g_dwPacketInitialTick;
#endif //ACC_PACKETSIZE


BOOL CreateSocket(char *IpAddr, unsigned short Port);
void DeleteSocket();
void ProtocolCompiler( CWsctlc *pSocketClient = &SocketClient, int iTranslation = 0, int iParam = 0);
void ReceiveCharacterList(BYTE* ReceiveBuffer);
void ReceiveMovePosition(BYTE* ReceiveBuffer);
void ReceiveMoveCharacter(BYTE* ReceiveBuffer);


void InitGame ();
void InitGuildWar();


bool Check_Switch (PRECEIVE_CROWN_SWITCH_INFO *Data);
bool Delete_Switch();

typedef struct _CROWN_SWITCH_INFO
{
	BYTE				m_bySwitchState;
	BYTE				m_JoinSide;
	BYTE				m_szGuildName[9];
	BYTE				m_szUserName[MAX_ID_SIZE+1];

	_CROWN_SWITCH_INFO()
	{
		Reset();	
	}

	void Reset()
	{
		m_bySwitchState = 0;
		m_JoinSide = 0;
		ZeroMemory(m_szGuildName, sizeof(m_szGuildName));
		ZeroMemory(m_szUserName, sizeof(m_szUserName));
	}
} CROWN_SWITCH_INFO;
extern CROWN_SWITCH_INFO *Switch_Info;

extern MASTER_LEVEL_VALUE	Master_Level_Data;
#endif
