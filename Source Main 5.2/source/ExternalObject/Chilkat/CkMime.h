// CkMime.h: interface for the CkMime class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKMIME_H
#define _CKMIME_H

#pragma once

class CkMime;
class CkEmail;
#include "CkString.h"

#ifdef WIN32
#ifndef WINCE
class CkCSP;
#endif
#endif

class CkCert;
class CkPrivateKey;

class CkByteData;
#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkMime
class CkMime  : public CkObject
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkMime(const CkMime &) { } 
	CkMime &operator=(const CkMime &) { return *this; }

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const { return m_impl; } 
	void inject(void *impl);

	CkMime(void *impl);
	CkMime();
	virtual ~CkMime();


	bool AppendPart(const CkMime *mime);

#ifdef WIN32
#ifndef WINCE
	bool SetCSP(const CkCSP *csp);
	bool SetCSP(const CkCSP &csp) { return SetCSP(&csp); }
#endif
#endif
	bool Encrypt(const CkCert *cert);
	bool ConvertToSigned(const CkCert *cert);
	bool AddDetachedSignature(const CkCert *cert);

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);


	void get_Version(CkString &str);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	bool SetBodyFromPlainText(const char *str);
	bool SetHeaderField(const char *name, const char *value);
	bool GetHeaderField(const char *name, CkString &value);

	bool GetHeaderFieldName(int index, CkString &fieldName);
	bool GetHeaderFieldValue(int index, CkString &fieldValue);

	void SetVerifyCert(const CkCert *cert);
	void SetVerifyCert(const CkCert &cert) { SetVerifyCert(&cert); }
	bool LoadMimeFile(const char *fileName);
	bool LoadMimeBytes(CkByteData &bData);
	bool LoadMime(const char *mimeMsg);
	bool SaveMime(const char *filename);

	CkEmail *ToEmailObject(void);

	long get_NumParts(void);
	CkMime *GetPart(long index);
	void get_Protocol(CkString &str);
	void put_Protocol(const char *newVal);
	void get_Micalg(CkString &str);
	void put_Micalg(const char *newVal);
	void get_Boundary(CkString &str);
	void put_Boundary(const char *newVal);
	void get_Name(CkString &str);
	void put_Name(const char *newVal);
	void get_Filename(CkString &str);
	void put_Filename(const char *newVal);
	void get_Charset(CkString &sb);
	void put_Charset(const char *newVal);
	void get_Disposition(CkString &str);
	void put_Disposition(const char *newVal);
	void get_Encoding(CkString &str);
	void put_Encoding(const char *newVal);
	void get_ContentType(CkString &str);
	void put_ContentType(const char *newVal);

	bool UnlockComponent(const char *unlockCode);
	bool IsUnlocked(void) const;

	void ConvertToMultipartMixed(void);
	void ConvertToMultipartAlt(void);

	long get_NumHeaderFields(void);

	bool get_UseMmDescription(void);
	void put_UseMmDescription(bool newVal);
	bool get_UnwrapExtras(void);
	void put_UnwrapExtras(bool newVal);
	long get_NumSignerCerts(void);
	long get_NumEncryptCerts(void);

	bool AppendPart(const CkMime &mime) { return AppendPart(&mime); }
	bool AppendPartFromFile(const char *filename);
	bool NewMultipartMixed();
	bool NewMultipartRelated();
	bool NewMultipartAlternative();
	bool NewMessageRfc822(const CkMime &mimeObject);

	bool SaveBody(const char *filename);
	bool IsMultipart(void);
	bool IsPlainText(void);
	bool IsHtml(void);
	bool IsXml(void);
	bool IsSigned(void);
	bool IsEncrypted(void);
	bool IsMultipartMixed(void);
	bool IsMultipartAlternative(void);
	bool IsMultipartRelated(void);
	bool IsAttachment(void);
	bool IsText(void);
	bool IsApplicationData(void);
	bool IsImage(void);
	bool IsAudio(void);
	bool IsVideo(void);
	bool GetBodyBinary(CkByteData &db);
	bool GetBodyEncoded(CkString &encodedBody);
	bool GetBodyDecoded(CkString &decodedBody);
	bool SetBodyFromEncoded(const char *encoding, const char *str);
	bool SetBodyFromHtml(const char *str);
	bool SetBodyFromXml(const char *str);
	bool SetBodyFromBinary(const CkByteData &dbuf);
	bool SetBodyFromFile(const char *fileName);

	bool Encrypt(const CkCert &cert) { return Encrypt(&cert); }
	bool ConvertToSigned(const CkCert &cert) { return ConvertToSigned(&cert); }
	bool AddDetachedSignature(const CkCert &cert) { return AddDetachedSignature(&cert); }
	CkCert *GetEncryptCert(long index);
	CkCert *GetSignerCert(long index);
	bool AddDetachedSignaturePk(const CkCert &cert, const CkPrivateKey &privateKey);
	bool AddDetachedSignaturePk2(const CkCert &cert, const CkPrivateKey &privateKey, bool transferHeaderFields);
	bool ConvertToSignedPk(const CkCert &cert, const CkPrivateKey &privateKey);

	bool Decrypt(void);
	bool Decrypt2(const CkCert &cert, const CkPrivateKey &privateKey);

	// Verify and unwrap the signed MIME, possibly using the verify cert
	// set previously via SetVerifyCert
	bool Verify(void);

	bool AddDetachedSignature2(const CkCert &cert, bool transferHeaderFields);

	// Returns true if the MIME message or any sub-parts
	// are encrypted.
	bool ContainsEncryptedParts(void);
	// Returns true if the MIME message or any sub-parts
	// are signed.
	bool ContainsSignedParts(void);

	bool UnwrapSecurity(void);

	bool SaveXml(const char *filename);
	bool GetMime(CkString &mime);
	bool LoadXml(const char *xml);
	bool GetXml(CkString &xml);
	bool LoadXmlFile(const char *fileName);

	void GetEntireHead(CkString &str);
	void GetEntireBody(CkString &str);

	bool RemovePart(int index);

	bool AddHeaderField(const char *name, const char *value);


	const char *entireHead(void);
	const char *entireBody(void);
	const char *xml(void);
	const char *mime(void);
	const char *bodyEncoded(void);
	const char *bodyDecoded(void);

	const char *charset(void);
	const char *disposition(void);
	const char *encoding(void);
	const char *contentType(void);
	const char *filename(void);
	const char *name(void);
	const char *boundary(void);
	const char *micalg(void);
	const char *protocol(void);

	const char *header(const char *name);
	const char *headerName(int index);
	const char *headerValue(int index);

	const char *getHeaderField(const char *name);
	const char *getHeaderFieldName(int index);
	const char *getHeaderFieldValue(int index);

	const char *getEntireHead(void);
	const char *getEntireBody(void);
	const char *getXml(void);
	const char *getMime(void);
	const char *getBodyEncoded(void);
	const char *getBodyDecoded(void);

        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);

	// CONVERT8BIT_BEGIN
	void Convert8Bit(void);
	// CONVERT8BIT_END
	// GETMIMEBYTES_BEGIN
	bool GetMimeBytes(CkByteData &outBytes);
	// GETMIMEBYTES_END
	// EXTRACTPARTSTOFILES_BEGIN
