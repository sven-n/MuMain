#ifndef _CkCertStore_H
#define _CkCertStore_H
#include "Chilkat_C.h"

HCkCertStore CkCertStore_Create(void);
void CkCertStore_Dispose(HCkCertStore handle);
void CkCertStore_getLastErrorHtml(HCkCertStore handle, HCkString retval);
void CkCertStore_getLastErrorText(HCkCertStore handle, HCkString retval);
void CkCertStore_getLastErrorXml(HCkCertStore handle, HCkString retval);
long CkCertStore_getNumCertificates(HCkCertStore handle);
long CkCertStore_getNumEmailCerts(HCkCertStore handle);
BOOL CkCertStore_getUtf8(HCkCertStore handle);
void CkCertStore_putUtf8(HCkCertStore handle, BOOL newVal);
void CkCertStore_getVersion(HCkCertStore handle, HCkString retval);
BOOL CkCertStore_AddCertificate(HCkCertStore handle, HCkCert cert);
BOOL CkCertStore_CreateFileStore(HCkCertStore handle, const char *filename);
BOOL CkCertStore_CreateMemoryStore(HCkCertStore handle);
BOOL CkCertStore_CreateRegistryStore(HCkCertStore handle, const char *regRoot, const char *regPath);
HCkCert CkCertStore_FindCertByRfc822Name(HCkCertStore handle, const char *name);
HCkCert CkCertStore_FindCertBySerial(HCkCertStore handle, const char *serialNumber);
HCkCert CkCertStore_FindCertBySha1Thumbprint(HCkCertStore handle, const char *str);
HCkCert CkCertStore_FindCertBySubject(HCkCertStore handle, const char *subject);
HCkCert CkCertStore_FindCertBySubjectCN(HCkCertStore handle, const char *commonName);
HCkCert CkCertStore_FindCertBySubjectE(HCkCertStore handle, const char *emailAddress);
HCkCert CkCertStore_FindCertBySubjectO(HCkCertStore handle, const char *organization);
HCkCert CkCertStore_FindCertForEmail(HCkCertStore handle, const char *emailAddress);
HCkCert CkCertStore_GetCertificate(HCkCertStore handle, long index);
HCkCert CkCertStore_GetEmailCert(HCkCertStore handle, long index);
BOOL CkCertStore_LoadPfxData(HCkCertStore handle, HCkByteData pfxData, const char *password);
BOOL CkCertStore_LoadPfxData2(HCkCertStore handle, const unsigned char *buf, unsigned long bufLen, const char *password);
BOOL CkCertStore_LoadPfxFile(HCkCertStore handle, const char *filename, const char *password);
BOOL CkCertStore_OpenChilkatStore(HCkCertStore handle, BOOL readOnly);
BOOL CkCertStore_OpenCurrentUserStore(HCkCertStore handle, BOOL readOnly);
BOOL CkCertStore_OpenFileStore(HCkCertStore handle, const char *filename, BOOL readOnly);
BOOL CkCertStore_OpenOutlookStore(HCkCertStore handle, BOOL readOnly);
BOOL CkCertStore_OpenRegistryStore(HCkCertStore handle, const char *regRoot, const char *regPath, BOOL readOnly);
BOOL CkCertStore_RemoveCertificate(HCkCertStore handle, HCkCert cert);
BOOL CkCertStore_SaveLastError(HCkCertStore handle, const char *filename);
const char *CkCertStore_lastErrorHtml(HCkCertStore handle);
const char *CkCertStore_lastErrorText(HCkCertStore handle);
const char *CkCertStore_lastErrorXml(HCkCertStore handle);
const char *CkCertStore_version(HCkCertStore handle);
#endif
