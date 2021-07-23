// CkEmail.h: interface for the CkEmail class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKEMAIL_H
#define _CKEMAIL_H


#pragma once

#include "CkString.h"
#ifdef WIN32
class CkCSP;
#endif
class CkCert;
class CkByteData;
class CkMime;
class CkStringArray;
class CkSpamDefs;
class CkPrivateKey;

#include "CkObject.h"

#if !defined(WIN32) && !defined(WINCE)
#include "SystemTime.h"
#endif

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

    IMPORTANT: A return value of True indicates success, and False indicates failure.

  */

#pragma pack (push, 8) 

// CLASS: CkEmail
class CkEmail : public CkObject
{
    public:

	CkEmail();
	virtual ~CkEmail();

	void AddRelatedData2(const char *data, unsigned long dataLen, const char *fileNameInHtml);
	bool AddRelatedData(const char *nameInHtml, const char *data, unsigned long dataLen, CkString *strContentId);
	bool AddDataAttachment(const char *fileName, const char *data, unsigned long dataLen);
    	bool SetMbPlainTextBody(const char *charset, const char *data, unsigned long dataLen);
    	bool SetMbHtmlBody(const char *charset, const char *data, unsigned long dataLen);
	bool QEncodeBytes(const char *data, unsigned long dataLen, const char *charset, CkString &encodedStr);
	bool BEncodeBytes(const char *data, unsigned long dataLen, const char *charset, CkString &encodedStr);
	const char *qEncodeBytes(const char *data, unsigned long dataLen, const char *charset);
	const char *bEncodeBytes(const char *data, unsigned long dataLen, const char *charset);
	bool UidlEquals(const CkEmail *e);


	// BEGIN PUBLIC INTERFACE

	// Discards the current contents of the email and replaces it with the email passed in as MIME text.
	bool SetFromMimeText(const char *mimeText);
	bool SetFromMimeText2(const char *mimeText, int numBytes);
	bool LoadEml(const char *mimeFilename);
	bool LoadXml(const char *xmlFilename);
	bool SetFromXmlText(const char *xmlStr);

	// Replaces all the attachments of an email with a single Zip file attachment having the filename specified. 
	bool ZipAttachments(const char *zipFilename);

	// Unzips and replaces any Zip file attachments with the expanded contents. As an example, if 
	// an email contained a single Zip file containing 3 GIF image files as an attachment, then after 
	// calling this method the email would contain 3 GIF file attachments, and the Zip attachment 
	// would be gone.
	// If an email contains multiple Zip file attachments, each Zip is expanded and replaced 
	// with the contents. 
	bool UnzipAttachments();

	// Encrypts the email body, all alternative bodies, all message sub-parts and attachments using 
	// 128-bit AES (Rijndael, CBC mode) encryption. To decrypt, you must use the AesDecrypt 
	// method with the same password. 
	// The AesEncrypt/Decrypt methods use symmetric password-based greatly simplify sending and 
	// receiving encrypted emails because certificates and public/private key issues do not have to 
	// be dealt with. However, the sending and receiving applications must both be using Chilkat Mail.
	bool AesEncrypt(const char *password);

	// Decrypts and restores an email message that was previously encrypted using AesEncrypt. 
	// The password must match the password used for encryption.
	bool AesDecrypt(const char *password);

	// Creates a copy of the email object.
	CkEmail *Clone(void);

	// Returns a copy of the CkEmail object with the body and header fields changed so that the 
	// newly created email can be forwarded. After calling CreateForward, simply add new recipients 
	// to the created email, and call CkMailMan.SendEmail. 
	CkEmail *CreateForward(void);

	// Returns a copy of the CkEmail object with the body and header fields changed so that the newly 
	// created email can be sent as a reply. After calling CreateReply, simply prepend additional 
	// information to the body, and call CkMailMan.SendEmail. 
	CkEmail *CreateReply(void);

	// Sets the encryption and digital signature preferences for this email. 
	// The SendSigned and SendEncrypted properties still need to be set to cause an email to be 
	// sent encrypted or signed, but the CkCSP object will control the Cryptographic Service Provider used, 
	// the key container within the CSP to be used, and the encryption/hash algorithms. 
#ifdef WIN32
#ifndef NO_MS_CRYPTO
	bool SetCSP(const CkCSP *csp);
	bool SetCSP(const CkCSP &csp) { return SetCSP(&csp); }
#endif
#endif
	// Returns the certificate associated with a received signed email.
    	CkCert *GetSignedByCert(void); 
	// Returns the certificate associated with a received encrypted email.
	CkCert *GetEncryptedByCert(void); 

	// Gets the cert previously set by SetEncryptCert
	CkCert *GetEncryptCert(void);//

