#ifndef _CkDsaW_H
#define _CkDsaW_H
#pragma once

#include "CkString.h"
#include "CkDsa.h"


#pragma pack (push, 8) 


class CkDsaW : public CkDsa
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
	CkDsaW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkDsaW() { }
	bool fromDerFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FromDerFile(sTemp1.getUtf8());
		}
	bool fromEncryptedPemW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->FromEncryptedPem(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool fromPemW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FromPem(sTemp1.getUtf8());
		}
	bool fromPublicDerFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FromPublicDerFile(sTemp1.getUtf8());
		}
	bool fromPublicPemW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FromPublicPem(sTemp1.getUtf8());
		}
	bool fromXmlW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FromXml(sTemp1.getUtf8());
		}
	bool genKeyFromParamsDerFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GenKeyFromParamsDerFile(sTemp1.getUtf8());
		}
	bool genKeyFromParamsPemW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GenKeyFromParamsPem(sTemp1.getUtf8());
		}
	bool genKeyFromParamsPemFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GenKeyFromParamsPemFile(sTemp1.getUtf8());
		}
	bool getEncodedHashW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetEncodedHash(sTemp1.getUtf8(),outStr);
		}
	bool getEncodedSignatureW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetEncodedSignature(sTemp1.getUtf8(),outStr);
		}
	bool loadTextW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadText(sTemp1.getUtf8(),outStr);
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool saveTextW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SaveText(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool setEncodedHashW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SetEncodedHash(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool setEncodedSignatureW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SetEncodedSignature(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool setKeyExplicitW(int groupSizeInBytes,const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,const wchar_t *w4)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		CkString sTemp4; sTemp4.appendU(w4);
		return this->SetKeyExplicit(groupSizeInBytes,sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),sTemp4.getUtf8());
		}
	bool setPubKeyExplicitW(int groupSizeInBytes,const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,const wchar_t *w4)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		CkString sTemp4; sTemp4.appendU(w4);
		return this->SetPubKeyExplicit(groupSizeInBytes,sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),sTemp4.getUtf8());
		}
	bool toDerFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ToDerFile(sTemp1.getUtf8());
		}
	bool toEncryptedPemW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ToEncryptedPem(sTemp1.getUtf8(),outStr);
		}
	bool toPublicDerFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ToPublicDerFile(sTemp1.getUtf8());
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	const wchar_t *getEncodedHashW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetEncodedHash(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getEncodedSignatureW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetEncodedSignature(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *hexGW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_HexG(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *hexPW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_HexP(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *hexQW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_HexQ(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *hexXW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_HexX(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *hexYW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_HexY(m_resultStringW[idx]);
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
	const wchar_t *loadTextW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->LoadText(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *toEncryptedPemW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->ToEncryptedPem(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *toPemW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->ToPem(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *toPublicPemW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->ToPublicPem(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *toXmlW(bool bPublicOnly)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->ToXml(bPublicOnly,m_resultStringW[idx]);
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
