// CkDh.h: interface for the CkDh class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKDH_H
#define _CKDH_H

#pragma once

#include "CkString.h"
class CkByteData;

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#include "CkObject.h"

class CkDhProgress;

#pragma pack (push, 8) 

// CLASS: CkDh
class CkDh  : public CkObject
{
    private:
	CkDhProgress *m_callback;

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkDh(const CkDh &) { } 
	CkDh &operator=(const CkDh &) { return *this; }
	CkDh(void *impl) : m_impl(impl),m_callback(0) { }

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const { return m_impl; } 

	CkDh();
	virtual ~CkDh();


	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);
	
	CkDhProgress *get_EventCallbackObject(void) const;
	void put_EventCallbackObject(CkDhProgress *progress);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
	void LastErrorXml(CkString &str);
	void LastErrorHtml(CkString &str);
	void LastErrorText(CkString &str);

    const char *lastErrorText(void);
    const char *lastErrorXml(void);
    const char *lastErrorHtml(void);
	void UseKnownPrime(int index);
	// G_BEGIN
	int get_G(void);
	// G_END
	// P_BEGIN
	void get_P(CkString &str);
	const char *p(void);
	// P_END
	// SETPG_BEGIN
	bool SetPG(const char *p, int g);
	// SETPG_END
	// CREATEE_BEGIN
	bool CreateE(int numBits, CkString &outStr);
	const char *createE(int numBits);
	// CREATEE_END
	// FINDK_BEGIN
	bool FindK(const char *e, CkString &outStr);
	const char *findK(const char *e);
	// FINDK_END
	// GENPG_BEGIN
	bool GenPG(int numBits, int g);
	// GENPG_END
	// VERSION_BEGIN
	void get_Version(CkString &str);
	const char *version(void);
	// VERSION_END
	// UNLOCKCOMPONENT_BEGIN
	bool UnlockComponent(const char *unlockCode);
	// UNLOCKCOMPONENT_END

	// DH_INSERT_POINT

	// END PUBLIC INTERFACE

	


};
#pragma pack (pop)


#endif