#ifndef MOBILE_MIME
	CkStringArray *ExtractPartsToFiles(const char *dirPath);
#endif
	// EXTRACTPARTSTOFILES_END
	// ASNBODYTOXML_BEGIN
	bool AsnBodyToXml(CkString &outStr);
	const char *asnBodyToXml(void);
	// ASNBODYTOXML_END
	// CURRENTDATETIME_BEGIN
	void get_CurrentDateTime(CkString &str);
	const char *currentDateTime(void);
	// CURRENTDATETIME_END
	// ENCRYPTN_BEGIN
	bool EncryptN(void);
	// ENCRYPTN_END
	// ADDENCRYPTCERT_BEGIN
	bool AddEncryptCert(CkCert &cert);
	// ADDENCRYPTCERT_END
	// CLEARENCRYPTCERTS_BEGIN
	void ClearEncryptCerts(void);
	// CLEARENCRYPTCERTS_END
	// SETBODY_BEGIN
	void SetBody(const char *str);
	// SETBODY_END
	// URLENCODEBODY_BEGIN
	void UrlEncodeBody(const char *charset);
	// URLENCODEBODY_END
	// ADDCONTENTLENGTH_BEGIN
	void AddContentLength(void);
	// ADDCONTENTLENGTH_END
	// VERBOSELOGGING_BEGIN
	bool get_VerboseLogging(void);
	void put_VerboseLogging(bool newVal);
	// VERBOSELOGGING_END
	// DECRYPTUSINGPFXFILE_BEGIN
	bool DecryptUsingPfxFile(const char *pfxFilePath, const char *password);
	// DECRYPTUSINGPFXFILE_END
	// DECRYPTUSINGPFXDATA_BEGIN
	bool DecryptUsingPfxData(CkByteData &pfxData, const char *password);
	// DECRYPTUSINGPFXDATA_END
	// ADDPFXSOURCEDATA_BEGIN
	bool AddPfxSourceData(CkByteData &pfxData, const char *password);
	// ADDPFXSOURCEDATA_END
	// ADDPFXSOURCEFILE_BEGIN
	bool AddPfxSourceFile(const char *pfxFilePath, const char *password);
	// ADDPFXSOURCEFILE_END
	// PKCS7CRYPTALG_BEGIN
	void get_Pkcs7CryptAlg(CkString &str);
	const char *pkcs7CryptAlg(void);
	void put_Pkcs7CryptAlg(const char *newVal);
	// PKCS7CRYPTALG_END
	// PKCS7KEYLENGTH_BEGIN
	int get_Pkcs7KeyLength(void);
	void put_Pkcs7KeyLength(int numBits);
	// PKCS7KEYLENGTH_END
	// SIGNINGHASHALG_BEGIN
	void get_SigningHashAlg(CkString &str);
	const char *signingHashAlg(void);
	void put_SigningHashAlg(const char *newVal);
	// SIGNINGHASHALG_END
	// GETHEADERFIELDATTRIBUTE_BEGIN
	bool GetHeaderFieldAttribute(const char *name, const char *attrName, CkString &outStr);
	const char *getHeaderFieldAttribute(const char *name, const char *attrName);
	// GETHEADERFIELDATTRIBUTE_END
	bool HasSignatureSigningTime(int index);
	bool GetSignatureSigningTime(int index, SYSTEMTIME &sysTime);
	// REMOVEHEADERFIELD_BEGIN
	void RemoveHeaderField(const char *name, bool bAllOccurances);
	// REMOVEHEADERFIELD_END

	// MIME_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)


#endif


