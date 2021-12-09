#ifndef _CkKeyContainer_H
#define _CkKeyContainer_H
#include "Chilkat_C.h"

HCkKeyContainer CkKeyContainer_Create(void);
void CkKeyContainer_Dispose(HCkKeyContainer handle);
void CkKeyContainer_getContainerName(HCkKeyContainer handle, HCkString retval);
BOOL CkKeyContainer_getIsMachineKeyset(HCkKeyContainer handle);
BOOL CkKeyContainer_getIsOpen(HCkKeyContainer handle);
void CkKeyContainer_getLastErrorHtml(HCkKeyContainer handle, HCkString retval);
void CkKeyContainer_getLastErrorText(HCkKeyContainer handle, HCkString retval);
void CkKeyContainer_getLastErrorXml(HCkKeyContainer handle, HCkString retval);
BOOL CkKeyContainer_getUtf8(HCkKeyContainer handle);
void CkKeyContainer_putUtf8(HCkKeyContainer handle, BOOL newVal);
void CkKeyContainer_CloseContainer(HCkKeyContainer handle);
BOOL CkKeyContainer_CreateContainer(HCkKeyContainer handle, const char *name, BOOL machineKeyset);
BOOL CkKeyContainer_DeleteContainer(HCkKeyContainer handle);
BOOL CkKeyContainer_FetchContainerNames(HCkKeyContainer handle, BOOL bMachineKeyset);
BOOL CkKeyContainer_GenerateKeyPair(HCkKeyContainer handle, BOOL bKeyExchangePair, int keyLengthInBits);
void CkKeyContainer_GenerateUuid(HCkKeyContainer handle, HCkString guid);
void CkKeyContainer_GetContainerName(HCkKeyContainer handle, BOOL bMachineKeyset, int index, HCkString name);
int CkKeyContainer_GetNumContainers(HCkKeyContainer handle, BOOL bMachineKeyset);
HCkPrivateKey CkKeyContainer_GetPrivateKey(HCkKeyContainer handle, BOOL bKeyExchangePair);
HCkPublicKey CkKeyContainer_GetPublicKey(HCkKeyContainer handle, BOOL bKeyExchangePair);
BOOL CkKeyContainer_ImportPrivateKey(HCkKeyContainer handle, HCkPrivateKey key, BOOL bKeyExchangePair);
BOOL CkKeyContainer_ImportPublicKey(HCkKeyContainer handle, HCkPublicKey key, BOOL bKeyExchangePair);
BOOL CkKeyContainer_OpenContainer(HCkKeyContainer handle, const char *name, BOOL needPrivateKeyAccess, BOOL machineKeyset);
BOOL CkKeyContainer_SaveLastError(HCkKeyContainer handle, const char *filename);
const char *CkKeyContainer_containerName(HCkKeyContainer handle);
const char *CkKeyContainer_generateUuid(HCkKeyContainer handle);
const char *CkKeyContainer_getContainerNameByIndex(HCkKeyContainer handle, BOOL bMachineKeyset, int index);
const char *CkKeyContainer_lastErrorHtml(HCkKeyContainer handle);
const char *CkKeyContainer_lastErrorText(HCkKeyContainer handle);
const char *CkKeyContainer_lastErrorXml(HCkKeyContainer handle);
#endif
