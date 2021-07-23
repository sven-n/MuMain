// DummyAttackProtocol.h: interface for the CDummyAttackProtocol class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DUMMYATTACKPROTOCOL_H__9CB5AB50_D2E6_46AB_8B37_B261286D5FE6__INCLUDED_)
#define AFX_DUMMYATTACKPROTOCOL_H__9CB5AB50_D2E6_46AB_8B37_B261286D5FE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef KJH_ADD_DUMMY_SKILL_PROTOCOL

#include "WSclient.h"

enum
{
	PROTOCOL_TYPE_ATTACK_NORMAL			= 1,
	PROTOCOL_TYPE_ATTACK_SKILL			= 2,
};

CONST int MAX_DUMMY_PROTOCOL_ERROR_COUNT	= 3;


class CDummyAttackProtocol  
{
protected:
	CDummyAttackProtocol();
public:
	virtual ~CDummyAttackProtocol();

	//////////////////////////////////////////////////////////////////////////
	//
	// 1. 서버에서는 시드값을 발급하고 시드값을 서버로 전송한다. (이 순서는 역으로 설정해도 상관없음)
	// 2. 클라이언트는 시드값으로 다음 더미 프로토콜을 보낼 순서를 확인하고 해당 순서에 더미 프로토콜을 전송한다. (GetDummyProtocolNextSeq())
	// 3. 더미 프로토콜은 일반 공격 프로토콜과 같지만 값을 다르게 보낸다.
	// 4. VerifyDummyProtocol로 해당 프로토콜이 더미 프로토콜인지 확인한다.
	// 5. 클라이언트는 MakeDummyProtocol()을 더미프로토콜을 보내야 할 시기에 서버에 전송한다.
	//
	// - 클라이언트 사용 방법 -
	// 1. 서버에서 PMSG_SET_ATTACK_PROTOCOL_DUMMY_VALUE이 전송되면 해당 프로토콜의 dwDummySeedValue값을 저장한다.
	// 2. GetDummyProtocolNextSeq()함수를 통해 더미 프로토콜을 보낼 순서를 결정한다. ( 파라미터는 dwDummySeedValue 값 )
	// 3. 더미 프로토콜은 MakeDummyProtocol()함수를 통해 생성된 더미 프로토콜을 공격 프로토콜의 header를 제외한 4BYTE에 입력하여 서버에 전송한다.
	// 
	//////////////////////////////////////////////////////////////////////////

public:
	static CDummyAttackProtocol* GetInstance();

	// 1. 초기화
	// 1-1. 초기화
#ifdef KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	void	Initialize(DWORD dwDummySeedValue, DWORD dwAttackSerialSeedValue);			// 1. 서버로부터 받는 SeedValue
#else // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	void	Initialize(DWORD dwSeedValue);			// 1. 서버로부터 받는 SeedValue
#endif // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL

	// 3. 더미 프로토콜 관련
	// 3-1. 다음 더미 프로토콜을 보낼 순서를 결정한다.
	WORD	GetDummyProtocolNextSeq();

	// 3-4. 더미 프로토콜을 생성한다.
	void	MakeDummyProtocol(LPBYTE lpProtocol);

#ifdef KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	// 4. 공격 프로토콜 시리얼 관련
	// 4-1. 이전 시리얼 값으로 시리얼을 생성한다.
	BYTE	GetSerial();
#endif // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL

	void	AddSkillCount();
	int		GetSkillCount();
	void	InitSkillCount();

private:
	// 2-2. 다음 시드값을 생성한다.
#ifndef KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL		// #ifndef
	void	GenerateNextSeedValue();
#endif // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL

private:
#ifdef KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	DWORD	m_dwDummySeedValue;
	DWORD	m_dwAttackSerialSeedValue;
#else // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	DWORD	m_dwSeedValue;
#endif // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	int		m_iSkillCount;
};

#define g_DummyAttackChecker CDummyAttackProtocol::GetInstance()

#endif // KJH_ADD_DUMMY_SKILL_PROTOCOL

#endif // !defined(AFX_DUMMYATTACKPROTOCOL_H__9CB5AB50_D2E6_46AB_8B37_B261286D5FE6__INCLUDED_)
