// CkAtom.h: interface for the CkAtom class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKAtom_H
#define _CKAtom_H

#pragma once

#include "CkString.h"
class CkByteData;

#if !defined(WIN32) && !defined(WINCE)
#include "SystemTime.h"
#endif

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#include "CkObject.h"

class CkAtomProgress;

#pragma pack (push, 8) 

// CLASS: CkAtom
class CkAtom  : public CkObject
{
    private:
	CkAtomProgress *m_callback;

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkAtom(const CkAtom &) { } 
	CkAtom &operator=(const CkAtom &) { return *this; }
	CkAtom(void *impl) : m_impl(impl),m_callback(0) { }

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const { return m_impl; } 

	CkAtom();
	virtual ~CkAtom();


	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);
	
	CkAtomProgress *get_EventCallbackObject(void) const;
	void put_EventCallbackObject(CkAtomProgress *progress);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
	void LastErrorXml(CkString &str);
	void LastErrorHtml(CkString &str);
	void LastErrorText(CkString &str);

    const char *lastErrorText(void);
    const char *lastErrorXml(void);
    const char *lastErrorHtml(void);
    	// NUMENTRIES_BEGIN
	int get_NumEntries(void);
	// NUMENTRIES_END
	// ADDELEMENT_BEGIN
	int AddElement(const char *tag, const char *value);
	// ADDELEMENT_END
	// ADDELEMENTDATE_BEGIN
	int AddElementDate(const char *tag, SYSTEMTIME &dateTime);
	// ADDELEMENTDATE_END
	// ADDELEMENTHTML_BEGIN
	int AddElementHtml(const char *tag, const char *htmlStr);
	// ADDELEMENTHTML_END
	// ADDELEMENTXHTML_BEGIN
	int AddElementXHtml(const char *tag, const char *xmlStr);
	// ADDELEMENTXHTML_END
	// ADDELEMENTXML_BEGIN
	int AddElementXml(const char *tag, const char *xmlStr);
	// ADDELEMENTXML_END
	// ADDENTRY_BEGIN
	void AddEntry(const char *xmlStr);
	// ADDENTRY_END
	// ADDLINK_BEGIN
	void AddLink(const char *rel, const char *href, const char *title, const char *typ);
	// ADDLINK_END
	// ADDPERSON_BEGIN
	void AddPerson(const char *tag, const char *name, const char *uri, const char *email);
	// ADDPERSON_END
	// DELETEELEMENT_BEGIN
	void DeleteElement(const char *tag, int index);
	// DELETEELEMENT_END
	// DELETEELEMENTATTR_BEGIN
	void DeleteElementAttr(const char *tag, int index, const char *attrName);
	// DELETEELEMENTATTR_END
	// DELETEPERSON_BEGIN
	void DeletePerson(const char *tag, int index);
	// DELETEPERSON_END
	// DOWNLOADATOM_BEGIN
	bool DownloadAtom(const char *url);
	// DOWNLOADATOM_END
	// GETELEMENT_BEGIN
	bool GetElement(const char *tag, int index, CkString &outStr);
	const char *getElement(const char *tag, int index);
	// GETELEMENT_END
	// GETELEMENTATTR_BEGIN
	bool GetElementAttr(const char *tag, int index, const char *attrName, CkString &outStr);
	const char *getElementAttr(const char *tag, int index, const char *attrName);
	// GETELEMENTATTR_END
	// GETELEMENTCOUNT_BEGIN
	int GetElementCount(const char *tag);
	// GETELEMENTCOUNT_END
	// GETELEMENTDATE_BEGIN
	bool GetElementDate(const char *tag, int index, SYSTEMTIME &sysTime);
	// GETELEMENTDATE_END
	// GETENTRY_BEGIN
	CkAtom *GetEntry(int index);
	// GETENTRY_END
	// GETLINKHREF_BEGIN
	bool GetLinkHref(const char *relName, CkString &outStr);
	const char *getLinkHref(const char *relName);
	// GETLINKHREF_END
	// GETPERSONINFO_BEGIN
	bool GetPersonInfo(const char *tag, int index, const char *tag2, CkString &outStr);
	const char *getPersonInfo(const char *tag, int index, const char *tag2);
	// GETPERSONINFO_END
	// GETTOPATTR_BEGIN
	bool GetTopAttr(const char *attrName, CkString &outStr);
	const char *getTopAttr(const char *attrName);
	// GETTOPATTR_END
	// HASELEMENT_BEGIN
	bool HasElement(const char *tag);
	// HASELEMENT_END
	// LOADXML_BEGIN
	bool LoadXml(const char *xmlStr);
	// LOADXML_END
	// NEWENTRY_BEGIN
	void NewEntry(void);
	// NEWENTRY_END
	// NEWFEED_BEGIN
	void NewFeed(void);
	// NEWFEED_END
	// SETELEMENTATTR_BEGIN
	void SetElementAttr(const char *tag, int index, const char *attrName, const char *attrValue);
	// SETELEMENTATTR_END
	// SETTOPATTR_BEGIN
	void SetTopAttr(const char *attrName, const char *value);
	// SETTOPATTR_END
	// TOXMLSTRING_BEGIN
	bool ToXmlString(CkString &outStr);
	const char *toXmlString(void);
	// TOXMLSTRING_END
	// UPDATEELEMENT_BEGIN
	void UpdateElement(const char *tag, int index, const char *value);
	// UPDATEELEMENT_END
	// UPDATEELEMENTDATE_BEGIN
	void UpdateElementDate(const char *tag, int index, SYSTEMTIME &dateTime);
	// UPDATEELEMENTDATE_END
	// UPDATEELEMENTHTML_BEGIN
	void UpdateElementHtml(const char *tag, int index, const char *htmlStr);
	// UPDATEELEMENTHTML_END
	// UPDATEELEMENTXHTML_BEGIN
	void UpdateElementXHtml(const char *tag, int index, const char *xmlStr);
	// UPDATEELEMENTXHTML_END
	// UPDATEELEMENTXML_BEGIN
	void UpdateElementXml(const char *tag, int index, const char *xmlStr);
	// UPDATEELEMENTXML_END
	// UPDATEPERSON_BEGIN
	void UpdatePerson(const char *tag, int index, const char *name, const char *uri, const char *email);
	// UPDATEPERSON_END

	// ATOM_INSERT_POINT

	// END PUBLIC INTERFACE

	


};
#pragma pack (pop)


#endif


