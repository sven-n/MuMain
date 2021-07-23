// CkHtmlToXml.h: interface for the CkHtmlToXml class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CkHtmlToXml_H
#define _CkHtmlToXml_H

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

// CLASS: CkHtmlToXml
class CkHtmlToXml  : public CkObject
{
    public:
	CkHtmlToXml();
	virtual ~CkHtmlToXml();

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

	void get_Version(CkString &str);


	// 0 (default) = convert to space, 1 = convert to &#160, 2 = drop.
	void put_Nbsp(long v);
	long get_Nbsp(void);
	
	void put_Html(const char *html);
	void get_Html(CkString &str);
	
	void put_XmlCharset(const char *charset);
	void get_XmlCharset(CkString &str);
	
	bool SetHtmlFromFile(const char *filename);
	
	// Do it this way because we might want to 
	// extract other information out of the HTML.
	bool ToXml(CkString &str);
	bool Xml(CkString &str);    // Same as ToXml.
		
	bool ReadFileToString(const char *filename, const char *srcCharset, CkString &str);

	bool WriteStringToFile(const char *str, const char *filename, const char *charset);

	bool ConvertFile(const char *inHtmlFilename, const char *outXmlFilename);
		
	// Drop any tag type, such as "b", "br", etc.
	void DropTagType(const char *tagName);
	void UndropTagType(const char *tagName);

	// Drop b, i, u, em, strong, font, 
	void DropTextFormattingTags(void);
	void UndropTextFormattingTags(void);

	// Drop any tags not recognized as HTML tags.
	void put_DropCustomTags(bool v);
	bool get_DropCustomTags(void);
		
	CkString m_resultString;
        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);

	const char *html(void);
	const char *xmlCharset(void);
	const char *xml(void);

	const char *version(void);
	const char *toXml(void);
	const char *readFileToString(const char *filename, const char *srcCharset);
	// SETHTMLBYTES_BEGIN
	void SetHtmlBytes(CkByteData &inData);
	// SETHTMLBYTES_END

	// HTMLTOXML_INSERT_POINT

	// END PUBLIC INTERFACE


    // For internal use only.
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkHtmlToXml(const CkHtmlToXml &) { } 
	CkHtmlToXml &operator=(const CkHtmlToXml &) { return *this; }

};
#pragma pack (pop)


#endif


