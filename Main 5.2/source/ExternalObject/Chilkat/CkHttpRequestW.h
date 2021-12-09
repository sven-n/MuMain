#ifndef _CkHttpRequestW_H
#define _CkHttpRequestW_H
#pragma once

#include "CkString.h"
#include "CkHttpRequest.h"


#pragma pack (push, 8) 


class CkHttpRequestW : public CkHttpRequest
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
	CkHttpRequestW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkHttpRequestW() { }
	void put_CharsetW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Charset(s1.getUtf8());
	    }
	void put_HttpVersionW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_HttpVersion(s1.getUtf8());
	    }
	void put_PathW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Path(s1.getUtf8());
	    }
	bool addBytesForUploadW(const wchar_t *w1,const wchar_t *w2,CkByteData &byteData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddBytesForUpload(sTemp1.getUtf8(),sTemp2.getUtf8(),byteData);
		}
	bool addFileForUploadW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddFileForUpload(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void addHeaderW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->AddHeader(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void addParamW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->AddParam(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool getHeaderFieldW(const wchar_t *w1,CkString &str)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetHeaderField(sTemp1.getUtf8(),str);
		}
	bool getParamW(const wchar_t *w1,CkString &str)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetParam(sTemp1.getUtf8(),str);
		}
	bool loadBodyFromFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadBodyFromFile(sTemp1.getUtf8());
		}
	bool loadBodyFromStringW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->LoadBodyFromString(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void removeParamW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->RemoveParam(sTemp1.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	void setFromUrlW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->SetFromUrl(sTemp1.getUtf8());
		}
	void useXmlHttpW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->UseXmlHttp(sTemp1.getUtf8());
		}
	const wchar_t *charsetW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Charset(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *generateRequestTextW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GenerateRequestText(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getHeaderFieldW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetHeaderField(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getHeaderNameW(int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetHeaderName(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getHeaderValueW(int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetHeaderValue(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getParamW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetParam(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getParamNameW(int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetParamName(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getParamValueW(int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetParamValue(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *httpVersionW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_HttpVersion(m_resultStringW[idx]);
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
	const wchar_t *pathW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Path(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
