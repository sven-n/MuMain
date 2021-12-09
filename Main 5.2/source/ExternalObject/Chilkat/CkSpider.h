// CkSpider.h: interface for the CkSpider class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKSPIDER_H
#define _CKSPIDER_H

#pragma once

#include "CkString.h"
class CkSpiderProgress;

#if !defined(WIN32) && !defined(WINCE)
#include "SystemTime.h"
#endif

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkSpider
class CkSpider  : public CkObject
{
    private:
	CkSpiderProgress *m_callback;
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkSpider(const CkSpider &) { } 
	CkSpider &operator=(const CkSpider &) { return *this; }

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	//void *getImpl(void) const { return m_impl; } 
	//CkSpider(void *impl) : m_impl(impl) { }
	CkSpider();
	virtual ~CkSpider();

	CkSpiderProgress *get_EventCallbackObject(void) const;
	void put_EventCallbackObject(CkSpiderProgress *progress);

	// BEGIN PUBLIC INTERFACE
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	void SleepMs(long millisec);

	void get_LastHtmlDescription(CkString &strOut);
	void get_LastHtmlKeywords(CkString &strOut);
	void get_LastHtmlTitle(CkString &strOut);
	void get_LastHtml(CkString &strOut);

	bool get_LastFromCache(void);
	// SYSTEMTIME is defined in <windows.h>
	// For more information, Google "SYSTEMTIME"
	void get_LastModDate(SYSTEMTIME &sysTime);
	void get_LastUrl(CkString &strOut);
	void get_LastModDateStr(CkString &strOut);

	void SkipUnspidered(long index);
	bool FetchRobotsText(CkString &strOut);

	void get_Domain(CkString &strOut);
	void AddMustMatchPattern(const char *pattern);
	void AddAvoidOutboundLinkPattern(const char *pattern);
	bool GetAvoidPattern(long index, CkString &strOut);
	void AddAvoidPattern(const char *pattern);
	bool GetOutboundLink(long index, CkString &strOut);
	bool GetFailedUrl(long index, CkString &strOut);
	bool GetSpideredUrl(long index, CkString &strOut);
	bool GetUnspideredUrl(long index, CkString &strOut);
	bool RecrawlLast(void);

	void ClearOutboundLinks();
	void ClearFailedUrls();
	void ClearSpideredUrls();

	int get_WindDownCount(void);
	void put_WindDownCount(long newVal);

	// These times are in milliseconds.
	long get_HeartbeatMs(void);
	void put_HeartbeatMs(long newVal);

	int get_NumAvoidPatterns(void);
	int get_NumOutboundLinks(void);
	int get_NumFailed(void);
	int get_NumSpidered(void);
	int get_NumUnspidered(void);
	
	bool CrawlNext(void);

	bool get_ChopAtQuery(void);
	void put_ChopAtQuery(bool newVal);

	bool get_AvoidHttps(void);
	void put_AvoidHttps(bool newVal);

	int get_MaxResponseSize(void);
	void put_MaxResponseSize(long newVal);

	int get_MaxUrlLen(void);
	void put_MaxUrlLen(long newVal);

	void get_CacheDir(CkString &strOut);
	void put_CacheDir(const char *dir);

	bool get_UpdateCache(void);
	void put_UpdateCache(bool newVal);

	bool get_FetchFromCache(void);
	void put_FetchFromCache(bool newVal);

	int get_ConnectTimeout(void);
	void put_ConnectTimeout(long numSeconds);

	void put_UserAgent(const char *ua);
	const char *userAgent(void);
	void get_UserAgent(CkString &strOut);

	int get_ReadTimeout(void);
	void put_ReadTimeout(long numSeconds);

	void AddUnspidered(const char *url);
	void Initialize(const char *domain);

	void GetDomain(const char *url, CkString &domainOut);
	void GetBaseDomain(const char *domain, CkString &domainOut);
	void CanonicalizeUrl(const char *url, CkString &urlOut);

	const char *getDomain(const char *url);
	const char *getBaseDomain(const char *domain);
	const char *canonicalizeUrl(const char *url);

	const char *getAvoidPattern(long index);
	const char *getOutboundLink(long index);
	const char *getFailedUrl(long index);
	const char *getSpideredUrl(long index);
	const char *getUnspideredUrl(long index);

	const char *cacheDir(void);
	const char *avoidPattern(long index);
	const char *outboundLink(long index);
	const char *failedUrl(long index);
	const char *spideredUrl(long index);
	const char *unspideredUrl(long index);
	const char *domain(void);
	const char *lastHtmlDescription(void);
	const char *lastHtmlKeywords(void);
	const char *lastHtmlTitle(void);
	const char *lastHtml(void);
	const char *lastUrl(void);
	const char *lastModDateStr(void);
	const char *fetchRobotsText(void);

        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);
	// PROXYDOMAIN_BEGIN
	void get_ProxyDomain(CkString &str);
	const char *proxyDomain(void);
	void put_ProxyDomain(const char *newVal);
	// PROXYDOMAIN_END
	// PROXYLOGIN_BEGIN
	void get_ProxyLogin(CkString &str);
	const char *proxyLogin(void);
	void put_ProxyLogin(const char *newVal);
	// PROXYLOGIN_END
	// PROXYPASSWORD_BEGIN
	void get_ProxyPassword(CkString &str);
	const char *proxyPassword(void);
	void put_ProxyPassword(const char *newVal);
	// PROXYPASSWORD_END
	// PROXYPORT_BEGIN
	int get_ProxyPort(void);
	void put_ProxyPort(int newVal);
	// PROXYPORT_END
	// VERBOSELOGGING_BEGIN
	bool get_VerboseLogging(void);
	void put_VerboseLogging(bool newVal);
	// VERBOSELOGGING_END

	// SPIDER_INSERT_POINT

	// END PUBLIC INTERFACE

};

#pragma pack (pop)

#endif


