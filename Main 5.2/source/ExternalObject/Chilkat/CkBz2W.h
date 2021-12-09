#ifndef _CkBz2W_H
#define _CkBz2W_H
#pragma once

#include "CkString.h"
#include "CkBz2.h"


#pragma pack (push, 8) 


class CkBz2W : public CkBz2
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
	CkBz2W() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkBz2W() { }
	bool compressFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->CompressFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool compressFileToMemW(const wchar_t *w1,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CompressFileToMem(sTemp1.getUtf8(),outBytes);
		}
	bool compressMemToFileW(CkByteData &inData,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CompressMemToFile(inData,sTemp1.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool uncompressFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->UncompressFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool uncompressFileToMemW(const wchar_t *w1,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UncompressFileToMem(sTemp1.getUtf8(),outBytes);
		}
	bool uncompressMemToFileW(CkByteData &inData,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UncompressMemToFile(inData,sTemp1.getUtf8());
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
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
