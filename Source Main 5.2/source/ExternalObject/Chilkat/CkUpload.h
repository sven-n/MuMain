// CkUpload.h: interface for the CkUpload class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKUPLOAD_H
#define _CKUPLOAD_H

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


// CLASS: CkUpload
class CkUpload  : public CkObject
{
    public:
	CkUpload();
	virtual ~CkUpload();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	void get_Version(CkString &str) const;

	void SleepMs(int millisec) const;

	int get_ChunkSize(void) const;
	void put_ChunkSize(int numBytes);

	int get_IdleTimeoutMs(void) const;
	void put_IdleTimeoutMs(int millisec);

	//int get_HeartbeatMs(void) const;
	//void put_HeartbeatMs(int millisec);

	bool get_UploadInProgress(void) const;
	bool get_UploadSuccess(void) const;

	void AddCustomHeader(const char *name, const char *value);
	void AddParam(const char *name, const char *value);
	void AddFileReference(const char *name, const char *filename);

	int get_Port(void) const;
	void put_Port(int port);

	bool get_Ssl(void) const;
	void put_Ssl(bool newVal);

	void get_Hostname(CkString &hostname) const;
	void put_Hostname(const char *hostname);

	void get_Path(CkString &path) const;
	void put_Path(const char *path);

	unsigned long get_TotalUploadSize(void) const;
	unsigned long get_NumBytesSent(void) const;
	unsigned long get_PercentUploaded(void) const;

	int get_ResponseStatus(void) const;
	void get_ResponseHeader(CkString &header) const;
	void get_ResponseBody(CkByteData &body) const;

	// Blocking HTTP upload
	bool BlockingUpload(void);


	void get_Login(CkString &str);
	void put_Login(const char *newVal);

	void get_Password(CkString &str);
	void put_Password(const char *newVal);

	const char *login(void);
	const char *password(void);


	long get_ProxyPort(void);
	void put_ProxyPort(long n);
	
	void put_ProxyDomain(const char *v);
	void get_ProxyDomain(CkString &str);

	void get_ProxyLogin(CkString &str);
	void put_ProxyLogin(const char *newVal);

	void get_ProxyPassword(CkString &str);
	void put_ProxyPassword(const char *newVal);

	const char *proxyLogin(void);
	const char *proxyPassword(void);
	const char *proxyDomain(void);



	// Non-blocking HTTP upload.
#ifndef SINGLE_THREADED
	bool BeginUpload(void);
	void AbortUpload(void);
#endif

	// Upload to memory.
	bool UploadToMemory(CkByteData &dataBuf);

	CkString m_resultString;
        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);
	const char *hostname(void);
	const char *path(void);
	const char *responseHeader(void);
	const char *version(void);

	// CK_INSERT_POINT

	// END PUBLIC INTERFACE



    // For internal use only.
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkUpload(const CkUpload &) { } 
	CkUpload &operator=(const CkUpload &) { return *this; }
	CkUpload(void *impl);


};

#pragma pack (pop)

#endif


