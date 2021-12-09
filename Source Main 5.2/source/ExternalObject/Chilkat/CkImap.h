// CkImap.h: interface for the CkImap class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKIMAP_H
#define _CKIMAP_H

#pragma once

class CkByteData;
#include "CkString.h"
class CkMailboxes;
class CkMessageSet;
class CkEmail;
class CkEmailBundle;
class CkStringArray;
class CkImapProgress;
class CkCert;
class CkPrivateKey;
class CkSshKey;

#if !defined(WIN32) && !defined(WINCE)
#include "SystemTime.h"
#endif

#include "CkObject.h"

#undef Copy

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#pragma pack (push, 8) 

// CLASS: CkImap
class CkImap  : public CkObject
{
    public:
	CkImap();
	virtual ~CkImap();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	bool SetDecryptCert2(const CkCert &cert, CkPrivateKey &key);

	bool Connect(const char *hostname);
	bool Disconnect(void);
	bool IsConnected(void);

	bool Login(const char *login, const char *password);
	bool Logout(void);
	bool IsLoggedIn(void);

	void get_LoggedInUser(CkString &str);
	void get_ConnectedToHost(CkString &str);

	bool CreateMailbox(const char *mailbox);
	bool DeleteMailbox(const char *mailbox);
	bool RenameMailbox(const char *fromMailbox, const char *toMailbox);

	// Gets the last response returned by the IMAP server.
	void LastResponse(CkString &str);

	CkMailboxes *ListMailboxes(const char *reference, const char *wildcardedMailbox);

	bool SelectMailbox(const char *mailbox);    // Email flags are modified when full email is fetched.
	bool ExamineMailbox(const char *mailbox);   // Email flags are not modified, mailbox cannot be modified.

	bool Subscribe(const char *mailbox);   
	bool Unsubscribe(const char *mailbox);   

	bool SetSslClientCert(CkCert &cert);
	CkCert *GetSslServerCert(void);
	bool get_SslServerCertVerified(void);

	// Search for messages and return a message set of either UIDs (if bUid = true) or sequence numbers.
	CkMessageSet *Search(const char *criteria, bool bUid);

	CkMessageSet *GetAllUids(void);

	CkEmail *FetchSingle(long msgId, bool bUid);
	CkEmail *FetchSingleHeader(long msgId, bool bUid);
	CkEmailBundle *FetchBundle(const CkMessageSet &messageSet);
	CkEmailBundle *FetchHeaders(const CkMessageSet &messageSet);
	CkEmailBundle *FetchChunk(int startSeqNum, int count, CkMessageSet &failedSet, CkMessageSet &fetchedSet);

	CkStringArray *FetchSequenceAsMime(int startSeqNum, int count);
	CkEmailBundle *FetchSequence(int startSeqNum, int numMessages);
	CkEmailBundle *FetchSequenceHeaders(int startSeqNum, int numMessages);

	bool FetchSingleAsMime(long msgId, bool bUid, CkString &strMime);
	CkStringArray *FetchBundleAsMime(const CkMessageSet &messageSet);

	bool Expunge(void);
	bool ExpungeAndClose(void);

	bool StoreFlags(long msgId, bool bUid, const char *flagName, long value);
	bool FetchFlags(long msgId, bool bUid, CkString &strFlags);

	bool SetFlag(long msgId, bool bUid, const char *flagName, long value);
	bool SetFlags(const CkMessageSet &messageSet, const char *flagName, long value);

	void get_AuthMethod(CkString &str);
	void put_AuthMethod(const char *str);

	char get_SeparatorChar(void);
	void put_SeparatorChar(char c);

	void get_Domain(CkString &str);
	void put_Domain(const char *str);

	int get_Port(void);
	void put_Port(int port);

	bool get_Ssl(void);
	void put_Ssl(bool s);

	void ClearSessionLog(void);
	void get_SessionLog(CkString &str);
	bool get_KeepSessionLog(void);
	void put_KeepSessionLog(bool newVal);

	void get_LastResponse(CkString &str);
	void get_LastIntermediateResponse(CkString &str);
	void get_LastCommand(CkString &str);

	// These times are in milliseconds.
	long get_HeartbeatMs(void);
	void put_HeartbeatMs(long newVal);

