// CkZipCrc.h: interface for the CkZipCrc class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKZIPCRC_H
#define _CKZIPCRC_H

#pragma once


class CkByteData;
#include "CkString.h"
#include "CkObject.h"

#pragma warning( disable : 4068 )
#pragma unmanaged

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#pragma pack (push, 8) 

// CLASS: CkZipCrc
class CkZipCrc  : public CkObject
{
    public:
	CkZipCrc();
	virtual ~CkZipCrc();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	void ToHex(unsigned long crc, CkString &strHex) const;
	unsigned long FileCrc(const char *filename);
	unsigned long CalculateCrc(CkByteData &byteData);
	void MoreData(CkByteData &byteData);
	unsigned long EndStream(void);
	void BeginStream(void);

	CkString m_resultString;
	const char *toHex(unsigned long crc);

	// CK_INSERT_POINT

	// END PUBLIC INTERFACE


    // For internal use only.
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkZipCrc(const CkZipCrc &) { } 
	CkZipCrc &operator=(const CkZipCrc &) { return *this; }

    public:
	void *getImpl(void) const { return m_impl; } 

	CkZipCrc(void *impl);


};
#pragma pack (pop)


#endif


