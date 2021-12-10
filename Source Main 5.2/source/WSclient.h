#ifndef __SOCKETCLIENT_H__
#define __SOCKETCLIENT_H__

#pragma once

#include "wsctlc.h"
#include "SimpleModulus.h"
#include "CSMapServer.h"

#define WM_ASYNCSELECTMSG (WM_USER+0)

#define MAX_CHAT_SIZE 90
#define SIZE_PROTOCOLVERSION	( 5)
#define SIZE_PROTOCOLSERIAL		( 16)
#define MAX_GUILDNAME			8

#define	REQUEST_JOIN_SERVER      		        0
#define RECEIVE_JOIN_SERVER_WAITING				1
#define	RECEIVE_JOIN_SERVER_SUCCESS		        2
#define	RECEIVE_JOIN_SERVER_FAIL_VERSION        3
#define	REQUEST_CREATE_ACCOUNT			        10
#define	RECEIVE_CREATE_ACCOUNT_SUCCESS		    11
#define	RECEIVE_CREATE_ACCOUNT_FAIL_ID          12
#define	RECEIVE_CREATE_ACCOUNT_FAIL_RESIDENT    13

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
#define RECEIVE_LOG_IN_FAIL_DATE				33
#define RECEIVE_LOG_IN_FAIL_POINT_DATE			34
#define RECEIVE_LOG_IN_FAIL_POINT_HOUR			35
#define RECEIVE_LOG_IN_FAIL_INVALID_IP			36
#define RECEIVE_LOG_IN_FAIL_NO_PAYMENT_INFO		37
#define RECEIVE_LOG_IN_FAIL_ONLY_OVER_15		38
#define RECEIVE_LOG_IN_FAIL_CHARGED_CHANNEL		39

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
#define	RECEIVE_CONFIRM_PASSWORD_SUCCESS        62
#define	RECEIVE_CONFIRM_PASSWORD_FAIL_ID        63
#define	RECEIVE_CONFIRM_PASSWORD2_SUCCESS       64
#define	RECEIVE_CONFIRM_PASSWORD2_FAIL_ID       65
#define	RECEIVE_CONFIRM_PASSWORD2_FAIL_ANSWER   66
#define	RECEIVE_CONFIRM_PASSWORD2_FAIL_RESIDENT 67
#define RECEIVE_CHANGE_PASSWORD_SUCCESS         68
#define RECEIVE_CHANGE_PASSWORD_FAIL_ID         69
#define RECEIVE_CHANGE_PASSWORD_FAIL_RESIDENT   70
#define RECEIVE_CHANGE_PASSWORD_FAIL_PASSWORD   71

#define PACKET_ITEM_LENGTH  12

#define EQUIPMENT_LENGTH    17
#define MAX_SPE_BUFFERSIZE_	( 2048)

extern int CurrentProtocolState;

typedef struct 
{
	BYTE Code;
	BYTE Size;
	BYTE HeadCode;
} PBMSG_HEADER, * LPPBMSG_HEADER;

typedef struct 
{
	BYTE Code;
	BYTE SizeH;
	BYTE SizeL;
	BYTE HeadCode;
} PWMSG_HEADER, * LPPWMSG_HEADER;

typedef struct 
{
	BYTE Code;
	BYTE Size;
	BYTE byBuffer[255];
} PBMSG_ENCRYPTED, * LPPBMSG_ENCRYPTED;

typedef struct 
{
	BYTE Code;
	BYTE SizeH;
	BYTE SizeL;
	BYTE byBuffer[MAX_SPE_BUFFERSIZE_];
} PWMSG_ENCRYPTED, * LPWBMSG_ENCRYPTED;

//request default SubCode
typedef struct 
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
} PREQUEST_DEFAULT_SUBCODE, * LPPREQUEST_DEFAULT_SUBCODE;

//receive default
typedef struct 
{
	PBMSG_HEADER Header;
	BYTE         Value;
} PHEADER_DEFAULT, * LPPHEADER_DEFAULT;

typedef struct 
{
	PBMSG_HEADER	Header;
	BYTE			result;
	WORD			btStatValue;
	BYTE			btFruitType;
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
#ifndef NEW_PROTOCOL_SYSTEM
	PBMSG_HEADER Header;
	BYTE         SubCode;
#endif
    BYTE         MaxClass;
	BYTE		 MoveCount;
	BYTE         Value;
} PHEADER_DEFAULT_CHARACTER_LIST, * LPPHEADER_DEFAULT_CHARACTER_LIST;

#define CLASS_SUMMONER_CARD		0x01
#define CLASS_DARK_LORD_CARD	0x02
#define CLASS_DARK_CARD			0x04
#define CLASS_CHARACTERCARD_TOTALCNT	3

typedef struct 
{
	PBMSG_HEADER	header;
	BYTE			Flag;
	BYTE			CharacterCard;
} PHEADER_CHARACTERCARD, *LPPHEADER_CHARACTERCARD;

typedef struct 
{
	bool bCharacterEnable[CLASS_CHARACTERCARD_TOTALCNT];
}CHARACTER_ENABLE, *LPCHARACTER_ENABLE;

extern CHARACTER_ENABLE g_CharCardEnable;

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

typedef struct {
    PBMSG_HEADER Header;
    BYTE         Value;
    BYTE         KeyH;
    BYTE         KeyM;
    BYTE         KeyL;
} PHEADER_MATCH_OPEN_VALUE, * LPPHEADER_MATCH_OPEN_VALUE;

typedef struct
{
	PBMSG_HEADER Header;
	BYTE		State;
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

typedef struct 
{
	WORD         Index;
	BYTE         Percent;
} PRECEIVE_SERVER_LIST, * LPPRECEIVE_SERVER_LIST;

//receive join server
typedef struct 
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         Result;
	BYTE         NumberH;
	BYTE         NumberL;
	BYTE         Version[SIZE_PROTOCOLVERSION];
} PRECEIVE_JOIN_SERVER, * LPPRECEIVE_JOIN_SERVER;

//request create acoount
typedef struct 
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         ID[MAX_ID_SIZE];
	BYTE         Password[MAX_ID_SIZE];
	BYTE         Name[10];
	BYTE         ResidentNumber[13];
	BYTE         Quiz[30];
	BYTE         Answer[30];
	BYTE         PhoneNumber[20];
	BYTE         Email[50];
} PREQUEST_CREATE_ACCOUNT, * LPPREQUEST_CREATE_ACCOUNT;

//request log in
typedef struct 
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         ID[MAX_ID_SIZE];
	BYTE         Password[MAX_ID_SIZE];
	DWORD        Time;
	BYTE         Version[SIZE_PROTOCOLVERSION];
	BYTE         Serial[SIZE_PROTOCOLSERIAL];
} PREQUEST_LOG_IN, * LPPREQUEST_LOG_IN;

//request confirm password
typedef struct 
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         ID[MAX_ID_SIZE];
} PREQUEST_CONFIRM_PASSWORD, * LPPREQUEST_CONFIRM_PASSWORD;

//receive confirm password
typedef struct 
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         Result;
	BYTE         Question[30];
} PRECEIVE_CONFIRM_PASSWORD, * LPPRECEIVE_CONFIRM_PASSWORD;

//request confirm password
typedef struct 
{
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
typedef struct 
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         ID[MAX_ID_SIZE];
	BYTE         OldPassword[MAX_ID_SIZE];
	BYTE         NewPassword[MAX_ID_SIZE];
	BYTE         ResidentNumber[13];
} PREQUEST_CHANGE_PASSWORD, * LPPREQUEST_CHANGE_PASSWORD;

//receive characters list
typedef struct 
{
	BYTE         Index;
	BYTE         ID[MAX_ID_SIZE];
	WORD         Level;
	BYTE		 CtlCode;
	BYTE         Class;
	BYTE         Equipment[EQUIPMENT_LENGTH];
    BYTE         byGuildStatus;
} PRECEIVE_CHARACTER_LIST, * LPPRECEIVE_CHARACTER_LIST;

//request create character
typedef struct 
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         ID[MAX_ID_SIZE];
	BYTE         Class;
} PREQUEST_CREATE_CHARACTER, * LPREQUEST_CREATE_CHARACTER;

//receive create character
typedef struct 
{
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
typedef struct 
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         ID[MAX_ID_SIZE];
	BYTE         Resident[10];
} PREQUEST_DELETE_CHARACTER, * LPREQUEST_DELETE_CHARACTER;

//request join map server
typedef struct 
{
	PBMSG_HEADER Header;
	BYTE		 SubCode;
	BYTE         ID[MAX_ID_SIZE];
} PREQUEST_JOIN_MAP_SERVER, * LPPREQUEST_JOIN_MAP_SERVER;

//receive join map server
typedef struct 
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE         PositionX;
	BYTE         PositionY;
	BYTE         Map;
	BYTE         Angle;
	BYTE		btMExp1;
	BYTE		btMExp2;
	BYTE		btMExp3;
	BYTE		btMExp4;
	BYTE		btMExp5;
	BYTE		btMExp6;
	BYTE		btMExp7;
	BYTE		btMExp8;
	BYTE		btMNextExp1;
	BYTE		btMNextExp2;
	BYTE		btMNextExp3;
	BYTE		btMNextExp4;
	BYTE		btMNextExp5;
	BYTE		btMNextExp6;
	BYTE		btMNextExp7;
	BYTE		btMNextExp8;
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
    WORD         Charisma;
	WORD		 wMinusPoint;
	WORD		 wMaxMinusPoint;
} PRECEIVE_JOIN_MAP_SERVER, * LPPRECEIVE_JOIN_MAP_SERVER;

