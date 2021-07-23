// CkPfx.h: interface for the CkPfx class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKPFX_H
#define _CKPFX_H


#pragma once

#include "CkString.h"
class CkEmail;
#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkPfx
class CkPfx  : public CkObject
{
    private:
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkPfx(const CkPfx &) { } 
	CkPfx &operator=(const CkPfx &) { return *this; }
	CkPfx(void *impl) : m_impl(impl) { }

    public:
	void *getImpl(void) const { return m_impl; } 

	CkPfx();
	virtual ~CkPfx();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	bool UnlockComponent(const char *unlockCode);

	bool ImportPfxFile(const char *pfxFilename, const char *password, bool bMachineKeyset, bool bLocalMachineCertStore, bool bExportable, bool bUseWarningDialog);

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