	// Gets the cert previously set by SetSigningCert
	CkCert *GetSigningCert(void);

	// Explicitly sets the certificate to be used for encryption when sending encrypted (S/MIME) emails.
	bool SetEncryptCert(const CkCert *cert);
	bool SetEncryptCert(const CkCert &cert) { return SetEncryptCert(&cert); }
	// Explicitly sets the certificate to be used for signing when sending digitally signed (S/MIME) emails.
	bool SetSigningCert(const CkCert *cert);
	bool SetSigningCert(const CkCert &cert) { return SetSigningCert(&cert); }
	bool SetSigningCert2(const CkCert &cert, CkPrivateKey &key);

	// Convenience method to load the complete contents of a text file into a CkByteData.
	bool GetFileContent(const char *filename, CkByteData &bData);
	CkMime **Decompose(int &numParts);
	// 
	// NOTE: Related items are images, style sheets, or other external resources
	// that are included within a multipart/related HTML email and are referenced
	// from the HTML via CID URLs.
	// 

	// Retrieves a related item's data as a String. All end-of-lines will be translated to CRLF sequences. 
	// Indexing begins at 0.
	bool GetRelatedStringCrLf(long index, const char *charset, CkString &str);

	// Retrieves the contentID of a related item. Indexing begins at 0.
	bool GetRelatedContentID(long index, CkString &strContentID);

	bool GetRelatedContentType(long index, CkString &strContentType);

	// Retrieves a related item's filename. Indexing begins at 0. 
	bool GetRelatedFilename(long index, CkString &strFilename);

	// Retrieves a related item's data as a String. All CRLF sequences will be translated 
	// to single newline characters. Indexing begins at 0.
	bool GetRelatedString(long index, const char *charset, CkString &strData);

	// Retrieves a related item's binary data. Indexing begins at 0.
	bool GetRelatedData(long index, CkByteData &buffer);

	// Any standard or non-standard (custom) header field can be added to the email with this method. 
	// One interesting feature is that all header fields whose name begins with "CKX-" will not be 
	// included in the header when an email is sent. These fields will be included when saved to 
	// or loaded from XML. This makes it easy to include persistent meta-data with an email which 
	// your programs can use in any way it chooses.
	void AddHeaderField(const char *fieldName, const char *fieldValue);

	// Removes a field from the email header. If duplicates of the field exist, all occurances are 
	// removed. Calling AddHeaderField(fieldname,0) also removes the header field.
	void RemoveHeaderField(const char *fieldName);

	// Sets the Body property, and sets the email's content type to "text/html". 
	// Use this method if the body is HTML, and if there are no other alternative bodies.
	void SetHtmlBody(const char *html);

	// Saves a related item to a file in a directory. The NumRelatedItems property contains the 
	// number of related items contained in the email. The function returns True if successful. 
	// Indexing begins at 0.
	bool SaveRelatedItem(long index, const char *directory);

	void DropRelatedItem(long index);
	void DropRelatedItems(void);

	// Adds the contents of a file to the email and returns the Content-ID. 
	// Emails formatted in HTML can include images with this call and internally reference the 
	// image through a "cid" hyperlink.
    	bool AddRelatedFile(const char *fileName, CkString *strContentID);
	bool AddRelatedFile(const char *fileName, CkString &strContentID) { return AddRelatedFile(fileName,&strContentID); }


    	bool AddRelatedFile2(const char *fileNameOnDisk, const char *filenameInHtml);
	void AddRelatedData2(const unsigned char *data, unsigned long dataLen, const char *fileNameInHtml);
	void AddRelatedString2(const char *str, const char *charset, const char *filenameInHtml);

	// Adds the contents of a memory data buffer to the email and returns the Content-ID. 
	// This data buffer would typically contain image data in GIF or JPG format. Although the data 
	// comes from an in-memory data buffer, a fictional filename must be provided to (1) determine the 
	// format of the data, and (2) determine the filename if the related data item was to be saved 
	// to disk (by the email recipient) and HTML reconstructed. 
	bool AddRelatedData(const char *nameInHtml, const unsigned char *data, unsigned long dataLen, CkString *strContentId);
	bool AddRelatedData(const char *nameInHtml, const unsigned char *data, unsigned long dataLen, CkString &strContentId){return AddRelatedData(nameInHtml,data,dataLen,&strContentId);}

	// Sets the HTML body of the email. Use this method if there will be multiple versions of the 
	// body, but in different formats, such as HTML and plain text. Otherwise, set the body by calling 
	// the SetHtmlBody method.  (For non-English emails, the input string should be in the utf-8 character
	// encoding.)
	bool AddHtmlAlternativeBody(const char *body);

