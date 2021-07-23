#ifndef _CkAtom_H
#define _CkAtom_H
#include "Chilkat_C.h"

HCkAtom CkAtom_Create(void);
void CkAtom_Dispose(HCkAtom handle);
void CkAtom_getLastErrorHtml(HCkAtom handle, HCkString retval);
void CkAtom_getLastErrorText(HCkAtom handle, HCkString retval);
void CkAtom_getLastErrorXml(HCkAtom handle, HCkString retval);
int CkAtom_getNumEntries(HCkAtom handle);
BOOL CkAtom_getUtf8(HCkAtom handle);
void CkAtom_putUtf8(HCkAtom handle, BOOL newVal);
int CkAtom_AddElement(HCkAtom handle, const char *tag, const char *value);
int CkAtom_AddElementDate(HCkAtom handle, const char *tag, SYSTEMTIME *dateTime);
int CkAtom_AddElementHtml(HCkAtom handle, const char *tag, const char *htmlStr);
int CkAtom_AddElementXHtml(HCkAtom handle, const char *tag, const char *xmlStr);
int CkAtom_AddElementXml(HCkAtom handle, const char *tag, const char *xmlStr);
void CkAtom_AddEntry(HCkAtom handle, const char *xmlStr);
void CkAtom_AddLink(HCkAtom handle, const char *rel, const char *href, const char *title, const char *typ);
void CkAtom_AddPerson(HCkAtom handle, const char *tag, const char *name, const char *uri, const char *email);
void CkAtom_DeleteElement(HCkAtom handle, const char *tag, int index);
void CkAtom_DeleteElementAttr(HCkAtom handle, const char *tag, int index, const char *attrName);
void CkAtom_DeletePerson(HCkAtom handle, const char *tag, int index);
BOOL CkAtom_DownloadAtom(HCkAtom handle, const char *url);
BOOL CkAtom_GetElement(HCkAtom handle, const char *tag, int index, HCkString outStr);
BOOL CkAtom_GetElementAttr(HCkAtom handle, const char *tag, int index, const char *attrName, HCkString outStr);
int CkAtom_GetElementCount(HCkAtom handle, const char *tag);
BOOL CkAtom_GetElementDate(HCkAtom handle, const char *tag, int index, SYSTEMTIME *sysTime);
HCkAtom CkAtom_GetEntry(HCkAtom handle, int index);
BOOL CkAtom_GetLinkHref(HCkAtom handle, const char *relName, HCkString outStr);
BOOL CkAtom_GetPersonInfo(HCkAtom handle, const char *tag, int index, const char *tag2, HCkString outStr);
BOOL CkAtom_GetTopAttr(HCkAtom handle, const char *attrName, HCkString outStr);
BOOL CkAtom_HasElement(HCkAtom handle, const char *tag);
BOOL CkAtom_LoadXml(HCkAtom handle, const char *xmlStr);
void CkAtom_NewEntry(HCkAtom handle);
void CkAtom_NewFeed(HCkAtom handle);
BOOL CkAtom_SaveLastError(HCkAtom handle, const char *filename);
void CkAtom_SetElementAttr(HCkAtom handle, const char *tag, int index, const char *attrName, const char *attrValue);
void CkAtom_SetTopAttr(HCkAtom handle, const char *attrName, const char *value);
BOOL CkAtom_ToXmlString(HCkAtom handle, HCkString outStr);
void CkAtom_UpdateElement(HCkAtom handle, const char *tag, int index, const char *value);
void CkAtom_UpdateElementDate(HCkAtom handle, const char *tag, int index, SYSTEMTIME *dateTime);
void CkAtom_UpdateElementHtml(HCkAtom handle, const char *tag, int index, const char *htmlStr);
void CkAtom_UpdateElementXHtml(HCkAtom handle, const char *tag, int index, const char *xmlStr);
void CkAtom_UpdateElementXml(HCkAtom handle, const char *tag, int index, const char *xmlStr);
void CkAtom_UpdatePerson(HCkAtom handle, const char *tag, int index, const char *name, const char *uri, const char *email);
const char *CkAtom_getElement(HCkAtom handle, const char *tag, int index);
const char *CkAtom_getElementAttr(HCkAtom handle, const char *tag, int index, const char *attrName);
const char *CkAtom_getLinkHref(HCkAtom handle, const char *relName);
const char *CkAtom_getPersonInfo(HCkAtom handle, const char *tag, int index, const char *tag2);
const char *CkAtom_getTopAttr(HCkAtom handle, const char *attrName);
const char *CkAtom_lastErrorHtml(HCkAtom handle);
const char *CkAtom_lastErrorText(HCkAtom handle);
const char *CkAtom_lastErrorXml(HCkAtom handle);
const char *CkAtom_toXmlString(HCkAtom handle);
#endif
