#ifndef _CkCkSettings_H
#define _CkCkSettings_H
#include "Chilkat_C.h"

HCkCkSettings CkCkSettings_Create(void);
void CkCkSettings_Dispose(HCkCkSettings handle);
/*
	Name: cleanupMemory
	Type: static void
*/
static void CkCkSettings_cleanupMemory(HCkCkSettings handle);
/*
	Name: getTotalSizeProcessHeaps
	Type: static unsigned long
*/
static unsigned long CkCkSettings_getTotalSizeProcessHeaps(HCkCkSettings handle);
/*
	Name: initializeMultithreaded
	Type: static void
*/
static void CkCkSettings_initializeMultithreaded(HCkCkSettings handle);
#endif
