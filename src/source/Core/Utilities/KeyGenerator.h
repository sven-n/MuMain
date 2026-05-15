// KeyGenerator.h: interface for the CKeyGenerator class.
//////////////////////////////////////////////////////////////////////
#pragma once

CONST int MAX_KEY_GENERATOR_FILTER = 10;

class CKeyGenerator
{
public:
    CKeyGenerator();
    virtual ~CKeyGenerator();
    DWORD		GenerateKeyValue(DWORD dwKeyValue);
    bool		CheckKeyValue(DWORD* dwOldKeyValue, DWORD dwReceiveKeyValue);

};

extern CKeyGenerator g_KeyGenerator;
