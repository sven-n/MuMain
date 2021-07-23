// CkXmp.h: interface for the CkXmp class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CKXMP_H
#define _CKXMP_H

#pragma once

#include "CkObject.h"
#include "CkString.h"

#if !defined(WIN32) && !defined(WINCE)
#include "SystemTime.h"
#endif

class CkXml;
class CkStringArray;
class CkByteData;

/*
    IMPORTANT: Objects returned by methods as non-const pointers must be deleted
    by the calling application. 

  */

#pragma pack (push, 8) 

// CLASS: CkXmp
class CkXmp : public CkObject
{
    private:
	void *m_impl;
	
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// If false, use rdf:parseType="Resource" for structures instead of 
	// inner description nodes.  The default value is true.
	bool m_structInnerDescrip;
	
	CkString m_errorInfo;
	
	CkXml **m_xml;
	

	//bool addDescrip(CkXml &xml, const char *ns, CkXml &xmlOut);
	//void getAbout(CkXml &xml, CkString &strAbout);
	//void buildArray(CkXml &xml, CkStringArray &array);


	// Don't allow assignment or copying these objects.
	CkXmp(const CkXmp &) { } 
	CkXmp &operator=(const CkXmp &) { return *this; }

    public:
    	
	CkXmp();
	virtual ~CkXmp();

	// BEGIN PUBLIC INTERFACE
	bool get_Utf8(void) const;
	void put_Utf8(bool b);

	// Error log retrieval and saving (these methods are common to all Chilkat VC++ classes.)
	bool SaveLastError(const char *filename);
	void LastErrorXml(CkString &str);
	void LastErrorHtml(CkString &str);
	void LastErrorText(CkString &str);

	bool UnlockComponent(const char *unlockCode);
	
	void get_Version(CkString &strOut);
	
	long get_NumEmbedded(void);

	bool LoadAppFile(const char *filename);
	bool SaveAppFile(const char *filename);
	
	bool LoadFromBuffer(CkByteData &byteData, const char *ext);
	bool SaveToBuffer(CkByteData &byteData);
	
	CkXml *GetEmbedded(int index);
	
	bool GetSimpleStr(CkXml &xml, const char *propName, CkString &strOut);
	int GetSimpleInt(CkXml &xml, const char *propName);
	bool GetSimpleDate(CkXml &xml, const char *propName, SYSTEMTIME &sysTime);
	
	bool AddSimpleStr(CkXml &xml, const char *propName, const char *propVal);
	bool AddSimpleInt(CkXml &xml, const char *propName, int propVal);
	bool AddSimpleDate(CkXml &xml, const char *propName, SYSTEMTIME &sysTime);
	
	void AddNsMapping(const char *ns, const char *uri);
	void RemoveNsMapping(const char *ns);
	
	void NewXmp(CkXml &xmlOut);
	
	bool StringToDate(const char *str, SYSTEMTIME &sysTime);
	bool DateToString(SYSTEMTIME &sysTime, CkString &strOut);
	
	bool RemoveSimple(CkXml &xml, const char *propName);
	bool Append(CkXml &xml);
	
	void RemoveAllEmbedded(void);
	void RemoveEmbedded(int index);

	bool get_StructInnerDescrip(void);
	void put_StructInnerDescrip(bool val);

	bool GetArray(CkXml &xml, const char *propName, CkStringArray &array);
	bool RemoveArray(CkXml &xml, const char *propName);
	bool AddArray(CkXml &xml, const char *arrType, const char *propName, CkStringArray &values);

	bool GetStructValue(CkXml &xml, const char *structName, const char *propName, CkString &strOut);
	bool GetStructPropNames(CkXml &xml, const char *structName, CkStringArray &array);
	
	bool RemoveStruct(CkXml &xml, const char *structName);
	bool RemoveStructProp(CkXml &xml, const char *structName, const char *propName);
	bool AddStructProp(CkXml &xml, const char *structName, const char *propName, const char *propVal);

	CkString m_resultString;
        const char *lastErrorText(void);
        const char *lastErrorXml(void);
        const char *lastErrorHtml(void);
	const char *getSimpleStr(CkXml &xml, const char *propName);
	const char *getStructValue(CkXml &xml, const char *structName, const char *propName);

	// Same as getSimpleStr/getStructValue:
	const char *simpleStr(CkXml &xml, const char *propName);
	const char *structValue(CkXml &xml, const char *structName, const char *propName);

	const char *version(void);
	// SYSTEMTIME is defined in <windows.h>
	// For more information, Google "SYSTEMTIME"
	const char *dateToString(SYSTEMTIME &sysTime);

	// CK_INSERT_POINT

	// END PUBLIC INTERFACE




};

#pragma pack (pop)

#endif


