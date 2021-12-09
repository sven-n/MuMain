// CkKeyContainer.h: interface for the CkKeyContainer class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _CKKEYCONTAINER_H
#define _CKKEYCONTAINER_H

#pragma once

class CkString;

#include "CkObject.h"
#include "CkString.h"
#include "CkByteData.h"

class CkPrivateKey;
class CkPublicKey;
class CkStringArray;

#pragma pack (push, 8) 

// CLASS: CkKeyContainer
class CkKeyContainer : public CkObject 
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkKeyContainer(const CkKeyContainer &) { } 
	CkKeyContainer &operator=(const CkKeyContainer &) { return *this; }
	CkKeyContainer(void *impl) : m_impl(impl) { }

    public:

	CkKeyContainer();
	virtual ~CkKeyContainer();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);


	// If false, no key container has been opened.
	bool get_IsOpen(void);

	// If true, this key container is in the machine keyset.
	bool get_IsMachineKeyset(void);

	void get_ContainerName(CkString &name);

	// Useful for creating new key containers with unique names.
	void GenerateUuid(CkString &guid);

	// Create a new key container.  (If a container of the same name exists???)
	bool CreateContainer(const char *name, bool machineKeyset);
	bool OpenContainer(const char *name, bool needPrivateKeyAccess, bool machineKeyset);
	bool DeleteContainer(void);	// Delete this key container.
	void CloseContainer(void);

//	    RSA Base Provider
//	    Signature and ExchangeKeys
//		(minimum)     (default) (max)
//		    384 	512 	16,384
//
//	    RSA Strong and Enhanced Providers
//	    Signature and Exchange Keys
//		(minimum)     (default) (max)
//		    384 	1,024 	16,384

	// Pass 0 for the default key length.
	bool GenerateKeyPair(bool bKeyExchangePair, int keyLengthInBits);
	//bool GenerateKeyExchangePair(int keyLengthInBits);

	CkPrivateKey *GetPrivateKey(bool bKeyExchangePair);
	//CkPrivateKey *GetKeyExchangePrivateKey(void);

	CkPublicKey *GetPublicKey(bool bKeyExchangePair);
	//CkPublicKey *GetKeyExchangePublicKey(void);

	bool ImportPublicKey(CkPublicKey &key, bool bKeyExchangePair);
	bool ImportPrivateKey(CkPrivateKey &key, bool bKeyExchangePair);

	bool FetchContainerNames(bool bMachineKeyset);

	int GetNumContainers(bool bMachineKeyset);
	void GetContainerName(bool bMachineKeyset, int index, CkString &name);

	const char *getContainerName(bool bMachineKeyset, int index);
	const char *containerName(void);
	const char *generateUuid(void);


	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
	void LastErrorXml(CkString &str);
	void LastErrorHtml(CkString &str);
	void LastErrorText(CkString &str);

	CkString m_resultString;

	const char *lastErrorText(void);
	const char *lastErrorXml(void);
	const char *lastErrorHtml(void);


	// CK_INSERT_POINT

	// END PUBLIC INTERFACE
	

};
#pragma pack (pop)

#endif
