#pragma once

#include "dsplaysound.h"
#include "zzzscene.h"
#include "zzzinterface.h"
#include "zzzinventory.h"
#include "zzzMixInventory.h"
#include "SimpleModulus.h"
#include "WSclient.h"
#include "SocketSystem.h"
#include "ItemAddOptioninfo.h"
extern ItemAddOptioninfo*			g_pItemAddOptioninfo;
#ifdef _DEBUG
#define SAVE_PACKET
#define PACKET_SAVE_FILE	"PacketList.txt"
#include "./ExternalObject/leaf/stdleaf.h"
#endif
#include "./Utilities/Log/muConsoleDebug.h"
#include "NewUISystem.h" 
#include "ProtocolSend.h" 
#include "Utilities\Log\DebugAngel.h" 

#define PACKET_MOVE         0xD4
#define PACKET_POSITION     0x15
#define PACKET_MAGIC_ATTACK 0xDB
#define PACKET_ATTACK       0x11

extern CWsctlc SocketClient;
extern CSimpleModulus g_SimpleModulusCS;
extern CSimpleModulus g_SimpleModulusSC;
extern BYTE g_byPacketSerialSend;

extern int HeroKey;
extern int CurrentProtocolState;

extern int DirTable[16];

void BuxConvert(BYTE *Buffer,int Size);

__forceinline bool FindText2(char *Text,char *Token,bool First=false)
{
	int LengthToken = (int)strlen(Token);
	int Length = (int)strlen(Text)-LengthToken;
	if(First)
		Length = 0;
	if(Length < 0)
		return false;
	for(int i=0;i<=Length;i++)
	{
		bool Success = true;
		for(int j=0;j<LengthToken;j++)
		{
			if(Text[i+j] != Token[j])
			{
				Success = false;
				break;
			}
		}
		if(Success)
			return true;
	}
	return false;
}

extern CWsctlc * g_pSocketClient;

__forceinline int SendPacket( char *buf, int len, BOOL bEncrypt = FALSE, BOOL bForceC4 = FALSE)
{
	#ifdef NEW_PROTOCOL_SYSTEM
	gProtocolSend.SendPacketClassic((uint8_t*)buf,len);

	if(SceneFlag >= CHARACTER_SCENE)
		return 1;
	#endif

#ifdef SAVE_PACKET
	LPPHEADER_DEFAULT_SUBCODE pData = ( LPPHEADER_DEFAULT_SUBCODE)buf;
	std::string timeString;
	leaf::GetTimeString(timeString);
	DebugAngel_Write((char*)PACKET_SAVE_FILE, "%s Send \t0x%02X 0x%02X (size = %d)\r\n", timeString.c_str(), pData->Header.HeadCode, pData->SubCode, len);
#endif

	if ( !bEncrypt)
	{
		return ( g_pSocketClient->sSend( buf, len));
	}

	BYTE byBuffer[MAX_SPE_BUFFERSIZE_];
	memcpy( byBuffer, buf, len);
	byBuffer[len] = rand() % 256;

	int iSkip = ( byBuffer[0] == 0xC1) ? 2 : 3;

	byBuffer[iSkip - 1] = g_byPacketSerialSend++;
	--iSkip;

	PBMSG_ENCRYPTED bc;
	PWMSG_ENCRYPTED wc;

	int iSize = g_SimpleModulusCS.Encrypt( NULL, byBuffer + iSkip, len - iSkip);
	
	if ( iSize < 256 && bForceC4 == FALSE)
	{
		int iLength = iSize + 2;

		bc.Code = 0xC3;
		bc.Size = iLength;

		g_SimpleModulusCS.Encrypt( bc.byBuffer, byBuffer + iSkip, len - iSkip);
		assert( iSize < 256);

		return ( g_pSocketClient->sSend( ( char*)&bc, iLength));
	}
	else
	{
		int iLength = iSize + 3;

		wc.Code = 0xC4;
		wc.SizeL = iLength % 256;
		wc.SizeH = iLength / 256;

		g_SimpleModulusCS.Encrypt( wc.byBuffer, byBuffer + iSkip, len - iSkip);

		assert( iSize <= MAX_SPE_BUFFERSIZE_);
		return ( g_pSocketClient->sSend( ( char*)&wc, iLength));
	}
}

#include "StreamPacketEngine.h"

#define END_OF_FUNCTION( pos)	;
#define SendCrcOfFunction( p_Index, p_Next, p_Function, p_Key)	;

#define SendRequestServerList()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xF4);\
	BYTE byData = ( BYTE)0x06;\
	spe.AddData( &byData, 1, FALSE);\
	spe.Send();\
}

#define SendRequestServerAddress( p_Index)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xF4);\
	BYTE byData = ( BYTE)0x03;\
	spe.AddData( &byData, 1, FALSE);\
	WORD Index = ( WORD)( p_Index);\
	spe.AddData( &Index, 2, FALSE);\
	spe.Send();\
	g_pChatListBox->AddText("",GlobalText[470],SEASON3B::TYPE_SYSTEM_MESSAGE);\
	g_pChatListBox->AddText("",GlobalText[471],SEASON3B::TYPE_SYSTEM_MESSAGE);\
}

extern int  LogIn;
extern char LogInID[MAX_ID_SIZE+1];

extern bool First;
extern int FirstTime;
extern BOOL g_bGameServerConnected;

__forceinline void SendCheck( void)
{	
	if ( !g_bGameServerConnected)
	{
		return;
	}

	//gProtocolSend.SendPingTest();

	g_ConsoleDebug->Write(MCD_SEND, "SendCheck");

	CStreamPacketEngine spe;
	spe.Init( 0xC1, 0x0E);
	DWORD dwTick = GetTickCount();
	spe.AddNullData( 1);
	spe << dwTick;

	if(CharacterAttribute->Ability & ABILITY_FAST_ATTACK_SPEED)
	{
		spe << ( WORD)( CharacterAttribute->AttackSpeed-20) << ( WORD)( CharacterAttribute->MagicSpeed-20);
	}
	else if(CharacterAttribute->Ability & ABILITY_FAST_ATTACK_SPEED2)
	{
		spe << ( WORD)( CharacterAttribute->AttackSpeed-20) << ( WORD)( CharacterAttribute->MagicSpeed-20);
	}
	else
	{
		spe << ( WORD)( CharacterAttribute->AttackSpeed) << ( WORD)( CharacterAttribute->MagicSpeed);
	}
	spe.Send( TRUE);

	if(!First)
	{
		First = true;
		FirstTime = dwTick;
	}
}

#define SendCheckSum( dwCheckSum)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x03);\
	spe.AddNullData( 1);\
	spe << ( DWORD)( dwCheckSum);\
	spe.Send( TRUE);\
}

#define SendHackingChecked( byType, byParam)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xF1);\
	spe << ( BYTE)0x03 << ( BYTE)( byType) << ( BYTE)( byParam);\
	spe.Send( TRUE);\
}

extern BYTE Version[SIZE_PROTOCOLVERSION];
extern BYTE Serial[SIZE_PROTOCOLSERIAL+1];

#define SendRequestLogIn( p_lpszID, p_lpszPassword)\
{\
	LogIn = 1;\
	strcpy(LogInID, ( p_lpszID));\
	CurrentProtocolState = REQUEST_LOG_IN;\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xF1);\
	spe << ( BYTE)0x01;\
	char lpszID[MAX_ID_SIZE+1];\
	char lpszPass[MAX_PASSWORD_SIZE+1];\
	ZeroMemory( lpszID, MAX_ID_SIZE+1);\
	ZeroMemory( lpszPass, MAX_PASSWORD_SIZE+1);\
	strncpy(lpszID, p_lpszID, MAX_ID_SIZE);\
	strncpy(lpszPass, p_lpszPassword, MAX_PASSWORD_SIZE);\
	BuxConvert(( BYTE*)lpszID,MAX_ID_SIZE);\
	BuxConvert(( BYTE*)lpszPass,MAX_PASSWORD_SIZE);\
	spe.AddData( lpszID, MAX_ID_SIZE);\
	spe.AddData( lpszPass, MAX_PASSWORD_SIZE);\
	spe << GetTickCount();\
	for(int i=0;i<SIZE_PROTOCOLVERSION;i++)\
	spe << ( BYTE)( Version[i]-(i+1));\
	for(int i=0;i<SIZE_PROTOCOLSERIAL;i++)\
	spe << Serial[i];\
	spe.Send( TRUE);\
	g_pChatListBox->AddText("",GlobalText[472],SEASON3B::TYPE_SYSTEM_MESSAGE);\
	g_pChatListBox->AddText("",GlobalText[473],SEASON3B::TYPE_SYSTEM_MESSAGE);\
}

extern bool LogOut;

__forceinline void SendRequestLogOut(int Flag)
{
	LogOut = true;
	CStreamPacketEngine spe;
	spe.Init( 0xC1, 0xF1);
	spe << ( BYTE)0x02 << ( BYTE)Flag;
	spe.Send( TRUE);

	g_ConsoleDebug->Write(MCD_SEND, "0xF1 [SendRequestLogOut]");
}

extern char Password[MAX_ID_SIZE+1];
extern char QuestionID[MAX_ID_SIZE+1];
extern char Question[31];

#define SendRequestCharactersList( byLanguage)\
{\
	CurrentProtocolState = REQUEST_CHARACTERS_LIST;\
	CStreamPacketEngine spe;\
	spe.Init(0xC1, 0xF3);\
	spe << (BYTE)0x00;\
	spe << (BYTE)byLanguage;\
	spe.Send();\
}

#define SendRequestCreateCharacter( p_ID, p_Class, p_Skin)\
{\
	CurrentProtocolState = REQUEST_CREATE_CHARACTER;\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xF3);\
	spe << ( BYTE)0x01;\
	spe.AddData( ( p_ID), strlen( p_ID));\
	spe.AddNullData( MAX_ID_SIZE - strlen( p_ID));\
	spe << ( BYTE)( (( p_Class)<<4)+( p_Skin));\
	spe.Send();\
}

