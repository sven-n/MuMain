// GuardianStatue.h: interface for the CGuardianStatue class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#if(GAMESERVER_TYPE==1)

class CGuardianStatue
{
public:
	CGuardianStatue();
	virtual ~CGuardianStatue();
	void GuardianStatueAct(int aIndex);
};

extern CGuardianStatue gGuardianStatue;

#endif
