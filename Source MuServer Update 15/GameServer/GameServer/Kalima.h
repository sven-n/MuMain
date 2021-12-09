// Kalima.h: interface for the CKalima class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

#define MAX_KALIMA_LEVEL 7
#define MAX_KALIMA_ENTER 5
#define MAX_KALIMA_STAND 180000
#define MAX_KALIMA_NEXT_STAND 60000

#define KALIMA_ATTRIBUTE_RANGE(x) (((x)<51)?0:((x)>58)?0:1)

class CKalima
{
public:
	CKalima();
	virtual ~CKalima();
	int GetKalimaGateLevel(LPOBJ lpObj);
	bool CreateKalimaGate(int aIndex,int level,int x,int y);
	bool CreateNextKalimaGate(int aIndex,int map,int x,int y);
	bool DeleteKalimaGate(int aIndex);
	void KalimaGateAct(int aIndex);
	void KalimaNextGateAct(int aIndex);
private:
	CRITICAL_SECTION m_critical;
};

extern CKalima gKalima;
