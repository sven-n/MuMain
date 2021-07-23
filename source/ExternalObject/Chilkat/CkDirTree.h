// CkDirTree.h: interface for the CkDirTree class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CkDirTree_H
#define _CkDirTree_H

#pragma once

#include "CkString.h"
#include "CkObject.h"

#ifndef WIN32
#include "int64.h"
#endif

#pragma pack (push, 8) 

// CLASS: CkDirTree
class CkDirTree  : public CkObject
{
    private:

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkDirTree(const CkDirTree &);
	CkDirTree &operator=(const CkDirTree &);

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const;

	CkDirTree(void *impl);

	CkDirTree();
	virtual ~CkDirTree();

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
	

	// BASEDIR_BEGIN
	void get_BaseDir(CkString &str);
	const char *baseDir(void);
	void put_BaseDir(const char *newVal);
	// BASEDIR_END
	// RECURSE_BEGIN
	bool get_Recurse(void);
	void put_Recurse(bool newVal);
	// RECURSE_END
	// DONEITERATING_BEGIN
	bool get_DoneIterating(void);
	// DONEITERATING_END
	// BEGINITERATE_BEGIN
	bool BeginIterate(void);
	// BEGINITERATE_END
	// ADVANCEPOSITION_BEGIN
	bool AdvancePosition(void);
	// ADVANCEPOSITION_END
	// ISDIRECTORY_BEGIN
	bool get_IsDirectory(void);
	// ISDIRECTORY_END
	// RELATIVEPATH_BEGIN
	void get_RelativePath(CkString &str);
	const char *relativePath(void);
	// RELATIVEPATH_END
	// FULLPATH_BEGIN
	void get_FullPath(CkString &str);
	const char *fullPath(void);
	// FULLPATH_END
	// FULLUNCPATH_BEGIN
	void get_FullUncPath(CkString &str);
	const char *fullUncPath(void);
	// FULLUNCPATH_END
	// FILESIZE32_BEGIN
	int get_FileSize32(void);
	// FILESIZE32_END
	// FILESIZE64_BEGIN
	__int64 get_FileSize64(void);
	// FILESIZE64_END

	// DIRTREE_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)


#endif


