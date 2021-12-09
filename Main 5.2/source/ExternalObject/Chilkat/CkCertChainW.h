#ifndef _CkCertChainW_H
#define _CkCertChainW_H
#pragma once

#include "CkString.h"
#include "CkCertChain.h"


#pragma pack (push, 8) 


class CkCertChainW : public CkCertChain
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
	CkCertChainW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkCertChainW() { }
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}

};
#endif