#define SendRequestDeleteCharacter( p_ID, p_Resident)\
{\
	CurrentProtocolState = REQUEST_DELETE_CHARACTER;\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xF3);\
	spe << ( BYTE)0x02;\
	spe.AddData( ( p_ID), strlen( p_ID));\
	spe.AddNullData( MAX_ID_SIZE - strlen( p_ID));\
	spe.AddData( ( p_Resident), 20);\
	spe.Send();\
}

#define SendRequestJoinMapServer( p_ID)\
{\
	CurrentProtocolState = REQUEST_JOIN_MAP_SERVER;\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xF3);\
	spe << ( BYTE)0x03;\
	spe.AddData( ( p_ID), strlen( ( p_ID)));\
	spe.AddNullData( MAX_ID_SIZE - strlen( ( p_ID)));\
	spe.Send();\
}

extern BOOL g_bWhileMovingZone;
extern DWORD g_dwLatestZoneMoving;

#define SendRequestFinishLoading()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xF3);\
	spe << ( BYTE)0x12;\
	spe.Send();\
\
	g_dwLatestZoneMoving = GetTickCount();\
	g_bWhileMovingZone = FALSE;\
}

extern int  ChatTime;
extern char ChatText[256];

__forceinline void SendChat(const char* Text)
{
	if(ChatTime > 50)
	{
		return;
	}
	else if(ChatTime > 0)
	{
		if(strcmp(ChatText,Text)==NULL) return;
	}
	strcpy(ChatText,Text);
	ChatTime = 70;

    if(FindText2(Hero->ID, (char*)"webzen"))
	{
		BOOL bReturn = TRUE;
		char *lspzCommand[2] = { "/", "!"};
		for ( int i = 0; i < sizeof ( lspzCommand) / sizeof ( char*); i++)
		{
			if ( 0 == strncmp( Text, lspzCommand[i], strlen( lspzCommand[i])))
			{
				bReturn = FALSE;
				break;
			}
		}

		if ( bReturn)
		{
			return;
		}
	}
    if ( Hero->Dead>0 )
    {
	    if ( strlen(GlobalText[260]) > 0 && !strncmp( Text, GlobalText[260], strlen(GlobalText[260]) ) )
	    {
            return;
	    }
    }
    else if ( Text[0]=='/' )
    {
        if( strlen(GlobalText[264]) > 0 && !strncmp( Text, GlobalText[264], strlen( GlobalText[264] ) ) )
        {
			g_pChatInputBox->SetBlockWhisper(true);
			g_pChatListBox->AddText("", GlobalText[267], SEASON3B::TYPE_SYSTEM_MESSAGE);
            return;
        }
        else if( strlen(GlobalText[265]) && !strncmp( Text, GlobalText[265], strlen( GlobalText[265] ) ) )
        {
			g_pChatInputBox->SetBlockWhisper(false);
			g_pChatListBox->AddText("", GlobalText[268], SEASON3B::TYPE_SYSTEM_MESSAGE);
            return;
        }
    }

	CStreamPacketEngine spe;
	spe.Init( 0xC1, 0x00);
	spe.AddData( Hero->ID, MAX_ID_SIZE);
	spe.AddData( (void*)Text, (WORD)min( strlen( Text) + 1, MAX_CHAT_SIZE));
	spe.Send();
}

extern char ChatWhisperID[MAX_ID_SIZE+1];

#define SendChatWhisper( p_TargetID, p_Text)\
{\
    if(!FindText2(Hero->ID,"webzen"))\
	{\
		CStreamPacketEngine spe;\
		spe.Init( 0xC1, 0x02);\
		spe.AddData( ( p_TargetID), MAX_ID_SIZE);\
		spe.AddData( ( p_Text), min( strlen( p_Text) + 1, MAX_CHAT_SIZE));\
		spe.Send();\
\
		memcpy(ChatWhisperID, ( p_TargetID),MAX_ID_SIZE);\
		ChatWhisperID[MAX_ID_SIZE] = NULL;\
	}\
}

#define SendPosition( p_x, p_y)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, PACKET_POSITION );\
	spe << ( BYTE)( p_x) << ( BYTE)( p_y);\
	spe.Send();\
}


extern int MoveCount;

__forceinline void SendCharacterMove(unsigned short Key,float Angle,unsigned char PathNum,unsigned char *PathX,unsigned char *PathY,unsigned char TargetX,unsigned char TargetY)
{
	if(PathNum < 1) 
		return;

	if(PathNum >= MAX_PATH_FIND) 
	{
		PathNum = MAX_PATH_FIND-1;
	}

	CStreamPacketEngine spe;
	spe.Init( 0xC1, PACKET_MOVE );
	spe << PathX[0] << PathY[0];
	BYTE Path[8];
	ZeroMemory( Path, 8);
	BYTE Dir = 0;
	for(int i=1; i<PathNum; i++)
	{
		Dir = 0;
		for(int j=0; j<8; j++)
		{
			if(DirTable[j*2] == (PathX[i]-PathX[i-1]) && DirTable[j*2+1]==(PathY[i]-PathY[i-1]))
			{
				Dir = j;
				break;
			}
		}
		
		if(i%2 == 1)
		{
			Path[(i+1)/2]  = Dir<<4;
		}
		else
		{
			Path[(i+1)/2] += Dir;
		}
	}
	if(PathNum == 1)
	{
		Path[0] = ((BYTE)((Angle+22.5f)/360.f*8.f+1.f)%8) << 4;
	}
	else
	{
		for(int j=0; j<8; j++)
		{
			if(DirTable[j*2]==(TargetX-PathX[PathNum-1]) && DirTable[j*2+1]==(TargetY-PathY[PathNum-1]))
			{
				Dir = j;
				break;
			}
		}
		Path[0] = Dir << 4;
	}
	Path[0] += (BYTE)(PathNum - 1);
	spe.AddData( Path, 1 + (PathNum)/2);
	spe.Send();
}

#define SendRequestAction( p_Action, p_Angle)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x18);\
	spe << ( BYTE)( p_Angle) << ( BYTE)( p_Action);\
	spe.Send();\
}

#define SendRequestAttack( p_Key, p_Dir)\
{\
    if(!FindText2(Hero->ID,"webzen"))\
	{\
		CStreamPacketEngine spe;\
		spe.Init( 0xC1, PACKET_ATTACK);\
		spe << ( BYTE)( ( p_Key) >> 8) << ( BYTE)( ( p_Key)&0xff) << ( BYTE)AT_ATTACK1 << ( BYTE)( p_Dir);\
		spe.Send();\
	}\
}

extern DWORD g_dwLatestMagicTick;

#ifndef _DEBUG

#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
#define SendRequestMagic( p_Type, p_Key)\
{\
	if(!FindText2(Hero->ID,"webzen") && ( p_Type==40 || p_Type==263 || p_Type==261 || abs( (int)(GetTickCount() - g_dwLatestMagicTick)) > 300 ))\
	{\
		g_dwLatestMagicTick = GetTickCount();\
		CStreamPacketEngine spe;\
		WORD Type = (WORD)p_Type;\
		spe.Init( 0xC1, 0x19);\
		spe << ( BYTE)(HIBYTE(Type))<<( BYTE)(LOBYTE(Type)) << ( BYTE)( ( p_Key)>>8) << ( BYTE)( ( p_Key)&0xff);\
		spe.Send( TRUE);\
		hanguo_check3();\
	}\
}
#else //PBG_ADD_NEWCHAR_MONK_SKILL
#define SendRequestMagic( p_Type, p_Key)\
{\
	if(!FindText2(Hero->ID,"webzen") && ( p_Type==40 || abs( (int)(GetTickCount() - g_dwLatestMagicTick)) > 300 ))\
	{\
		g_dwLatestMagicTick = GetTickCount();\
		CStreamPacketEngine spe;\
		WORD Type = (WORD)p_Type;\
		spe.Init( 0xC1, 0x19);\
		spe << ( BYTE)(HIBYTE(Type))<<( BYTE)(LOBYTE(Type)) << ( BYTE)( ( p_Key)>>8) << ( BYTE)( ( p_Key)&0xff);\
		spe.Send( TRUE);\
	}\
}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL

#else // _DEBUG
__forceinline void SendRequestMagic(int Type,int Key)
{
	if( !IsCanBCSkill(Type) )
		return;

#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	if(!FindText2(Hero->ID,"webzen") && ( Type==40 || Type==263 || Type==261 || abs( (int)(GetTickCount() - g_dwLatestMagicTick)) > 300 ))
#else //PBG_ADD_NEWCHAR_MONK_SKILL
	if(!FindText2(Hero->ID,"webzen") && ( Type==40 || abs( (int)(GetTickCount() - g_dwLatestMagicTick)) > 300 ))
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	{
		g_dwLatestMagicTick = GetTickCount();
		CStreamPacketEngine spe;

		WORD p_Type = (WORD)Type;
		spe.Init( 0xC1, 0x19);
		spe << ( BYTE)(HIBYTE(p_Type))<<( BYTE)(LOBYTE(p_Type))<< ( BYTE)( Key>>8) << ( BYTE)( Key&0xff);
		spe.Send( TRUE);
	
	g_ConsoleDebug->Write(MCD_SEND, "0x19 [SendRequestMagic(%d %d)]", Type, Key);
	}
}
#endif //_DEBUG


#define SendRequestCancelMagic( p_Type, p_Key)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x1B);\
	WORD Type = (WORD)p_Type;\
	spe << ( BYTE)(HIBYTE(Type))<<( BYTE)(LOBYTE(Type))<< ( BYTE)( ( p_Key)>>8) << ( BYTE)( ( p_Key)&0xff);\
	spe.Send( TRUE);\
}

BYTE MakeSkillSerialNumber(BYTE * pSerialNumber);

