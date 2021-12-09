// CkNtlm.h: interface for the CkNtlm class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CkNtlm_H
#define _CkNtlm_H

#pragma once

#include "CkString.h"
#include "CkObject.h"

#pragma pack (push, 8) 

// CLASS: CkNtlm
class CkNtlm  : public CkObject
{
    private:
    	
	void *m_impl;
	bool m_utf8;	// If true, all input "const char *" parameters are utf-8, otherwise they are ANSI strings.

	// Don't allow assignment or copying these objects.
	CkNtlm(const CkNtlm &);
	CkNtlm &operator=(const CkNtlm &);

	unsigned long nextIdx(void);
	unsigned long m_resultIdx;
	CkString m_resultString[10];

    public:
	void *getImpl(void) const;

	CkNtlm(void *impl);

	CkNtlm();
	virtual ~CkNtlm();

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
	
	// UNLOCKCOMPONENT_BEGIN
	bool UnlockComponent(const char *unlockCode);
	// TARGETNAME_BEGIN
	void get_TargetName(CkString &str);
	const char *targetName(void);
	void put_TargetName(const char *newVal);
	// TARGETNAME_END
	// NETBIOSCOMPUTERNAME_BEGIN
	void get_NetBiosComputerName(CkString &str);
	const char *netBiosComputerName(void);
	void put_NetBiosComputerName(const char *newVal);
	// NETBIOSCOMPUTERNAME_END
	// NETBIOSDOMAINNAME_BEGIN
	void get_NetBiosDomainName(CkString &str);
	const char *netBiosDomainName(void);
	void put_NetBiosDomainName(const char *newVal);
	// NETBIOSDOMAINNAME_END
	// DNSDOMAINNAME_BEGIN
	void get_DnsDomainName(CkString &str);
	const char *dnsDomainName(void);
	void put_DnsDomainName(const char *newVal);
	// DNSDOMAINNAME_END
	// DNSCOMPUTERNAME_BEGIN
	void get_DnsComputerName(CkString &str);
	const char *dnsComputerName(void);
	void put_DnsComputerName(const char *newVal);
	// DNSCOMPUTERNAME_END
	// USERNAME_BEGIN
	void get_UserName(CkString &str);
	const char *userName(void);
	void put_UserName(const char *newVal);
	// USERNAME_END
	// PASSWORD_BEGIN
	void get_Password(CkString &str);
	const char *password(void);
	void put_Password(const char *newVal);
	// PASSWORD_END
	// NTLMVERSION_BEGIN
	int get_NtlmVersion(void);
	void put_NtlmVersion(int newVal);
	// NTLMVERSION_END
	// OEMCODEPAGE_BEGIN
	int get_OemCodePage(void);
	void put_OemCodePage(int newVal);
	// OEMCODEPAGE_END
	// FLAGS_BEGIN
	void get_Flags(CkString &str);
	const char *flags(void);
	void put_Flags(const char *newVal);
	// FLAGS_END
	// DOMAIN_BEGIN
	void get_Domain(CkString &str);
	const char *domain(void);
	void put_Domain(const char *newVal);
	// DOMAIN_END
	// WORKSTATION_BEGIN
	void get_Workstation(CkString &str);
	const char *workstation(void);
	void put_Workstation(const char *newVal);
	// WORKSTATION_END
	// GENTYPE1_BEGIN
	bool GenType1(CkString &outStr);
	const char *genType1(void);
	// GENTYPE1_END
	// GENTYPE2_BEGIN
	bool GenType2(const char *type1Msg, CkString &outStr);
	const char *genType2(const char *type1Msg);
	// GENTYPE2_END
	// GENTYPE3_BEGIN
	bool GenType3(const char *type2Msg, CkString &outStr);
	const char *genType3(const char *type2Msg);
	// GENTYPE3_END
	// PARSETYPE1_BEGIN
	bool ParseType1(const char *type1Msg, CkString &outStr);
	const char *parseType1(const char *type1Msg);
	// PARSETYPE1_END
	// PARSETYPE2_BEGIN
	bool ParseType2(const char *type2Msg, CkString &outStr);
	const char *parseType2(const char *type2Msg);
	// PARSETYPE2_END
	// PARSETYPE3_BEGIN
	bool ParseType3(const char *type3Msg, CkString &outStr);
	const char *parseType3(const char *type3Msg);
	// PARSETYPE3_END
	// ENCODINGMODE_BEGIN
	void get_EncodingMode(CkString &str);
	const char *encodingMode(void);
	void put_EncodingMode(const char *newVal);
	// ENCODINGMODE_END
	// SETFLAG_BEGIN
	bool SetFlag(const char *flagLetter, bool on);
	// SETFLAG_END
	// CLIENTCHALLENGE_BEGIN
	void get_ClientChallenge(CkString &str);
	const char *clientChallenge(void);
	void put_ClientChallenge(const char *newVal);
	// CLIENTCHALLENGE_END
	// SERVERCHALLENGE_BEGIN
	void get_ServerChallenge(CkString &str);
	const char *serverChallenge(void);
	void put_ServerChallenge(const char *newVal);
	// SERVERCHALLENGE_END
	// LOADTYPE3_BEGIN
	bool LoadType3(const char *type3Msg);
	// LOADTYPE3_END

	// NTLM_INSERT_POINT

	// END PUBLIC INTERFACE


};
#pragma pack (pop)


#endif


