// CkGzip.h: interface for the CkGzip class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CK_GZIP_H)
#define _CK_GZIP_H

#pragma once

#include "CkObject.h"

#include "CkString.h"
class CkByteData;

#if !defined(WIN32) && !defined(WINCE)
#include "SystemTime.h"
#endif

class CkGzipProgress;

#pragma pack (push, 8) 

// CLASS: CkGzip
class CkGzip  : public CkObject 
{
public:
    private:
	CkGzipProgress *m_callback;
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

	// Don't allow assignment or copying these objects.
	CkGzip(const CkGzip &) { } 
	CkGzip &operator=(const CkGzip &) { return *this; }
	CkGzip(void *impl);

    public:
	void *getImpl(void) const { return m_impl; } 

	CkGzipProgress *get_EventCallbackObject(void) const { return m_callback; }
	void put_EventCallbackObject(CkGzipProgress *progress) { m_callback = progress; }

	CkGzip();
	virtual ~CkGzip();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	bool UnlockComponent(const char *unlockCode);
	bool IsUnlocked(void) const;


	// File-to-file
	bool CompressFile(const char *inFilename, const char *outFilename);
	bool UncompressFile(const char *inFilename, const char *outFilename);

	// File-to-memory
	bool CompressFileToMem(const char *inFilename, CkByteData &db);
	bool UncompressFileToMem(const char *inFilename, CkByteData &db);

	// Memory-to-file
	bool CompressMemToFile(const CkByteData &db, const char *outFilename);
	bool UncompressMemToFile(const CkByteData &db, const char *outFilename);
	
	// Memory-to-memory
	bool CompressMemory(const CkByteData &dbIn, CkByteData &dbOut);
	bool UncompressMemory(const CkByteData &dbIn, CkByteData &dbOut);

	// String deflate/inflate
	void DeflateStringENC(const char *str, const char *charset, const char *encoding, CkString &strOut);
	void InflateStringENC(const char *str, const char *charset, const char *encoding, CkString &strOut);
	const char *deflateStringENC(const char *str, const char *charset, const char *encoding);
	const char *inflateStringENC(const char *str, const char *charset, const char *encoding);

	void get_Filename(CkString &str);
	void put_Filename(const char *str);

	void get_Comment(CkString &str);
	void put_Comment(const char *str);

	void get_ExtraData(CkByteData &data);
	void put_ExtraData(const CkByteData &data);

	// SYSTEMTIME is defined in <windows.h>
	// For more information, Google "SYSTEMTIME"
	void get_LastMod(SYSTEMTIME &sysTime);
	void put_LastMod(SYSTEMTIME &sysTime);

	bool UnTarGz(const char *gzFilename, const char *destDir, bool bNoAbsolute);

        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);
	const char *comment(void);
	const char *filename(void);
	// VERSION_BEGIN
	void get_Version(CkString &str);
	const char *version(void);
	// VERSION_END
	// READFILE_BEGIN
	bool ReadFile(const char *filename, CkByteData &outBytes);
	// READFILE_END
	// WRITEFILE_BEGIN
	bool WriteFile(const char *filename, CkByteData &binaryData);
	// WRITEFILE_END
	// COMPRESSFILE2_BEGIN
	bool CompressFile2(const char *inFilename, const char *embeddedFilename, const char *outFilename);
	// COMPRESSFILE2_END
	// COMPRESSSTRING_BEGIN
	bool CompressString(const char *inStr, const char *outCharset, CkByteData &outBytes);
	// COMPRESSSTRING_END
	// COMPRESSSTRINGTOFILE_BEGIN
	bool CompressStringToFile(const char *inStr, const char *outCharset, const char *outFilename);
	// COMPRESSSTRINGTOFILE_END
	// UNCOMPRESSFILETOSTRING_BEGIN
	bool UncompressFileToString(const char *inFilename, const char *inCharset, CkString &outStr);
	const char *uncompressFileToString(const char *inFilename, const char *inCharset);
	// UNCOMPRESSFILETOSTRING_END
	// UNCOMPRESSSTRING_BEGIN
	bool UncompressString(CkByteData &inData, const char *inCharset, CkString &outStr);
	const char *uncompressString(CkByteData &inData, const char *inCharset);
	// UNCOMPRESSSTRING_END
	// EXAMINEFILE_BEGIN
	bool ExamineFile(const char *inGzFilename);
	// EXAMINEFILE_END
	// EXAMINEMEMORY_BEGIN
	bool ExamineMemory(CkByteData &inGzData);
	// EXAMINEMEMORY_END
	// DECODE_BEGIN
	bool Decode(const char *str, const char *encoding, CkByteData &outBytes);
	// DECODE_END
	// USECURRENTDATE_BEGIN
	bool get_UseCurrentDate(void);
	void put_UseCurrentDate(bool newVal);
	// USECURRENTDATE_END
	// XFDLTOXML_BEGIN
	bool XfdlToXml(const char *xfdl, CkString &outStr);
	const char *xfdlToXml(const char *xfdl);
	// XFDLTOXML_END
	// ENCODE_BEGIN
	bool Encode(CkByteData &byteData, const char *encoding, CkString &outStr);
	const char *encode(CkByteData &byteData, const char *encoding);
	// ENCODE_END
	// VERBOSELOGGING_BEGIN
	bool get_VerboseLogging(void);
	void put_VerboseLogging(bool newVal);
	// VERBOSELOGGING_END

	// GZIP_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)

#endif // !defined(_CK_GZIP_H)