#ifndef ENABLE_EDIT
#define SendRequestMagicAttack( p_Type, p_x, p_y, p_Serial, p_Count, p_Key, p_SkillSerial)\
{\
    if(!FindText2(Hero->ID,"webzen"))\
	{\
		CStreamPacketEngine spe;\
		spe.Init( 0xC1, PACKET_MAGIC_ATTACK );\
		WORD Type = (WORD)p_Type;\
		spe << ( BYTE)(HIBYTE(Type))<<( BYTE)(LOBYTE(Type)) << ( BYTE)( p_x) << ( BYTE)( p_y) << (BYTE)MakeSkillSerialNumber(&p_Serial) << ( BYTE)( p_Count);\
		int *pKey = ( int*)( p_Key);\
		for (int i=0;i<p_Count;i++)\
		{\
			spe << ( BYTE)(pKey[i]>>8) << ( BYTE)( pKey[i]&0xff);\
			spe << ( BYTE)p_SkillSerial;\
		}\
		spe.Send( TRUE);\
	}\
}
#else // ENABLE_EDIT
__forceinline void SendRequestMagicAttack(int Type,int x,int y,BYTE Serial,int Count,int *Key, WORD SkillSerial)
{
    if(FindText2(Hero->ID,"webzen")) return;
	CStreamPacketEngine spe;
	WORD p_Type = (WORD)Type;

	spe.Init( 0xC1, PACKET_MAGIC_ATTACK);
	spe << ( BYTE)(HIBYTE(p_Type))<<( BYTE)(LOBYTE(p_Type))<< ( BYTE)x << ( BYTE)y << (BYTE)MakeSkillSerialNumber(&Serial) << ( BYTE)Count;
	for (int i=0;i<Count;i++)
	{
		spe << ( BYTE)(Key[i]>>8) << ( BYTE)( Key[i]&0xff);
		spe << ( BYTE)SkillSerial;
	}
	spe.Send( TRUE);

	g_ConsoleDebug->Write(MCD_SEND, "0x1D [SendRequestMagicAttack(%d)]", Serial);

}
#endif //ENABLE_EDIT

extern int CurrentSkill;

inline BYTE GetDestValue( int xPos, int yPos, int xDst, int yDst)
{
    int DestX = xDst-xPos;
    int DestY = yDst-yPos;
    if ( DestX<-8 ) DestX = -8;
    if ( DestX>7  ) DestX = 7;
    if ( DestY<-8 ) DestY = -8;
    if ( DestY>7  ) DestY = 7;
	assert( -8 <= DestX && DestX <= 7);
	assert( -8 <= DestY && DestY <= 7);
	BYTE byValue1 = ( ( BYTE)( DestX + 8)) << 4;
	BYTE byValue2 = ( ( BYTE)( DestY + 8)) & 0xf;
	return ( byValue1 | byValue2);
}

#ifndef ENABLE_EDIT
#define SendRequestMagicContinue( p_Type, p_x, p_y,p_Angle,p_Dest,p_Tpos,p_TKey,p_SkillSerial)\
{\
    if(!FindText2(Hero->ID,"webzen"))\
	{\
		CurrentSkill = p_Type;\
		CStreamPacketEngine spe;\
		WORD Type = (WORD)p_Type;\
		spe.Init( 0xC1, 0x1E);\
		spe << ( BYTE)(HIBYTE(Type))<<( BYTE)(LOBYTE(Type)) << ( BYTE)( p_x) << ( BYTE)( p_y) << ( BYTE)( p_Angle) << ( BYTE)( p_Dest) << ( BYTE)( p_Tpos) << ( BYTE)( ( p_TKey)>>8) << ( BYTE)( ( p_TKey)&0xff);\
		spe << MakeSkillSerialNumber(p_SkillSerial);\
		spe.Send( TRUE);\
	}\
}
#else // ENABLE_EDIT
__forceinline void SendRequestMagicContinue(int Type,int x,int y,int Angle, BYTE Dest, BYTE Tpos, WORD TKey, 
											BYTE * pSkillSerial
											)
{
    if(FindText2(Hero->ID,"webzen")) return;
	CurrentSkill = Type;
	CStreamPacketEngine spe;
	WORD p_Type = (WORD)Type;
	spe.Init( 0xC1, 0x1E);
	spe << ( BYTE)(HIBYTE(p_Type))<<( BYTE)(LOBYTE(p_Type))
		<< ( BYTE)( x) << ( BYTE)( y) << ( BYTE)( Angle) << ( BYTE)( Dest) << ( BYTE)( Tpos) << ( BYTE)( ( TKey)>>8) << ( BYTE)( ( TKey)&0xff);
	spe << MakeSkillSerialNumber(pSkillSerial);
	spe.Send( TRUE);

	g_ConsoleDebug->Write(MCD_SEND, "0x1E [SendRequestMagicContinue]");
}
#endif //ENABLE_EDIT

extern bool Teleport;

#define SendRequestMagicTeleport( p_pbResult, p_Type, p_x, p_y)\
{	\
	if ( Teleport || g_bWhileMovingZone || ( GetTickCount() - g_dwLatestZoneMoving < 3000))\
	{\
		*( p_pbResult) = false;\
	}\
	else\
	{\
		if(( p_Type)==0)\
		{\
			Teleport = true;\
		}\
		CStreamPacketEngine spe;\
		spe.Init( 0xC1, 0x1C);\
		spe.AddNullData( 1);\
		spe << ( WORD)( p_Type) << ( BYTE)( p_x) << ( BYTE)( p_y);\
		spe.Send( TRUE);\
\
		*( p_pbResult) = true;\
	}\
}

#define SendRequestMagicTeleportB( p_pbResult, p_Index, p_x, p_y )\
{	\
	if ( Teleport )\
	{\
        Teleport = false;\
		*( p_pbResult) = false;\
	}\
	else\
	{\
		Teleport = true;\
		CStreamPacketEngine spe;\
		spe.Init( 0xC1, 0xB0);\
		spe << ( WORD)( p_Index) << ( BYTE)( p_x) << ( BYTE)( p_y);\
		spe.Send( TRUE);\
\
		*( p_pbResult) = true;\
	}\
}

#define SendRequestTalk( p_Key)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x30);\
	spe << ( BYTE)( ( p_Key)>>8) << ( BYTE)( ( p_Key)&0xff);\
	spe.Send( TRUE);\
}

#define SendExitInventory()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x31);\
	spe.Send( FALSE);\
}

#define SendRequestSell( p_Index)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x33);\
	spe << ( BYTE)( p_Index);\
	spe.Send( TRUE);\
	g_pNPCShop->SetSellingItem(true);\
}

extern int BuyCost;

__forceinline void SendRequestBuy(int Index,int Cost)
{
    if(BuyCost != 0) return;
	CStreamPacketEngine spe;
	spe.Init( 0xC1, 0x32);
	spe << ( BYTE)Index;
	spe.Send( TRUE);
	BuyCost = Cost;

	g_ConsoleDebug->Write(MCD_SEND, "0x32 [SendRequestBuy(%d)]", Index);
}

#define SendRequestRepair( p_Index, p_AddGold)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x34);\
	spe << ( BYTE)( p_Index ) << ( BYTE )( p_AddGold );\
	spe.Send( TRUE);\
}

#define SendRequestEventChip( p_Type, p_Index )\
{\
    CStreamPacketEngine spe;\
    spe.Init( 0xC1, 0x95);\
    spe << ( BYTE)( p_Type) << ( BYTE)( p_Index);\
    spe.Send( FALSE);\
}

#define SendRequestMutoNumber()\
{\
    CStreamPacketEngine spe;\
    spe.Init( 0xC1, 0x96);\
    spe.Send( FALSE);\
}

//  
#define SendRequestEventChipExit()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x97);\
	spe.Send( FALSE);\
}

#define SendRequestLenaExchange( p_byType )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x98);\
    spe << (BYTE)( p_byType );\
	spe.Send( FALSE);\
}

#define SendRequestScratchSerial( p_strSerial1, p_strSerial2, p_strSerial3 )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x9D );\
	spe.AddData( p_strSerial1, 5 );\
	spe.AddData( p_strSerial2, 5 );\
	spe.AddData( p_strSerial3, 5 );\
	spe.Send( FALSE );\
}

#define SendRequestServerImmigration( p_ResidentNumber)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x99);\
	spe.AddData( ( p_ResidentNumber), 10);\
	spe.Send( TRUE);\
}

#define SendRequestQuestHistory()\
{\
    CStreamPacketEngine spe;\
    spe.Init ( 0xC1, 0xA0 );\
    spe.Send ( TRUE );\
}

#define SendRequestQuestState( p_Index, p_State )\
{\
    CStreamPacketEngine spe;\
    spe.Init( 0xC1, 0xA2 );\
    spe << (BYTE)( p_Index ) << (BYTE)( p_State );\
    spe.Send ( TRUE );\
}

#define SendRequestAttribute( p_Att )\
{\
    CStreamPacketEngine spe;\
    spe.Init ( 0xC1, 0x9B );\
    spe << (BYTE)( p_Att );\
    spe.Send ( FALSE );\
}

//----------------------------------------------------------------------------
// CG [0xA4][0x00]
//----------------------------------------------------------------------------
#define SendRequestQuestMonKillInfo(byQuestIndex)\
{\
    CStreamPacketEngine spe;\
    spe.Init(0xC1, 0xA4);\
	spe << (BYTE)0x00;\
    spe << (BYTE)byQuestIndex;\
    spe.Send();\
}

#define SendQuestSelection(dwQuestIndex, byResult)\
{\
    CStreamPacketEngine spe;\
    spe.Init(0xC1, 0xF6);\
	spe << (BYTE)0x0A;\
	spe << (DWORD)(dwQuestIndex);\
    spe << (BYTE)(byResult);\
    spe.Send();\
}

#define SendQuestSelAnswer(dwQuestIndex, bySelAnswer)\
{\
    CStreamPacketEngine spe;\
    spe.Init(0xC1, 0xF6);\
	spe << (BYTE)0x0B;\
	spe << (DWORD)(dwQuestIndex);\
    spe << (BYTE)(bySelAnswer);\
    spe.Send();\
}

