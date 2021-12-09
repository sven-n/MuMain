// CkDhProgress.h: interface for the CkDhProgress class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKDHPROGRESS_H
#define _CKDHPROGRESS_H

#pragma once

#pragma pack (push, 8) 

class CkDhProgress  
{
    public:
	CkDhProgress() { }
	virtual ~CkDhProgress() { }

	// Called periodically to check to see if the method call should be aborted.
	virtual void AbortCheck(bool *abort) { }

};
#pragma pack (pop)

#endif
