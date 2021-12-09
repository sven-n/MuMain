#ifndef _CkImapW_H
#define _CkImapW_H
#pragma once

#include "CkString.h"
#include "CkImap.h"


#pragma pack (push, 8) 


class CkImapW : public CkImap
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
	CkImapW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkImapW() { }
	void put_AuthMethodW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_AuthMethod(s1.getUtf8());
	    }
	void put_AuthzIdW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_AuthzId(s1.getUtf8());
	    }
	void put_DomainW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Domain(s1.getUtf8());
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
	bool appendMailW(const wchar_t *w1,const CkEmail &email)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendMail(sTemp1.getUtf8(),email);
		}
	bool appendMimeW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AppendMime(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool appendMimeWithDateW(const wchar_t *w1,const wchar_t *w2,SYSTEMTIME &internalDate)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AppendMimeWithDate(sTemp1.getUtf8(),sTemp2.getUtf8(),internalDate);
		}
	bool appendMimeWithFlagsW(const wchar_t *w1,const wchar_t *w2,bool seen,bool flagged,bool answered,bool draft)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AppendMimeWithFlags(sTemp1.getUtf8(),sTemp2.getUtf8(),seen,flagged,answered,draft);
		}
	bool connectW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Connect(sTemp1.getUtf8());
		}
	bool copyW(long msgId,bool bUid,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Copy(msgId,bUid,sTemp1.getUtf8());
		}
	bool copyMultipleW(CkMessageSet &messageSet,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CopyMultiple(messageSet,sTemp1.getUtf8());
		}
	bool copySequenceW(int startSeqNum,int count,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CopySequence(startSeqNum,count,sTemp1.getUtf8());
		}
	bool createMailboxW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CreateMailbox(sTemp1.getUtf8());
		}
	bool deleteMailboxW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DeleteMailbox(sTemp1.getUtf8());
		}
	bool examineMailboxW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ExamineMailbox(sTemp1.getUtf8());
		}
	long getMailFlagW(const CkEmail &email,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetMailFlag(email,sTemp1.getUtf8());
		}
	CkMailboxes *listMailboxesW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ListMailboxes(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool loginW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->Login(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool renameMailboxW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->RenameMailbox(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	CkMessageSet *searchW(const wchar_t *w1,bool bUid)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Search(sTemp1.getUtf8(),bUid);
		}
	bool selectMailboxW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SelectMailbox(sTemp1.getUtf8());
		}
	bool sendRawCommandW(const wchar_t *w1,CkString &rawResponse)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SendRawCommand(sTemp1.getUtf8(),rawResponse);
		}
	bool setFlagW(long msgId,bool bUid,const wchar_t *w1,long value)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetFlag(msgId,bUid,sTemp1.getUtf8(),value);
		}
	bool setFlagsW(const CkMessageSet &messageSet,const wchar_t *w1,long value)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetFlags(messageSet,sTemp1.getUtf8(),value);
		}
	bool setMailFlagW(const CkEmail &email,const wchar_t *w1,long value)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetMailFlag(email,sTemp1.getUtf8(),value);
		}
	bool setSslClientCertPfxW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->SetSslClientCertPfx(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool sshAuthenticatePkW(const wchar_t *w1,CkSshKey &privateKey)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SshAuthenticatePk(sTemp1.getUtf8(),privateKey);
		}
	bool sshAuthenticatePwW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SshAuthenticatePw(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool sshTunnelW(const wchar_t *w1,int sshServerPort)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SshTunnel(sTemp1.getUtf8(),sshServerPort);
		}
	bool storeFlagsW(long msgId,bool bUid,const wchar_t *w1,long value)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->StoreFlags(msgId,bUid,sTemp1.getUtf8(),value);
		}
	bool subscribeW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Subscribe(sTemp1.getUtf8());
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	bool unsubscribeW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Unsubscribe(sTemp1.getUtf8());
		}
	const wchar_t *authMethodW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_AuthMethod(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *authzIdW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_AuthzId(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *connectedToHostW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ConnectedToHost(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *domainW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Domain(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *fetchFlagsW(long msgId,bool bUid)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->FetchFlags(msgId,bUid,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *fetchSingleAsMimeW(long msgId,bool bUid)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->FetchSingleAsMime(msgId,bUid,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *fetchSingleHeaderAsMimeW(int msgId,bool bUID)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->FetchSingleHeaderAsMime(msgId,bUID,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getMailAttachFilenameW(const CkEmail &email,long attachIndex)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetMailAttachFilename(email,attachIndex,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastAppendedMimeW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastAppendedMime(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastCommandW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastCommand(m_resultStringW[idx]);
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
	const wchar_t *lastIntermediateResponseW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastIntermediateResponse(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastResponseW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LastResponse(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *loggedInUserW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LoggedInUser(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *selectedMailboxW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SelectedMailbox(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *sendRawCommandW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->SendRawCommand(sTemp1.getUtf8(),m_resultStringW[idx]);
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
	const wchar_t *versionW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Version(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
