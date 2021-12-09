// CkDkimProgress.h: interface for the CkDkimProgress class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKDKIMPROGRESS_H
#define _CKDKIMPROGRESS_H

#pragma once

#pragma pack (push, 8) 

class CkDkimProgress  
{
    public:
	CkDkimProgress() { }
	virtual ~CkDkimProgress() { }

	// Called periodically to check to see if the method call should be aborted.
	virtual void AbortCheck(bool *abort) { }

};
#pragma pack (pop)

#endif
