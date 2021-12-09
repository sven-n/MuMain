/*-----------------------------------------------------------------------------
 * Sharememory.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh Yong Hwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * Revision History:
 * Date					Who					What
 * ----------------		----------------	----------------
 * 13.11.2007			Noh Yong Hwan		birth
**---------------------------------------------------------------------------*/

#ifndef _SHARE_MEMORY_H_
#define _SHARE_MEMORY_H_

const TCHAR	WL_NAME_LENGTH			= 32;
const DWORD WL_ID_LENGTH			= 32;
const DWORD WL_KEY_LENGTH			= MAX_PATH;

typedef struct _WLAUTH_INFO
{
	TCHAR		id[WL_ID_LENGTH];
	BYTE		key[WL_KEY_LENGTH];
	VOID*		pIEHandle;
	VOID*		pMapHandle;
} WLAUTH_INFO, *PWLAUTH_INFO;

class IShareMem
{
public:
	virtual PBYTE GetSharePtr() = 0;
	virtual int WriteToShareMemory(IN LPCTSTR Name, PWLAUTH_INFO AuthInfo) = 0;
	virtual int ReadFromShareMemory(IN LPCTSTR Name, PWLAUTH_INFO AuthInfo) = 0;
	virtual int DestroyShareMemory(IN LPCTSTR Name, PWLAUTH_INFO AuthInfo) = 0;
};

//
// factory method
//
IShareMem*	CreateShareMem(void);
void		DestroyShareMem(IShareMem* instance);

//
// Resources manager
//
class RMSM
{
public:
	RMSM(IShareMem* instance): m_instance(instance){};
    ~RMSM()
	{
		if (NULL != m_instance){ DestroyShareMem(m_instance); }
	}

	IShareMem* get() { return m_instance; };
private:
	IShareMem* m_instance;
};


#endif//_SHARE_MEMORY_H_