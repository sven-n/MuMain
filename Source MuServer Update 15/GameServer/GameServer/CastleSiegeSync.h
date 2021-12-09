// CastleSiegeSync.h: interface for the CCastleSiegeSync class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_TRIBUTE_MONEY MAX_MONEY

class CCastleSiegeSync
{
public:
	CCastleSiegeSync();
	virtual ~CCastleSiegeSync();
	void Clear();
	int GetCastleState();
	int GetTaxRateChaos(int aIndex);
	int GetTaxRateStore(int aIndex);
	int GetTaxHuntZone(int aIndex,bool CheckOwnerGuild);
	char* GetCastleOwnerGuild();
	bool CheckCastleOwnerMember(int aIndex);
	bool CheckCastleOwnerUnionMember(int aIndex);
	void ResetTributeMoney();
	void AddTributeMoney(int money);
	void AdjustTributeMoney();
	void SetCastleOwnerGuild(char* GuildName);
	void SetCastleState(int state);
	void SetTaxRateChaos(int rate);
	void SetTaxRateStore(int rate);
	void SetTaxHuntZone(int rate);
	int GetTributeMoney();
private:
	int m_CurCastleState;
	int m_CurTaxRateChaos;
	int m_CurTaxRateStore;
	int m_CurTaxHuntZone;
	int m_CastleTributeMoney;
	char m_CastleOwnerGuild[16];
	int m_CsTributeMoneyTimer;
};

extern CCastleSiegeSync gCastleSiegeSync;