	// Sets the plain-text body of the email. Use this method if there will be multiple versions of 
	// the body, but in different formats, such as HTML and plain text. Otherwise, simply set the 
	// Body property (i.e. put_Body method)
	bool AddPlainTextAlternativeBody(const char *body);

	// Returns the Nth alternative body. The NumAlternatives property tells the number of 
	// alternative bodies present. Use the GetHtmlBody and GetPlainTextBody methods to easily get 
	// the HTML or plain text alternative bodies. Indexing begins at 0. The text is returned as
	// a utf-8 string.
	bool GetAlternativeBody(long index, CkString &strBody);

	// Returns the content type of the Nth alternative body. The NumAlternatives property tells 
	// the number of alternative bodies present. Indexing begins at 0.
	bool GetAlternativeContentType( long index,  CkString &strContentType);

	// Returns the body that has the "text/html" content type. The HTML is returned as a utf-8 string
	bool GetHtmlBody(CkString &strBody);

	// Returns the body that has the "text/plain" content type. The text is returned as a utf-8 string.
	bool GetPlainTextBody(CkString &strBody);

	void SetTextBody(const char *bodyText, const char *contentType);

	// Return the name of the Nth header field. The NumHeaderFields() method can be used to get the 
	// number of header fields. The GetHeaderField() method can be used to get the value of the 
	// field given the field name. Indexing begins at 0.
	long get_NumHeaderFields(void);
	bool GetHeaderFieldName(long index, CkString &strFieldName);
	bool GetHeaderFieldValue(long index, CkString &strFieldValue);

	// Returns the number of header fields in this email.
	// use get_NumHeaderFields.
	//long NumHeaderFields(void);

	// Retrieves an attachment's data as a string. 
	// All end-of-lines will be translated to CRLF sequences. Indexing begins at 0. 
	bool GetAttachmentStringCrLf(long index, const char *charset, CkString &strData);

	// Retrieves the contentID of an attachment. Indexing begins at 0.
	bool GetAttachmentContentID(long index, CkString &strContentID);

	// Retrieves the content type of an attachment. Indexing begins at 0.
	bool GetAttachmentContentType(long index, CkString &strContentType);

	// Returns one of the header fields for an attachment.
	bool GetAttachmentHeader(long index, const char *fieldName, CkString &fieldValue);

	// Retrieves an attachment's size in bytes. Indexing begins at 0.
	long GetAttachmentSize(long index);

	// Retrieves an attachment's data as a string. All CRLF sequences will be translated to 
	// single newline characters. Indexing begins at 0.
	bool GetAttachmentString(long index, const char *charset, CkString &str);

	// Save all of this email's attachments to a directory. Return True if successful. 
	bool SaveAllAttachments(const char *directory);

	// Save one of the email's attachments to a directory. Return True if successful. Indexing begins at 0.
	bool SaveAttachedFile(long index, const char *directory);

	// Retrieves an attachment's binary data. Indexing begins at 0.
	bool GetAttachmentData(long index, CkByteData &buffer);

	// Drops a single attachment from the email. Returns True if successful. Indexing begins at 0.
	bool DropSingleAttachment(long index);

	// Change the filename of one of the email attachments. Return True if successful. Indexing begins at 0.
	bool SetAttachmentFilename(long index, const char *filename);

	// Retrieves an attachment's filename. Indexing begins at 0
	bool GetAttachmentFilename(long index, CkString &strFilename);

	void AddAttachmentHeader(int index, const char *fieldName, const char *fieldValue);

	// Removes all attachments from the email.
	void DropAttachments(void);

	// Adds a file as an attachment to the email. Returns the MIME content-type of the attachment, 
	// which is inferred based on the filename extension.
	// Returns true for success and false for failure.
	bool AddFileAttachment(const char *fileName, CkString *strContentType);
	bool AddFileAttachment(const char *fileName, CkString &strContentType){return AddFileAttachment(fileName,&strContentType);}

	// Same as AddFileAttachment, but the content type can be explicitly specified
	bool AddFileAttachment2(const char *fileName, const char *contentType);

	// Adds a memory buffer as an attachment to the email. The fileName is passed in as the name 
	// of the file that would get created if the recipient saves the attachment to disk. 
	// Returns true for success and false for failure.
	bool AddDataAttachment(const char *fileName, const unsigned char *data, unsigned long dataLen);

	// New as of 19-Aug-2006
	bool AddStringAttachment(const char *fileName, const char *str);
	bool AddStringAttachment2(const char *fileName, const char *str, const char *charset);
	bool AddRelatedString(const char *nameInHtml, const char *str, const char *charset, CkString *cid);
	bool AddRelatedString(const char *nameInHtml, const char *str, const char *charset, CkString &cid){return AddRelatedString(nameInHtml,str,charset,&cid);}

