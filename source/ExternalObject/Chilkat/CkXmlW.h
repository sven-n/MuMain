#ifndef _CkXmlW_H
#define _CkXmlW_H
#pragma once

#include "CkString.h"
#include "CkXml.h"


#pragma pack (push, 8) 


class CkXmlW : public CkXml
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
	CkXmlW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkXmlW() { }
	void put_ContentW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Content(s1.getUtf8());
	    }
	void put_EncodingW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Encoding(s1.getUtf8());
	    }
	void put_TagW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Tag(s1.getUtf8());
	    }
	void accumulateTagContentW(const wchar_t *w1,const wchar_t *w2,CkString &str)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->AccumulateTagContent(sTemp1.getUtf8(),sTemp2.getUtf8(),str);
		}
	bool addAttributeW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->AddAttribute(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool addAttributeIntW(const wchar_t *w1,int value)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AddAttributeInt(sTemp1.getUtf8(),value);
		}
	void addOrUpdateAttributeW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->AddOrUpdateAttribute(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void addOrUpdateAttributeIW(const wchar_t *w1,long value)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddOrUpdateAttributeI(sTemp1.getUtf8(),value);
		}
	void addStyleSheetW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddStyleSheet(sTemp1.getUtf8());
		}
	void addToAttributeW(const wchar_t *w1,long amount)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddToAttribute(sTemp1.getUtf8(),amount);
		}
	void addToChildContentW(const wchar_t *w1,long amount)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->AddToChildContent(sTemp1.getUtf8(),amount);
		}
	bool appendToContentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->AppendToContent(sTemp1.getUtf8());
		}
	bool bEncodeContentW(const wchar_t *w1,const CkByteData &db)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->BEncodeContent(sTemp1.getUtf8(),db);
		}
	bool childContentMatchesW(const wchar_t *w1,const wchar_t *w2,bool caseSensitive)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->ChildContentMatches(sTemp1.getUtf8(),sTemp2.getUtf8(),caseSensitive);
		}
	bool contentMatchesW(const wchar_t *w1,bool caseSensitive)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ContentMatches(sTemp1.getUtf8(),caseSensitive);
		}
	bool decodeEntitiesW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DecodeEntities(sTemp1.getUtf8(),outStr);
		}
	bool decryptContentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->DecryptContent(sTemp1.getUtf8());
		}
	bool encryptContentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->EncryptContent(sTemp1.getUtf8());
		}
	CkXml *extractChildByNameW(const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->ExtractChildByName(sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	CkXml *findChildW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FindChild(sTemp1.getUtf8());
		}
	bool findChild2W(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FindChild2(sTemp1.getUtf8());
		}
	CkXml *findNextRecordW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->FindNextRecord(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	CkXml *findOrAddNewChildW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->FindOrAddNewChild(sTemp1.getUtf8());
		}
	bool getAttrValueW(const wchar_t *w1,CkString &str)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetAttrValue(sTemp1.getUtf8(),str);
		}
	int getAttrValueIntW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetAttrValueInt(sTemp1.getUtf8());
		}
	bool getBinaryContentW(CkByteData &data,bool unzipFlag,bool decryptFlag,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetBinaryContent(data,unzipFlag,decryptFlag,sTemp1.getUtf8());
		}
	bool getChildBoolValueW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetChildBoolValue(sTemp1.getUtf8());
		}
	bool getChildContentW(const wchar_t *w1,CkString &str)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetChildContent(sTemp1.getUtf8(),str);
		}
	CkXml *getChildExactW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->GetChildExact(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool getChildIntValueW(const wchar_t *w1,int &value)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetChildIntValue(sTemp1.getUtf8(),value);
		}
	int getChildIntValueW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetChildIntValue(sTemp1.getUtf8());
		}
	CkXml *getChildWithContentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetChildWithContent(sTemp1.getUtf8());
		}
	CkXml *getChildWithTagW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetChildWithTag(sTemp1.getUtf8());
		}
	CkXml *getNthChildWithTagW(const wchar_t *w1,long n)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetNthChildWithTag(sTemp1.getUtf8(),n);
		}
	bool getNthChildWithTag2W(const wchar_t *w1,long n)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GetNthChildWithTag2(sTemp1.getUtf8(),n);
		}
	bool hasAttrWithValueW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->HasAttrWithValue(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool hasAttributeW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HasAttribute(sTemp1.getUtf8());
		}
	bool hasChildWithContentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HasChildWithContent(sTemp1.getUtf8());
		}
	bool hasChildWithTagW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HasChildWithTag(sTemp1.getUtf8());
		}
	bool hasChildWithTagAndContentW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->HasChildWithTagAndContent(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	CkXml *httpGetW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HttpGet(sTemp1.getUtf8());
		}
	CkXml *httpPostW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->HttpPost(sTemp1.getUtf8());
		}
	bool loadXmlW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadXml(sTemp1.getUtf8());
		}
	bool loadXml2W(const wchar_t *w1,bool autoTrim)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadXml2(sTemp1.getUtf8(),autoTrim);
		}
	bool loadXmlFileW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadXmlFile(sTemp1.getUtf8());
		}
	bool loadXmlFile2W(const wchar_t *w1,bool autoTrim)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->LoadXmlFile2(sTemp1.getUtf8(),autoTrim);
		}
	CkXml *newChildW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->NewChild(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void newChild2W(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->NewChild2(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	CkXml *newChildAfterW(int index,const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->NewChildAfter(index,sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	CkXml *newChildBeforeW(int index,const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->NewChildBefore(index,sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void newChildInt2W(const wchar_t *w1,int value)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->NewChildInt2(sTemp1.getUtf8(),value);
		}
	long numChildrenHavingTagW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->NumChildrenHavingTag(sTemp1.getUtf8());
		}
	bool qEncodeContentW(const wchar_t *w1,const CkByteData &db)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->QEncodeContent(sTemp1.getUtf8(),db);
		}
	bool removeAttributeW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->RemoveAttribute(sTemp1.getUtf8());
		}
	void removeChildW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->RemoveChild(sTemp1.getUtf8());
		}
	void removeChildWithContentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->RemoveChildWithContent(sTemp1.getUtf8());
		}
	bool saveBinaryContentW(const wchar_t *w1,bool unzipFlag,bool decryptFlag,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SaveBinaryContent(sTemp1.getUtf8(),unzipFlag,decryptFlag,sTemp2.getUtf8());
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool saveXmlW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveXml(sTemp1.getUtf8());
		}
	CkXml *searchAllForContentW(const CkXml *after,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SearchAllForContent(after,sTemp1.getUtf8());
		}
	CkXml *searchAllForContentW(const CkXml &after,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SearchAllForContent(after,sTemp1.getUtf8());
		}
	bool searchAllForContent2W(const CkXml *after,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SearchAllForContent2(after,sTemp1.getUtf8());
		}
	bool searchAllForContent2W(const CkXml &after,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SearchAllForContent2(after,sTemp1.getUtf8());
		}
	CkXml *searchForAttributeW(const CkXml *after,const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->SearchForAttribute(after,sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	CkXml *searchForAttributeW(const CkXml &after,const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->SearchForAttribute(after,sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool searchForAttribute2W(const CkXml *after,const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->SearchForAttribute2(after,sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	bool searchForAttribute2W(const CkXml &after,const wchar_t *w1,const wchar_t *w2,const wchar_t *w3)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		CkString sTemp3; sTemp3.appendU(w3);
		return this->SearchForAttribute2(after,sTemp1.getUtf8(),sTemp2.getUtf8(),sTemp3.getUtf8());
		}
	CkXml *searchForContentW(const CkXml *after,const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SearchForContent(after,sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	CkXml *searchForContentW(const CkXml &after,const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SearchForContent(after,sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool searchForContent2W(const CkXml *after,const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SearchForContent2(after,sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool searchForContent2W(const CkXml &after,const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SearchForContent2(after,sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	CkXml *searchForTagW(const CkXml *after,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SearchForTag(after,sTemp1.getUtf8());
		}
	CkXml *searchForTagW(const CkXml &after,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SearchForTag(after,sTemp1.getUtf8());
		}
	bool searchForTag2W(const CkXml *after,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SearchForTag2(after,sTemp1.getUtf8());
		}
	bool searchForTag2W(const CkXml &after,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SearchForTag2(after,sTemp1.getUtf8());
		}
	bool setBinaryContentW(const unsigned char *data,unsigned long dataLen,bool zipFlag,bool encryptFlag,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetBinaryContent(data,dataLen,zipFlag,encryptFlag,sTemp1.getUtf8());
		}
	bool setBinaryContentW(const CkByteData &data,bool zipFlag,bool encryptFlag,const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetBinaryContent(data,zipFlag,encryptFlag,sTemp1.getUtf8());
		}
	bool setBinaryContentFromFileW(const wchar_t *w1,bool zipFlag,bool encryptFlag,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->SetBinaryContentFromFile(sTemp1.getUtf8(),zipFlag,encryptFlag,sTemp2.getUtf8());
		}
	void sortByAttributeW(const wchar_t *w1,long ascending)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->SortByAttribute(sTemp1.getUtf8(),ascending);
		}
	void sortByAttributeIntW(const wchar_t *w1,bool ascending)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->SortByAttributeInt(sTemp1.getUtf8(),ascending);
		}
	void sortRecordsByAttributeW(const wchar_t *w1,const wchar_t *w2,long ascending)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->SortRecordsByAttribute(sTemp1.getUtf8(),sTemp2.getUtf8(),ascending);
		}
	void sortRecordsByContentW(const wchar_t *w1,long ascending)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->SortRecordsByContent(sTemp1.getUtf8(),ascending);
		}
	void sortRecordsByContentIntW(const wchar_t *w1,bool ascending)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->SortRecordsByContentInt(sTemp1.getUtf8(),ascending);
		}
	bool tagContentW(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->TagContent(sTemp1.getUtf8(),outStr);
		}
	bool tagEqualsW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->TagEquals(sTemp1.getUtf8());
		}
	bool updateAttributeW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		return this->UpdateAttribute(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	bool updateAttributeIntW(const wchar_t *w1,int value)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UpdateAttributeInt(sTemp1.getUtf8(),value);
		}
	void updateChildContentW(const wchar_t *w1,const wchar_t *w2)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->UpdateChildContent(sTemp1.getUtf8(),sTemp2.getUtf8());
		}
	void updateChildContentIntW(const wchar_t *w1,int value)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		this->UpdateChildContentInt(sTemp1.getUtf8(),value);
		}
	const wchar_t *accumulateTagContentW(const wchar_t *w1,const wchar_t *w2)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		CkString sTemp2; sTemp2.appendU(w2);
		this->AccumulateTagContent(sTemp1.getUtf8(),sTemp2.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *attrW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetAttrValue(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *attrNameW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetAttributeName(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *attrValueW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetAttributeValue(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *childContentW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetChildContent(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *contentW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Content(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *decodeEntitiesW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->DecodeEntities(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *encodingW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Encoding(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getAttrValueW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetAttrValue(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getAttributeNameW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetAttributeName(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getAttributeValueW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetAttributeValue(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getChildContentW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GetChildContent(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getChildContentByIndexW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetChildContentByIndex(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getChildTagW(long index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetChildTag(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getChildTagByIndexW(int index)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetChildTagByIndex(index,m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *getXmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetXml(m_resultStringW[idx]);
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
	const wchar_t *tagW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Tag(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *tagContentW(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->TagContent(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *versionW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Version(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *xmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GetXml(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
