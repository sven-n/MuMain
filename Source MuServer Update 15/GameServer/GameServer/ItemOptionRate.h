// ItemOptionRate.h: interface for the CItemOptionRate class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_ITEM_OPTION0_RATE 16
#define MAX_ITEM_OPTION1_RATE 2
#define MAX_ITEM_OPTION2_RATE 2
#define MAX_ITEM_OPTION3_RATE 8
#define MAX_ITEM_OPTION4_RATE 7
#define MAX_ITEM_OPTION5_RATE 3
#define MAX_ITEM_OPTION6_RATE 6

struct ITEM_OPTION0_RATE_INFO
{
	int Index;
	int Rate[MAX_ITEM_OPTION0_RATE];
};

struct ITEM_OPTION1_RATE_INFO
{
	int Index;
	int Rate[MAX_ITEM_OPTION1_RATE];
};

struct ITEM_OPTION2_RATE_INFO
{
	int Index;
	int Rate[MAX_ITEM_OPTION2_RATE];
};

struct ITEM_OPTION3_RATE_INFO
{
	int Index;
	int Rate[MAX_ITEM_OPTION3_RATE];
};

struct ITEM_OPTION4_RATE_INFO
{
	int Index;
	int Rate[MAX_ITEM_OPTION4_RATE];
};

struct ITEM_OPTION5_RATE_INFO
{
	int Index;
	int Rate[MAX_ITEM_OPTION5_RATE];
};

struct ITEM_OPTION6_RATE_INFO
{
	int Index;
	int Rate[MAX_ITEM_OPTION6_RATE];
};

class CItemOptionRate
{
public:
	CItemOptionRate();
	virtual ~CItemOptionRate();
	void Load(char* path);
	bool GetItemOption0(int index,BYTE* option);
	bool GetItemOption1(int index,BYTE* option);
	bool GetItemOption2(int index,BYTE* option);
	bool GetItemOption3(int index,BYTE* option);
	bool GetItemOption4(int index,BYTE* option);
	bool GetItemOption5(int index,BYTE* option);
	bool GetItemOption6(int index,BYTE* option);
	bool MakeNewOption(int ItemIndex,int value,BYTE* option);
	bool MakeSetOption(int ItemIndex,int value,BYTE* option);
	bool MakeSocketOption(int ItemIndex,int value,BYTE* option);
private:
	std::map<int,ITEM_OPTION0_RATE_INFO> m_ItemOption0RateInfo;
	std::map<int,ITEM_OPTION1_RATE_INFO> m_ItemOption1RateInfo;
	std::map<int,ITEM_OPTION2_RATE_INFO> m_ItemOption2RateInfo;
	std::map<int,ITEM_OPTION3_RATE_INFO> m_ItemOption3RateInfo;
	std::map<int,ITEM_OPTION4_RATE_INFO> m_ItemOption4RateInfo;
	std::map<int,ITEM_OPTION5_RATE_INFO> m_ItemOption5RateInfo;
	std::map<int,ITEM_OPTION6_RATE_INFO> m_ItemOption6RateInfo;
};

extern CItemOptionRate gItemOptionRate;
