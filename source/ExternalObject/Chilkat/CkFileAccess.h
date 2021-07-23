// CkFileAccess.h: interface for the CkFileAccess class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKFILEACCESS_H
#define _CKFILEACCESS_H

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

// CLASS: CkFileAccess
class CkFileAccess  : public CkObject
{
    public:
	CkFileAccess();
	virtual ~CkFileAccess();

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

        bool ReadEntireTextFile(const char *filename, const char *charset, CkString &strFileContents);

        const char *readEntireTextFile(const char *filename, const char *charset);
	// CURRENTDIR_BEGIN
	void get_CurrentDir(CkString &str);
	const char *currentDir(void);
	// CURRENTDIR_END
	// ENDOFFILE_BEGIN
	bool get_EndOfFile(void);
	// ENDOFFILE_END
	// SETCURRENTDIR_BEGIN
	bool SetCurrentDir(const char *path);
	// SETCURRENTDIR_END
	// APPENDANSI_BEGIN
	bool AppendAnsi(const char *text);
	// APPENDANSI_END
	// APPENDTEXT_BEGIN
	bool AppendText(const char *text, const char *charset);
	// APPENDTEXT_END
	// APPENDUNICODEBOM_BEGIN
	bool AppendUnicodeBOM(void);
	// APPENDUNICODEBOM_END
	// APPENDUTF8BOM_BEGIN
	bool AppendUtf8BOM(void);
	// APPENDUTF8BOM_END
	// DIRAUTOCREATE_BEGIN
	bool DirAutoCreate(const char *path);
	// DIRAUTOCREATE_END
	// DIRCREATE_BEGIN
	bool DirCreate(const char *path);
	// DIRCREATE_END
	// DIRDELETE_BEGIN
	bool DirDelete(const char *path);
	// DIRDELETE_END
	// DIRENSUREEXISTS_BEGIN
	bool DirEnsureExists(const char *filePath);
	// DIRENSUREEXISTS_END
	// FILECLOSE_BEGIN
	void FileClose(void);
	// FILECLOSE_END
	// FILECOPY_BEGIN
	bool FileCopy(const char *existing, const char *newFilename, bool failIfExists);
	// FILECOPY_END
	// FILEDELETE_BEGIN
	bool FileDelete(const char *filename);
	// FILEDELETE_END
	// FILEEXISTS_BEGIN
	bool FileExists(const char *filename);
	// FILEEXISTS_END
	// FILEOPEN_BEGIN
	bool FileOpen(const char *filename, int accessMode, int shareMode, int createDisp, int attr);
	// FILEOPEN_END
	// FILEREAD_BEGIN
	bool FileRead(int numBytes, CkByteData &outBytes);
	// FILEREAD_END
	// FILERENAME_BEGIN
	bool FileRename(const char *existing, const char *newFilename);
	// FILERENAME_END
	// FILESEEK_BEGIN
	bool FileSeek(int offset, int origin);
	// FILESEEK_END
	// FILESIZE_BEGIN
	int FileSize(const char *filename);
	// FILESIZE_END
	// FILEWRITE_BEGIN
	bool FileWrite(CkByteData &data);
	// FILEWRITE_END
	// GETTEMPFILENAME_BEGIN
	bool GetTempFilename(const char *dirName, const char *prefix, CkString &outStr);
	const char *getTempFilename(const char *dirName, const char *prefix);
	// GETTEMPFILENAME_END
	// READENTIREFILE_BEGIN
	bool ReadEntireFile(const char *filename, CkByteData &outBytes);
	// READENTIREFILE_END
	// TREEDELETE_BEGIN
	bool TreeDelete(const char *path);
	// TREEDELETE_END
	// WRITEENTIREFILE_BEGIN
	bool WriteEntireFile(const char *filename, CkByteData &fileData);
	// WRITEENTIREFILE_END
	// WRITEENTIRETEXTFILE_BEGIN
	bool WriteEntireTextFile(const char *filename, const char *fileData, const char *charset, bool includePreamble);
	// WRITEENTIRETEXTFILE_END
	// SPLITFILE_BEGIN
	bool SplitFile(const char *fileToSplit, const char *partPrefix, const char *partExtension, int partSize, const char *outputDirPath);
	// SPLITFILE_END
	// REASSEMBLEFILE_BEGIN
	bool ReassembleFile(const char *partsDirPath, const char *partPrefix, const char *partExtension, const char *reassembledFilename);
	// REASSEMBLEFILE_END
	// READBINARYTOENCODED_BEGIN
	bool ReadBinaryToEncoded(const char *filename, const char *encoding, CkString &outStr);
	const char *readBinaryToEncoded(const char *filename, const char *encoding);
	// READBINARYTOENCODED_END

	// FILEACCESS_INSERT_POINT

	// END PUBLIC INTERFACE

    // For internal use only.
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

	// Don't allow assignment or copying these objects.
	CkFileAccess(const CkFileAccess &) { } 
	CkFileAccess &operator=(const CkFileAccess &) { return *this; }
	CkFileAccess(void *impl);

    public:
	void *getImpl(void) const { return m_impl; } 


};

#pragma pack (pop)

#endif


