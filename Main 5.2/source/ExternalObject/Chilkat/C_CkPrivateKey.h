#ifndef _CkPrivateKey_H
#define _CkPrivateKey_H
#include "Chilkat_C.h"

HCkPrivateKey CkPrivateKey_Create(void);
void CkPrivateKey_Dispose(HCkPrivateKey handle);
void CkPrivateKey_getLastErrorHtml(HCkPrivateKey handle, HCkString retval);
void CkPrivateKey_getLastErrorText(HCkPrivateKey handle, HCkString retval);
void CkPrivateKey_getLastErrorXml(HCkPrivateKey handle, HCkString retval);
BOOL CkPrivateKey_getUtf8(HCkPrivateKey handle);
void CkPrivateKey_putUtf8(HCkPrivateKey handle, BOOL newVal);
BOOL CkPrivateKey_GetPkcs8(HCkPrivateKey handle, HCkByteData data);
BOOL CkPrivateKey_GetPkcs8Encrypted(HCkPrivateKey handle, const char *password, HCkByteData outBytes);
BOOL CkPrivateKey_GetPkcs8EncryptedPem(HCkPrivateKey handle, const char *password, HCkString outStr);
BOOL CkPrivateKey_GetPkcs8Pem(HCkPrivateKey handle, HCkString str);
BOOL CkPrivateKey_GetRsaDer(HCkPrivateKey handle, HCkByteData data);
BOOL CkPrivateKey_GetRsaPem(HCkPrivateKey handle, HCkString str);
BOOL CkPrivateKey_GetXml(HCkPrivateKey handle, HCkString str);
BOOL CkPrivateKey_LoadEncryptedPem(HCkPrivateKey handle, const char *pemStr, const char *password);
BOOL CkPrivateKey_LoadEncryptedPemFile(HCkPrivateKey handle, const char *filename, const char *password);
BOOL CkPrivateKey_LoadPem(HCkPrivateKey handle, const char *str);
BOOL CkPrivateKey_LoadPemFile(HCkPrivateKey handle, const char *filename);
BOOL CkPrivateKey_LoadPkcs8(HCkPrivateKey handle, HCkByteData data);
BOOL CkPrivateKey_LoadPkcs8Encrypted(HCkPrivateKey handle, HCkByteData data, const char *password);
BOOL CkPrivateKey_LoadPkcs8EncryptedFile(HCkPrivateKey handle, const char *filename, const char *password);
BOOL CkPrivateKey_LoadPkcs8File(HCkPrivateKey handle, const char *filename);
BOOL CkPrivateKey_LoadPvk(HCkPrivateKey handle, HCkByteData data, const char *password);
BOOL CkPrivateKey_LoadPvkFile(HCkPrivateKey handle, const char *filename, const char *password);
BOOL CkPrivateKey_LoadRsaDer(HCkPrivateKey handle, HCkByteData data);
BOOL CkPrivateKey_LoadRsaDerFile(HCkPrivateKey handle, const char *filename);
BOOL CkPrivateKey_LoadXml(HCkPrivateKey handle, const char *xml);
BOOL CkPrivateKey_LoadXmlFile(HCkPrivateKey handle, const char *filename);
BOOL CkPrivateKey_SaveLastError(HCkPrivateKey handle, const char *filename);
BOOL CkPrivateKey_SavePkcs8EncryptedFile(HCkPrivateKey handle, const char *password, const char *filename);
BOOL CkPrivateKey_SavePkcs8EncryptedPemFile(HCkPrivateKey handle, const char *password, const char *filename);
BOOL CkPrivateKey_SavePkcs8File(HCkPrivateKey handle, const char *filename);
BOOL CkPrivateKey_SavePkcs8PemFile(HCkPrivateKey handle, const char *filename);
BOOL CkPrivateKey_SaveRsaDerFile(HCkPrivateKey handle, const char *filename);
BOOL CkPrivateKey_SaveRsaPemFile(HCkPrivateKey handle, const char *filename);
BOOL CkPrivateKey_SaveXmlFile(HCkPrivateKey handle, const char *filename);
const char *CkPrivateKey_getPkcs8EncryptedPem(HCkPrivateKey handle, const char *password);
const char *CkPrivateKey_getPkcs8Pem(HCkPrivateKey handle);
const char *CkPrivateKey_getRsaPem(HCkPrivateKey handle);
const char *CkPrivateKey_getXml(HCkPrivateKey handle);
const char *CkPrivateKey_lastErrorHtml(HCkPrivateKey handle);
const char *CkPrivateKey_lastErrorText(HCkPrivateKey handle);
const char *CkPrivateKey_lastErrorXml(HCkPrivateKey handle);
#endif
