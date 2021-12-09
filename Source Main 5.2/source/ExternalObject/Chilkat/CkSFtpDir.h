// CkSFtpDir.h: interface for the CkSFtpDir class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CkSFtpDir_H
#define _CkSFtpDir_H

#pragma once

#include "CkString.h"
#include "CkObject.h"

class CkSFtpFile;

#pragma pack (push, 8) 

// CLASS: CkSFtpDir
class CkSFtpDir  : public CkObject
{
    private:

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkSFtpDir(const CkSFtpDir &);
	CkSFtpDir &operator=(const CkSFtpDir &);

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const;

	CkSFtpDir(void *impl);

	CkSFtpDir();
	virtual ~CkSFtpDir();

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
		// NUMFILESANDDIRS_BEGIN
	int get_NumFilesAndDirs(void);
	// NUMFILESANDDIRS_END
	// ORIGINALPATH_BEGIN
	void get_OriginalPath(CkString &str);
	const char *originalPath(void);
	// ORIGINALPATH_END
	// GETFILEOBJECT_BEGIN
	CkSFtpFile *GetFileObject(int index);
	// GETFILEOBJECT_END
	// GETFILENAME_BEGIN
	bool GetFilename(int index, CkString &outStr);
	const char *getFilename(int index);
	// GETFILENAME_END

	// SFTPDIR_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)


#endif


