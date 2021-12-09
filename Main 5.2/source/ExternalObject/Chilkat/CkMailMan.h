// CkMailMan.h: interface for the CkMailMan class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKMAILMAN_H
#define _CKMAILMAN_H


#pragma once

#include "CkString.h"
class CkByteData;
class CkEmail;
class CkEmailBundle;
class CkStringArray;
class CkMailProgress;
class CkPrivateKey;
class CkCert;
class CkSshKey;

#include "CkObject.h"

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#pragma pack (push, 8) 

// CLASS: CkMailMan
class CkMailMan  : public CkObject
{
    public:
	CkMailMan();
	virtual ~CkMailMan();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);


	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
	// Error information for the last method call formatted as XML. 
        void LastErrorXml(CkString &str);
	// Error information for the last method call formatted as HTML with "<br>" line endings. 
        void LastErrorHtml(CkString &str);
	// Error information for the last method call formatted as plain-text with CRLF line endings. 
        void LastErrorText(CkString &str);

	bool SetDecryptCert2(const CkCert &cert, CkPrivateKey &key);
	
	void get_SmtpPassword(CkString &str);
	void put_SmtpPassword(const char *str);

	void get_SmtpUsername(CkString &str);
	void put_SmtpUsername(const char *str);

	void get_DsnNotify(CkString &str);
	void put_DsnNotify(const char *str);

	void get_DsnEnvid(CkString &str);
	void put_DsnEnvid(const char *str);

	void get_DsnRet(CkString &str);
	void put_DsnRet(const char *str);

	// The last SMTP diagnostic status code
	long get_LastSmtpStatus(void);

	bool SetSslClientCert(CkCert &cert);
	CkCert *GetPop3SslServerCert(void);
	CkCert *GetSmtpSslServerCert(void);
	bool get_Pop3SslServerCertVerified(void);
	bool get_SmtpSslServerCertVerified(void);

	// Send AbortCheck events periodically according to the 
	// heartbeat interval.  0 indicates no heartbeat events..
	long get_HeartbeatMs(void);
	void put_HeartbeatMs(long millisec);

	// The maximum number of seconds to wait for new data to arrive 
	// from the POP3 server. A value of 0 (the default) sets the timeout 
	// to infinity. Setting a reasonable timeout value will prevent your 
	// application from hanging if the POP3 server stops responding.
	long get_ReadTimeout(void);
	void put_ReadTimeout(long numSeconds);

	// The maximum number of seconds to wait when connecting to a POP3 or 
	// SMTP server. The default is 5 seconds.
	long get_ConnectTimeout(void);
	void put_ConnectTimeout(long numSeconds);

	// When an email is downloaded from a POP3 server, or when it is loaded from a file, 
	// CkMailMan automatically "unwraps" it if the email is signed and/or encrypted. 
	// The results of the digital signature verification and decryption can be accessed 
	// with the following ChilkatEmail properties and methods: EncryptedBy, SignedBy, 
	// ReceivedEncrypted, ReceivedSigned, GetEncryptedByCert, and GetSignedByCert. 
	// The AutoUnwrapSecurity property controls whether the automatic unwrap occurs or not. 
	// By default, this property is set to 1, which enables the auto-unwrap. Setting the 
	// property to 0 disables it, allowing for the S/MIME message parts or envelopes to be 
	// saved or accessed. 
	bool get_AutoUnwrapSecurity(void);
	void put_AutoUnwrapSecurity(bool newVal);

	// Controls whether the email date is reset to the current date/time whenever it is 
	// re-created by loading it from a file or memory. This affects the following methods: 
	// LoadMbx, LoadEml, LoadMime, LoadXmlFile, and LoadXmlString.
	// The default value is false.
	bool get_ResetDateOnLoad(void);
	void put_ResetDateOnLoad(bool newVal);

	// Controls whether opaque signing or clear-text multipart signing is used
	// when sending signed email.
	bool get_OpaqueSigning(void);
	void put_OpaqueSigning(bool newVal);

	// Option for sending email.  If the SMTP server reports an error for any recipient,
	// then the email will not be sent, even if some recipients are valid.
	// AllOrNone is false by default.
	bool get_AllOrNone(void);
	void put_AllOrNone(bool newVal);

	// POP3 Secure Password Authentication.
	bool get_Pop3SPA(void);
	void put_Pop3SPA(bool newVal);

	// SMTP STARTTLS
	bool get_StartTLS(void);
	void put_StartTLS(bool newVal);

	// Option when sending signed email.  This controls whether intermediate
	// certificates in the chain of authentication (excluding the root) are 
	// included in a signed email.
	// EmbedCertChain is false by default.
	bool get_EmbedCertChain(void);
	void put_EmbedCertChain(bool newVal);

	// Controls whether certificates found in incoming signed emails are 
	// automatically saved to the "Chilkat" certificate store. The default is TRUE. 
	// The Chilkat cert store is located in the registry in HKEY_LOCAL_MACHINE/Software/Chilkat/SystemCertificates.
	bool get_AutoSaveCerts(void);
	void put_AutoSaveCerts(bool newVal);

	// If true, connect to POP3 server using SSL
	bool get_PopSsl(void);
	void put_PopSsl(bool newVal);

	// If true, connect to SMTP server using SSL
	bool get_SmtpSsl(void);
	void put_SmtpSsl(bool newVal);

	// This property determines how emails are sent to distribution lists. 
	// If True, emails are sent to each recipient in the list one at a 
	// time, with the "To" header field containing the email address of the recipient. 
	// If False, emails will contain <undisclosed-recipients> in the "To" header field, 
	// and are sent to 100 BCC recipients at a time. As an example, if your 
	// distribution list contained 350 email addresses, 4 emails would be sent, 
	// the first 3 having 100 BCC recipients, and the last email with 50 BCC recipients.
	// The default is value of this property is True
	bool get_SendIndividual(void);
	void put_SendIndividual(bool newVal);

	// This property should usually be left empty. The CkMailMan will by 
	// default choose the most secure login method available to prevent 
	// unencrypted username and passwords from being transmitted if possible. 
	// However, some SMTP servers may not advertise the authorization methods 
	// it accepts to some or all clients, and therefore Chilkat Mail cannot 
	// determine which authorization method to use. To force a particular method 
	// to be used, or to prevent any authorization from being used, set this 
	// property to one of the following values: "NONE", "LOGIN", "PLAIN", 
	// "CRAM-MD5", or "NTLM". 
	void get_SmtpAuthMethod(CkString &str);
	void put_SmtpAuthMethod(const char *str);

	// The Windows domain for logging into the SMTP server. Use this only 
	// if your SMTP server requires NTLM authentication, which means your 
	// SMTP server uses Integrated Windows Authentication. If there is no domain, 
	// this can be left empty
	void get_SmtpLoginDomain(CkString &str);
	void put_SmtpLoginDomain(const char *str);

	// An expression that is applied to any of the following method calls when present: 
	// LoadXmlFile, LoadXmlString, LoadMbx, CopyMail, and TransferMail. 
	// For these methods, only the emails that match the filter's expression are returned 
	// in the email bundle. In the case of TransferMail, only the matching emails are removed 
	// from the POP3 server. The filter allows any header field, or the body, to be checked. 
	// Here are some examples of expressions:
	// 
	// Body like "mortgage rates*". 
	// Subject contains "update" and From contains "chilkat" 
	// To = "info@chilkatsoft.com" 
	// 
	// Here are the general rules for forming filter expressions:
	// 
	// Any MIME header field name can be used, case is insensitive. 
	// Literal strings are double-quoted, and case is insensitive. 
	// The "*" wildcard matches 0 or more occurances of any character. 
	// Parentheses can be used to control precedence. 
	// The logical operators are: AND, OR, NOT (case insensitive) 
	// Comparison operators are: =, <, >, <=, >=, <> 
	// String comparison operators are: CONTAINS, LIKE (case insensitive)
	void get_Filter(CkString &str);
	void put_Filter(const char *str);

	// Tells the mailman to not read any messages that are greater than this size in bytes.
	long get_SizeLimit(void);
	void put_SizeLimit(long newVal);

	// Tells the mailman to never read more than this many messages at a time from the POP3 server.
	long get_MaxCount(void);
	void put_MaxCount(long newVal);

	// The port number of the POP3 server. Only needs to be set if the 
	// POP3 server is running on a non-standard port
	long get_MailPort(void);
	void put_MailPort(long newVal);

	long get_SmtpPort(void);
	void put_SmtpPort(long newVal);

	void get_PopPassword(CkString &str);
	void put_PopPassword(const char *str);
	void put_PopPasswordBase64(const char *strBase64);

	void get_PopUsername(CkString &str);
	void put_PopUsername(const char *str);

	// The hostname where the POP3 server is located
	void get_MailHost(CkString &str);
	void put_MailHost(const char *str);

	// The domain name passed when saying hello to the SMTP server.
	void get_HeloHostname(CkString &str);
	void put_HeloHostname(const char *str);

	// Get the filename of the file created by the most recent call to SendQ
	void get_LastSendQFilename(CkString &str);

	void get_Version(CkString &str);

	void get_SmtpHost(CkString &str);
	void put_SmtpHost(const char *str);

	// Automatically generate unique message ID during the mail sending process.
	// (default is true)
	bool get_AutoGenMessageId(void);
	void put_AutoGenMessageId(bool newVal);


	// A file where each message is logged in the form it was received from a POP3 server. 
	// This property is provided for help in debugging. If there is a problem with a particular 
	// email, please send a problem report to support@chilkatsoft.com, and include the email that 
	// was logged to this file.
	void get_LogMailReceivedFilename(CkString &str);
	void put_LogMailReceivedFilename(const char *str);

	// A filename where each message is logged in the form it is sent to the SMTP server. 
	// This property is provided for help in debugging. If the mail being sent via ChilkatMail 
	// is not received as expected, please send a problem report to support@chilkatsoft.com and 
	// include the email that was logged to this file.
	void get_LogMailSentFilename(CkString &str);
	void put_LogMailSentFilename(const char *str);


	// Clears the list of bad email addresses that may have accumulated during SMTP mail sending. 
	// (see GetBadEmailAddresses below)
        void ClearBadEmailAddresses(void);

	// Returns a CkStringArray object containing the list of failed email addresses that have 
	// accumulated during SMTP sends. The list will not contain duplicates. Also, this only works 
	// with some SMTP servers -- not all SMTP servers check the validity of each email address.
        CkStringArray *GetBadEmailAddresses(void);

	// Returns the UIDLs of the emails currently stored on the POP3 server.
	CkStringArray *GetUidls(void);

	// Return true if the SMTP server is DSN capable
	bool IsSmtpDsnCapable(void);

	// Return true if login successful.
	bool VerifySmtpLogin(void);

	// Return true if connection is possible.
	bool VerifySmtpConnection(void);

	// Attempts to login to the POP server and returns true if successful, otherwise returns false.
	bool VerifyPopLogin(void);

	// Returns true if a connection can be made to the POP server, otherwise returns false.
	bool VerifyPopConnection(void);

	bool FetchMime(const char *uidl, CkByteData &mimeBytes);

	int GetSizeByUidl(const char *uidl);

	unsigned long GetMailboxSize(void);
	int GetMailboxCount(void); // Same as CheckMail
	bool Pop3BeginSession(void);
	bool Pop3EndSession(void);
	bool Pop3Noop(void);
	bool Pop3Reset(void);

	// Given an array of UIDL strings, this method fetches all the emails from 
	// the POP3 server whose UIDL is present in the array, and returns the emails 
	// in a bundle. The fetched emails are not removed from the POP3 server.
	CkEmailBundle *FetchMultiple(const CkStringArray *uidlArray);
	CkEmailBundle *FetchMultiple(const CkStringArray &uidlArray) { return this->FetchMultiple(&uidlArray); }

	CkStringArray *FetchMultipleMime(const CkStringArray *uidlArray);
	CkStringArray *TransferMultipleMime(const CkStringArray *uidlArray);
	CkStringArray *FetchMultipleMime(const CkStringArray &uidlArray) { return this->FetchMultipleMime(&uidlArray); }
	CkStringArray *TransferMultipleMime(const CkStringArray &uidlArray) { return this->TransferMultipleMime(&uidlArray); }

	// Given an array of UIDL strings, this method fetchs all the email headers from 
	// the POP3 server whose UIDL is present in the array.
	// The fetched emails are not removed from the POP3 server.
	CkEmailBundle *FetchMultipleHeaders(const CkStringArray *uidlArray, long numBodyLines);
	CkEmailBundle *FetchMultipleHeaders(const CkStringArray &uidlArray, long numBodyLines) { return this->FetchMultipleHeaders(&uidlArray,numBodyLines); }

	// Removes a single email from the server. Returns the a status code having one of the following values:
	//     -2 -- Failed to connect to the POP3 server.
	//     -1 -- Connected to the POP3 server, but failed to login or issue delete command
	//     0 -- The UIDL was not found on the server.
	//     1 -- The UIDL existed on the server and was deleted.
	bool DeleteByUidl(const char *uidl);

	// Fetches an email from the POP3 server given its UIDL. 
	// The email is left on the POP3 server. A typical program might get 
	// the email headers from the POP3 server by calling GetAllHeaders or 
	// GetHeaders, and then fetch individual emails via their UIDL properties 
	// one at a time.
	CkEmail *FetchEmail(const char *uidl);

	// Removes from the POP3 server any email that has a UIDL equal to one of the 
	// UIDLs in the array.
	bool DeleteMultiple(const CkStringArray *uidlArray);
	bool DeleteMultiple(const CkStringArray &uidlArray) { return this->DeleteMultiple(&uidlArray); }

	// Removes all the emails present in the bundle from the POP3 server. 
	bool DeleteBundle(const CkEmailBundle *bundle);
	bool DeleteBundle(const CkEmailBundle &bundle) { return this->DeleteBundle(&bundle); }

	// If a partial email was obtained using GetHeaders or GetAllHeaders, this function 
	// will take the partial email as an argument, and download the full email from the server. 
	// A new CkEmail object (separate from the partial email) is returned.
	CkEmail *GetFullEmail(const CkEmail *email);
	CkEmail *GetFullEmail(const CkEmail &email) { return this->GetFullEmail(&email); }

	// The same as the GetAllHeaders method, except only the emails from fromIndex 
	// to toIndex on the POP3 server are returned. The first email on the server is at 
	// index 0.
	CkEmailBundle *GetHeaders(long numBodyLines, long fromIndex, long toIndex);

	CkEmail *FetchSingleHeader(long numBodyLines, long index);
	CkEmail *FetchSingleHeaderByUidl(long numBodyLines, const char *uidl);

	// Removes an email from the POP3 server. The email argument is an email that was 
	// previously downloaded from the POP3 server, either in its entirety, or partially 
	// (as with GetAllHeaders or GetHeaders).
	bool DeleteEmail(const CkEmail *email);
	bool DeleteEmail(const CkEmail &email) { return this->DeleteEmail(&email); }

	// If true, then all methods that mark email for deletion will automatically
	// send a QUIT command and disconnect so that the email is deleted from the
	// POP3 server.  The default value of this property is true.
	bool get_ImmediateDelete(void);
	void put_ImmediateDelete(bool newVal);

	// Returns all the emails from the POP3 server, but only the first numBodyLines lines 
	// of the body. Attachments are not returned. The emails returned in the CkEmailBundle 
	// are valid CkEmail objects, the only difference is that the body is truncated to 
	// include only the top numBodyLines lines.
	CkEmailBundle *GetAllHeaders(long numBodyLines);

	// Loads from an XML string containing emails and returns an email bundle. 
	// If a Filter is present, only emails matching the filter are returned.
	CkEmailBundle *LoadXmlString(const char *xmlString);

	// Loads an XML file containing emails and returns an email bundle. 
	// If a Filter is present, only emails matching the filter are returned.
	CkEmailBundle *LoadXmlFile(const char *folderPath);

	// Loads an XML file containing a single email and returns a CkEmail object.
	CkEmail *LoadXmlEmail(const char *folderPath);