#define SendRequestQuestComplete(dwQuestIndex)\
{\
    CStreamPacketEngine spe;\
    spe.Init(0xC1, 0xF6);\
	spe << (BYTE)0x0D;\
    spe << (DWORD)(dwQuestIndex);\
    spe.Send();\
}

#define SendSatisfyQuestRequestFromClient(dwQuestIndex)\
{\
    CStreamPacketEngine spe;\
    spe.Init(0xC1, 0xF6);\
	spe << (BYTE)0x10;\
    spe << (DWORD)(dwQuestIndex);\
    spe.Send();\
}

#define SendRequestProgressQuestList()\
{\
    CStreamPacketEngine spe;\
    spe.Init(0xC1, 0xF6);\
	spe << (BYTE)0x1A;\
    spe.Send();\
}

#define SendRequestProgressQuestRequestReward(dwQuestIndex)\
{\
    CStreamPacketEngine spe;\
    spe.Init(0xC1, 0xF6);\
	spe << (BYTE)0x1B;\
    spe << (DWORD)(dwQuestIndex);\
    spe.Send();\
}

#define SendRequestQuestGiveUp(dwQuestIndex)\
{\
    CStreamPacketEngine spe;\
    spe.Init(0xC1, 0xF6);\
	spe << (BYTE)0x0F;\
    spe << (DWORD)(dwQuestIndex);\
    spe.Send();\
}

#define SendRequestQuestByEtcEPList()\
{\
    CStreamPacketEngine spe;\
    spe.Init(0xC1, 0xF6);\
	spe << (BYTE)0x21;\
    spe.Send();\
}

#ifdef ASG_ADD_GENS_SYSTEM
#define SendRequestGensJoining(byInfluence)\
{\
    CStreamPacketEngine spe;\
    spe.Init(0xC1, 0xF8);\
	spe << (BYTE)0x01;\
    spe << (BYTE)(byInfluence);\
    spe.Send();\
}

#define SendRequestGensSecession()\
{\
    CStreamPacketEngine spe;\
    spe.Init(0xC1, 0xF8);\
	spe << (BYTE)0x03;\
    spe.Send();\
}

#define SendRequestGensReward(byInfluence)\
{\
    CStreamPacketEngine spe;\
    spe.Init(0xC1, 0xF8);\
	spe << (BYTE)0x09;\
	spe << (BYTE)(byInfluence);\
    spe.Send();\
}
#define SendRequestGensInfo_Open()\
{\
    CStreamPacketEngine spe;\
    spe.Init(0xC1, 0xF8);\
	spe << (BYTE)0x0B;\
    spe.Send();\
}
#endif //PBG_ADD_GENSRANKING

// (0xF6 0x0A)
#define SendRequestQuestByNPCEPList()\
{\
    CStreamPacketEngine spe;\
    spe.Init(0xC1, 0xF6);\
	spe << (BYTE)0x30;\
    spe.Send();\
}

#define SendRequestAPDPUp()\
{\
    CStreamPacketEngine spe;\
    spe.Init(0xC1, 0xF6);\
	spe << (BYTE)0x31;\
    spe.Send();\
}

__forceinline bool SendRequestEquipmentItem(int iSrcType,int iSrcIndex, ITEM* pItem, int iDstType,int iDstIndex)
{
	if(EquipmentItem || NULL == pItem) return false;

	EquipmentItem = true;

	CStreamPacketEngine spe;
	spe.Init( 0xC1, 0x24);
	
	BYTE splitType;
	if( pItem->option_380 )
	{
		splitType = ((BYTE)(pItem->Type>>5)&240) | 0x08;
	}
	else
		splitType = ((BYTE)(pItem->Type>>5)&240);

	if( pItem->bPeriodItem == true )
	{
		splitType |= 0x02;
	}
	
	if( pItem->bExpiredPeriod == true )
	{
		splitType |= 0x04;
	}

	BYTE spareBits;
	if (g_SocketItemMgr.IsSocketItem(pItem))
	{
		spareBits = pItem->SocketSeedSetOption;
	}
	else
	{
		spareBits = (((BYTE)pItem->Jewel_Of_Harmony_Option)<<4)+((BYTE)pItem->Jewel_Of_Harmony_OptionLevel);
	}

	BYTE socketBits[5] = { pItem->bySocketOption[0], pItem->bySocketOption[1], pItem->bySocketOption[2], pItem->bySocketOption[3], pItem->bySocketOption[4] };
	
#ifdef KJH_FIX_SEND_REQUEST_INVENTORY_ITEMINFO_CASTING
	spe << (BYTE)(iSrcType&0xff) << (BYTE)(iSrcIndex&0xff) << (BYTE)(pItem->Type&0xff) << (BYTE)(pItem->Level&0xff)
		<< pItem->Durability << pItem->Option1 << pItem->ExtOption
		<< splitType << spareBits
		<< socketBits[0] << socketBits[1] << socketBits[2] << socketBits[3] << socketBits[4]
		<< (BYTE)(iDstType&0xff) << (BYTE)(iDstIndex&0xff);
#else // KJH_FIX_SEND_REQUEST_INVENTORY_ITEMINFO_CASTING
	spe << BYTECAST(char, iSrcType) << BYTECAST(char, iSrcIndex) << BYTECAST(char, pItem->Type) << BYTECAST(char, pItem->Level)
		<< BYTECAST(char,pItem->Durability) << BYTECAST(char,pItem->Option1) << BYTECAST(char,pItem->ExtOption)
		<< splitType << spareBits
		<< socketBits[0] << socketBits[1] << socketBits[2] << socketBits[3] << socketBits[4]
		<< ( BYTE)iDstType << ( BYTE)iDstIndex;
#endif // KJH_FIX_SEND_REQUEST_INVENTORY_ITEMINFO_CASTING
	spe.Send( TRUE);

	g_ConsoleDebug->Write(MCD_SEND, "0x24 [SendRequestEquipmentItem(%d %d %d %d %d %d %d)]", iSrcIndex, iDstIndex, iSrcType, iDstType, (pItem->Type&0x1FFF), ( BYTE)( pItem->Level), ( BYTE)( pItem->Durability));

	return true;
}

__forceinline void SendRequestEquipmentItem(int SrcFlag,int SrcIndex,int DstFlag,int DstIndex)
{
	if(EquipmentItem) return;
	EquipmentItem = true;

	CStreamPacketEngine spe;
	spe.Init( 0xC1, 0x24);
	
	BYTE splitType;

	if( PickItem.option_380 )
	{
		splitType = ((BYTE)(PickItem.Type>>5)&240) | 0x08;
	}
	else
		splitType = ((BYTE)(PickItem.Type>>5)&240);

#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
	if( PickItem.bPeriodItem == true )
	{
		splitType |= 0x02;
	}

	if( PickItem.bExpiredPeriod == true )
	{
		splitType |= 0x04;
	}
#endif // #ifdef KJH_ADD_PERIOD_ITEM_SYSTEM

	BYTE spareBits;
	if (g_SocketItemMgr.IsSocketItem(&PickItem))
	{
		spareBits = PickItem.SocketSeedSetOption;
	}
	else
	{
		spareBits = (((BYTE)PickItem.Jewel_Of_Harmony_Option)<<4)+((BYTE)PickItem.Jewel_Of_Harmony_OptionLevel);
	}

	BYTE socketBits[5] = { PickItem.bySocketOption[0], PickItem.bySocketOption[1], PickItem.bySocketOption[2], PickItem.bySocketOption[3], PickItem.bySocketOption[4] };
	
	spe << ( BYTE)SrcFlag << ( BYTE)SrcIndex << ( BYTE)PickItem.Type << ( BYTE)( PickItem.Level)
		<< ( BYTE)( PickItem.Durability) << ( BYTE)( PickItem.Option1) << ( BYTE)( PickItem.ExtOption) << splitType << spareBits
		<< socketBits[0] << socketBits[1] << socketBits[2] << socketBits[3] << socketBits[4]
		<< ( BYTE)DstFlag << ( BYTE)DstIndex;
	spe.Send( TRUE);

	g_ConsoleDebug->Write(MCD_SEND, "0x24 [SendRequestEquipmentItem(%d %d %d %d %d %d %d)]", SrcIndex,DstIndex,SrcFlag,DstFlag, (PickItem.Type&0x1FFF), ( BYTE)( PickItem.Level), ( BYTE)( PickItem.Durability));
}

extern int  EnableUse;


#ifndef ENABLE_EDIT

#define SendRequestUse( p_Index, p_Target)\
{\
	if( !IsCanUseItem() )\
	{\
		g_pChatListBox->AddText("",GlobalText[474],SEASON3B::TYPE_ERROR_MESSAGE);\
	}\
	else\
	{\
		if(EnableUse <= 0)\
		{\
			EnableUse = 10;\
			CStreamPacketEngine spe;\
			spe.Init( 0xC1, 0x26);\
			spe << ( BYTE)( ( p_Index)+12) << ( BYTE)( p_Target);\
			spe << (BYTE)g_byItemUseType;\
			spe.Send( TRUE);\
			ITEM* pItem = g_pMyInventory->FindItem(p_Index);\
			if(pItem)\
			{\
				if(pItem->Type == ITEM_POTION)\
					PlayBuffer(SOUND_EAT_APPLE01);\
				else if(pItem->Type >= ITEM_POTION+1 && pItem->Type <= ITEM_POTION + 9)\
					PlayBuffer(SOUND_DRINK01);\
			}\
		}\
	}\
}

