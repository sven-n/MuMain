// KeyGenerater.cpp: implementation of the CKeyGenerater class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KeyGenerater.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL

CKeyGenerater g_KeyGenerater;

#ifdef USE_MAPMOVE_KEY_GENERATOR_SERVER_SIDE
#include "LargeRand.h"
#endif // USE_MAPMOVE_KEY_GENERATOR_SERVER_SIDE

static int KEY_GENERATE_FILTER[MAX_KEY_GENERATER_FILTER][4] = 
{
	{ 321,	37531879,	8734,	32 },		// 0
	{ 873,	64374332,	3546,	87 },
	{ 537,	24798765,	5798,	32 },
	{ 654,	32498765,	3573,	73 },
	{ 546,	98465432,	6459,	12 },		// 4
	{ 987,	24654876,	5616,	54 },
	{ 357,	34599876,	8764,	98 },
	{ 665,	78641332,	6547,	54 },
	{ 813,	85132165,	8421,	98 },
	{ 454,	57684216,	6875,	45 }
};

CKeyGenerater::CKeyGenerater()
{

}

CKeyGenerater::~CKeyGenerater()
{

}

#ifdef USE_MAPMOVE_KEY_GENERATOR_SERVER_SIDE
// 1. 키 값 생성 관련
// 1-1. 초기의 시드값을 생성한다.
DWORD CKeyGenerater::GenerateSeedValue()
{
	return GetLargeRand();
}
#endif // USE_MAPMOVE_KEY_GENERATOR_SERVER_SIDE

// 1-2. 키값으로 다음 키 값을 생성한다.
DWORD CKeyGenerater::GenerateKeyValue(DWORD dwKeyValue)
{
	DWORD dwRegenerateKeyValue = 0;
	BYTE btNumericValue = 0;

	// 고정 연산 과정을 통해 다음 키 값을 생성해낸다.
	// *, /, +, -아 %연산 모두를 사용한다.
	// 복잡도를 증가시키기 위해 키값의 mod 연산을 통해 10개의 필터로 값을 분배한다.
	btNumericValue = dwKeyValue % MAX_KEY_GENERATER_FILTER;

#ifdef KJH_FIX_MOVE_MAP_GENERATE_KEY
	dwRegenerateKeyValue = ( ( dwKeyValue % KEY_GENERATE_FILTER[btNumericValue][2] ) * KEY_GENERATE_FILTER[btNumericValue][0] ) + KEY_GENERATE_FILTER[btNumericValue][1] - KEY_GENERATE_FILTER[btNumericValue][3];
#else // KJH_FIX_MOVE_MAP_GENERATE_KEY
	dwRegenerateKeyValue = dwKeyValue * KEY_GENERATE_FILTER[btNumericValue][0] + KEY_GENERATE_FILTER[btNumericValue][1] - KEY_GENERATE_FILTER[btNumericValue][2] / KEY_GENERATE_FILTER[btNumericValue][3];
#endif // KJH_FIX_MOVE_MAP_GENERATE_KEY

	return dwRegenerateKeyValue;
}

// 1-3. 입력된 키 값다 다음 키 값을 비교한다.
bool CKeyGenerater::CheckKeyValue(DWORD* dwOldKeyValue, DWORD dwReceiveKeyValue)
{
	// 클라이언트로부터 받은 키 값과 기존에 저장하고 있던 키 값을 비교한다.
	DWORD dwGeneratedKeyValue = 0;

	dwGeneratedKeyValue = GenerateKeyValue( *dwOldKeyValue );
	if( dwReceiveKeyValue == dwGeneratedKeyValue )
	{
		*dwOldKeyValue = dwGeneratedKeyValue;
		return true;
	}

	return false;
}

#ifdef USE_MAPMOVE_KEY_GENERATOR_SERVER_SIDE
void CKeyGenerater::_SimulationKeyGenerate()
{
	DWORD dwKeyValue = 0;

	dwKeyValue = GenerateSeedValue();

	for( int i = 0 ; i < 10000 ; i++ )
	{
		dwKeyValue = GenerateKeyValue( dwKeyValue );
	}
}
#endif // USE_MAPMOVE_KEY_GENERATOR_SERVER_SIDE

#endif // YDG_ADD_MOVE_COMMAND_PROTOCOL