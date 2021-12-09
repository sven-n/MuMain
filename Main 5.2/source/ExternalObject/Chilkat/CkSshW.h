#ifndef _CkSshW_H
#define _CkSshW_H
#pragma once

#include "CkString.h"
#include "CkSsh.h"


#pragma pack (push, 8) 


class CkSshW : public CkSsh
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
	CkSshW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkSshW() { }
	void put_ClientIdentifierW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ClientIdentifier(s1.getUtf8());
	    }
	bool authenticatePkW(const wchar_t *w1,CkSshKey &privateKey)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AuthenticatePk(sTemp1.getUtf8(),privateKey);
		}
	bool authenticatePwW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AuthenticatePw(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool channelReceiveUntilMatchW(int channelNum,const wchar_t *w1,const wchar_t *w2,bool caseSensitive)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ChannelReceiveUntilMatch(channelNum,sTemp1.getUtf8(),sTemp2.getUtf8(),caseSensitive);
		}
	bool channelSendStringW(int channelNum,const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ChannelSendString(channelNum,sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool connectW(const wchar_t *w1,int port)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Connect(sTemp1.getUtf8(),port);
		}
	bool getReceivedTextW(int channelNum,const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetReceivedText(channelNum,sTemp1.getUtf8(),outStr);
		}
	bool getReceivedTextSW(int channelNum,const wchar_t *w1,const wchar_t *w2,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->GetReceivedTextS(channelNum,sTemp1.getUtf8(),sTemp2.getUtf8(),outStr);
		}
	int openCustomChannelW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpenCustomChannel(sTemp1.getUtf8());
		}
	int openDirectTcpIpChannelW(const wchar_t *w1,int port)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->OpenDirectTcpIpChannel(sTemp1.getUtf8(),port);
		}
	bool peekReceivedTextW(int channelNum,const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->PeekReceivedText(channelNum,sTemp1.getUtf8(),outStr);
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool sendReqExecW(int channelNum,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SendReqExec(channelNum,sTemp1.getUtf8());
		}
	bool sendReqPtyW(int channelNum,const wchar_t *w1,int widthInChars,int heightInRows,int pixWidth,int pixHeight)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SendReqPty(channelNum,sTemp1.getUtf8(),widthInChars,heightInRows,pixWidth,pixHeight);
		}
	bool sendReqSetEnvW(int channelNum,const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SendReqSetEnv(channelNum,sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool sendReqSignalW(int channelNum,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SendReqSignal(channelNum,sTemp1.getUtf8());
		}
	bool sendReqSubsystemW(int channelNum,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SendReqSubsystem(channelNum,sTemp1.getUtf8());
		}
	bool sendReqX11ForwardingW(int channelNum,bool singleConnection,const wchar_t *w1,const wchar_t *w2,int screenNum)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SendReqX11Forwarding(channelNum,singleConnection,sTemp1.getUtf8(),sTemp2.getUtf8(),screenNum);
		}
	bool setTtyModeW(const wchar_t *w1,int value)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetTtyMode(sTemp1.getUtf8(),value);
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	const wchar_t *channelOpenFailReasonW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ChannelOpenFailReason(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *clientIdentifierW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ClientIdentifier(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *disconnectReasonW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_DisconnectReason(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getChannelTypeW(int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetChannelType(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getReceivedTextW(int channelNum,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetReceivedText(channelNum,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getReceivedTextSW(int channelNum,const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->GetReceivedTextS(channelNum,sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
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
	const wchar_t *peekReceivedTextW(int channelNum,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->PeekReceivedText(channelNum,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *sessionLogW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SessionLog(m_resultStringW[idx]);
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
