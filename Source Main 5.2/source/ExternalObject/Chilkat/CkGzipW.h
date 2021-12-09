#ifndef _CkGzipW_H
#define _CkGzipW_H
#pragma once

#include "CkString.h"
#include "CkGzip.h"


#pragma pack (push, 8) 


class CkGzipW : public CkGzip
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
	CkGzipW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkGzipW() { }
	void put_CommentW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Comment(s1.getUtf8());
	    }
	void put_FilenameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Filename(s1.getUtf8());
	    }
	bool compressFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->CompressFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool compressFile2W(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->CompressFile2(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool compressFileToMemW(const wchar_t *w1,CkByteData &db)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CompressFileToMem(sTemp1.getUtf8(),db);
		}
	bool compressMemToFileW(const CkByteData &db,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CompressMemToFile(db,sTemp1.getUtf8());
		}
	bool compressStringW(const wchar_t *w1,const wchar_t *w2,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->CompressString(sTemp1.getUtf8(),sTemp2.getUtf8(),outBytes);
		}
	bool compressStringToFileW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->CompressStringToFile(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool decodeW(const wchar_t *w1,const wchar_t *w2,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->Decode(sTemp1.getUtf8(),sTemp2.getUtf8(),outBytes);
		}
	void deflateStringENCW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,CkString &strOut)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		this->DeflateStringENC(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),strOut);
		}
	bool encodeW(CkByteData &byteData,const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Encode(byteData,sTemp1.getUtf8(),outStr);
		}
	bool examineFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ExamineFile(sTemp1.getUtf8());
		}
	void inflateStringENCW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,CkString &strOut)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		this->InflateStringENC(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),strOut);
		}
	bool readFileW(const wchar_t *w1,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ReadFile(sTemp1.getUtf8(),outBytes);
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool unTarGzW(const wchar_t *w1,const wchar_t *w2,bool bNoAbsolute)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->UnTarGz(sTemp1.getUtf8(),sTemp2.getUtf8(),bNoAbsolute);
		}
	bool uncompressFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->UncompressFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool uncompressFileToMemW(const wchar_t *w1,CkByteData &db)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UncompressFileToMem(sTemp1.getUtf8(),db);
		}
	bool uncompressFileToStringW(const wchar_t *w1,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->UncompressFileToString(sTemp1.getUtf8(),sTemp2.getUtf8(),outStr);
		}
	bool uncompressMemToFileW(const CkByteData &db,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UncompressMemToFile(db,sTemp1.getUtf8());
		}
	bool uncompressStringW(CkByteData &inData,const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UncompressString(inData,sTemp1.getUtf8(),outStr);
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	bool writeFileW(const wchar_t *w1,CkByteData &binaryData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->WriteFile(sTemp1.getUtf8(),binaryData);
		}
	bool xfdlToXmlW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->XfdlToXml(sTemp1.getUtf8(),outStr);
		}
	const wchar_t *commentW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Comment(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *deflateStringENCW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		this->DeflateStringENC(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *encodeW(CkByteData &byteData,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->Encode(byteData,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *filenameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Filename(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *inflateStringENCW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		this->InflateStringENC(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),m_resultStringW[idx]);
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
	const wchar_t *uncompressFileToStringW(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->UncompressFileToString(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *uncompressStringW(CkByteData &inData,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->UncompressString(inData,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *versionW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Version(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *xfdlToXmlW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->XfdlToXml(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
