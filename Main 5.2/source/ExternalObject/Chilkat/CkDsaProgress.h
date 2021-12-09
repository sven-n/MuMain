// CkDsaProgress.h: interface for the CkDsaProgress class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKDSAPROGRESS_H
#define _CKDSAPROGRESS_H

#pragma once

#pragma pack (push, 8) 

class CkDsaProgress  
{
    public:
	CkDsaProgress() { }
	virtual ~CkDsaProgress() { }

	// Called periodically to check to see if the method call should be aborted.
	virtual void AbortCheck(bool *abort) { }

};
#pragma pack (pop)

#endif