//receive revival
typedef struct 
{
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
	BYTE		btMExp1;
	BYTE		btMExp2;
	BYTE		btMExp3;
	BYTE		btMExp4;
	BYTE		btMExp5;
	BYTE		btMExp6;
	BYTE		btMExp7;
	BYTE		btMExp8;

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
#ifndef NEW_PROTOCOL_SYSTEM
	PBMSG_HEADER  Header;
#endif
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

typedef struct {
	PBMSG_HEADER  Header;
	BYTE          KeyH;
	BYTE          KeyL;
	BYTE          Action;
	BYTE          Dir;
} PREQUEST_ATTACK, * LPPREQUEST_ATTACK;

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
    BYTE         ListType;
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
	BYTE		  m_byType;
	BYTE		  m_byCount;
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
	WORD         MaxLife;
	WORD         MaxMana;
	WORD		 MaxShield;
	WORD		 SkillManaMax;
    short        AddPoint;
    short        MaxAddPoint;
	WORD		 wMinusPoint;
	WORD		 wMaxMinusPoint;
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
	BYTE		 GuildStatus;
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

typedef struct
{
	int			 GuildKey;
	BYTE		 GuildStatus;
	BYTE		 GuildType;
	BYTE		 GuildRelationShip;
	BYTE		 KeyH;
	BYTE		 KeyL;
} PRECEIVE_GUILD_ID, *LPPRECEIVE_GUILD_ID;

typedef struct
{
	PBMSG_HEADER Header;
	int			GuildKey;
	BYTE		GuildType;
	BYTE		UnionName[MAX_GUILDNAME];
	BYTE		GuildName[MAX_GUILDNAME];
	BYTE		Mark[32];
} PPRECEIVE_GUILDINFO, *LPPPRECEIVE_GUILDINFO;

enum GUILD_REQ_COMMON_RESULT
{
	GUILD_ANS_NOTEXIST_GUILD			= 0x10,
	GUILD_ANS_UNIONFAIL_BY_CASTLE		= 0x10,
	GUILD_ANS_NOTEXIST_PERMISSION		= 0x11,
	GUILD_ANS_NOTEXIST_EXTRA_STATUS		= 0x12,
	GUILD_ANS_NOTEXIST_EXTRA_TYPE		= 0x13,
	GUILD_ANS_EXIST_RELATIONSHIP_UNION	= 0x15,
	GUILD_ANS_EXIST_RELATIONSHIP_RIVAL	= 0x16,
	GUILD_ANS_EXIST_UNION				= 0x17,
	GUILD_ANS_EXIST_RIVAL				= 0x18,
	GUILD_ANS_NOTEXIST_UNION			= 0x19,
	GUILD_ANS_NOTEXIST_RIVAL			= 0x1A,
	GUILD_ANS_NOT_UNION_MASTER			= 0x1B,
	GUILD_ANS_NOT_GUILD_RIVAL			= 0x1C,
	GUILD_ANS_CANNOT_BE_UNION_MASTER_GUILD = 0x1D,
	GUILD_ANS_EXCEED_MAX_UNION_MEMBER	= 0x1E,
	GUILD_ANS_CANCEL_REQUEST			= 0x20,
	GUILD_ANS_UNION_MASTER_NOT_GENS		= 0xA1,
	GUILD_ANS_GUILD_MASTER_NOT_GENS		= 0xA2,
	GUILD_ANS_UNION_MASTER_DISAGREE_GENS = 0xA3,
};

typedef struct 
{
	PBMSG_HEADER	Header;
	BYTE			Value;
	BYTE			GuildType;
} PMSG_GUILD_CREATE_RESULT, *LPPMSG_GUILD_CREATE_RESULT;

typedef struct
{
	PBMSG_HEADER	Header;
	BYTE			byGuildType;
	BYTE			byResult;
} PMSG_GUILD_ASSIGN_TYPE_RESULT, *LPPMSG_GUILD_ASSIGN_TYPE_RESULT;

typedef struct 
{
	PBMSG_HEADER	Header;
	BYTE			byType;
	BYTE			byResult;
	char			szTargetName[MAX_ID_SIZE];
} PRECEIVE_GUILD_ASSIGN, *LPPRECEIVE_GUILD_ASSIGN;

typedef struct 
{
	PWMSG_HEADER	Header;
	BYTE			byCount;
	BYTE			byResult;
	BYTE			byRivalCount;
	BYTE			byUnionCount;
} PMSG_UNIONLIST_COUNT, *LPPMSG_UNIONLIST_COUNT;

typedef struct 
{
	BYTE			byMemberCount;
	BYTE			GuildMark[32];
	char			szGuildName[MAX_GUILDNAME];
} PMSG_UNIONLIST, *LPPMSG_UNIONLIST;

typedef struct 
{
	PBMSG_HEADER	Header;
	BYTE			byRelationShipType;
	BYTE			byRequestType;
	BYTE			byTargetUserIndexH;
	BYTE			byTargetUserIndexL;
} PMSG_GUILD_RELATIONSHIP, *LPPMSG_GUILD_RELATIONSHIP;

typedef struct 
{
	PBMSG_HEADER	Header;
	BYTE			byRelationShipType;
	BYTE			byRequestType;
	BYTE			byResult;
	BYTE			byTargetUserIndexH;
	BYTE			byTargetUserIndexL;
} PMSG_GUILD_RELATIONSHIP_RESULT, *LPPMSG_GUILD_RELATIONSHIP_RESULT;

typedef struct 
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE			byResult;
	BYTE			byRequestType;
	BYTE			byRelationShipType;
} PMSG_BAN_UNIONGUILD, *LPPMSG_BAN_UNIONGUILD;

