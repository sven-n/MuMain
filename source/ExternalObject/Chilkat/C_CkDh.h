#ifndef _CkDh_H
#define _CkDh_H
#include "Chilkat_C.h"

HCkDh CkDh_Create(void);
void CkDh_Dispose(HCkDh handle);
int CkDh_getG(HCkDh handle);
void CkDh_getLastErrorHtml(HCkDh handle, HCkString retval);
void CkDh_getLastErrorText(HCkDh handle, HCkString retval);
void CkDh_getLastErrorXml(HCkDh handle, HCkString retval);
void CkDh_getP(HCkDh handle, HCkString retval);
BOOL CkDh_getUtf8(HCkDh handle);
void CkDh_putUtf8(HCkDh handle, BOOL newVal);
void CkDh_getVersion(HCkDh handle, HCkString retval);
BOOL CkDh_CreateE(HCkDh handle, int numBits, HCkString outStr);
BOOL CkDh_FindK(HCkDh handle, const char *e, HCkString outStr);
BOOL CkDh_GenPG(HCkDh handle, int numBits, int g);
BOOL CkDh_SaveLastError(HCkDh handle, const char *filename);
BOOL CkDh_SetPG(HCkDh handle, const char *p, int g);
BOOL CkDh_UnlockComponent(HCkDh handle, const char *unlockCode);
void CkDh_UseKnownPrime(HCkDh handle, int index);
const char *CkDh_createE(HCkDh handle, int numBits);
const char *CkDh_findK(HCkDh handle, const char *e);
const char *CkDh_lastErrorHtml(HCkDh handle);
const char *CkDh_lastErrorText(HCkDh handle);
const char *CkDh_lastErrorXml(HCkDh handle);
const char *CkDh_p(HCkDh handle);
const char *CkDh_version(HCkDh handle);
#endif
