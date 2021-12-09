// CkCSP.h: interface for the CkCSP class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKCSP_H
#define _CKCSP_H

#pragma once

class CkStringArray;

#include "CkObject.h"
#include "CkString.h"

#pragma pack (push, 8) 

// CLASS: CkCSP
class CkCSP  : public CkObject
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkCSP(const CkCSP &) { } 
	CkCSP &operator=(const CkCSP &) { return *this; }
	CkCSP(void *impl) : m_impl(impl) { }

    public:
	void *getImpl(void) const { return m_impl; } 

	CkCSP();
	virtual ~CkCSP();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);


	bool SetProviderMicrosoftBase();
	bool SetProviderMicrosoftStrong();
	bool SetProviderMicrosoftEnhanced();
	bool SetProviderMicrosoftRsaAes();
	long get_ProviderType();
	CkStringArray *GetKeyContainerNames(void);

	long SetHashAlgorithm(const char *name);
	bool HasHashAlgorithm(const char *name, long numBits);

	long SetEncryptAlgorithm(const char *name);
	bool HasEncryptAlgorithm(const char *name, long numBits);
	long get_HashAlgorithmID();
	long get_EncryptAlgorithmID();
	long get_HashNumBits();
	long get_EncryptNumBits();
	void get_HashAlgorithm(CkString &alg);
	void get_EncryptAlgorithm(CkString &alg);
	bool GetKeyContainerName(long index, CkString &name);
	long get_NumKeyContainers();
	long GetSignatureNumBits(long index);
	bool GetSignatureAlgorithm(long index, CkString &name);
	long GetKeyExchangeNumBits(long index);
	bool GetKeyExchangeAlgorithm(long index, CkString &name);
	long GetHashNumBits(long index);
	bool GetHashAlgorithm(long index, CkString &name);
	long GetEncryptionNumBits(long index);
	bool GetEncryptionAlgorithm(long index, CkString &name);
	void get_KeyContainerName(CkString &name);
	void put_KeyContainerName(const char *name);
	void get_ProviderName(CkString &name);
	void put_ProviderName(const char *name);
	long get_NumKeyExchangeAlgorithms();
	long get_NumSignatureAlgorithms();
	long get_NumEncryptAlgorithms();
	long get_NumHashAlgorithms();

	bool get_MachineKeyset();
	void put_MachineKeyset(bool val);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	CkString m_resultString;
        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);

	const char *hashAlgorithm(void);
	const char *encryptAlgorithm(void);
	const char *getKeyContainerName(long index);
	const char *getSignatureAlgorithm(long index);
	const char *getKeyExchangeAlgorithm(long index);
	const char *getHashAlgorithm(long index);
	const char *getEncryptionAlgorithm(long index);
	const char *keyContainerName(void);
	const char *providerName(void);

	// CSP_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)


#endif
