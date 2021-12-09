#ifndef _CkNtlmW_H
#define _CkNtlmW_H
#pragma once

#include "CkString.h"
#include "CkNtlm.h"


#pragma pack (push, 8) 


class CkNtlmW : public CkNtlm
{
    private:
	unsigned long nextIdxW(void)
	    {
	    m_resultIdxW++;
	    if (m_resultIdxW >= 10)
		{
		m_resultIdxW = 0;
		}
	    return m_resultIdxW;
	    }
	unsigned long m_resultIdxW;
	CkString m_resultStringW[10];

    public:
	CkNtlmW() : m_resultIdxW(0) { this->put_Utf8(true); }
	virtual ~CkNtlmW() { }
	void put_ClientChallengeW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ClientChallenge(s1.getUtf8());
	    }
	void put_DnsComputerNameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_DnsComputerName(s1.getUtf8());
	    }
	void put_DnsDomainNameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_DnsDomainName(s1.getUtf8());
	    }
	void put_DomainW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Domain(s1.getUtf8());
	    }
	void put_EncodingModeW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_EncodingMode(s1.getUtf8());
	    }
	void put_FlagsW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Flags(s1.getUtf8());
	    }
	void put_NetBiosComputerNameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_NetBiosComputerName(s1.getUtf8());
	    }
	void put_NetBiosDomainNameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_NetBiosDomainName(s1.getUtf8());
	    }
	void put_PasswordW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Password(s1.getUtf8());
	    }
	void put_ServerChallengeW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_ServerChallenge(s1.getUtf8());
	    }
	void put_TargetNameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_TargetName(s1.getUtf8());
	    }
	void put_UserNameW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_UserName(s1.getUtf8());
	    }
	void put_WorkstationW(const wchar_t *newVal)
	    {
	    CkString s1;
	    s1.appendU(newVal);
	    this->put_Workstation(s1.getUtf8());
	    }
	bool genType2W(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GenType2(sTemp1.getUtf8(),outStr);
		}
	bool genType3W(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->GenType3(sTemp1.getUtf8(),outStr);
		}
	bool parseType1W(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ParseType1(sTemp1.getUtf8(),outStr);
		}
	bool parseType2W(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ParseType2(sTemp1.getUtf8(),outStr);
		}
	bool parseType3W(const wchar_t *w1,CkString &outStr)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->ParseType3(sTemp1.getUtf8(),outStr);
		}
	bool saveLastErrorW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SaveLastError(sTemp1.getUtf8());
		}
	bool setFlagW(const wchar_t *w1,bool on)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->SetFlag(sTemp1.getUtf8(),on);
		}
	bool unlockComponentW(const wchar_t *w1)
		{
		CkString sTemp1; sTemp1.appendU(w1);
		return this->UnlockComponent(sTemp1.getUtf8());
		}
	const wchar_t *clientChallengeW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ClientChallenge(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *dnsComputerNameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_DnsComputerName(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *dnsDomainNameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_DnsDomainName(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *domainW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Domain(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *encodingModeW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_EncodingMode(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *flagsW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Flags(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *genType1W(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		
		this->GenType1(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *genType2W(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GenType2(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *genType3W(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->GenType3(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastErrorHtmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->LastErrorHtml(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastErrorTextW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->LastErrorText(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *lastErrorXmlW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->LastErrorXml(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *netBiosComputerNameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_NetBiosComputerName(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *netBiosDomainNameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_NetBiosDomainName(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *parseType1W(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->ParseType1(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *parseType2W(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->ParseType2(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *parseType3W(const wchar_t *w1)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		CkString sTemp1; sTemp1.appendU(w1);
		this->ParseType3(sTemp1.getUtf8(),m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *passwordW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Password(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *serverChallengeW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_ServerChallenge(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *targetNameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_TargetName(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *userNameW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_UserName(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}
	const wchar_t *workstationW(void)
		{
		unsigned long idx = nextIdxW(); m_resultStringW[idx].clear();
		this->get_Workstation(m_resultStringW[idx]);
		return m_resultStringW[idx].getUnicode();
		}

};
#endif
