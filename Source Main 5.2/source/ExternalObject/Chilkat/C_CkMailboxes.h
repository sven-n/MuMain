#ifndef _CkMailboxes_H
#define _CkMailboxes_H
#include "Chilkat_C.h"

HCkMailboxes CkMailboxes_Create(void);
void CkMailboxes_Dispose(HCkMailboxes handle);
long CkMailboxes_getCount(HCkMailboxes handle);
BOOL CkMailboxes_getUtf8(HCkMailboxes handle);
void CkMailboxes_putUtf8(HCkMailboxes handle, BOOL newVal);
BOOL CkMailboxes_GetName(HCkMailboxes handle, long index, HCkString strName);
BOOL CkMailboxes_HasInferiors(HCkMailboxes handle, long index);
BOOL CkMailboxes_IsMarked(HCkMailboxes handle, long index);
BOOL CkMailboxes_IsSelectable(HCkMailboxes handle, long index);
const char *CkMailboxes_getName(HCkMailboxes handle, long index);
#endif
