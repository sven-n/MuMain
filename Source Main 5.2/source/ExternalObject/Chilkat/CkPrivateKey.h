// CkPrivateKey.h: interface for the CkPrivateKey class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _CKPRIVATEKEY_H
#define _CKPRIVATEKEY_H

#pragma once

class CkString;

#include "CkObject.h"
#include "CkString.h"
#include "CkByteData.h"

#pragma pack (push, 8) 

// CLASS: CkPrivateKey
class CkPrivateKey : public CkObject 
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkPrivateKey(const CkPrivateKey &) { } 
	CkPrivateKey &operator=(const CkPrivateKey &) { return *this; }

    public:
	void *getImpl(void) const { return m_impl; } 

	CkPrivateKey();
	CkPrivateKey(void *impl);
	virtual ~CkPrivateKey();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	// Load from file:
	bool LoadRsaDerFile(const char *filename);
	bool LoadPkcs8File(const char *filename);
	bool LoadPemFile(const char *filename);
	bool LoadXmlFile(const char *filename);

	// Load from memory.
	bool LoadPem(const char *str);
	bool LoadRsaDer(CkByteData &data);
	bool LoadPkcs8(CkByteData &data);
	bool LoadXml(const char *xml);

#ifdef WIN32
	bool LoadPvk(CkByteData &data, const char *password);
	bool LoadPvkFile(const char *filename, const char *password);
#endif

	// Save to file:
	bool SaveRsaDerFile(const char *filename);
	bool SavePkcs8File(const char *filename);
	bool SaveRsaPemFile(const char *filename);
	bool SavePkcs8PemFile(const char *filename);
	bool SaveXmlFile(const char *filename);

	// Get to memory
	bool GetRsaDer(CkByteData &data);
	bool GetPkcs8(CkByteData &data);
	bool GetRsaPem(CkString &str);
	bool GetPkcs8Pem(CkString &str);
	bool GetXml(CkString &str);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
	void LastErrorXml(CkString &str);
	void LastErrorHtml(CkString &str);
	void LastErrorText(CkString &str);

	CkString m_resultString;

	const char *lastErrorText(void);
	const char *lastErrorXml(void);
	const char *lastErrorHtml(void);

	const char *getRsaPem(void);
	const char *getPkcs8Pem(void);
	const char *getXml(void);
	// LOADENCRYPTEDPEM_BEGIN
	bool LoadEncryptedPem(const char *pemStr, const char *password);
	// LOADENCRYPTEDPEM_END
	// LOADENCRYPTEDPEMFILE_BEGIN
	bool LoadEncryptedPemFile(const char *filename, const char *password);
	// LOADENCRYPTEDPEMFILE_END
	// LOADPKCS8ENCRYPTED_BEGIN
	bool LoadPkcs8Encrypted(CkByteData &data, const char *password);
	// LOADPKCS8ENCRYPTED_END
	// LOADPKCS8ENCRYPTEDFILE_BEGIN
	bool LoadPkcs8EncryptedFile(const char *filename, const char *password);
	// LOADPKCS8ENCRYPTEDFILE_END
	// GETPKCS8ENCRYPTED_BEGIN
	bool GetPkcs8Encrypted(const char *password, CkByteData &outBytes);
	// GETPKCS8ENCRYPTED_END
	// GETPKCS8ENCRYPTEDPEM_BEGIN
	bool GetPkcs8EncryptedPem(const char *password, CkString &outStr);
	const char *getPkcs8EncryptedPem(const char *password);
	// GETPKCS8ENCRYPTEDPEM_END
	// SAVEPKCS8ENCRYPTEDFILE_BEGIN
	bool SavePkcs8EncryptedFile(const char *password, const char *filename);
	// SAVEPKCS8ENCRYPTEDFILE_END
	// SAVEPKCS8ENCRYPTEDPEMFILE_BEGIN
	bool SavePkcs8EncryptedPemFile(const char *password, const char *filename);
	// SAVEPKCS8ENCRYPTEDPEMFILE_END

	// PRIVATEKEY_INSERT_POINT

	// END PUBLIC INTERFACE
	

};
#pragma pack (pop)

#endif
