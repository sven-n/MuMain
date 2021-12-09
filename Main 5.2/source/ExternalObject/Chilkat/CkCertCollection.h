// CkCertCollection.h: interface for the CkCertCollection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CKCERTCOLLECTION_H__C76FB184_136E_442A_ACFF_773BD284D60E__INCLUDED_)
#define AFX_CKCERTCOLLECTION_H__C76FB184_136E_442A_ACFF_773BD284D60E__INCLUDED_

#pragma once

class CkCert;
class CkString;
#include "CkObject.h"
#include "CkString.h"

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#pragma pack (push, 8) 

// Holds one or more certificates.
// CLASS: CkCertCollection
class CkCertCollection  : public CkObject
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkCertCollection(const CkCertCollection &) { } 
	CkCertCollection &operator=(const CkCertCollection &) { return *this; }
	CkCertCollection(void *impl) : m_impl(impl) { }

    public:
	CkCertCollection();
	virtual ~CkCertCollection();

	void *getImpl(void) const { return m_impl; } 

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);


	bool LoadPkcs7File(const char *filename);
	bool LoadPkcs7Data(const unsigned char *data, unsigned long dataLen);

	CkCert *GetCert(int index);

	int get_NumCerts(void) const;

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	// CK_INSERT_POINT

	// END PUBLIC INTERFACE

};
#pragma pack (pop)

#endif // !defined(AFX_CKCERTCOLLECTION_H__C76FB184_136E_442A_ACFF_773BD284D60E__INCLUDED_)

