#ifndef _CkZipEntry_H
#define _CkZipEntry_H
#include "Chilkat_C.h"

HCkZipEntry CkZipEntry_Create(void);
void CkZipEntry_Dispose(HCkZipEntry handle);
void CkZipEntry_getComment(HCkZipEntry handle, HCkString retval);
void CkZipEntry_putComment(HCkZipEntry handle, const char *newVal);
unsigned long CkZipEntry_getCompressedLength(HCkZipEntry handle);
long CkZipEntry_getCompressionLevel(HCkZipEntry handle);
void CkZipEntry_putCompressionLevel(HCkZipEntry handle, long newVal);
long CkZipEntry_getCompressionMethod(HCkZipEntry handle);
void CkZipEntry_putCompressionMethod(HCkZipEntry handle, long newVal);
long CkZipEntry_getEntryID(HCkZipEntry handle);
long CkZipEntry_getEntryType(HCkZipEntry handle);
void CkZipEntry_getFileDateTime(HCkZipEntry handle, SYSTEMTIME *retval);
void CkZipEntry_putFileDateTime(HCkZipEntry handle, SYSTEMTIME *newVal);
void CkZipEntry_getFileName(HCkZipEntry handle, HCkString retval);
void CkZipEntry_putFileName(HCkZipEntry handle, const char *newVal);
BOOL CkZipEntry_getIsDirectory(HCkZipEntry handle);
void CkZipEntry_getLastErrorHtml(HCkZipEntry handle, HCkString retval);
void CkZipEntry_getLastErrorText(HCkZipEntry handle, HCkString retval);
void CkZipEntry_getLastErrorXml(HCkZipEntry handle, HCkString retval);
unsigned long CkZipEntry_getUncompressedLength(HCkZipEntry handle);
BOOL CkZipEntry_getUtf8(HCkZipEntry handle);
void CkZipEntry_putUtf8(HCkZipEntry handle, BOOL newVal);
BOOL CkZipEntry_AppendData(HCkZipEntry handle, HCkByteData bdata);
BOOL CkZipEntry_AppendTextData(HCkZipEntry handle, HCkString sdata);
BOOL CkZipEntry_Copy(HCkZipEntry handle, HCkByteData bdata);
BOOL CkZipEntry_CopyToBase64(HCkZipEntry handle, HCkString str);
BOOL CkZipEntry_CopyToHex(HCkZipEntry handle, HCkString str);
BOOL CkZipEntry_Extract(HCkZipEntry handle, const char *dirPath);
BOOL CkZipEntry_ExtractInto(HCkZipEntry handle, const char *dirPath);
BOOL CkZipEntry_Inflate(HCkZipEntry handle, HCkByteData bdata);
BOOL CkZipEntry_InflateToString(HCkZipEntry handle, BOOL addCR, HCkString str);
BOOL CkZipEntry_InflateToString2(HCkZipEntry handle, HCkString str);
HCkZipEntry CkZipEntry_NextEntry(HCkZipEntry handle);
BOOL CkZipEntry_ReplaceData(HCkZipEntry handle, HCkByteData bdata);
BOOL CkZipEntry_ReplaceStringData(HCkZipEntry handle, HCkString sdata);
BOOL CkZipEntry_SaveLastError(HCkZipEntry handle, const char *filename);
const char *CkZipEntry_comment(HCkZipEntry handle);
const char *CkZipEntry_copyToBase64(HCkZipEntry handle);
const char *CkZipEntry_copyToHex(HCkZipEntry handle);
const char *CkZipEntry_fileName(HCkZipEntry handle);
const char *CkZipEntry_inflateToString(HCkZipEntry handle, BOOL addCR);
const char *CkZipEntry_inflateToString2(HCkZipEntry handle);
const char *CkZipEntry_lastErrorHtml(HCkZipEntry handle);
const char *CkZipEntry_lastErrorText(HCkZipEntry handle);
const char *CkZipEntry_lastErrorXml(HCkZipEntry handle);
#endif
