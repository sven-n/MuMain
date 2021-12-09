// CkHttpRequest.h: interface for the CkHttpRequest class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKHTTPREQUEST_H
#define _CKHTTPREQUEST_H

#pragma once

#include "CkString.h"
class CkByteData;

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkHttpRequest
class CkHttpRequest  : public CkObject
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkHttpRequest(const CkHttpRequest &) { } 
	CkHttpRequest &operator=(const CkHttpRequest &) { return *this; }
	CkHttpRequest(void *impl) : m_impl(impl) { }

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const { return m_impl; } 

	CkHttpRequest();
	virtual ~CkHttpRequest();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);


	void UsePost(void);
	void UseGet(void);
	void UseHead(void);
	void UseXmlHttp(const char *xmlBody);
	void UseUpload(void);

	void SetFromUrl(const char *url);

	// Path if not set from a URL.
	void put_Path(const char *path);
	void get_Path(CkString &str);
	
	// Default is "utf-8"
	void put_Charset(const char *charset);
	void get_Charset(CkString &str);
	
	// If true, the charset attribute is included in the
	// content-type header field in the HTTP request.
	void put_SendCharset(bool val);
	bool get_SendCharset(void);


	void put_HttpVersion(const char *version);
	void get_HttpVersion(CkString &str);

	// HTTP headers.
	void AddHeader(const char *name, const char *value);

	// Add a string request parameter.
	void AddParam(const char *name, const char *value);
	void RemoveParam(const char *name);

	// Data to upload.
	bool AddFileForUpload(const char *name, const char *filename);

	// Generate the HTTP request header for this object.
	void GenerateRequestText(CkString &str);

	// Get a header field by name.
	bool GetHeaderField(const char *name, CkString &str);
	int get_NumHeaderFields(void);
	bool GetHeaderName(int index, CkString &str);
	bool GetHeaderValue(int index, CkString &str);

	// Get request parameters.
	bool GetParam(const char *name, CkString &str);
	int get_NumParams(void);
	bool GetParamName(int index, CkString &str);
	bool GetParamValue(int index, CkString &str);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);


	const char *path(void);
	const char *charset(void);
	const char *httpVersion(void);
	const char *getHeaderField(const char *name);
	const char *getHeaderName(int index);
	const char *getHeaderValue(int index);
	const char *getParam(const char *name);
	const char *getParamName(int index);
	const char *getParamValue(int index);
        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);
	const char *generateRequestText(void);

	// REMOVEALLPARAMS_BEGIN
	void RemoveAllParams(void);
	// REMOVEALLPARAMS_END
	// LOADBODYFROMFILE_BEGIN
	bool LoadBodyFromFile(const char *filename);
	// LOADBODYFROMFILE_END
	// LOADBODYFROMBYTES_BEGIN
	bool LoadBodyFromBytes(CkByteData &binaryData);
	// LOADBODYFROMBYTES_END
	// LOADBODYFROMSTRING_BEGIN
	bool LoadBodyFromString(const char *bodyStr, const char *charset);
	// LOADBODYFROMSTRING_END
	// USEPUT_BEGIN
	void UsePut(void);
	// USEPUT_END
	// ADDBYTESFORUPLOAD_BEGIN
	bool AddBytesForUpload(const char *name, const char *filename, CkByteData &byteData);
	// ADDBYTESFORUPLOAD_END
	// USEPOSTMULTIPARTFORM_BEGIN
	void UsePostMultipartForm(void);
	// USEPOSTMULTIPARTFORM_END
	// GETURLENCODEDPARAMS_BEGIN
	bool GetUrlEncodedParams(CkString &outStr);
	const char *getUrlEncodedParams(void);
	// GETURLENCODEDPARAMS_END
	// ADDSTRINGFORUPLOAD_BEGIN
	bool AddStringForUpload(const char *name, const char *filename, const char *strData, const char *charset);
	// ADDSTRINGFORUPLOAD_END
	// ADDFILEFORUPLOAD2_BEGIN
	bool AddFileForUpload2(const char *name, const char *filename, const char *contentType);
	// ADDFILEFORUPLOAD2_END
	// ADDSTRINGFORUPLOAD2_BEGIN
	bool AddStringForUpload2(const char *name, const char *filename, const char *strData, const char *charset, const char *contentType);
	// ADDSTRINGFORUPLOAD2_END
	// ADDBYTESFORUPLOAD_BEGIN
	bool AddBytesForUpload2(const char *name, const char *filename, CkByteData &byteData, const char *contentType);
	// ADDBYTESFORUPLOAD_END

	// HTTPREQUEST_INSERT_POINT

	// END PUBLIC INTERFACE



};
#pragma pack (pop)


#endif


