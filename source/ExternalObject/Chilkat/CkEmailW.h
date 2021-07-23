#ifndef _CkEmailW_H
#define _CkEmailW_H
#pragma once

#include "CkString.h"
#include "CkEmail.h"


#pragma pack (push, 8) 


class CkEmailW : public CkEmail
{
    private:
	unsigned long nextIdxW(void)
	    {
	    m_resultIdxW++;
	    if (m_resultIdxW >= 10)
		{
		m_resultIdxW = 0;
		}
	    return m_resultIdxW;
	    }
	unsigned long m_resultIdxW;
	CkString m_resultStringW[10];

    public:
	CkEmailW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkEmailW() { }
	void put_BodyW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Body(s1.getUtf8());
	    }
	void put_BounceAddressW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_BounceAddress(s1.getUtf8());
	    }
	void put_CharsetW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Charset(s1.getUtf8());
	    }
	void put_FileDistListW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_FileDistList(s1.getUtf8());
	    }
	void put_FromW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_From(s1.getUtf8());
	    }
	void put_FromAddressW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_FromAddress(s1.getUtf8());
	    }
	void put_FromNameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_FromName(s1.getUtf8());
	    }
	void put_MailerW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Mailer(s1.getUtf8());
	    }
	void put_ReplyToW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ReplyTo(s1.getUtf8());
	    }
	void put_SubjectW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Subject(s1.getUtf8());
	    }
	void addAttachmentHeaderW(int index,const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->AddAttachmentHeader(index,sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool addBccW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddBcc(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool addCCW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddCC(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool addDataAttachmentW(const wchar_t *w1,const unsigned char *data,unsigned long dataLen)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddDataAttachment(sTemp1.getUtf8(),data,dataLen);
		}
	bool addDataAttachment2W(const wchar_t *w1,CkByteData &content,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddDataAttachment2(sTemp1.getUtf8(),content,sTemp2.getUtf8());
		}
	bool addFileAttachmentW(const wchar_t *w1,CkString *strContentType)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddFileAttachment(sTemp1.getUtf8(),strContentType);
		}
	bool addFileAttachmentW(const wchar_t *w1,CkString &strContentType)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddFileAttachment(sTemp1.getUtf8(),strContentType);
		}
	bool addFileAttachment2W(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddFileAttachment2(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void addHeaderFieldW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->AddHeaderField(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool addHtmlAlternativeBodyW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddHtmlAlternativeBody(sTemp1.getUtf8());
		}
	bool addMultipleBccW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddMultipleBcc(sTemp1.getUtf8());
		}
	bool addMultipleCCW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddMultipleCC(sTemp1.getUtf8());
		}
	bool addMultipleToW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddMultipleTo(sTemp1.getUtf8());
		}
	bool addPlainTextAlternativeBodyW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddPlainTextAlternativeBody(sTemp1.getUtf8());
		}
	bool addRelatedDataW(const wchar_t *w1,const unsigned char *data,unsigned long dataLen,CkString *strContentId)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddRelatedData(sTemp1.getUtf8(),data,dataLen,strContentId);
		}
	bool addRelatedDataW(const wchar_t *w1,const unsigned char *data,unsigned long dataLen,CkString &strContentId)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddRelatedData(sTemp1.getUtf8(),data,dataLen,strContentId);
		}
	void addRelatedData2W(const unsigned char *data,unsigned long dataLen,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddRelatedData2(data,dataLen,sTemp1.getUtf8());
		}
	bool addRelatedFileW(const wchar_t *w1,CkString *strContentID)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddRelatedFile(sTemp1.getUtf8(),strContentID);
		}
	bool addRelatedFileW(const wchar_t *w1,CkString &strContentID)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddRelatedFile(sTemp1.getUtf8(),strContentID);
		}
	bool addRelatedFile2W(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddRelatedFile2(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool addRelatedStringW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,CkString *cid)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->AddRelatedString(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),cid);
		}
	bool addRelatedStringW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,CkString &cid)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->AddRelatedString(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),cid);
		}
	void addRelatedString2W(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		this->AddRelatedString2(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool addStringAttachmentW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddStringAttachment(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool addStringAttachment2W(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->AddStringAttachment2(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool addToW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddTo(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool aesDecryptW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AesDecrypt(sTemp1.getUtf8());
		}
	bool aesEncryptW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AesEncrypt(sTemp1.getUtf8());
		}
	void appendToBodyW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->AppendToBody(sTemp1.getUtf8());
		}
	bool aspUnpackW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,bool cleanFiles)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->AspUnpack(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),cleanFiles);
		}
	bool aspUnpack2W(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3,bool cleanFiles,CkByteData &outHtml)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->AspUnpack2(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),cleanFiles,outHtml);
		}
	bool bEncodeBytesW(const unsigned char *data,unsigned long dataLen,const wchar_t *w1,CkString &encodedStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->BEncodeBytes(data,dataLen,sTemp1.getUtf8(),encodedStr);
		}
	bool bEncodeStringW(const wchar_t *w1,const wchar_t *w2,CkString &encodedStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->BEncodeString(sTemp1.getUtf8(),sTemp2.getUtf8(),encodedStr);
		}
	bool computeGlobalKeyW(const wchar_t *w1,bool bFold,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ComputeGlobalKey(sTemp1.getUtf8(),bFold,outStr);
		}
	bool createTempMhtW(const wchar_t *w1,CkString &tempMhtFilenameInOut)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->CreateTempMht(sTemp1.getUtf8(),tempMhtFilenameInOut);
		}
	bool getAltHeaderFieldW(int index,const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetAltHeaderField(index,sTemp1.getUtf8(),outStr);
		}
	bool getAttachmentHeaderW(long index,const wchar_t *w1,CkString &fieldValue)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetAttachmentHeader(index,sTemp1.getUtf8(),fieldValue);
		}
	bool getAttachmentStringW(long index,const wchar_t *w1,CkString &str)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetAttachmentString(index,sTemp1.getUtf8(),str);
		}
	bool getAttachmentStringCrLfW(long index,const wchar_t *w1,CkString &strData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetAttachmentStringCrLf(index,sTemp1.getUtf8(),strData);
		}
	bool getDeliveryStatusInfoW(const wchar_t *w1,CkString &fieldValue)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetDeliveryStatusInfo(sTemp1.getUtf8(),fieldValue);
		}
	bool getFileContentW(const wchar_t *w1,CkByteData &bData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetFileContent(sTemp1.getUtf8(),bData);
		}
	bool getHeaderFieldW(const wchar_t *w1,CkString &strFieldData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetHeaderField(sTemp1.getUtf8(),strFieldData);
		}
	bool getMbHeaderFieldW(const wchar_t *w1,CkByteData &fieldData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetMbHeaderField(sTemp1.getUtf8(),fieldData);
		}
	bool getMbHeaderField2W(const wchar_t *w1,const wchar_t *w2,CkByteData &fieldData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->GetMbHeaderField2(sTemp1.getUtf8(),sTemp2.getUtf8(),fieldData);
		}
	bool getMbHtmlBodyW(const wchar_t *w1,CkByteData &data)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetMbHtmlBody(sTemp1.getUtf8(),data);
		}
	bool getMbPlainTextBodyW(const wchar_t *w1,CkByteData &data)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetMbPlainTextBody(sTemp1.getUtf8(),data);
		}
	bool getRelatedStringW(long index,const wchar_t *w1,CkString &strData)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetRelatedString(index,sTemp1.getUtf8(),strData);
		}
	bool getRelatedStringCrLfW(long index,const wchar_t *w1,CkString &str)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetRelatedStringCrLf(index,sTemp1.getUtf8(),str);
		}
	bool getReplaceString2W(const wchar_t *w1,CkString &str)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetReplaceString2(sTemp1.getUtf8(),str);
		}
	bool hasHeaderMatchingW(const wchar_t *w1,const wchar_t *w2,bool caseInsensitive)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->HasHeaderMatching(sTemp1.getUtf8(),sTemp2.getUtf8(),caseInsensitive);
		}
	bool loadEmlW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadEml(sTemp1.getUtf8());
		}
	bool loadXmlW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadXml(sTemp1.getUtf8());
		}
	bool loadXmlStringW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadXmlString(sTemp1.getUtf8());
		}
	bool qEncodeBytesW(const unsigned char *data,unsigned long dataLen,const wchar_t *w1,CkString &encodedStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->QEncodeBytes(data,dataLen,sTemp1.getUtf8(),encodedStr);
		}
	bool qEncodeStringW(const wchar_t *w1,const wchar_t *w2,CkString &encodedStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->QEncodeString(sTemp1.getUtf8(),sTemp2.getUtf8(),encodedStr);
		}
	void removeHeaderFieldW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->RemoveHeaderField(sTemp1.getUtf8());
		}
	bool saveAllAttachmentsW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveAllAttachments(sTemp1.getUtf8());
		}
	bool saveAttachedFileW(long index,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveAttachedFile(index,sTemp1.getUtf8());
		}
	bool saveEmlW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveEml(sTemp1.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool saveRelatedItemW(long index,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveRelatedItem(index,sTemp1.getUtf8());
		}
	bool saveXmlW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveXml(sTemp1.getUtf8());
		}
	bool setAttachmentCharsetW(int index,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetAttachmentCharset(index,sTemp1.getUtf8());
		}
	bool setAttachmentFilenameW(long index,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetAttachmentFilename(index,sTemp1.getUtf8());
		}
	bool setFromMimeTextW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetFromMimeText(sTemp1.getUtf8());
		}
	bool setFromMimeText2W(const wchar_t *w1,int numBytes)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetFromMimeText2(sTemp1.getUtf8(),numBytes);
		}
	bool setFromXmlTextW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetFromXmlText(sTemp1.getUtf8());
		}
	void setHtmlBodyW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->SetHtmlBody(sTemp1.getUtf8());
		}
	bool setMbHtmlBodyW(const wchar_t *w1,const unsigned char *data,unsigned long dataLen)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetMbHtmlBody(sTemp1.getUtf8(),data,dataLen);
		}
	bool setMbPlainTextBodyW(const wchar_t *w1,const unsigned char *data,unsigned long dataLen)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetMbPlainTextBody(sTemp1.getUtf8(),data,dataLen);
		}
	bool setReplacePatternW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SetReplacePattern(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void setTextBodyW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->SetTextBody(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	bool unpackHtmlW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->UnpackHtml(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool zipAttachmentsW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ZipAttachments(sTemp1.getUtf8());
		}
	const wchar_t *addFileAttachmentW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddFileAttachment(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *addRelatedFileW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddRelatedFile(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *addRelatedStringW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		this->AddRelatedString(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *bEncodeBytesW(const unsigned char *data,unsigned long dataLen,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->BEncodeBytes(data,dataLen,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *bEncodeStringW(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->BEncodeString(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *bodyW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Body(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *bounceAddressW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_BounceAddress(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *charsetW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Charset(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *ck_fromW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->get_From(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *computeGlobalKeyW(const wchar_t *w1,bool bFold)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->ComputeGlobalKey(sTemp1.getUtf8(),bFold,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *createTempMhtW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->CreateTempMht(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *encryptedByW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_EncryptedBy(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *fileDistListW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_FileDistList(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *fromAddressW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_FromAddress(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *fromNameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_FromName(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *generateFilenameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GenerateFilename(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getAltHeaderFieldW(int index,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetAltHeaderField(index,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getAlternativeBodyW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetAlternativeBody(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getAlternativeContentTypeW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetAlternativeContentType(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getAttachedMessageFilenameW(int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetAttachedMessageFilename(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getAttachmentContentIDW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetAttachmentContentID(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getAttachmentContentTypeW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetAttachmentContentType(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getAttachmentFilenameW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetAttachmentFilename(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getAttachmentHeaderW(long index,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetAttachmentHeader(index,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getAttachmentStringW(long index,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetAttachmentString(index,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getAttachmentStringCrLfW(long index,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetAttachmentStringCrLf(index,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getBccW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetBcc(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getBccAddrW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetBccAddr(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getBccNameW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetBccName(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getCCW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetCC(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getCcAddrW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetCcAddr(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getCcNameW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetCcName(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getDeliveryStatusInfoW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetDeliveryStatusInfo(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getHeaderFieldW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetHeaderField(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getHeaderFieldNameW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetHeaderFieldName(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getHeaderFieldValueW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetHeaderFieldValue(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getHtmlBodyW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetHtmlBody(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getMimeW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetMime(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getPlainTextBodyW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetPlainTextBody(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getRelatedContentIDW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetRelatedContentID(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getRelatedContentLocationW(int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetRelatedContentLocation(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getRelatedContentTypeW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetRelatedContentType(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getRelatedFilenameW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetRelatedFilename(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getRelatedStringW(long index,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetRelatedString(index,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getRelatedStringCrLfW(long index,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetRelatedStringCrLf(index,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getReplacePatternW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetReplacePattern(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getReplaceStringW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetReplaceString(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getReplaceString2W(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetReplaceString2(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getToW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetTo(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getToAddrW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetToAddr(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getToNameW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetToName(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getXmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetXml(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *headerW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Header(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *languageW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Language(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastErrorHtmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->LastErrorHtml(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastErrorTextW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->LastErrorText(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastErrorXmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->LastErrorXml(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *mailerW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Mailer(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *qEncodeBytesW(const unsigned char *data,unsigned long dataLen,const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->QEncodeBytes(data,dataLen,sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *qEncodeStringW(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->QEncodeString(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *replyToW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ReplyTo(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *signedByW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_SignedBy(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *subjectW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Subject(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *uidlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Uidl(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
