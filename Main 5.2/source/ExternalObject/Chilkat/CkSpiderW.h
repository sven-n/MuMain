#ifndef _CkSpiderW_H
#define _CkSpiderW_H
#pragma once

#include "CkString.h"
#include "CkSpider.h"


#pragma pack (push, 8) 


class CkSpiderW : public CkSpider
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
	CkSpiderW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkSpiderW() { }
	void put_CacheDirW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_CacheDir(s1.getUtf8());
	    }
	void put_ProxyDomainW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ProxyDomain(s1.getUtf8());
	    }
	void put_ProxyLoginW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ProxyLogin(s1.getUtf8());
	    }
	void put_ProxyPasswordW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ProxyPassword(s1.getUtf8());
	    }
	void put_UserAgentW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_UserAgent(s1.getUtf8());
	    }
	void addAvoidOutboundLinkPatternW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddAvoidOutboundLinkPattern(sTemp1.getUtf8());
		}
	void addAvoidPatternW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddAvoidPattern(sTemp1.getUtf8());
		}
	void addMustMatchPatternW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddMustMatchPattern(sTemp1.getUtf8());
		}
	void addUnspideredW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddUnspidered(sTemp1.getUtf8());
		}
	void canonicalizeUrlW(const wchar_t *w1,CkString &urlOut)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->CanonicalizeUrl(sTemp1.getUtf8(),urlOut);
		}
	void getBaseDomainW(const wchar_t *w1,CkString &domainOut)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetBaseDomain(sTemp1.getUtf8(),domainOut);
		}
	void getDomainW(const wchar_t *w1,CkString &domainOut)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetDomain(sTemp1.getUtf8(),domainOut);
		}
	void initializeW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->Initialize(sTemp1.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	const wchar_t *avoidPatternW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetAvoidPattern(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *cacheDirW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_CacheDir(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *canonicalizeUrlW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->CanonicalizeUrl(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *domainW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Domain(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *failedUrlW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetFailedUrl(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *fetchRobotsTextW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->FetchRobotsText(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getAvoidPatternW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetAvoidPattern(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getBaseDomainW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetBaseDomain(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getDomainW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetDomain(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getFailedUrlW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetFailedUrl(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getOutboundLinkW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetOutboundLink(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getSpideredUrlW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetSpideredUrl(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getUnspideredUrlW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetUnspideredUrl(index,m_resultStringW[idx]);
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
	const wchar_t *lastHtmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastHtml(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastHtmlDescriptionW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastHtmlDescription(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastHtmlKeywordsW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastHtmlKeywords(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastHtmlTitleW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastHtmlTitle(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastModDateStrW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastModDateStr(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastUrlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastUrl(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *outboundLinkW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetOutboundLink(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *proxyDomainW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ProxyDomain(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *proxyLoginW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ProxyLogin(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *proxyPasswordW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ProxyPassword(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *spideredUrlW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetSpideredUrl(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *unspideredUrlW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetUnspideredUrl(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *userAgentW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_UserAgent(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
