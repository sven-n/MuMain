#pragma once

#include <istream>

#include "dsplaysound.h"
#include "zzzscene.h"
#include "zzzinterface.h"
#include "zzzinventory.h"
#include "zzzMixInventory.h"

#include "WSclient.h"
#include "SocketSystem.h"
#include "ItemAddOptioninfo.h"
#include "Dotnet/Connection.h"
//extern ItemAddOptioninfo* g_pItemAddOptioninfo;

#include "./Utilities/Log/muConsoleDebug.h"
#include "NewUISystem.h"
#include "ProtocolSend.h"


// English Protocol:
#define PACKET_MOVE         0xD4
#define PACKET_POSITION     0x15
#define PACKET_MAGIC_ATTACK 0xDB
#define PACKET_ATTACK       0x11

// extern Connection SocketClient;
extern BYTE g_byPacketSerialSend;

extern int DirTable[16];

void BuxConvert(BYTE* Buffer, int Size);

__forceinline bool FindText2(wchar_t* Text, wchar_t* Token, bool First = false)
{
    int LengthToken = (int)wcslen(Token);
    int Length = (int)wcslen(Text) - LengthToken;
    if (First)
        Length = 0;
    if (Length < 0)
        return false;
    for (int i = 0; i <= Length; i++)
    {
        bool Success = true;
        for (int j = 0; j < LengthToken; j++)
        {
            if (Text[i + j] != Token[j])
            {
                Success = false;
                break;
            }
        }
        if (Success)
            return true;
    }
    return false;
}

extern Connection* g_pSocketClient;

__forceinline void SendPacket(const BYTE* buf, int32_t len)
{
    // TODO: this code is pure bullshit, i know. We should get rid of global variables.
    if (g_pSocketClient == nullptr)
    {
        g_pSocketClient = SocketClient;
    }

    g_pSocketClient->Send(buf, len);
}

#include "StreamPacketEngine.h"

#define END_OF_FUNCTION( pos)	;
#define SendCrcOfFunction( p_Index, p_Next, p_Function, p_Key)	;

extern int  LogIn;
extern wchar_t LogInID[MAX_ID_SIZE + 1];

extern bool First;
extern int FirstTime;
extern BOOL g_bGameServerConnected;


extern bool LogOut;


extern wchar_t Password[MAX_ID_SIZE + 1];
extern wchar_t QuestionID[MAX_ID_SIZE + 1];
extern wchar_t Question[31];


inline bool IsWebzenCharacter()
{
	const std::wstring character_name = std::wstring(Hero->ID);

	return character_name.find(L"webzen") >= 0;
}

extern int MoveCount;

__forceinline void SendCharacterMove(unsigned short Key, float Angle, unsigned char PathNum, unsigned char* PathX, unsigned char* PathY, unsigned char TargetX, unsigned char TargetY)
{
    if (PathNum < 1)
        return;

    if (PathNum >= MAX_PATH_FIND)
    {
        PathNum = MAX_PATH_FIND - 1;
    }

    CStreamPacketEngine spe;
    spe.Init(0xC1, PACKET_MOVE);
    spe << PathX[0] << PathY[0];
    BYTE Path[8];
    ZeroMemory(Path, 8);
    BYTE Dir = 0;
    for (int i = 1; i < PathNum; i++)
    {
        Dir = 0;
        for (int j = 0; j < 8; j++)
        {
            if (DirTable[j * 2] == (PathX[i] - PathX[i - 1]) && DirTable[j * 2 + 1] == (PathY[i] - PathY[i - 1]))
            {
                Dir = j;
                break;
            }
        }

        if (i % 2 == 1)
        {
            Path[(i + 1) / 2] = Dir << 4;
        }
        else
        {
            Path[(i + 1) / 2] += Dir;
        }
    }
    if (PathNum == 1)
    {
        Path[0] = ((BYTE)((Angle + 22.5f) / 360.f * 8.f + 1.f) % 8) << 4;
    }
    else
    {
        for (int j = 0; j < 8; j++)
        {
            if (DirTable[j * 2] == (TargetX - PathX[PathNum - 1]) && DirTable[j * 2 + 1] == (TargetY - PathY[PathNum - 1]))
            {
                Dir = j;
                break;
            }
        }
        Path[0] = Dir << 4;
    }
    Path[0] += (BYTE)(PathNum - 1);
    spe.AddData(Path, 1 + (PathNum) / 2);
    spe.Send();
}


