// CkEmailBundle.h: interface for the CkEmailBundle class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKEMAILBUNDLE_H
#define _CKEMAILBUNDLE_H


#pragma once

class CkStringArray;
class CkEmail;
#include "CkObject.h"
#include "CkString.h"

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#pragma pack (push, 8) 

// CLASS: CkEmailBundle
class CkEmailBundle  : public CkObject
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkEmailBundle(const CkEmailBundle &) { } 
	CkEmailBundle &operator=(const CkEmailBundle &) { return *this; }
	CkEmailBundle(void *impl) : m_impl(impl) { }

    public:
	void *getImpl(void) const { return m_impl; } 
	void inject(void *impl);

	CkEmailBundle();
	virtual ~CkEmailBundle();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);


	CkStringArray *GetUidls(void);
	bool RemoveEmail(const CkEmail *email);
	bool RemoveEmail(const CkEmail &email) { return this->RemoveEmail(&email); }
	CkEmail *GetEmail(long index);
	bool AddEmail(const CkEmail *email);
	bool AddEmail(const CkEmail &email) { return this->AddEmail(&email); }

	long get_MessageCount(void);

	void GetXml(CkString &sbXml);
	bool SaveXml(const char *filename);
	
	bool LoadXml(const char *filename);
	bool LoadXmlString(const char *xmlStr);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
        void LastErrorXml(CkString &str);
        void LastErrorHtml(CkString &str);
        void LastErrorText(CkString &str);

	void SortBySubject(bool ascending);
	void SortBySender(bool ascending);
	void SortByRecipient(bool ascending);
	void SortByDate(bool ascending);

	CkString m_resultString;
        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);
	const char *getXml(void);
	// FINDBYHEADER_BEGIN
	CkEmail *FindByHeader(const char *name, const char *value);
	// FINDBYHEADER_END
	// REMOVEEMAILBYINDEX_BEGIN
	bool RemoveEmailByIndex(int index);
	// REMOVEEMAILBYINDEX_END

	// EMAILBUNDLE_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)


#endif
