#ifndef _CkAtomW_H
#define _CkAtomW_H
#pragma once

#include "CkString.h"
#include "CkAtom.h"


#pragma pack (push, 8) 


class CkAtomW : public CkAtom
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
	CkAtomW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkAtomW() { }
	int addElementW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddElement(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	int addElementDateW(const wchar_t *w1,SYSTEMTIME &dateTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddElementDate(sTemp1.getUtf8(),dateTime);
		}
	int addElementHtmlW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddElementHtml(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	int addElementXHtmlW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddElementXHtml(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	int addElementXmlW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddElementXml(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void addEntryW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddEntry(sTemp1.getUtf8());
		}
	void addLinkW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,const wchar_t *w4)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		CkString sTemp4; sTemp4.appendU(w4);
		this->AddLink(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),sTemp4.getUtf8());
		}
	void addPersonW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,const wchar_t *w4)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		CkString sTemp4; sTemp4.appendU(w4);
		this->AddPerson(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),sTemp4.getUtf8());
		}
	void deleteElementW(const wchar_t *w1,int index)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->DeleteElement(sTemp1.getUtf8(),index);
		}
	void deleteElementAttrW(const wchar_t *w1,int index,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->DeleteElementAttr(sTemp1.getUtf8(),index,sTemp2.getUtf8());
		}
	void deletePersonW(const wchar_t *w1,int index)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->DeletePerson(sTemp1.getUtf8(),index);
		}
	bool downloadAtomW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DownloadAtom(sTemp1.getUtf8());
		}
	bool getElementW(const wchar_t *w1,int index,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetElement(sTemp1.getUtf8(),index,outStr);
		}
	bool getElementAttrW(const wchar_t *w1,int index,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->GetElementAttr(sTemp1.getUtf8(),index,sTemp2.getUtf8(),outStr);
		}
	int getElementCountW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetElementCount(sTemp1.getUtf8());
		}
	bool getElementDateW(const wchar_t *w1,int index,SYSTEMTIME &sysTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetElementDate(sTemp1.getUtf8(),index,sysTime);
		}
	bool getLinkHrefW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetLinkHref(sTemp1.getUtf8(),outStr);
		}
	bool getPersonInfoW(const wchar_t *w1,int index,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->GetPersonInfo(sTemp1.getUtf8(),index,sTemp2.getUtf8(),outStr);
		}
	bool getTopAttrW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetTopAttr(sTemp1.getUtf8(),outStr);
		}
	bool hasElementW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HasElement(sTemp1.getUtf8());
		}
	bool loadXmlW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadXml(sTemp1.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	void setElementAttrW(const wchar_t *w1,int index,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		this->SetElementAttr(sTemp1.getUtf8(),index,sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	void setTopAttrW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->SetTopAttr(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void updateElementW(const wchar_t *w1,int index,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->UpdateElement(sTemp1.getUtf8(),index,sTemp2.getUtf8());
		}
	void updateElementDateW(const wchar_t *w1,int index,SYSTEMTIME &dateTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->UpdateElementDate(sTemp1.getUtf8(),index,dateTime);
		}
	void updateElementHtmlW(const wchar_t *w1,int index,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->UpdateElementHtml(sTemp1.getUtf8(),index,sTemp2.getUtf8());
		}
	void updateElementXHtmlW(const wchar_t *w1,int index,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->UpdateElementXHtml(sTemp1.getUtf8(),index,sTemp2.getUtf8());
		}
	void updateElementXmlW(const wchar_t *w1,int index,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->UpdateElementXml(sTemp1.getUtf8(),index,sTemp2.getUtf8());
		}
	void updatePersonW(const wchar_t *w1,int index,const wchar_t *w2,const wchar_t *w3,const wchar_t *w4)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		CkString sTemp4; sTemp4.appendU(w4);
		this->UpdatePerson(sTemp1.getUtf8(),index,sTemp2.getUtf8(),sTemp3.getUtf8(),sTemp4.getUtf8());
		}
	const wchar_t *getElementW(const wchar_t *w1,int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetElement(sTemp1.getUtf8(),index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getElementAttrW(const wchar_t *w1,int index,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->GetElementAttr(sTemp1.getUtf8(),index,sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getLinkHrefW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetLinkHref(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getPersonInfoW(const wchar_t *w1,int index,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->GetPersonInfo(sTemp1.getUtf8(),index,sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getTopAttrW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetTopAttr(sTemp1.getUtf8(),m_resultStringW[idx]);
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
	const wchar_t *toXmlStringW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->ToXmlString(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