typedef struct 
{
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
typedef struct 
{
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

#pragma pack(push, 1)
typedef struct 
{
    PBMSG_HEADER Header;
    BYTE         SubCode;
	BYTE         HotKey[20];
    BYTE         GameOption;
    BYTE         KeyQWE[3];
	BYTE		 ChatLogBox;
	BYTE		 KeyR;
	int			 QWERLevel;
} PRECEIVE_OPTION, * LPPRECEIVE_OPTION;
#pragma pack(pop)

typedef struct 
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		 Cmd1;
	BYTE		 Cmd2;
	BYTE		 Cmd3;
} PRECEIVE_SERVER_COMMAND, * LPPRECEIVE_SERVER_COMMAND;


typedef struct 
{
	PBMSG_HEADER Header;
	BYTE		 m_MyRank;
	BYTE         m_Count;
	BYTE		 m_byRank;
} PDEVILRANK, * LPPDEVILRANK;

typedef struct 
{
    PBMSG_HEADER Header;
    BYTE        m_byType;
	int			m_nChipCount;
    short       m_shMutoNum[3];
} PRECEIVE_EVENT_CHIP_INFO, * LPPRECEIVE_EVENT_CHIP_INFO;

typedef struct 
{
    PBMSG_HEADER Header;
    BYTE        m_byType;
	unsigned int  m_unChipCount;
} PRECEIVE_ENVET_CHIP, * LPPRECEIVE_EVENT_CHIP;

typedef struct 
{
    PBMSG_HEADER Header;
    short        m_shMutoNum[3];
} PRECEIVE_MUTONUMBER, * LPPRECEIVE_MUTONUMBER;

typedef struct 
{
	PBMSG_HEADER Header;
	BYTE		 m_byIsRegistered;
	char		 m_strGiftName[64];
} PRECEIVE_SCRATCH_TICKET_EVENT, *LPPRECEIVE_SCRATCH_TICKET_EVENT;

typedef struct {
	PBMSG_HEADER Header;
	WORD		 wEffectNum;
} PRECEIVE_PLAY_SOUND_EFFECT, *LPPRECEIVE_PLAY_SOUND_EFFECT;

typedef struct {
	PBMSG_HEADER Header;
	BYTE         m_byValue;
    BYTE         m_byNumber;
} PHEADER_EVENT, * LPPHEADER_EVENT;

typedef struct {
    PBMSG_HEADER    Header;
    BYTE            m_wEventType;
    BYTE            m_wLeftEnterCount;
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

// GC[0xF6][0x0A]
typedef struct 
{	
	PBMSG_HEADER	Header;
	BYTE			SubCode;

	WORD			m_wNPCIndex;
	WORD			m_wQuestCount;
} PMSG_NPCTALK_QUESTLIST, *LPPMSG_NPCTALK_QUESTLIST;

#pragma pack(push, 1)
typedef struct 
{	
	PBMSG_HEADER	Header;
	BYTE			SubCode;
	DWORD			m_dwQuestIndex;
	BYTE			m_byRequestCount;
	BYTE			m_byRewardCount;
	BYTE			m_byRandRewardCount;
} PMSG_NPC_QUESTEXP_INFO, *LPPMSG_NPC_QUESTEXP_INFO;
#pragma pack(pop)

enum QUEST_REQUEST_TYPE
{
	QUEST_REQUEST_NONE		= 0x0000,
	QUEST_REQUEST_MONSTER	= 0x0001,
	QUEST_REQUEST_SKILL		= 0x0002,
	QUEST_REQUEST_ITEM		= 0x0004,
	QUEST_REQUEST_LEVEL		= 0x0008,
	QUEST_REQUEST_TUTORIAL	= 0x0010,
	QUEST_REQUEST_BUFF		= 0x0020,
	QUEST_REQUEST_EVENT_MAP_USER_KILL		= 0x0040,
	QUEST_REQUEST_EVENT_MAP_MON_KILL		= 0x0041,
	QUEST_REQUEST_EVENT_MAP_BLOOD_GATE		= 0x0042,
	QUEST_REQUEST_EVENT_MAP_CLEAR_BLOOD		= 0x0100,
	QUEST_REQUEST_EVENT_MAP_CLEAR_CHAOS		= 0x0101,
	QUEST_REQUEST_EVENT_MAP_CLEAR_DEVIL		= 0x0102,
	QUEST_REQUEST_EVENT_MAP_CLEAR_ILLUSION	= 0x0103,
	QUEST_REQUEST_EVENT_MAP_DEVIL_POINT		= 0x0104,
	QUEST_REQUEST_ZEN		= 0x0105,
	QUEST_REQUEST_PVP_POINT	= 0x0106,
	QUEST_REQUEST_NPC_TALK	= 0x0107,
	QUEST_REQUEST_TYPE_MAX
};

#pragma pack(push, 1)
typedef struct 
{	
	DWORD	m_dwType;
	WORD	m_wIndex;
	DWORD	m_dwValue;
	WORD	m_wCurValue;
	BYTE	m_byItemInfo[PACKET_ITEM_LENGTH];
} NPC_QUESTEXP_REQUEST_INFO, *LPNPC_QUESTEXP_REQUEST_INFO;
#pragma pack(pop)

// 보상
enum QUEST_REWARD_TYPE
{
	QUEST_REWARD_NONE		= 0x0000,
	QUEST_REWARD_EXP		= 0x0001,
	QUEST_REWARD_ZEN		= 0x0002,
	QUEST_REWARD_ITEM		= 0x0004,
	QUEST_REWARD_BUFF		= 0x0008,
	QUEST_REWARD_CONTRIBUTE	= 0x0010,
	QUEST_REWARD_RANDOM		= 0x0020,
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

typedef struct 
{	
	NPC_QUESTEXP_REQUEST_INFO	NpcQuestRequestInfo[5];
	NPC_QUESTEXP_REWARD_INFO	NpcQuestRewardInfo[5];
} NPC_QUESTEXP_INFO, *LPNPC_QUESTEXP_INFO;

//----------------------------------------------------------------------------
// GC[0xF6][0x0D]
//----------------------------------------------------------------------------
typedef struct 
{	
	PBMSG_HEADER	Header;
	BYTE			SubCode;

	DWORD			m_dwQuestIndex;
	BYTE			m_byResult;
} PMSG_ANS_QUESTEXP_COMPLETE, *LPPMSG_ANS_QUESTEXP_COMPLETE;

//----------------------------------------------------------------------------
// GC[0xF6][0x0F]
//----------------------------------------------------------------------------
typedef struct 
{	
	PBMSG_HEADER	Header;
	BYTE			SubCode;
	
	DWORD			m_dwQuestGiveUpIndex;
} PMSG_ANS_QUESTEXP_GIVEUP, *LPPMSG_ANS_QUESTEXP_GIVEUP;

//----------------------------------------------------------------------------
// GC[0xF6][0x1A]
//----------------------------------------------------------------------------
typedef struct 
{	
	PBMSG_HEADER	Header;
	BYTE			SubCode;

	BYTE			m_byQuestCount;
} PMSG_ANS_QUESTEXP_PROGRESS_LIST, *LPPMSG_ANS_QUESTEXP_PROGRESS_LIST;

//----------------------------------------------------------------------------
// GC[0xF8][0x02]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER	Header;
	BYTE			SubCode;

	BYTE			m_byResult;
	BYTE			m_byInfluence;
} PMSG_ANS_REG_GENS_MEMBER, *LPPMSG_ANS_REG_GENS_MEMBER;

//----------------------------------------------------------------------------
// GC[0xF8][0x04]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER	Header;
	BYTE			SubCode;
	
	BYTE			m_byResult;
	
} PMSG_ANS_SECEDE_GENS_MEMBER, *LPPMSG_ANS_SECEDE_GENS_MEMBER;

//----------------------------------------------------------------------------
// GC[0xF8][0x07]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER	Header;
	BYTE			SubCode;
	
	BYTE			m_byInfluence;
	int				m_nRanking;
	int				m_nGensClass;
	int				m_nContributionPoint;
	int				m_nNextContributionPoint;
} PMSG_MSG_SEND_GENS_INFO, *LPPMSG_MSG_SEND_GENS_INFO;

//----------------------------------------------------------------------------
// GC[0xF8][0x05]
//----------------------------------------------------------------------------
typedef struct
{
	PWMSG_HEADER    Header;
	BYTE			SubCode;

	BYTE			m_byCount;	
} PMSG_SEND_GENS_MEMBER_VIEWPORT, *LPPMSG_SEND_GENS_MEMBER_VIEWPORT;

typedef struct
{
	BYTE			m_byInfluence;
	BYTE			m_byNumberH;
	BYTE			m_byNumberL;
	int				m_nRanking;
	int				m_nGensClass;
	int				m_nContributionPoint;
} PMSG_GENS_MEMBER_VIEWPORT_INFO, *LPPMSG_GENS_MEMBER_VIEWPORT_INFO;
#endif	// ASG_ADD_GENS_SYSTEM

//----------------------------------------------------------------------------
// GC[0xF8][0x08]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER	Header;
	BYTE			SubCode;
	BYTE			m_byRewardResult;
}PMSG_GENS_REWARD_CODE, *LPPMSG_GENS_REWARD_CODE;

//----------------------------------------------------------------------------
// GC[0xF9][0x01]
//----------------------------------------------------------------------------
typedef struct 
{	
	PBMSG_HEADER	Header;
	BYTE			SubCode;
	
	WORD			m_wNPCIndex;
	DWORD			m_dwContributePoint;
} PMSG_ANS_NPC_CLICK, *LPPMSG_ANS_NPC_CLICK;

typedef struct {
    BYTE        m_byX;
    BYTE        m_byY;
}PRECEIVE_MAP_ATTRIBUTE, *LPPRECEIVE_MAP_ATTRIBUTE;

typedef struct {
    PBMSG_HEADER    Header;
    BYTE            m_byType;
    BYTE            m_byMapAttr;
    BYTE            m_byMapSetType;
    BYTE            m_byCount;

    PRECEIVE_MAP_ATTRIBUTE  m_vAttribute[128*128];
}PRECEIVE_SET_MAPATTRIBUTE, *LPPRECEIVE_SET_MAPATTRIBUTE;

typedef struct  {
    PBMSG_HEADER    Header;
    BYTE            m_byPlayState;
    WORD            m_wRemainSec;
    WORD            m_wMaxKillMonster; 
    WORD            m_wCurKillMonster;
    WORD            m_wIndex;
    BYTE            m_byItemType;
}PRECEIVE_MATCH_GAME_STATE, *LPPRECEIVE_MATCH_GAME_STATE;

typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		nResult;
	BYTE		bIndexH;
	BYTE		bIndexL;
	CHAR		szID[MAX_ID_SIZE];
} PMSG_ANS_DUEL_INVITE, *LPPMSG_ANS_DUEL_INVITE;

typedef struct _tagPMSG_REQ_DUEL_ANSWER		// SC2
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		bIndexH;
	BYTE		bIndexL;
	CHAR		szID[MAX_ID_SIZE];
} PMSG_REQ_DUEL_ANSWER, *LPPMSG_REQ_DUEL_ANSWER;

typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		nResult;
	BYTE		bIndexH;
	BYTE		bIndexL;
	CHAR		szID[MAX_ID_SIZE];
} PMSG_ANS_DUEL_EXIT, *LPPMSG_ANS_DUEL_EXIT;

typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		bIndexH1;
	BYTE		bIndexL1;
	BYTE		bIndexH2;
	BYTE		bIndexL2;
	BYTE		btDuelScore1;
	BYTE		btDuelScore2;
} PMSG_DUEL_SCORE_BROADCAST, *LPPMSG_DUEL_SCORE_BROADCAST;

typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		bIndexH1;
	BYTE		bIndexL1;
	BYTE		bIndexH2;
	BYTE		bIndexL2;
	BYTE		btHP1;
	BYTE		btHP2;
	BYTE		btShield1;
	BYTE		btShield2;
} PMSG_DUEL_HP_BROADCAST, *LPPMSG_DUEL_HP_BROADCAST;

typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	struct {
		CHAR	szID1[MAX_ID_SIZE];
		CHAR	szID2[MAX_ID_SIZE];
		BYTE	bStart;
		BYTE	bWatch;
	} channel[4];
} PMSG_ANS_DUEL_CHANNELLIST, *LPPMSG_ANS_DUEL_CHANNELLIST;

typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		nResult;
	BYTE		nChannelId;
	CHAR		szID1[MAX_ID_SIZE];
	CHAR		szID2[MAX_ID_SIZE];
	BYTE		bIndexH1;
	BYTE		bIndexL1;
	BYTE		bIndexH2;
	BYTE		bIndexL2;
} PMSG_ANS_DUEL_JOINCNANNEL, *LPPMSG_ANS_DUEL_JOINCNANNEL;

typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	CHAR		szID[MAX_ID_SIZE];
} PMSG_DUEL_JOINCNANNEL_BROADCAST, *LPPMSG_DUEL_JOINCNANNEL_BROADCAST;

typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		nResult;
} PMSG_ANS_DUEL_LEAVECNANNEL, *LPPMSG_ANS_DUEL_LEAVECNANNEL;

typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	CHAR		szID[MAX_ID_SIZE];
} PMSG_DUEL_LEAVECNANNEL_BROADCAST, *LPPMSG_DUEL_LEAVECNANNEL_BROADCAST;

typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		nCount;
	struct {
		CHAR	szID[MAX_ID_SIZE];
	} user[10];
} PMSG_DUEL_OBSERVERLIST_BROADCAST, *LPPMSG_DUEL_OBSERVERLIST_BROADCAST;

typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	CHAR		szWinner[MAX_ID_SIZE];
	CHAR		szLoser[MAX_ID_SIZE];
} PMSG_DUEL_RESULT_BROADCAST, *LPPMSG_DUEL_RESULT_BROADCAST;

typedef struct
{
	PBMSG_HEADER Header;
	BYTE         SubCode;
	BYTE		nFlag;
} PMSG_DUEL_ROUNDSTART_BROADCAST, *LPPMSG_DUEL_ROUNDSTART_BROADCAST;

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
	BYTE			byType;
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
	BYTE			Subject[MAX_LETTER_TITLE_LENGTH];
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
	BYTE			Memo[MAX_LETTERTEXT_LENGTH];
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
// GC [0x2D]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER	h;

	WORD		wOptionType;
	WORD		wEffectType;
	BYTE		byEffectOption;
	int			wEffectTime;
	BYTE		byBuffType;
} PMSG_ITEMEFFECTCANCEL, *LPPMSG_ITEMEFFECTCANCEL;

typedef struct
{
    PBMSG_HEADER    m_Header;
    BYTE            m_byPetType;
    BYTE            m_byCommand;
    BYTE            m_byKeyH;
    BYTE            m_byKeyL;
}PRECEIVE_PET_COMMAND, *LPPRECEIVE_PET_COMMAND;


typedef struct 
{
    PBMSG_HEADER    m_Header;
    BYTE            m_byPetType;
    BYTE            m_bySkillType; 
    BYTE            m_byKeyH;
    BYTE            m_byKeyL;
    BYTE            m_byTKeyH;
    BYTE            m_byTKeyL;
}PRECEIVE_PET_ATTACK, *LPPRECEIVE_PET_ATTACK;


typedef struct 
{
    PBMSG_HEADER    m_Header;
    BYTE            m_byPetType;
    BYTE            m_byInvType;
    BYTE            m_byPos;
    BYTE            m_byLevel;
    int             m_iExp;
	BYTE			m_byLife;
}PRECEIVE_PET_INFO, *LPPRECEIVE_PET_INFO;

typedef struct
{
	PBMSG_HEADER    m_Header;
	BYTE			m_subCode;
	BYTE			m_Type;	
	WORD			m_Time;
}PMSG_MATCH_TIMEVIEW, *LPPMSG_MATCH_TIMEVIEW;

typedef struct
{
	void			Clear()
	{ 
		memset(&m_MatchTeamName1, NULL, MAX_ID_SIZE); 
		memset(&m_MatchTeamName2, NULL, MAX_ID_SIZE);
		m_Score1 = 0;
		m_Score2 = 0;
		m_Type = 0;
	}

	PBMSG_HEADER	m_Header;
	BYTE			m_subCode;
	BYTE			m_Type;

	char			m_MatchTeamName1[MAX_ID_SIZE];
	WORD			m_Score1;

	char			m_MatchTeamName2[MAX_ID_SIZE];
	WORD			m_Score2;

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

typedef struct
{
    PBMSG_HEADER    m_Header;
    MServerInfo     m_vSvrInfo;
}PHEADER_MAP_CHANGESERVER_INFO, *LPPHEADER_MAP_CHANGESERVER_INFO;


enum CASTLESIEGE_STATE
{
	CASTLESIEGE_STATE_NONE					= -1,
	CASTLESIEGE_STATE_IDLE_1				= 0,
	CASTLESIEGE_STATE_REGSIEGE				= 1,
	CASTLESIEGE_STATE_IDLE_2				= 2,
	CASTLESIEGE_STATE_REGMARK				= 3,
	CASTLESIEGE_STATE_IDLE_3				= 4,
	CASTLESIEGE_STATE_NOTIFY				= 5,
	CASTLESIEGE_STATE_READYSIEGE			= 6,
	CASTLESIEGE_STATE_STARTSIEGE			= 7,
	CASTLESIEGE_STATE_ENDSIEGE				= 8,
	CASTLESIEGE_STATE_ENDCYCLE				= 9,
};

//----------------------------------------------------------------------------
// GC [0xB2][0x00]
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;
	CHAR		cCastleSiegeState;
	BYTE		btStartYearH;
	BYTE		btStartYearL;
	BYTE		btStartMonth;
	BYTE		btStartDay;
	BYTE		btStartHour;
	BYTE		btStartMinute;
	BYTE		btEndYearH;
	BYTE		btEndYearL;
	BYTE		btEndMonth;
	BYTE		btEndDay;
	BYTE		btEndHour;
	BYTE		btEndMinute;
	BYTE		btSiegeStartYearH;
	BYTE		btSiegeStartYearL;
	BYTE		btSiegeStartMonth;
	BYTE		btSiegeStartDay;
	BYTE		btSiegeStartHour;
	BYTE		btSiegeStartMinute;
	CHAR		cOwnerGuild[8];
	CHAR		cOwnerGuildMaster[10];

	CHAR		btStateLeftSec1;
	CHAR		btStateLeftSec2;
	CHAR		btStateLeftSec3;
	CHAR		btStateLeftSec4;
} PMSG_ANS_CASTLESIEGESTATE, *LPPMSG_ANS_CASTLESIEGESTATE;

//----------------------------------------------------------------------------
// GC [0xB2][0x01]
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;
	CHAR		szGuildName[8];
} PMSG_ANS_REGCASTLESIEGE, *LPPMSG_ANS_REGCASTLESIEGE;

//----------------------------------------------------------------------------
// GC [0xB2][0x02]
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;
	BYTE		btIsGiveUp;
	CHAR		szGuildName[8];
} PMSG_ANS_GIVEUPCASTLESIEGE, *LPPMSG_ANS_GIVEUPCASTLESIEGE;

//----------------------------------------------------------------------------
// GC [0xB2][0x03]
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;
	CHAR		szGuildName[8];
	BYTE		btGuildMark1;
	BYTE		btGuildMark2;
	BYTE		btGuildMark3;
	BYTE		btGuildMark4;
	BYTE		btIsGiveUp;
	BYTE		btRegRank;
} PMSG_ANS_GUILDREGINFO, *LPPMSG_ANS_GUILDREGINFO;

//----------------------------------------------------------------------------
// GC [0xB2][0x04]
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;
	CHAR		szGuildName[8];	
	BYTE		btGuildMark1;
	BYTE		btGuildMark2;
	BYTE		btGuildMark3;
	BYTE		btGuildMark4;
} PMSG_ANS_REGGUILDMARK, *LPPMSG_ANS_REGGUILDMARK;

//----------------------------------------------------------------------------
// GC [0xB2][0x05]
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;
	INT			iNpcNumber;
	INT			iNpcIndex;
} PMSG_ANS_NPCBUY, *LPPMSG_ANS_NPCBUY;

//----------------------------------------------------------------------------
// GC [0xB2][0x06]
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;
	INT			iNpcNumber;
	INT			iNpcIndex;
	INT			iNpcHP;
	INT			iNpcMaxHP;
} PMSG_ANS_NPCREPAIR, *LPPMSG_ANS_NPCREPAIR;

//----------------------------------------------------------------------------
// GC [0xB2][0x07]
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;
	INT			iNpcNumber;
	INT			iNpcIndex;
	INT			iNpcUpType;
	INT			iNpcUpValue;
} PMSG_ANS_NPCUPGRADE, *LPPMSG_ANS_NPCUPGRADE;

//----------------------------------------------------------------------------
// CG [0xB2][0x08]
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;
	BYTE		btTaxRateChaos;	
	BYTE		btTaxRateStore;
	BYTE		btMoney1;
	BYTE		btMoney2;
	BYTE		btMoney3;
	BYTE		btMoney4;	
	BYTE		btMoney5;
	BYTE		btMoney6;
	BYTE		btMoney7;
	BYTE		btMoney8;
} PMSG_ANS_TAXMONEYINFO, *LPPMSG_ANS_TAXMONEYINFO;

