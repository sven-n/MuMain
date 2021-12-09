// CkCertStore.h: interface for the CkCertStore class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKCERTSTORE_H
#define _CKCERTSTORE_H

#pragma once

class CkCert;
class CkString;
#include "CkObject.h"
#include "CkString.h"

class CkByteData;

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#pragma pack (push, 8) 

// CLASS: CkCertStore
class CkCertStore  : public CkObject
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkCertStore(const CkCertStore &) { } 
	CkCertStore &operator=(const CkCertStore &) { return *this; }
	CkCertStore(void *impl) : m_impl(impl) { }

    public:
	void *getImpl(void) const { return m_impl; } 
	void inject(void *impl);

	CkCertStore();
	virtual ~CkCertStore();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	bool LoadPfxFile(const char *filename, const char *password);
	bool LoadPfxData(CkByteData &pfxData, const char *password);
	bool LoadPfxData2(const unsigned char *buf, unsigned long bufLen, const char *password);

	CkCert *FindCertBySerial(const char *serialNumber);
	CkCert *FindCertBySubjectE(const char *emailAddress);
	CkCert *FindCertBySubjectO(const char *organization);
	CkCert *FindCertBySubjectCN(const char *commonName);
	CkCert *FindCertBySubject(const char *subject);
	CkCert *GetCertificate(long index);

	long get_NumCertificates(void);
	void get_Version(CkString &version);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	CkString m_resultString;
        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);
        const char *version(void);

	CkCert *FindCertByRfc822Name(const char *name);
	CkCert *FindCertBySha1Thumbprint(const char *str);
	bool get_VerboseLogging(void);
	void put_VerboseLogging(bool newVal);

	// CERTSTORE_INSERT_POINT

	// END PUBLIC INTERFACE

#ifdef WIN32
	CkCert *GetEmailCert(long index);
	CkCert *FindCertForEmail(const char *emailAddress);
	long get_NumEmailCerts(void);

	bool RemoveCertificate(const CkCert *cert);
	bool AddCertificate(const CkCert *cert);
	bool RemoveCertificate(const CkCert &cert) { return this->RemoveCertificate(&cert); }
	bool AddCertificate(const CkCert &cert) { return this->AddCertificate(&cert); }

	bool CreateFileStore(const char *filename);
	bool CreateMemoryStore(void);
	bool CreateRegistryStore(const char *regRoot, const char *regPath);

	bool OpenChilkatStore(bool readOnly);
	bool OpenCurrentUserStore(bool readOnly);
	bool OpenFileStore(const char *filename, bool readOnly);
	bool OpenOutlookStore(bool readOnly);
	bool OpenRegistryStore(const char *regRoot, const char *regPath, bool readOnly);

        bool GetCertStoreSpec(CkString &css);
#endif

};
#pragma pack (pop)

#endif
