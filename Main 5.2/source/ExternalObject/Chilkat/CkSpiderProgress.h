// CkSpiderProgress.h: interface for the CkSpiderProgress class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKSPIDERPROGRESS_H
#define _CKSPIDERPROGRESS_H

#pragma once

class CkSpiderProgress  
{
    public:
	CkSpiderProgress();
	virtual ~CkSpiderProgress();

	// Called periodically to check to see if the method call should be aborted.
	virtual void AbortCheck(bool *abort) { }

};

#endif
