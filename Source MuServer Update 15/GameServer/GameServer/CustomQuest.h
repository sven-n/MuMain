// CustomQuest.h: interface for the CCustomQuest class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Item.h"

#define MAX_CUSTOM_QUEST 50

struct CUSTOM_QUEST_CHARACTER
{
	int Index;
	int Level;
	int Reset;
	int MReset;
	int Zen;
	int Coin1;
	int Coin2;
	int Coin3;
	int VipLevel;
	int RankUser;
	int Kills;
};

struct CUSTOM_QUEST_ITEM
{
	int Index;
	int Category;
	int ItemIndex;
	int Level;
	int Luck;
	int Skill;
	int Quantity;
};

struct CUSTOM_QUEST_MONSTER
{
	int Index;
	int MonsterIndex;
	int Quantity;
};

struct CUSTOM_QUEST_REWARD
{
	int Index;
	int Value1;
	int Value2;
	int Value3;
};

struct CUSTOM_QUEST_REWARD_ITEM
{
	int Index;
	int ItemIndex;
	int ItemLevel;
	int ItemDurability;
	int ItemOption1;
	int ItemOption2;
	int ItemOption3;
	int ItemNewOption;
	int AncOption;
	int JOH;
	int OpEx;
	int Socket1;
	int Socket2;
	int Socket3;
	int Socket4;
	int Socket5;
	int Duration;
	int Class;
};

struct CUSTOM_QUEST_REWARD_BUFF
{
	int Index;
	int EffectID;
	int Power1;
	int Power2;
	int Time;
};

struct CUSTOM_QUEST_REWARD_EXP
{
	int Index;
	int Experience;
};

struct SDHP_CUSTOMQUEST_SEND
{
    PBMSG_HEAD header; // C1:F1
    WORD index;
    char name[11];
};

struct SDHP_CUSTOMQUEST_SAVE_SEND
{
    PBMSG_HEAD header; // C1:F2
    WORD index;
    char name[11];
	DWORD quest;
};

struct SDHP_CUSTOMQUEST_RECV
{
    PBMSG_HEAD header; // C1:F1
    WORD index;
    char name[11];
	DWORD quest;
};

class CCustomQuest
{
public:
	CCustomQuest();
	virtual ~CCustomQuest();
	void Init();
	void Load(char* path);
	bool CheckAll(int quest);
	bool CheckCharacter(LPOBJ lpObj,int quest,int Npc);
	bool CheckItem(LPOBJ lpObj,int quest,int Npc);
	bool CheckMonster(LPOBJ lpObj,int quest,int Npc);
	bool CheckItemRewardInventorySpace(LPOBJ lpObj,int quest,int Npc);
	bool CheckItemInventorySpace(LPOBJ lpObj,int index);
	void RemoveItem(LPOBJ lpObj,int quest);
	void RemoveMoney(LPOBJ lpObj,int quest);
	void AddRewardCoin(LPOBJ lpObj,int quest);
	void AddRewardItem(LPOBJ lpObj,int quest);
	void AddRewardBuff(LPOBJ lpObj,int quest);
	void AddRewardExperience(LPOBJ lpObj,int quest);
	void QuestCommand(LPOBJ lpObj,char* arg,int Npc);
	void DGCustomQuestRecv(SDHP_CUSTOMQUEST_RECV* lpMsg);
	void DGCustomQuestSend(int aIndex);
private:
	std::vector<CUSTOM_QUEST_CHARACTER>		m_CustomQuestCharacterInfo;
	std::vector<CUSTOM_QUEST_ITEM>			m_CustomQuestItemInfo;
	std::vector<CUSTOM_QUEST_MONSTER>		m_CustomQuestMonsterInfo;
	std::vector<CUSTOM_QUEST_REWARD>		m_CustomQuestRewardInfo;
	std::vector<CUSTOM_QUEST_REWARD_ITEM>	m_CustomQuestRewardItemInfo;
	std::vector<CUSTOM_QUEST_REWARD_BUFF>	m_CustomQuestRewardBuffInfo;
	std::vector<CUSTOM_QUEST_REWARD_EXP>	m_CustomQuestRewardExpInfo;
private:
	int m_LastPosX;
	int m_LastPosY;
};

extern CCustomQuest gCustomQuest;