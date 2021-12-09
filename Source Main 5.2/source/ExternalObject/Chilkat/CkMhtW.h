#ifndef _CkMhtW_H
#define _CkMhtW_H
#pragma once

#include "CkString.h"
#include "CkMht.h"


#pragma pack (push, 8) 


class CkMhtW : public CkMht
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
	CkMhtW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkMhtW() { }
	void put_BaseUrlW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_BaseUrl(s1.getUtf8());
	    }
	void put_DebugHtmlAfterW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_DebugHtmlAfter(s1.getUtf8());
	    }
	void put_DebugHtmlBeforeW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_DebugHtmlBefore(s1.getUtf8());
	    }
	void put_ProxyW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Proxy(s1.getUtf8());
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
	void put_SocksHostnameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_SocksHostname(s1.getUtf8());
	    }
	void put_SocksPasswordW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_SocksPassword(s1.getUtf8());
	    }
	void put_SocksUsernameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_SocksUsername(s1.getUtf8());
	    }
	void put_WebSiteLoginW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_WebSiteLogin(s1.getUtf8());
	    }
	void put_WebSitePasswordW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_WebSitePassword(s1.getUtf8());
	    }
	void addCacheRootW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddCacheRoot(sTemp1.getUtf8());
		}
	void addCustomHeaderW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->AddCustomHeader(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void addExternalStyleSheetW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddExternalStyleSheet(sTemp1.getUtf8());
		}
	void excludeImagesMatchingW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->ExcludeImagesMatching(sTemp1.getUtf8());
		}
	bool getAndSaveEMLW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->GetAndSaveEML(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool getAndSaveMHTW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->GetAndSaveMHT(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool getAndZipEMLW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->GetAndZipEML(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool getAndZipMHTW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->GetAndZipMHT(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool getEMLW(const wchar_t *w1,CkString &eml)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetEML(sTemp1.getUtf8(),eml);
		}
	CkEmail *getEmailW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetEmail(sTemp1.getUtf8());
		}
	bool getMHTW(const wchar_t *w1,CkString &mht)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetMHT(sTemp1.getUtf8(),mht);
		}
	CkMime *getMimeW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetMime(sTemp1.getUtf8());
		}
	bool htmlToEMLW(const wchar_t *w1,CkString &eml)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HtmlToEML(sTemp1.getUtf8(),eml);
		}
	bool htmlToEMLFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->HtmlToEMLFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	CkEmail *htmlToEmailW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HtmlToEmail(sTemp1.getUtf8());
		}
	bool htmlToMHTW(const wchar_t *w1,CkString &mht)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HtmlToMHT(sTemp1.getUtf8(),mht);
		}
	bool htmlToMHTFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->HtmlToMHTFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void removeCustomHeaderW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->RemoveCustomHeader(sTemp1.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	bool unpackMHTW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,const wchar_t *w4)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		CkString sTemp4; sTemp4.appendU(w4);
		return this->UnpackMHT(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),sTemp4.getUtf8());
		}
	bool unpackMHTStringW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,const wchar_t *w4)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		CkString sTemp4; sTemp4.appendU(w4);
		return this->UnpackMHTString(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),sTemp4.getUtf8());
		}
	const wchar_t *baseUrlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_BaseUrl(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *debugHtmlAfterW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_DebugHtmlAfter(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *debugHtmlBeforeW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_DebugHtmlBefore(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getCacheRootW(int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetCacheRoot(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getEMLW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetEML(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getMHTW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetMHT(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *htmlToEMLW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->HtmlToEML(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *htmlToMHTW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->HtmlToMHT(sTemp1.getUtf8(),m_resultStringW[idx]);
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
	const wchar_t *proxyW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Proxy(m_resultStringW[idx]);
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
	const wchar_t *socksHostnameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SocksHostname(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *socksPasswordW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SocksPassword(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *socksUsernameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SocksUsername(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *versionW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Version(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *webSiteLoginW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_WebSiteLogin(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *webSitePasswordW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_WebSitePassword(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