	bool AppendMime(const char *mailbox, const char *mimeText);
	bool AppendMail(const char *mailbox, const CkEmail &email);

	// Copy a message from the currently selected mailbox to a new mailbox.
	bool Copy(long msgId, bool bUid, const char *copyToMailbox);
	bool CopyMultiple(CkMessageSet &messageSet, const char *copyToMailbox);
	bool CopySequence(int startSeqNum, int count, const char *copyToMailbox);

	bool SetMailFlag(const CkEmail &email, const char *flagName, long value);
	long GetMailFlag(const CkEmail &email, const char *flagName);
	// Query the IMAP server get get up-to-date flag information.
	bool RefetchMailFlags(CkEmail &emailInOut);

	long get_NumMessages(void);

	// When headers are downloaded, information about the email can be retrieved
	// using these methods.
	long GetMailSize(const CkEmail &email);
	long GetMailNumAttach(const CkEmail &email);
	long GetMailAttachSize(const CkEmail &email, long attachIndex);
	bool GetMailAttachFilename(const CkEmail &email, long attachIndex, CkString &strFilename);

	// Automatically decrypt and/or verify signatures (the default).
	// Results of the "unwrapping" are available via the CkEmail object.
	bool get_AutoUnwrapSecurity(void);
	void put_AutoUnwrapSecurity(bool bUnwrap);

	// When peek-mode is true, fetching entire emails will not affect
	// the email flags, regardless of whether the mailbox was "selected"
	// using SelectMailbox or "examined" with ExamineMailbox.
	bool get_PeekMode(void);
	void put_PeekMode(bool bPeek);

