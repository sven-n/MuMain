// CkSocksProxy.h: interface for the CkSocksProxy class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CkSocksProxy_H
#define _CkSocksProxy_H

#pragma once

#include "CkString.h"
class CkSocksProxyProgress;
#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkSocksProxy
class CkSocksProxy  : public CkObject
{
    private:
	CkSocksProxyProgress *m_callback;

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkSocksProxy(const CkSocksProxy &);
	CkSocksProxy &operator=(const CkSocksProxy &);

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const;

	CkSocksProxy(void *impl);

	CkSocksProxy();
	virtual ~CkSocksProxy();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
    void LastErrorXml(CkString &str);
    void LastErrorHtml(CkString &str);
    void LastErrorText(CkString &str);

    const char *lastErrorText(void);
    const char *lastErrorXml(void);
    const char *lastErrorHtml(void);
	
	CkSocksProxyProgress *get_EventCallbackObject(void) const;
	void put_EventCallbackObject(CkSocksProxyProgress *progress);

	// UNLOCKCOMPONENT_BEGIN
	bool UnlockComponent(const char *unlockCode);
	// UNLOCKCOMPONENT_END
	// SOCKSVERSION_BEGIN
	int get_SocksVersion(void);
	// SOCKSVERSION_END
	// AUTHENTICATEDSOCKS5_BEGIN
	bool get_AuthenticatedSocks5(void);
	// AUTHENTICATEDSOCKS5_END
	// CONNECTIONPENDING_BEGIN
	bool get_ConnectionPending(void);
	// CONNECTIONPENDING_END
	// CLIENTPORT_BEGIN
	int get_ClientPort(void);
	// CLIENTPORT_END
	// SERVERPORT_BEGIN
	int get_ServerPort(void);
	// SERVERPORT_END
	// CLIENTIP_BEGIN
	void get_ClientIp(CkString &str);
	const char *clientIp(void);
	// CLIENTIP_END
	// SERVERIP_BEGIN
	void get_ServerIp(CkString &str);
	const char *serverIp(void);
	// SERVERIP_END
	// LOGIN_BEGIN
	void get_Login(CkString &str);
	const char *login(void);
	// LOGIN_END
	// PASSWORD_BEGIN
	void get_Password(CkString &str);
	const char *password(void);
	// PASSWORD_END
	// INITIALIZE_BEGIN
	bool Initialize(int port);
	// INITIALIZE_END
	// WAITFORCONNECTION_BEGIN
	bool WaitForConnection(int maxWaitMs);
	// WAITFORCONNECTION_END
	// ALLOWCONNECTION_BEGIN
	bool AllowConnection(void);
	// ALLOWCONNECTION_END
	// PROCEEDSOCKS5_BEGIN
	bool ProceedSocks5(void);
	// PROCEEDSOCKS5_END
	// REJECTCONNECTION_BEGIN
	bool RejectConnection(void);
	// REJECTCONNECTION_END
	// ALLOWUNAUTHENTICATEDSOCKS5_BEGIN
	bool get_AllowUnauthenticatedSocks5(void);
	void put_AllowUnauthenticatedSocks5(bool newVal);
	// ALLOWUNAUTHENTICATEDSOCKS5_END
	// LISTENBINDIPADDRESS_BEGIN
	void get_ListenBindIpAddress(CkString &str);
	const char *listenBindIpAddress(void);
	void put_ListenBindIpAddress(const char *newVal);
	// LISTENBINDIPADDRESS_END
	// OUTBOUNDBINDIPADDRESS_BEGIN
	void get_OutboundBindIpAddress(CkString &str);
	const char *outboundBindIpAddress(void);
	void put_OutboundBindIpAddress(const char *newVal);
	// OUTBOUNDBINDIPADDRESS_END
	// OUTBOUNDBINDPORT_BEGIN
	int get_OutboundBindPort(void);
	void put_OutboundBindPort(int newVal);
	// OUTBOUNDBINDPORT_END
	// GETTUNNELSXML_BEGIN
	bool GetTunnelsXml(CkString &outStr);
	const char *getTunnelsXml(void);
	// GETTUNNELSXML_END
	// STOPALLTUNNELS_BEGIN
	bool StopAllTunnels(int maxWaitMs);
	// STOPALLTUNNELS_END

	// SOCKSPROXY_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)


#endif


