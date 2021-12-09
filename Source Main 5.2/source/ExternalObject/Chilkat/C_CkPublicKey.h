#ifndef _CkPublicKey_H
#define _CkPublicKey_H
#include "Chilkat_C.h"

HCkPublicKey CkPublicKey_Create(void);
void CkPublicKey_Dispose(HCkPublicKey handle);
void CkPublicKey_getLastErrorHtml(HCkPublicKey handle, HCkString retval);
void CkPublicKey_getLastErrorText(HCkPublicKey handle, HCkString retval);
void CkPublicKey_getLastErrorXml(HCkPublicKey handle, HCkString retval);
BOOL CkPublicKey_getUtf8(HCkPublicKey handle);
void CkPublicKey_putUtf8(HCkPublicKey handle, BOOL newVal);
BOOL CkPublicKey_GetOpenSslDer(HCkPublicKey handle, HCkByteData data);
BOOL CkPublicKey_GetOpenSslPem(HCkPublicKey handle, HCkString str);
BOOL CkPublicKey_GetRsaDer(HCkPublicKey handle, HCkByteData data);
BOOL CkPublicKey_GetXml(HCkPublicKey handle, HCkString str);
BOOL CkPublicKey_LoadOpenSslDer(HCkPublicKey handle, HCkByteData data);
BOOL CkPublicKey_LoadOpenSslDerFile(HCkPublicKey handle, const char *filename);
BOOL CkPublicKey_LoadOpenSslPem(HCkPublicKey handle, const char *str);
BOOL CkPublicKey_LoadOpenSslPemFile(HCkPublicKey handle, const char *filename);
BOOL CkPublicKey_LoadRsaDer(HCkPublicKey handle, HCkByteData data);
BOOL CkPublicKey_LoadRsaDerFile(HCkPublicKey handle, const char *filename);
BOOL CkPublicKey_LoadXml(HCkPublicKey handle, const char *xml);
BOOL CkPublicKey_LoadXmlFile(HCkPublicKey handle, const char *filename);
BOOL CkPublicKey_SaveLastError(HCkPublicKey handle, const char *filename);
BOOL CkPublicKey_SaveOpenSslDerFile(HCkPublicKey handle, const char *filename);
BOOL CkPublicKey_SaveOpenSslPemFile(HCkPublicKey handle, const char *filename);
BOOL CkPublicKey_SaveRsaDerFile(HCkPublicKey handle, const char *filename);
BOOL CkPublicKey_SaveXmlFile(HCkPublicKey handle, const char *filename);
const char *CkPublicKey_getOpenSslPem(HCkPublicKey handle);
const char *CkPublicKey_getXml(HCkPublicKey handle);
const char *CkPublicKey_lastErrorHtml(HCkPublicKey handle);
const char *CkPublicKey_lastErrorText(HCkPublicKey handle);
const char *CkPublicKey_lastErrorXml(HCkPublicKey handle);
#endif
