// CkImapProgress.h: interface for the CkImapProgress class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKIMAPPROGRESS_H
#define _CKIMAPPROGRESS_H

#pragma once

#pragma pack (push, 8) 

class CkImapProgress  
{
    public:
	CkImapProgress() { }
	virtual ~CkImapProgress() { }

	// Called periodically to check to see if the method call should be aborted.
	virtual void AbortCheck(bool *abort) { }
	virtual void PercentDone(int pctDone, bool *abort) { }

};
#pragma pack (pop)

#endif
