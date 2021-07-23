// CkHtmlToText.h: interface for the CkHtmlToText class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CkHtmlToText_H
#define _CkHtmlToText_H

#pragma once


class CkByteData;
#include "CkString.h"
#include "CkObject.h"

#pragma warning( disable : 4068 )
#pragma unmanaged

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#pragma pack (push, 8) 

// CLASS: CkHtmlToText
class CkHtmlToText  : public CkObject
{
    public:
	CkHtmlToText();
	virtual ~CkHtmlToText();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	bool UnlockComponent(const char *code);
	bool IsUnlocked(void) const;

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);
	
        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);

	const char *readFileToString(const char *filename, const char *srcCharset);
	bool ReadFileToString(const char *filename, const char *srcCharset, CkString &str);
	bool WriteStringToFile(const char *str, const char *filename, const char *charset);

	// TOTEXT_BEGIN
	bool ToText(const char *html, CkString &outStr);
	const char *toText(const char *html);
	// TOTEXT_END
	// RIGHTMARGIN_BEGIN
	int get_RightMargin(void);
	void put_RightMargin(int newVal);
	// RIGHTMARGIN_END

	// HTMLTOTEXT_INSERT_POINT

	// END PUBLIC INTERFACE


    // For internal use only.
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkHtmlToText(const CkHtmlToText &) { } 
	CkHtmlToText &operator=(const CkHtmlToText &) { return *this; }

	CkString m_resultString;

};
#pragma pack (pop)


#endif


