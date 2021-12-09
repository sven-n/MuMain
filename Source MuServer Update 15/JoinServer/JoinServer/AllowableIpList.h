// AllowableIpList.h: interface for the CAllowableIpList class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

struct ALLOWABLE_IP_INFO
{
	char IpAddr[32];
};

class CAllowableIpList
{
public:
	CAllowableIpList();
	virtual ~CAllowableIpList();
	void Load(char* path);
	bool CheckAllowableIp(char* ip);
private:
	std::map<std::string,ALLOWABLE_IP_INFO> m_AllowableIpInfo;
};

extern CAllowableIpList gAllowableIpList;
