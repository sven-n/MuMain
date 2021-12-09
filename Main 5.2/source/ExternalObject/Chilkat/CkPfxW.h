#ifndef _CkPfxW_H
#define _CkPfxW_H
#pragma once

#include "CkString.h"
#include "CkPfx.h"


#pragma pack (push, 8) 


class CkPfxW : public CkPfx
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
	CkPfxW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkPfxW() { }
	bool importPfxFileW(const wchar_t *w1,const wchar_t *w2,bool bMachineKeyset,bool bLocalMachineCertStore,bool bExportable,bool bUseWarningDialog)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ImportPfxFile(sTemp1.getUtf8(),sTemp2.getUtf8(),bMachineKeyset,bLocalMachineCertStore,bExportable,bUseWarningDialog);
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
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