#else	// ENABLE_EDIT
__forceinline void SendRequestUse(int Index,int Target)
{
	if( !IsCanUseItem() )
	{
		g_pChatListBox->AddText("", GlobalText[474], SEASON3B::TYPE_ERROR_MESSAGE);
		return;
	}
	if(EnableUse > 0) 
	{
		return;
	}
	EnableUse = 10;
	CStreamPacketEngine spe;
	spe.Init( 0xC1, 0x26);
	spe << ( BYTE)( Index+MAX_EQUIPMENT_INDEX ) << ( BYTE)Target;
	spe << (BYTE)g_byItemUseType;
	spe.Send( TRUE);
	if(Inventory[Index].Type==ITEM_POTION)
		PlayBuffer(SOUND_EAT_APPLE01);
	else if(Inventory[Index].Type>=ITEM_POTION + 1 && Inventory[Index].Type<=ITEM_POTION + 9)
		PlayBuffer(SOUND_DRINK01);

	g_ConsoleDebug->Write(MCD_SEND, "0x26 [SendRequestUse(%d)]", Index+12);
}
#endif //ENABLE_EDIT

extern int SendGetItem;
extern int SendDropItem;

#define SendRequestGetItem( p_Key)\
{\
	if(SendGetItem == -1)\
	{\
		SendGetItem = p_Key;\
\
		CStreamPacketEngine spe;\
		spe.Init( 0xC1, 0x22);\
		spe << ( BYTE)( ( p_Key)>>8) << ( BYTE)( ( p_Key)&0xff);\
		spe.Send( TRUE);\
	}\
}

#define SendRequestDropItem( p_InventoryIndex, p_x, p_y)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x23);\
	spe << ( BYTE)( p_x) << ( BYTE)( p_y) << ( BYTE)( p_InventoryIndex);\
	spe.Send( TRUE);\
	SendDropItem = ( p_InventoryIndex);\
}

#ifdef _PVP_ADD_MOVE_SCROLL
#define SendRequestCharacterEffect( p_key, p_type)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x48);\
	spe << ( BYTE)( p_key >> 8) << ( BYTE)( p_key & 0x00FF) << ( BYTE)( p_type);\
	spe.Send( FALSE);\
}
#endif	// _PVP_ADD_MOVE_SCROLL

#define SendRequestAddPoint( p_Type)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xF3);\
	spe << ( BYTE)0x06 << ( BYTE)( p_Type);\
	spe.Send();\
    PlayBuffer(SOUND_CLICK01);\
}

#define SendRequestTrade( p_Key)\
{\
	if( IsCanTrade() && EnableMainRender )\
	{\
		CStreamPacketEngine spe;\
		spe.Init( 0xC1, 0x36);\
		spe << ( BYTE)( ( p_Key)>>8) << ( BYTE)( ( p_Key)&0xff);\
		spe.Send( TRUE);\
\
		char Text[100];\
		sprintf(Text,GlobalText[475],CharactersClient[FindCharacterIndex(p_Key)].ID);\
		g_pChatListBox->AddText("", Text, SEASON3B::TYPE_SYSTEM_MESSAGE);\
	}\
}

#define SendRequestTradeAnswer( p_Result)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x37);\
	spe << ( BYTE)( p_Result);\
	spe.Send();\
}

#define SendRequestTradeGold( p_Gold)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x3A);\
	spe.AddNullData( 1);\
	spe << ( DWORD)( p_Gold);\
	spe.Send();\
}

#define SendRequestTradeResult( p_Result)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x3C);\
	spe << ( BYTE)( p_Result);\
	spe.Send( TRUE);\
}

#define SendRequestTradeExit()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x3D);\
	spe.Send( TRUE);\
}

#define SendPing()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x71);\
	spe.Send();\
}

__forceinline void SendRequestMoveMap(DWORD dwBlockKey,WORD wMapIndex)
{
	CStreamPacketEngine spe;
	spe.Init( 0xC1, 0x8E);
	spe << (BYTE)0x02;\
	spe << ( DWORD)dwBlockKey << ( WORD)wMapIndex;
	spe.Send();

	g_ConsoleDebug->Write(MCD_SEND, "0x8E [SendRequestMoveMap(%d %d)]", dwBlockKey, wMapIndex);
}

__forceinline void SendRequestStorageGold(int Flag,int Gold)
{
	CStreamPacketEngine spe;
	spe.Init( 0xC1, 0x81);
	spe << ( BYTE)Flag << ( DWORD)Gold;
	spe.Send();

	g_ConsoleDebug->Write(MCD_SEND, "0x81 Send [SendRequestStorageGold(%d %d)]", Flag, Gold);
}

__forceinline bool SendRequestStorageExit()
{
	CStreamPacketEngine spe;
	spe.Init( 0xC1, 0x82);
	spe.Send();

	g_ConsoleDebug->Write(MCD_SEND, "0x82 Send [SendRequestStorageExit]");
	return true;
}

#define SendStoragePassword( p_byType, p_wPassword, p_ResidentNumber)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x83);\
	spe << ( BYTE)( p_byType) << ( WORD)( p_wPassword);\
	spe.AddData( ( p_ResidentNumber), 20);\
	spe.Send( TRUE);\
}

#define SendRequestParty( p_Key)\
{\
	if(EnableMainRender)\
	{\
		PartyKey = p_Key;\
		CStreamPacketEngine spe;\
		spe.Init( 0xC1, 0x40);\
		spe << ( BYTE)( ( p_Key)>>8) << ( BYTE)( ( p_Key)&0xff);\
		spe.Send( TRUE);\
\
		char Text[100];\
		sprintf(Text,GlobalText[476],CharactersClient[FindCharacterIndex(p_Key)].ID);\
		g_pChatListBox->AddText("",Text,SEASON3B::TYPE_SYSTEM_MESSAGE);\
	}\
}

#define SendRequestPartyAnswer( p_Result)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x41);\
	spe << ( BYTE)( p_Result) << ( BYTE)( PartyKey>>8) << ( BYTE)( PartyKey&0xff);\
	spe.Send( TRUE);\
}

#define SendRequestPartyList()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x42);\
	spe.Send();\
}

#define SendRequestPartyLeave( p_Index)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x43);\
	spe << ( BYTE)( p_Index);\
	spe.Send();\
}

#define SendRequestGuildMaster( p_Value)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x54);\
	spe << ( BYTE)( p_Value);\
	spe.Send();\
}

#define SendRequestCreateGuild( GuildType, pGuildName, pGuildMark )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x55);\
	spe << (BYTE)(GuildType);\
	spe.AddData( (pGuildName), 8);\
	spe.AddData( (pGuildMark), 32);\
	spe.Send();\
}

#define SendRequestEditGuildType( GuildType )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xE2);\
	spe << (BYTE)(GuildType);\
	spe.Send();\
}

#define SendRequestGuildRelationShip( RelationType, RequestType, TargetUserIndexH, TargetUserIndexL )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xE5);\
	spe << (BYTE)(RelationType);\
	spe << (BYTE)(RequestType);\
	spe << (BYTE)(TargetUserIndexH);\
	spe << (BYTE)(TargetUserIndexL);\
	spe.Send();\
	char szTmp[100];\
	if( RelationType == 0x01 && RequestType == 0x01 )\
		sprintf(szTmp,GlobalText[1358],CharactersClient[FindCharacterIndex(MAKEWORD(TargetUserIndexL,TargetUserIndexH))].ID);\
	else if( RelationType == 0x01 && RequestType == 0x02 )\
		sprintf(szTmp,GlobalText[1387]);\
	else if( RelationType == 0x02 && RequestType == 0x01 )\
		sprintf(szTmp,GlobalText[1359],CharactersClient[FindCharacterIndex(MAKEWORD(TargetUserIndexL,TargetUserIndexH))].ID);\
	else if( RelationType == 0x02 && RequestType == 0x02 )\
		sprintf(szTmp,GlobalText[1360],CharactersClient[FindCharacterIndex(MAKEWORD(TargetUserIndexL,TargetUserIndexH))].ID);\
	g_pChatListBox->AddText("",szTmp,SEASON3B::TYPE_SYSTEM_MESSAGE);\
}

#define SendRequestGuildRelationShipResult( Type, RequestType, Result, TargetUserIndexH, TargetUserIndexL )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xE6);\
	spe << (BYTE)(Type);\
	spe << (BYTE)(RequestType);\
	spe << (BYTE)(Result);\
	spe << (BYTE)(TargetUserIndexH);\
	spe << (BYTE)(TargetUserIndexL);\
	spe.Send();\
}
#define SendRequestBanUnionGuild( GuildName )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xEB);\
	spe << (BYTE)0x01;\
	spe.AddData( (GuildName), 8);\
    spe.Send();\
}

#define SendRequestUnionList()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xE9);\
	spe.Send();\
}

#define SendRequestEditGuildMark( p_Name, p_Mark )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x55);\
	spe.AddData( ( p_Name), 8);\
	spe.AddData( ( p_Mark), 32);\
	spe.Send();\
}

#define SendRequestGuild( p_Key)\
{\
	if(EnableMainRender)\
	{\
		GuildPlayerKey = ( p_Key);\
		CStreamPacketEngine spe;\
		spe.Init( 0xC1, 0x50);\
		spe << ( BYTE)( ( p_Key)>>8) << ( BYTE)( ( p_Key)&0xff);\
		spe.Send();\
\
		char Text[100];\
		sprintf(Text,GlobalText[477],CharactersClient[FindCharacterIndex(p_Key)].ID);\
		g_pChatListBox->AddText("",Text,SEASON3B::TYPE_SYSTEM_MESSAGE);\
	}\
}

#define SendRequestGuildAnswer( p_Result)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x51);\
	spe << ( BYTE)( p_Result) << ( BYTE)( GuildPlayerKey>>8) << ( BYTE)( GuildPlayerKey&0xff);\
	spe.Send();\
}

#define SendRequestCreateGuildCancel()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x57);\
	spe.Send();\
}

#define SendRequestGuildList()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x52);\
	spe.Send();\
}

#define SendRequestGuildLeave( p_ID, p_ResidentNumber)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x53);\
	spe.AddData( ( p_ID), MAX_ID_SIZE);\
	spe.AddData( ( p_ResidentNumber), 20);\
	spe.Send();\
}