	/*

	    Mail Merge Features

	    Any number of replacement pattern/string pairs can be defined by calling SetReplacePattern
	    multiple times.  When an email is sent, each replacement pattern is replaced by its corresponding
	    replacement string.

	*/
	long get_NumReplacePatterns(void);

	// Gets one of the replacement strings, if any replacment patterns have been defined. This is a mail-merge feature.
	bool GetReplaceString2(const char *pattern, CkString &str);

	// Gets one of the replacement strings, if any replacment patterns have been defined. This is a mail-merge feature. Indexing begins at 0.
	bool GetReplaceString(long index, CkString &str);

	// Gets one of the replacement patterns, if any have been defined. This is a mail-merge feature. Indexing begins at 0.
	bool GetReplacePattern(long index, CkString &strPattern);

	// Define a replacement pattern. When the mailman sends this email, it will first find all occurances of each pattern 
	// and substitute the replacment string. 
	bool SetReplacePattern(const char *pattern, const char *replaceString);

	// Generates a unique filename for this email. The filename will be different each time the method is called.
	void GenerateFilename(CkString &strFilename);

	// Adds multiple recipients to the blind carbon-copy, carbon-copy, or "to" recipient list. 
	// The parameter is a string containing a comma separated list of full email addresses. 
	// For example: "Chilkat Support" <support@chilkatsoft.com>, "Chilkat Sales" <sales@chilkatsoft.com>
	// Returns True if successful.
	bool AddMultipleBcc(const char *commaSeparatedAddresses);
	bool AddMultipleCC(const char *commaSeparatedAddresses);
	bool AddMultipleTo(const char *commaSeparatedAddresses);

	// Retrieves the value of a header field as a utf-8 string. If the header field is B or Q encoded, 
	// it is automatically decoded, converted to utf-8, and returned.
	bool GetHeaderField(const char *fieldName, CkString &strFieldData);

	// Returns a blind carbon-copy, carbon-copy, or "to" recipient. Indexing begins at 0.
	bool GetBcc(long index, CkString &str);
	bool GetCC(long index, CkString &str);
	bool GetTo(long index, CkString &str);

	bool GetBccAddr(long index, CkString &str);
	bool GetCcAddr(long index, CkString &str);
	bool GetToAddr(long index, CkString &str);

	bool GetBccName(long index, CkString &str);
	bool GetCcName(long index, CkString &str);
	bool GetToName(long index, CkString &str);

	// Clears the list of blind carbon-copy, carbon-copy, or "to" recipients.
	void ClearBcc(void);
	void ClearCC(void);
	void ClearTo(void);

	// Adds a recipient to the blind carbon-copy list. address is required, but name may be empty. 
	// Returns True if successful.
	bool AddBcc(const char *friendlyName, const char *emailAddress);
	// Adds a recipient to the carbon-copy list. address is required, but name may be empty. 
	// Returns True if successful.
	bool AddCC(const char *friendlyName, const char *emailAddress);
	// Adds a recipient to the "to" list. address is required, but name may be empty. 
	// Returns True if successful.
	bool AddTo(const char *friendlyName, const char *emailAddress);

	// Returns the email as MIME text.
	void GetMime(CkString &strMime);
	// Returns the email converted to XML.
	void GetXml(CkString &strXml);
	// Saves the email in XML format to a file.
	bool SaveXml(const char *filename);
	// Saves the email in MIME format to a file.
	bool SaveEml(const char *filename);

	// Emails received from a POP3 server will typically have a UIDL, but an email
	// created dynamically by a program will not.  (POP3 servers add UIDLs to emails.)
	void get_Uidl(CkString &str);

	// Set this property to True for a return-receipt.
	bool get_ReturnReceipt(void);
	void put_ReturnReceipt(bool newVal);

	// Size in bytes of the email, including attachments.
	long get_Size(void);

	// Number of multipart/alternatives.  (Such as HTML and/or plain-text.)
	long get_NumAlternatives(void);

	// For HTML emails, the number of related items including images, style sheets, etc. included in the email.
	long get_NumRelatedItems(void);

	// Set to true to send this email with S/MIME encryption.
	bool get_SendEncrypted(void);
	void put_SendEncrypted(bool newVal);

	// Set this property to send an email to a list of recipients stored in a 
	// plain text file. The file format is simple: one recipient per line, no comments allowed, 
	// blank lines are ignored. There is no need to add "To", "CC", or "Bcc" recipients if this 
	// property is set.

	// Setting this property is equivalent to adding a "CKX-FileDistList" header field to the email. 
	// Chilkat Mail treats header fields beginning with "CKX-" specially in that these fields are never 
	// transmitted with the email when sent. However, CKX fields are saved and restored when saving to 
	// XML or loading from XML (or MIME). 

