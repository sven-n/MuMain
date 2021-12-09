// CannonTower.h: interface for the CCannonTower class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CCannonTower
{
public:
	CCannonTower();
	virtual ~CCannonTower();
	void CannonTowerAct(int aIndex);
};

extern CCannonTower gCannonTower;
