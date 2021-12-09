// CkMailboxes.h: interface for the (IMAP) CkMailboxes class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKMAILBOXES_H
#define _CKMAILBOXES_H

#pragma once

#include "CkString.h"
#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkMailboxes
class CkMailboxes  : public CkObject
{
    private:
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

    public:
	CkMailboxes();
	virtual ~CkMailboxes();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	bool IsMarked(long index);
	bool HasInferiors(long index);
	bool IsSelectable(long index);
	// Return the Nth mailbox name in strName.  Return false if index is out of range.
	bool GetName(long index, CkString &strName);
	long get_Count(void);

	CkString m_resultString;
	const char *getName(long index);


	// CK_INSERT_POINT

	// END PUBLIC INTERFACE


    // For internal use only.
    private:
	void *m_impl;

	// Don't allow assignment or copying these objects.
	CkMailboxes(const CkMailboxes &) { } 
	CkMailboxes &operator=(const CkMailboxes &) { return *this; }
	CkMailboxes(void *impl) : m_impl(impl) { }

    public:
	void *getImpl(void) const { return m_impl; } 



};

#pragma pack (pop)

#endif


