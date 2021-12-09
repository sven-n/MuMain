// CharacterManager.h: interface for the CCharacterManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "CriticalSection.h"

struct CHARACTER_INFO
{
	char Name[11];
	char Account[11];
	WORD UserIndex;
	WORD GameServerCode;
};

class CCharacterManager
{
public:
	CCharacterManager();
	virtual ~CCharacterManager();
	void ClearServerCharacterInfo(WORD ServerCode);
	bool GetCharacterInfo(CHARACTER_INFO* lpCharacterInfo,char* name);
	void InsertCharacterInfo(CHARACTER_INFO CharacterInfo);
	void RemoveCharacterInfo(CHARACTER_INFO CharacterInfo);
	long GetCharacterCount();
private:
	CCriticalSection m_critical;
	std::map<std::string,CHARACTER_INFO> m_CharacterInfo;
};

extern CCharacterManager gCharacterManager;
