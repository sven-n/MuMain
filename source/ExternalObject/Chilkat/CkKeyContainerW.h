#ifndef _CkKeyContainerW_H
#define _CkKeyContainerW_H
#pragma once

#include "CkString.h"
#include "CkKeyContainer.h"


#pragma pack (push, 8) 


class CkKeyContainerW : public CkKeyContainer
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
	CkKeyContainerW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkKeyContainerW() { }
	bool createContainerW(const wchar_t *w1,bool machineKeyset)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CreateContainer(sTemp1.getUtf8(),machineKeyset);
		}
	bool openContainerW(const wchar_t *w1,bool needPrivateKeyAccess,bool machineKeyset)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpenContainer(sTemp1.getUtf8(),needPrivateKeyAccess,machineKeyset);
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	const wchar_t *containerNameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ContainerName(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *generateUuidW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GenerateUuid(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getContainerNameW(bool bMachineKeyset,int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetContainerName(bMachineKeyset,index,m_resultStringW[idx]);
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
