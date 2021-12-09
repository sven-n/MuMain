// BlackWin.h: interface for the CBlackWin class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Win.h"

class CBlackWin : public CWin  
{
protected:
	double	m_dFadeDeltaTickSum;

public:
	CBlackWin();
	virtual ~CBlackWin();

	void Create();
	bool CursorInWin(int nArea);

protected:
	void UpdateWhileActive(double dDeltaTick);
};
