// CkFtpProgress.h: interface for the CkFtpProgress class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKFTPPROGRESS_H
#define _CKFTPPROGRESS_H

#pragma once

/*

  To receive progress events (callbacks), create a C++ class that 
  inherits this one and provides one or more overriding implementations 
  for the events you wish to receive.  Then pass your progress object
  to CkFtp2 methods such as PutFileWithProgress, GetFileWithProgress, etc.

  */
#pragma pack (push, 8) 

class CkFtpProgress  
{
    public:
	CkFtpProgress();
	virtual ~CkFtpProgress();

	virtual void FtpPercentDone(long pctDone, bool *abort) { }

	// Called periodically to check to see if the transfer should be aborted.
	virtual void AbortCheck(bool *abort) { }

	// Called to notify the application of the port number of the data connection
	// being used for Active connections (as opposed to Passive connections).
	// For Active data transfers, the client creates a data socket and chooses
	// a random port number between 1024 and 5000.
	virtual void DataPort(int portNumber) { }

	virtual void BeginDownloadFile(const char *pathUtf8, bool *skip) { }
	virtual void EndDownloadFile(const char *pathUtf8, unsigned long numBytes) { }
	virtual void VerifyDownloadDir(const char *pathUtf8, bool *skip) { }

	virtual void BeginUploadFile(const char *pathUtf8, bool *skip) { }
	virtual void EndUploadFile(const char *pathUtf8, unsigned long numBytes) { }
	virtual void VerifyUploadDir(const char *pathUtf8, bool *skip) { }

	virtual void VerifyDeleteDir(const char *pathUtf8, bool *skip) { }
	virtual void VerifyDeleteFile(const char *pathUtf8, bool *skip) { }

	virtual void UploadRate(unsigned long byteCount, unsigned long bytesPerSec) { }
	virtual void DownloadRate(unsigned long byteCount, unsigned long bytesPerSec) { }

	virtual void ProgressInfo(const char *name, const char *value) { }

};
#pragma pack (pop)

#endif
