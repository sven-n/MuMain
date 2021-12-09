// CkCrypt.h: interface for the CkCrypt class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKCRYPT_H
#define _CKCRYPT_H

#pragma once

#ifdef WIN32
class CkCSP;
#endif

class CkCert;
class CkCertStore;
#include "CkString.h"
class CkByteData;
#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkCrypt
class CkCrypt  : public CkObject
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkCrypt(const CkCrypt &) { } 
	CkCrypt &operator=(const CkCrypt &) { return *this; }
	CkCrypt(void *impl) : m_impl(impl) { }

    public:
	void *getImpl(void) const { return m_impl; } 

	CkCrypt();
	virtual ~CkCrypt();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);


#ifdef WIN32
	bool SetCSP(const CkCSP *csp);
#endif
	void SetAlgorithmRSA();
	void SetAlgorithmTwofish();
	void SetAlgorithmBlowfish();
	void SetAlgorithmRijndael();

#ifdef WIN32
	bool AddSearchCertStore(const CkCertStore *certStore);
#endif

	void SetEncryptCertificate(const CkCert *cert);
	void SetSigningCertificate(const CkCert *cert);

	bool UnlockComponent(const char *unlockCode);
	bool IsUnlocked(void) const;

	void get_LastCertSubject(CkString &str);
	void get_LastAlgorithm(CkString &str);

	void get_SigningCertSubject(CkString &str);
	void put_SigningCertSubject(const char *str);

	void get_EncryptCertSubject(CkString &str);
	void put_EncryptCertSubject(const char *str);

	//bool get_UseStandardSearchCertStores(void);
	//void put_UseStandardSearchCertStores(bool newVal);

	// Key length is implied by the length of the secret key.
	void get_SecretKey(CkByteData &data);
	void put_SecretKey(const unsigned char *data, unsigned long dataLen);

	// Generate a secret key according to a particular method.
	void PassPhraseToSecretKey(const char *newVal);

	void get_Version(CkString &str);

	long get_KeyLength(void);
	void put_KeyLength(long newVal);

	bool CreateSignature(const unsigned char *data, unsigned long dataLen, CkByteData &sigData);
	bool VerifySignature(const unsigned char *data, unsigned long dataLen, 
	    const unsigned char *sig, unsigned long sigLen);

	bool Encrypt(const unsigned char *data, unsigned long dataLen, CkByteData &outData);
	bool Decrypt(const unsigned char *data, unsigned long dataLen, CkByteData &outData);

	void Base64Encode(const unsigned char *data, unsigned long dataLen, CkString &strOut);
	bool Base64Decode(const char *str, CkByteData &dbOut);

	void QuotedPrintableEncode(const unsigned char *data, unsigned long dataLen, CkString &strOut);
	bool QuotedPrintableDecode(const char *str, CkByteData &dbOut);
	
	void HexEncode(const unsigned char *data, unsigned long dataLen, CkString &strOut);
	void HexDecode(const char *str, CkByteData &dbOut);

	void CalcDigestMD5(const unsigned char *data, unsigned long dataLen, CkByteData &digest);
	bool CalcSha1Hash(const unsigned char *data, unsigned long dataLen, CkByteData &hash);
	bool CalcMd2Hash(const unsigned char *data, unsigned long dataLen, CkByteData &hash);

	bool get_HasIV(void);
	void put_HasIV(bool b);

	void get_IV(CkByteData &db);
	void put_IV(const unsigned char *ivData);

	void GenerateRandomIV(void);

	bool ZipAndEncrypt(const unsigned char *data, unsigned long dataLen, CkByteData &outData);
	bool DecryptAndUnzip(const unsigned char *data, unsigned long dataLen, CkByteData &outData);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	// CRYPT_INSERT_POINT

	// END PUBLIC INTERFACE

};
#pragma pack (pop)


#endif


