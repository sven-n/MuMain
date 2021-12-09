// Mercenary.h: interface for the CMercenary class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

class CMercenary
{
public:
	CMercenary();
	virtual ~CMercenary();
	bool CreateMercenary(int aIndex,int MonsterClass,BYTE x,BYTE y);
	bool DeleteMercenary(int aIndex);
	bool SearchEnemy(LPOBJ lpObj);
	void MercenaryAct(int aIndex);
private:
	int m_MercenaryCount;
};

extern CMercenary gMercenary;