//----------------------------------------------------------------------------
// CG [0xB2][0x09]
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;
	BYTE		btTaxType;
	BYTE		btTaxRate1;
	BYTE		btTaxRate2;
	BYTE		btTaxRate3;
	BYTE		btTaxRate4;
} PMSG_ANS_TAXRATECHANGE, *LPPMSG_ANS_TAXRATECHANGE;

//----------------------------------------------------------------------------
// CG [0xB2][0x10]
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btResult;
	BYTE		btMoney1;
	BYTE		btMoney2;
	BYTE		btMoney3;
	BYTE		btMoney4;
	BYTE		btMoney5;
	BYTE		btMoney6;
	BYTE		btMoney7;
	BYTE		btMoney8;
} PMSG_ANS_MONEYDRAWOUT, *LPPMSG_ANS_MONEYDRAWOUT;

//----------------------------------------------------------------------------
// CG [0xB2][0x1A]
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	Header;
	BYTE		btTaxType;
	BYTE		btTaxRate;
} PMSG_ANS_MAPSVRTAXINFO, *LPPMSG_ANS_MAPSVRTAXINFO;

//----------------------------------------------------------------------------
// GC [0xB3]
//----------------------------------------------------------------------------
typedef struct
{
    PREQUEST_DEFAULT_SUBCODE    m_Header;
	BYTE		btResult;
	INT			iCount;
} PMSG_ANS_NPCDBLIST, *LPPMSG_ANS_NPCDBLIST;
typedef struct
{
	INT			iNpcNumber;
	INT			iNpcIndex;
	INT			iNpcDfLevel;
	INT			iNpcRgLevel;
	INT			iNpcMaxHp;
	INT			iNpcHp;
	BYTE		btNpcX;
	BYTE		btNpcY;
	BYTE		btNpcLive;
} PMSG_NPCDBLIST, *LPPMSG_NPCDBLIST;

//----------------------------------------------------------------------------
// GC [0xB4]
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	h;
	BYTE		btResult;
	INT			iCount;
} PMSG_ANS_CSREGGUILDLIST, *LPPMSG_ANS_CSREGGUILDLIST;
typedef struct
{
	CHAR		szGuildName[8];
	BYTE		btRegMarks1;
	BYTE		btRegMarks2;
	BYTE		btRegMarks3;
	BYTE		btRegMarks4;
	BYTE		btIsGiveUp;
	BYTE		btSeqNum;
} PMSG_CSREGGUILDLIST, *LPPMSG_CSREGGUILDLIST;

//----------------------------------------------------------------------------
// GC [0xB5]
//----------------------------------------------------------------------------
typedef struct
{
	PREQUEST_DEFAULT_SUBCODE	h;
	BYTE		btResult;
	INT			iCount;
} PMSG_ANS_CSATTKGUILDLIST, *LPPMSG_ANS_CSATTKGUILDLIST;
typedef struct
{
	BYTE		btCsJoinSide;
	BYTE		btGuildInvolved;
	CHAR		szGuildName[8];
	INT			iGuildScore;
} PMSG_CSATTKGUILDLIST, *LPPMSG_CSATTKGUILDLIST;


typedef struct 
{
	PBMSG_HEADER    m_Header;
	BYTE            m_bySubCode;
}PBMSG_HEADER2;

//----------------------------------------------------------------------------
// GC [0xA4][0x00]
//----------------------------------------------------------------------------
typedef struct
{ 
	PBMSG_HEADER2	h;
	BYTE	byResult;		
	BYTE	byQuestIndex;
	int		anKillCountInfo[10];
} PMSG_ANS_QUEST_MONKILL_INFO, *LPPMSG_ANS_QUEST_MONKILL_INFO;

    typedef struct 
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byResult;
        BYTE                m_byHuntZoneEnter;
    }PMSG_CSHUNTZONEENTER, *LPPMSG_CSHUNTZONEENTER;

    typedef struct 
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byResult;
        BYTE                m_byEnable;
        INT                 m_iCurrPrice;
        INT                 m_iMaxPrice;
        INT                 m_iUnitPrice;
    }PRECEIVE_CASTLE_HUNTZONE_INFO, *LPPRECEIVE_CASTLE_HUNTZONE_INFO; 

    typedef struct 
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byResult;
    }PRECEVIE_CASTLE_HUNTZONE_RESULT, *LPPRECEVIE_CASTLE_HUNTZONE_RESULT;

    typedef struct 
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byGuildMark[32];
    }PRECEIVE_CASTLE_FLAG, *LPPRECEIVE_CASTLE_FLAG;

    typedef struct
    {
        PBMSG_HEADER2        m_Header;
        BYTE                m_byResult;
        BYTE                m_byKeyH; 
        BYTE                m_byKeyL;
    }PRECEIVE_GATE_STATE, *LPPRECEIVE_GATE_STATE;


    typedef struct
    {
        PBMSG_HEADER2        m_Header;
        BYTE                m_byResult;
        BYTE                m_byOperator;
        BYTE                m_byKeyH;
        BYTE                m_byKeyL;
    }PRECEIVE_GATE_OPERATOR, *LPPRECEIVE_GATE_OPERATOR;


    typedef struct
    {
        PBMSG_HEADER2        m_Header;
        BYTE                m_byOperator;
        BYTE                m_byKeyH;
        BYTE                m_byKeyL;
    }PRECEIVE_GATE_CURRENT_STATE, *LPPRECEIVE_GATE_CURRENT_STATE;


    typedef struct
    {
        PBMSG_HEADER2       m_Header;   
        BYTE                m_byIndexH;
        BYTE                m_byIndexL;
        BYTE                m_byKeyH;
        BYTE                m_byKeyL;
        BYTE                m_byState;
    }PRECEIVE_SWITCH_PROC, *LPPRECEIVE_SWITCH_PROC;


    typedef struct
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byCrownState;

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


    typedef struct
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byBasttleCastleState;
        BYTE                m_szGuildName[8];
    }PRECEIVE_BC_PROCESS, *LPPRECEIVE_BC_PROCESS;

    typedef struct
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byKeyH;
        BYTE                m_byKeyL;

        BYTE                m_byBuildTime;
    }PRECEIVE_MONSTER_BUILD_TIME, *LPPRECEIVE_MONSTER_BUILD_TIME;

    typedef struct
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byResult;

        BYTE                m_byWeaponType;

        BYTE                m_byKeyH;
        BYTE                m_byKeyL;
    }PRECEIVE_CATAPULT_STATE, *LPPRECEIVE_CATAPULT_STATE;

    typedef struct
    {
        PBMSG_HEADER2        m_Header;
        BYTE                m_byResult;
        BYTE                m_byKeyH;
        BYTE                m_byKeyL;           

        BYTE                m_byWeaponType;

        BYTE                m_byTargetX;
        BYTE                m_byTargetY;
    }PRECEIVE_WEAPON_FIRE, *LPPRECEIVE_WEAPON_FIRE;

    typedef struct
    {
        PBMSG_HEADER2        m_Header;
        BYTE                m_byWeaponType;

        BYTE                m_byTargetX;
        BYTE                m_byTargetY;
    }PRECEIVE_BOMBING_ALERT, *LPPRECEIVE_BOMBING_ALERT;

    typedef struct
    {
        PBMSG_HEADER2        m_Header;
        BYTE                m_byKeyH;
        BYTE                m_byKeyL;

        BYTE                m_byWeaponType;
    }PRECEIVE_BOMBING_TARGET, *LPPRECEIVE_BOMBING_TARGET;

    typedef struct
    {
        BYTE                m_byObjType;
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
        BYTE                m_byType;
        BYTE                m_byX;
        BYTE                m_byY;
    }PRECEIVE_NPC_LOCATION, *LPPRECEIVE_NPC_LOCATION;

    typedef struct
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byResult;
    }PRECEIVE_MAP_INFO_RESULT, *LPPRECEIVE_MAP_INFO_RESULT;

    typedef struct
    {
        PBMSG_HEADER2       m_Header;
        BYTE                m_byHour;
        BYTE                m_byMinute;
    }PRECEIVE_MATCH_TIMER, *LPPRECEIVE_MATCH_TIMER;
	
	typedef struct  
	{
		PBMSG_HEADER2		m_Header;	
		BYTE				m_iJewelType;
		BYTE				m_iJewelMix;
	}PMSG_REQ_JEWEL_MIX, *LPPMSG_REQ_JEWEL_MIX;

	typedef struct 
	{
		PBMSG_HEADER2		m_Header;
		BYTE				m_iResult;
	}PMSG_ANS_JEWEL_MIX, *LPPMSG_ANS_JEWEL_MIX;

	typedef struct  
	{
		PBMSG_HEADER2		m_Header;			
		BYTE				m_iJewelType;
		BYTE				m_iJewelLevel;
		BYTE				m_iJewelPos;
	}PMSG_REQ_JEWEL_UNMIX, *LPPMSG_REQ_JEWEL_UNMIX;

	typedef struct  
	{
		PBMSG_HEADER2		m_Header;
		
		int		iMasterLevelSkill;
	}PMSG_REQ_MASTERLEVEL_SKILL, *LPPMSG_REQ_MASTERLEVEL_SKILL;

	typedef struct 
	{
		PBMSG_HEADER2		m_Header;
		BYTE				m_iResult;
	}PMSG_ANS_JEWEL_UNMIX, *LPPMSG_ANS_JEWEL_UNMIX;

