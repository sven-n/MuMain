#ifndef _CkCacheW_H
#define _CkCacheW_H
#pragma once

#include "CkString.h"
#include "CkCache.h"


#pragma pack (push, 8) 


class CkCacheW : public CkCache
{
    private:
	unsigned long nextIdxW(void)
	    {
	    m_resultIdxW++;
	    if (m_resultIdxW >= 10)
		{
		m_resultIdxW = 0;
		}
	    return m_resultIdxW;
	    }
	unsigned long m_resultIdxW;
	CkString m_resultStringW[10];

    public:
	CkCacheW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkCacheW() { }
	void addRootW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddRoot(sTemp1.getUtf8());
		}
	bool deleteFromCacheW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DeleteFromCache(sTemp1.getUtf8());
		}
	bool fetchFromCacheW(const wchar_t *w1,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FetchFromCache(sTemp1.getUtf8(),outBytes);
		}
	bool fetchTextW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FetchText(sTemp1.getUtf8(),outStr);
		}
	bool getEtagW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetEtag(sTemp1.getUtf8(),outStr);
		}
	bool getExpirationW(const wchar_t *w1,SYSTEMTIME &sysTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetExpiration(sTemp1.getUtf8(),sysTime);
		}
	bool getFilenameW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetFilename(sTemp1.getUtf8(),outStr);
		}
	bool isCachedW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->IsCached(sTemp1.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool saveTextW(const wchar_t *w1,SYSTEMTIME &expire,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->SaveText(sTemp1.getUtf8(),expire,sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool saveTextNoExpireW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->SaveTextNoExpire(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool saveToCacheW(const wchar_t *w1,SYSTEMTIME &expire,const wchar_t *w2,CkByteData &data)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SaveToCache(sTemp1.getUtf8(),expire,sTemp2.getUtf8(),data);
		}
	bool saveToCacheNoExpireW(const wchar_t *w1,const wchar_t *w2,CkByteData &data)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SaveToCacheNoExpire(sTemp1.getUtf8(),sTemp2.getUtf8(),data);
		}
	bool updateExpirationW(const wchar_t *w1,SYSTEMTIME &dt)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UpdateExpiration(sTemp1.getUtf8(),dt);
		}
	const wchar_t *fetchTextW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->FetchText(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getEtagW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetEtag(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getFilenameW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetFilename(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getRootW(int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetRoot(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastErrorHtmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->LastErrorHtml(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastErrorTextW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->LastErrorText(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastErrorXmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->LastErrorXml(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastEtagFetchedW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastEtagFetched(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastKeyFetchedW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastKeyFetched(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
