#ifndef _CkRss_H
#define _CkRss_H
#include "Chilkat_C.h"

HCkRss CkRss_Create(void);
void CkRss_Dispose(HCkRss handle);
void CkRss_getLastErrorHtml(HCkRss handle, HCkString retval);
void CkRss_getLastErrorText(HCkRss handle, HCkString retval);
void CkRss_getLastErrorXml(HCkRss handle, HCkString retval);
int CkRss_getNumChannels(HCkRss handle);
int CkRss_getNumItems(HCkRss handle);
BOOL CkRss_getUtf8(HCkRss handle);
void CkRss_putUtf8(HCkRss handle, BOOL newVal);
HCkRss CkRss_AddNewChannel(HCkRss handle);
HCkRss CkRss_AddNewImage(HCkRss handle);
HCkRss CkRss_AddNewItem(HCkRss handle);
BOOL CkRss_DownloadRss(HCkRss handle, const char *url);
BOOL CkRss_GetAttr(HCkRss handle, const char *tag, const char *attrName, HCkString outStr);
HCkRss CkRss_GetChannel(HCkRss handle, int index);
int CkRss_GetCount(HCkRss handle, const char *tag);
BOOL CkRss_GetDate(HCkRss handle, const char *tag, SYSTEMTIME *sysTime);
HCkRss CkRss_GetImage(HCkRss handle);
int CkRss_GetInt(HCkRss handle, const char *tag);
HCkRss CkRss_GetItem(HCkRss handle, int index);
BOOL CkRss_GetString(HCkRss handle, const char *tag, HCkString outStr);
BOOL CkRss_LoadRssFile(HCkRss handle, const char *filename);
BOOL CkRss_LoadRssString(HCkRss handle, const char *rssString);
BOOL CkRss_MGetAttr(HCkRss handle, const char *tag, int idx, const char *attrName, HCkString outStr);
BOOL CkRss_MGetString(HCkRss handle, const char *tag, int idx, HCkString outStr);
BOOL CkRss_MSetAttr(HCkRss handle, const char *tag, int idx, const char *attrName, const char *value);
BOOL CkRss_MSetString(HCkRss handle, const char *tag, int idx, const char *value);
void CkRss_NewRss(HCkRss handle);
void CkRss_Remove(HCkRss handle, const char *tag);
BOOL CkRss_SaveLastError(HCkRss handle, const char *filename);
void CkRss_SetAttr(HCkRss handle, const char *tag, const char *attrName, const char *value);
void CkRss_SetDate(HCkRss handle, const char *tag, SYSTEMTIME *d);
void CkRss_SetDateNow(HCkRss handle, const char *tag);
void CkRss_SetInt(HCkRss handle, const char *tag, int value);
void CkRss_SetString(HCkRss handle, const char *tag, const char *value);
BOOL CkRss_ToXmlString(HCkRss handle, HCkString outStr);
const char *CkRss_getAttr(HCkRss handle, const char *tag, const char *attrName);
const char *CkRss_getString(HCkRss handle, const char *tag);
const char *CkRss_lastErrorHtml(HCkRss handle);
const char *CkRss_lastErrorText(HCkRss handle);
const char *CkRss_lastErrorXml(HCkRss handle);
const char *CkRss_mGetAttr(HCkRss handle, const char *tag, int idx, const char *attrName);
const char *CkRss_mGetString(HCkRss handle, const char *tag, int idx);
const char *CkRss_toXmlString(HCkRss handle);
#endif
