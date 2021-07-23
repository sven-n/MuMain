#ifndef _CkRsaW_H
#define _CkRsaW_H
#pragma once

#include "CkString.h"
#include "CkRsa.h"


#pragma pack (push, 8) 


class CkRsaW : public CkRsa
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
	CkRsaW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkRsaW() { }
	void put_CharsetW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Charset(s1.getUtf8());
	    }
	void put_EncodingModeW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_EncodingMode(s1.getUtf8());
	    }
	bool decryptBytesENCW(const wchar_t *w1,bool bUsePrivateKey,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DecryptBytesENC(sTemp1.getUtf8(),bUsePrivateKey,out);
		}
	bool decryptStringENCW(const wchar_t *w1,bool bUsePrivateKey,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DecryptStringENC(sTemp1.getUtf8(),bUsePrivateKey,out);
		}
	bool encryptStringW(const wchar_t *w1,bool bUsePrivateKey,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->EncryptString(sTemp1.getUtf8(),bUsePrivateKey,out);
		}
	bool encryptStringENCW(const wchar_t *w1,bool bUsePrivateKey,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->EncryptStringENC(sTemp1.getUtf8(),bUsePrivateKey,out);
		}
	bool importPrivateKeyW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ImportPrivateKey(sTemp1.getUtf8());
		}
	bool importPublicKeyW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ImportPublicKey(sTemp1.getUtf8());
		}
	bool openSslSignStringW(const wchar_t *w1,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpenSslSignString(sTemp1.getUtf8(),outBytes);
		}
	bool openSslSignStringENCW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpenSslSignStringENC(sTemp1.getUtf8(),outStr);
		}
	bool openSslVerifyBytesENCW(const wchar_t *w1,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpenSslVerifyBytesENC(sTemp1.getUtf8(),outBytes);
		}
	bool openSslVerifyStringENCW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpenSslVerifyStringENC(sTemp1.getUtf8(),outStr);
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool signBytesW(const CkByteData &bData,const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SignBytes(bData,sTemp1.getUtf8(),out);
		}
	bool signBytesENCW(const CkByteData &bData,const wchar_t *w1,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SignBytesENC(bData,sTemp1.getUtf8(),out);
		}
	bool signStringW(const wchar_t *w1,const wchar_t *w2,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SignString(sTemp1.getUtf8(),sTemp2.getUtf8(),out);
		}
	bool signStringENCW(const wchar_t *w1,const wchar_t *w2,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SignStringENC(sTemp1.getUtf8(),sTemp2.getUtf8(),out);
		}
	bool snkToXmlW(const wchar_t *w1,CkString &strXml)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SnkToXml(sTemp1.getUtf8(),strXml);
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	bool verifyBytesW(const CkByteData &bData,const wchar_t *w1,const CkByteData &sigData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->VerifyBytes(bData,sTemp1.getUtf8(),sigData);
		}
	bool verifyBytesENCW(const CkByteData &bData,const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->VerifyBytesENC(bData,sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool verifyPrivateKeyW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->VerifyPrivateKey(sTemp1.getUtf8());
		}
	bool verifyStringW(const wchar_t *w1,const wchar_t *w2,const CkByteData &sigData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->VerifyString(sTemp1.getUtf8(),sTemp2.getUtf8(),sigData);
		}
	bool verifyStringENCW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->VerifyStringENC(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	const wchar_t *charsetW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Charset(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *decryptStringW(const CkByteData &bData,bool bUsePrivateKey)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->DecryptString(bData,bUsePrivateKey,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *decryptStringENCW(const wchar_t *w1,bool bUsePrivateKey)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->DecryptStringENC(sTemp1.getUtf8(),bUsePrivateKey,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *encodingModeW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_EncodingMode(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *encryptBytesENCW(const CkByteData &bData,bool bUsePrivateKey)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->EncryptBytesENC(bData,bUsePrivateKey,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *encryptStringENCW(const wchar_t *w1,bool bUsePrivateKey)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->EncryptStringENC(sTemp1.getUtf8(),bUsePrivateKey,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *exportPrivateKeyW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->ExportPrivateKey(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *exportPublicKeyW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->ExportPublicKey(m_resultStringW[idx]);
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
	const wchar_t *openSslSignBytesENCW(CkByteData &data)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->OpenSslSignBytesENC(data,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *openSslSignStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->OpenSslSignStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *openSslVerifyStringW(CkByteData &data)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->OpenSslVerifyString(data,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *openSslVerifyStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->OpenSslVerifyStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *signBytesENCW(const CkByteData &bData,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->SignBytesENC(bData,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *signStringENCW(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->SignStringENC(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *snkToXmlW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->SnkToXml(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *versionW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Version(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
