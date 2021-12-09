// KeyGenerater.h: interface for the CKeyGenerater class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KEYGENERATER_H__EC134DC8_2BD1_4EC9_8F3B_09C85AD67F00__INCLUDED_)
#define AFX_KEYGENERATER_H__EC134DC8_2BD1_4EC9_8F3B_09C85AD67F00__INCLUDED_

#pragma once

#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL

CONST int MAX_KEY_GENERATER_FILTER	= 10;

class CKeyGenerater  
{
public:
	CKeyGenerater();
	virtual ~CKeyGenerater();

#ifdef USE_MAPMOVE_KEY_GENERATOR_SERVER_SIDE
	DWORD		GenerateSeedValue();
#endif // USE_MAPMOVE_KEY_GENERATOR_SERVER_SIDE
	DWORD		GenerateKeyValue(DWORD dwKeyValue);
	bool		CheckKeyValue(DWORD* dwOldKeyValue, DWORD dwReceiveKeyValue);

#ifdef USE_MAPMOVE_KEY_GENERATOR_SERVER_SIDE
	void		_SimulationKeyGenerate();
#endif // USE_MAPMOVE_KEY_GENERATOR_SERVER_SIDE
};

extern CKeyGenerater g_KeyGenerater;

#endif // YDG_ADD_MOVE_COMMAND_PROTOCOL

#endif // !defined(AFX_KEYGENERATER_H__EC134DC8_2BD1_4EC9_8F3B_09C85AD67F00__INCLUDED_)