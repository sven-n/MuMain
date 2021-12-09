// CkFileList.h: interface for the CkFileList class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKFILELIST_H
#define _CKFILELIST_H

#pragma once


class CkByteData;
#include "CkString.h"
#include "CkObject.h"

#pragma warning( disable : 4068 )
#pragma unmanaged

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#pragma pack (push, 8) 

// CLASS: CkFileList
class CkFileList  : public CkObject
{
    public:
	CkFileList();
	virtual ~CkFileList();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	void get_AppendFromDir(CkString &str) const;
	void put_AppendFromDir(const char *dir);

	int AppendFilesEx(const char *filenamePattern, 
	    bool recurse, bool saveExtraPath, bool archiveOnly, 
	    bool includeHidden, bool includeSystem);

	int AppendFiles(const char *filenamePattern, bool recurse);

	// Append a single file (relative to AppendFromDir)
	bool AppendFile(const char *filePath);

	// Add an arbitrary directory entry that may not necessarily be on disk.
	bool AppendDirEntry(const char *dirPath);

	int get_Count(void) const;

	bool GetArchivePath(int idx, CkString &str);
	bool SetArchivePath(int idx, const char *filePath);

	bool GetDiskPath(int idx, CkString &str);
	bool IsOnDisk(int idx);
	bool IsDirectory(int idx);

	bool GetData(int idx, CkByteData &byteData);

	// Replace findStr with replaceStr in all the ArchivePaths.
	void StringReplace(const char *findStr, const char *replaceStr);

	CkString m_resultString;
        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);

	const char *getArchivePath(int index);
	const char *getDiskPath(int index);
	const char *appendFromDir(int index);

	// FILELIST_INSERT_POINT

	// END PUBLIC INTERFACE

    // For internal use only.
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkFileList(const CkFileList &) { } 
	CkFileList &operator=(const CkFileList &) { return *this; }
	CkFileList(void *impl);

    public:
	void *getImpl(void) const { return m_impl; } 


};

#pragma pack (pop)

#endif


