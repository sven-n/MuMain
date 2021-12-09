#ifndef _CkCertW_H
#define _CkCertW_H
#pragma once

#include "CkString.h"
#include "CkCert.h"


#pragma pack (push, 8) 


class CkCertW : public CkCert
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
	CkCertW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkCertW() { }
	bool exportCertDerFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ExportCertDerFile(sTemp1.getUtf8());
		}
	bool exportCertPemFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ExportCertPemFile(sTemp1.getUtf8());
		}
	bool exportToPfxFileW(const wchar_t *w1,const wchar_t *w2,bool bIncludeChain)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ExportToPfxFile(sTemp1.getUtf8(),sTemp2.getUtf8(),bIncludeChain);
		}
	bool linkPrivateKeyW(const wchar_t *w1,bool bMachineKeyset,bool bForSigning)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LinkPrivateKey(sTemp1.getUtf8(),bMachineKeyset,bForSigning);
		}
	bool loadByCommonNameW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadByCommonName(sTemp1.getUtf8());
		}
	bool loadByEmailAddressW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadByEmailAddress(sTemp1.getUtf8());
		}
	bool loadFromBase64W(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadFromBase64(sTemp1.getUtf8());
		}
	bool loadFromFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadFromFile(sTemp1.getUtf8());
		}
	bool loadPfxDataW(CkByteData &pfxData,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadPfxData(pfxData,sTemp1.getUtf8());
		}
	bool loadPfxData2W(const unsigned char *buf,unsigned long bufLen,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadPfxData2(buf,bufLen,sTemp1.getUtf8());
		}
	bool loadPfxFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->LoadPfxFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool pemFileToDerFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->PemFileToDerFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool saveToFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveToFile(sTemp1.getUtf8());
		}
	bool setFromEncodedW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetFromEncoded(sTemp1.getUtf8());
		}
	const wchar_t *cspNameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_CspName(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *exportCertPemW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->ExportCertPem(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getEncodedW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetEncoded(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *issuerCW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_IssuerC(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *issuerCNW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_IssuerCN(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *issuerDNW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_IssuerDN(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *issuerEW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_IssuerE(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *issuerLW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_IssuerL(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *issuerOW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_IssuerO(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *issuerOUW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_IssuerOU(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *issuerSW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_IssuerS(m_resultStringW[idx]);
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
	const wchar_t *rfc822NameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Rfc822Name(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *serialNumberW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SerialNumber(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *sha1ThumbprintW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Sha1Thumbprint(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *subjectCW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SubjectC(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *subjectCNW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SubjectCN(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *subjectDNW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SubjectDN(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *subjectEW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SubjectE(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *subjectLW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SubjectL(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *subjectOW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SubjectO(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *subjectOUW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SubjectOU(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *subjectSW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SubjectS(m_resultStringW[idx]);
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
