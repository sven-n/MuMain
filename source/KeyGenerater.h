// KeyGenerater.h: interface for the CKeyGenerater class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KEYGENERATER_H__EC134DC8_2BD1_4EC9_8F3B_09C85AD67F00__INCLUDED_)
#define AFX_KEYGENERATER_H__EC134DC8_2BD1_4EC9_8F3B_09C85AD67F00__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL

CONST int MAX_KEY_GENERATER_FILTER	= 10;

class CKeyGenerater  
{
public:
	CKeyGenerater();
	virtual ~CKeyGenerater();

	// 1. 키 값 생성 관련
#ifdef USE_MAPMOVE_KEY_GENERATOR_SERVER_SIDE
	// 1-1. 초기의 시드값을 생성한다.
	DWORD		GenerateSeedValue();
#endif // USE_MAPMOVE_KEY_GENERATOR_SERVER_SIDE
	// 1-2. 키값으로 다음 키 값을 생성한다.
	DWORD		GenerateKeyValue(DWORD dwKeyValue);
	// 1-3. 입력된 키 값다 다음 키 값을 비교한다. (서버 전용)
	bool		CheckKeyValue(DWORD* dwOldKeyValue, DWORD dwReceiveKeyValue);

#ifdef USE_MAPMOVE_KEY_GENERATOR_SERVER_SIDE
	void		_SimulationKeyGenerate();
#endif // USE_MAPMOVE_KEY_GENERATOR_SERVER_SIDE
};

extern CKeyGenerater g_KeyGenerater;

#endif // YDG_ADD_MOVE_COMMAND_PROTOCOL

#endif // !defined(AFX_KEYGENERATER_H__EC134DC8_2BD1_4EC9_8F3B_09C85AD67F00__INCLUDED_)