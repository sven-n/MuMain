// MossMerchant.h: interface for the CMossMerchant class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

enum eMossMerchantState
{
	MOSS_STATE_BLANK = 0,
	MOSS_STATE_EMPTY = 1,
	MOSS_STATE_START = 2,
};

struct MOSS_MERCHANT_START_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

struct MOSS_MERCHANT_ITEM_INFO
{
	int Index;
	int Group;
	int Option0;
	int Option1;
	int Option2;
	int Option3;
	int Option4;
	int Option5;
	int Option6;
	int GambleRate;
};

class CMossMerchant
{
public:
	CMossMerchant();
	virtual ~CMossMerchant();
	void Init();
	void Load(char* path);
	void MainProc();
	void ProcState_BLANK();
	void ProcState_EMPTY();
	void ProcState_START();
	void SetState(int state);
	void SetState_BLANK();
	void SetState_EMPTY();
	void SetState_START();
	void CheckSync();
	void SetMossMerchant();
	bool GambleItem(LPOBJ lpObj,int group);
private:
	int m_State;
	int m_RemainTime;
	int m_TargetTime;
	int m_TickCount;
	int m_MossMerchantIndex;
	std::vector<MOSS_MERCHANT_START_TIME> m_MossMerchantStartTime;
	std::vector<MOSS_MERCHANT_ITEM_INFO> m_MossMerchantItemInfo;

};

extern CMossMerchant gMossMerchant;
