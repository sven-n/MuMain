#ifndef _CkFileListW_H
#define _CkFileListW_H
#pragma once

#include "CkString.h"
#include "CkFileList.h"


#pragma pack (push, 8) 


class CkFileListW : public CkFileList
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
	CkFileListW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkFileListW() { }
	void put_AppendFromDirW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_AppendFromDir(s1.getUtf8());
	    }
	bool appendDirEntryW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendDirEntry(sTemp1.getUtf8());
		}
	bool appendFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendFile(sTemp1.getUtf8());
		}
	int appendFilesW(const wchar_t *w1,bool recurse)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendFiles(sTemp1.getUtf8(),recurse);
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool setArchivePathW(int idx,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetArchivePath(idx,sTemp1.getUtf8());
		}
	void stringReplaceW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->StringReplace(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	const wchar_t *appendFromDirW(int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_AppendFromDir(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getArchivePathW(int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetArchivePath(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getDiskPathW(int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetDiskPath(index,m_resultStringW[idx]);
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