#ifndef WINCE
	bool MxLookup(const char *emailAddr, CkString &strHostname);
	CkStringArray *MxLookupAll(const char *emailAddress);
#endif
	// Loads an XML string containing a single email and returns a CkEmail object
	CkEmail *LoadXmlEmailString(const char *xmlString);

	// A quick way to send an email to a single recipient without having to create a CkEmail object 
	// explicitly. Returns true on successful SMTP server handoff, and false if failed. 
	bool QuickSend(const char *from, const char *to, const char *subject, const char *body, const char *smtpServer);

	bool UnlockComponent(const char *code);

	// Return the number of messages available on the POP3 server.
	// Returns -1 if unable to check the POP server for any reason.
	long CheckMail(void);

	// Transfer the emails from the POP3 server into a CkEmailBundle.
	CkEmailBundle *TransferMail(void);

	// Copy the mail from a POP3 server into a CkEmailBundle. 
	// This does not remove the email from the POP3 server.
	CkEmailBundle *CopyMail(void);

	// Sends multiple emails at once.  Returns true if all emails were sent
	// without errors, otherwise returns false.  This is the same as 
	// looping over each email in a bundle and calling SendEmail for each.
	bool SendBundle(const CkEmailBundle *bundle);
	bool SendBundle(const CkEmailBundle &bundle) { return this->SendBundle(&bundle); }

	// Send an email. Returns true if the hand-off to the SMTP server was 
	// successful for all recipients, otherwise returns false.
	bool SendEmail(const CkEmail *email);
	bool SendEmail(const CkEmail &email) { return this->SendEmail(&email); }

	// Initiate the SMTP session as if sending email, passing the "RCPT TO" command
	// to the SMTP server for each recipients, but abort the SMTP session prior to sending email.
	// Any bad email addresses reported by the SMTP server are saved in saBadAddrs.
	bool VerifyRecipients(const CkEmail &email, CkStringArray &saBadAddrs);

	// Sends an email via the Chilkat smtpQ service. Returns true for success, false for failure.
