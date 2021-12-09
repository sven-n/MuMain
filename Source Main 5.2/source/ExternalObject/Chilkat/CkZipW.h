#ifndef _CkZipW_H
#define _CkZipW_H
#pragma once

#include "CkString.h"
#include "CkZip.h"


#pragma pack (push, 8) 


class CkZipW : public CkZip
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
	CkZipW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkZipW() { }
	void put_AppendFromDirW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_AppendFromDir(s1.getUtf8());
	    }
	void put_AutoRunW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_AutoRun(s1.getUtf8());
	    }
	void put_AutoRunParamsW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_AutoRunParams(s1.getUtf8());
	    }
	void put_CommentW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Comment(s1.getUtf8());
	    }
	void put_DecryptPasswordW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_DecryptPassword(s1.getUtf8());
	    }
	void put_EncryptPasswordW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_EncryptPassword(s1.getUtf8());
	    }
	void put_ExeDefaultDirW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ExeDefaultDir(s1.getUtf8());
	    }
	void put_ExeIconFileW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ExeIconFile(s1.getUtf8());
	    }
	void put_ExeSourceUrlW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ExeSourceUrl(s1.getUtf8());
	    }
	void put_ExeTitleW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ExeTitle(s1.getUtf8());
	    }
	void put_ExeUnzipCaptionW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ExeUnzipCaption(s1.getUtf8());
	    }
	void put_ExeUnzipDirW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ExeUnzipDir(s1.getUtf8());
	    }
	void put_ExeXmlConfigW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ExeXmlConfig(s1.getUtf8());
	    }
	void put_FileNameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_FileName(s1.getUtf8());
	    }
	void put_PathPrefixW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_PathPrefix(s1.getUtf8());
	    }
	void put_ProxyW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Proxy(s1.getUtf8());
	    }
	void put_TempDirW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_TempDir(s1.getUtf8());
	    }
	bool addEmbeddedW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->AddEmbedded(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	void addNoCompressExtensionW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddNoCompressExtension(sTemp1.getUtf8());
		}
	CkZipEntry *appendBase64W(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AppendBase64(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	CkZipEntry *appendCompressedW(const wchar_t *w1,const CkByteData &data)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendCompressed(sTemp1.getUtf8(),data);
		}
	CkZipEntry *appendDataW(const wchar_t *w1,const CkByteData &data)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendData(sTemp1.getUtf8(),data);
		}
	CkZipEntry *appendDataW(const wchar_t *w1,const wchar_t *w2,unsigned long dataLen)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AppendData(sTemp1.getUtf8(),sTemp2.getUtf8(),dataLen);
		}
	bool appendFilesW(const wchar_t *w1,bool recurse,CkZipProgress *progress)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendFiles(sTemp1.getUtf8(),recurse,progress);
		}
	bool appendFilesExW(const wchar_t *w1,bool recurse,bool saveExtraPath,bool archiveOnly,bool includeHidden,bool includeSystem,CkZipProgress *progress)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendFilesEx(sTemp1.getUtf8(),recurse,saveExtraPath,archiveOnly,includeHidden,includeSystem,progress);
		}
	CkZipEntry *appendHexW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AppendHex(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	CkZipEntry *appendNewW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendNew(sTemp1.getUtf8());
		}
	CkZipEntry *appendNewDirW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendNewDir(sTemp1.getUtf8());
		}
	bool appendOneFileOrDirW(const wchar_t *w1,bool saveExtraPath)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendOneFileOrDir(sTemp1.getUtf8(),saveExtraPath);
		}
	CkZipEntry *appendStringW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AppendString(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	CkZipEntry *appendString2W(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->AppendString2(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool appendZipW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendZip(sTemp1.getUtf8());
		}
	void excludeDirW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->ExcludeDir(sTemp1.getUtf8());
		}
	bool extractW(const wchar_t *w1,CkZipProgress *progress)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Extract(sTemp1.getUtf8(),progress);
		}
	bool extractIntoW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ExtractInto(sTemp1.getUtf8());
		}
	bool extractMatchingW(const wchar_t *w1,const wchar_t *w2,CkZipProgress *progress)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ExtractMatching(sTemp1.getUtf8(),sTemp2.getUtf8(),progress);
		}
	bool extractNewerW(const wchar_t *w1,CkZipProgress *progress)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ExtractNewer(sTemp1.getUtf8(),progress);
		}
	CkZipEntry *firstMatchingEntryW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FirstMatchingEntry(sTemp1.getUtf8());
		}
	CkZipEntry *getEntryByNameW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetEntryByName(sTemp1.getUtf8());
		}
	void getExeConfigParamW(const wchar_t *w1,CkString &value)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetExeConfigParam(sTemp1.getUtf8(),value);
		}
	CkZipEntry *insertNewW(const wchar_t *w1,long beforeIndex)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->InsertNew(sTemp1.getUtf8(),beforeIndex);
		}
	bool isNoCompressExtensionW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->IsNoCompressExtension(sTemp1.getUtf8());
		}
	bool isPasswordProtectedW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->IsPasswordProtected(sTemp1.getUtf8());
		}
	bool newZipW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->NewZip(sTemp1.getUtf8());
		}
	bool openEmbeddedW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->OpenEmbedded(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool openFromWebW(const wchar_t *w1,CkZipProgress *progress)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpenFromWeb(sTemp1.getUtf8(),progress);
		}
	bool openMyEmbeddedW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpenMyEmbedded(sTemp1.getUtf8());
		}
	bool openZipW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpenZip(sTemp1.getUtf8());
		}
	bool quickAppendW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->QuickAppend(sTemp1.getUtf8());
		}
	bool removeEmbeddedW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->RemoveEmbedded(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void removeNoCompressExtensionW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->RemoveNoCompressExtension(sTemp1.getUtf8());
		}
	bool replaceEmbeddedW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->ReplaceEmbedded(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	void setExeConfigParamW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->SetExeConfigParam(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void setPasswordW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->SetPassword(sTemp1.getUtf8());
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	int unzipW(const wchar_t *w1,CkZipProgress *progress)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Unzip(sTemp1.getUtf8(),progress);
		}
	int unzipIntoW(const wchar_t *w1,CkZipProgress *progress)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnzipInto(sTemp1.getUtf8(),progress);
		}
	int unzipMatchingW(const wchar_t *w1,const wchar_t *w2,bool verbose,CkZipProgress *progress)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->UnzipMatching(sTemp1.getUtf8(),sTemp2.getUtf8(),verbose,progress);
		}
	int unzipMatchingIntoW(const wchar_t *w1,const wchar_t *w2,bool verbose,CkZipProgress *progress)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->UnzipMatchingInto(sTemp1.getUtf8(),sTemp2.getUtf8(),verbose,progress);
		}
	int unzipNewerW(const wchar_t *w1,CkZipProgress *progress)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnzipNewer(sTemp1.getUtf8(),progress);
		}
	bool writeExeW(const wchar_t *w1,CkZipProgress *progress)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->WriteExe(sTemp1.getUtf8(),progress);
		}
	bool writeExe2W(const wchar_t *w1,const wchar_t *w2,bool bAesEncrypt,int keyLength,const wchar_t *w3,CkZipProgress *progress)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->WriteExe2(sTemp1.getUtf8(),sTemp2.getUtf8(),bAesEncrypt,keyLength,sTemp3.getUtf8(),progress);
		}
	const wchar_t *appendFromDirW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_AppendFromDir(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *autoRunW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_AutoRun(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *autoRunParamsW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_AutoRunParams(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *decryptPasswordW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_DecryptPassword(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *encryptPasswordW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_EncryptPassword(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *exeDefaultDirW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ExeDefaultDir(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *exeIconFileW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ExeIconFile(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *exeSourceUrlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ExeSourceUrl(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *exeTitleW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ExeTitle(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *exeUnzipCaptionW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ExeUnzipCaption(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *exeUnzipDirW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ExeUnzipDir(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *exeXmlConfigW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ExeXmlConfig(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *fileNameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_FileName(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getDirectoryAsXMLW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetDirectoryAsXML(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getExeConfigParamW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetExeConfigParam(sTemp1.getUtf8(),m_resultStringW[idx]);
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
	const wchar_t *pathPrefixW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_PathPrefix(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *proxyW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Proxy(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *tempDirW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_TempDir(m_resultStringW[idx]);
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