//--------------------------------------------------------------------------
// GC [0xBD][0x00]
//--------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;

	BYTE			btOccupationState;
	BYTE			btCrywolfState;

} PMSG_ANS_CRYWOLF_INFO, *LPPMSG_ANS_CRYWOLF_INFO;

//--------------------------------------------------------------------------
// GC [0xBD][0x02] 방어막, 제단 상태 정보
//--------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	INT				iCrywolfStatueHP;
	BYTE			btAltarState1;
	BYTE			btAltarState2;
	BYTE			btAltarState3;
	BYTE			btAltarState4;
	BYTE			btAltarState5;

} PMSG_ANS_CRYWOLF_STATE_ALTAR_INFO, *LPPMSG_ANS_CRYWOLF_STATE_ALTAR_INFO;

//--------------------------------------------------------------------------
// GC [0xBD][0x03]
//--------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE			bResult;
	BYTE			btAltarState;
	BYTE			btObjIndexH;
	BYTE			btObjIndexL;

} PMSG_ANS_CRYWOLF_ALTAR_CONTRACT, *LPPPMSG_ANS_CRYWOLF_ALTAR_CONTRACT;

//--------------------------------------------------------------------------
// GC [0xBD][0x04]
//--------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE			btHour;
	BYTE			btMinute;

} PMSG_ANS_CRYWOLF_LEFTTIME, *LPPPMSG_ANS_CRYWOLF_LEFTTIME;

//--------------------------------------------------------------------------
// GC [0xBD][0x0C]
//--------------------------------------------------------------------------
typedef struct
{
    PBMSG_HEADER2       h;

    BYTE                btObjClassH;
    BYTE                btObjClassL;           

	BYTE                btSourceX;
    BYTE                btSourceY;

    BYTE                btPointX;
    BYTE                btPointY;
} PMSG_NOTIFY_REGION_MONSTER_ATTACK, *LPPMSG_NOTIFY_REGION_MONSTER_ATTACK;

typedef struct
{
	PBMSG_HEADER2	h;
	int				btBossHP;
	BYTE			btMonster2;
	
} PMSG_ANS_CRYWOLF_BOSSMONSTER_INFO, *LPPMSG_ANS_CRYWOLF_BOSSMONSTER_INFO;


typedef struct 
{
	PBMSG_HEADER2	h;
	BYTE			btPlusChaosRate;
} PMSG_ANS_CRYWOLF_BENEFIT_PLUS_CHAOSRATE, *LPPMSG_ANS_CRYWOLF_BENEFIT_PLUS_CHAOSRATE;

typedef struct
{
	PBMSG_HEADER2	h;
	BYTE			btRank;			//0 : D    1 : C   2 : B   3 : A   4 : S
	int				iGettingExp;
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

//----------------------------------------------------------------------------
// CG [0xD1][0x00]
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;
	BYTE	btState;
	BYTE	btDetailState;
	BYTE	btEnter;
	BYTE	btUserCount;
	int		iRemainTime;
} PMSG_ANS_KANTURU_STATE_INFO, *LPPMSG_ANS_KANTURU_STATE_INFO;

//----------------------------------------------------------------------------
// GC [0xD1][0x01]
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;
	BYTE		btResult;
} PMSG_ANS_ENTER_KANTURU_BOSS_MAP, *LPPMSG_ANS_ENTER_KANTURU_BOSS_MAP;

//----------------------------------------------------------------------------
// GC [0xD1][0x02]
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;
	BYTE		btCurrentState;
	BYTE		btCurrentDetailState;
} PMSG_ANS_KANTURU_CURRENT_STATE, *LPPMSG_ANS_KANTURU_CURRENT_STATE;

//----------------------------------------------------------------------------
// GC [0xD1][0x03]
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;

	BYTE		btState;
	BYTE		btDetailState;
	
} PMSG_ANS_KANTURU_STATE_CHANGE, *LPPMSG_ANS_KANTURU_STATE_CHANGE;

//----------------------------------------------------------------------------
// GC [0xD1][0x04]
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;

	BYTE		btResult;
	
} PMSG_ANS_KANTURU_BATTLE_RESULT, *LPPMSG_ANS_KANTURU_BATTLE_RESULT;

//----------------------------------------------------------------------------
// GC [0xD1][0x05]
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;

	int			btTimeLimit;
	
} PMSG_ANS_KANTURU_BATTLE_SCENE_TIMELIMIT, *LPPMSG_ANS_KANTURU_BATTLE_SCENE_TIMELIMIT;

//----------------------------------------------------------------------------
// GC [0xD1][0x06]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;

	BYTE		btObjClassH;
	BYTE		btObjClassL;
	
	BYTE		btType;

} PMSG_NOTIFY_KANTURU_WIDE_AREA_ATTACK, *LPPMSG_NOTIFY_KANTURU_WIDE_AREA_ATTACK;

//----------------------------------------------------------------------------
// GC [0xD1][0x07]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;

	BYTE		bMonsterCount;
	BYTE		btUserCount;

} PMSG_NOTIFY_KANTURU_USER_MONSTER_COUNT, *LPPMSG_NOTIFY_KANTURU_USER_MONSTER_COUNT;


//----------------------------------------------------------------------------
// CG [0xBF][0x00]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE			btCursedTempleIdx;
	BYTE			iItemPos;
} PMSG_REQ_ENTER_CURSED_TEMPLE, *LPPMSG_REQ_ENTER_CURSED_TEMPLE;

//----------------------------------------------------------------------------
// GC [0xBF][0x00]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE			Result;
} PMSG_RESULT_ENTER_CURSED_TEMPLE, *LPPMSG_RESULT_ENTER_CURSED_TEMPLE;

//----------------------------------------------------------------------------
// GC [0xBF][0x01]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	WORD		wRemainSec;
	WORD		btUserIndex;
	BYTE		btX;
	BYTE		btY;
	BYTE		btAlliedPoint;
	BYTE		btIllusionPoint;
	BYTE		btMyTeam;

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
// CG [0xBF][0x02]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;

	BYTE          MagicH;
	BYTE          MagicL;
	WORD		wTargetObjIndex;
	BYTE		Dis;
} PMSG_CURSED_TEMPLE_USE_MAGIC, *LPPMSG_CURSED_TEMPLE_USE_MAGIC;

//----------------------------------------------------------------------------
// GC [0xBF][0x03]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		btUserCount[6];
} PMSG_CURSED_TEMPLE_USER_COUNT, *LPPMSG_CURSED_TEMPLE_USER_COUNT;

//----------------------------------------------------------------------------
// GC [0xBF][0x02]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE        MagicResult;
	BYTE          MagicH;
	BYTE          MagicL;
	WORD		wSourceObjIndex;
	WORD		wTargetObjIndex;
} PMSG_CURSED_TEMPLE_USE_MAGIC_RESULT, *LPPMSG_CURSED_TEMPLE_USE_MAGIC_RESULT;

//----------------------------------------------------------------------------
// GC [0xBF][0x04]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		btAlliedPoint;
	BYTE		btIllusionPoint;

	BYTE		btUserCount;
} PMSG_CURSED_TEMPLE_RESULT, *LPPMSG_CURSED_TEMPLE_RESULT;

typedef struct 
{
	char		GameId[MAX_ID_SIZE];
	BYTE		byMapNumber;
	BYTE		btTeam;
	BYTE		btClass;
	int			nAddExp;
} PMSG_CURSED_TEMPLE_USER_ADD_EXP, *LPPMSG_CURSED_TEMPLE_USER_ADD_EXP;


//----------------------------------------------------------------------------
// GC [0xBF][0x06]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
	BYTE		btSkillPoint;
} PMSG_CURSED_TEMPLE_SKILL_POINT, *LPPMSG_CURSED_TEMPLE_SKILL_POINT;

//----------------------------------------------------------------------------
// GC [0xBF][0x07]
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;

	BYTE          MagicH;
	BYTE          MagicL;

	WORD		wObjIndex;
} PMSG_CURSED_TEMPLE_SKILL_END, *LPPMSG_CURSED_TEMPLE_SKILL_END;

//----------------------------------------------------------------------------
// GC [0xBF][0x08]
//----------------------------------------------------------------------------
typedef struct  
{
	PBMSG_HEADER2 h;
	WORD		wUserIndex;
	char		Name[MAX_ID_SIZE];	
} PMSG_RELICS_GET_USER, *LPPMSG_RELICS_GET_USER;

//----------------------------------------------------------------------------
// GC [0xBF][0x09]
//----------------------------------------------------------------------------
typedef struct  
{
	PBMSG_HEADER2 h;
	BYTE		btTempleNumber;
	BYTE		btIllusionTempleState;	// 0: wait, 1: wait->ready, 2: ready->play, 3: play->end, 
} PMSG_ILLUSION_TEMPLE_STATE, *LPPMSG_ILLUSION_TEMPLE_STATE;

//----------------------------------------------------------------------------
// GC [0xBF][0x0a]
//----------------------------------------------------------------------------
typedef struct {
	PBMSG_HEADER2	Header;

	BYTE			MagicH;
	BYTE			MagicL;

	WORD			wUserIndex;
	BYTE			byCount;
} PRECEIVE_CHAIN_MAGIC, *LPPRECEIVE_CHAIN_MAGIC;

