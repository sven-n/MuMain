#ifndef _CkMessageSetW_H
#define _CkMessageSetW_H
#pragma once

#include "CkString.h"
#include "CkMessageSet.h"


#pragma pack (push, 8) 


class CkMessageSetW : public CkMessageSet
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
	CkMessageSetW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkMessageSetW() { }
	bool fromCompactStringW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FromCompactString(sTemp1.getUtf8());
		}
	const wchar_t *toCompactStringW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->ToCompactString(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *toStringW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->ToString(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
