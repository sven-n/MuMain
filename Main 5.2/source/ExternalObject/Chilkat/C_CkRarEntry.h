#ifndef _CkRarEntry_H
#define _CkRarEntry_H
#include "Chilkat_C.h"

HCkRarEntry CkRarEntry_Create(void);
void CkRarEntry_Dispose(HCkRarEntry handle);
unsigned long CkRarEntry_getCompressedSize(HCkRarEntry handle);
unsigned long CkRarEntry_getCrc(HCkRarEntry handle);
void CkRarEntry_getFilename(HCkRarEntry handle, HCkString retval);
BOOL CkRarEntry_getIsDirectory(HCkRarEntry handle);
BOOL CkRarEntry_getIsReadOnly(HCkRarEntry handle);
void CkRarEntry_getLastErrorHtml(HCkRarEntry handle, HCkString retval);
void CkRarEntry_getLastErrorText(HCkRarEntry handle, HCkString retval);
void CkRarEntry_getLastErrorXml(HCkRarEntry handle, HCkString retval);
void CkRarEntry_getLastModified(HCkRarEntry handle, SYSTEMTIME *retval);
unsigned long CkRarEntry_getUncompressedSize(HCkRarEntry handle);
BOOL CkRarEntry_getUtf8(HCkRarEntry handle);
void CkRarEntry_putUtf8(HCkRarEntry handle, BOOL newVal);
BOOL CkRarEntry_SaveLastError(HCkRarEntry handle, const char *filename);
BOOL CkRarEntry_Unrar(HCkRarEntry handle, const char *dirPath);
const char *CkRarEntry_filename(HCkRarEntry handle);
const char *CkRarEntry_lastErrorHtml(HCkRarEntry handle);
const char *CkRarEntry_lastErrorText(HCkRarEntry handle);
const char *CkRarEntry_lastErrorXml(HCkRarEntry handle);
#endif
