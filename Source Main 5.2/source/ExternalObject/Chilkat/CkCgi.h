// CkCgi.h: interface for the CkCgi class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKCGI_H
#define _CKCGI_H

#pragma once


class CkByteData;
class CkString;

#include "CkObject.h"
#include "CkString.h"

#pragma warning( disable : 4068 )
#pragma unmanaged

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#pragma pack (push, 8) 

// CLASS: CkCgi
class CkCgi  : public CkObject
{
    public:
	CkCgi();
	virtual ~CkCgi();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	void get_Version(CkString &str);

	void SleepMs(int millisec) const;

	int get_ReadChunkSize(void) const;
	void put_ReadChunkSize(int numBytes);

	int get_IdleTimeoutMs(void) const;
	void put_IdleTimeoutMs(int millisec);

	int get_HeartbeatMs(void) const;
	void put_HeartbeatMs(int millisec);

	unsigned long get_SizeLimitKB(void) const;
	void put_SizeLimitKB(unsigned long kilobytes);

	void get_DebugLogFilename(CkString &str);
	void put_DebugLogFilename(const char *newVal);

	void get_UploadDir(CkString &str);
	void put_UploadDir(const char *newVal);

	bool get_StreamToUploadDir(void) const;
	void put_StreamToUploadDir(bool newVal);

	// What do we have?
	bool IsPost(void);
	bool IsGet(void);
	bool IsHead(void);
	bool IsUpload(void);

	// Fetching file upload information.
	int get_NumUploadFiles(void);

	bool GetUploadFilename(int idx, CkString &str);
	unsigned long GetUploadSize(int idx);
	bool GetUploadData(int idx, CkByteData &byteData);

	// Fetching Query parameters.
	bool GetParam(const char *paramName, CkString &str);

	int get_NumParams(void);
	bool GetParamName(int idx, CkString &str);
	bool GetParamValue(int idx, CkString &str);

	void GetRawPostData(CkByteData &byteData);

	bool GetEnv(const char *varName, CkString &str);

	// Read the HTTP request on stdin.
	// Also gathers the environment variables.
	bool ReadRequest(void);
#ifdef WIN32
#ifndef SINGLE_THREADED
	bool AsyncReadRequest(void);
#endif
#endif

	// Asynchronous Progress Monitoring and Abort:
	unsigned long get_AsyncPostSize(void) const;
	unsigned long get_AsyncBytesRead(void) const;
	bool get_AsyncInProgress(void) const;
	bool get_AsyncSuccess(void) const;
	void AbortAsync(void);

	CkString m_resultString;
	const char *version(void);
	const char *getUploadFilename(int idx);
	const char *getParam(const char *paramName);
	const char *getParamName(int idx);
	const char *getParamValue(int idx);
	const char *getEnv(const char *varName);

        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);

	// CGI_INSERT_POINT

	// END PUBLIC INTERFACE

    // For internal use only.
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkCgi(const CkCgi &) { } 
	CkCgi &operator=(const CkCgi &) { return *this; }
	CkCgi(void *impl);


};
#pragma pack (pop)


#endif


