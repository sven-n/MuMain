// CkIcon.h: interface for the CkIcon class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CkIcon_H
#define _CkIcon_H

#pragma once

#include "CkString.h"
class CkByteData;

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkIcon
class CkIcon  : public CkObject
{
    private:

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkIcon(const CkIcon &) { } 
	CkIcon &operator=(const CkIcon &) { return *this; }
	CkIcon(void *impl) : m_impl(impl) { }

    public:
	void *getImpl(void) const { return m_impl; } 

	CkIcon();
	virtual ~CkIcon();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
	void LastErrorXml(CkString &str);
	void LastErrorHtml(CkString &str);
	void LastErrorText(CkString &str);

    const char *lastErrorText(void);
    const char *lastErrorXml(void);
    const char *lastErrorHtml(void);
    
	// ICON_INSERT_POINT

	// END PUBLIC INTERFACE
		
	CkString m_resultString;
	



};
#pragma pack (pop)


#endif


