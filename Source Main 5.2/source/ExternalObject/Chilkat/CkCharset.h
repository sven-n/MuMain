// CkCharset.h: interface for the CkCharset class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKCHARSET_H
#define _CKCHARSET_H

#pragma once


class CkByteData;
#include "CkObject.h"
#include "CkString.h"

#pragma pack (push, 8) 

// CLASS: CkCharset
class CkCharset  : public CkObject
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkCharset(const CkCharset &) { } 
	CkCharset &operator=(const CkCharset &) { return *this; }

    public:
	CkCharset();
	virtual ~CkCharset();

	bool GetHtmlCharset(const char *htmlData, unsigned long htmlDataLen, CkString &strCharset);
	bool ConvertHtml(const char *htmlIn, unsigned long htmlInLen, CkByteData &htmlOut);
	bool WriteFile(const char *filename, const char *data, unsigned long dataLen);
	bool ConvertFromUnicode(const char *data, unsigned long dataLen, CkByteData &mbData);
	bool ConvertToUnicode(const char *data, unsigned long dataLen, CkByteData &uniData);
	bool ConvertData(const char *inData, unsigned long inDataLen, CkByteData &outData);
	void SetErrorBytes(const char *data, unsigned long dataLen);
	const char *getHtmlCharset(const char *htmlData, unsigned long htmlDataLen);


	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	const char *upperCase(const char *inStr);
	const char *lowerCase(const char *inStr);
	void UpperCase(const char *inStr, CkString &outStr);
	void LowerCase(const char *inStr, CkString &outStr);

	bool GetHtmlFileCharset(const char *htmlFilename, CkString &strCharset);
	bool GetHtmlCharset(const CkByteData &htmlData, CkString &strCharset);
	bool GetHtmlCharset(const unsigned char *htmlData, unsigned long htmlDataLen, CkString &strCharset);
	bool ConvertHtmlFile(const char *inFilename, const char *outFilename);
	bool ConvertHtml(const CkByteData &htmlIn, CkByteData &htmlOut);
	bool ConvertHtml(const unsigned char *htmlIn, unsigned long htmlInLen, CkByteData &htmlOut);

	void get_LastOutputAsQP(CkString &str);
	void get_LastInputAsQP(CkString &str);
	void get_LastOutputAsHex(CkString &str);
	void get_LastInputAsHex(CkString &str);
	void put_SaveLast(bool value);
	bool get_SaveLast(void);

	bool WriteFile(const char *filename, const CkByteData &dataBuf);
	bool WriteFile(const char *filename, const unsigned char *data, unsigned long dataLen);
	bool ReadFile(const char *filename, CkByteData &dataBuf);

	bool ConvertFromUnicode(const CkByteData &uniData, CkByteData &mbData);
	bool ConvertToUnicode(const CkByteData &mbData, CkByteData &uniData);

	bool ConvertFromUnicode(const unsigned char *data, unsigned long dataLen, CkByteData &mbData);
	bool ConvertToUnicode(const unsigned char *data, unsigned long dataLen, CkByteData &uniData);

	bool VerifyFile(const char *charset, const char *filename);
	bool VerifyData(const char *charset, const CkByteData &charData);

	bool ConvertFile(const char *inFilename, const char *outFilename);

	bool HtmlEntityDecode(CkByteData &inData, CkByteData &outData);
	bool HtmlDecodeToStr(const char *str, CkString &strOut);
	bool HtmlEntityDecodeFile(const char *inFilename, const char *outFilename);

	void EntityEncodeHex(const char *inStr, CkString &outStr);
	void EntityEncodeDec(const char *inStr, CkString &outStr);
	const char *entityEncodeHex(const char *inStr);
	const char *entityEncodeDec(const char *inStr);

	bool ConvertData(const CkByteData &inData, CkByteData &outData);
	bool ConvertData(const unsigned char *inData, unsigned long inDataLen, CkByteData &outData);

	// Convert byte data into the memory buffer you provide. outDataLen is set
	// to the size of the output buffer 
	bool ConvertData(const unsigned char *inData, unsigned long inDataLen,unsigned char *outData, unsigned long *outDataLen);

	void get_ToCharset(CkString &str);
	void put_ToCharset(const char *charset);
	void get_FromCharset(CkString &str);
	void put_FromCharset(const char *charset);
	void get_Version(CkString &str);

	bool UnlockComponent(const char *unlockCode);
	bool IsUnlocked(void) const;

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	int CharsetToCodePage(const char *charsetName);
	bool CodePageToCharset(int codePage, CkString &sCharset);

	// Error actions:
	// 0: drop error character.
	// 1: substitute default bytes
	// 2: hex numeric escape &#xXXXX;          
	// 3: RESERVED
	// 4: RESERVED
	// 5: RESERVED
	// 6: use alternate To charset
	// 7: pass through non-convertable bytes unchanged.
	int get_ErrorAction(void);
	void put_ErrorAction(int val);

	void get_AltToCharset(CkString &str);
	void put_AltToCharset(const char *charsetName);

	void SetErrorBytes(const unsigned char *data, unsigned long dataLen);
	void SetErrorString(const char *str);

	CkString m_resultString;
        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);

	const char *getHtmlFileCharset(const char *htmlFilename);
	const char *getHtmlCharset(const CkByteData &htmlData);
	const char *getHtmlCharset(const unsigned char *htmlData, unsigned long htmlDataLen);
	const char *lastOutputAsQP(void);
	const char *lastInputAsQP(void);
	const char *lastOutputAsHex(void);
	const char *lastInputAsHex(void);
	const char *htmlDecodeToStr(const char *str);
	const char *toCharset(void);
	const char *fromCharset(void);
	const char *version(void);
	const char *codePageToCharset(int codePage);
	const char *altToCharset(void);
	// URLDECODESTR_BEGIN
	bool UrlDecodeStr(const char *inStr, CkString &outStr);
	const char *urlDecodeStr(const char *inStr);
	// URLDECODESTR_END

	// CHARSET_INSERT_POINT

	// END PUBLIC INTERFACE



};
#pragma pack (pop)

#endif
