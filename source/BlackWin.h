// BlackWin.h: interface for the CBlackWin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BLACKWIN_H__C8CD4546_6A49_4ABD_BA54_A5DADEFD65FC__INCLUDED_)
#define AFX_BLACKWIN_H__C8CD4546_6A49_4ABD_BA54_A5DADEFD65FC__INCLUDED_

#pragma once

#include "Win.h"

class CBlackWin : public CWin  
{
protected:
	double	m_dFadeDeltaTickSum;	// »Â∏• Ω√∞£.

public:
	CBlackWin();
	virtual ~CBlackWin();

	void Create();
	bool CursorInWin(int nArea);

protected:
	void UpdateWhileActive(double dDeltaTick);
};


#endif // !defined(AFX_BLACKWIN_H__C8CD4546_6A49_4ABD_BA54_A5DADEFD65FC__INCLUDED_)
