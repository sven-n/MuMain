// CkFtp2.h: interface for the CkFtp2 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKFTP2_H
#define _CKFTP2_H

#pragma once


class CkByteData;


class CkCert;


#ifndef WIN32
#include "int64.h"
#endif

#include "CkString.h"
#include "CkObject.h"

#if !defined(WIN32) && !defined(WINCE)
#include "SystemTime.h"
#include "FileTime.h"
#endif

#pragma warning( disable : 4068 )
#pragma unmanaged

class CkFtpProgress;
/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#pragma pack (push, 8) 

// CLASS: CkFtp2
class CkFtp2  : public CkObject
{
    public:
	CkFtp2();
	virtual ~CkFtp2();

	bool PutTree(const char *localDir, CkFtpProgress &progress);
	bool AppendFile2(const char *localFilename,  const char *remoteFilename, CkFtpProgress &progress);
	bool AppendFileFromBinaryData2(const char *remoteFilename, const CkByteData &binaryData, CkFtpProgress &progress);
	bool AppendFileFromTextData2(const char *remoteFilename, const char *textData, CkFtpProgress &progress);
	// Return the number of files retrieved.
	long MGetFiles2(const char *remotePattern, const char *localDir, CkFtpProgress &progress);
	// Return the number of files uploaded.
	long MPutFiles2(const char *pattern, CkFtpProgress &progress);

	bool GetFile2(const char *remoteFilename,  const char *localFilename, CkFtpProgress &progress);
	bool PutFile2(const char *localFilename,  const char *remoteFilename, CkFtpProgress &progress);
	bool PutFileFromBinaryData2(const char *remoteFilename, const CkByteData &binaryData, CkFtpProgress &progress);
	bool PutFileFromTextData2(const char *remoteFilename, const char *textData, CkFtpProgress &progress);

	bool GetRemoteFileBinaryData2(const char *remoteFilename, CkByteData &data, CkFtpProgress &progress);
	bool GetRemoteFileTextData2(const char *remoteFilename, CkString &str, CkFtpProgress &progress);


	// BEGIN PUBLIC INTERFACE
	void SleepMs(int millisec) const;

	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	bool UnlockComponent(const char *code);
	bool IsUnlocked(void) const;

	long get_BandwidthThrottleUp(void);
	void put_BandwidthThrottleUp(long bytesPerSec);
	long get_BandwidthThrottleDown(void);
	void put_BandwidthThrottleDown(long bytesPerSec);

	CkFtpProgress *get_EventCallbackObject(void) const;
	void put_EventCallbackObject(CkFtpProgress *progress);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	bool get_IsConnected(void);

	bool get_PartialTransfer(void);
	bool get_LoginVerified(void);
	bool get_ConnectVerified(void);

	// Return the number of files deleted.
	long DeleteMatching(const char *remotePattern);

	// Return the number of files retrieved.
	long MGetFiles(const char *remotePattern, const char *localDir);
	// Return the number of files uploaded.
	long MPutFiles(const char *pattern);

	bool PutTree(const char *localDir);

	bool GetFile(const char *remoteFilename,  const char *localFilename);
	bool PutFile(const char *localFilename,  const char *remoteFilename);

	bool PutFileFromBinaryData(const char *remoteFilename, const CkByteData &binaryData);
	bool PutFileFromTextData(const char *remoteFilename, const char *textData);

	bool GetRemoteFileBinaryData(const char *remoteFilename, CkByteData &data);
	bool GetRemoteFileTextData(const char *remoteFilename, CkString &str);
	bool GetRemoteFileTextC(const char *remoteFilename, const char *charset, CkString &str);

	bool AppendFile(const char *localFilename,  const char *remoteFilename);
	bool AppendFileFromBinaryData(const char *remoteFilename, const CkByteData &binaryData);
	bool AppendFileFromTextData(const char *remoteFilename, const char *textData);





	void get_Version(CkString &str);
	bool RenameRemoteFile(const char *existingFilename, const char *newFilename);
	bool DeleteRemoteFile(const char *filename);
	bool RemoveRemoteDir(const char *dir);
	bool CreateRemoteDir(const char *dir);

	bool Disconnect(void);
	bool Connect(void);

	bool GetCurrentRemoteDir(CkString &str);
	bool ChangeRemoteDir(const char *relativeDirPath);

	bool ClearControlChannel(void);

