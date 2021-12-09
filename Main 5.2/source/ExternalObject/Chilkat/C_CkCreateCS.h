#ifndef _CkCreateCS_H
#define _CkCreateCS_H
#include "Chilkat_C.h"

HCkCreateCS CkCreateCS_Create(void);
void CkCreateCS_Dispose(HCkCreateCS handle);
void CkCreateCS_getLastErrorHtml(HCkCreateCS handle, HCkString retval);
void CkCreateCS_getLastErrorText(HCkCreateCS handle, HCkString retval);
void CkCreateCS_getLastErrorXml(HCkCreateCS handle, HCkString retval);
BOOL CkCreateCS_getReadOnly(HCkCreateCS handle);
void CkCreateCS_putReadOnly(HCkCreateCS handle, BOOL newVal);
BOOL CkCreateCS_getUtf8(HCkCreateCS handle);
void CkCreateCS_putUtf8(HCkCreateCS handle, BOOL newVal);
void CkCreateCS_getVersion(HCkCreateCS handle, HCkString retval);
HCkCertStore CkCreateCS_CreateFileStore(HCkCreateCS handle, const char *filename);
HCkCertStore CkCreateCS_CreateMemoryStore(HCkCreateCS handle);
HCkCertStore CkCreateCS_CreateRegistryStore(HCkCreateCS handle, const char *regRoot, const char *regPath);
HCkCertStore CkCreateCS_OpenChilkatStore(HCkCreateCS handle);
HCkCertStore CkCreateCS_OpenCurrentUserStore(HCkCreateCS handle);
HCkCertStore CkCreateCS_OpenFileStore(HCkCreateCS handle, const char *filename);
HCkCertStore CkCreateCS_OpenLocalSystemStore(HCkCreateCS handle);
HCkCertStore CkCreateCS_OpenOutlookStore(HCkCreateCS handle);
HCkCertStore CkCreateCS_OpenRegistryStore(HCkCreateCS handle, const char *regRoot, const char *regPath);
BOOL CkCreateCS_SaveLastError(HCkCreateCS handle, const char *filename);
const char *CkCreateCS_lastErrorHtml(HCkCreateCS handle);
const char *CkCreateCS_lastErrorText(HCkCreateCS handle);
const char *CkCreateCS_lastErrorXml(HCkCreateCS handle);
#endif
