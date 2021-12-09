// CkCreateCS.h: interface for the CkCreateCS class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKCREATECS_H
#define _CKCREATECS_H


#pragma once

class CkCertStore;
#include "CkString.h"
#include "CkObject.h"

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#pragma pack (push, 8) 

// CLASS: CkCreateCS
class CkCreateCS  : public CkObject
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkCreateCS(const CkCreateCS &) { } 
	CkCreateCS &operator=(const CkCreateCS &) { return *this; }
	CkCreateCS(void *impl) : m_impl(impl) { }

    public:
	void *getImpl(void) const { return m_impl; } 

	CkCreateCS();
	virtual ~CkCreateCS();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);


	bool get_ReadOnly(void);
	void put_ReadOnly(bool newVal);
	void get_Version(CkString &version);

	CkCertStore *CreateRegistryStore(const char *regRoot, const char *regPath);
	CkCertStore *OpenRegistryStore(const char *regRoot, const char *regPath);
	CkCertStore *CreateFileStore(const char *filename);
	CkCertStore *OpenFileStore(const char *filename);
	CkCertStore *CreateMemoryStore(void);
	CkCertStore *OpenChilkatStore(void);
	CkCertStore *OpenOutlookStore(void);
	CkCertStore *OpenLocalSystemStore(void);
	CkCertStore *OpenCurrentUserStore(void);


	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	CkString m_resultString;
        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);

	// CREATECS_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)


#endif

