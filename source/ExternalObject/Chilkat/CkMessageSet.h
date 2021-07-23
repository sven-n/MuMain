// CkMessageSet.h: interface for the (IMAP) CkMessageSet class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKMESSAGESET_H
#define _CKMESSAGESET_H

#pragma once

class CkByteData;
#include "CkString.h"
#include "CkObject.h"

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#pragma pack (push, 8) 

// Contains a set of IMAP message IDs.  The message set can contain either
// IMAP sequence numbers, or IMAP UIDs.
// CLASS: CkMessageSet
class CkMessageSet  : public CkObject
{
    public:
	CkMessageSet();
	virtual ~CkMessageSet();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	// Return the complete message set serialized to a string.
	void ToString(CkString &str);
	void ToCompactString(CkString &str);

	// Return true if the message set contains the id.
	bool ContainsId(long id);

	// Remove an id from the message set.
	void RemoveId(long id);

	// Insert an id into the message set (duplicates are not inserted)
	void InsertId(long id);


	// Get the Nth id.  Indexing begins at 0.
	long GetId(long index);

	// Return the number of ids in the set.
	long get_Count(void);

	// Return true if this message set contains UIDs rather than sequence numbers.
	//bool HasUids(void);
	void put_HasUids(bool value);
	bool get_HasUids(void) const;

	CkString m_resultString;
	const char *toString(void);
	const char *toCompactString(void);
	// FROMCOMPACTSTRING_BEGIN
	bool FromCompactString(const char *str);
	// FROMCOMPACTSTRING_END

	// MESSAGESET_INSERT_POINT

	// END PUBLIC INTERFACE

    // For internal use only.
    private:
	void *m_impl;

	bool m_utf8;

	// Don't allow assignment or copying these objects.
	CkMessageSet(const CkMessageSet &);
	CkMessageSet &operator=(const CkMessageSet &);
	CkMessageSet(void *impl);

    public:
	void *getImpl(void) const { return m_impl; } 
	void inject(void *impl);


};
#pragma pack (pop)


#endif