	// When sending an email containing a "CKX-FileDistList" header field, Chilkat Mail will read the 
	// distribution list file and send the email to each recipient. Emails can be sent individually, 
	// or with BCC, 100 recipients at a time. (see the CkMailMan.SendIndividual property). 
	void get_FileDistList(CkString &str);
	void put_FileDistList(const char *str);

	// Set this property to send an email to an Outlook distribution list. The string format is to 
	// include the folder path and distribution list name such as "Personal Folders/Contacts/My List". 
	// Forward slashes should be used instead of backslashes. There is no need to add "To", "CC", or 
	// "Bcc" recipients if this property is set.

	void get_Charset(CkString &str);
	void put_Charset(const char *str);

	// Controls whether files are overwritten when saving attachments.
	bool get_OverwriteExisting(void);
	void put_OverwriteExisting(bool newVal);

	// Set this property to true to send a digitally signed email. If this property and 
	// SendEncrypted are both true, the email will be sent both encrypted and signed
	bool get_SendSigned(void);
	void put_SendSigned(bool newVal);

	// If the email was received in encrypted format, this property contains the details of the 
	// certificate used to encrypt the email. 
	void get_EncryptedBy(CkString &str);

	// If the email was received with encrypted parts, either in whole or part, this property is TRUE if all encrypted 
	// parts were successfully decrypted. (Chilkat Mail automatically decrypts messages when they are read from a POP3 server.) 
	bool get_Decrypted(void);
	
	// If the email was received with a digital signature, this property is true if all signatures 
	// were verified to be valid. (Chilkat Mail automatically verifies digital signatures when an 
	// email is read from a POP3 server.) 
	bool get_SignaturesValid(void);

	// If the email was digitally signed, this property contains the details of the signer.
	void get_SignedBy(CkString &str);

	// True if this email was originally received with an attached digital signature.
	bool get_ReceivedSigned(void);

	// True if this email was originally received with S/MIME encryption.
	bool get_ReceivedEncrypted(void);

	// Number of file attachments.
	long get_NumAttachments(void) const;

	// IMPORTANT: Unless otherwise noted, strings are always passed/returned in the utf-8 character encoding.
	void get_FromAddress(CkString &str);
	void put_FromAddress(const char *str);

	// IMPORTANT: Unless otherwise noted, strings are always passed/returned in the utf-8 character encoding.
	void get_FromName(CkString &str);
	void put_FromName(const char *str);

	// The date converted to local time.
	// SYSTEMTIME is defined in <windows.h>
	// For more information, Google "SYSTEMTIME"
	void get_LocalDate(SYSTEMTIME &sysTime);
	// The date in GMT time.
	void get_EmailDate(SYSTEMTIME &sysTime);

	void put_LocalDate(SYSTEMTIME &sysTime);
	// The date in GMT time.
	void put_EmailDate(SYSTEMTIME &sysTime);

	// IMPORTANT: Unless otherwise noted, strings are always passed/returned in the utf-8 character encoding.
	void get_Mailer(CkString &str);
	void put_Mailer(const char *str);

	// The raw email header.
	void get_Header(CkString &str);

	long get_NumBcc(void);
	long get_NumCC(void);
	long get_NumTo(void);

	// The full from name + address.
	void get_From(CkString &str);
	void put_From(const char *str);

	// IMPORTANT: Unless otherwise noted, strings are always passed/returned in the utf-8 character encoding.
	// If the header field was B or Q encoded, it is automatically decoded before being returned.
	// When setting the subject, the input string may be utf-8 or optionally B- or Q-encoded.
	void get_Subject(CkString &str);
	void put_Subject(const char *str);

	// IMPORTANT: Unless otherwise noted, strings are always passed/returned in the utf-8 character encoding.
	void get_ReplyTo(CkString &str);
	void put_ReplyTo(const char *str);

	// IMPORTANT: Unless otherwise noted, strings are always passed/returned in the utf-8 character encoding.
	void get_BounceAddress(CkString &str);
	void put_BounceAddress(const char *str);

	// The default email body.  
	void get_Body(CkString &str);
	void put_Body(const char *str);

	// True if the caller email has a UIDL that equals the email passed in the parameter.
	bool UidlEquals(const CkEmail &e) { return UidlEquals(&e); }

