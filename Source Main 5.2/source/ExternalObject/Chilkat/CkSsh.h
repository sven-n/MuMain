// CkSsh.h: interface for the CkSsh class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CkSsh_H
#define _CkSsh_H

#pragma once

#include "CkString.h"
class CkByteData;
class CkSshProgress;
class CkSshKey;
#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkSsh
class CkSsh  : public CkObject
{
    private:
	CkSshProgress *m_callback;

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkSsh(const CkSsh &);
	CkSsh &operator=(const CkSsh &);

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const;

	CkSsh(void *impl);

	CkSsh();
	virtual ~CkSsh();

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
	
	CkSshProgress *get_EventCallbackObject(void) const;
	void put_EventCallbackObject(CkSshProgress *progress);

	// CONNECT_BEGIN
	bool Connect(const char *hostname, int port);
	// CONNECT_END
	// UNLOCKCOMPONENT_BEGIN
	bool UnlockComponent(const char *unlockCode);
	// UNLOCKCOMPONENT_END
	// AUTHENTICATEPW_BEGIN
	bool AuthenticatePw(const char *login, const char *password);
	// AUTHENTICATEPW_END
	// VERSION_BEGIN
	void get_Version(CkString &str);
	const char *version(void);
	// VERSION_END
	// KEEPSESSIONLOG_BEGIN
	bool get_KeepSessionLog(void);
	void put_KeepSessionLog(bool newVal);
	// KEEPSESSIONLOG_END
	// SESSIONLOG_BEGIN
	void get_SessionLog(CkString &str);
	const char *sessionLog(void);
	// SESSIONLOG_END
	// IDLETIMEOUTMS_BEGIN
	int get_IdleTimeoutMs(void);
	void put_IdleTimeoutMs(int newVal);
	// IDLETIMEOUTMS_END
	// CONNECTTIMEOUTMS_BEGIN
	int get_ConnectTimeoutMs(void);
	void put_ConnectTimeoutMs(int newVal);
	// CONNECTTIMEOUTMS_END
	// CHANNELOPENFAILCODE_BEGIN
	int get_ChannelOpenFailCode(void);
	// CHANNELOPENFAILCODE_END
	// DISCONNECTCODE_BEGIN
	int get_DisconnectCode(void);
	// DISCONNECTCODE_END
	// DISCONNECTREASON_BEGIN
	void get_DisconnectReason(CkString &str);
	const char *disconnectReason(void);
	// DISCONNECTREASON_END
	// CHANNELOPENFAILREASON_BEGIN
	void get_ChannelOpenFailReason(CkString &str);
	const char *channelOpenFailReason(void);
	// CHANNELOPENFAILREASON_END
	// MAXPACKETSIZE_BEGIN
	int get_MaxPacketSize(void);
	void put_MaxPacketSize(int newVal);
	// MAXPACKETSIZE_END
	// DISCONNECT_BEGIN
	void Disconnect(void);
	// DISCONNECT_END
	// OPENSESSIONCHANNEL_BEGIN
	int OpenSessionChannel(void);
	// OPENSESSIONCHANNEL_END
	// OPENX11CHANNEL_BEGIN
	//int OpenX11Channel(const char *xAddress, int port);
	// OPENX11CHANNEL_END
	// OPENFORWARDEDTCPIPCHANNEL_BEGIN
	//int OpenForwardedTcpIpChannel(void);
	// OPENFORWARDEDTCPIPCHANNEL_END
	// OPENDIRECTTCPIPCHANNEL_BEGIN
	//int OpenDirectTcpIpChannel(void);
	// OPENDIRECTTCPIPCHANNEL_END
	// OPENCUSTOMCHANNEL_BEGIN
	int OpenCustomChannel(const char *channelType);
	// OPENCUSTOMCHANNEL_END
	// NUMOPENCHANNELS_BEGIN
	int get_NumOpenChannels(void);
	// NUMOPENCHANNELS_END
	// GETCHANNELNUMBER_BEGIN
	int GetChannelNumber(int index);
	// GETCHANNELNUMBER_END
	// GETCHANNELTYPE_BEGIN
	bool GetChannelType(int index, CkString &outStr);
	const char *getChannelType(int index);
	// GETCHANNELTYPE_END
	// SENDREQPTY_BEGIN
	bool SendReqPty(int channelNum, const char *xTermEnvVar, int widthInChars, int heightInRows, int pixWidth, int pixHeight);
	// SENDREQPTY_END
	// SENDREQX11FORWARDING_BEGIN
	bool SendReqX11Forwarding(int channelNum, bool singleConnection, const char *authProt, const char *authCookie, int screenNum);
	// SENDREQX11FORWARDING_END
	// SENDREQSETENV_BEGIN
	bool SendReqSetEnv(int channelNum, const char *name, const char *value);
	// SENDREQSETENV_END
	// SENDREQSHELL_BEGIN
	bool SendReqShell(int channelNum);
	// SENDREQSHELL_END
	// SENDREQEXEC_BEGIN
	bool SendReqExec(int channelNum, const char *command);
	// SENDREQEXEC_END
	// SENDREQSUBSYSTEM_BEGIN
	bool SendReqSubsystem(int channelNum, const char *subsystemName);
	// SENDREQSUBSYSTEM_END
	// SENDREQWINDOWCHANGE_BEGIN
	bool SendReqWindowChange(int channelNum, int widthInChars, int heightInRows, int pixWidth, int pixHeight);
	// SENDREQWINDOWCHANGE_END
	// SENDREQXONXOFF_BEGIN
	bool SendReqXonXoff(int channelNum, bool clientCanDo);
	// SENDREQXONXOFF_END
	// SENDREQSIGNAL_BEGIN
	bool SendReqSignal(int channelNum, const char *signalName);
	// SENDREQSIGNAL_END
	// CHANNELSENDDATA_BEGIN
	bool ChannelSendData(int channelNum, CkByteData &data);
	// CHANNELSENDDATA_END
	// CHANNELSENDSTRING_BEGIN
	bool ChannelSendString(int channelNum, const char *strData, const char *charset);
	// CHANNELSENDSTRING_END
	// CHANNELPOLL_BEGIN
	int ChannelPoll(int channelNum, int pollTimeoutMs);
	// CHANNELPOLL_END
	// CHANNELREADANDPOLL_BEGIN
	int ChannelReadAndPoll(int channelNum, int pollTimeoutMs);
	// CHANNELREADANDPOLL_END
	// CHANNELREAD_BEGIN
	int ChannelRead(int channelNum);
	// CHANNELREAD_END
	// GETRECEIVEDDATA_BEGIN
	void GetReceivedData(int channelNum, CkByteData &outBytes);
	// GETRECEIVEDDATA_END
	// GETRECEIVEDSTDERR_BEGIN
	void GetReceivedStderr(int channelNum, CkByteData &outBytes);
	// GETRECEIVEDSTDERR_END
	// CHANNELRECEIVEDEOF_BEGIN
	bool ChannelReceivedEof(int channelNum);
	// CHANNELRECEIVEDEOF_END
	// CHANNELRECEIVEDCLOSE_BEGIN
	bool ChannelReceivedClose(int channelNum);
	// CHANNELRECEIVEDCLOSE_END
	// CHANNELSENDCLOSE_BEGIN
	bool ChannelSendClose(int channelNum);
	// CHANNELSENDCLOSE_END
	// CHANNELSENDEOF_BEGIN
	bool ChannelSendEof(int channelNum);
	// CHANNELSENDEOF_END
	// CHANNELISOPEN_BEGIN
	bool ChannelIsOpen(int channelNum);
	// CHANNELISOPEN_END
	// CHANNELRECEIVETOCLOSE_BEGIN
	bool ChannelReceiveToClose(int channelNum);
	// CHANNELRECEIVETOCLOSE_END
	// CLEARTTYMODES_BEGIN
	void ClearTtyModes(void);
	// CLEARTTYMODES_END
	// SETTTYMODE_BEGIN
	bool SetTtyMode(const char *name, int value);
	// SETTTYMODE_END
	// ISCONNECTED_BEGIN
	bool get_IsConnected(void);
	// ISCONNECTED_END
	// REKEY_BEGIN
	bool ReKey(void);
	// REKEY_END
	// AUTHENTICATEPK_BEGIN
	bool AuthenticatePk(const char *username, CkSshKey &privateKey);
	// AUTHENTICATEPK_END
	// GETRECEIVEDTEXT_BEGIN
	bool GetReceivedText(int channelNum, const char *charset, CkString &outStr);
	const char *getReceivedText(int channelNum, const char *charset);
	// GETRECEIVEDTEXT_END
	// GETRECEIVEDNUMBYTES_BEGIN
	int GetReceivedNumBytes(int channelNum);
	// GETRECEIVEDNUMBYTES_END
	// CHANNELRECEIVEUNTILMATCH_BEGIN
	bool ChannelReceiveUntilMatch(int channelNum, const char *matchPattern, const char *charset, bool caseSensitive);
	// CHANNELRECEIVEUNTILMATCH_END
	// SENDIGNORE_BEGIN
	bool SendIgnore(void);
	// SENDIGNORE_END
	// OPENDIRECTTCPIPCHANNEL_BEGIN
	int OpenDirectTcpIpChannel(const char *hostname, int port);
	// OPENDIRECTTCPIPCHANNEL_END
	// GETRECEIVEDTEXTS_BEGIN
	bool GetReceivedTextS(int channelNum, const char *substr, const char *charset, CkString &outStr);
	const char *getReceivedTextS(int channelNum, const char *substr, const char *charset);
	// GETRECEIVEDTEXTS_END
	// GETRECEIVEDDATAN_BEGIN
	bool GetReceivedDataN(int channelNum, int numBytes, CkByteData &outBytes);
	// GETRECEIVEDDATAN_END
	// PEEKRECEIVEDTEXT_BEGIN
	bool PeekReceivedText(int channelNum, const char *charset, CkString &outStr);
	const char *peekReceivedText(int channelNum, const char *charset);
	// PEEKRECEIVEDTEXT_END
	// HEARTBEATMS_BEGIN
	int get_HeartbeatMs(void);
	void put_HeartbeatMs(int newVal);
	// HEARTBEATMS_END
	// CHANNELRECEIVEDEXITSTATUS_BEGIN
	bool ChannelReceivedExitStatus(int channelNum);
	// CHANNELRECEIVEDEXITSTATUS_END
	// GETCHANNELEXITSTATUS_BEGIN
	int GetChannelExitStatus(int channelNum);
	// GETCHANNELEXITSTATUS_END
	// CLIENTIDENTIFIER_BEGIN
	void get_ClientIdentifier(CkString &str);
	const char *clientIdentifier(void);
	void put_ClientIdentifier(const char *newVal);
	// CLIENTIDENTIFIER_END
	// READTIMEOUTMS_BEGIN
	int get_ReadTimeoutMs(void);
	void put_ReadTimeoutMs(int newVal);
	// READTIMEOUTMS_END
	// TCPNODELAY_BEGIN
	bool get_TcpNoDelay(void);
	void put_TcpNoDelay(bool newVal);
	// TCPNODELAY_END
	// VERBOSELOGGING_BEGIN
	bool get_VerboseLogging(void);
	void put_VerboseLogging(bool newVal);
	// VERBOSELOGGING_END
	// HOSTKEYFINGERPRINT_BEGIN
	void get_HostKeyFingerprint(CkString &str);
	const char *hostKeyFingerprint(void);
	// HOSTKEYFINGERPRINT_END
	// SOCKSVERSION_BEGIN
	int get_SocksVersion(void);
	void put_SocksVersion(int newVal);
	// SOCKSVERSION_END
	// SOCKSPORT_BEGIN
	int get_SocksPort(void);
	void put_SocksPort(int newVal);
	// SOCKSPORT_END
	// SOCKSHOSTNAME_BEGIN
	void get_SocksHostname(CkString &str);
	const char *socksHostname(void);
	void put_SocksHostname(const char *newVal);
	// SOCKSHOSTNAME_END
	// SOCKSUSERNAME_BEGIN
	void get_SocksUsername(CkString &str);
	const char *socksUsername(void);
	void put_SocksUsername(const char *newVal);
	// SOCKSUSERNAME_END
	// SOCKSPASSWORD_BEGIN
	void get_SocksPassword(CkString &str);
	const char *socksPassword(void);
	void put_SocksPassword(const char *newVal);
	// SOCKSPASSWORD_END
	// HTTPPROXYAUTHMETHOD_BEGIN
	void get_HttpProxyAuthMethod(CkString &str);
	const char *httpProxyAuthMethod(void);
	void put_HttpProxyAuthMethod(const char *newVal);
	// HTTPPROXYAUTHMETHOD_END
	// HTTPPROXYUSERNAME_BEGIN
	void get_HttpProxyUsername(CkString &str);
	const char *httpProxyUsername(void);
	void put_HttpProxyUsername(const char *newVal);
	// HTTPPROXYUSERNAME_END
	// HTTPPROXYPASSWORD_BEGIN
	void get_HttpProxyPassword(CkString &str);
	const char *httpProxyPassword(void);
	void put_HttpProxyPassword(const char *newVal);
	// HTTPPROXYPASSWORD_END
	// HTTPPROXYHOSTNAME_BEGIN
	void get_HttpProxyHostname(CkString &str);
	const char *httpProxyHostname(void);
	void put_HttpProxyHostname(const char *newVal);
	// HTTPPROXYHOSTNAME_END
	// HTTPPROXYPORT_BEGIN
	int get_HttpProxyPort(void);
	void put_HttpProxyPort(int newVal);
	// HTTPPROXYPORT_END
	// CHANNELRECEIVEUNTILMATCHN_BEGIN
	bool ChannelReceiveUntilMatchN(int channelNum, CkStringArray &matchPatterns, const char *charset, bool caseSensitive);
	// CHANNELRECEIVEUNTILMATCHN_END
	// CHANNELREADANDPOLL2_BEGIN
	int ChannelReadAndPoll2(int channelNum, int pollTimeoutMs, int maxNumBytes);
	// CHANNELREADANDPOLL2_END
	// PASSWORDCHANGEREQUESTED_BEGIN
	bool get_PasswordChangeRequested(void);
	// PASSWORDCHANGEREQUESTED_END
	// AUTHENTICATEPWPK_BEGIN
	bool AuthenticatePwPk(const char *username, const char *password, CkSshKey &privateKey);
	// AUTHENTICATEPWPK_END

	// SSH_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)


#endif


