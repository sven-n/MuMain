// CkCompression.h: interface for the CkCompression class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKCOMPRESSION_H
#define _CKCOMPRESSION_H

#pragma once

#include "CkString.h"
class CkByteData;
//class CkCompressionProgress;
#include "CkObject.h"

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

*/
#pragma pack (push, 8) 

// CLASS: CkCompression
class CkCompression  : public CkObject
{
    public:
	CkCompression();
	virtual ~CkCompression();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	void get_Algorithm(CkString &out);
	void put_Algorithm(const char *alg);

	bool CompressFile(const char *inFile, const char *outFile);
	bool DecompressFile(const char *inFile, const char *outFile);



	bool DecompressStringENC(const char *str, CkString &out);
	bool DecompressString(const CkByteData &bData, CkString &out);
	bool DecompressBytesENC(const char *str, CkByteData &out);
	bool DecompressBytes(const CkByteData &bData, CkByteData &out);

	bool CompressStringENC(const char *str, CkString &out);
	bool CompressBytesENC(const CkByteData &bData, CkString &out);
	bool CompressString(const char *str, CkByteData &out);
	bool CompressBytes(const CkByteData &bData, CkByteData &out);

	const char *compressStringENC(const char *str);
	const char *compressBytesENC(const CkByteData &bData);

	const char *decompressString(const CkByteData &bData);
	const char *decompressStringENC(const char *str);



	bool BeginDecompressStringENC(const char *str, CkString &out);
	bool BeginDecompressString(const CkByteData &bData, CkString &out);
	bool BeginDecompressBytesENC(const char *str, CkByteData &out);
	bool BeginDecompressBytes(const CkByteData &bData, CkByteData &out);

	bool BeginCompressStringENC(const char *str, CkString &out);
	bool BeginCompressBytesENC(const CkByteData &bData, CkString &out);
	bool BeginCompressString(const char *str, CkByteData &out);
	bool BeginCompressBytes(const CkByteData &bData, CkByteData &out);

	const char *beginCompressStringENC(const char *str);
	const char *beginCompressBytesENC(const CkByteData &bData);

	const char *beginDecompressString(const CkByteData &bData);
	const char *beginDecompressStringENC(const char *str);




	bool MoreDecompressStringENC(const char *str, CkString &out);
	bool MoreDecompressString(const CkByteData &bData, CkString &out);
	bool MoreDecompressBytesENC(const char *str, CkByteData &out);
	bool MoreDecompressBytes(const CkByteData &bData, CkByteData &out);

	bool MoreCompressStringENC(const char *str, CkString &out);
	bool MoreCompressBytesENC(const CkByteData &bData, CkString &out);
	bool MoreCompressString(const char *str, CkByteData &out);
	bool MoreCompressBytes(const CkByteData &bData, CkByteData &out);

	const char *moreCompressStringENC(const char *str);
	const char *moreCompressBytesENC(const CkByteData &bData);

	const char *moreDecompressString(const CkByteData &bData);
	const char *moreDecompressStringENC(const char *str);




	bool EndDecompressStringENC(CkString &out);
	bool EndDecompressString(CkString &out);
	bool EndDecompressBytesENC(CkByteData &out);
	bool EndDecompressBytes(CkByteData &out);

	bool EndCompressStringENC(CkString &out);
	bool EndCompressBytesENC(CkString &out);
	bool EndCompressString(CkByteData &out);
	bool EndCompressBytes(CkByteData &out);

	const char *endCompressStringENC(void);
	const char *endCompressBytesENC(void);

	const char *endDecompressString(void);
	const char *endDecompressStringENC(void);



	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	// CHARSET_BEGIN
	void get_Charset(CkString &str);
	const char *charset(void);
	void put_Charset(const char *newVal);
	// CHARSET_END
	// ENCODINGMODE_BEGIN
	void get_EncodingMode(CkString &str);
	const char *encodingMode(void);
	void put_EncodingMode(const char *newVal);
	// ENCODINGMODE_END
	// UNLOCKCOMPONENT_BEGIN
	bool UnlockComponent(const char *unlockCode);
	// UNLOCKCOMPONENT_END

        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);
	const char *algorithm(void);
	// VERSION_BEGIN
	void get_Version(CkString &str);
	const char *version(void);
	// VERSION_END

	// COMPRESSION_INSERT_POINT

	// END PUBLIC INTERFACE






    // Everything below is for internal use only.
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkCompression(const CkCompression &);
	CkCompression &operator=(const CkCompression &);
	CkCompression(void *impl);

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const;


};
#pragma pack (pop)

#endif
