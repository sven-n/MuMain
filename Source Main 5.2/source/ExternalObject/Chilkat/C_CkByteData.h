#ifndef _CkByteData_H
#define _CkByteData_H
#include "Chilkat_C.h"

HCkByteData CkByteData_Create(void);
void CkByteData_Dispose(HCkByteData handle);
BOOL CkByteData_getUtf8(HCkByteData handle);
void CkByteData_putUtf8(HCkByteData handle, BOOL newVal);
void CkByteData_append(HCkByteData handle, const unsigned char *data, unsigned long numBytes);
void CkByteData_append2(HCkByteData handle, const char *byteData, unsigned long numBytes);
void CkByteData_appendChar(HCkByteData handle, char ch);
void CkByteData_appendEncoded(HCkByteData handle, const char *str, const char *encoding);
BOOL CkByteData_appendFile(HCkByteData handle, const char *filename);
void CkByteData_appendInt(HCkByteData handle, int v, BOOL littleEndian);
void CkByteData_appendRange(HCkByteData handle, HCkByteData byteData, unsigned long index, unsigned long numBytes);
void CkByteData_appendShort(HCkByteData handle, short v, BOOL littleEndian);
void CkByteData_appendStr(HCkByteData handle, const char *str);
BOOL CkByteData_beginsWith(HCkByteData handle, HCkByteData byteData);
BOOL CkByteData_beginsWith2(HCkByteData handle, const char *byteData, unsigned long byteDataLen);
void CkByteData_borrowData(HCkByteData handle, unsigned char *yourData, unsigned long numBytes);
void CkByteData_byteSwap4321(HCkByteData handle);
void CkByteData_clear(HCkByteData handle);
const char *CkByteData_computeHash(HCkByteData handle, const char *hashAlg, const char *outputEncoding);
void CkByteData_encode(HCkByteData handle, const char *encoding, HCkString str);
void CkByteData_ensureBuffer(HCkByteData handle, unsigned long numBytes);
int CkByteData_findBytes(HCkByteData handle, HCkByteData byteData);
int CkByteData_findBytes2(HCkByteData handle, const char *byteData, unsigned long byteDataLen);
unsigned char CkByteData_getByte(HCkByteData handle, unsigned long byteIndex);
const unsigned char *CkByteData_getBytes(HCkByteData handle);
char CkByteData_getChar(HCkByteData handle, unsigned long byteIndex);
const unsigned char *CkByteData_getData(HCkByteData handle);
const unsigned char *CkByteData_getDataAt(HCkByteData handle, unsigned long byteIndex);
const char *CkByteData_getEncoded(HCkByteData handle, const char *encoding);
const char *CkByteData_getEncodedRange(HCkByteData handle, const char *encoding, unsigned long index, unsigned long numBytes);
int CkByteData_getInt(HCkByteData handle, unsigned long byteIndex);
const unsigned char *CkByteData_getRange(HCkByteData handle, unsigned long byteIndex, unsigned long numBytes);
const char *CkByteData_getRangeStr(HCkByteData handle, unsigned long byteIndex, unsigned long numAnsiChars);
short CkByteData_getShort(HCkByteData handle, unsigned long byteIndex);
unsigned long CkByteData_getSize(HCkByteData handle);
unsigned int CkByteData_getUInt(HCkByteData handle, unsigned long byteIndex);
unsigned short CkByteData_getUShort(HCkByteData handle, unsigned long byteIndex);
const char *CkByteData_getXmlCharset(HCkByteData handle);
BOOL CkByteData_is7bit(HCkByteData handle);
BOOL CkByteData_loadFile(HCkByteData handle, const char *filename);
void CkByteData_pad(HCkByteData handle, int blockSize, int paddingScheme);
void CkByteData_preAllocate(HCkByteData handle, unsigned long expectedNumBytes);
void CkByteData_removeChunk(HCkByteData handle, unsigned long index, unsigned long numBytes);
unsigned char *CkByteData_removeData(HCkByteData handle);
BOOL CkByteData_saveFile(HCkByteData handle, const char *filename);
void CkByteData_shorten(HCkByteData handle, unsigned long numBytes);
const char *CkByteData_to_s(HCkByteData handle);
void CkByteData_unpad(HCkByteData handle, int blockSize, int paddingScheme);
#endif
