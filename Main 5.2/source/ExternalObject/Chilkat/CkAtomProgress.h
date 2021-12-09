// CkAtomProgress.h: interface for the CkAtomProgress class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKAtomPROGRESS_H
#define _CKAtomPROGRESS_H

#pragma once

#pragma pack (push, 8) 

class CkAtomProgress  
{
    public:
	CkAtomProgress() { }
	virtual ~CkAtomProgress() { }

	// Called periodically to check to see if the method call should be aborted.
	virtual void AbortCheck(bool *abort) { }

};
#pragma pack (pop)

#endif
