#ifndef _CkFtp2W_H
#define _CkFtp2W_H
#pragma once

#include "CkString.h"
#include "CkFtp2.h"


#pragma pack (push, 8) 


class CkFtp2W : public CkFtp2
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
	CkFtp2W() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkFtp2W() { }
	void put_AccountW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Account(s1.getUtf8());
	    }
	void put_DirListingCharsetW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_DirListingCharset(s1.getUtf8());
	    }
	void put_ForcePortIpAddressW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ForcePortIpAddress(s1.getUtf8());
	    }
	void put_HostnameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Hostname(s1.getUtf8());
	    }
	void put_ListPatternW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ListPattern(s1.getUtf8());
	    }
	void put_PasswordW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Password(s1.getUtf8());
	    }
	void put_ProxyHostnameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ProxyHostname(s1.getUtf8());
	    }
	void put_ProxyPasswordW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ProxyPassword(s1.getUtf8());
	    }
	void put_ProxyUsernameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ProxyUsername(s1.getUtf8());
	    }
	void put_SocksHostnameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_SocksHostname(s1.getUtf8());
	    }
	void put_SocksPasswordW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_SocksPassword(s1.getUtf8());
	    }
	void put_SocksUsernameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_SocksUsername(s1.getUtf8());
	    }
	void put_UsernameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Username(s1.getUtf8());
	    }
	bool appendFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AppendFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool appendFileFromBinaryDataW(const wchar_t *w1,const CkByteData &binaryData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendFileFromBinaryData(sTemp1.getUtf8(),binaryData);
		}
	bool appendFileFromTextDataW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AppendFileFromTextData(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool asyncAppendFileStartW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AsyncAppendFileStart(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool asyncGetFileStartW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AsyncGetFileStart(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool asyncPutFileStartW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AsyncPutFileStart(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool changeRemoteDirW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ChangeRemoteDir(sTemp1.getUtf8());
		}
	bool createPlanW(const wchar_t *w1,CkString &str)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CreatePlan(sTemp1.getUtf8(),str);
		}
	bool createRemoteDirW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CreateRemoteDir(sTemp1.getUtf8());
		}
	long deleteMatchingW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DeleteMatching(sTemp1.getUtf8());
		}
	bool deleteRemoteFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DeleteRemoteFile(sTemp1.getUtf8());
		}
	bool downloadTreeW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DownloadTree(sTemp1.getUtf8());
		}
	bool getCreateTimeByNameW(const wchar_t *w1,SYSTEMTIME &sysTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetCreateTimeByName(sTemp1.getUtf8(),sysTime);
		}
	bool getFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->GetFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool getLastAccessTimeByNameW(const wchar_t *w1,SYSTEMTIME &sysTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetLastAccessTimeByName(sTemp1.getUtf8(),sysTime);
		}
	bool getLastModifiedTimeByNameW(const wchar_t *w1,SYSTEMTIME &sysTime)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetLastModifiedTimeByName(sTemp1.getUtf8(),sysTime);
		}
	bool getRemoteFileBinaryDataW(const wchar_t *w1,CkByteData &data)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetRemoteFileBinaryData(sTemp1.getUtf8(),data);
		}
	bool getRemoteFileTextCW(const wchar_t *w1,const wchar_t *w2,CkString &str)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->GetRemoteFileTextC(sTemp1.getUtf8(),sTemp2.getUtf8(),str);
		}
	bool getRemoteFileTextDataW(const wchar_t *w1,CkString &str)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetRemoteFileTextData(sTemp1.getUtf8(),str);
		}
	long getSizeByNameW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetSizeByName(sTemp1.getUtf8());
		}
	bool getSizeStrByNameW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetSizeStrByName(sTemp1.getUtf8(),outStr);
		}
	bool getTextDirListingW(const wchar_t *w1,CkString &strRawListing)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetTextDirListing(sTemp1.getUtf8(),strRawListing);
		}
	bool getXmlDirListingW(const wchar_t *w1,CkString &strXmlListing)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetXmlDirListing(sTemp1.getUtf8(),strXmlListing);
		}
	long mGetFilesW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->MGetFiles(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	long mPutFilesW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->MPutFiles(sTemp1.getUtf8());
		}
	bool nlstXmlW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->NlstXml(sTemp1.getUtf8(),outStr);
		}
	bool putFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->PutFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool putFileFromBinaryDataW(const wchar_t *w1,const CkByteData &binaryData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->PutFileFromBinaryData(sTemp1.getUtf8(),binaryData);
		}
	bool putFileFromTextDataW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->PutFileFromTextData(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool putPlanW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->PutPlan(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool putTreeW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->PutTree(sTemp1.getUtf8());
		}
	bool quoteW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Quote(sTemp1.getUtf8());
		}
	bool removeRemoteDirW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->RemoveRemoteDir(sTemp1.getUtf8());
		}
	bool renameRemoteFileW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->RenameRemoteFile(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool sendCommandW(const wchar_t *w1,CkString &reply)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SendCommand(sTemp1.getUtf8(),reply);
		}
	bool setRemoteFileDateTimeW(SYSTEMTIME &dt,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetRemoteFileDateTime(dt,sTemp1.getUtf8());
		}
	void setSslCertRequirementW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->SetSslCertRequirement(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool setSslClientCertPfxW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->SetSslClientCertPfx(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool siteW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Site(sTemp1.getUtf8());
		}
	bool syncLocalTreeW(const wchar_t *w1,int mode)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SyncLocalTree(sTemp1.getUtf8(),mode);
		}
	bool syncRemoteTreeW(const wchar_t *w1,int mode)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SyncRemoteTree(sTemp1.getUtf8(),mode);
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	const wchar_t *accountW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Account(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *asyncLogW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_AsyncLog(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *ck_statW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->Stat(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *createPlanW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->CreatePlan(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *determineSettingsW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->DetermineSettings(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *dirListingCharsetW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_DirListingCharset(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *dirTreeXmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->DirTreeXml(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *featW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->Feat(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *forcePortIpAddressW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ForcePortIpAddress(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getCurrentRemoteDirW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetCurrentRemoteDir(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getFilenameW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetFilename(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getRemoteFileTextCW(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->GetRemoteFileTextC(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getRemoteFileTextDataW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetRemoteFileTextData(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getSizeStrW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetSizeStr(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getSizeStrByNameW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetSizeStrByName(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getTextDirListingW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetTextDirListing(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getXmlDirListingW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetXmlDirListing(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *greetingW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Greeting(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *hostnameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Hostname(m_resultStringW[idx]);
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
	const wchar_t *listPatternW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ListPattern(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *nlstXmlW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->NlstXml(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *passwordW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Password(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *proxyHostnameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ProxyHostname(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *proxyPasswordW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ProxyPassword(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *proxyUsernameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ProxyUsername(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *sendCommandW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->SendCommand(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *sessionLogW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SessionLog(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *socksHostnameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SocksHostname(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *socksPasswordW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SocksPassword(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *socksUsernameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SocksUsername(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *systW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->Syst(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *usernameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Username(m_resultStringW[idx]);
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