#define SendRequestDeclareWar( p_Name)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x60);\
	spe.AddData( ( p_Name), 8);\
	spe.Send();\
}


void InitGuildWar();

#define SendRequestGuildWarAnswer( p_Result)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x61);\
	spe << ( BYTE)( p_Result);\
	spe.Send();\
	if(!p_Result)\
	{\
		InitGuildWar();\
	}\
}

#define SendRequestGuildInfo( p_GuildKey)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x66);\
	spe.AddNullData( 1);\
	spe << ( int)( p_GuildKey);\
	spe.Send();\
}

#define SendRequestGuildAssign( Type, GuildStatus, Name )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xE1 );\
	spe << (BYTE)(Type);\
	spe << (BYTE)(GuildStatus);\
	spe.AddData( (Name), MAX_ID_SIZE );\
	spe.Send();\
}

#define SendRequestMix( p_Type, p_SubType)\
{	\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x86);\
    spe << ( BYTE)( p_Type);\
    spe << ( BYTE)( p_SubType);\
	spe.Send();\
}

__forceinline bool SendRequestMixExit()
{
	CStreamPacketEngine spe;
	spe.Init( 0xC1, 0x87);
	spe.Send();

	g_ConsoleDebug->Write(MCD_SEND, "0x87 [SendRequestMixExit]");
	return true;
}
#define SendRequestGemMix( iType, iLevel )\
{	\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xBC);\
	spe << (BYTE)0x00;\
	spe << ( BYTE)( iType ) << ( BYTE)( iLevel );\
	spe.Send();\
}

#define SendRequestGemUnMix( iType, iLevel, iPos )\
{	\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xBC);\
	spe << (BYTE)0x01;\
	spe << ( BYTE)( iType ) << ( BYTE)( iLevel ) << (BYTE)(iPos);\
	spe.Send();\
}

#define SendRequestMoveToDevilSquare( bySquareNumber, iItemIndex)\
{	\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x90);\
	spe << ( BYTE)( bySquareNumber) << ( BYTE)( ( iItemIndex)+12);\
	spe.Send();\
}

#define SendRequestDevilSquareOpenTime()\
{	\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x91);\
	spe.Send();\
}

#define SendRequestEventCount(wType)\
{   \
    CStreamPacketEngine spe;\
    spe.Init( 0xC1, 0x9F );\
    spe << ( BYTE)( wType );\
    spe.Send();\
}

#define SendRequestMoveToEventMatch(byCastleNumber, iItemIndex)\
{	\
	CStreamPacketEngine spe;\
	spe.Init(0xC1, 0x9A);\
	spe << (BYTE)(byCastleNumber) << (BYTE)(iItemIndex);\
	spe.Send();\
}

#define SendRequestEventZoneOpenTime(byZoneType, iItemLevel)\
{	\
	CStreamPacketEngine spe;\
	spe.Init(0xC1, 0x91);\
	spe << (BYTE)(byZoneType) << (BYTE)(iItemLevel);\
	spe.Send();\
}

#define SendRequestMoveToEventMatch2(byCastleNumber, iItemIndex)\
{	\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xAF);\
	spe << (BYTE)0x01;\
    spe << (BYTE)(byCastleNumber) << (BYTE)(iItemIndex);\
	spe.Send();\
}


#define SendRequestCheckPosition(byPositionX, byPositionY)\
{   \
    CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xAF);\
	spe << (BYTE)0x02;\
    spe << (BYTE)(byPositionX) << (BYTE)(byPositionY);\
    spe.Send();\
}

#define SendRequestHotKey(option)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xF3);\
	spe << ( BYTE)0x30;\
	spe.AddData( option, 30);\
	spe.Send();\
}

#define SendRequestDuelStart(index, name)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xAA);\
	spe << ( BYTE)0x01;\
	spe << (BYTE)((SHORT)index>>8) << (BYTE)((SHORT)index&0xFF);\
	spe.AddData(name, 10);\
	spe.Send( TRUE);\
}
#define SendRequestDuelOk(ok, index, name)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xAA);\
	spe << ( BYTE)0x02;\
	spe << ( BYTE)( ok) << (BYTE)((SHORT)index>>8) << (BYTE)((SHORT)index&0xFF);\
	spe.AddData( name, 10);\
	spe.Send( TRUE);\
}
#define SendRequestDuelEnd()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xAA);\
	spe << ( BYTE)0x03;\
	spe.Send( TRUE);\
}
#define SendRequestJoinChannel(channelid)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xAA);\
	spe << ( BYTE)0x07;\
	spe << ( BYTE)( channelid);\
	spe.Send( TRUE);\
}
#define SendRequestQuitChannel(channelid)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xAA);\
	spe << ( BYTE)0x09;\
	spe << ( BYTE)( channelid);\
	spe.Send( TRUE);\
}

#define SendRequestSetSalePrice(offset, money)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x3F);\
	spe << ( BYTE)0x01;\
	spe << ( BYTE)offset;\
	spe << ( INT)money;\
	spe.Send( TRUE);\
}

#define SendRequestCreatePersonalShop(name)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x3F);\
	spe << ( BYTE)0x02;\
	spe.AddData( name, MAX_SHOPTITLE);\
	spe.Send( TRUE);\
}

#define SendRequestDestoryPersonalShop()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x3F);\
	spe << (BYTE)0x03;\
	spe.Send( TRUE);\
}

#define SendRequestOpenPersonalShop(index, Id)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x3F);\
	spe << (BYTE)0x05;\
	spe << (BYTE)((SHORT)index>>8) << (BYTE)((SHORT)index&0xFF);\
	spe.AddData(Id, MAX_ID_SIZE);\
	spe.Send( TRUE);\
}

#define SendRequestPurchase(index, Id, offset)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x3F);\
	spe << (BYTE)0x06;\
	spe << (BYTE)((SHORT)index>>8) << (BYTE)((SHORT)index&0xFF);\
	spe.AddData(Id, MAX_ID_SIZE);\
	spe << (BYTE)offset;\
	spe.Send( TRUE);\
}

#define SendRequestClosePersonalShop(index, Id)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x3F);\
	spe << (BYTE)0x07;\
	spe << (BYTE)((SHORT)index>>8) << (BYTE)((SHORT)index&0xFF);\
	spe.AddData(Id, MAX_ID_SIZE);\
	spe.Send( TRUE);\
}

#define SendRequestInventory()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xF3);\
	spe << (BYTE)0x10;\
	spe.Send( TRUE);\
}

#define SendRequestFriendList()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xC0);\
	spe.Send();\
}

#define SendRequestAddFriend(name)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xC1);\
	spe.AddData(name, 10);\
	spe.Send();\
}

#define SendAcceptAddFriend(result, name)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xC2);\
	spe << ( BYTE)( result);\
	spe.AddData(name, 10);\
	spe.Send();\
}

#define SendRequestDeleteFriend(name)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xC3);\
	spe.AddData(name, 10);\
	spe.Send();\
}

#define SendRequestChangeState(ChatState)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xC4);\
	spe << (BYTE)(ChatState);\
	spe.Send();\
}

#define SendRequestSendLetter(windowuiid, name, subject, photodir, photoaction, memosize, memo)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC2, 0xC5);\
	spe << (DWORD)(windowuiid);\
	spe.AddData(name, MAX_ID_SIZE);\
	spe.AddData(subject, MAX_LETTER_TITLE_LENGTH);\
	spe << (BYTE)(photodir);\
	spe << (BYTE)(photoaction);\
	spe << (WORD)(memosize);\
	spe.AddData(memo, memosize);\
	spe.Send(TRUE, TRUE);\
}

#define SendRequestLetterText(index)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xC7);\
	spe.AddNullData( 1);\
	spe << (WORD)(index);\
	spe.Send();\
}

#define SendRequestDeleteLetter(index)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xC8);\
	spe.AddNullData( 1);\
	spe << (WORD)(index);\
	spe.Send();\
}

#define SendRequestDeliverLetter()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xC9);\
	spe.Send();\
}

#define SendRequestCreateChatRoom(Name)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xCA);\
	spe.AddData(Name, 10);\
	spe.Send();\
}

#define SendRequestCRInviteFriend(Name, RoomNumber, WindowUIID)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xCB);\
	spe.AddData(Name, 10);\
	spe.AddNullData( 1);\
	spe << (WORD)(RoomNumber);\
	spe << (DWORD)(WindowUIID);\
	spe.Send();\
}

#define SendRequestCRConnectRoom(pSocketClient, RoomNumber, Ticket)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x00);\
	spe.AddNullData( 1);\
	spe << (WORD)(RoomNumber);\
	spe.AddData(Ticket, 10);\
	g_pSocketClient = pSocketClient;\
	spe.Send();\
	g_pSocketClient = &SocketClient;\
}

#define SendRequestCRDisconnectRoom(pSocketClient)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x01);\
	g_pSocketClient = pSocketClient;\
	spe.Send();\
	g_pSocketClient = &SocketClient;\
}

#define SendRequestCRUserList(pSocketClient)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x02);\
	g_pSocketClient = pSocketClient;\
	spe.Send();\
	g_pSocketClient = &SocketClient;\
}

#define SendRequestCRChatText(pSocketClient, Index, MsgSize, Msg)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x04);\
	spe << (BYTE)(Index);\
	spe << (BYTE)(MsgSize);\
	BuxConvert((BYTE *)Msg, MsgSize);\
	spe.AddData(Msg, MsgSize);\
	g_pSocketClient = pSocketClient;\
	spe.Send();\
	g_pSocketClient = &SocketClient;\
}

#define SendRequestCRConnectCheck(pSocketClient)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x05);\
	g_pSocketClient = pSocketClient;\
	spe.Send();\
	g_pSocketClient = &SocketClient;\
}

#define SendRequestPetCommand(PetType, Command, Key )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xA7);\
	spe << (BYTE)(PetType);\
	spe << (BYTE)(Command);\
	spe << (BYTE)( (Key)>>8 ) << (BYTE)( (Key)&0xff);\
	spe.Send();\
}

