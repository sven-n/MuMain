#ifndef _CkCSPW_H
#define _CkCSPW_H
#pragma once

#include "CkString.h"
#include "CkCSP.h"


#pragma pack (push, 8) 


class CkCSPW : public CkCSP
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
	CkCSPW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkCSPW() { }
	void put_KeyContainerNameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_KeyContainerName(s1.getUtf8());
	    }
	void put_ProviderNameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ProviderName(s1.getUtf8());
	    }
	bool hasEncryptAlgorithmW(const wchar_t *w1,long numBits)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HasEncryptAlgorithm(sTemp1.getUtf8(),numBits);
		}
	bool hasHashAlgorithmW(const wchar_t *w1,long numBits)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HasHashAlgorithm(sTemp1.getUtf8(),numBits);
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	long setEncryptAlgorithmW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetEncryptAlgorithm(sTemp1.getUtf8());
		}
	long setHashAlgorithmW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetHashAlgorithm(sTemp1.getUtf8());
		}
	const wchar_t *encryptAlgorithmW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_EncryptAlgorithm(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getEncryptionAlgorithmW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetEncryptionAlgorithm(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getHashAlgorithmW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetHashAlgorithm(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getKeyContainerNameW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetKeyContainerName(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getKeyExchangeAlgorithmW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetKeyExchangeAlgorithm(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getSignatureAlgorithmW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetSignatureAlgorithm(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *hashAlgorithmW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_HashAlgorithm(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *keyContainerNameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_KeyContainerName(m_resultStringW[idx]);
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
	const wchar_t *providerNameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ProviderName(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
