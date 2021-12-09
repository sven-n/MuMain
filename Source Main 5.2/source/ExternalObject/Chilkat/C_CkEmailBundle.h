#ifndef _CkEmailBundle_H
#define _CkEmailBundle_H
#include "Chilkat_C.h"

HCkEmailBundle CkEmailBundle_Create(void);
void CkEmailBundle_Dispose(HCkEmailBundle handle);
void CkEmailBundle_getLastErrorHtml(HCkEmailBundle handle, HCkString retval);
void CkEmailBundle_getLastErrorText(HCkEmailBundle handle, HCkString retval);
void CkEmailBundle_getLastErrorXml(HCkEmailBundle handle, HCkString retval);
long CkEmailBundle_getMessageCount(HCkEmailBundle handle);
BOOL CkEmailBundle_getUtf8(HCkEmailBundle handle);
void CkEmailBundle_putUtf8(HCkEmailBundle handle, BOOL newVal);
BOOL CkEmailBundle_AddEmail(HCkEmailBundle handle, HCkEmail email);
HCkEmail CkEmailBundle_FindByHeader(HCkEmailBundle handle, const char *name, const char *value);
HCkEmail CkEmailBundle_GetEmail(HCkEmailBundle handle, long index);
HCkStringArray CkEmailBundle_GetUidls(HCkEmailBundle handle);
void CkEmailBundle_GetXml(HCkEmailBundle handle, HCkString sbXml);
BOOL CkEmailBundle_LoadXml(HCkEmailBundle handle, const char *filename);
BOOL CkEmailBundle_LoadXmlString(HCkEmailBundle handle, const char *xmlStr);
BOOL CkEmailBundle_RemoveEmail(HCkEmailBundle handle, HCkEmail email);
BOOL CkEmailBundle_RemoveEmailByIndex(HCkEmailBundle handle, int index);
BOOL CkEmailBundle_SaveLastError(HCkEmailBundle handle, const char *filename);
BOOL CkEmailBundle_SaveXml(HCkEmailBundle handle, const char *filename);
void CkEmailBundle_SortByDate(HCkEmailBundle handle, BOOL ascending);
void CkEmailBundle_SortByRecipient(HCkEmailBundle handle, BOOL ascending);
void CkEmailBundle_SortBySender(HCkEmailBundle handle, BOOL ascending);
void CkEmailBundle_SortBySubject(HCkEmailBundle handle, BOOL ascending);
const char *CkEmailBundle_getXml(HCkEmailBundle handle);
const char *CkEmailBundle_lastErrorHtml(HCkEmailBundle handle);
const char *CkEmailBundle_lastErrorText(HCkEmailBundle handle);
const char *CkEmailBundle_lastErrorXml(HCkEmailBundle handle);
#endif
