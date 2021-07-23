// CkZipProgress.h: interface for the CkZipProgress class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKZIPPROGRESS_H
#define _CKZIPPROGRESS_H

#pragma once

/*

  To receive progress events (callbacks), create a C++ class that 
  inherits this one and provides one or more overriding implementations 
  for the events you wish to receive.  Then pass your Zip progress object
  to CkZip methods such as WriteZip, Extract, etc.

  */
#pragma pack (push, 8) 

class CkZipProgress  
{
    public:
	CkZipProgress();
	virtual ~CkZipProgress();

	virtual void WriteZipPercentDone(long pctDone, bool *abort) { }
	virtual void UnzipPercentDone(long pctDone, bool *abort) { }

	// Called periodically to check to see if the Zip / Unzip should be aborted.
	virtual void AbortCheck(bool *abort) { }

	virtual void ToBeAdded(const char *fileName, 
	    unsigned long fileSize, 
	    bool *excludeFlag) { }

	virtual void DirToBeAdded(const char *fileName, 
	    bool *excludeFlag) { }

	virtual void FileAdded(const char *fileName, 
	    unsigned long fileSize, 
	    bool *abort) { }

	virtual void ToBeZipped(const char *fileName, 
	    unsigned long fileSize, 
	    bool *abort) { }

	virtual void FileZipped(const char *fileName, 
	    unsigned long fileSize, 
	    unsigned long compressedSize, 
	    bool *abort) { }

	virtual void ToBeUnzipped(const char *fileName, 
	    unsigned long compressedSize,
	    unsigned long fileSize, 
	    bool *abort) { }

	virtual void FileUnzipped(const char *fileName, 
	    unsigned long compressedSize,
	    unsigned long fileSize, 
	    bool *abort) { }

	virtual void AddFilesBegin(void) { }
	virtual void AddFilesEnd(void) { }
	virtual void WriteZipBegin(void) { }
	virtual void WriteZipEnd(void) { }
	virtual void UnzipBegin(void) { }
	virtual void UnzipEnd(void) { }
};
#pragma pack (pop)

#endif
