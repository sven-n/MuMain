// CkCache.h: interface for the CkCache class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKCACHE_H
#define _CKCACHE_H

#pragma once

#include "CkString.h"
class CkByteData;

#if !defined(WIN32) && !defined(WINCE)
#include "SystemTime.h"
#endif

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkCache
class CkCache  : public CkObject
{
    private:

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkCache(const CkCache &) { } 
	CkCache &operator=(const CkCache &) { return *this; }
	CkCache(void *impl) : m_impl(impl) { }

    public:
	void *getImpl(void) const { return m_impl; } 

	CkCache();
	virtual ~CkCache();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);
	
	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
	void LastErrorXml(CkString &str);
	void LastErrorHtml(CkString &str);
	void LastErrorText(CkString &str);

    const char *lastErrorText(void);
    const char *lastErrorXml(void);
    const char *lastErrorHtml(void);
    	// LASTETAGFETCHED_BEGIN
	void get_LastEtagFetched(CkString &str);
	const char *lastEtagFetched(void);
	// LASTETAGFETCHED_END
	// LASTEXPIRATIONFETCHED_BEGIN
	void get_LastExpirationFetched(SYSTEMTIME &sysTime);
	// LASTEXPIRATIONFETCHED_END
	// LASTHITEXPIRED_BEGIN
	bool get_LastHitExpired(void);
	// LASTHITEXPIRED_END
	// LASTRESOURCEFETCHED_BEGIN
	void get_LastKeyFetched(CkString &str);
	const char *lastKeyFetched(void);
	// LASTRESOURCEFETCHED_END
	// LEVEL_BEGIN
	int get_Level(void);
	void put_Level(int newVal);
	// LEVEL_END
	// NUMROOTS_BEGIN
	int get_NumRoots(void);
	// NUMROOTS_END
	// ADDROOT_BEGIN
	void AddRoot(const char *path);
	// ADDROOT_END
	// DELETEALL_BEGIN
	int DeleteAll(void);
	// DELETEALL_END
	// DELETEALLEXPIRED_BEGIN
	int DeleteAllExpired(void);
	// DELETEALLEXPIRED_END
	// DELETEFROMCACHE_BEGIN
	bool DeleteFromCache(const char *url);
	// DELETEFROMCACHE_END
	// DELETEOLDER_BEGIN
	int DeleteOlder(SYSTEMTIME &dt);
	// DELETEOLDER_END
	// FETCHFROMCACHE_BEGIN
	bool FetchFromCache(const char *url, CkByteData &outBytes);
	// FETCHFROMCACHE_END
	// GETETAG_BEGIN
	bool GetEtag(const char *url, CkString &outStr);
	const char *getEtag(const char *url);
	// GETETAG_END
	// GETEXPIRATION_BEGIN
	bool GetExpiration(const char *url, SYSTEMTIME &sysTime);
	// GETEXPIRATION_END
	// GETFILENAME_BEGIN
	bool GetFilename(const char *url, CkString &outStr);
	const char *getFilename(const char *url);
	// GETFILENAME_END
	// GETROOT_BEGIN
	bool GetRoot(int index, CkString &outStr);
	const char *getRoot(int index);
	// GETROOT_END
	// ISCACHED_BEGIN
	bool IsCached(const char *url);
	// ISCACHED_END
	// SAVETOCACHE_BEGIN
	bool SaveToCache(const char *url, SYSTEMTIME &expire, const char *eTag, CkByteData &data);
	// SAVETOCACHE_END
	// SAVETOCACHENOEXPIRE_BEGIN
	bool SaveToCacheNoExpire(const char *url, const char *eTag, CkByteData &data);
	// SAVETOCACHENOEXPIRE_END
	// UPDATEEXPIRATION_BEGIN
	bool UpdateExpiration(const char *url, SYSTEMTIME &dt);
	// UPDATEEXPIRATION_END
	// FETCHTEXT_BEGIN
	bool FetchText(const char *key, CkString &outStr);
	const char *fetchText(const char *key);
	// FETCHTEXT_END
	// SAVETEXT_BEGIN
	bool SaveText(const char *key, SYSTEMTIME &expire, const char *eTag, const char *strData);
	// SAVETEXT_END
	// SAVETEXTNOEXPIRE_BEGIN
	bool SaveTextNoExpire(const char *key, const char *eTag, const char *strData);
	// SAVETEXTNOEXPIRE_END

	// CACHE_INSERT_POINT

	// END PUBLIC INTERFACE

	CkString m_resultString;
	


};
#pragma pack (pop)


#endif


