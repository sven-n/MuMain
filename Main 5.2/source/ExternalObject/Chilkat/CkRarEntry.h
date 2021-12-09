// CkRarEntry.h: interface for the CkRarEntry class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKRARENTRY__H
#define _CKRARENTRY__H

#pragma once

#include "CkString.h"
class CkByteData;
class CkRar;
#include "CkObject.h"

#if !defined(WIN32) && !defined(WINCE)
#include "SystemTime.h"
#endif

#undef Copy

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

*/
#pragma pack (push, 8) 

// CLASS: CkRarEntry
class CkRarEntry  : public CkObject
{
    public:

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);



	// This entry's filename.
	void get_Filename(CkString &str);

	// The uncompressed size in bytes.
	unsigned long get_UncompressedSize(void);

	// The compressed size in bytes.
	unsigned long get_CompressedSize(void);

	// The entry's file date/time.
	void get_LastModified(SYSTEMTIME &sysTime);

	// True if this entry is a directory.
	bool get_IsDirectory(void);
	
	bool get_IsReadOnly(void);

	// Extracts this entry into the specified directory. 
	bool Unrar(const char *dirPath);

	unsigned long get_Crc(void);



	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	CkString m_resultString;
        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);
	const char *filename(void);

	// CK_INSERT_POINT

	// END PUBLIC INTERFACE


    // Everything below here is for internal use only.
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkRarEntry(const CkRarEntry &);
	CkRarEntry &operator=(const CkRarEntry &);


    public:
	void *getImpl(void) const;

	CkRarEntry();
	CkRarEntry(void *impl);

	virtual ~CkRarEntry();


};
#pragma pack (pop)

#endif

