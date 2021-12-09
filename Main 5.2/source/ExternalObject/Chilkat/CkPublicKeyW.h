#ifndef _CkPublicKeyW_H
#define _CkPublicKeyW_H
#pragma once

#include "CkString.h"
#include "CkPublicKey.h"


#pragma pack (push, 8) 


class CkPublicKeyW : public CkPublicKey
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
	CkPublicKeyW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkPublicKeyW() { }
	bool loadOpenSslDerFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadOpenSslDerFile(sTemp1.getUtf8());
		}
	bool loadOpenSslPemW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadOpenSslPem(sTemp1.getUtf8());
		}
	bool loadOpenSslPemFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadOpenSslPemFile(sTemp1.getUtf8());
		}
	bool loadPkcs1PemW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadPkcs1Pem(sTemp1.getUtf8());
		}
	bool loadRsaDerFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadRsaDerFile(sTemp1.getUtf8());
		}
	bool loadXmlW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadXml(sTemp1.getUtf8());
		}
	bool loadXmlFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadXmlFile(sTemp1.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool saveOpenSslDerFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveOpenSslDerFile(sTemp1.getUtf8());
		}
	bool saveOpenSslPemFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveOpenSslPemFile(sTemp1.getUtf8());
		}
	bool saveRsaDerFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveRsaDerFile(sTemp1.getUtf8());
		}
	bool saveXmlFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveXmlFile(sTemp1.getUtf8());
		}
	const wchar_t *getOpenSslPemW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetOpenSslPem(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getXmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetXml(m_resultStringW[idx]);
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

};
#endif
