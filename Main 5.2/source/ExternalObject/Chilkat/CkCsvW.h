#ifndef _CkCsvW_H
#define _CkCsvW_H
#pragma once

#include "CkString.h"
#include "CkCsv.h"


#pragma pack (push, 8) 


class CkCsvW : public CkCsv
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
	CkCsvW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkCsvW() { }
	void put_DelimiterW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Delimiter(s1.getUtf8());
	    }
	int getIndexW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetIndex(sTemp1.getUtf8());
		}
	bool loadFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadFile(sTemp1.getUtf8());
		}
	bool loadFile2W(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->LoadFile2(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool loadFromStringW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadFromString(sTemp1.getUtf8());
		}
	bool saveFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveFile(sTemp1.getUtf8());
		}
	bool saveFile2W(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SaveFile2(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool setCellW(int row,int col,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetCell(row,col,sTemp1.getUtf8());
		}
	bool setColumnNameW(int index,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetColumnName(index,sTemp1.getUtf8());
		}
	const wchar_t *delimiterW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Delimiter(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getCellW(int row,int col)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetCell(row,col,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getColumnNameW(int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetColumnName(index,m_resultStringW[idx]);
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
	const wchar_t *saveToStringW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->SaveToString(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
