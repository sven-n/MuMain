// CkMailProgress.h: interface for the CkMailProgress class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CkMailProgress_H
#define _CkMailProgress_H

#pragma once

/*

  To receive progress events (callbacks), create a C++ class that 
  inherits this one and provides one or more overriding implementations 
  for the events you wish to receive.  

  */
#pragma pack (push, 8) 


class CkMailProgress  
{
    public:
	CkMailProgress();
	virtual ~CkMailProgress();

	virtual void SendPercentDone(long pctDone, bool *abort) { }
	virtual void ReadPercentDone(long pctDone, bool *abort) { }

	// Called periodically to check to see if the email sending/receiving should be aborted.
	virtual void AbortCheck(bool *abort) { }

	virtual void EmailReceived(const char *subject, 
			    const char *fromAddr, const char *fromName, 
			    const char *returnPath, 
			    const char *date, 
			    const char *uidl, 
			    int sizeInBytes) { }

};
#pragma pack (pop)

#endif
