#ifndef _CkCertStoreW_H
#define _CkCertStoreW_H
#pragma once

#include "CkString.h"
#include "CkCertStore.h"


#pragma pack (push, 8) 


class CkCertStoreW : public CkCertStore
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
	CkCertStoreW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkCertStoreW() { }
	bool createFileStoreW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CreateFileStore(sTemp1.getUtf8());
		}
	bool createRegistryStoreW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->CreateRegistryStore(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	CkCert *findCertByRfc822NameW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FindCertByRfc822Name(sTemp1.getUtf8());
		}
	CkCert *findCertBySerialW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FindCertBySerial(sTemp1.getUtf8());
		}
	CkCert *findCertBySha1ThumbprintW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FindCertBySha1Thumbprint(sTemp1.getUtf8());
		}
	CkCert *findCertBySubjectW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FindCertBySubject(sTemp1.getUtf8());
		}
	CkCert *findCertBySubjectCNW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FindCertBySubjectCN(sTemp1.getUtf8());
		}
	CkCert *findCertBySubjectEW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FindCertBySubjectE(sTemp1.getUtf8());
		}
	CkCert *findCertBySubjectOW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FindCertBySubjectO(sTemp1.getUtf8());
		}
	CkCert *findCertForEmailW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FindCertForEmail(sTemp1.getUtf8());
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
	bool openFileStoreW(const wchar_t *w1,bool readOnly)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpenFileStore(sTemp1.getUtf8(),readOnly);
		}
	bool openRegistryStoreW(const wchar_t *w1,const wchar_t *w2,bool readOnly)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->OpenRegistryStore(sTemp1.getUtf8(),sTemp2.getUtf8(),readOnly);
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
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
	const wchar_t *versionW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Version(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
