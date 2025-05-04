#ifndef __NEW_UI_MASTER_SKILL_TREE_H__
#define __NEW_UI_MASTER_SKILL_TREE_H__

#pragma once

#include "NewUIBase.h"
#include "NewUIButton.h"

#define MAX_MASTER_SKILL_DATA 512

#define MAX_MASTER_SKILL_CATEGORY 3

class CSkillTreeInfo
{
public:
	CSkillTreeInfo()
	{
		this->SkillPoint = 0;
		this->SkillValue = 0.0f;
		this->SkillNextValue = 0.0f;
	};

	~CSkillTreeInfo()
	{
	};

	void SetSkillTreeInfo(BYTE point, float value, float nextvalue)
	{
		this->SkillPoint = point;
		this->SkillValue = value;
		this->SkillNextValue = nextvalue;
	}

	BYTE GetSkillPoint() { return this->SkillPoint; }
	float GetSkillValue() { return this->SkillValue; }
	float GetSkillNextValue() { return this->SkillNextValue; }
private:
	BYTE SkillPoint;
	float SkillValue;
	float SkillNextValue;
};

struct _MASTER_SKILLTREE_DATA
{
	WORD Index;
	WORD ClassCode;
	BYTE Group;
	BYTE RequiredPoints;
	BYTE MaxLevel;
	BYTE ArrowDirection;
	ActionSkillType RequireSkill[2];
	ActionSkillType Skill;
	float DefValue;
};

struct _MASTER_SKILL_TOOLTIP
{
	ActionSkillType SkillNumber;
	WORD ClassCode;
	wchar_t Info1[64];
	wchar_t Info2[256];
	wchar_t Info3[32];
	wchar_t Info4[64];
	wchar_t Info5[64];
	wchar_t Info6[64];
	wchar_t Info7[64];
};

struct _MASTER_SKILL_TOOLTIP_FILE
{
	_MASTER_SKILL_TOOLTIP_FILE()
	{
		sizeof(_MASTER_SKILL_TOOLTIP_FILE);
	}
	int SkillNumber;
	WORD ClassCode;
	char Info1[64];
	char Info2[256];
	char Info3[32];
	char Info4[64];
	char Info5[64];
	char Info6[64];
	char Info7[64];
};

namespace SEASON3B
{
	//size = 404
	class CNewUIMasterLevel : public CNewUIObj
	{
	public:
		enum IMAGE_LIST
		{
			IMAGE_MASTER_INTERFACE = BITMAP_INTERFACE_MASTER_BEGIN,
		};

		CNewUIMasterLevel();
		~CNewUIMasterLevel();
		void ClearSkillTreeInfo();
		BYTE GetConsumePoint();
		int GetCurSkillID();
		bool Create(CNewUIManager* pNewUIMng);
		void Release();
		void SetPos();
		void OpenMasterSkillTreeData(wchar_t* path);
		void OpenMasterSkillTooltip(wchar_t* path);
		void InitMasterSkillPoint();
		void SetMasterType(CLASS_TYPE Class);
		void SetMasterSkillTreeData();
		void SetMasterSkillToolTipData();
		bool SetMasterSkillTreeInfo(int index, int SkillPoint, float value, float nextvalue);
		int SetDivideString(wchar_t* text, int isItemTollTip, int TextNum, int iTextColor, int iTextBold, bool isPercent);
		bool Render();
		bool Update();
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		float GetLayerDepth();
		void LoadImages();
		void UnloadImages();
		void RenderText();
		void RenderIcon();
		void RenderToolTip();
		bool CheckMouse(int posx, int posy);
		bool CheckBtn();
		
		bool CheckSkillPoint(WORD mLevelUpPoint, const _MASTER_SKILLTREE_DATA skillData, WORD skillPoint);
		bool CheckParentSkill(ActionSkillType reqSkill1, ActionSkillType reqSkill2);
		bool CheckRankPoint(BYTE group, BYTE rank, BYTE skillPoint);
		bool CheckBeforeSkill(ActionSkillType skill, BYTE skillPoint);
		int GetBeforeSkillID(int index);
		int GetBaseSkillID(int index);
		void SkillUpgrade(int index, char point, float value, float nextvalue);
		//bool IsLearnSkill(int index);
		float GetSkillValue(ActionSkillType index);
		//float GetSkillCumulativeValue(int index, BYTE damage);
		//BYTE GetSkillLevel(int index);
		void RegisterSkillInCharacterAttribute(ActionSkillType index);
		void SetConsumePoint(BYTE ConsumePoint);
		void SetCurSkillID(int index);
		void ClearSkillTreeData();
		void ClearSkillTooltipData();
	private:
		int PosX;
		int PosY;
		int width;
		int height;

		DWORD ButtonX[MAX_MASTER_SKILL_CATEGORY];
		DWORD ButtonY[MAX_MASTER_SKILL_CATEGORY];
		CNewUIButton m_CloseBT;
		int CategoryPoint[MAX_MASTER_SKILL_CATEGORY];
		int skillPoint[MAX_MASTER_SKILL_CATEGORY][10];
		BYTE ConsumePoint;
		int CurSkillID;
		DWORD classCode;
		POINT categoryPos[MAX_MASTER_SKILL_CATEGORY];
		DWORD CategoryTextIndex;
		DWORD ClassNameTextIndex;
		std::map<ActionSkillType, CSkillTreeInfo> map_skilltreeinfo;
		std::map<ActionSkillType, _MASTER_SKILL_TOOLTIP> map_masterSkillToolTip;
	    std::map<BYTE, _MASTER_SKILLTREE_DATA> map_masterData;
		
		CNewUIManager* m_pNewUIMng;

		bool CheckAttributeArea(const _MASTER_SKILLTREE_DATA skillData);
	};
}

#endif