	void get_DirListingCharset(CkString &str);
	void put_DirListingCharset(const char *newVal);

	void get_Account(CkString &str);
	void put_Account(const char *newVal);
	const char *account(void);

	void get_ListPattern(CkString &strPattern);
	void put_ListPattern(const char *pattern);
	void get_Password(CkString &str);
	void put_Password(const char *newVal);
	void get_Username(CkString &str);
	void put_Username(const char *newVal);
	unsigned long get_AllocateSize(void);
	void put_AllocateSize(unsigned long newVal);
	long get_Port(void);
	void put_Port(long newVal);
	long get_ActivePortRangeStart(void);
	void put_ActivePortRangeStart(long newVal);
	long get_ActivePortRangeEnd(void);
	void put_ActivePortRangeEnd(long newVal);
	long get_ProxyPort(void);
	void put_ProxyPort(long newVal);
	void get_Hostname(CkString &str);
	void put_Hostname(const char *newVal);
	void get_SessionLog(CkString &str);
	bool get_Passive(void);
	void put_Passive(bool newVal);
	bool get_Ssl(void);
	void put_Ssl(bool newVal);
	bool get_AuthTls(void);
	void put_AuthTls(bool newVal);
	bool get_KeepSessionLog(void);
	void put_KeepSessionLog(bool newVal);
	bool get_PassiveUseHostAddr(void);
	void put_PassiveUseHostAddr(bool newVal);

	bool get_HasModeZ(void);

	bool get_RestartNext(void);
	void put_RestartNext(bool newVal);

	bool get_AutoXcrc(void);
	void put_AutoXcrc(bool newVal);
	bool get_AutoFeat(void);
	void put_AutoFeat(bool newVal);
	bool get_AutoSyst(void);
	void put_AutoSyst(bool newVal);
	bool Feat(CkString &sOut);
	const char *feat(void);
	

	bool GetSizeStr(long index, CkString &str);
	long GetSize(long index);
	bool GetFilename(long index, CkString &str);
	bool GetIsDirectory(long index);
	bool GetIsSymbolicLink(long index);

	void ClearDirCache(void);

	long GetSizeByName(const char *filname);

	bool GetCreateTime(long index, FILETIME &fTime);
	bool GetLastAccessTime(long index, FILETIME &fTime);
	bool GetLastModifiedTime(long index, FILETIME &fTime);

	bool GetCreateTime(long index, SYSTEMTIME &sysTime);
	bool GetLastAccessTime(long index, SYSTEMTIME &sysTime);
	bool GetLastModifiedTime(long index, SYSTEMTIME &sysTime);

	long get_NumFilesAndDirs(void);

	void get_ProxyHostname(CkString &str);
	void put_ProxyHostname(const char *newVal);
	void get_ProxyUsername(CkString &str);
	void put_ProxyUsername(const char *newVal);
	void get_ProxyPassword(CkString &str);
	void put_ProxyPassword(const char *newVal);
	long get_ProxyMethod(void);
	void put_ProxyMethod(long newVal);

	void ClearSessionLog(void);

	bool SetTypeBinary(void);
	bool SetTypeAscii(void);
	bool SetModeZ(void);

	bool Quote(const char *cmd);
	bool Site(const char *params);
	bool Syst(CkString &sOut);
	bool Stat(CkString &sOut);

	// 0 = success
	// 1-299 = underlying reasons
	// Normal sockets:
	//  1 = empty hostname
	//  2 = DNS lookup failed
	//  3 = DNS timeout
	//  4 = Aborted by application.
	//  5 = Internal failure.
	//  6 = Connect Timed Out
	//  7 = Connect Rejected (or failed for some other reason)
	// SSL:
	//  100 = Internal schannel error
	//  101 = Failed to create credentials
	//  102 = Failed to send initial message to proxy.
	//  103 = Handshake failed.
	//  104 = Failed to obtain remote certificate.
	// Protocol/Component:
	// 300 = asynch op in progress
	// 301 = login failure.
	long get_ConnectFailReason(void);

	// Timeouts are in seconds
	long get_ReadTimeout(void);
	void put_ReadTimeout(long numSeconds);

	// Timeouts are in seconds
	long get_ConnectTimeout(void);
	void put_ConnectTimeout(long numSeconds);

	// These times are in milliseconds.
	long get_HeartbeatMs(void);
	void put_HeartbeatMs(long millisec);

