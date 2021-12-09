#ifndef _CkMailboxesW_H
#define _CkMailboxesW_H
#pragma once

#include "CkString.h"
#include "CkMailboxes.h"


#pragma pack (push, 8) 


class CkMailboxesW : public CkMailboxes
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
	CkMailboxesW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkMailboxesW() { }
	const wchar_t *getNameW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetName(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
