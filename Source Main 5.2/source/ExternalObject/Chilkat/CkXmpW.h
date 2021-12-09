#ifndef _CkXmpW_H
#define _CkXmpW_H
#pragma once

#include "CkString.h"
#include "CkXmp.h"


#pragma pack (push, 8) 


class CkXmpW : public CkXmp
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
	CkXmpW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkXmpW() { }
	bool addArrayW(CkXml &xml,const wchar_t *w1,const wchar_t *w2,CkStringArray &values)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddArray(xml,sTemp1.getUtf8(),sTemp2.getUtf8(),values);
		}
	void addNsMappingW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->AddNsMapping(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool addSimpleDateW(CkXml &xml,const wchar_t *w1,SYSTEMTIME &sysTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddSimpleDate(xml,sTemp1.getUtf8(),sysTime);
		}
	bool addSimpleIntW(CkXml &xml,const wchar_t *w1,int propVal)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddSimpleInt(xml,sTemp1.getUtf8(),propVal);
		}
	bool addSimpleStrW(CkXml &xml,const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddSimpleStr(xml,sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool addStructPropW(CkXml &xml,const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->AddStructProp(xml,sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool getArrayW(CkXml &xml,const wchar_t *w1,CkStringArray &array)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetArray(xml,sTemp1.getUtf8(),array);
		}
	bool getSimpleDateW(CkXml &xml,const wchar_t *w1,SYSTEMTIME &sysTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetSimpleDate(xml,sTemp1.getUtf8(),sysTime);
		}
	int getSimpleIntW(CkXml &xml,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetSimpleInt(xml,sTemp1.getUtf8());
		}
	bool getSimpleStrW(CkXml &xml,const wchar_t *w1,CkString &strOut)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetSimpleStr(xml,sTemp1.getUtf8(),strOut);
		}
	bool getStructPropNamesW(CkXml &xml,const wchar_t *w1,CkStringArray &array)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetStructPropNames(xml,sTemp1.getUtf8(),array);
		}
	bool getStructValueW(CkXml &xml,const wchar_t *w1,const wchar_t *w2,CkString &strOut)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->GetStructValue(xml,sTemp1.getUtf8(),sTemp2.getUtf8(),strOut);
		}
	bool loadAppFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadAppFile(sTemp1.getUtf8());
		}
	bool loadFromBufferW(CkByteData &byteData,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadFromBuffer(byteData,sTemp1.getUtf8());
		}
	bool removeArrayW(CkXml &xml,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->RemoveArray(xml,sTemp1.getUtf8());
		}
	void removeNsMappingW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->RemoveNsMapping(sTemp1.getUtf8());
		}
	bool removeSimpleW(CkXml &xml,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->RemoveSimple(xml,sTemp1.getUtf8());
		}
	bool removeStructW(CkXml &xml,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->RemoveStruct(xml,sTemp1.getUtf8());
		}
	bool removeStructPropW(CkXml &xml,const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->RemoveStructProp(xml,sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool saveAppFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveAppFile(sTemp1.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool stringToDateW(const wchar_t *w1,SYSTEMTIME &sysTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->StringToDate(sTemp1.getUtf8(),sysTime);
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	const wchar_t *dateToStringW(SYSTEMTIME &sysTime)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->DateToString(sysTime,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getSimpleStrW(CkXml &xml,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetSimpleStr(xml,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getStructValueW(CkXml &xml,const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->GetStructValue(xml,sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
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
	const wchar_t *simpleStrW(CkXml &xml,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetSimpleStr(xml,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *structValueW(CkXml &xml,const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->GetStructValue(xml,sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
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
