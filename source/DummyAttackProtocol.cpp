// DummyAttackProtocol.cpp: implementation of the CDummyAttackProtocol class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef KJH_ADD_DUMMY_SKILL_PROTOCOL

#include "DummyAttackProtocol.h"
#include "KeyGenerater.h"
//#include "LargeRand.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CDummyAttackProtocol::CDummyAttackProtocol()
{
#ifdef KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	m_dwDummySeedValue			= 0;
	m_dwAttackSerialSeedValue	= 0;
#else // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	m_dwSeedValue = 0;
#endif // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	m_iSkillCount = 0;
}

CDummyAttackProtocol::~CDummyAttackProtocol()
{

}

CDummyAttackProtocol* CDummyAttackProtocol::GetInstance()
{
	static CDummyAttackProtocol s_DummyAttackChecker;
	return &s_DummyAttackChecker;
}

// 1. 초기화
// 1-1. 초기화
#ifdef KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
void CDummyAttackProtocol::Initialize(DWORD dwDummySeedValue, DWORD dwAttackSerialSeedValue)
{
	m_dwDummySeedValue			= dwDummySeedValue;
	m_dwAttackSerialSeedValue	= dwAttackSerialSeedValue;
#else // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
void CDummyAttackProtocol::Initialize(DWORD dwSeedValue)
{
	m_dwSeedValue = dwSeedValue;
#endif // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
#ifdef KJH_FIX_DUMMY_SKILL_PROTOCOL_BUG
	m_iSkillCount = 0;
#endif // KJH_FIX_DUMMY_SKILL_PROTOCOL_BUG
}

// 2-2. 다음 시드값을 생성한다.
#ifndef KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL						// #ifndef
void CDummyAttackProtocol::GenerateNextSeedValue()
{
	DWORD dwBeforeSeedValue = 0;
	DWORD dwNewSeedValue = 0;

	dwBeforeSeedValue = m_dwSeedValue;

	// 시드값을 가지고 새로운 시드값을 계산한다.
	Initialize( g_KeyGenerater.GenerateKeyValue( dwBeforeSeedValue ) );
}
#endif // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL

WORD CDummyAttackProtocol::GetDummyProtocolNextSeq()
{
	WORD wDummyProtocolSeq = 0;
	BYTE btModeValue = 0;

	// 우선 더미 프로토콜이 올 순서인지 확인한다.

	// 기존 시드값의 mod연산이 0일 경우를 대비하여 숫자를 더해준다.
#ifdef KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	wDummyProtocolSeq = m_dwDummySeedValue % 100;
	wDummyProtocolSeq += ( m_dwDummySeedValue % 77 ) + 23;
#else // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
#ifdef KJH_FIX_DUMMY_SKILL_PROTOCOL_BUG
	wDummyProtocolSeq = m_dwSeedValue % 100;
	wDummyProtocolSeq += ( m_dwSeedValue % 77 ) + 23;
#else // KJH_FIX_DUMMY_SKILL_PROTOCOL_BUG
	btModeValue = m_dwSeedValue % 100;
	btModeValue += ( m_dwSeedValue % 50 ) + 10;

	wDummyProtocolSeq = m_dwSeedValue % btModeValue;
#endif // KJH_FIX_DUMMY_SKILL_PROTOCOL_BUG
#endif // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL

	return wDummyProtocolSeq;
}

// 3-4. 더미 프로토콜을 생성한다.
void CDummyAttackProtocol::MakeDummyProtocol(LPBYTE lpProtocol)
{
	WORD wDummyProtocol[2] = {0,};
	BYTE btDummyProtocol[4] = {0,};

#ifdef KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	wDummyProtocol[0] = LOWORD( m_dwDummySeedValue );
	wDummyProtocol[1] = HIWORD( m_dwDummySeedValue );
#else // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	wDummyProtocol[0] = LOWORD( m_dwSeedValue );
	wDummyProtocol[1] = HIWORD( m_dwSeedValue );
#endif // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	btDummyProtocol[0] = LOBYTE( wDummyProtocol[0] );
	btDummyProtocol[1] = HIBYTE( wDummyProtocol[0] );
	btDummyProtocol[2] = LOBYTE( wDummyProtocol[1] );
	btDummyProtocol[3] = HIBYTE( wDummyProtocol[1] );

	memcpy( lpProtocol, btDummyProtocol, 4 );

#ifdef KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	m_dwDummySeedValue = g_KeyGenerater.GenerateKeyValue( m_dwDummySeedValue );
#else // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	GenerateNextSeedValue();
#endif // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
}

#ifdef KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
BYTE CDummyAttackProtocol::GetSerial()
{
	BYTE btSerialValue = 0;
	
	btSerialValue = m_dwAttackSerialSeedValue % 149;
	btSerialValue += ( m_dwAttackSerialSeedValue % 64 ) + 16;

	// 다음 시리얼 시드값 생성
	m_dwAttackSerialSeedValue = g_KeyGenerater.GenerateKeyValue( m_dwAttackSerialSeedValue );
	
	return btSerialValue;
}
#endif // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL

void CDummyAttackProtocol::AddSkillCount()
{
	m_iSkillCount++;
}

void CDummyAttackProtocol::InitSkillCount()
{
	m_iSkillCount = 0;
}

int CDummyAttackProtocol::GetSkillCount()
{
	return m_iSkillCount;
}

#endif // KJH_ADD_DUMMY_SKILL_PROTOCOL
