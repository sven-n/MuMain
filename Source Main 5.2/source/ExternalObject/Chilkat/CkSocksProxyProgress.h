// CkSocksProxyProgress.h: interface for the CkSocksProxyProgress class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKSOCKSPROXYPROGRESS_H
#define _CKSOCKSPROXYPROGRESS_H

#pragma once

#pragma pack (push, 8) 

class CkSocksProxyProgress  
{
    public:
	CkSocksProxyProgress() { }
	virtual ~CkSocksProxyProgress() { }

	// Called periodically to check to see if the method call should be aborted.
	virtual void AbortCheck(bool *abort) { }

};
#pragma pack (pop)

#endif
