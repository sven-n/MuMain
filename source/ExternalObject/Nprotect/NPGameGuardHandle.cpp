// NPGameGuardHandle.cpp: implementation of the CNPGameGuardHandle class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#ifdef PBG_MOD_GAMEGUARD_HANDLE
#include "NPGameGuardHandle.h"
#include "npGameGuard.h"
#include "wsclientinline.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNPGameGuardHandle::CNPGameGuardHandle() : m_iTime(1000)
{
	Init();
}

CNPGameGuardHandle::~CNPGameGuardHandle()
{
	Destroy();
}

void CNPGameGuardHandle::Init()
{
	// 기준 30초를 경과 하는것으로 한다
	m_nStandardTime = 30;
	// +랜덤10초를 한다
	m_nRandTime = rand()%10;
	m_dwNPGameCurrTime = 0;

	// 캐릭터의 정보를 저장
	memset(m_szCharName, 0, sizeof(char)*MAX_TEXT);
	memset(m_szHackName, 0, sizeof(char)*MAX_TEXT);

	// 핵사용을 하는지여부
	m_bHack = false;
	// 종료 메시지 박스
	m_bDestroyMsg =false;
	// 게임가드 에러 메시지인덱스
	m_dwErrorMsgIndex =0;
}

void CNPGameGuardHandle::Destroy()
{

}

void CNPGameGuardHandle::SetNPHack(BOOL _IsHack)
{
	m_bHack = _IsHack;

	if(m_bHack)
		m_dwNPGameCurrTime=0;	// 핵을 사용하고 있다면 카운트 시작
}

BOOL CNPGameGuardHandle::IsNPHack()
{
	// 핵을 사용하고 있는지
	return m_bHack;	
}

void CNPGameGuardHandle::SetCharacterInfo(char* _Name, char* _Hack)
{
	// 캐릭터 정보
	memcpy(m_szCharName, _Name, strlen(_Name)+1);
	memcpy(m_szHackName, _Name, strlen(_Hack)+1);
}

char* CNPGameGuardHandle::GetCharacterName()
{
	return m_szCharName;
}

char* CNPGameGuardHandle::GetCharacterHack()
{
	return m_szHackName;
}

bool CNPGameGuardHandle::CheckTime()
{
	timeBeginPeriod(1);

	static DWORD _CurrTick = timeGetTime();
	static DWORD _OldTick = 0;
	static DWORD _Cumulative = 0;

	DWORD _CurrentTimeTemp = timeGetTime();

	_CurrTick = _CurrentTimeTemp - _CurrTick;
	_Cumulative = _CurrTick + _OldTick;

	// 초단위로만 검사한다
	if(_Cumulative >= m_iTime)
	{
		_OldTick = _Cumulative - m_iTime;	
		_CurrTick = _CurrentTimeTemp;

		m_dwNPGameCurrTime++;
		return false;
	}

	_OldTick = _Cumulative;			
	_CurrTick = _CurrentTimeTemp;
	return true;

	timeEndPeriod(1);
}

void CNPGameGuardHandle::SetDestroyMsg(bool _bDestroyMsg)
{
	m_bDestroyMsg = _bDestroyMsg;
}

BOOL CNPGameGuardHandle::GetDestroyMsg()
{
	return m_bDestroyMsg;
}

void CNPGameGuardHandle::SetErrorMsgIndex(DWORD _nMsgIndex)
{
	m_dwErrorMsgIndex = _nMsgIndex;
}

DWORD CNPGameGuardHandle::GetErrorMsgIndex()
{
	return m_dwErrorMsgIndex;
}

bool CNPGameGuardHandle::CheckDestroyWin()
{
	if(IsNPHack())
	{
		// 캐릭터 선택창 이상 시간이 나가야 한다
		if(SceneFlag <= CHARACTER_SCENE)
			return false;

		// 기준 시간+랜덤 시간 체크를 넘으면 종료
		if(m_dwNPGameCurrTime >= m_nStandardTime + m_nRandTime)
		{
			DWORD CurrTick = timeGetTime();
			SendRequestNpDisconnect(CurrTick);
			return true;
		}
	}
	return false;
}

CNPGameGuardHandle* CNPGameGuardHandle::GetInstance()
{
	static CNPGameGuardHandle sInstance;
	return &sInstance;
}
#endif //PBG_MOD_GAMEGUARD_HANDLE
