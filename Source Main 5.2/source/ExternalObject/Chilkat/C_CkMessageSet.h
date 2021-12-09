#ifndef _CkMessageSet_H
#define _CkMessageSet_H
#include "Chilkat_C.h"

HCkMessageSet CkMessageSet_Create(void);
void CkMessageSet_Dispose(HCkMessageSet handle);
long CkMessageSet_getCount(HCkMessageSet handle);
BOOL CkMessageSet_getHasUids(HCkMessageSet handle);
void CkMessageSet_putHasUids(HCkMessageSet handle, BOOL newVal);
BOOL CkMessageSet_ContainsId(HCkMessageSet handle, long id);
BOOL CkMessageSet_FromCompactString(HCkMessageSet handle, const char *str);
long CkMessageSet_GetId(HCkMessageSet handle, long index);
void CkMessageSet_InsertId(HCkMessageSet handle, long id);
void CkMessageSet_RemoveId(HCkMessageSet handle, long id);
void CkMessageSet_ToCompactString(HCkMessageSet handle, HCkString str);
void CkMessageSet_ToString(HCkMessageSet handle, HCkString str);
const char *CkMessageSet_toCompactString(HCkMessageSet handle);
const char *CkMessageSet_toString(HCkMessageSet handle);
#endif