#ifndef WINCE
	bool SendQ(const CkEmail *email);
	bool SendQ2(const CkEmail *email, const char *queueDir);
	bool SendQ(const CkEmail &email) { return this->SendQ(&email); }
	bool SendQ2(const CkEmail &email, const char *queueDir) { return this->SendQ2(&email,queueDir); }
#endif
	// Loads a .mbx file containing emails and returns an email bundle. 
	// If a Filter is present, only emails matching the filter are returned. 
	CkEmailBundle *LoadMbx(const char *mbxFileName);

	// Reads a .eml file returns it as a CkEmail object.
	CkEmail *LoadEml(const char *emlFilename);

	// Creates a CkEmail object from a MIME string.
	CkEmail *LoadMime(const char *mimeText);

	// Returns true if the component is unlocked.
	bool IsUnlocked(void) const;

	// When an email is sent by calling SendEmail, it is first "rendered" according to the values 
	// of the email properties and contents. It may be digitally signed, encrypted, values 
	// substituted for replacement patterns, and header fields "Q" or "B" encoded as needed based on 
	// the email. The RenderToMime method performs the rendering, but without the actual sending. 
	// The MIME text produced is exactly what would be sent to the SMTP server had SendEmail been 
	// called. (The SendEmail method is effectively the same as calling RenderToMime followed by a 
	// call to SendMime.) 
	bool RenderToMime(const CkEmail *email, CkString &str);
	bool RenderToMime(const CkEmail &email, CkString &str) { return this->RenderToMime(&email,str); }

	bool SendMime(const char *from, const char *recipients, const char *mimeMsg);
	bool SendMimeQ(const char *from, const char *recipients, const char *mimeMsg);


	bool OpenSmtpConnection(void);
	bool CloseSmtpConnection(void);
	bool SmtpReset(void);
	bool SmtpNoop(void);

	bool GetMailboxInfoXml(CkString &xml);

	void get_SmtpSessionLog(CkString &log);
	void ClearSmtpSessionLog(void);
        const char *smtpSessionLog(void);

	void get_Pop3SessionLog(CkString &log);
	void ClearPop3SessionLog(void);
        const char *pop3SessionLog(void);

	void SetProgressCallback(CkMailProgress *cb) { m_callback = cb; }

        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);
	const char *smtpPassword(void);
	const char *smtpUsername(void);
	const char *dsnNotify(void);
	const char *dsnEnvid(void);
	const char *dsnRet(void);
	const char *smtpAuthMethod(void);
	const char *smtpLoginDomain(void);
	const char *filter(void);
	const char *popPassword(void);
	const char *popUsername(void);
	const char *mailHost(void);
	const char *heloHostname(void);
	const char *lastSendQFilename(void);
	const char *version(void);
	const char *smtpHost(void);
	const char *logMailReceivedFilename(void);
	const char *logMailSentFilename(void);