#define SendRequestPetInfo(PetType, InvType, nPos )\
{\
 	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xA9);\
    spe << (BYTE)(PetType);\
    spe << (BYTE)(InvType);\
    spe << (BYTE)(nPos);\
	spe.Send();\
}

#define SendChangeMServer(lpID, lpChr, iAuth1, iAuth2, iAuth3, iAuth4 )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xB1);\
	spe << ( BYTE)0x01;\
	char lpszID[MAX_ID_SIZE+2];\
	char lpszCHR[MAX_ID_SIZE+2];\
	ZeroMemory( lpszID, MAX_ID_SIZE+2);\
	ZeroMemory( lpszCHR, MAX_ID_SIZE+2);\
	strcpy( lpszID, lpID);\
	strcpy( lpszCHR, lpChr);\
	BuxConvert(( BYTE*)lpszID,MAX_ID_SIZE+2);\
	spe.AddData( lpszID, MAX_ID_SIZE+2);\
	spe.AddData( lpszCHR, MAX_ID_SIZE+2);\
	spe << (INT)(iAuth1);\
	spe << (INT)(iAuth2);\
	spe << (INT)(iAuth3);\
	spe << (INT)(iAuth4);\
	spe << GetTickCount();\
	for(int i=0;i<SIZE_PROTOCOLVERSION;i++)\
	spe << ( BYTE)( Version[i]-(i+1));\
	for(int i=0;i<SIZE_PROTOCOLSERIAL;i++)\
	spe << Serial[i];\
	spe.Send(TRUE);\
}

#define SendRequestBCStatus()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xB2);\
	spe << (BYTE)0x00;\
    spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xB2][0x01] 
//----------------------------------------------------------------------------
#define SendRequestBCReg()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xB2);\
	spe << (BYTE)0x01;\
    spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xB2][0x02]
//----------------------------------------------------------------------------
#define SendRequestBCGiveUp( GiveUp )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xB2);\
	spe << (BYTE)0x02;\
	spe << (BYTE)GiveUp;\
    spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xB2][0x03] 
//----------------------------------------------------------------------------
#define SendRequestBCRegInfo()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xB2);\
	spe << (BYTE)0x03;\
    spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xB2][0x04]
//----------------------------------------------------------------------------
#define SendRequestBCRegMark( ItemPos )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xB2);\
	spe << (BYTE)0x04;\
 	spe << (BYTE)ItemPos;\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xB2][0x05]
//----------------------------------------------------------------------------
#define SendRequestBCNPCBuy( NPCNumber, NPCIndex )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xB2);\
	spe << (BYTE)0x05;\
	spe << (int)NPCNumber;\
	spe << (int)NPCIndex;\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xB2][0x06]
//----------------------------------------------------------------------------
#define SendRequestBCNPCRepair( NPCNumber, NPCIndex )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xB2);\
	spe << (BYTE)0x06;\
	spe << (int)NPCNumber;\
	spe << (int)NPCIndex;\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xB2][0x07]
//----------------------------------------------------------------------------
#define SendRequestBCNPCUpgrade( NPCNumber, NPCIndex, NPCUpType, NPCUpValue )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xB2);\
	spe << (BYTE)0x07;\
	spe << (int)NPCNumber;\
	spe << (int)NPCIndex;\
	spe << (int)NPCUpType;\
	spe << (int)NPCUpValue;\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xB2][0x08] 
//----------------------------------------------------------------------------
#define SendRequestBCGetTaxInfo()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xB2);\
	spe << (BYTE)0x08;\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xB2][0x09] 
//----------------------------------------------------------------------------
#define SendRequestBCChangeTaxRate( TaxType, TaxRate1, TaxRate2, TaxRate3, TaxRate4 )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xB2);\
	spe << (BYTE)0x09;\
	spe << (BYTE)TaxType;\
    spe << (BYTE)TaxRate1 << (BYTE)TaxRate2 <<(BYTE)TaxRate3 <<(BYTE)TaxRate4;\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xB2][0x10]
//----------------------------------------------------------------------------
#define SendRequestBCWithdraw( Money1, Money2, Money3, Money4 )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xB2);\
	spe << (BYTE)0x10;\
	spe << (BYTE)Money1;\
	spe << (BYTE)Money2;\
	spe << (BYTE)Money3;\
	spe << (BYTE)Money4;\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xB3]
//----------------------------------------------------------------------------
#define SendRequestBCNPCList( MonsterCode )\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xB3);\
	spe << (BYTE)MonsterCode;\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xB4]
//----------------------------------------------------------------------------
#define SendRequestBCDeclareGuildList()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xB4);\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xB5]
//----------------------------------------------------------------------------
#define SendRequestBCGuildList()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xB5);\
	spe.Send();\
}

//////////////////////////////////////////////////////////////////////////
//  CG [0xB2][0x1F]
//////////////////////////////////////////////////////////////////////////
#define SendRequestHuntZoneEnter( p_HuntZoneEnter )\
{\
    CStreamPacketEngine spe;\
    spe.Init( 0xC1, 0xB2 );\
    spe << (BYTE)0x1F;\
    spe << (BYTE)p_HuntZoneEnter;\
    spe.Send();\
}

    #define SendGetCastleGuildMark()\
    {\
        CStreamPacketEngine spe;\
        spe.Init( 0xC1, 0xB9);\
        spe << (BYTE)0x02;\
        spe.Send();\
    }

    #define SendCastleHuntZoneEnter( p_Money)\
    {\
        CStreamPacketEngine spe;\
        spe.Init( 0xC1, 0xB9);\
        spe << (BYTE)0x05;\
        spe << (INT)p_Money;\
        spe.Send();\
    }

    #define SendGateOparator( p_Operator, p_Key )\
    {\
        CStreamPacketEngine spe;\
        spe.Init( 0xC1, 0xB2);\
        spe << (BYTE)0x12;\
        spe << (BYTE)p_Operator << (BYTE)( ( p_Key) >> 8) << ( BYTE)( ( p_Key)&0xff);\
        spe.Send();\
    }

    #define SendCatapultFire( p_Key, p_Index )\
    {\
        CStreamPacketEngine spe;\
        spe.Init( 0xC1, 0xB7);\
        spe << (BYTE)0x01;\
        spe << (BYTE)( ( p_Key) >> 8) << ( BYTE)( ( p_Key)&0xff) << ( BYTE)(p_Index);\
        spe.Send();\
    }

    #define SendWeaponExplosion( p_KeyH, p_KeyL )\
    {\
        CStreamPacketEngine spe;\
        spe.Init( 0xC1, 0xB7);\
        spe << (BYTE)0x04;\
        spe << (BYTE)p_KeyH << ( BYTE)p_KeyL;\
        spe.Send();\
    }


    #define SendGuildCommand( p_Team, p_posX, p_posY, p_Cmd )\
    {\
        CStreamPacketEngine spe;\
        spe.Init( 0xC1, 0xB2);\
        spe << (BYTE)0x1D;\
        spe << (BYTE)p_Team << (BYTE)p_posX << (BYTE)p_posY << (BYTE)p_Cmd;\
        spe.Send();\
    }

#define SendRequestCrywolfInfo( )\
{	\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xBD);\
	spe << (BYTE)0x00;\
	spe.Send();\
}

#define SendRequestCrywolfAltarContract(p_Key)\
{	\
    CStreamPacketEngine spe;\
    spe.Init( 0xC1, 0xBD);\
    spe << (BYTE)0x03;\
    spe << (BYTE)( (p_Key)>>8 ) << (BYTE)( (p_Key)&0xff);\
    spe.Send();\
}

#define SendRequestCrywolfBenefitPlusChaosRate( )\
{	\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xBD);\
	spe << (BYTE)0x09;\
	spe.Send();\
}

#define SendRequestKanturuInfo( )\
{	\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xD1);\
	spe << (BYTE)0x00;\
	spe.Send();\
}

#define SendRequestKanturuEnter( )\
{	\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xD1);\
	spe << (BYTE)0x01;\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xD0][0x03]
//----------------------------------------------------------------------------
#define SendRequestWhiteAngelItem( )\
{	\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xD0);\
	spe << (BYTE)0x03;\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xD0][0x07]
//----------------------------------------------------------------------------
#define SendRequestEnterOnWerwolf()\
{\
	CStreamPacketEngine spe;\
	spe.Init(0xC1, 0xD0);\
	spe << (BYTE)0x07;\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xD0][0x08]
//----------------------------------------------------------------------------
#define SendRequestEnterOnGateKeeper()\
{\
	CStreamPacketEngine spe;\
	spe.Init(0xC1, 0xD0);\
	spe << (BYTE)0x08;\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xD0][0x09]
//----------------------------------------------------------------------------
#define SendRequest3ColorHarvestItem( )\
{	\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xD0 );\
	spe << (BYTE)0x09;\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xD0][0x10]
//----------------------------------------------------------------------------
#define SendRequestSantaItem( )\
{	\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xD0 );\
	spe << (BYTE)0x10;\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xD0][0x0a]
//----------------------------------------------------------------------------
#define SendRequestMoveDevias( )\
{	\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xD0 );\
	spe << (BYTE)0x0a;\
	spe.Send();\
}

//----------------------------------------------------------------------------
// CG [0xD1][0x10]
//----------------------------------------------------------------------------
#define SendRequestRaklionStateInfo()\
{	\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0xD1 );\
	spe << (BYTE)0x10;\
	spe.Send();\
}

#define SendRequestCursedEnter( mapnumber, itempos ) \
{ \
	CStreamPacketEngine spe; \
	spe.Init(0xC1, 0xBF); \
	spe << (BYTE)0x00; \
	spe << (BYTE)mapnumber << (BYTE)itempos; \
	spe.Send(); \
}