	// If AppendSeen is true (the default), then AppendMail and AppendMime will append
	// an email as already-seen, otherwise it will be initialized to "unseen".
	bool get_AppendSeen(void);
	void put_AppendSeen(bool alreadySeen);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);


	bool UnlockComponent(const char *unlockCode);
	bool IsUnlocked(void) const;


	// timeouts in seconds...
	int get_ReadTimeout(void) const;
	void put_ReadTimeout(int numSeconds);

	int get_ConnectTimeout(void) const;
	void put_ConnectTimeout(int numSeconds);


	bool SendRawCommand(const char *rawCommand, CkString &rawResponse);

        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);

	const char *getMailAttachFilename(const CkEmail &email, long attachIndex);
	const char *fetchFlags(long msgId, bool bUid);
        const char *authMethod(void);
        const char *domain(void);
	const char *fetchSingleAsMime(long msgId, bool bUid);
	const char *loggedInUser(void);
	const char *connectedToHost(void);
	const char *sendRawCommand(const char *rawCommand);
	const char *sessionLog(void);

	const char *lastResponse(void);
	const char *lastCommand(void);
	const char *lastIntermediateResponse(void);
	// LASTAPPENDEDMIME_BEGIN
	void get_LastAppendedMime(CkString &str);
	const char *lastAppendedMime(void);
	// LASTAPPENDEDMIME_END
	// VERSION_BEGIN
	void get_Version(CkString &str);
	const char *version(void);
	// VERSION_END
	// APPENDMIMEWITHFLAGS_BEGIN
	bool AppendMimeWithFlags(const char *mailbox, const char *mimeText, bool seen, bool flagged, bool answered, bool draft);
	// APPENDMIMEWITHFLAGS_END
	// FETCHSINGLEHEADERASMIME_BEGIN
	bool FetchSingleHeaderAsMime(int msgId, bool bUID, CkString &outStr);
	const char *fetchSingleHeaderAsMime(int msgId, bool bUID);
	// FETCHSINGLEHEADERASMIME_END

	CkImapProgress *get_EventCallbackObject(void) const { return m_callback; }
	void put_EventCallbackObject(CkImapProgress *progress) { m_callback = progress; }
	// APPENDMIMEWITHDATE_BEGIN
	// SYSTEMTIME is defined in <windows.h>
	// For more information, Google "SYSTEMTIME"
	bool AppendMimeWithDate(const char *mailbox, const char *mimeText, SYSTEMTIME &internalDate);
	// APPENDMIMEWITHDATE_END
	// SELECTEDMAILBOX_BEGIN
	void get_SelectedMailbox(CkString &str);
	const char *selectedMailbox(void);
	// SELECTEDMAILBOX_END
	// APPENDUID_BEGIN
	int get_AppendUid(void);
	// APPENDUID_END
	// AUTHZID_BEGIN
	void get_AuthzId(CkString &str);
	const char *authzId(void);
	void put_AuthzId(const char *newVal);
	// AUTHZID_END
	// SENDBUFFERSIZE_BEGIN
	int get_SendBufferSize(void);
	void put_SendBufferSize(int newVal);
	// SENDBUFFERSIZE_END
	// SSHTUNNEL_BEGIN
	bool SshTunnel(const char *sshServerHostname, int sshServerPort);
	// SSHTUNNEL_END
	// SSHAUTHENTICATEPW_BEGIN
	bool SshAuthenticatePw(const char *sshLogin, const char *sshPassword);
	// SSHAUTHENTICATEPW_END
	// SSHAUTHENTICATEPK_BEGIN
	bool SshAuthenticatePk(const char *sshLogin, CkSshKey &privateKey);
	// SSHAUTHENTICATEPK_END
	// NOOP_BEGIN
	bool Noop(void);
	// NOOP_END
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
	// SOCKSPORT_BEGIN
	int get_SocksPort(void);
	void put_SocksPort(int newVal);
	// SOCKSPORT_END
	// SOCKSVERSION_BEGIN
	int get_SocksVersion(void);
	void put_SocksVersion(int newVal);
	// SOCKSVERSION_END
	// SETSSLCLIENTCERTPFX_BEGIN
	bool SetSslClientCertPfx(const char *pfxFilename, const char *pfxPassword, const char *certSubjectCN);
	// SETSSLCLIENTCERTPFX_END
	// STARTTLS_BEGIN
	bool get_StartTls(void);
	void put_StartTls(bool newVal);
	// STARTTLS_END
	// SSLPROTOCOL_BEGIN
	void get_SslProtocol(CkString &str);
	const char *sslProtocol(void);
	void put_SslProtocol(const char *newVal);
	// SSLPROTOCOL_END
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
	// HTTPPROXYAUTHMETHOD_BEGIN
	void get_HttpProxyAuthMethod(CkString &str);
	const char *httpProxyAuthMethod(void);
	void put_HttpProxyAuthMethod(const char *newVal);
	// HTTPPROXYAUTHMETHOD_END
	// HTTPPROXYHOSTNAME_BEGIN
	void get_HttpProxyHostname(CkString &str);
	const char *httpProxyHostname(void);
	void put_HttpProxyHostname(const char *newVal);
	// HTTPPROXYHOSTNAME_END
	// HTTPPROXYPORT_BEGIN
	int get_HttpProxyPort(void);
	void put_HttpProxyPort(int newVal);
	// HTTPPROXYPORT_END
	// UIDNEXT_BEGIN
	int get_UidNext(void);
	// UIDNEXT_END
	// AUTOFIX_BEGIN
	bool get_AutoFix(void);
	void put_AutoFix(bool newVal);
	// AUTOFIX_END
	// ADDPFXSOURCEDATA_BEGIN
	bool AddPfxSourceData(CkByteData &pfxData, const char *password);
	// ADDPFXSOURCEDATA_END
	// ADDPFXSOURCEFILE_BEGIN
	bool AddPfxSourceFile(const char *pfxFilePath, const char *password);
	// ADDPFXSOURCEFILE_END

	// IMAP_INSERT_POINT

	// END PUBLIC INTERFACE



    // For internal use only.
    private:
	CkImapProgress *m_callback;

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkImap(const CkImap &) { } 
	CkImap &operator=(const CkImap &) { return *this; }
	CkImap(void *impl) : m_impl(impl) { }

	//CkEmailBundle *FetchBundle(const CkMessageSet *messageSet) { return 0; }
	//CkEmailBundle *FetchHeaders(const CkMessageSet *messageSet) { return 0; }
	//CkEmailBundle *FetchChunk(int startSeqNum, int count, CkMessageSet *failedSet, CkMessageSet *fetchedSet) { return 0; }
	//CkStringArray *FetchBundleAsMime(const CkMessageSet *messageSet) { return 0; }
	//bool SetFlags(const CkMessageSet *messageSet, const char *flagName, long value) { return false; }

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const { return m_impl; } 




};
#pragma pack (pop)


#endif


