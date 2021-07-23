#ifndef _CkPrivateKeyW_H
#define _CkPrivateKeyW_H
#pragma once

#include "CkString.h"
#include "CkPrivateKey.h"


#pragma pack (push, 8) 


class CkPrivateKeyW : public CkPrivateKey
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
	CkPrivateKeyW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkPrivateKeyW() { }
	bool getPkcs8EncryptedW(const wchar_t *w1,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetPkcs8Encrypted(sTemp1.getUtf8(),outBytes);
		}
	bool getPkcs8EncryptedPemW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetPkcs8EncryptedPem(sTemp1.getUtf8(),outStr);
		}
	bool loadEncryptedPemW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->LoadEncryptedPem(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool loadEncryptedPemFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->LoadEncryptedPemFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool loadPemW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadPem(sTemp1.getUtf8());
		}
	bool loadPemFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadPemFile(sTemp1.getUtf8());
		}
	bool loadPkcs8EncryptedW(CkByteData &data,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadPkcs8Encrypted(data,sTemp1.getUtf8());
		}
	bool loadPkcs8EncryptedFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->LoadPkcs8EncryptedFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool loadPkcs8FileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadPkcs8File(sTemp1.getUtf8());
		}
	bool loadPvkW(CkByteData &data,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadPvk(data,sTemp1.getUtf8());
		}
	bool loadPvkFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->LoadPvkFile(sTemp1.getUtf8(),sTemp2.getUtf8());
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
	bool savePkcs8EncryptedFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SavePkcs8EncryptedFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool savePkcs8EncryptedPemFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SavePkcs8EncryptedPemFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool savePkcs8FileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SavePkcs8File(sTemp1.getUtf8());
		}
	bool savePkcs8PemFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SavePkcs8PemFile(sTemp1.getUtf8());
		}
	bool saveRsaDerFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveRsaDerFile(sTemp1.getUtf8());
		}
	bool saveRsaPemFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveRsaPemFile(sTemp1.getUtf8());
		}
	bool saveXmlFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveXmlFile(sTemp1.getUtf8());
		}
	const wchar_t *getPkcs8EncryptedPemW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetPkcs8EncryptedPem(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getPkcs8PemW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetPkcs8Pem(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getRsaPemW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetRsaPem(m_resultStringW[idx]);
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
