// CkService.h: interface for the CkService class.
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32

#ifndef _CKSERVICE_H
#define _CKSERVICE_H

#pragma once

#include "CkString.h"
class CkByteData;

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkService
class CkService  : public CkObject
{
    private:

	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkService(const CkService &) { } 
	CkService &operator=(const CkService &) { return *this; }
	CkService(void *impl) : m_impl(impl) { }

    public:
	void *getImpl(void) const { return m_impl; } 

	CkService();
	virtual ~CkService();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
	void LastErrorXml(CkString &str);
	void LastErrorHtml(CkString &str);
	void LastErrorText(CkString &str);

    const char *lastErrorText(void);
    const char *lastErrorXml(void);
    const char *lastErrorHtml(void);
    	// INSTALL_BEGIN
	bool Install(void);
	// INSTALL_END
	// UNINSTALL_BEGIN
	bool Uninstall(void);
	// UNINSTALL_END
	// START_BEGIN
	bool Start(void);
	// START_END
	// STOP_BEGIN
	bool Stop(void);
	// STOP_END
	// SERVICENAME_BEGIN
	void get_ServiceName(CkString &str);
	const char *serviceName(void);
	void put_ServiceName(const char *newVal);
	// SERVICENAME_END
	// DISPLAYNAME_BEGIN
	void get_DisplayName(CkString &str);
	const char *displayName(void);
	void put_DisplayName(const char *newVal);
	// DISPLAYNAME_END
	// EXEFILENAME_BEGIN
	void get_ExeFilename(CkString &str);
	const char *exeFilename(void);
	void put_ExeFilename(const char *newVal);
	// EXEFILENAME_END
	// AUTOSTART_BEGIN
	bool get_AutoStart(void);
	void put_AutoStart(bool newVal);
	// AUTOSTART_END
	// SETAUTOSTART_BEGIN
	bool SetAutoStart(void);
	// SETAUTOSTART_END
	// SETDEMANDSTART_BEGIN
	bool SetDemandStart(void);
	// SETDEMANDSTART_END
	// DISABLE_BEGIN
	bool Disable(void);
	// DISABLE_END
	// ISAUTOSTART_BEGIN
	int IsAutoStart(void);
	// ISAUTOSTART_END
	// ISDEMANDSTART_BEGIN
	int IsDemandStart(void);
	// ISDEMANDSTART_END
	// ISDISABLED_BEGIN
	int IsDisabled(void);
	// ISDISABLED_END
	// ISINSTALLED_BEGIN
	int IsInstalled(void);
	// ISINSTALLED_END
	// ISRUNNING_BEGIN
	int IsRunning(void);
	// ISRUNNING_END

	// SERVICE_INSERT_POINT

	// END PUBLIC INTERFACE
		
	CkString m_resultString;
	



};
#pragma pack (pop)


#endif


#endif
