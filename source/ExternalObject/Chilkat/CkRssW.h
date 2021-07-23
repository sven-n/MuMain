#ifndef _CkRssW_H
#define _CkRssW_H
#pragma once

#include "CkString.h"
#include "CkRss.h"


#pragma pack (push, 8) 


class CkRssW : public CkRss
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
	CkRssW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkRssW() { }
	bool downloadRssW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DownloadRss(sTemp1.getUtf8());
		}
	bool getAttrW(const wchar_t *w1,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->GetAttr(sTemp1.getUtf8(),sTemp2.getUtf8(),outStr);
		}
	int getCountW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetCount(sTemp1.getUtf8());
		}
	bool getDateW(const wchar_t *w1,SYSTEMTIME &sysTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetDate(sTemp1.getUtf8(),sysTime);
		}
	int getIntW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetInt(sTemp1.getUtf8());
		}
	bool getStringW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetString(sTemp1.getUtf8(),outStr);
		}
	bool loadRssFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadRssFile(sTemp1.getUtf8());
		}
	bool loadRssStringW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadRssString(sTemp1.getUtf8());
		}
	bool mGetAttrW(const wchar_t *w1,int index,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->MGetAttr(sTemp1.getUtf8(),index,sTemp2.getUtf8(),outStr);
		}
	bool mGetStringW(const wchar_t *w1,int index,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->MGetString(sTemp1.getUtf8(),index,outStr);
		}
	bool mSetAttrW(const wchar_t *w1,int index,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->MSetAttr(sTemp1.getUtf8(),index,sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool mSetStringW(const wchar_t *w1,int index,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->MSetString(sTemp1.getUtf8(),index,sTemp2.getUtf8());
		}
	void removeW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->Remove(sTemp1.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	void setAttrW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		this->SetAttr(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	void setDateW(const wchar_t *w1,SYSTEMTIME &d)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->SetDate(sTemp1.getUtf8(),d);
		}
	void setDateNowW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->SetDateNow(sTemp1.getUtf8());
		}
	void setIntW(const wchar_t *w1,int value)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->SetInt(sTemp1.getUtf8(),value);
		}
	void setStringW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->SetString(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	const wchar_t *getAttrW(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->GetAttr(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getStringW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetString(sTemp1.getUtf8(),m_resultStringW[idx]);
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
	const wchar_t *mGetAttrW(const wchar_t *w1,int index,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->MGetAttr(sTemp1.getUtf8(),index,sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *mGetStringW(const wchar_t *w1,int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->MGetString(sTemp1.getUtf8(),index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *toXmlStringW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->ToXmlString(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
