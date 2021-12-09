// CkOmaDrm.h: interface for the CkOmaDrm class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKOMADRM_H
#define _CKOMADRM_H

#pragma once

#include "CkString.h"
class CkByteData;
#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkOmaDrm
class CkOmaDrm  : public CkObject
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkOmaDrm(const CkOmaDrm &);
	CkOmaDrm &operator=(const CkOmaDrm &);
	CkOmaDrm(void *impl);

    public:
	void *getImpl(void) const;

	CkOmaDrm();
	virtual ~CkOmaDrm();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	bool UnlockComponent(const char *unlockCode);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);


	bool LoadDcfFile(const char *filename);
	bool LoadDcfData(const CkByteData &data);
	
	void get_Base64Key(CkString &str);
	void put_Base64Key(const char *key);
	
	int get_DrmContentVersion(void);
	
	void get_ContentType(CkString &str);
	void put_ContentType(const char *contentType);
	
	void get_ContentUri(CkString &str);
	void put_ContentUri(const char *contentUri);
	
	void get_Headers(CkString &str);
	void put_Headers(const char *headers);
	
	void get_IV(CkByteData &data);
	void put_IV(const CkByteData &data);
	
	void get_EncryptedData(CkByteData &data);
	void get_DecryptedData(CkByteData &data);
	
	bool SaveDecrypted(const char *filename);
	
	bool GetHeaderField(const char *fieldName, CkString &fieldVal);
	
	void LoadUnencryptedData(const CkByteData &data);
	bool LoadUnencryptedFile(const char *filename);
	
	void SetEncodedIV(const char *encodedIv, const char *encoding);
	
	bool CreateDcfFile(const char *filename);

	CkString m_resultString;
        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);
	const char *base64Key(void);
	const char *contentType(void);
	const char *contentUri(void);
	const char *headers(void);
	const char *getHeaderField(const char *fieldName);

	// CK_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)


#endif