#define SendRequestCursedTempleMagic( MagicNumber, TargetObjIndex, Dis ) \
{ \
	CStreamPacketEngine spe; \
	WORD Magic = (WORD)MagicNumber;\
	spe.Init(0xC1, 0xBF); \
	spe << (BYTE)0x02; \
	spe << ( BYTE)(HIBYTE(Magic))<<( BYTE)(LOBYTE(Magic))<< (BYTE)((TargetObjIndex)>>8) << (BYTE)((TargetObjIndex)&0xff) << (BYTE)Dis; \
	spe.Send(); \
}

#define SendRequestCursedTempleRequital() \
{ \
	CStreamPacketEngine spe; \
	spe.Init(0xC1, 0xBF); \
	spe << (BYTE)0x05; \
	spe.Send(); \
}

#define SendRequestMasterLevelSkill(SkillNum) \
{ \
	CStreamPacketEngine spe; \
	spe.Init(0xC1, 0xF3); \
	spe << (BYTE)0x52; \
	spe << (int)SkillNum;\
	spe.Send(); \
}

#define SendRequestResetCharacterPoint() \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xF2); \
	spe << (BYTE)0x00;\
    spe.Send(); \
}


//////////////////////////////////////////////////////////////////////////
// 0xBF
//////////////////////////////////////////////////////////////////////////
#define SendRequestRegistedLuckyCoin() \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xBF); \
	spe << (BYTE)0x0b;\
    spe.Send(); \
}

#define SendRequestRegistLuckyCoin() \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xBF); \
	spe << (BYTE)0x0C;\
    spe.Send(); \
}

#define SendRequestExChangeLuckyCoin(nCoinCnt) \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xBF); \
	spe << (BYTE)0x0D;\
	spe << (int)nCoinCnt;\
    spe.Send(); \
}

#define SendRequestEnterDoppelGangerEvent(btItemPos) \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xBF); \
	spe << (BYTE)0x0E;\
	spe << (BYTE)btItemPos;\
    spe.Send(); \
}

#define SendRequestEnterEmpireGuardianEvent() \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xF7); \
	spe << (BYTE)0x01;\
	spe << (BYTE)0x01;\
    spe.Send(); \
}

#define SendRequestEnterUnitedMarketPlaceEvent() \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xBF); \
	spe << (BYTE)0x17;\
    spe.Send(); \
}

//----------------------------------------------------------------------------
// (0xD2)(0x01)
//----------------------------------------------------------------------------
#define SendRequestIGS_CashPointInfo() \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xD2); \
	spe << (BYTE)0x01; \
    spe.Send(); \
}


//----------------------------------------------------------------------------
// (0xD2)(0x02)
//----------------------------------------------------------------------------
#define SendRequestIGS_CashShopOpen(byShopOpenType) \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xD2); \
	spe << (BYTE)0x02; \
	spe << (BYTE)byShopOpenType; \
    spe.Send(); \
}

//----------------------------------------------------------------------------
// (0xD2)(0x03)
//----------------------------------------------------------------------------
#define SendRequestIGS_BuyItem(lBuyItemPackageSeq, lBuyItemDisplaySeq, lBuyItemPriceSeq, wItemCode, iCashType) \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xD2); \
	spe << (BYTE)0x03; \
	spe << (long)lBuyItemPackageSeq; \
	spe << (long)lBuyItemDisplaySeq; \
	spe << (long)lBuyItemPriceSeq; \
	spe << (WORD)wItemCode; \
	spe << (int)iCashType; \
    spe.Send(); \
}

#define SendRequestIGS_SendItemGift(lGiftItemPackageSeq, lGiftItemPriceSeq, lGiftItemDisplaySeq, \
	lSaleZone, wItemCode, iCashType, pstrReceiveUserID, pstrGiftMessage) \
{ \
	char strReceiveUserID[MAX_ID_SIZE+1]; \
	char strGiftMessage[MAX_GIFT_MESSAGE_SIZE]; \
	ZeroMemory( strReceiveUserID, MAX_ID_SIZE+1); \
	ZeroMemory( strGiftMessage, MAX_GIFT_MESSAGE_SIZE); \
	memcpy( strReceiveUserID, pstrReceiveUserID, MAX_ID_SIZE+1 ); \
	memcpy( strGiftMessage, pstrGiftMessage, MAX_GIFT_MESSAGE_SIZE); \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xD2); \
	spe << (BYTE)0x04; \
	spe << (long)lGiftItemPackageSeq; \
	spe << (long)lGiftItemPriceSeq; \
	spe << (long)lGiftItemDisplaySeq; \
	spe << (long)lSaleZone; \
	spe << (WORD)wItemCode; \
	spe << (int)iCashType; \
	spe.AddData( strReceiveUserID, MAX_ID_SIZE+1); \
	spe.AddData( strGiftMessage, MAX_GIFT_MESSAGE_SIZE); \
    spe.Send(); \
}

#define SendRequestIGS_ItemStorageList(iPageIndex, szStorageType) \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xD2); \
	spe << (BYTE)0x05; \
	spe << (int)iPageIndex; \
	spe.AddData( szStorageType, sizeof(char)+1 ); \
    spe.Send(); \
}

#define SendRequestIGS_SendCashGift(dCashValue, pstrReceiveUserID, pstrGiftMessage) \
{ \
	char strReceiveUserID[MAX_ID_SIZE+2]; \
	char strGiftMessage[MAX_GIFT_MESSAGE_SIZE+1]; \
	ZeroMemory( lpszID, MAX_ID_SIZE+2); \
	ZeroMemory( lpszPass, MAX_GIFT_MESSAGE_SIZE+1); \
	memcpy( strReceiveUserID, pstrReceiveUserID, MAX_ID_SIZE+1 ); \
	memcpy( strGiftMessage, pstrGiftMessage, MAX_GIFT_MESSAGE_SIZE); \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xD2); \
	spe << (BYTE)0x07; \
	spe << (double)dCashValue; \
	spe.AddData( strReceiveUserID, MAX_ID_SIZE+1); \
	spe.AddData( strGiftMessage, MAX_GIFT_MESSAGE_SIZE); \
    spe.Send(); \
}

#define SendRequestIGS_PossibleBuy() \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xD2); \
	spe << (BYTE)0x08; \
    spe.Send(); \
}

#define SendRequestIGS_LeftCountItem(lPackageSeq) \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xD2); \
	spe << (BYTE)0x09; \
	spe << (long)lPackageSeq; \
    spe.Send(); \
}

#define SendRequestIGS_DeleteStorageItem(lStorageSeq, lStorageItemSeq, pstrStorageItemType) \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xD2); \
	spe << (BYTE)0x0A; \
	spe << (long)lStorageSeq; \
	spe << (long)lStorageItemSeq; \
	spe.AddData( pstrStorageItemType, sizeof(char)); \
    spe.Send(); \
}

#define SendRequestIGS_UseStorageItem(lStorageSeq, lStorageItemSeq, wItemCode, pstrStorageItemType) \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xD2); \
	spe << (BYTE)0x0B; \
	spe << (long)lStorageSeq; \
	spe << (long)lStorageItemSeq; \
	spe << (WORD)wItemCode; \
	spe.AddData( pstrStorageItemType, sizeof(char)); \
    spe.Send(); \
}

#define SendRequestIGS_EventItemList(lEventCategorySeq) \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0xD2); \
	spe << (BYTE)0x13; \
	spe << (long)lEventCategorySeq; \
    spe.Send(); \
}

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
__forceinline bool SendRequestEquippingInventoryItem(int iItemPos, int iValue)
{ 
    CStreamPacketEngine spe; 
    spe.Init( 0xC1, 0xBF); 
	spe << (BYTE)0x20;
	spe << (BYTE)iItemPos; 
	spe << (BYTE)iValue; 
    spe.Send(); 

#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_SEND, "0xBF, 0x20 [SendRequestEquippingInventoryItem(%d, %d)]", iItemPos, iValue);
#endif // CONSOLE_DEBUG
	return true;

}
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
#ifdef _DEBUG
__forceinline void SendRequestRageAtt(int Type,int Key)
{
	if( !IsCanBCSkill(Type) )
		return;
	if(!FindText2(Hero->ID,"webzen"))
	{
		CStreamPacketEngine spe;
		WORD p_Type = (WORD)Type;
		spe.Init( 0xC1, 0x4A);
		spe << ( BYTE)(HIBYTE(p_Type))<<( BYTE)(LOBYTE(p_Type)) << (BYTE)(0) << ( BYTE)(Key>>8) << ( BYTE)(Key&0xff);
		spe.Send( TRUE);
	}
}

__forceinline void SendRequestDarkside(WORD nSkill, int Key)
{
    CStreamPacketEngine spe;
    spe.Init( 0xC1, 0x4B);
	spe <<(BYTE)(HIBYTE(nSkill))<<(BYTE)(LOBYTE(nSkill))<<(BYTE)(Key>>8)<<(BYTE)(Key&0xff);
	spe.Send( TRUE);
}
#else //_DEBUG
#define SendRequestRageAtt( p_Type, p_Key)\
{\
    if(!FindText2(Hero->ID,"webzen"))\
	{\
		CStreamPacketEngine spe;\
		WORD Type = (WORD)p_Type;\
		spe.Init( 0xC1, 0x4A);\
		spe << ( BYTE)(HIBYTE(Type))<<( BYTE)(LOBYTE(Type)) << (BYTE)(0) << ( BYTE)( (p_Key)>>8) << ( BYTE)( (p_Key)&0xff);\
		spe.Send( TRUE);\
		hanguo_check3();\
	}\
}

#define SendRequestDarkside( nSkill, p_Key) \
{ \
    CStreamPacketEngine spe; \
    spe.Init( 0xC1, 0x4B); \
	spe <<(BYTE)(HIBYTE(nSkill))<<(BYTE)(LOBYTE(nSkill))<<(BYTE)((p_Key)>>8)<<(BYTE)((p_Key)&0xff);\
    spe.Send(); \
}
#endif //_DEBUG
#endif //PBG_ADD_NEWCHAR_MONK_SKILL