#ifndef WINCE
	const char *mxLookup(const char *emailAddr);
#endif
	const char *renderToMime(const CkEmail &email);
	const char *getMailboxInfoXml(void);

	// SENDMIMETOLIST_BEGIN
	bool SendMimeToList(const char *from, const char *distListFile, const char *mimeText);
	// SENDMIMETOLIST_END
	// SENDTODISTRIBUTIONLIST_BEGIN
	bool SendToDistributionList(CkEmail &email, CkStringArray &array);
	// SENDTODISTRIBUTIONLIST_END
	// CLIENTIPADDRESS_BEGIN
	void get_ClientIpAddress(CkString &str);
	const char *clientIpAddress(void);
	void put_ClientIpAddress(const char *newVal);
	// CLIENTIPADDRESS_END
	// POP3SESSIONID_BEGIN
	int get_Pop3SessionId(void);
	// POP3SESSIONID_END
	// DELETEBYMSGNUM_BEGIN
	bool DeleteByMsgnum(int msgnum);
	// DELETEBYMSGNUM_END
	// FETCHBYMSGNUM_BEGIN
	CkEmail *FetchByMsgnum(int msgnum);
	// FETCHBYMSGNUM_END
	// FETCHMIMEBYMSGNUM_BEGIN
	bool FetchMimeByMsgnum(int msgnum, CkByteData &outBytes);
	// FETCHMIMEBYMSGNUM_END
	// SENDBUFFERSIZE_BEGIN
	int get_SendBufferSize(void);
	void put_SendBufferSize(int newVal);
	// SENDBUFFERSIZE_END
	// SSHTUNNEL_BEGIN
	bool SshTunnel(bool bSmtp, const char *sshServerHostname, int sshServerPort);
	// SSHTUNNEL_END
	// SSHAUTHENTICATEPW_BEGIN
	bool SshAuthenticatePw(bool bSmtp, const char *sshLogin, const char *sshPassword);
	// SSHAUTHENTICATEPW_END
	// SSHAUTHENTICATEPK_BEGIN
	bool SshAuthenticatePk(bool bSmtp, const char *sshLogin, CkSshKey &privateKey);
	// SSHAUTHENTICATEPK_END
	// SSHCLOSETUNNEL_BEGIN
	bool SshCloseTunnel(bool bSmtp);
	// SSHCLOSETUNNEL_END
	// INCLUDEROOTCERT_BEGIN
	bool get_IncludeRootCert(void);
	void put_IncludeRootCert(bool newVal);
	// INCLUDEROOTCERT_END
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
	// SENDMIMEBYTES_BEGIN
	bool SendMimeBytes(const char *from, const char *recipients, CkByteData &mimeData);
	// SENDMIMEBYTES_END
	// RENDERTOMIMEBYTES_BEGIN
	bool RenderToMimeBytes(CkEmail &email, CkByteData &outBytes);
	// RENDERTOMIMEBYTES_END
