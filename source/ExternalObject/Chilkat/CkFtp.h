// CkFtp.h: interface for the CkFtp class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKFTP_H
#define _CKFTP_H

#pragma once

class CkByteData;
class CkCert;
#include "CkString.h"
#include "CkObject.h"

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#pragma pack (push, 8) 

// CLASS: CkFtp
class CkFtp  : public CkObject
{
    public:
	CkFtp();
	virtual ~CkFtp();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);


	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	bool get_IsConnected(void);

	// Return the number of files deleted.
	long DeleteMatching(const char *remotePattern);

	// Return the number of files retrieved.
	long MGetFiles(const char *remotePattern, const char *localDir);

	// Return the number of files uploaded.
	long MPutFiles(const char *pattern);

	void get_Version(CkString &str);
	bool RenameRemoteFile(const char *existingFilename, const char *newFilename);
	bool DeleteRemoteFile(const char *filename);
	bool RemoveRemoteDir(const char *dir);
	bool CreateRemoteDir(const char *dir);
	bool GetCurrentDirListing(const char *pattern, CkString &strListing);
	bool PutFileFromBinaryData(const char *remoteFilename, CkByteData &binaryData);
	bool PutFileFromTextData(const char *remoteFilename, const char *textData);
	bool Disconnect(void);
	bool Connect(void);
	bool GetRemoteFileBinaryData(const char *remoteFilename, CkByteData &data);
	bool GetRemoteFileTextData(const char *remoteFilename, CkString &str);
	bool GetCurrentRemoteDir(CkString &str);
	bool ChangeRemoteDir(const char *relativeDirPath);
	bool GetFile(const char *remoteFilename,  const char *localFilename);
	bool PutFile(const char *localFilename,  const char *remoteFilename);
	void get_ListPattern(CkString &strPattern);
	void put_ListPattern(const char *pattern);
	void get_Password(CkString &str);
	void put_Password(const char *newVal);
	void get_Username(CkString &str);
	void put_Username(const char *newVal);
	long get_Port(void);
	void put_Port(long newVal);
	void get_Hostname(CkString &str);
	void put_Hostname(const char *newVal);
	bool get_Passive(void);
	void put_Passive(bool newVal);
	long GetSize(long index);
	bool GetFilename(long index, CkString &str);
	bool GetIsDirectory(long index);
	bool GetCreateTime(long index, FILETIME &fTime);
	bool GetLastAccessTime(long index, FILETIME &fTime);
	bool GetLastModifiedTime(long index, FILETIME &fTime);
	long get_NumFilesAndDirs(void);
	void get_Proxy(CkString &str);
	void put_Proxy(const char *newVal);
	bool get_UseIEProxy(void);
	void put_UseIEProxy(bool newVal);

	// END PUBLIC INTERFACE

    // For internal use only.
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkFtp(const CkFtp &) { } 
	CkFtp &operator=(const CkFtp &) { return *this; }

    public:
	void *getImpl(void) const { return m_impl; } 

	CkFtp(void *impl) : m_impl(impl) { }


};

#pragma pack (pop)


#endif


