#ifndef _CkCharsetW_H
#define _CkCharsetW_H
#pragma once

#include "CkString.h"
#include "CkCharset.h"


#pragma pack (push, 8) 


class CkCharsetW : public CkCharset
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
	CkCharsetW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkCharsetW() { }
	void put_AltToCharsetW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_AltToCharset(s1.getUtf8());
	    }
	void put_FromCharsetW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_FromCharset(s1.getUtf8());
	    }
	void put_ToCharsetW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ToCharset(s1.getUtf8());
	    }
	int charsetToCodePageW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CharsetToCodePage(sTemp1.getUtf8());
		}
	bool convertFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ConvertFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool convertHtmlFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ConvertHtmlFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void entityEncodeDecW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->EntityEncodeDec(sTemp1.getUtf8(),outStr);
		}
	void entityEncodeHexW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->EntityEncodeHex(sTemp1.getUtf8(),outStr);
		}
	bool getHtmlFileCharsetW(const wchar_t *w1,CkString &strCharset)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetHtmlFileCharset(sTemp1.getUtf8(),strCharset);
		}
	bool htmlDecodeToStrW(const wchar_t *w1,CkString &strOut)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HtmlDecodeToStr(sTemp1.getUtf8(),strOut);
		}
	bool htmlEntityDecodeFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->HtmlEntityDecodeFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void lowerCaseW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->LowerCase(sTemp1.getUtf8(),outStr);
		}
	bool readFileW(const wchar_t *w1,CkByteData &dataBuf)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ReadFile(sTemp1.getUtf8(),dataBuf);
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	void setErrorStringW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->SetErrorString(sTemp1.getUtf8());
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	void upperCaseW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->UpperCase(sTemp1.getUtf8(),outStr);
		}
	bool verifyDataW(const wchar_t *w1,const CkByteData &charData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->VerifyData(sTemp1.getUtf8(),charData);
		}
	bool verifyFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->VerifyFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool writeFileW(const wchar_t *w1,const CkByteData &dataBuf)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->WriteFile(sTemp1.getUtf8(),dataBuf);
		}
	bool writeFileW(const wchar_t *w1,const unsigned char *data,unsigned long dataLen)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->WriteFile(sTemp1.getUtf8(),data,dataLen);
		}
	const wchar_t *altToCharsetW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_AltToCharset(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *codePageToCharsetW(int codePage)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->CodePageToCharset(codePage,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *entityEncodeDecW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->EntityEncodeDec(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *entityEncodeHexW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->EntityEncodeHex(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *fromCharsetW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_FromCharset(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getHtmlCharsetW(const CkByteData &htmlData)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetHtmlCharset(htmlData,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getHtmlCharsetW(const unsigned char *htmlData,unsigned long htmlDataLen)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetHtmlCharset(htmlData,htmlDataLen,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getHtmlFileCharsetW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetHtmlFileCharset(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *htmlDecodeToStrW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->HtmlDecodeToStr(sTemp1.getUtf8(),m_resultStringW[idx]);
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
	const wchar_t *lastInputAsHexW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastInputAsHex(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastInputAsQPW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastInputAsQP(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastOutputAsHexW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastOutputAsHex(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastOutputAsQPW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastOutputAsQP(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lowerCaseW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->LowerCase(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *toCharsetW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ToCharset(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *upperCaseW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->UpperCase(sTemp1.getUtf8(),m_resultStringW[idx]);
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
