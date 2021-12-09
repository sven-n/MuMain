// CkPublicKey.h: interface for the CkPublicKey class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _CKPUBLICKEY_H
#define _CKPUBLICKEY_H

#pragma once

class CkString;

#include "CkObject.h"
#include "CkString.h"
#include "CkByteData.h"

#pragma pack (push, 8) 

// CLASS: CkPublicKey
class CkPublicKey : public CkObject 
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkPublicKey &operator=(const CkPublicKey &) { return *this; }

    public:
	void *getImpl(void) const { return m_impl; } 

	CkPublicKey(const CkPublicKey &pubKey);

	CkPublicKey();
	CkPublicKey(void *impl);
	virtual ~CkPublicKey();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	// Load from file:
	bool LoadRsaDerFile(const char *filename);
	bool LoadOpenSslDerFile(const char *filename);
	bool LoadOpenSslPemFile(const char *filename);
	bool LoadXmlFile(const char *filename);

	// Load from memory.
	bool LoadOpenSslPem(const char *str);
	bool LoadOpenSslDer(CkByteData &data);
	bool LoadRsaDer(CkByteData &data);
	bool LoadXml(const char *xml);

	// Save to file:
	bool SaveRsaDerFile(const char *filename);
	bool SaveOpenSslDerFile(const char *filename);
	bool SaveOpenSslPemFile(const char *filename);
	bool SaveXmlFile(const char *filename);

	// Get to memory
	bool GetRsaDer(CkByteData &data) const;
	bool GetOpenSslDer(CkByteData &data) const;
	bool GetOpenSslPem(CkString &str) const;
	bool GetXml(CkString &str) const;

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
	void LastErrorXml(CkString &str);
	void LastErrorHtml(CkString &str);
	void LastErrorText(CkString &str);

	CkString m_resultString;

	const char *lastErrorText(void);
	const char *lastErrorXml(void);
	const char *lastErrorHtml(void);

	const char *getOpenSslPem(void);
	const char *getXml(void);
	// LOADPKCS1PEM_BEGIN
	bool LoadPkcs1Pem(const char *str);
	// LOADPKCS1PEM_END

	// PUBLICKEY_INSERT_POINT

	// END PUBLIC INTERFACE
	

};
#pragma pack (pop)

#endif