	// Saves the email to a temporary MHT file so that a WebBrowser control can navigate to it and 
	// display it. If fileName is empty, a temporary filename is generated and this filename is returned.
	// If fileName is non-empty, then that file will be created or overwritten, and the input filename 
	// is simply returned.
	// The MHT file that is created will not contain any of the email's attachments, if any existed. 
	// Also, if the email was plain-text, the MHT file will be saved such that the plain-text is 
	// converted to HTML using pre-formatted text (<pre> HTML tags) allowing it to be displayed correctly
	// in a WebBrowser.
	bool CreateTempMht(const char *inFilename, CkString &tempMhtFilenameInOut);
	CkMime *GetMimeObject(void);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);


	// Sets the plain-text email body from a byte array containing character data in the 
	// specified character set. This method also updates the email "content-type" header to properly 
	// reflect the content type of the body.
    	bool SetMbPlainTextBody(const char *charset, const unsigned char *data, unsigned long dataLen);

	// Sets the HTML email body from a byte array containing character data in the specified 
	// character set. This method also updates the email "content-type" header to properly reflect 
	// the content type of the body. 
    	bool SetMbHtmlBody(const char *charset, const unsigned char *data, unsigned long dataLen);

	// Returns the plain-text body converted to a specified charset. 
	// The return value is a byte array containing multibyte character data.
	bool GetMbPlainTextBody(const char *charset, CkByteData &data);

	// Returns the HTML body converted to a specified charset. If no HTML body exists, the returned byte 
	// array is empty. The returned data will be such that not only is the character data converted 
	// (if necessary) to the convertToCharset, but the HTML is edited to add or modify the META tag 
	// that specifies the charset within the HTML.
	bool GetMbHtmlBody(const char *charset, CkByteData &data);

	// Returns a header field's raw data in a byte array. If the field was Q or B encoded, this is 
	// automatically decoded, and the raw bytes of the field are returned. Call GetHeaderField to 
	// retrieve the header field as a utf-8 string.
	bool GetMbHeaderField(const char *fieldName, CkByteData &fieldData);

	// Same as GetMbHeaderField, except the field value is converted to the charset specified.
	// If conversion is not possible, then utf-8 is returned and the method returns false.
	bool GetMbHeaderField2(const char *charset, const char *fieldName, CkByteData &fieldData);

	// Takes a byte array of multibyte character data having the character encoding specified by the charset parameter, 
	// and returns a us-ascii Q-Encoded string. 
	bool QEncodeBytes(const unsigned char *data, unsigned long dataLen, const char *charset, CkString &encodedStr);
	// Takes a byte array of multibyte character data having the character encoding specified by the charset parameter, 
	// and returns a us-ascii B-Encoded string. 
	bool BEncodeBytes(const unsigned char *data, unsigned long dataLen, const char *charset, CkString &encodedStr);
	// Takes a utf-8 string, converts it to the charset specified in the 2nd parameter, B-Encodes the converted multibyte data, 
	// and returns the Q-encoded us-ascii string.
	bool QEncodeString(const char *str, const char *charset, CkString &encodedStr);
	// Takes a utf-8 string, converts it to the charset specified in the 2nd parameter, B-Encodes the converted multibyte data, 
	// and returns the B-encoded us-ascii string.
	bool BEncodeString(const char *str, const char *charset, CkString &encodedStr);

	// This, or CkMailMan::UnlockComponent, should be called once at the beginning of the program.  
	bool UnlockComponent(const char *unlockCode);

	// Number of embedded email messages attached to this email.
	long get_NumAttachedMessages(void);

	// Retrieves one of the emails (if any) attached to the calling email object. 
	// Indexing begins at 0. Returns the attached email as a full email object which may contain 
	// additional attached emails.
	CkEmail *GetAttachedMessage(long index);

	// Remove all attached messages.
	void RemoveAttachedMessages(void);
	// Remove the Nth attached message.
	void RemoveAttachedMessage(long index);

	// Return the linked domains found in the email.
	void GetLinkedDomains(CkStringArray &array);

	// Un-obfuscate what spammers do to obfuscate HTML emails.
	void UnSpamify(void);

	// Return true if this email is a multipart/report type email.  Multipart/report emails are
	// typically bounced email notifications.
	bool IsMultipartReport(void);

	// Only call this if the email is a multipart/report
	// Field name can be:
	// "Final-Recipient", "Action", "Status", "X-Supplementary-Info", etc.
	bool GetDeliveryStatusInfo(const char *fieldName, CkString &fieldValue);
	const char *getDeliveryStatusInfo(const char *fieldName);

	// Return true if this email has an HTML body.
	bool HasHtmlBody(void);

	// Return true if this email has a plain-text body.
	bool HasPlainTextBody(void);

	// Removes the path information from attachment filenames so that 
	// the attachments are simply a filename with extension.
	// Example:  c:\myDirectory\myAttachment.txt  -->  myAttachment.txt
	void RemoveAttachmentPaths(void);



	bool UnpackHtml(const char *unpackDir, const char *htmlFilename, const char *partsSubdir);


	bool get_Utf8(void) const;
	void put_Utf8(bool b);

        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);

	const char *getRelatedStringCrLf(long index, const char *charset);
	const char *getRelatedContentID(long index);
	const char *getRelatedContentType(long index);
	const char *getRelatedFilename(long index);
	const char *getRelatedString(long index, const char *charset);
	const char *getAlternativeBody(long index);
	const char *getAlternativeContentType(long index);
	const char *getHtmlBody(void);
	const char *getPlainTextBody(void);
	const char *getHeaderFieldName(long index);
	const char *getHeaderFieldValue(long index);
	const char *getAttachmentStringCrLf(long index, const char *charset);
	const char *getAttachmentContentID(long index);
	const char *getAttachmentContentType(long index);
	const char *getAttachmentHeader(long index, const char *fieldName);
	const char *getAttachmentString(long index, const char *charset);
	const char *getAttachmentFilename(long index);
	const char *getHeaderField(const char *fieldName);
	const char *getBcc(long index);
	const char *getCC(long index);
	const char *getTo(long index);
	const char *getBccAddr(long index);
	const char *getCcAddr(long index);
	const char *getToAddr(long index);
	const char *getBccName(long index);
	const char *getCcName(long index);
	const char *getToName(long index);
	const char *getMime(void);
	const char *getXml(void);
	const char *uidl(void);
	const char *charset(void);
	const char *encryptedBy(void);
	const char *signedBy(void);
	const char *fromAddress(void);
	const char *fromName(void);
	const char *mailer(void);
	const char *header(void);
	const char *ck_from(void);
	const char *subject(void);
	const char *replyTo(void);
	const char *bounceAddress(void);
	const char *body(void);
	const char *qEncodeString(const char *str, const char *charset);
	const char *bEncodeString(const char *str, const char *charset);

	const char *getReplaceString2(const char *pattern);
	const char *getReplaceString(long index);
	const char *getReplacePattern(long index);
	const char *generateFilename(void);
	const char *fileDistList(void);
	const char *createTempMht(const char *inFilename);
	const char *qEncodeBytes(const unsigned char *data, unsigned long dataLen, const char *charset);
	const char *bEncodeBytes(const unsigned char *data, unsigned long dataLen, const char *charset);

	const char *addFileAttachment(const char *fileName);
	const char *addRelatedFile(const char *fileName);
	const char *addRelatedString(const char *nameInHtml, const char *str, const char *charset);

	const char *getAltHeaderField(int index, const char *fieldName);
	const char *getAttachedMessageFilename(int index);


	// ADDDATAATTACHMENT2_BEGIN
	bool AddDataAttachment2(const char *fileName, CkByteData &content, const char *contentType);
	// ADDDATAATTACHMENT2_END
	// APPENDTOBODY_BEGIN
	void AppendToBody(const char *str);
	// APPENDTOBODY_END
	// GETALTHEADERFIELD_BEGIN
	bool GetAltHeaderField(int index, const char *fieldName, CkString &outStr);
	// GETALTHEADERFIELD_END
	// GETATTACHEDMESSAGEFILENAME_BEGIN
	bool GetAttachedMessageFilename(int index, CkString &outStr);
	// GETATTACHEDMESSAGEFILENAME_END
	// LOADXMLSTRING_BEGIN
	bool LoadXmlString(const char *xmlStr);
	// LOADXMLSTRING_END
	// SETATTACHMENTCHARSET_BEGIN
	bool SetAttachmentCharset(int index, const char *charset);
	// SETATTACHMENTCHARSET_END
	// SETFROMMIMEOBJECT_BEGIN
	bool SetFromMimeObject(CkMime &mime);
	// SETFROMMIMEOBJECT_END
	// ASPUNPACK_BEGIN
	bool AspUnpack(const char *prefix, const char *saveDir, const char *urlPath, bool cleanFiles);
	bool AspUnpack2(const char *prefix, const char *saveDir, const char *urlPath, bool cleanFiles, CkByteData &outHtml);
	// ASPUNPACK_END
	// GETIMAPUID_BEGIN
	int GetImapUid(void);
	// GETIMAPUID_END
	// HASHEADERMATCHING_BEGIN
	bool HasHeaderMatching(const char *fieldName, const char *valuePattern, bool caseInsensitive);
	// HASHEADERMATCHING_END
	// LANGUAGE_BEGIN
	void get_Language(CkString &str);
	const char *language(void);
	// LANGUAGE_END
	// GETDSNFINALRECIPIENTS_BEGIN
	CkStringArray *GetDsnFinalRecipients(void);
	// GETDSNFINALRECIPIENTS_END
	// GETRELATEDCONTENTLOCATION_BEGIN
	bool GetRelatedContentLocation(int index, CkString &outStr);
	const char *getRelatedContentLocation(int index);
	// GETRELATEDCONTENTLOCATION_END
	// GETMIMEBINARY_BEGIN
	void GetMimeBinary(CkByteData &outBytes);
	// GETMIMEBINARY_END
	// ATTACHMESSAGE_BEGIN
	bool AttachMessage(CkByteData &mimeBytes);
	// ATTACHMESSAGE_END
	// COMPUTEGLOBALKEY_BEGIN
	bool ComputeGlobalKey(const char *encoding, bool bFold, CkString &outStr);
	const char *computeGlobalKey(const char *encoding, bool bFold);
	// COMPUTEGLOBALKEY_END
	// NUMDAYSOLD_BEGIN
	int get_NumDaysOld(void);
	// NUMDAYSOLD_END
	// UNWRAPSECURITY_BEGIN
	//bool UnwrapSecurity(void);
	// UNWRAPSECURITY_END
	// PREFERREDCHARSET_BEGIN
	void get_PreferredCharset(CkString &str);
	const char *preferredCharset(void);
	void put_PreferredCharset(const char *newVal);
	// PREFERREDCHARSET_END
	// VERBOSELOGGING_BEGIN
	bool get_VerboseLogging(void);
	void put_VerboseLogging(bool newVal);
	// VERBOSELOGGING_END
	// ADDRELATEDHEADER_BEGIN
	void AddRelatedHeader(int index, const char *fieldName, const char *fieldValue);
	// ADDRELATEDHEADER_END
	// TESTFILTER_BEGIN
	bool TestFilter(const char *expression);
	// TESTFILTER_END
	// CLEARENCRYPTCERTS_BEGIN
	void ClearEncryptCerts(void);
	// CLEARENCRYPTCERTS_END
	// ADDENCRYPTCERT_BEGIN
	bool AddEncryptCert(CkCert &cert);
	// ADDENCRYPTCERT_END
	// SETFROMMIMEBYTES_BEGIN
	bool SetFromMimeBytes(CkByteData &mimeBytes);
	// SETFROMMIMEBYTES_END
	// SETATTACHMENTDISPOSITION_BEGIN
	bool SetAttachmentDisposition(int index, const char *disposition);
	// SETATTACHMENTDISPOSITION_END
	// REMOVEHTMLALTERNATIVE_BEGIN
	void RemoveHtmlAlternative(void);
	// REMOVEHTMLALTERNATIVE_END
	// REMOVEPLAINTEXTALTERNATIVE_BEGIN
	void RemovePlainTextAlternative(void);
	// REMOVEPLAINTEXTALTERNATIVE_END
	// ADDHEADERFIELD2_BEGIN
	void AddHeaderField2(const char *fieldName, const char *fieldValue);
	// ADDHEADERFIELD2_END
	// PREPENDHEADERS_BEGIN
	bool get_PrependHeaders(void);
	void put_PrependHeaders(bool newVal);
	// PREPENDHEADERS_END
	// CREATEDSN_BEGIN
	CkEmail *CreateDsn(const char *explanation, const char *xmlDeliveryStatus, bool bHeaderOnly);
	// CREATEDSN_END
	// CREATEMDN_BEGIN
	CkEmail *CreateMdn(const char *explanation, const char *xmlMdnFields, bool bHeaderOnly);
	// CREATEMDN_END
	// SIGNINGHASHALG_BEGIN
	void get_SigningHashAlg(CkString &str);
	const char *signingHashAlg(void);
	void put_SigningHashAlg(const char *newVal);
	// SIGNINGHASHALG_END
	// PKCS7CRYPTALG_BEGIN
	void get_Pkcs7CryptAlg(CkString &str);
	const char *pkcs7CryptAlg(void);
	void put_Pkcs7CryptAlg(const char *newVal);
	// PKCS7CRYPTALG_END
	// PKCS7KEYLENGTH_BEGIN
	int get_Pkcs7KeyLength(void);
	void put_Pkcs7KeyLength(int numBits);
	// PKCS7KEYLENGTH_END

	// EMAIL_INSERT_POINT

	// END PUBLIC INTERFACE



    private:
	// Internal implementation.
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.
	bool m_ownership;
	void *m_impl;

	// Don't allow assignment or copying these objects.
	CkEmail(const CkEmail &) { } 
	CkEmail &operator=(const CkEmail &) { return *this; }
	CkEmail(void *impl) : m_impl(impl) { }

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	// For internal use only.
	void *getImpl(void) const;

	CkEmail(void *impl, bool ownership);



};

#pragma pack (pop)



#endif
