// KeyGenerater.h: interface for the CKeyGenerater class.
//////////////////////////////////////////////////////////////////////
#pragma once

CONST int MAX_KEY_GENERATER_FILTER = 10;

class CKeyGenerater
{
public:
    CKeyGenerater();
    virtual ~CKeyGenerater();
    DWORD		GenerateKeyValue(DWORD dwKeyValue);
    bool		CheckKeyValue(DWORD* dwOldKeyValue, DWORD dwReceiveKeyValue);

};

extern CKeyGenerater g_KeyGenerater;
