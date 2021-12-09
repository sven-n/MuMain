// LifeStone.h: interface for the CLifeStone class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CLifeStone
{
public:
	CLifeStone();
	virtual ~CLifeStone();
	bool CreateLifeStone(int aIndex);
	bool DeleteLifeStone(int aIndex);
	bool SetReSpawnUserXY(int aIndex);
	void LifeStoneAct(int aIndex);
};

extern CLifeStone gLifeStone;
