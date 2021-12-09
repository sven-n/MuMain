// CkBz2.h: interface for the CkBz2 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CkBz2_H
#define _CkBz2_H

#pragma once

#include "CkString.h"
class CkByteData;

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#include "CkObject.h"

class CkBz2Progress;

#pragma pack (push, 8) 

// CLASS: CkBz2
class CkBz2  : public CkObject
{
    private:
	CkBz2Progress *m_callback;

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkBz2(const CkBz2 &) { } 
	CkBz2 &operator=(const CkBz2 &) { return *this; }
	CkBz2(void *impl) : m_impl(impl),m_callback(0) { }

    public:
	void *getImpl(void) const { return m_impl; } 

	CkBz2();
	virtual ~CkBz2();


	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);
	
	CkBz2Progress *get_EventCallbackObject(void) const;
	void put_EventCallbackObject(CkBz2Progress *progress);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
	void LastErrorXml(CkString &str);
	void LastErrorHtml(CkString &str);
	void LastErrorText(CkString &str);

    const char *lastErrorText(void);
    const char *lastErrorXml(void);
    const char *lastErrorHtml(void);
	// UNCOMPRESSFILE_BEGIN
	bool UncompressFile(const char *inFilename, const char *outFilename);
	// UNCOMPRESSFILE_END
	// COMPRESSFILE_BEGIN
	bool CompressFile(const char *inFilename, const char *outFilename);
	// COMPRESSFILE_END
	// UNCOMPRESSFILETOMEM_BEGIN
	bool UncompressFileToMem(const char *inFilename, CkByteData &outBytes);
	// UNCOMPRESSFILETOMEM_END
	// COMPRESSFILETOMEM_BEGIN
	bool CompressFileToMem(const char *inFilename, CkByteData &outBytes);
	// COMPRESSFILETOMEM_END
	// COMPRESSMEMTOFILE_BEGIN
	bool CompressMemToFile(CkByteData &inData, const char *outFilename);
	// COMPRESSMEMTOFILE_END
	// UNCOMPRESSMEMTOFILE_BEGIN
	bool UncompressMemToFile(CkByteData &inData, const char *outFilename);
	// UNCOMPRESSMEMTOFILE_END
	// COMPRESSMEMORY_BEGIN
	bool CompressMemory(CkByteData &inData, CkByteData &outBytes);
	// COMPRESSMEMORY_END
	// UNCOMPRESSMEMORY_BEGIN
	bool UncompressMemory(CkByteData &inData, CkByteData &outBytes);
	// UNCOMPRESSMEMORY_END
	// UNLOCKCOMPONENT_BEGIN
	bool UnlockComponent(const char *regCode);
	// UNLOCKCOMPONENT_END

	// BZ2_INSERT_POINT

	// END PUBLIC INTERFACE

	CkString m_resultString;
	


};
#pragma pack (pop)


#endif


