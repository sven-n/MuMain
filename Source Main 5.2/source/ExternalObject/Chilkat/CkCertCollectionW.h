#ifndef _CkCertCollectionW_H
#define _CkCertCollectionW_H
#pragma once

#include "CkString.h"
#include "CkCertCollection.h"


#pragma pack (push, 8) 


class CkCertCollectionW : public CkCertCollection
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
	CkCertCollectionW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkCertCollectionW() { }
	bool loadPkcs7FileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadPkcs7File(sTemp1.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}

};
#endif
