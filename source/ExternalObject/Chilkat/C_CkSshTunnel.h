#ifndef _CkSshTunnel_H
#define _CkSshTunnel_H
#include "Chilkat_C.h"

HCkSshTunnel CkSshTunnel_Create(void);
void CkSshTunnel_Dispose(HCkSshTunnel handle);
void CkSshTunnel_getConnectLog(HCkSshTunnel handle, HCkString retval);
void CkSshTunnel_putConnectLog(HCkSshTunnel handle, const char *newVal);
void CkSshTunnel_getDestHostname(HCkSshTunnel handle, HCkString retval);
void CkSshTunnel_putDestHostname(HCkSshTunnel handle, const char *newVal);
int CkSshTunnel_getDestPort(HCkSshTunnel handle);
void CkSshTunnel_putDestPort(HCkSshTunnel handle, int newVal);
BOOL CkSshTunnel_getIsAccepting(HCkSshTunnel handle);
void CkSshTunnel_getLastErrorHtml(HCkSshTunnel handle, HCkString retval);
void CkSshTunnel_getLastErrorText(HCkSshTunnel handle, HCkString retval);
void CkSshTunnel_getLastErrorXml(HCkSshTunnel handle, HCkString retval);
void CkSshTunnel_getListenBindIpAddress(HCkSshTunnel handle, HCkString retval);
void CkSshTunnel_putListenBindIpAddress(HCkSshTunnel handle, const char *newVal);
void CkSshTunnel_getOutboundBindIpAddress(HCkSshTunnel handle, HCkString retval);
void CkSshTunnel_putOutboundBindIpAddress(HCkSshTunnel handle, const char *newVal);
int CkSshTunnel_getOutboundBindPort(HCkSshTunnel handle);
void CkSshTunnel_putOutboundBindPort(HCkSshTunnel handle, int newVal);
void CkSshTunnel_getSshHostname(HCkSshTunnel handle, HCkString retval);
void CkSshTunnel_putSshHostname(HCkSshTunnel handle, const char *newVal);
void CkSshTunnel_getSshLogin(HCkSshTunnel handle, HCkString retval);
void CkSshTunnel_putSshLogin(HCkSshTunnel handle, const char *newVal);
void CkSshTunnel_getSshPassword(HCkSshTunnel handle, HCkString retval);
void CkSshTunnel_putSshPassword(HCkSshTunnel handle, const char *newVal);
int CkSshTunnel_getSshPort(HCkSshTunnel handle);
void CkSshTunnel_putSshPort(HCkSshTunnel handle, int newVal);
BOOL CkSshTunnel_getUtf8(HCkSshTunnel handle);
void CkSshTunnel_putUtf8(HCkSshTunnel handle, BOOL newVal);
BOOL CkSshTunnel_BeginAccepting(HCkSshTunnel handle, int listenPort);
BOOL CkSshTunnel_GetTunnelsXml(HCkSshTunnel handle, HCkString outStr);
BOOL CkSshTunnel_SaveLastError(HCkSshTunnel handle, const char *filename);
BOOL CkSshTunnel_SetSshAuthenticationKey(HCkSshTunnel handle, HCkSshKey key);
BOOL CkSshTunnel_StopAccepting(HCkSshTunnel handle);
BOOL CkSshTunnel_StopAllTunnels(HCkSshTunnel handle, int maxWaitMs);
BOOL CkSshTunnel_UnlockComponent(HCkSshTunnel handle, const char *unlockCode);
const char *CkSshTunnel_connectLog(HCkSshTunnel handle);
const char *CkSshTunnel_destHostname(HCkSshTunnel handle);
const char *CkSshTunnel_getTunnelsXml(HCkSshTunnel handle);
const char *CkSshTunnel_lastErrorHtml(HCkSshTunnel handle);
const char *CkSshTunnel_lastErrorText(HCkSshTunnel handle);
const char *CkSshTunnel_lastErrorXml(HCkSshTunnel handle);
const char *CkSshTunnel_listenBindIpAddress(HCkSshTunnel handle);
const char *CkSshTunnel_outboundBindIpAddress(HCkSshTunnel handle);
const char *CkSshTunnel_sshHostname(HCkSshTunnel handle);
const char *CkSshTunnel_sshLogin(HCkSshTunnel handle);
const char *CkSshTunnel_sshPassword(HCkSshTunnel handle);
#endif