typedef struct 
{
	WORD			wTargetIndex;
} PRECEIVE_CHAIN_MAGIC_OBJECT, *LPPRECEIVE_CHAIN_MAGIC_OBJECT;

typedef struct 
{
	PBMSG_HEADER  h;	
	BYTE		subcode;	
	short		nMLevel;
	BYTE		btMExp1;
	BYTE		btMExp2;
	BYTE		btMExp3;
	BYTE		btMExp4;
	BYTE		btMExp5;
	BYTE		btMExp6;
	BYTE		btMExp7;
	BYTE		btMExp8;

	BYTE		btMNextExp1;
	BYTE		btMNextExp2;
	BYTE		btMNextExp3;
	BYTE		btMNextExp4;
	BYTE		btMNextExp5;
	BYTE		btMNextExp6;
	BYTE		btMNextExp7;
	BYTE		btMNextExp8;
	short		nMLPoint;
	WORD		wMaxLife;
	WORD		wMaxMana;
	WORD		wMaxShield;
	WORD		wMaxSkillMana;
} PMSG_MASTERLEVEL_INFO, *LPPMSG_MASTERLEVEL_INFO;

typedef struct 
{
	PBMSG_HEADER	h;
	BYTE		subcode;
	short		nMLevel;
	short		nAddMPoint;
	short       nMLevelUpMPoint;
//	short		nTotalMPoint;
	short		nMaxPoint;
	WORD		wMaxLife;
	WORD		wMaxMana;
	WORD		wMaxShield;
	WORD		wMaxBP;
} PMSG_MASTERLEVEL_UP, *LPPMSG_MASTERLEVEL_UP;

typedef struct  
{
	PBMSG_HEADER	h;
	BYTE		subcode;
	BYTE		btResult;
	short       nMLPoint;
	int			nSkillNum;
	int			nSkillLevel;
} PMSG_ANS_MASTERLEVEL_SKILL, *LPPMSG_ANS_MASTERLEVEL_SKILL;

//----------------------------------------------------------------------------
// GC [0xD1][0x10] 
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;
	
	BYTE			btState;
	BYTE			btDetailState;
	
	BYTE			btEnter;
	int				iRemainTime;
} PMSG_ANS_RAKLION_STATE_INFO, *LPPMSG_ANS_RAKLION_STATE_INFO;

//----------------------------------------------------------------------------
// GC [0xD1][0x11] 
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;
	
	BYTE			btCurrentState;
	BYTE			btCurrentDetailState;
} PMSG_ANS_RAKLION_CURRENT_STATE, *LPPMSG_ANS_RAKLION_CURRENT_STATE;

//----------------------------------------------------------------------------
// GC [0xD1][0x12]
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;	
	
	BYTE			btState;
	BYTE			btDetailState;
} PMSG_ANS_RAKLION_STATE_CHANGE, *LPPMSG_ANS_RAKLION_STATE_CHANGE;

//----------------------------------------------------------------------------
// GC [0xD1][0x13]
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;
	
	BYTE			btResult;
} PMSG_ANS_RAKLION_BATTLE_RESULT, *LPPMSG_ANS_RAKLION_BATTLE_RESULT;

//----------------------------------------------------------------------------
// GC [0xD1][0x14]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	
	BYTE			btObjClassH;
	BYTE			btObjClassL;
	
	BYTE			btType;
	
} PMSG_NOTIFY_RAKLION_WIDE_AREA_ATTACK, *LPPMSG_NOTIFY_RAKLION_WIDE_AREA_ATTACK;

//----------------------------------------------------------------------------
// GC [0xD1][0x15]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	
	BYTE			btMonsterCount;
	BYTE			btUserCount;
} PMSG_NOTIFY_RAKLION_USER_MONSTER_COUNT, *LPPMSG_NOTIFY_RAKLION_USER_MONSTER_COUNT;

//----------------------------------------------------------------------------
// CG[0xBF][0x0b]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
	int			nRegCoinCnt;
} PMSG_REQ_GET_COIN_COUNT, *LPPMSG_REQ_GET_COIN_COUNT;

//----------------------------------------------------------------------------
// GC[0xBF][0x0b]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
	int			nCoinCnt;
}PMSG_ANS_GET_COIN_COUNT, *LPPMSG_ANS_GET_COIN_COUNT;

//----------------------------------------------------------------------------
// CG[0xBF][0x0c]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
}PMSG_REQ_REGEIST_COIN, *LPPMSG_REQ_REGEIST_COIN;

//----------------------------------------------------------------------------
// GC0xBF][0x0c]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
	BYTE		btResult;
	int			nCurCoinCnt;
}PMSG_ANS_REGEIST_COIN, *LPPMSG_ANS_REGEIST_COIN;

//----------------------------------------------------------------------------
// CG[0xBF][0x0d]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
	int			nCoinCnt;
}PMSG_REQ_TRADE_COIN, *LPPMSG_REG_TREADE_COIN;

//----------------------------------------------------------------------------
// GC[0xBF][0x0d]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2 h;
	BYTE		btResult;
}PMSG_ANS_TRADE_COIN, *LPPMSG_ANS_TREADE_COIN;

//----------------------------------------------------------------------------
// GC [0xBF][0x0E]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		btResult;
} PMSG_RESULT_ENTER_DOPPELGANGER, *LPPMSG_RESULT_ENTER_DOPPELGANGER;
//----------------------------------------------------------------------------
// GC [0xBF][0x0F]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		btPosIndex;
} PMSG_DOPPELGANGER_MONSTER_POSITION, *LPPMSG_DOPPELGANGER_MONSTER_POSITION;
//----------------------------------------------------------------------------
// GC [0xBF][0x10]
//----------------------------------------------------------------------------
typedef struct  
{
	PBMSG_HEADER2 h;
	BYTE		btDoppelgangerState;	// 0: wait, 1: wait->ready, 2: ready->play, 3: play->end, 
} PMSG_DOPPELGANGER_STATE, *LPPMSG_DOPPELGANGER_STATE;
//----------------------------------------------------------------------------
// GC [0xBF][0x11]
//----------------------------------------------------------------------------
typedef struct  
{
	PBMSG_HEADER2 h;
	BYTE		btIceworkerState;
	BYTE		btPosIndex;
} PMSG_DOPPELGANGER_ICEWORKER_STATE, *LPPMSG_DOPPELGANGER_ICEWORKER_STATE;
//----------------------------------------------------------------------------
// GC [0xBF][0x12] 
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	WORD		wRemainSec;
	BYTE		btUserCount;
	BYTE		btDummy;
	BYTE		UserPosData;
} PMSG_DOPPELGANGER_PLAY_INFO, *LPPMSG_DOPPELGANGER_PLAY_INFO;

typedef struct 
{
	WORD		wUserIndex;
	BYTE		byMapNumber;
	BYTE		btPosIndex;
} PMSG_DOPPELGANGER_USER_POS, *LPPMSG_DOPPELGANGER_USER_POS;
//----------------------------------------------------------------------------
// GC [0xBF][0x13]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		btResult;
	DWORD		dwRewardExp;
}PMSG_DOPPELGANGER_RESULT, *LPPMSG_DOPPELGANGER_RESULT;
//----------------------------------------------------------------------------
// GC [0xBF][0x14]
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2 h;
	BYTE		btMaxGoalCnt;
	BYTE		btGoalCnt;	
}PMSG_DOPPELGANGER_MONSTER_GOAL, * LPPMSG_DOPPELGANGER_MONSTER_GOAL;

#ifdef PBG_ADD_SECRETBUFF
//----------------------------------------------------------------------------
// GC [0xBF][0x15]
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2 h;
	BYTE		btFatiguePercentage;
}PMSG_FATIGUEPERCENTAGE, * LPPMSG_FATIGUEPERCENTAGE;
#endif //PBG_ADD_SECRETBUFF


#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
//----------------------------------------------------------------------------
// GC [0xBF][0x20] 
//----------------------------------------------------------------------------
typedef struct _tagPMSG_ANS_INVENTORY_EQUIPMENT_ITEM
{
	PBMSG_HEADER2	h;
	
	BYTE	btItemPos;
	BYTE	btResult;
} PMSG_ANS_INVENTORY_EQUIPMENT_ITEM, *LPPMSG_ANS_INVENTORY_EQUIPMENT_ITEM;
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY

//----------------------------------------------------------------------------
// GC[0x8E][0x01]
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;
	DWORD		dwKeyValue;
}PMSG_MAPMOVE_CHECKSUM, *LPPMSG_MAPMOVE_CHECKSUM;

//----------------------------------------------------------------------------
// GC[0x8E][0x03]
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2	h;
	BYTE		btResult;		
}PMSG_ANS_MAPMOVE, *LPPMSG_ANS_MAPMOVE;

//----------------------------------------------------------------------------
// GC [0xF7][0x02]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		Result;
	BYTE		Day;
	BYTE		Zone;
	BYTE		Wheather;	
	DWORD		RemainTick;
} PMSG_RESULT_ENTER_EMPIREGUARDIAN, *LPPMSG_RESULT_ENTER_EMPIREGUARDIAN;

