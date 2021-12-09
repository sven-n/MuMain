// CkRsa.h: interface for the CkRsa class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CkRsa_H
#define _CkRsa_H

#pragma once

#include "CkString.h"
class CkByteData;
#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkRsa
class CkRsa  : public CkObject
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkRsa(const CkRsa &);
	CkRsa &operator=(const CkRsa &);

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const;

	CkRsa(void *impl);

	CkRsa();
	virtual ~CkRsa();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	bool UnlockComponent(const char *unlockCode);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	bool DecryptStringENC(const char *str, bool bUsePrivateKey, CkString &out);
	bool DecryptString(const CkByteData &bData, bool bUsePrivateKey, CkString &out);
	bool DecryptBytesENC(const char *str, bool bUsePrivateKey, CkByteData &out);
	bool DecryptBytes(const CkByteData &bData, bool bUsePrivateKey, CkByteData &out);

	bool EncryptStringENC(const char *str, bool bUsePrivateKey, CkString &out);
	bool EncryptBytesENC(const CkByteData &bData, bool bUsePrivateKey, CkString &out);
	bool EncryptString(const char *str, bool bUsePrivateKey, CkByteData &out);
	bool EncryptBytes(const CkByteData &bData, bool bUsePrivateKey, CkByteData &out);

	void get_EncodingMode(CkString &out);
	void put_EncodingMode(const char *str);
	void get_Charset(CkString &out);
	void put_Charset(const char *str);

	// NEED VERSION.
	//void get_Version(CkString &out);

	// numBits can range from 384 to 16384
	bool GenerateKey(int numBits);

	bool ExportPublicKey(CkString &strXml);
	bool ExportPrivateKey(CkString &strXml);

	bool ImportPublicKey(const char *strXml);
	bool ImportPrivateKey(const char *strXml);

	// x1 is a .snk filename.  Returns XML
	bool SnkToXml(const char *filename, CkString &strXml);

	// Number of bits in the RSA key (i.e. 1024, 2048, etc.)
	long get_NumBits(void);

	bool get_OaepPadding(void);
	void put_OaepPadding(bool newVal);

	// If true, use Crypto API's byte ordering (little endian) when creating signatures.
	// The default value of this property is true.
	bool get_LittleEndian(void);
	void put_LittleEndian(bool newVal);

	// New in Version 3.0
	bool VerifyStringENC(const char *str, const char *hashAlg, const char *sig);
	bool VerifyString(const char *str, const char *hashAlg, const CkByteData &sigData);
	bool VerifyBytesENC(const CkByteData &bData, const char *hashAlg, const char *encodedSig);
	bool VerifyBytes(const CkByteData &bData, const char *hashAlg, const CkByteData &sigData);

	const char *signStringENC(const char *str, const char *hashAlg);
	const char *signBytesENC(const CkByteData &bData, const char *hashAlg);

	bool SignStringENC(const char *str, const char *hashAlg, CkString &out);
	bool SignBytesENC(const CkByteData &bData, const char *hashAlg, CkString &out);
	bool SignString(const char *str, const char *hashAlg, CkByteData &out);
	bool SignBytes(const CkByteData &bData, const char *hashAlg, CkByteData &out);


        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);

	const char *decryptStringENC(const char *str, bool bUsePrivateKey);
	const char *decryptString(const CkByteData &bData, bool bUsePrivateKey);
	const char *encryptStringENC(const char *str, bool bUsePrivateKey);
	const char *encryptBytesENC(const CkByteData &bData, bool bUsePrivateKey);
	const char *encodingMode(void);
	const char *charset(void);
	const char *exportPublicKey(void);
	const char *exportPrivateKey(void);
	const char *snkToXml(const char *filename);

	// VERSION_BEGIN
	void get_Version(CkString &str);
	const char *version(void);
	// VERSION_END
	// OPENSSLVERIFYBYTES_BEGIN
	bool OpenSslVerifyBytes(CkByteData &signature, CkByteData &outBytes);
	// OPENSSLVERIFYBYTES_END
	// OPENSSLSIGNBYTES_BEGIN
	bool OpenSslSignBytes(CkByteData &data, CkByteData &outBytes);
	// OPENSSLSIGNBYTES_END
	// OPENSSLSIGNBYTESENC_BEGIN
	bool OpenSslSignBytesENC(CkByteData &data, CkString &outStr);
	const char *openSslSignBytesENC(CkByteData &data);
	// OPENSSLSIGNBYTESENC_END
	// OPENSSLSIGNSTRING_BEGIN
	bool OpenSslSignString(const char *str, CkByteData &outBytes);
	// OPENSSLSIGNSTRING_END
	// OPENSSLSIGNSTRINGENC_BEGIN
	bool OpenSslSignStringENC(const char *str, CkString &outStr);
	const char *openSslSignStringENC(const char *str);
	// OPENSSLSIGNSTRINGENC_END
	// OPENSSLVERIFYBYTESENC_BEGIN
	bool OpenSslVerifyBytesENC(const char *str, CkByteData &outBytes);
	// OPENSSLVERIFYBYTESENC_END
	// OPENSSLVERIFYSTRING_BEGIN
	bool OpenSslVerifyString(CkByteData &data, CkString &outStr);
	const char *openSslVerifyString(CkByteData &data);
	// OPENSSLVERIFYSTRING_END
	// OPENSSLVERIFYSTRINGENC_BEGIN
	bool OpenSslVerifyStringENC(const char *str, CkString &outStr);
	const char *openSslVerifyStringENC(const char *str);
	// OPENSSLVERIFYSTRINGENC_END
	// VERIFYPRIVATEKEY_BEGIN
	bool VerifyPrivateKey(const char *xml);
	// VERIFYPRIVATEKEY_END
	// VERIFYHASH_BEGIN
	bool VerifyHash(CkByteData &hashBytes, const char *hashAlg, CkByteData &sigBytes);
	// VERIFYHASH_END
	// VERIFYHASHENC_BEGIN
	bool VerifyHashENC(const char *encodedHash, const char *hashAlg, const char *encodedSig);
	// VERIFYHASHENC_END
	// SIGNHASH_BEGIN
	bool SignHash(CkByteData &hashBytes, const char *hashAlg, CkByteData &outBytes);
	// SIGNHASH_END
	// SIGNHASHENC_BEGIN
	bool SignHashENC(const char *encodedHash, const char *hashAlg, CkString &outStr);
	const char *signHashENC(const char *encodedHash, const char *hashAlg);
	// SIGNHASHENC_END
	// VERBOSELOGGING_BEGIN
	bool get_VerboseLogging(void);
	void put_VerboseLogging(bool newVal);
	// VERBOSELOGGING_END
	// NOUNPAD_BEGIN
	bool get_NoUnpad(void);
	void put_NoUnpad(bool newVal);
	// NOUNPAD_END

	// RSA_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)


#endif


