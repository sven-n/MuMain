// CkSshKey.h: interface for the CkSshKey class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CkSshKey_H
#define _CkSshKey_H

#pragma once

#include "CkString.h"
class CkByteData;
#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkSshKey
class CkSshKey  : public CkObject
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkSshKey(const CkSshKey &);
	CkSshKey &operator=(const CkSshKey &);

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const;

	CkSshKey(void *impl);

	CkSshKey();
	virtual ~CkSshKey();

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

	// ISRSAKEY_BEGIN
	bool get_IsRsaKey(void);
	// ISRSAKEY_END
	// ISDSAKEY_BEGIN
	bool get_IsDsaKey(void);
	// ISDSAKEY_END
	// ISPRIVATEKEY_BEGIN
	bool get_IsPrivateKey(void);
	// ISPRIVATEKEY_END
	// PASSWORD_BEGIN
	void get_Password(CkString &str);
	const char *password(void);
	void put_Password(const char *newVal);
	// PASSWORD_END
	// COMMENT_BEGIN
	void get_Comment(CkString &str);
	const char *comment(void);
	void put_Comment(const char *newVal);
	// COMMENT_END
	// GENFINGERPRINT_BEGIN
	bool GenFingerprint(CkString &outStr);
	const char *genFingerprint(void);
	// GENFINGERPRINT_END
	// TOXML_BEGIN
	bool ToXml(CkString &outStr);
	const char *toXml(void);
	// TOXML_END
	// TORFC4716PUBLICKEY_BEGIN
	bool ToRfc4716PublicKey(CkString &outStr);
	const char *toRfc4716PublicKey(void);
	// TORFC4716PUBLICKEY_END
	// TOOPENSSHPUBLICKEY_BEGIN
	bool ToOpenSshPublicKey(CkString &outStr);
	const char *toOpenSshPublicKey(void);
	// TOOPENSSHPUBLICKEY_END
	// FROMXML_BEGIN
	bool FromXml(const char *xmlKey);
	// FROMXML_END
	// FROMRFC4716PUBLICKEY_BEGIN
	bool FromRfc4716PublicKey(const char *keyStr);
	// FROMRFC4716PUBLICKEY_END
	// FROMOPENSSHPUBLICKEY_BEGIN
	bool FromOpenSshPublicKey(const char *keyStr);
	// FROMOPENSSHPUBLICKEY_END
	// FROMPUTTYPRIVATEKEY_BEGIN
	bool FromPuttyPrivateKey(const char *keyStr);
	// FROMPUTTYPRIVATEKEY_END
	// FROMOPENSSHPRIVATEKEY_BEGIN
	bool FromOpenSshPrivateKey(const char *keyStr);
	// FROMOPENSSHPRIVATEKEY_END
	// GENERATERSAKEY_BEGIN
	bool GenerateRsaKey(int numBits, int exponent);
	// GENERATERSAKEY_END
	// GENERATEDSAKEY_BEGIN
	bool GenerateDsaKey(int numBits);
	// GENERATEDSAKEY_END
	// SAVETEXT_BEGIN
	bool SaveText(const char *strToSave, const char *filename);
	// SAVETEXT_END
	// LOADTEXT_BEGIN
	bool LoadText(const char *filename, CkString &outStr);
	const char *loadText(const char *filename);
	// LOADTEXT_END
	// TOOPENSSHPRIVATEKEY_BEGIN
	bool ToOpenSshPrivateKey(bool bEncrypt, CkString &outStr);
	const char *toOpenSshPrivateKey(bool bEncrypt);
	// TOOPENSSHPRIVATEKEY_END
	// TOPUTTYPRIVATEKEY_BEGIN
	bool ToPuttyPrivateKey(bool bEncrypt, CkString &outStr);
	const char *toPuttyPrivateKey(bool bEncrypt);
	// TOPUTTYPRIVATEKEY_END

	// SSHKEY_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)


#endif


