// CkZipEntry.h: interface for the CkZipEntry class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKZIPENTRY__H
#define _CKZIPENTRY__H

#pragma once

#ifndef WIN32
#include "int64.h"
#endif

#include "CkString.h"
class CkByteData;
class CkZip;
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

// CLASS: CkZipEntry
class CkZipEntry  : public CkObject
{
    public:

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);


	// This entry's filename.
	void get_FileName(CkString &str);
	void put_FileName(const char *pStr);

	// The uncompressed size in bytes.
	unsigned long get_UncompressedLength(void);

	// The compression level. 0 = no compression, while 9 = maximum compression. 
	// The default is 6.
	long get_CompressionLevel(void);
	void put_CompressionLevel(long newVal);

	// Set to 0 for no compression, or 8 for the Deflate algorithm. 
	// The Deflate algorithm is the default algorithm of the most popular Zip utility programs.
	long get_CompressionMethod(void);
	void put_CompressionMethod(long newVal);

	// The compressed size in bytes.
	unsigned long get_CompressedLength(void);

	// The comment associated with this file in the Zip.
	void get_Comment(CkString &str);
	void put_Comment(const char *pStr);

	// Indicates the origin of the entry. There are three possible values: 
	// (0) a file on disk, 
	// (1) a entry in an existing Zip file, and 
	// (2) an entry created directly from data in memory. 
	long get_EntryType(void);

	// The entry's file date/time.
	// SYSTEMTIME is defined in <windows.h>
	// For more information, Google "SYSTEMTIME"
	void get_FileDateTime(SYSTEMTIME &sysTime);
	void put_FileDateTime(const SYSTEMTIME &sysTime);

	// True if this entry is a directory.
	bool get_IsDirectory(void);

	// The unique ID assigned by Chilkat Zip while the object is instantiated in memory.
	long get_EntryID(void);

	// Unzips this entry into the specified base directory. 
	// The file is unzipped to the subdirectory according to the relative path 
	// stored with the entry in the Zip.
	bool Extract(const char *dirPath);

	// Unzips this entry into the specified directory, regardless of the 
	// relative directory of the entry.
	bool ExtractInto(const char *dirPath);

	// Returns the uncompressed data as a string. Line-endings are modified to 
	// include or not include carriage-returns based on addCR. If addCR = false, 
	// then bare linefeeds are returned, otherwise CRLFs are returned.
	bool InflateToString(bool addCR, CkString &str);

	// Inflates to a string, but leaves the line endings alone.
	bool InflateToString2(CkString &str);

	// Inflates and returns the uncompressed data.
	bool Inflate(CkByteData &bdata);

	// Inflate to an already-open file handle.
#if defined(WIN32) | defined(WINCE)
	bool InflateToHandle(HANDLE handle);
#endif

	// Replaces the entry's data.
	bool ReplaceData(const CkByteData &bdata);
	bool ReplaceData(const CkString &sdata);

	// Appends more data to the entry's data.
	bool AppendData(const CkByteData &bdata);
	bool AppendData(const CkString &sdata);

	// Returns the compressed data as a Base64-encoded string. 
	// This data can be re-added to a Zip by calling CkZip.AppendBase64.
	bool CopyToBase64(CkString &str);

	// Returns the compressed data as a Hex-encoded string. 
	// This data can be re-added to a Zip by calling CkZip.AppendHex.
	bool CopyToHex(CkString &str);

	// Returns the raw compressed data. 
	bool Copy(CkByteData &bdata);

	// Returns the next entry in the Zip.  The returned object must be deleted by
	// the application.
	CkZipEntry *NextEntry(void);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	CkString m_resultString;
        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);
	const char *fileName(void);
	const char *comment(void);
	const char *inflateToString(bool addCR);
	const char *inflateToString2(void);
	const char *copyToBase64(void);
	const char *copyToHex(void);
	// CRC_BEGIN
	int get_Crc(void);
	// CRC_END
	// COMPRESSEDLENGTH64_BEGIN
	__int64 get_CompressedLength64(void);
	// COMPRESSEDLENGTH64_END
	// UNCOMPRESSEDLENGTH64_BEGIN
	__int64 get_UncompressedLength64(void);
	// UNCOMPRESSEDLENGTH64_END

	// ZIPENTRY_INSERT_POINT

	// END PUBLIC INTERFACE


    // Everything below here is for internal use only.
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkZipEntry(const CkZipEntry &) { } 
	CkZipEntry &operator=(const CkZipEntry &) { return *this; }
	CkZipEntry(void *impl);


    public:
	void *getImpl(void) const { return m_impl; }
	void inject(void *impl);

	CkZipEntry(); // the default constructor should NEVER be called...
		
	virtual ~CkZipEntry();


};
#pragma pack (pop)

#endif