inline void SendRequestAction(OBJECT& obj, BYTE action)
{
	BYTE rotation = (BYTE)((obj.Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
	SocketClient->ToGameServer()->SendAnimationRequest(rotation, action);
}


extern DWORD g_dwLatestMagicTick;

#ifndef _DEBUG

inline void SendRequestMagic(Type, Key)
{
	if(!IsWebzenCharacter() && (Type ==40 || Type ==263 || Type ==261 || abs( (int)(GetTickCount() - g_dwLatestMagicTick)) > 300 ))
	{
		g_dwLatestMagicTick = GetTickCount();
		SocketClient->ToGameServer()->SendTargetedSkill(Type, Key);
	}
}
#else // _DEBUG
inline void SendRequestMagic(int Type, int Key)
{
    if (!IsCanBCSkill(Type))
        return;

    if (!IsWebzenCharacter() && (Type == 40 || Type == 263 || Type == 261 || abs((int)(GetTickCount() - g_dwLatestMagicTick)) > 300))
    {
        g_dwLatestMagicTick = GetTickCount();
        SocketClient->ToGameServer()->SendTargetedSkill(Type, Key);
        g_ConsoleDebug->Write(MCD_SEND, L"0x19 [SendRequestMagic(%d %d)]", Type, Key);
    }
}
#endif //_DEBUG

inline WORD g_byLastSkillSerialNumber = 0;

inline BYTE MakeSkillSerialNumber(BYTE* pSerialNumber)
{
    if (pSerialNumber == NULL) return 0;

    ++g_byLastSkillSerialNumber;
    if (g_byLastSkillSerialNumber > 50) g_byLastSkillSerialNumber = 1;

    *pSerialNumber = g_byLastSkillSerialNumber;
    return g_byLastSkillSerialNumber;
}

#ifndef ENABLE_EDIT
#define SendRequestMagicAttack( p_Type, p_x, p_y, p_Serial, p_Count, p_Key, p_SkillSerial)\
{\
    if(!IsWebzenCharacter())\
	{\
		CStreamPacketEngine spe;\
		spe.Init( 0xC3, PACKET_MAGIC_ATTACK );\
		WORD Type = (WORD)p_Type;\
		spe << ( BYTE)(HIBYTE(Type))<<( BYTE)(LOBYTE(Type)) << ( BYTE)( p_x) << ( BYTE)( p_y) << (BYTE)MakeSkillSerialNumber(&p_Serial) << ( BYTE)( p_Count);\
		int *pKey = ( int*)( p_Key);\
		for (int i=0;i<p_Count;i++)\
		{\
			spe << ( BYTE)(pKey[i]>>8) << ( BYTE)( pKey[i]&0xff);\
			spe << ( BYTE)p_SkillSerial;\
		}\
		spe.Send();\
	}\
}
#else // ENABLE_EDIT
__forceinline void SendRequestMagicAttack(int Type, int x, int y, BYTE Serial, int Count, int* Key, WORD SkillSerial)
{
    if (IsWebzenCharacter()) return;
    CStreamPacketEngine spe;
    WORD p_Type = (WORD)Type;

    spe.Init(0xC3, PACKET_MAGIC_ATTACK);
    spe << (BYTE)(HIBYTE(p_Type)) << (BYTE)(LOBYTE(p_Type)) << (BYTE)x << (BYTE)y << (BYTE)MakeSkillSerialNumber(&Serial) << (BYTE)Count;
    for (int i = 0; i < Count; i++)
    {
        spe << (BYTE)(Key[i] >> 8) << (BYTE)(Key[i] & 0xff);
        spe << (BYTE)SkillSerial;
    }
    spe.Send();

    g_ConsoleDebug->Write(MCD_SEND, L"0x1D [SendRequestMagicAttack(%d)]", Serial);
}
#endif //ENABLE_EDIT

//BYTE MakeSkillSerialNumber(BYTE* pSerialNumber);

extern int CurrentSkill;

inline BYTE GetDestValue(int xPos, int yPos, int xDst, int yDst)
{
    int DestX = xDst - xPos;
    int DestY = yDst - yPos;
    if (DestX < -8) DestX = -8;
    if (DestX > 7) DestX = 7;
    if (DestY < -8) DestY = -8;
    if (DestY > 7) DestY = 7;
    assert(-8 <= DestX && DestX <= 7);
    assert(-8 <= DestY && DestY <= 7);
    BYTE byValue1 = ((BYTE)(DestX + 8)) << 4;
    BYTE byValue2 = ((BYTE)(DestY + 8)) & 0xf;
    return (byValue1 | byValue2);
}

inline void SendRequestMagicContinue(int Type, int x, int y, int Angle, BYTE Dest, BYTE Tpos, WORD TKey,
    BYTE* pSkillSerial
)
{
    if (IsWebzenCharacter()) return;
    CurrentSkill = Type;

    SocketClient->ToGameServer()->SendAreaSkill(Type, x, y, Angle, TKey, MakeSkillSerialNumber(pSkillSerial));

    g_ConsoleDebug->Write(MCD_SEND, L"0x1E [SendRequestMagicContinue]");
}

__forceinline bool SendRequestEquipmentItem(STORAGE_TYPE iSrcType, int iSrcIndex, ITEM* pItem, STORAGE_TYPE iDstType, int iDstIndex)
{
    if (EquipmentItem || NULL == pItem) return false;

    EquipmentItem = true;

    BYTE splitType;
    if (pItem->option_380)
    {
        splitType = ((BYTE)(pItem->Type >> 5) & 240) | 0x08;
    }
    else
        splitType = ((BYTE)(pItem->Type >> 5) & 240);

    if (pItem->bPeriodItem == true)
    {
        splitType |= 0x02;
    }

    if (pItem->bExpiredPeriod == true)
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
        spareBits = (((BYTE)pItem->Jewel_Of_Harmony_Option) << 4) + ((BYTE)pItem->Jewel_Of_Harmony_OptionLevel);
    }

	const BYTE ItemData[12]
	{
	    BYTECAST(char, pItem->Type),
		BYTECAST(char, pItem->Level),
		BYTECAST(char, pItem->Durability),
		BYTECAST(char, pItem->Option1),
		BYTECAST(char, pItem->ExtOption),
		splitType,
		spareBits,
		pItem->bySocketOption[0], pItem->bySocketOption[1], pItem->bySocketOption[2], pItem->bySocketOption[3], pItem->bySocketOption[4]
	};

	SocketClient->ToGameServer()->SendItemMoveRequest((uint32_t)iSrcType, iSrcIndex, ItemData, sizeof ItemData, (uint32_t)iDstType, iDstIndex);

    g_ConsoleDebug->Write(MCD_SEND, L"0x24 [SendRequestEquipmentItem(%d %d %d %d %d %d %d)]", iSrcIndex, iDstIndex, iSrcType, iDstType, (pItem->Type & 0x1FFF), (BYTE)(pItem->Level), (BYTE)(pItem->Durability));

    return true;
}

inline void SendRequestUse(int Index, int Target)
{
    if (!IsCanUseItem())
    {
        g_pChatListBox->AddText(L"", GlobalText[474], SEASON3B::TYPE_ERROR_MESSAGE);
        return;
    }
    if (EnableUse > 0)
    {
        return;
    }

    EnableUse = 10;
    SocketClient->ToGameServer()->SendConsumeItemRequest(Index, Target, g_byItemUseType);
    g_ConsoleDebug->Write(MCD_SEND, L"0x26 [SendRequestUse(%d)]", Index);
}

inline void SendRequestGetItem(int itemKey)
{
    if(SendGetItem == -1)
    {
        SendGetItem = itemKey;
        SocketClient->ToGameServer()->SendPickupItemRequest(itemKey);
    }
}

inline void SendRequestDropItem(BYTE itemIndex, BYTE p_x, BYTE p_y)\
{
    SocketClient->ToGameServer()->SendDropItemRequest(p_x, p_y, itemIndex);
    SendDropItem = itemIndex;
}

#ifdef _PVP_ADD_MOVE_SCROLL
#define SendRequestCharacterEffect( p_key, p_type)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x48);\
	spe << ( BYTE)( p_key >> 8) << ( BYTE)( p_key & 0x00FF) << ( BYTE)( p_type);\
	spe.Send();\
}
#endif	// _PVP_ADD_MOVE_SCROLL