//----------------------------------------------------------------------------
// GC [0xF7][0x04]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		Type;
	DWORD		RemainTick;
	BYTE		MonsterCount;
} PMSG_REMAINTICK_EMPIREGUARDIAN, *LPPMSG_REMAINTICK_EMPIREGUARDIAN;

//----------------------------------------------------------------------------
// GC [0xF7][0x06]
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2	h;
	BYTE		Result;
	DWORD		Exp;
} PMSG_CLEAR_RESULT_EMPIREGUARDIAN, *LPPMSG_CLEAR_RESULT_EMPIREGUARDIAN;

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

#pragma pack(push, 1)

//----------------------------------------------------------------------------
// (0xD2)(0x01)
//----------------------------------------------------------------------------
typedef struct 
{
	PBMSG_HEADER2		h;
}PMSG_CASHSHOP_CASHPOINT_REQ, *LPPMSG_CASHSHOP_CASHPOINT_REQ;

//----------------------------------------------------------------------------
// (0xD2)(0x01)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	BYTE				btViewType;

	double				dTotalCash;
	double				dCashCredit;		// C (CreditCard)
	double				dCashPrepaid;		// P (PrepaidCard)
	double				dTotalPoint;
	double				dTotalMileage;
}PMSG_CASHSHOP_CASHPOINT_ANS, *LPPMSG_CASHSHOP_CASHPOINT_ANS;

//----------------------------------------------------------------------------
// (0xD2)(0x02)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	BYTE				byShopOpenType;
}PMSG_CASHSHOP_SHOPOPEN_REQ, *LPPMSG_CASHSHOP_SHOPOPEN_REQ;

//----------------------------------------------------------------------------
// (0xD2)(0x02)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	BYTE				byShopOpenResult;
}PMSG_CASHSHOP_SHOPOPEN_ANS, *LPPMSG_CASHSHOP_SHOPOPEN_ANS;

//----------------------------------------------------------------------------
// (0xD2)(0x03)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;
	
	long				lBuyItemPackageSeq;
	long				lBuyItemDisplaySeq;
	long				lBuyItemPriceSeq;
	WORD				wItemCode;
}PMSG_CASHSHOP_BUYITEM_REQ, *LPPMSG_CASHSHOP_BUYITEM_REQ;

//----------------------------------------------------------------------------
// 아이템 구매 결과 (0xD2)(0x03)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	BYTE				byResultCode;
	long				lItemLeftCount;
}PMSG_CASHSHOP_BUYITEM_ANS, *LPPMSG_CASHSHOP_BUYITEM_ANS;

//----------------------------------------------------------------------------
// (0xD2)(0x04)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	long				lGiftItemPackageSeq;
	long				lDiftItemDisplaySeq;
	long				lGiftItemPriceSeq;
	long				lSaleZone;
	char				chReceiveUserID[MAX_ID_SIZE+1];
	char				chMessage[MAX_GIFT_MESSAGE_SIZE];
}PMSG_CASHSHOP_GIFTSEND_REQ, *LPPMSG_CASHSHOP_GIFTSEND_REQ;

//----------------------------------------------------------------------------
// (0xD2)(0x04)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	BYTE				byResultCode;
	long				lItemLeftCount;
	double				dLimitedCash;
}PMSG_CASHSHOP_GIFTSEND_ANS, *LPPMSG_CASHSHOP_GIFTSEND_ANS;

//----------------------------------------------------------------------------
// (0xD2)(0x05)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;
	int					iPageIndex;
	char				chStorageType;
}PMSG_CASHSHOP_STORAGELIST_REQ, *LPPMSG_CASHSHOP_STORAGELIST_REQ;

//----------------------------------------------------------------------------
// (0xD2)(0x06)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;
	WORD				wTotalItemCount;
	WORD				wCurrentItemCount;
	WORD				wPageIndex;
	WORD				wTotalPage;
}PMSG_CASHSHOP_STORAGECOUNT, *LPPMSG_CASHSHOP_STORAGECOUNT;

//----------------------------------------------------------------------------
// (0xD2)(0x0D)
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
// (0xD2)(0x0E)
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
// (0xD2)(0x07)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	double				dCashValue;

	char				chReceiveUserID[MAX_ID_SIZE+1];
	char				chMessage[MAX_GIFT_MESSAGE_SIZE];
}PMSG_CASHSHOP_CASHSEND_REQ, *LPPMSG_CASHSHOP_CASHSEND_REQ;

//----------------------------------------------------------------------------
// (0xD2)(0x07)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	double				dGiftCashLimit;
	BYTE				byResultCode;
}PMSG_CASHSHOP_CASHSEND_ANS, *LPPMSG_CASHSHOP_CASHSEND_ANS;

//----------------------------------------------------------------------------
// (0xD2)(0x08)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;
}PMSG_CASHSHOP_ITEMBUY_CONFIRM_REQ, *LPPMSG_CASHSHOP_ITEMBUY_CONFIRM_REQ;

//----------------------------------------------------------------------------
// (0xD2)(0x08)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	double				dPresentedCash;
	double				dPresenteLimitCash;

	BYTE				byResult;
	BYTE				byItemBuyPossible;
	BYTE				byPresendPossible;
}PMSG_CASHSHOP_ITEMBUY_CONFIRM_ANS, *LPPMSG_CASHSHOP_ITEMBUY_CONFIRM_ANS;

//----------------------------------------------------------------------------
// (0xD2)(0x09)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	long				lPackageSeq;
}PMSG_CASHSHOP_ITEMBUY_LEFT_COUNT_REQ, *LPPMSG_CASHSHOP_ITEMBUY_LEFT_COUNT_REQ;

//----------------------------------------------------------------------------
// (0xD2)(0x09)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	long				lPackageSeq;
	long				lLeftCount;
}PMSG_CASHSHOP_ITEMBUY_LEFT_COUNT_ANS, *LPPMSG_CASHSHOP_ITEMBUY_LEFT_COUNT_ANS;

//----------------------------------------------------------------------------
// (0xD2)(0x0A)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	long				lStorageSeq;
	long				lStorageItemSeq;
	char				chStorageItemType;
}PMSG_CASHSHOP_STORAGE_ITEM_THROW_REQ, *LPPMSG_CASHSHOP_STORAGE_ITEM_THROW_REQ;

//----------------------------------------------------------------------------
// (0xD2)(0x0A)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	BYTE				byResult;
}PMSG_CASHSHOP_STORAGE_ITEM_THROW_ANS, *LPPMSG_CASHSHOP_STORAGE_ITEM_THROW_ANS;

//----------------------------------------------------------------------------
// (0xD2)(0x0B)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	long				lStorageSeq;
	long				lStorageItemSeq;

	WORD				wItemCode;
	char				chStorageItemType;
}PMSG_CASHSHOP_STORAGE_ITEM_USE_REQ, *LPPMSG_CASHSHOP_STORAGE_ITEM_USE_REQ;

//----------------------------------------------------------------------------
// (0xD2)(0x0B)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;

	BYTE				byResult;
}PMSG_CASHSHOP_STORAGE_ITEM_USE_ANS, *LPPMSG_CASHSHOP_STORAGE_ITEM_USE_ANS;

//----------------------------------------------------------------------------
// (0xD2)(0x0C)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2		h;
	
	WORD				wSaleZone;
	WORD				wYear;
	WORD				wYearIdentify;
}PMSG_CASHSHOP_VERSION_UPDATE, *LPPMSG_CASHSHOP_VERSION_UPDATE;

//----------------------------------------------------------------------------
// (0xD2)(0x13)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2			h;
	
	long				lEventCategorySeq;
}PMSG_CASHSHOP_EVENTITEM_REQ, *LPPMSG_CASHSHOP_EVENTITEM_REQ;


//----------------------------------------------------------------------------
// (0xD2)(0x13)
//----------------------------------------------------------------------------
typedef struct
{
	PBMSG_HEADER2			h;
	
	WORD				wEventItemListCount;
}PMSG_CASHSHOP_EVENTITEM_COUNT, *LPPMSG_CASHSHOP_EVENTITEM_COUNT;


typedef struct
{
	PBMSG_HEADER2			h;
	
	long				lPackageSeq[INGAMESHOP_DISPLAY_ITEMLIST_SIZE];
}PMSG_CASHSHOP_EVENTITEM_LIST, *LPPMSG_CASHSHOP_EVENTITEM_LIST;

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

typedef struct
{
	PBMSG_HEADER2		h;
	
	BYTE				byPeriodItemCount;
}PMSG_PERIODITEMEX_ITEMCOUNT, *LPPMSG_PERIODITEMEX_ITEMCOUNT;


typedef struct
{
	PBMSG_HEADER2		h;
	
	WORD				wItemCode;
	WORD				wItemSlotIndex;
	long				lExpireDate;
}PMSG_PERIODITEMEX_ITEMLIST, *LPPMSG_PERIODITEMEX_ITEMLIST;

#endif // KJH_ADD_PERIOD_ITEM_SYSTEM


#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
//----------------------------------------------------------------------------
// 0x4A
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
// 0x4B
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
BOOL TranslateProtocol(int HeadCode,BYTE* ReceiveBuffer,int Size,BOOL bEncrypted);


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

