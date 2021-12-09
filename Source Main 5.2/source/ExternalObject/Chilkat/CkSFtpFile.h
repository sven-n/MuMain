// CkSFtpFile.h: interface for the CkSFtpFile class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CkSFtpFile_H
#define _CkSFtpFile_H

#pragma once

#ifndef WIN32
#include "int64.h"
#endif

#include "CkString.h"
#include "CkObject.h"

#if !defined(WIN32) && !defined(WINCE)
#include "SystemTime.h"
#endif

#pragma pack (push, 8) 

// CLASS: CkSFtpFile
class CkSFtpFile  : public CkObject
{
    private:

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkSFtpFile(const CkSFtpFile &);
	CkSFtpFile &operator=(const CkSFtpFile &);

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const;

	CkSFtpFile(void *impl);

	CkSFtpFile();
	virtual ~CkSFtpFile();

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
		// FILENAME_BEGIN
	void get_Filename(CkString &str);
	const char *filename(void);
	// FILENAME_END
	// OWNER_BEGIN
	void get_Owner(CkString &str);
	const char *owner(void);
	// OWNER_END
	// GROUP_BEGIN
	void get_Group(CkString &str);
	const char *group(void);
	// GROUP_END
	// FILETYPE_BEGIN
	void get_FileType(CkString &str);
	const char *fileType(void);
	// FILETYPE_END
	// ISREGULAR_BEGIN
	bool get_IsRegular(void);
	// ISREGULAR_END
	// ISDIRECTORY_BEGIN
	bool get_IsDirectory(void);
	// ISDIRECTORY_END
	// ISSYMLINK_BEGIN
	bool get_IsSymLink(void);
	// ISSYMLINK_END
	// ISREADONLY_BEGIN
	bool get_IsReadOnly(void);
	// ISREADONLY_END
	// ISSYSTEM_BEGIN
	bool get_IsSystem(void);
	// ISSYSTEM_END
	// ISHIDDEN_BEGIN
	bool get_IsHidden(void);
	// ISHIDDEN_END
	// ISCASEINSENSITIVE_BEGIN
	bool get_IsCaseInsensitive(void);
	// ISCASEINSENSITIVE_END
	// ISARCHIVE_BEGIN
	bool get_IsArchive(void);
	// ISARCHIVE_END
	// ISENCRYPTED_BEGIN
	bool get_IsEncrypted(void);
	// ISENCRYPTED_END
	// ISCOMPRESSED_BEGIN
	bool get_IsCompressed(void);
	// ISCOMPRESSED_END
	// ISSPARSE_BEGIN
	bool get_IsSparse(void);
	// ISSPARSE_END
	// ISAPPENDONLY_BEGIN
	bool get_IsAppendOnly(void);
	// ISAPPENDONLY_END
	// ISIMMUTABLE_BEGIN
	bool get_IsImmutable(void);
	// ISIMMUTABLE_END
	// ISSYNC_BEGIN
	bool get_IsSync(void);
	// ISSYNC_END
	// PERMISSIONS_BEGIN
	int get_Permissions(void);
	// PERMISSIONS_END
	// SIZE32_BEGIN
	int get_Size32(void);
	// SIZE32_END
	// SIZE64_BEGIN
	__int64 get_Size64(void);
	// SIZE64_END

	// SYSTEMTIME is defined in <windows.h>
	// For more information, Google "SYSTEMTIME"

	void get_LastModifiedTime(SYSTEMTIME &sysTime);
	void get_LastAccessTime(SYSTEMTIME &sysTime);
	void get_CreateTime(SYSTEMTIME &sysTime);

	// SIZESTR_BEGIN
	void get_SizeStr(CkString &str);
	const char *sizeStr(void);
	// SIZESTR_END
	// UID_BEGIN
	int get_Uid(void);
	// UID_END
	// GID_BEGIN
	int get_Gid(void);
	// GID_END

	// SFTPFILE_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)


#endif


