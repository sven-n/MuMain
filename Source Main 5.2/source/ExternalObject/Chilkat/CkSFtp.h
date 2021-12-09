// CkSFtp.h: interface for the CkSFtp class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CkSFtp_H
#define _CkSFtp_H

#pragma once

#ifndef WIN32
#include "int64.h"
#endif

#include "CkString.h"
class CkByteData;
class CkSFtpProgress;
class CkSshKey;
class CkSFtpDir;
#include "CkObject.h"

#if !defined(WIN32) && !defined(WINCE)
#include "SystemTime.h"
#endif

#pragma pack (push, 8) 

// CLASS: CkSFtp
class CkSFtp  : public CkObject
{
    private:
	CkSFtpProgress *m_callback;

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkSFtp(const CkSFtp &);
	CkSFtp &operator=(const CkSFtp &);

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const;

	CkSFtp(void *impl);

	CkSFtp();
	virtual ~CkSFtp();

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
	
	CkSFtpProgress *get_EventCallbackObject(void) const;
	void put_EventCallbackObject(CkSFtpProgress *progress);

	// UNLOCKCOMPONENT_BEGIN
	bool UnlockComponent(const char *unlockCode);
	// UNLOCKCOMPONENT_END
	// CONNECTTIMEOUTMS_BEGIN
	int get_ConnectTimeoutMs(void);
	void put_ConnectTimeoutMs(int newVal);
	// CONNECTTIMEOUTMS_END
	// DISCONNECTCODE_BEGIN
	int get_DisconnectCode(void);
	// DISCONNECTCODE_END
	// INITIALIZEFAILCODE_BEGIN
	int get_InitializeFailCode(void);
	// INITIALIZEFAILCODE_END
	// MAXPACKETSIZE_BEGIN
	int get_MaxPacketSize(void);
	void put_MaxPacketSize(int newVal);
	// MAXPACKETSIZE_END
	// IDLETIMEOUTMS_BEGIN
	int get_IdleTimeoutMs(void);
	void put_IdleTimeoutMs(int newVal);
	// IDLETIMEOUTMS_END
	// INITIALIZEFAILREASON_BEGIN
	void get_InitializeFailReason(CkString &str);
	const char *initializeFailReason(void);
	// INITIALIZEFAILREASON_END
	// DISCONNECTREASON_BEGIN
	void get_DisconnectReason(CkString &str);
	const char *disconnectReason(void);
	// DISCONNECTREASON_END
	// VERSION_BEGIN
	void get_Version(CkString &str);
	const char *version(void);
	// VERSION_END
	// ISCONNECTED_BEGIN
	bool get_IsConnected(void);
	// ISCONNECTED_END
	// KEEPSESSIONLOG_BEGIN
	bool get_KeepSessionLog(void);
	void put_KeepSessionLog(bool newVal);
	// KEEPSESSIONLOG_END
	// SESSIONLOG_BEGIN
	void get_SessionLog(CkString &str);
	const char *sessionLog(void);
	// SESSIONLOG_END
	// DISCONNECT_BEGIN
	void Disconnect(void);
	// DISCONNECT_END
	// CONNECT_BEGIN
	bool Connect(const char *hostname, int port);
	// CONNECT_END
	// AUTHENTICATEPK_BEGIN
	bool AuthenticatePk(const char *username, CkSshKey &privateKey);
	// AUTHENTICATEPK_END
	// AUTHENTICATEPW_BEGIN
	bool AuthenticatePw(const char *login, const char *password);
	// AUTHENTICATEPW_END
	// INITIALIZESFTP_BEGIN
	bool InitializeSftp(void);
	// INITIALIZESFTP_END
	// OPENFILE_BEGIN
	bool OpenFile(const char *filename, const char *access, const char *createDisp, CkString &outStr);
	const char *openFile(const char *filename, const char *access, const char *createDisp);
	// OPENFILE_END
	// OPENDIR_BEGIN
	bool OpenDir(const char *path, CkString &outStr);
	const char *openDir(const char *path);
	// OPENDIR_END
	// CLOSEHANDLE_BEGIN
	bool CloseHandle(const char *handle);
	// CLOSEHANDLE_END
	// GETFILESIZE_BEGIN
	unsigned long GetFileSize32(const char *filenameOrHandle, bool bFollowLinks, bool bIsHandle);
	// GETFILESIZE_END
	// GETFILESIZE64_BEGIN
	__int64 GetFileSize64(const char *filenameOrHandle, bool bFollowLinks, bool bIsHandle);
	// GETFILESIZE64_END
	// GETFILESIZESTR_BEGIN
	bool GetFileSizeStr(const char *filenameOrHandle, bool bFollowLinks, bool bIsHandle, CkString &outStr);
	const char *getFileSizeStr(const char *filenameOrHandle, bool bFollowLinks, bool bIsHandle);
	// GETFILESIZESTR_END

