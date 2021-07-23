// CkTarProgress.h: interface for the CkTarProgress class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKTARROGRESS_H
#define _CKTARPROGRESS_H

#pragma once

#ifndef WIN32
#include "int64.h"
#endif

#pragma pack (push, 8) 

class CkTarProgress  
{
    public:
	CkTarProgress() { }
	virtual ~CkTarProgress() { }

	// Called periodically to check to see if the method call should be aborted.
	virtual void AbortCheck(bool *abort) { }

	// Called just before appending to TAR when writing a .tar, and just before
	// extracting during untar.
	virtual void NextTarFile(const char *fileName, 
	    __int64 fileSize,
	    bool bIsDirectory, 
	    bool *skip) { }

};
#pragma pack (pop)

#endif
