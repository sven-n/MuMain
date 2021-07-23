#ifndef _CkBounce_H
#define _CkBounce_H
#include "Chilkat_C.h"

HCkBounce CkBounce_Create(void);
void CkBounce_Dispose(HCkBounce handle);
void CkBounce_getBounceAddress(HCkBounce handle, HCkString retval);
void CkBounce_getBounceData(HCkBounce handle, HCkString retval);
int CkBounce_getBounceType(HCkBounce handle);
void CkBounce_getLastErrorHtml(HCkBounce handle, HCkString retval);
void CkBounce_getLastErrorText(HCkBounce handle, HCkString retval);
void CkBounce_getLastErrorXml(HCkBounce handle, HCkString retval);
BOOL CkBounce_getUtf8(HCkBounce handle);
void CkBounce_putUtf8(HCkBounce handle, BOOL newVal);
void CkBounce_getVersion(HCkBounce handle, HCkString retval);
BOOL CkBounce_ExamineEmail(HCkBounce handle, HCkEmail email);
BOOL CkBounce_ExamineEml(HCkBounce handle, const char *emlFilename);
BOOL CkBounce_ExamineMime(HCkBounce handle, HCkString mimeString);
BOOL CkBounce_SaveLastError(HCkBounce handle, const char *filename);
BOOL CkBounce_UnlockComponent(HCkBounce handle, const char *unlockCode);
const char *CkBounce_bounceAddress(HCkBounce handle);
const char *CkBounce_bounceData(HCkBounce handle);
const char *CkBounce_lastErrorHtml(HCkBounce handle);
const char *CkBounce_lastErrorText(HCkBounce handle);
const char *CkBounce_lastErrorXml(HCkBounce handle);
const char *CkBounce_version(HCkBounce handle);
#endif