	// SYSTEMTIME is defined in <windows.h>
	// For more information, Google "SYSTEMTIME"
	bool GetFileLastAccess(const char *filenameOrHandle, bool bFollowLinks, bool bIsHandle, SYSTEMTIME &sysTime);
	bool GetFileLastModified(const char *filenameOrHandle, bool bFollowLinks, bool bIsHandle, SYSTEMTIME &sysTime);
	bool GetFileCreateTime(const char *filenameOrHandle, bool bFollowLinks, bool bIsHandle, SYSTEMTIME &sysTime);

	// GETFILEOWNER_BEGIN
	bool GetFileOwner(const char *filenameOrHandle, bool bFollowLinks, bool bIsHandle, CkString &outStr);
	const char *getFileOwner(const char *filenameOrHandle, bool bFollowLinks, bool bIsHandle);
	// GETFILEOWNER_END
	// GETFILEGROUP_BEGIN
	bool GetFileGroup(const char *filenameOrHandle, bool bFollowLinks, bool bIsHandle, CkString &outStr);
	const char *getFileGroup(const char *filenameOrHandle, bool bFollowLinks, bool bIsHandle);
	// GETFILEGROUP_END
	// GETFILEPERMISSIONS_BEGIN
	int GetFilePermissions(const char *filenameOrHandle, bool bFollowLinks, bool bIsHandle);
	// GETFILEPERMISSIONS_END
	// ADD64_BEGIN
	void Add64(const char *n1, const char *n2, CkString &outStr);
	const char *add64(const char *n1, const char *n2);
	// ADD64_END
	// READFILEBYTES32_BEGIN
	bool ReadFileBytes32(const char *handle, int offset, int numBytes, CkByteData &outBytes);
	// READFILEBYTES32_END
	// READFILEBYTES64_BEGIN
	bool ReadFileBytes64(const char *handle, __int64 offset64, int numBytes, CkByteData &outBytes);
	// READFILEBYTES64_END
	// READFILEBYTES64S_BEGIN
	bool ReadFileBytes64s(const char *handle, const char *offset64, int numBytes, CkByteData &outBytes);
	// READFILEBYTES64S_END
	// READFILETEXT64S_BEGIN
	bool ReadFileText64s(const char *handle, const char *offset64, int numBytes, const char *charset, CkString &outStr);
	const char *readFileText64s(const char *handle, const char *offset64, int numBytes, const char *charset);
	// READFILETEXT64S_END
	// READFILETEXT64_BEGIN
	bool ReadFileText64(const char *handle, __int64 offset64, int numBytes, const char *charset, CkString &outStr);
	const char *readFileText64(const char *handle, __int64 offset64, int numBytes, const char *charset);
	// READFILETEXT64_END
	// READFILETEXT32_BEGIN
	bool ReadFileText32(const char *handle, int offset32, int numBytes, const char *charset, CkString &outStr);
	const char *readFileText32(const char *handle, int offset32, int numBytes, const char *charset);
	// READFILETEXT32_END
	// DOWNLOADFILE_BEGIN
	bool DownloadFile(const char *handle, const char *toFilename);
	// DOWNLOADFILE_END
	// EOF_BEGIN
	bool Eof(const char *handle);
	// EOF_END
	// READFILEBYTES_BEGIN
	bool ReadFileBytes(const char *handle, int numBytes, CkByteData &outBytes);
	// READFILEBYTES_END
	// READFILETEXT_BEGIN
	bool ReadFileText(const char *handle, int numBytes, const char *charset, CkString &outStr);
	const char *readFileText(const char *handle, int numBytes, const char *charset);
	// READFILETEXT_END
	// LASTREADFAILED_BEGIN
	bool LastReadFailed(const char *handle);
	// LASTREADFAILED_END
	// LASTREADNUMBYTES_BEGIN
	int LastReadNumBytes(const char *handle);
	// LASTREADNUMBYTES_END
	// WRITEFILEBYTES_BEGIN
	bool WriteFileBytes(const char *handle, CkByteData &data);
	// WRITEFILEBYTES_END
	// WRITEFILEBYTES32_BEGIN
	bool WriteFileBytes32(const char *handle, int offset, CkByteData &data);
	// WRITEFILEBYTES32_END
	// WRITEFILEBYTES64_BEGIN
	bool WriteFileBytes64(const char *handle, __int64 offset64, CkByteData &data);
	// WRITEFILEBYTES64_END
	// WRITEFILEBYTES64S_BEGIN
	bool WriteFileBytes64s(const char *handle, const char *offset64, CkByteData &data);
	// WRITEFILEBYTES64S_END
	// WRITEFILETEXT_BEGIN
	bool WriteFileText(const char *handle, const char *charset, const char *textData);
	// WRITEFILETEXT_END
	// WRITEFILETEXT32_BEGIN
	bool WriteFileText32(const char *handle, int offset32, const char *charset, const char *textData);
	// WRITEFILETEXT32_END
	// WRITEFILETEXT64_BEGIN
	bool WriteFileText64(const char *handle, __int64 offset64, const char *charset, const char *textData);
	// WRITEFILETEXT64_END
	// WRITEFILETEXT64S_BEGIN
	bool WriteFileText64s(const char *handle, const char *offset64, const char *charset, const char *textData);
	// WRITEFILETEXT64S_END
	// UPLOADFILE_BEGIN
	bool UploadFile(const char *handle, const char *fromFilename);
	// UPLOADFILE_END
	// REALPATH_BEGIN
	bool RealPath(const char *originalPath, const char *composePath, CkString &outStr);
	const char *realPath(const char *originalPath, const char *composePath);
	// REALPATH_END
	// READDIR_BEGIN
	CkSFtpDir *ReadDir(const char *handle);
	// READDIR_END
	// REMOVEFILE_BEGIN
	bool RemoveFile(const char *filename);
	// REMOVEFILE_END
	// REMOVEDIR_BEGIN
	bool RemoveDir(const char *path);
	// REMOVEDIR_END
	// RENAMEFILEORDIR_BEGIN
	bool RenameFileOrDir(const char *oldPath, const char *newPath);
	// RENAMEFILEORDIR_END
	// CREATEDIR_BEGIN
	bool CreateDir(const char *path);
	// CREATEDIR_END

