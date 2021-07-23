// CkDkim.h: interface for the CkDkim class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CkDkim_H
#define _CkDkim_H

#pragma once

#include "CkString.h"
class CkByteData;
class CkDkimProgress;
#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkDkim
class CkDkim  : public CkObject
{
    private:
	CkDkimProgress *m_callback;

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkDkim(const CkDkim &);
	CkDkim &operator=(const CkDkim &);

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const;

	CkDkim(void *impl);

	CkDkim();
	virtual ~CkDkim();

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
	
	CkDkimProgress *get_EventCallbackObject(void) const;
	void put_EventCallbackObject(CkDkimProgress *progress);

	// UNLOCKCOMPONENT_BEGIN
	bool UnlockComponent(const char *unlockCode);
	// UNLOCKCOMPONENT_END
	// DKIMCANON_BEGIN
	void get_DkimCanon(CkString &str);
	const char *dkimCanon(void);
	void put_DkimCanon(const char *newVal);
	// DKIMCANON_END
	// DOMAINKEYCANON_BEGIN
	void get_DomainKeyCanon(CkString &str);
	const char *domainKeyCanon(void);
	void put_DomainKeyCanon(const char *newVal);
	// DOMAINKEYCANON_END
	// DKIMALG_BEGIN
	void get_DkimAlg(CkString &str);
	const char *dkimAlg(void);
	void put_DkimAlg(const char *newVal);
	// DKIMALG_END
	// DOMAINKEYALG_BEGIN
	void get_DomainKeyAlg(CkString &str);
	const char *domainKeyAlg(void);
	void put_DomainKeyAlg(const char *newVal);
	// DOMAINKEYALG_END
	// DKIMHEADERS_BEGIN
	void get_DkimHeaders(CkString &str);
	const char *dkimHeaders(void);
	void put_DkimHeaders(const char *newVal);
	// DKIMHEADERS_END
	// DOMAINKEYHEADERS_BEGIN
	void get_DomainKeyHeaders(CkString &str);
	const char *domainKeyHeaders(void);
	void put_DomainKeyHeaders(const char *newVal);
	// DOMAINKEYHEADERS_END
	// DKIMDOMAIN_BEGIN
	void get_DkimDomain(CkString &str);
	const char *dkimDomain(void);
	void put_DkimDomain(const char *newVal);
	// DKIMDOMAIN_END
	// DOMAINKEYDOMAIN_BEGIN
	void get_DomainKeyDomain(CkString &str);
	const char *domainKeyDomain(void);
	void put_DomainKeyDomain(const char *newVal);
	// DOMAINKEYDOMAIN_END
	// DKIMSELECTOR_BEGIN
	void get_DkimSelector(CkString &str);
	const char *dkimSelector(void);
	void put_DkimSelector(const char *newVal);
	// DKIMSELECTOR_END
	// DOMAINKEYSELECTOR_BEGIN
	void get_DomainKeySelector(CkString &str);
	const char *domainKeySelector(void);
	void put_DomainKeySelector(const char *newVal);
	// DOMAINKEYSELECTOR_END
	// DKIMBODYLENGTHCOUNT_BEGIN
	int get_DkimBodyLengthCount(void);
	void put_DkimBodyLengthCount(int newVal);
	// DKIMBODYLENGTHCOUNT_END
	// LOADDKIMPK_BEGIN
	bool LoadDkimPk(const char *privateKey, const char *optionalPassword);
	// LOADDKIMPK_END
	// LOADDOMAINKEYPK_BEGIN
	bool LoadDomainKeyPk(const char *privateKey, const char *optionalPassword);
	// LOADDOMAINKEYPK_END
	// LOADDKIMPKFILE_BEGIN
	bool LoadDkimPkFile(const char *privateKeyFilepath, const char *optionalPassword);
	// LOADDKIMPKFILE_END
	// LOADDOMAINKEYPKFILE_BEGIN
	bool LoadDomainKeyPkFile(const char *privateKeyFilepath, const char *optionalPassword);
	// LOADDOMAINKEYPKFILE_END
	// LOADPUBLICKEY_BEGIN
	bool LoadPublicKey(const char *selector, const char *domain, const char *publicKey);
	// LOADPUBLICKEY_END
	// LOADPUBLICKEYFILE_BEGIN
	bool LoadPublicKeyFile(const char *selector, const char *domain, const char *publicKeyFilepath);
	// LOADPUBLICKEYFILE_END
	// PREFETCHPUBLICKEY_BEGIN
	bool PrefetchPublicKey(const char *selector, const char *domain);
	// PREFETCHPUBLICKEY_END
	// VERIFYDKIMSIGNATURE_BEGIN
	bool VerifyDkimSignature(int sigIdx, CkByteData &mimeData);
	// VERIFYDKIMSIGNATURE_END
	// VERIFYDOMAINKEYSIGNATURE_BEGIN
	bool VerifyDomainKeySignature(int sigIdx, CkByteData &mimeData);
	// VERIFYDOMAINKEYSIGNATURE_END
	// NUMDKIMSIGNATURES_BEGIN
	int NumDkimSignatures(CkByteData &mimeData);
	// NUMDKIMSIGNATURES_END
	// NUMDOMAINKEYSIGNATURES_BEGIN
	int NumDomainKeySignatures(CkByteData &mimeData);
	// NUMDOMAINKEYSIGNATURES_END
	// ADDDKIMSIGNATURE_BEGIN
	bool AddDkimSignature(CkByteData &mimeIn, CkByteData &outBytes);
	// ADDDKIMSIGNATURE_END
	// ADDDOMAINKEYSIGNATURE_BEGIN
	bool AddDomainKeySignature(CkByteData &mimeIn, CkByteData &outBytes);
	// ADDDOMAINKEYSIGNATURE_END
	// LOADDKIMPKBYTES_BEGIN
	bool LoadDkimPkBytes(CkByteData &privateKeyDer, const char *optionalPassword);
	// LOADDKIMPKBYTES_END
	// LOADDOMAINKEYPKBYTES_BEGIN
	bool LoadDomainKeyPkBytes(CkByteData &privateKeyDer, const char *optionalPassword);
	// LOADDOMAINKEYPKBYTES_END

	// DKIM_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)


#endif


