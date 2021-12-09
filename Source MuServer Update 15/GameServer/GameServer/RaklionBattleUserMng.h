// RaklionBattleUserMng.h: interface for the CRaklionBattleUserMng class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CRaklionBattleUserMng
{
public:
	CRaklionBattleUserMng();
	virtual ~CRaklionBattleUserMng();
	void ResetAllData();
	void AddUserData(int aIndex);
	void DelUserData(int aIndex);
	void CheckUserState();
	void CheckUserOnRaklionBossMap();
	bool IsBattleUser(int aIndex);
	bool CheckBattleUsers();
	int GetBattleUserCount();
	int GetBattleUserIndex(int index);
public:
	std::vector<int> m_UserInfo;
};

extern CRaklionBattleUserMng gRaklionBattleUserMng;