	// SYSTEMTIME is defined in <windows.h>
	// For more information, Google "SYSTEMTIME"
	bool SetCreateTime(const char *pathOrHandle, bool bIsHandle, SYSTEMTIME &createTime);
	bool SetLastModifiedTime(const char *pathOrHandle, bool bIsHandle, SYSTEMTIME &lastModTime);
	bool SetLastAccessTime(const char *pathOrHandle, bool bIsHandle, SYSTEMTIME &lastAccessTime);

	bool SetOwnerAndGroup(const char *pathOrHandle, bool bIsHandle, const char *owner, const char *group);

	// SETPERMISSIONS_BEGIN
	bool SetPermissions(const char *pathOrHandle, bool bIsHandle, int perm);
	// SETPERMISSIONS_END
	// COPYFILEATTR_BEGIN
	bool CopyFileAttr(const char *localFilename, const char *remoteFilenameOrHandle, bool bIsHandle);
	// COPYFILEATTR_END
	// PROTOCOLVERSION_BEGIN
	int get_ProtocolVersion(void);
	// PROTOCOLVERSION_END
	// ENABLECACHE_BEGIN
	bool get_EnableCache(void);
	void put_EnableCache(bool newVal);
	// ENABLECACHE_END
	// CLEARCACHE_BEGIN
	void ClearCache(void);
	// CLEARCACHE_END
	// DOWNLOADFILEBYNAME_BEGIN
	bool DownloadFileByName(const char *remoteFilePath, const char *localFilePath);
	// DOWNLOADFILEBYNAME_END
	// HEARTBEATMS_BEGIN
	int get_HeartbeatMs(void);
	void put_HeartbeatMs(int newVal);
	// HEARTBEATMS_END
	// UPLOADFILEBYNAME_BEGIN
	bool UploadFileByName(const char *remoteFilePath, const char *localFilePath);
	// UPLOADFILEBYNAME_END
	// RESUMEUPLOADFILEBYNAME_BEGIN
	bool ResumeUploadFileByName(const char *remoteFilePath, const char *localFilePath);
	// RESUMEUPLOADFILEBYNAME_END
	// RESUMEDOWNLOADFILEBYNAME_BEGIN
	bool ResumeDownloadFileByName(const char *remoteFilePath, const char *localFilePath);
	// RESUMEDOWNLOADFILEBYNAME_END
	// VERBOSELOGGING_BEGIN
	bool get_VerboseLogging(void);
	void put_VerboseLogging(bool newVal);
	// VERBOSELOGGING_END
	// FILENAMECHARSET_BEGIN
	void get_FilenameCharset(CkString &str);
	const char *filenameCharset(void);
	void put_FilenameCharset(const char *newVal);
	// FILENAMECHARSET_END
	// CLEARSESSIONLOG_BEGIN
	void ClearSessionLog(void);
	// CLEARSESSIONLOG_END
	// FORCEV3_BEGIN
	bool get_ForceV3(void);
	void put_ForceV3(bool newVal);
	// FORCEV3_END
	// UTCMODE_BEGIN
	bool get_UtcMode(void);
	void put_UtcMode(bool newVal);
	// UTCMODE_END
	// PRESERVEDATE_BEGIN
	bool get_PreserveDate(void);
	void put_PreserveDate(bool newVal);
	// PRESERVEDATE_END
	// CLIENTIDENTIFIER_BEGIN
	void get_ClientIdentifier(CkString &str);
	const char *clientIdentifier(void);
	void put_ClientIdentifier(const char *newVal);
	// CLIENTIDENTIFIER_END
	// HOSTKEYFINGERPRINT_BEGIN
	void get_HostKeyFingerprint(CkString &str);
	const char *hostKeyFingerprint(void);
	// HOSTKEYFINGERPRINT_END
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
	// HTTPPROXYPORT_BEGIN
	int get_HttpProxyPort(void);
	void put_HttpProxyPort(int newVal);
	// HTTPPROXYPORT_END
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
	// TCPNODELAY_BEGIN
	bool get_TcpNoDelay(void);
	void put_TcpNoDelay(bool newVal);
	// TCPNODELAY_END
	// ACCUMULATEBUFFER_BEGIN
	void get_AccumulateBuffer(CkByteData &data);
	// ACCUMULATEBUFFER_END
	// CLEARACCUMULATEBUFFER_BEGIN
	void ClearAccumulateBuffer(void);
	// CLEARACCUMULATEBUFFER_END
	// ACCUMULATEBYTES_BEGIN
	int AccumulateBytes(const char *handle, int maxBytes);
	// ACCUMULATEBYTES_END
	// PASSWORDCHANGEREQUESTED_BEGIN
	bool get_PasswordChangeRequested(void);
	// PASSWORDCHANGEREQUESTED_END
	// AUTHENTICATEPK_BEGIN
	bool AuthenticatePk(const char *username, const char *password, CkSshKey &privateKey);
	// AUTHENTICATEPK_END
	// AUTHENTICATEPWPK_BEGIN
	bool AuthenticatePwPk(const char *username, const char *password, CkSshKey &privateKey);
	// AUTHENTICATEPWPK_END

	// SFTP_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)


#endif


