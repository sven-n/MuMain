// CkRssProgress.h: interface for the CkRssProgress class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKRssPROGRESS_H
#define _CKRssPROGRESS_H

#pragma once

#pragma pack (push, 8) 

class CkRssProgress  
{
    public:
	CkRssProgress() { }
	virtual ~CkRssProgress() { }

	// Called periodically to check to see if the method call should be aborted.
	virtual void AbortCheck(bool *abort) { }

};
#pragma pack (pop)

#endif
