// Reconnect.h: interface for the CReconnect class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "GensSystem.h"
#include "User.h"

struct RECONNECT_INFO
{
	char Name[11];
	DWORD ReconnectTime;
	int PartyNumber;
	int AutoAddPointCount;
	int AutoAddPointStats[5];
	int AutoResetEnable;
	int AutoResetStats[5];
	int RequestOption;
	char AutoPartyPassword[11];
	GENS_SYSTEM_VICTIM_LIST GensVictimList[MAX_GENS_SYSTEM_VICTIM];
};

class CReconnect
{
public:
	CReconnect();
	virtual ~CReconnect();
	void MainProc();
	void SetReconnectInfo(LPOBJ lpObj);
	void GetReconnectInfo(LPOBJ lpObj);
	bool CheckResumeParty(RECONNECT_INFO* lpInfo);
	void ResumeParty(LPOBJ lpObj,RECONNECT_INFO* lpInfo);
	void ResumeCommand(LPOBJ lpObj,RECONNECT_INFO* lpInfo);
	void ResumeGensVictimList(LPOBJ lpObj,RECONNECT_INFO* lpInfo);
private:
	std::map<std::string,RECONNECT_INFO> m_ReconnectInfo;
};

extern CReconnect gReconnect;
