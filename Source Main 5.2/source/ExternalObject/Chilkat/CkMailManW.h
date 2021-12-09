#ifndef _CkMailManW_H
#define _CkMailManW_H
#pragma once

#include "CkString.h"
#include "CkMailMan.h"


#pragma pack (push, 8) 


class CkMailManW : public CkMailMan
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
	CkMailManW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkMailManW() { }
	void put_ClientIpAddressW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ClientIpAddress(s1.getUtf8());
	    }
	void put_DsnEnvidW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_DsnEnvid(s1.getUtf8());
	    }
	void put_DsnNotifyW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_DsnNotify(s1.getUtf8());
	    }
	void put_DsnRetW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_DsnRet(s1.getUtf8());
	    }
	void put_FilterW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Filter(s1.getUtf8());
	    }
	void put_HeloHostnameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_HeloHostname(s1.getUtf8());
	    }
	void put_LogMailReceivedFilenameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_LogMailReceivedFilename(s1.getUtf8());
	    }
	void put_LogMailSentFilenameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_LogMailSentFilename(s1.getUtf8());
	    }
	void put_MailHostW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_MailHost(s1.getUtf8());
	    }
	void put_PopPasswordW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_PopPassword(s1.getUtf8());
	    }
	void put_PopUsernameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_PopUsername(s1.getUtf8());
	    }
	void put_SmtpAuthMethodW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_SmtpAuthMethod(s1.getUtf8());
	    }
	void put_SmtpHostW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_SmtpHost(s1.getUtf8());
	    }
	void put_SmtpLoginDomainW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_SmtpLoginDomain(s1.getUtf8());
	    }
	void put_SmtpPasswordW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_SmtpPassword(s1.getUtf8());
	    }
	void put_SmtpUsernameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_SmtpUsername(s1.getUtf8());
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
	bool deleteByUidlW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DeleteByUidl(sTemp1.getUtf8());
		}
	CkEmail *fetchEmailW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FetchEmail(sTemp1.getUtf8());
		}
	bool fetchMimeW(const wchar_t *w1,CkByteData &mimeBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FetchMime(sTemp1.getUtf8(),mimeBytes);
		}
	CkEmail *fetchSingleHeaderByUidlW(long numBodyLines,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FetchSingleHeaderByUidl(numBodyLines,sTemp1.getUtf8());
		}
	int getSizeByUidlW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetSizeByUidl(sTemp1.getUtf8());
		}
	CkEmail *loadEmlW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadEml(sTemp1.getUtf8());
		}
	CkEmailBundle *loadMbxW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadMbx(sTemp1.getUtf8());
		}
	CkEmail *loadMimeW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadMime(sTemp1.getUtf8());
		}
	CkEmail *loadXmlEmailW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadXmlEmail(sTemp1.getUtf8());
		}
	CkEmail *loadXmlEmailStringW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadXmlEmailString(sTemp1.getUtf8());
		}
	CkEmailBundle *loadXmlFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadXmlFile(sTemp1.getUtf8());
		}
	CkEmailBundle *loadXmlStringW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadXmlString(sTemp1.getUtf8());
		}
	bool mxLookupW(const wchar_t *w1,CkString &strHostname)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->MxLookup(sTemp1.getUtf8(),strHostname);
		}
	CkStringArray *mxLookupAllW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->MxLookupAll(sTemp1.getUtf8());
		}
	bool quickSendW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,const wchar_t *w4,const wchar_t *w5)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		CkString sTemp4; sTemp4.appendU(w4);
		CkString sTemp5; sTemp5.appendU(w5);
		return this->QuickSend(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),sTemp4.getUtf8(),sTemp5.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool sendMimeW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->SendMime(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool sendMimeQW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->SendMimeQ(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool sendMimeToListW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->SendMimeToList(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool sendQ2W(const CkEmail *email,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SendQ2(email,sTemp1.getUtf8());
		}
	bool sendQ2W(const CkEmail &email,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SendQ2(email,sTemp1.getUtf8());
		}
	bool setSslClientCertPfxW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->SetSslClientCertPfx(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool sshAuthenticatePkW(bool bSmtp,const wchar_t *w1,CkSshKey &privateKey)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SshAuthenticatePk(bSmtp,sTemp1.getUtf8(),privateKey);
		}
	bool sshAuthenticatePwW(bool bSmtp,const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SshAuthenticatePw(bSmtp,sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool sshTunnelW(bool bSmtp,const wchar_t *w1,int sshServerPort)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SshTunnel(bSmtp,sTemp1.getUtf8(),sshServerPort);
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	const wchar_t *clientIpAddressW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ClientIpAddress(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *dsnEnvidW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_DsnEnvid(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *dsnNotifyW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_DsnNotify(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *dsnRetW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_DsnRet(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *filterW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Filter(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getMailboxInfoXmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetMailboxInfoXml(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *heloHostnameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_HeloHostname(m_resultStringW[idx]);
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
	const wchar_t *lastSendQFilenameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastSendQFilename(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *logMailReceivedFilenameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LogMailReceivedFilename(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *logMailSentFilenameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LogMailSentFilename(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *mailHostW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_MailHost(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *mxLookupW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->MxLookup(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *pop3SessionLogW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Pop3SessionLog(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *popPasswordW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_PopPassword(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *popUsernameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_PopUsername(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *renderToMimeW(const CkEmail &email)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->RenderToMime(email,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *smtpAuthMethodW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SmtpAuthMethod(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *smtpHostW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SmtpHost(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *smtpLoginDomainW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SmtpLoginDomain(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *smtpPasswordW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SmtpPassword(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *smtpSessionLogW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SmtpSessionLog(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *smtpUsernameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SmtpUsername(m_resultStringW[idx]);
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
	const wchar_t *versionW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Version(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
