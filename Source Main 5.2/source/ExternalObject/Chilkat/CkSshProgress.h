// CkSshProgress.h: interface for the CkSshProgress class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKSshPROGRESS_H
#define _CKSshPROGRESS_H

#pragma once

#pragma pack (push, 8) 

class CkSshProgress  
{
    public:
	CkSshProgress() { }
	virtual ~CkSshProgress() { }

	// Called periodically to check to see if the method call should be aborted.
	virtual void AbortCheck(bool *abort) { }
	virtual void PercentDone(int pctDone, bool *abort) { }

};
#pragma pack (pop)

#endif