	long get_IdleTimeoutMs(void);
	void put_IdleTimeoutMs(long millisec);

	// CRLF mode for text (ASCII-mode) file downloads
	// 0 = no conversion, 1 = CR+LF, 2 = LF Only, 3 = CR Only
	long get_CrlfMode(void);
	void put_CrlfMode(long newVal);

	// These values are in bytes/second
	long get_UploadRate(void);
	long get_DownloadRate(void);


	// Progress Monitoring size (in bytes) for FTP servers that do not
	// respond with size information for GETs.
	long get_ProgressMonSize(void);
	void put_ProgressMonSize(long newVal);

	__int64 get_ProgressMonSize64(void);
	void put_ProgressMonSize64(__int64 newVal);

	bool GetXmlDirListing(const char *pattern, CkString &strXmlListing);
	bool GetTextDirListing(const char *pattern, CkString &strRawListing);

	bool DownloadTree(const char *localRoot);
	bool DeleteTree(void);

	// Fetch the remote directory (rooted at the current remote directory) tree as XML
	bool DirTreeXml(CkString &strXml);
	const char *dirTreeXml(void);

	// Copy local files to the FTP server to synchronize a directory tree.
	// mode: 0 = all files, 1 = only missing files, 2 = missing and newer files, 3 = only newer files (already existing)
	bool SyncRemoteTree(const char *localRoot, int mode);

	// Copy remote files from an FTP server to synchronize a directory tree.
	// mode: 0 = all files, 1 = only missing files, 2 = missing and newer files, 3 = only newer files (already existing)
	bool SyncLocalTree(const char *localRoot, int mode);


	// Asynchronous FTP
	bool AsyncGetFileStart(const char *remoteFilename,  const char *localFilename);
	bool AsyncPutFileStart(const char *localFilename,  const char *remoteFilename);
	bool AsyncAppendFileStart(const char *localFilename,  const char *remoteFilename);

	void AsyncAbort(void);	// Abort the async operation.

	bool get_AsyncFinished(void);	// True if finished, false if in progress.
	void get_AsyncLog(CkString &strLog);
	bool get_AsyncSuccess(void);	// True if successful, false if aborted or failed.

	// Can be called while the asynchronous upload or download is in progress to
	// get the bytes sent/received so far..
	unsigned long get_AsyncBytesReceived(void);
	unsigned long get_AsyncBytesSent(void);

	__int64 get_AsyncBytesReceived64(void);
	__int64 get_AsyncBytesSent64(void);

	bool CreatePlan(const char *localDir, CkString &str);
	//bool PutTree(const char *localDir);
	bool PutPlan(const char *planUtf8, const char *planLogFilename);

	bool get_SslServerCertVerified(void);

	CkCert *GetSslServerCert(void);
	bool SetSslClientCert(CkCert &cert);
	bool SetSslClientCertPfx(const char *pfxFilename, const char *pfxPassword, const char *certSubjectCN);

	bool get_RequireSslCertVerify(void);
	void put_RequireSslCertVerify(bool newVal);
	void SetSslCertRequirement(const char *name, const char *value);

	bool get_AuthSsl(void);
	void put_AuthSsl(bool newVal);

	int DetermineProxyMethod(void);
	bool DetermineSettings(CkString &xmlReport);
	const char *determineSettings(void);

