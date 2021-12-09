// CkTar.h: interface for the CkTar class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKTAR_H
#define _CKTAR_H

#pragma once


class CkByteData;
class CkTarProgress;

#include "CkString.h"
#include "CkObject.h"

#pragma warning( disable : 4068 )
#pragma unmanaged

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */
#pragma pack (push, 8) 


// CLASS: CkTar
class CkTar  : public CkObject
{
    private:
	CkTarProgress *m_callback;

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	CkTar();
	virtual ~CkTar();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	void get_Version(CkString &str);

	CkTarProgress *get_EventCallbackObject(void) const;
	void put_EventCallbackObject(CkTarProgress *progress);

        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);
	const char *version(void); 
	// NOABSOLUTEPATHS_BEGIN
	bool get_NoAbsolutePaths(void);
	void put_NoAbsolutePaths(bool newVal);
	// NOABSOLUTEPATHS_END
	// UNTARCASESENSITIVE_BEGIN
	bool get_UntarCaseSensitive(void);
	void put_UntarCaseSensitive(bool newVal);
	// UNTARCASESENSITIVE_END
	// UNTARDEBUGLOG_BEGIN
	bool get_UntarDebugLog(void);
	void put_UntarDebugLog(bool newVal);
	// UNTARDEBUGLOG_END
	// UNTARDISCARDPATHS_BEGIN
	bool get_UntarDiscardPaths(void);
	void put_UntarDiscardPaths(bool newVal);
	// UNTARDISCARDPATHS_END
	// UNTARFROMDIR_BEGIN
	void get_UntarFromDir(CkString &str);
	const char *untarFromDir(void);
	void put_UntarFromDir(const char *newVal);
	// UNTARFROMDIR_END
	// UNTARMATCHPATTERN_BEGIN
	void get_UntarMatchPattern(CkString &str);
	const char *untarMatchPattern(void);
	void put_UntarMatchPattern(const char *newVal);
	// UNTARMATCHPATTERN_END
	// UNTARMAXCOUNT_BEGIN
	int get_UntarMaxCount(void);
	void put_UntarMaxCount(int newVal);
	// UNTARMAXCOUNT_END
	// UNLOCKCOMPONENT_BEGIN
	bool UnlockComponent(const char *unlockCode);
	// UNLOCKCOMPONENT_END
	// UNTAR_BEGIN
	int Untar(const char *tarFilename);
	// UNTAR_END
	// UNTARFROMMEMORY_BEGIN
	int UntarFromMemory(CkByteData &tarFileBytes);
	// UNTARFROMMEMORY_END
	// UNTARFIRSTMATCHINGTOMEMORY_BEGIN
	bool UntarFirstMatchingToMemory(CkByteData &tarFileBytes, const char *matchPattern, CkByteData &outBytes);
	// UNTARFIRSTMATCHINGTOMEMORY_END
	// ADDDIRROOT_BEGIN
	bool AddDirRoot(const char *dirPath);
	// ADDDIRROOT_END
	// NUMDIRROOTS_BEGIN
	int get_NumDirRoots(void);
	// NUMDIRROOTS_END
	// GETDIRROOT_BEGIN
	bool GetDirRoot(int index, CkString &outStr);
	const char *getDirRoot(int index);
	// GETDIRROOT_END
	// WRITETAR_BEGIN
	bool WriteTar(const char *tarFilename);
	// WRITETAR_END
	// WRITETARGZ_BEGIN
	bool WriteTarGz(const char *outFilename);
	// WRITETARGZ_END
	// WRITETARBZ2_BEGIN
	bool WriteTarBz2(const char *outFilename);
	// WRITETARBZ2_END
	// WRITETARZ_BEGIN
	bool WriteTarZ(const char *outFilename);
	// WRITETARZ_END
	// LISTXML_BEGIN
	bool ListXml(const char *tarFilename, CkString &outStr);
	const char *listXml(const char *tarFilename);
	// LISTXML_END
	// CHARSET_BEGIN
	void get_Charset(CkString &str);
	const char *charset(void);
	void put_Charset(const char *newVal);
	// CHARSET_END
	// UNTARZ_BEGIN
	bool UntarZ(const char *tarFilename);
	// UNTARZ_END
	// UNTARGZ_BEGIN
	bool UntarGz(const char *tarFilename);
	// UNTARGZ_END
	// UNTARBZ2_BEGIN
	bool UntarBz2(const char *tarFilename);
	// UNTARBZ2_END
	// USERNAME_BEGIN
	void get_UserName(CkString &str);
	const char *userName(void);
	void put_UserName(const char *newVal);
	// USERNAME_END
	// GROUPNAME_BEGIN
	void get_GroupName(CkString &str);
	const char *groupName(void);
	void put_GroupName(const char *newVal);
	// GROUPNAME_END
	// USERID_BEGIN
	int get_UserId(void);
	void put_UserId(int newVal);
	// USERID_END
	// GROUPID_BEGIN
	int get_GroupId(void);
	void put_GroupId(int newVal);
	// GROUPID_END
	// DIRMODE_BEGIN
	int get_DirMode(void);
	void put_DirMode(int newVal);
	// DIRMODE_END
	// FILEMODE_BEGIN
	int get_FileMode(void);
	void put_FileMode(int newVal);
	// FILEMODE_END
	// VERBOSELOGGING_BEGIN
	bool get_VerboseLogging(void);
	void put_VerboseLogging(bool newVal);
	// VERBOSELOGGING_END
	// WRITEFORMAT_BEGIN
	void get_WriteFormat(CkString &str);
	const char *writeFormat(void);
	void put_WriteFormat(const char *newVal);
	// WRITEFORMAT_END
	// HEARTBEATMS_BEGIN
	int get_HeartbeatMs(void);
	void put_HeartbeatMs(int newVal);
	// HEARTBEATMS_END

	// TAR_INSERT_POINT

	// END PUBLIC INTERFACE


    // For internal use only.
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkTar(const CkTar &) { } 
	CkTar &operator=(const CkTar &) { return *this; }
	CkTar(void *impl);

    public:
	void *getImpl(void) const { return m_impl; } 



};

#pragma pack (pop)

#endif


