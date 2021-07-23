// CkSFtpProgress.h: interface for the CkSFtpProgress class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKSFTPPROGRESS_H
#define _CKSFTPPROGRESS_H

#pragma once

#pragma pack (push, 8) 

class CkSFtpProgress  
{
    public:
	CkSFtpProgress() { }
	virtual ~CkSFtpProgress() { }

	// Called periodically to check to see if the method call should be aborted.
	virtual void AbortCheck(bool *abort) { }
	virtual void PercentDone(int pctDone, bool *abort) { }

	virtual void UploadRate(unsigned long byteCount, unsigned long bytesPerSec) { }
	virtual void DownloadRate(unsigned long byteCount, unsigned long bytesPerSec) { }

};
#pragma pack (pop)

#endif