	bool SendCommand(const char *cmd, CkString &reply);
	const char *sendCommand(const char *cmd);

        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);

	const char *getFilename(long index);
	const char *listPattern(void);
	const char *password(void);
	const char *username(void);
	const char *hostname(void);
	const char *sessionLog(void);
	const char *getXmlDirListing(const char *pattern);
	const char *getTextDirListing(const char *pattern);
	const char *getRemoteFileTextData(const char *remoteFilename);
	const char *getRemoteFileTextC(const char *remoteFilename, const char *charset);
	const char *createPlan(const char *localDir);

	const char *version(void);
	const char *getCurrentRemoteDir(void);
	const char *getSizeStr(long index);
	const char *proxyHostname(void);
	const char *proxyUsername(void);
	const char *proxyPassword(void);
	const char *syst(void);
	const char *ck_stat(void);
	const char *asyncLog(void);
	const char *dirListingCharset(void);
	// SYSTEMTIME is defined in <windows.h>
	// For more information, Google "SYSTEMTIME"

	bool GetCreateTimeByName(const char *filename, SYSTEMTIME &sysTime);
	bool GetLastAccessTimeByName(const char *filename, SYSTEMTIME &sysTime);
	bool GetLastModifiedTimeByName(const char *filename, SYSTEMTIME &sysTime);

	// NOOP_BEGIN
	bool Noop(void);
	// NOOP_END
	// SETOLDESTDATE_BEGIN
	void SetOldestDate(SYSTEMTIME &oldestDateTime);
	// SETOLDESTDATE_END
	// CONVERTTOTLS_BEGIN
	bool ConvertToTls(void);
	// CONVERTTOTLS_END
	// GETSIZESTRBYNAME_BEGIN
	bool GetSizeStrByName(const char *filename, CkString &outStr);
	const char *getSizeStrByName(const char *filename);
	// GETSIZESTRBYNAME_END
	// SETREMOTEFILEDATETIME_BEGIN
	// SYSTEMTIME is defined in <windows.h>
	// For more information, Google "SYSTEMTIME"
	bool SetRemoteFileDateTime(SYSTEMTIME &dt, const char *remoteFilename);
	// SETREMOTEFILEDATETIME_END
	// SENDBUFFERSIZE_BEGIN
	int get_SendBufferSize(void);
	void put_SendBufferSize(int newVal);
	// SENDBUFFERSIZE_END
	// USEEPSV_BEGIN
	bool get_UseEpsv(void);
	void put_UseEpsv(bool newVal);
	// USEEPSV_END
	// FORCEPORTIPADDRESS_BEGIN
	void get_ForcePortIpAddress(CkString &str);
	const char *forcePortIpAddress(void);
	void put_ForcePortIpAddress(const char *newVal);
	// FORCEPORTIPADDRESS_END
	// NLSTXML_BEGIN
	bool NlstXml(const char *pattern, CkString &outStr);
	const char *nlstXml(const char *pattern);
	// NLSTXML_END
	// SOCKSVERSION_BEGIN
	int get_SocksVersion(void);
	void put_SocksVersion(int newVal);
	// SOCKSVERSION_END
	// SOCKSPORT_BEGIN
	int get_SocksPort(void);
	void put_SocksPort(int newVal);
	// SOCKSPORT_END
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
	// SOCKSHOSTNAME_BEGIN
	void get_SocksHostname(CkString &str);
	const char *socksHostname(void);
	void put_SocksHostname(const char *newVal);
	// SOCKSHOSTNAME_END
	// GREETING_BEGIN
	void get_Greeting(CkString &str);
	const char *greeting(void);
	// GREETING_END
	// VERBOSELOGGING_BEGIN
	bool get_VerboseLogging(void);
	void put_VerboseLogging(bool newVal);
	// VERBOSELOGGING_END
	// SSLPROTOCOL_BEGIN
	void get_SslProtocol(CkString &str);
	const char *sslProtocol(void);
	void put_SslProtocol(const char *newVal);
	// SSLPROTOCOL_END
	// AUTOGETSIZEFORPROGRESS_BEGIN
	bool get_AutoGetSizeForProgress(void);
	void put_AutoGetSizeForProgress(bool newVal);
	// AUTOGETSIZEFORPROGRESS_END
	// SYNCPREVIEW_BEGIN
	void get_SyncPreview(CkString &str);
	const char *syncPreview(void);
	// SYNCPREVIEW_END
	// SYNCREMOTETREE2_BEGIN
	bool SyncRemoteTree2(const char *localRoot, int mode, bool bDescend, bool bPreviewOnly);
	// SYNCREMOTETREE2_END
	// AUTOFIX_BEGIN
	bool get_AutoFix(void);
	void put_AutoFix(bool newVal);
	// AUTOFIX_END
	// CLIENTIPADDRESS_BEGIN
	void get_ClientIpAddress(CkString &str);
	const char *clientIpAddress(void);
	void put_ClientIpAddress(const char *newVal);
	// CLIENTIPADDRESS_END

	// FTP2_INSERT_POINT

	// END PUBLIC INTERFACE

	void TestDirListingParse(const char *textFilename);


    // For internal use only.
    private:
	CkFtpProgress *m_callback;

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkFtp2(const CkFtp2 &) { } 
	CkFtp2 &operator=(const CkFtp2 &) { return *this; }
	CkFtp2(void *impl) : m_impl(impl), m_callback(0) { }

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const { return m_impl; } 

};
#pragma pack (pop)


#endif


