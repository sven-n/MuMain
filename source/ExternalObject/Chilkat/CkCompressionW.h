#ifndef _CkCompressionW_H
#define _CkCompressionW_H
#pragma once

#include "CkString.h"
#include "CkCompression.h"


#pragma pack (push, 8) 


class CkCompressionW : public CkCompression
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
	CkCompressionW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkCompressionW() { }
	void put_AlgorithmW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Algorithm(s1.getUtf8());
	    }
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
	bool beginCompressStringW(const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->BeginCompressString(sTemp1.getUtf8(),out);
		}
	bool beginCompressStringENCW(const wchar_t *w1,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->BeginCompressStringENC(sTemp1.getUtf8(),out);
		}
	bool beginDecompressBytesENCW(const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->BeginDecompressBytesENC(sTemp1.getUtf8(),out);
		}
	bool beginDecompressStringENCW(const wchar_t *w1,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->BeginDecompressStringENC(sTemp1.getUtf8(),out);
		}
	bool compressFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->CompressFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool compressStringW(const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CompressString(sTemp1.getUtf8(),out);
		}
	bool compressStringENCW(const wchar_t *w1,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CompressStringENC(sTemp1.getUtf8(),out);
		}
	bool decompressBytesENCW(const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DecompressBytesENC(sTemp1.getUtf8(),out);
		}
	bool decompressFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->DecompressFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool decompressStringENCW(const wchar_t *w1,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DecompressStringENC(sTemp1.getUtf8(),out);
		}
	bool moreCompressStringW(const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->MoreCompressString(sTemp1.getUtf8(),out);
		}
	bool moreCompressStringENCW(const wchar_t *w1,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->MoreCompressStringENC(sTemp1.getUtf8(),out);
		}
	bool moreDecompressBytesENCW(const wchar_t *w1,CkByteData &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->MoreDecompressBytesENC(sTemp1.getUtf8(),out);
		}
	bool moreDecompressStringENCW(const wchar_t *w1,CkString &out)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->MoreDecompressStringENC(sTemp1.getUtf8(),out);
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
	const wchar_t *algorithmW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Algorithm(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *beginCompressBytesENCW(const CkByteData &bData)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->BeginCompressBytesENC(bData,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *beginCompressStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->BeginCompressStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *beginDecompressStringW(const CkByteData &bData)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->BeginDecompressString(bData,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *beginDecompressStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->BeginDecompressStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *charsetW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Charset(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *compressBytesENCW(const CkByteData &bData)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->CompressBytesENC(bData,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *compressStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->CompressStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *decompressStringW(const CkByteData &bData)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->DecompressString(bData,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *decompressStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->DecompressStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *encodingModeW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_EncodingMode(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *endCompressBytesENCW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->EndCompressBytesENC(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *endCompressStringENCW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->EndCompressStringENC(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *endDecompressStringW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->EndDecompressString(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *endDecompressStringENCW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->EndDecompressStringENC(m_resultStringW[idx]);
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
	const wchar_t *moreCompressBytesENCW(const CkByteData &bData)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->MoreCompressBytesENC(bData,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *moreCompressStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->MoreCompressStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *moreDecompressStringW(const CkByteData &bData)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->MoreDecompressString(bData,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *moreDecompressStringENCW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->MoreDecompressStringENC(sTemp1.getUtf8(),m_resultStringW[idx]);
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
