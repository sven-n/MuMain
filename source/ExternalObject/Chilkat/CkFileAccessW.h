#ifndef _CkFileAccessW_H
#define _CkFileAccessW_H
#pragma once

#include "CkString.h"
#include "CkFileAccess.h"


#pragma pack (push, 8) 


class CkFileAccessW : public CkFileAccess
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
	CkFileAccessW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkFileAccessW() { }
	bool appendAnsiW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendAnsi(sTemp1.getUtf8());
		}
	bool appendTextW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AppendText(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool dirAutoCreateW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DirAutoCreate(sTemp1.getUtf8());
		}
	bool dirCreateW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DirCreate(sTemp1.getUtf8());
		}
	bool dirDeleteW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DirDelete(sTemp1.getUtf8());
		}
	bool dirEnsureExistsW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DirEnsureExists(sTemp1.getUtf8());
		}
	bool fileCopyW(const wchar_t *w1,const wchar_t *w2,bool failIfExists)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->FileCopy(sTemp1.getUtf8(),sTemp2.getUtf8(),failIfExists);
		}
	bool fileDeleteW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FileDelete(sTemp1.getUtf8());
		}
	bool fileExistsW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FileExists(sTemp1.getUtf8());
		}
	bool fileOpenW(const wchar_t *w1,int accessMode,int shareMode,int createDisp,int attr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FileOpen(sTemp1.getUtf8(),accessMode,shareMode,createDisp,attr);
		}
	bool fileRenameW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->FileRename(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	int fileSizeW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FileSize(sTemp1.getUtf8());
		}
	bool getTempFilenameW(const wchar_t *w1,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->GetTempFilename(sTemp1.getUtf8(),sTemp2.getUtf8(),outStr);
		}
	bool readBinaryToEncodedW(const wchar_t *w1,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ReadBinaryToEncoded(sTemp1.getUtf8(),sTemp2.getUtf8(),outStr);
		}
	bool readEntireFileW(const wchar_t *w1,CkByteData &outBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ReadEntireFile(sTemp1.getUtf8(),outBytes);
		}
	bool readEntireTextFileW(const wchar_t *w1,const wchar_t *w2,CkString &strFileContents)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ReadEntireTextFile(sTemp1.getUtf8(),sTemp2.getUtf8(),strFileContents);
		}
	bool reassembleFileW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,const wchar_t *w4)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		CkString sTemp4; sTemp4.appendU(w4);
		return this->ReassembleFile(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),sTemp4.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool setCurrentDirW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetCurrentDir(sTemp1.getUtf8());
		}
	bool splitFileW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,int partSize,const wchar_t *w4)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		CkString sTemp4; sTemp4.appendU(w4);
		return this->SplitFile(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),partSize,sTemp4.getUtf8());
		}
	bool treeDeleteW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->TreeDelete(sTemp1.getUtf8());
		}
	bool writeEntireFileW(const wchar_t *w1,CkByteData &fileData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->WriteEntireFile(sTemp1.getUtf8(),fileData);
		}
	bool writeEntireTextFileW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,bool includePreamble)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->WriteEntireTextFile(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),includePreamble);
		}
	const wchar_t *currentDirW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_CurrentDir(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getTempFilenameW(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->GetTempFilename(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
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
	const wchar_t *readBinaryToEncodedW(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->ReadBinaryToEncoded(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *readEntireTextFileW(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->ReadEntireTextFile(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
