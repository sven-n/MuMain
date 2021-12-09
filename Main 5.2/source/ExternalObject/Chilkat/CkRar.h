// CkRar.h: interface for the CkRar class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKRAR_H
#define _CKRAR_H

#pragma once

#include "CkString.h"
class CkByteData;
//class CkRarProgress;
class CkRarEntry;
class CkStringArray;
#include "CkObject.h"

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

*/
#pragma pack (push, 8) 

// CLASS: CkRar
class CkRar  : public CkObject
{
    public:
	CkRar();
	virtual ~CkRar();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);


	bool Unrar(const char *dirPath);

	bool FastOpen(const char *filename);

	bool Open(const char *filename);

	bool Close(void);

	long get_NumEntries(void);
	CkRarEntry *GetEntryByName(const char *filename);
	CkRarEntry *GetEntryByIndex(long index);


	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	CkString m_resultString;
        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);


	// CK_INSERT_POINT

	// END PUBLIC INTERFACE



    // Everything below is for internal use only.
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkRar(const CkRar &);
	CkRar &operator=(const CkRar &);
	CkRar(void *impl);

    public:
	void *getImpl(void) const;


};
#pragma pack (pop)

#endif
