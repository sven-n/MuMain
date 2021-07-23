// PCRoomPoint.h: interface for the CPCRoomPtSys class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PCROOMPOINT_H__6D2A3C64_DA4F_432E_86EB_A6AF35A8BFF6__INCLUDED_)
#define AFX_PCROOMPOINT_H__6D2A3C64_DA4F_432E_86EB_A6AF35A8BFF6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef ADD_PCROOM_POINT_SYSTEM

class CPCRoomPtSys  
{
protected:
	bool	m_bPCRoom;			// PC방 여부.
#ifndef KJH_DEL_PC_ROOM_SYSTEM				// #ifndef
	short	m_nNowPt;			// 현재 포인트.
	short	m_nMaxPt;			// 최대 포인트.
	
	bool	m_bPCRoomPtShopMode;// true면 상점이 PC룸 포인트 상점 모드임.
	BYTE	m_byBuyItemPos;		// 구입할 아이템 위치.
#endif // KJH_DEL_PC_ROOM_SYSTEM

protected:
	CPCRoomPtSys();

public:
	virtual ~CPCRoomPtSys();

	static CPCRoomPtSys& Instance();

	// PC방 여부 세팅.
	void SetPCRoom(bool bPCRoom = true) { m_bPCRoom = bPCRoom; }
	bool IsPCRoom() { return m_bPCRoom; }		// PC방인가?

#ifndef KJH_DEL_PC_ROOM_SYSTEM			// # ifndef
	void SetPoint(short nNowPt, short nMaxPt)	// PC방 포인트 세팅.
	{ m_nNowPt = nNowPt; m_nMaxPt = nMaxPt; }
	short GetNowPoint() { return m_nNowPt; }	// PC방 현재 포인트 얻기.
	short GetMaxPoint() { return m_nMaxPt; }	// PC방 최대 포인트 얻기.

	// PC방 포인트 상점 여부 세팅.
	void SetPCRoomPointShopMode(bool bMode = true)
	{ m_bPCRoomPtShopMode = bMode; }
	bool IsPCRoomPointShopMode()				// PC방 포인트 상점인가?
	{
#ifdef CSK_FIX_PCROOM_POINT_ZEN_BUG
		return m_bPCRoomPtShopMode;
#else // CSK_FIX_PCROOM_POINT_ZEN_BUG
		if (m_bPCRoom)
			return m_bPCRoomPtShopMode;
		return false;
#endif // CSK_FIX_PCROOM_POINT_ZEN_BUG
	}

	// PC방 포인트 상점에서 구입할 아이템의 위치 임시 저장.
	void SetBuyItemPos(BYTE	byItemPos) { m_byBuyItemPos = byItemPos; }
	// PC방 포인트 상점에서 구입할 아이템 위치 임시 저장 값 얻기.
	BYTE GetBuyItemPos() { return m_byBuyItemPos; }
#endif // KJH_DEL_PC_ROOM_SYSTEM
};

#endif // ADD_PCROOM_POINT_SYSTEM
#endif // !defined(AFX_PCROOMPOINT_H__6D2A3C64_DA4F_432E_86EB_A6AF35A8BFF6__INCLUDED_)
