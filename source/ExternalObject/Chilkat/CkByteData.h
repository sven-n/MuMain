// CkByteData.h: interface for the CkByteData class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKBYTEDATA_H
#define _CKBYTEDATA_H

#pragma once

#include "CkObject.h"

#include "CkString.h"

// The CkByteData class is a convenient class for holding a block of
// binary data, or non-null terminated text data.

#pragma pack (push, 8) 

// CLASS: CkByteData
class CkByteData : public CkObject
{
    public:
	CkByteData();
	~CkByteData();

	CkByteData(const CkByteData &);
	CkByteData &operator=(const CkByteData &);

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);


	// Clears the object of data.
	void clear(void);

	// Get the size in bytes.
        unsigned long getSize(void) const;

	// Get a pointer to the data.
        const unsigned char *getData(void) const;
        const unsigned char *getBytes(void) const;
        const unsigned char *getDataAt(unsigned long byteIndex) const;
        const unsigned char *getRange(unsigned long byteIndex, unsigned long numBytes);
        const char *getRangeStr(unsigned long byteIndex, unsigned long numAnsiChars);
	
	unsigned char getByte(unsigned long byteIndex) const;
	char getChar(unsigned long byteIndex) const;
	unsigned int getUInt(unsigned long byteIndex) const;
	int getInt(unsigned long byteIndex) const;
	unsigned short getUShort(unsigned long byteIndex) const;
	short getShort(unsigned long byteIndex) const;

	void appendRandom(int numBytes);

	void appendInt(int v, bool littleEndian);
	void appendShort(short v, bool littleEndian);
	const char *getEncodedRange(const char *encoding, unsigned long index, unsigned long numBytes);
	void appendRange(const CkByteData &byteData, unsigned long index, unsigned long numBytes);
	void ensureBuffer(unsigned long numBytes);
	// Return -1 if not found, otherwise returns the index.
	int findBytes2(const char *byteData, unsigned long byteDataLen);
	int findBytes(const CkByteData &byteData);
	bool beginsWith2(const char *byteData, unsigned long byteDataLen);
	bool beginsWith(const CkByteData &byteData);
	void removeChunk(unsigned long index, unsigned long numBytes);
	void byteSwap4321(void);
	void pad(int blockSize, int paddingScheme);
	void unpad(int blockSize, int paddingScheme);
	bool is7bit(void) const;
	const char *computeHash(const char *hashAlg, const char *outputEncoding);

	// Encoding can be "base64", "quoted-printable", "hex", or "url"
	const char *getEncoded(const char *encoding);

	void replaceChar(unsigned char c, unsigned char replacement);

	// Append more data
        void append(const unsigned char *data, unsigned long numBytes);
	void append(const char *byteData, unsigned long numBytes);

	void appendStr(const char *str);
	void appendChar(char ch);
	void appendCharN(char ch, int numTimes);

	// Encoding can be "base64", "quoted-printable", "hex", or "url"
	void appendEncoded(const char *str, const char *encoding);
	void encode(const char *encoding, CkString &str);

	// Load a complete file into this object.  The contents of this object
	// are automatically cleared before the file is loaded, so the result
	// is a mirror image (in memory) of the bytes in the file.
        bool loadFile(const char *filename);
	bool saveFile(const char *filename);

	// Create a new file, or append to an existing file.
	bool appendFile(const char *filename);

	// Discards the last numBytes of data.
        void shorten(unsigned long numBytes);

	// The CkByteData will use *your* memory buffer, and will not delete
	// it when the object is destructed.  
	void borrowData(unsigned char *yourData, unsigned long numBytes);

	// Removes the data from the CkByteData object.  The caller will receive
	// a pointer to the memory buffer, and is responsible for deleting it.
	// (Example: 
	//	unsigned char *data = byteData.removeData();
	//	... do something with the data....
	//	delete [] data;
	unsigned char *removeData(void);

	void preAllocate(unsigned long expectedNumBytes);

        const char *to_s(void);

	const char *getXmlCharset(void);

	// BYTES_INSERT_POINT

	// END PUBLIC INTERFACE


    private:
	// Internal implementation.
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.
	void *m_pResultString;    // For to_s()

    public:
	// Used internally by Chilkat.
	void *getImpl(void) const;
	const void *getImplC(void) const;
	void setImpl(void *impl);

};

#pragma pack (pop)

#endif