#define SendRequestClosePersonalShop(index, Id)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC3, 0x3F);\
	spe << (BYTE)0x07;\
	spe << (BYTE)((SHORT)index>>8) << (BYTE)((SHORT)index&0xFF);\
	spe.AddData(Id, MAX_ID_SIZE);\
	spe.Send();\
}

#define SendRequestInventory()\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC3, 0xF3);\
	spe << (BYTE)0x10;\
	spe.Send();\
}

#define SendRequestCRDisconnectRoom(pSocketClient)\
{\
	CStreamPacketEngine spe;\
	spe.Init( 0xC1, 0x01);\
	g_pSocketClient = pSocketClient;\
	spe.Send();\
	g_pSocketClient = SocketClient;\
}

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
__forceinline bool SendRequestEquippingInventoryItem(int iItemPos, int iValue)
{
    CStreamPacketEngine spe;
    spe.Init(0xC1, 0xBF);
    spe << (BYTE)0x20;
    spe << (BYTE)iItemPos;
    spe << (BYTE)iValue;
    spe.Send();

#ifdef CONSOLE_DEBUG
    g_ConsoleDebug->Write(MCD_SEND, L"0xBF, 0x20 [SendRequestEquippingInventoryItem(%d, %d)]", iItemPos, iValue);
#endif // CONSOLE_DEBUG
    return true;
}
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
