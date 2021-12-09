#ifndef _CkTarW_H
#define _CkTarW_H
#pragma once

#include "CkString.h"
#include "CkTar.h"


#pragma pack (push, 8) 


class CkTarW : public CkTar
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
	CkTarW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkTarW() { }
	void put_CharsetW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Charset(s1.getUtf8());
	    }
	void put_GroupNameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_GroupName(s1.getUtf8());
	    }
	void put_UntarFromDirW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_UntarFromDir(s1.getUtf8());
	    }
	void put_UntarMatchPatternW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_UntarMatchPattern(s1.getUtf8());
	    }
	void put_UserNameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_UserName(s1.getUtf8());
	    }
	bool addDirRootW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddDirRoot(sTemp1.getUtf8());
		}
	bool listXmlW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ListXml(sTemp1.getUtf8(),outStr);
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
	int untarW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Untar(sTemp1.getUtf8());
		}
	bool untarBz2W(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UntarBz2(sTemp1.getUtf8());
		}
	bool untarFirstMatchingToMemoryW(CkByteData &tarFileBytes,const wchar_t *w1,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UntarFirstMatchingToMemory(tarFileBytes,sTemp1.getUtf8(),outBytes);
		}
	bool untarGzW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UntarGz(sTemp1.getUtf8());
		}
	bool untarZW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UntarZ(sTemp1.getUtf8());
		}
	bool writeTarW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->WriteTar(sTemp1.getUtf8());
		}
	bool writeTarBz2W(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->WriteTarBz2(sTemp1.getUtf8());
		}
	bool writeTarGzW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->WriteTarGz(sTemp1.getUtf8());
		}
	bool writeTarZW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->WriteTarZ(sTemp1.getUtf8());
		}
	const wchar_t *charsetW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Charset(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getDirRootW(int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetDirRoot(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *groupNameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_GroupName(m_resultStringW[idx]);
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
	const wchar_t *listXmlW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->ListXml(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *untarFromDirW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_UntarFromDir(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *untarMatchPatternW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_UntarMatchPattern(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *userNameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_UserName(m_resultStringW[idx]);
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
