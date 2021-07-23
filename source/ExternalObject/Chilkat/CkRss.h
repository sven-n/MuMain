// CkRss.h: interface for the CkRss class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKRss_H
#define _CKRss_H

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

class CkRssProgress;

#pragma pack (push, 8) 

// CLASS: CkRss
class CkRss  : public CkObject
{
    private:
	CkRssProgress *m_callback;

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkRss(const CkRss &) { } 
	CkRss &operator=(const CkRss &) { return *this; }
	CkRss(void *impl) : m_impl(impl),m_callback(0) { }

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const { return m_impl; } 

	CkRss();
	virtual ~CkRss();


	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);
	
	CkRssProgress *get_EventCallbackObject(void) const;
	void put_EventCallbackObject(CkRssProgress *progress);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
	void LastErrorXml(CkString &str);
	void LastErrorHtml(CkString &str);
	void LastErrorText(CkString &str);

    const char *lastErrorText(void);
    const char *lastErrorXml(void);
    const char *lastErrorHtml(void);
    	// NUMCHANNELS_BEGIN
	int get_NumChannels(void);
	// NUMCHANNELS_END
	// NUMITEMS_BEGIN
	int get_NumItems(void);
	// NUMITEMS_END
	// ADDNEWCHANNEL_BEGIN
	CkRss *AddNewChannel(void);
	// ADDNEWCHANNEL_END
	// ADDNEWIMAGE_BEGIN
	CkRss *AddNewImage(void);
	// ADDNEWIMAGE_END
	// ADDNEWITEM_BEGIN
	CkRss *AddNewItem(void);
	// ADDNEWITEM_END
	// DOWNLOADRSS_BEGIN
	bool DownloadRss(const char *url);
	// DOWNLOADRSS_END
	// GETATTR_BEGIN
	bool GetAttr(const char *tag, const char *attrName, CkString &outStr);
	const char *getAttr(const char *tag, const char *attrName);
	// GETATTR_END
	// GETCHANNEL_BEGIN
	CkRss *GetChannel(int index);
	// GETCHANNEL_END
	// GETCOUNT_BEGIN
	int GetCount(const char *tag);
	// GETCOUNT_END
	// GETDATE_BEGIN
	bool GetDate(const char *tag, SYSTEMTIME &sysTime);
	// GETDATE_END
	// GETIMAGE_BEGIN
	CkRss *GetImage(void);
	// GETIMAGE_END
	// GETINT_BEGIN
	int GetInt(const char *tag);
	// GETINT_END
	// GETITEM_BEGIN
	CkRss *GetItem(int index);
	// GETITEM_END
	// GETSTRING_BEGIN
	bool GetString(const char *tag, CkString &outStr);
	const char *getString(const char *tag);
	// GETSTRING_END
	// LOADRSSFILE_BEGIN
	bool LoadRssFile(const char *filename);
	// LOADRSSFILE_END
	// LOADRSSSTRING_BEGIN
	bool LoadRssString(const char *rssString);
	// LOADRSSSTRING_END
	// MGETATTR_BEGIN
	bool MGetAttr(const char *tag, int index, const char *attrName, CkString &outStr);
	const char *mGetAttr(const char *tag, int index, const char *attrName);
	// MGETATTR_END
	// MGETSTRING_BEGIN
	bool MGetString(const char *tag, int index, CkString &outStr);
	const char *mGetString(const char *tag, int index);
	// MGETSTRING_END
	// MSETATTR_BEGIN
	bool MSetAttr(const char *tag, int index, const char *attrName, const char *value);
	// MSETATTR_END
	// MSETSTRING_BEGIN
	bool MSetString(const char *tag, int index, const char *value);
	// MSETSTRING_END
	// NEWRSS_BEGIN
	void NewRss(void);
	// NEWRSS_END
	// REMOVE_BEGIN
	void Remove(const char *tag);
	// REMOVE_END
	// SETATTR_BEGIN
	void SetAttr(const char *tag, const char *attrName, const char *value);
	// SETATTR_END
	// SETDATE_BEGIN
	void SetDate(const char *tag, SYSTEMTIME &d);
	// SETDATE_END
	// SETDATENOW_BEGIN
	void SetDateNow(const char *tag);
	// SETDATENOW_END
	// SETINT_BEGIN
	void SetInt(const char *tag, int value);
	// SETINT_END
	// SETSTRING_BEGIN
	void SetString(const char *tag, const char *value);
	// SETSTRING_END
	// TOXMLSTRING_BEGIN
	bool ToXmlString(CkString &outStr);
	const char *toXmlString(void);
	// TOXMLSTRING_END

	// RSS_INSERT_POINT

	// END PUBLIC INTERFACE

	


};
#pragma pack (pop)


#endif