#ifndef WINCE
	// SENDMIMEBYTESQ_BEGIN
	bool SendMimeBytesQ(const char *from, const char *recipients, CkByteData &mimeData);
	// SENDMIMEBYTESQ_END
#endif
	// HTTPPROXYHOSTNAME_BEGIN
	void get_HttpProxyHostname(CkString &str);
	const char *httpProxyHostname(void);
	void put_HttpProxyHostname(const char *newVal);
	// HTTPPROXYHOSTNAME_END
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
	// HTTPPROXYPORT_BEGIN
	int get_HttpProxyPort(void);
	void put_HttpProxyPort(int newVal);
	// HTTPPROXYPORT_END
	// AUTOFIX_BEGIN
	bool get_AutoFix(void);
	void put_AutoFix(bool newVal);
	// AUTOFIX_END
	// VERBOSELOGGING_BEGIN
	bool get_VerboseLogging(void);
	void put_VerboseLogging(bool newVal);
	// VERBOSELOGGING_END
	// POP3STLS_BEGIN
	bool get_Pop3Stls(void);
	void put_Pop3Stls(bool newVal);
	// POP3STLS_END
	// ADDPFXSOURCEDATA_BEGIN
	bool AddPfxSourceData(CkByteData &pfxData, const char *password);
	// ADDPFXSOURCEDATA_END
	// ADDPFXSOURCEFILE_BEGIN
	bool AddPfxSourceFile(const char *pfxFilePath, const char *password);
	// ADDPFXSOURCEFILE_END
	// SMTPSENDRAWCOMMAND_BEGIN
	bool SmtpSendRawCommand(const char *command, const char *charset, bool bEncodeBase64, CkString &outStr);
	const char *smtpSendRawCommand(const char *command, const char *charset, bool bEncodeBase64);
	// SMTPSENDRAWCOMMAND_END
	// POP3SENDRAWCOMMAND_BEGIN
	bool Pop3SendRawCommand(const char *command, const char *charset, CkString &outStr);
	const char *pop3SendRawCommand(const char *command, const char *charset);
	// POP3SENDRAWCOMMAND_END
	// POP3ENDSESSIONNOQUIT_BEGIN
	bool Pop3EndSessionNoQuit(void);
	// POP3ENDSESSIONNOQUIT_END

	// MAILMAN_INSERT_POINT

	// END PUBLIC INTERFACE


    private:
	// Internal implementation.
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.
	CkMailProgress *m_callback;

	// Don't allow assignment or copying these objects.
	CkMailMan(const CkMailMan &) { } 
	CkMailMan &operator=(const CkMailMan &) { return *this; }
	CkMailMan(void *impl) : m_callback(0),m_impl(impl) { }

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const { return m_impl; } 

};
#pragma pack (pop)


#endif
