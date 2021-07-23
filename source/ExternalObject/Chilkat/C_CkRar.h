#ifndef _CkRar_H
#define _CkRar_H
#include "Chilkat_C.h"

HCkRar CkRar_Create(void);
void CkRar_Dispose(HCkRar handle);
void CkRar_getLastErrorHtml(HCkRar handle, HCkString retval);
void CkRar_getLastErrorText(HCkRar handle, HCkString retval);
void CkRar_getLastErrorXml(HCkRar handle, HCkString retval);
long CkRar_getNumEntries(HCkRar handle);
BOOL CkRar_getUtf8(HCkRar handle);
void CkRar_putUtf8(HCkRar handle, BOOL newVal);
BOOL CkRar_Close(HCkRar handle);
BOOL CkRar_FastOpen(HCkRar handle, const char *filename);
HCkRarEntry CkRar_GetEntryByIndex(HCkRar handle, long index);
HCkRarEntry CkRar_GetEntryByName(HCkRar handle, const char *filename);
BOOL CkRar_Open(HCkRar handle, const char *filename);
BOOL CkRar_SaveLastError(HCkRar handle, const char *filename);
BOOL CkRar_Unrar(HCkRar handle, const char *dirPath);
const char *CkRar_lastErrorHtml(HCkRar handle);
const char *CkRar_lastErrorText(HCkRar handle);
const char *CkRar_lastErrorXml(HCkRar handle);
#endif
