#ifndef _CkSshKey_H
#define _CkSshKey_H
#include "Chilkat_C.h"

HCkSshKey CkSshKey_Create(void);
void CkSshKey_Dispose(HCkSshKey handle);
void CkSshKey_getComment(HCkSshKey handle, HCkString retval);
void CkSshKey_putComment(HCkSshKey handle, const char *newVal);
BOOL CkSshKey_getIsDsaKey(HCkSshKey handle);
BOOL CkSshKey_getIsPrivateKey(HCkSshKey handle);
BOOL CkSshKey_getIsRsaKey(HCkSshKey handle);
void CkSshKey_getLastErrorHtml(HCkSshKey handle, HCkString retval);
void CkSshKey_getLastErrorText(HCkSshKey handle, HCkString retval);
void CkSshKey_getLastErrorXml(HCkSshKey handle, HCkString retval);
void CkSshKey_getPassword(HCkSshKey handle, HCkString retval);
void CkSshKey_putPassword(HCkSshKey handle, const char *newVal);
BOOL CkSshKey_getUtf8(HCkSshKey handle);
void CkSshKey_putUtf8(HCkSshKey handle, BOOL newVal);
BOOL CkSshKey_FromOpenSshPrivateKey(HCkSshKey handle, const char *keyStr);
BOOL CkSshKey_FromOpenSshPublicKey(HCkSshKey handle, const char *keyStr);
BOOL CkSshKey_FromPuttyPrivateKey(HCkSshKey handle, const char *keyStr);
BOOL CkSshKey_FromRfc4716PublicKey(HCkSshKey handle, const char *keyStr);
BOOL CkSshKey_FromXml(HCkSshKey handle, const char *xmlKey);
BOOL CkSshKey_GenFingerprint(HCkSshKey handle, HCkString outStr);
BOOL CkSshKey_GenerateDsaKey(HCkSshKey handle, int numBits);
BOOL CkSshKey_GenerateRsaKey(HCkSshKey handle, int numBits, int exponent);
BOOL CkSshKey_LoadText(HCkSshKey handle, const char *filename, HCkString outStr);
BOOL CkSshKey_SaveLastError(HCkSshKey handle, const char *filename);
BOOL CkSshKey_SaveText(HCkSshKey handle, const char *strToSave, const char *filename);
BOOL CkSshKey_ToOpenSshPrivateKey(HCkSshKey handle, BOOL bEncrypt, HCkString outStr);
BOOL CkSshKey_ToOpenSshPublicKey(HCkSshKey handle, HCkString outStr);
BOOL CkSshKey_ToPuttyPrivateKey(HCkSshKey handle, BOOL bEncrypt, HCkString outStr);
BOOL CkSshKey_ToRfc4716PublicKey(HCkSshKey handle, HCkString outStr);
BOOL CkSshKey_ToXml(HCkSshKey handle, HCkString outStr);
const char *CkSshKey_comment(HCkSshKey handle);
const char *CkSshKey_genFingerprint(HCkSshKey handle);
const char *CkSshKey_lastErrorHtml(HCkSshKey handle);
const char *CkSshKey_lastErrorText(HCkSshKey handle);
const char *CkSshKey_lastErrorXml(HCkSshKey handle);
const char *CkSshKey_loadText(HCkSshKey handle, const char *filename);
const char *CkSshKey_password(HCkSshKey handle);
const char *CkSshKey_toOpenSshPrivateKey(HCkSshKey handle, BOOL bEncrypt);
const char *CkSshKey_toOpenSshPublicKey(HCkSshKey handle);
const char *CkSshKey_toPuttyPrivateKey(HCkSshKey handle, BOOL bEncrypt);
const char *CkSshKey_toRfc4716PublicKey(HCkSshKey handle);
const char *CkSshKey_toXml(HCkSshKey handle);
#endif
