#ifndef _CkSocketW_H
#define _CkSocketW_H
#pragma once

#include "CkString.h"
#include "CkSocket.h"


#pragma pack (push, 8) 


class CkSocketW : public CkSocket
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
	CkSocketW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkSocketW() { }
	void put_ClientIpAddressW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ClientIpAddress(s1.getUtf8());
	    }
	void put_SessionLogEncodingW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_SessionLogEncoding(s1.getUtf8());
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
	void put_SslProtocolW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_SslProtocol(s1.getUtf8());
	    }
	void put_StringCharsetW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_StringCharset(s1.getUtf8());
	    }
	bool asyncConnectStartW(const wchar_t *w1,int port,bool ssl,int maxWaitMs)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AsyncConnectStart(sTemp1.getUtf8(),port,ssl,maxWaitMs);
		}
	bool asyncDnsStartW(const wchar_t *w1,int maxWaitMs)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AsyncDnsStart(sTemp1.getUtf8(),maxWaitMs);
		}
	bool asyncReceiveUntilMatchW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AsyncReceiveUntilMatch(sTemp1.getUtf8());
		}
	bool asyncSendStringW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AsyncSendString(sTemp1.getUtf8());
		}
	void buildHttpGetRequestW(const wchar_t *w1,CkString &str)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->BuildHttpGetRequest(sTemp1.getUtf8(),str);
		}
	bool connectW(const wchar_t *w1,int port,bool ssl,int maxWaitMs)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->Connect(sTemp1.getUtf8(),port,ssl,maxWaitMs);
		}
	bool dnsLookupW(const wchar_t *w1,int maxWaitMs,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DnsLookup(sTemp1.getUtf8(),maxWaitMs,outStr);
		}
	bool receiveUntilMatchW(const wchar_t *w1,CkString &strOut)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ReceiveUntilMatch(sTemp1.getUtf8(),strOut);
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool sendStringW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SendString(sTemp1.getUtf8());
		}
	bool setSslClientCertPfxW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->SetSslClientCertPfx(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	const wchar_t *asyncAcceptLogW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_AsyncAcceptLog(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *asyncConnectLogW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_AsyncConnectLog(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *asyncDnsLogW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_AsyncDnsLog(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *asyncDnsResultW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_AsyncDnsResult(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *asyncReceiveLogW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_AsyncReceiveLog(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *asyncReceivedStringW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_AsyncReceivedString(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *asyncSendLogW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_AsyncSendLog(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *buildHttpGetRequestW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->BuildHttpGetRequest(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *clientIpAddressW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ClientIpAddress(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *dnsLookupW(const wchar_t *w1,int maxWaitMs)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->DnsLookup(sTemp1.getUtf8(),maxWaitMs,m_resultStringW[idx]);
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
	const wchar_t *localIpAddressW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_LocalIpAddress(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *myIpAddressW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_MyIpAddress(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *receiveStringW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->ReceiveString(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *receiveStringMaxNW(int maxBytes)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->ReceiveStringMaxN(maxBytes,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *receiveStringUntilByteW(int byteValue)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->ReceiveStringUntilByte(byteValue,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *receiveToCRLFW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->ReceiveToCRLF(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *receiveUntilMatchW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->ReceiveUntilMatch(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *remoteIpAddressW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_RemoteIpAddress(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *sessionLogW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SessionLog(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *sessionLogEncodingW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SessionLogEncoding(m_resultStringW[idx]);
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
	const wchar_t *sslProtocolW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SslProtocol(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *stringCharsetW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_StringCharset(m_resultStringW[idx]);
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
