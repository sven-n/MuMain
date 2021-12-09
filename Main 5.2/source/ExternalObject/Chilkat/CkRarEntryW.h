#ifndef _CkRarEntryW_H
#define _CkRarEntryW_H
#pragma once

#include "CkString.h"
#include "CkRarEntry.h"


#pragma pack (push, 8) 


class CkRarEntryW : public CkRarEntry
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
	CkRarEntryW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkRarEntryW() { }
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool unrarW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Unrar(sTemp1.getUtf8());
		}
	const wchar_t *filenameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Filename(m_resultStringW[idx]);
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

};
#endif
