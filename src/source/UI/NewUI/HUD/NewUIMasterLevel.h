#ifndef __NEW_UI_MASTER_SKILL_TREE_H__
#define __NEW_UI_MASTER_SKILL_TREE_H__

#pragma once

#include "UI/NewUI/NewUIBase.h"
#include "UI/NewUI/Widgets/NewUIButton.h"

constexpr auto MAX_MASTER_SKILL_DATA = 512;

constexpr auto MAX_MASTER_SKILL_CATEGORY = 3;
constexpr auto MAX_MASTER_SKILL_REQUIRES = 2;
constexpr auto MASTER_SKILL_LEVEL_REQ_FOR_NEXT_RANK = 10;
constexpr auto MAX_MASTER_TREE_RANK = 10;

// One master skill a build wants at a given level. The tree requirements
// (parent skill, rank gate, base skill) are resolved by the simulation, so a
// preset only has to name the skills it actually cares about, in the order it
// wants them filled.
struct MASTER_SUGGEST_ENTRY
{
    ActionSkillType SkillNumber;
    BYTE TargetLevel;
};

// The points a single skill would receive. The list keeps the order in which
// the simulation spent them, because the server validates every request
// against the tree - a parent skill has to be sent before its child.
struct MASTER_SUGGEST_RESULT
{
    ActionSkillType SkillNumber;
    int Points;
};

enum MASTER_SKILL_TREE_CLASS : WORD
{
    MASTER_SKILL_TREE_CLASS_NONE = 0,
    MASTER_SKILL_TREE_CLASS_BLADEMASTER = 1,
    MASTER_SKILL_TREE_CLASS_GRANDMASTER = 2,
    
    MASTER_SKILL_TREE_CLASS_HIGHELF = 4,
    MASTER_SKILL_TREE_CLASS_DIMENSIONMASTER = 8,
    MASTER_SKILL_TREE_CLASS_DUELMASTER = 16,
    MASTER_SKILL_TREE_CLASS_LORDEMPEROR = 32,
    MASTER_SKILL_TREE_CLASS_TEMPLEKNIGHT = 64,
};

DEFINE_ENUM_FLAG_OPERATORS(MASTER_SKILL_TREE_CLASS);

struct _MASTER_SKILLTREE_DATA
{
    WORD Index;
    MASTER_SKILL_TREE_CLASS ClassCode;
    BYTE Group;
    BYTE RequiredPoints;
    BYTE MaxLevel;
    BYTE ArrowDirection;
    ActionSkillType RequireSkill[MAX_MASTER_SKILL_REQUIRES];
    ActionSkillType Skill;
    float DefValue;
};

struct _MASTER_SKILL_TOOLTIP
{
    ActionSkillType SkillNumber;
    MASTER_SKILL_TREE_CLASS ClassCode;
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
    int SkillNumber;
    MASTER_SKILL_TREE_CLASS ClassCode;
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
            IMAGE_MASTER_BTN_SUGGEST = BITMAP_INTERFACE_MASTER_BEGIN + 14,
        };

        // One build recommendation: the skills to fill, in priority order.
        struct SUGGEST_PRESET
        {
            const wchar_t* const* s_pNameSlot;
            const MASTER_SUGGEST_ENTRY* s_pEntries;
            int s_iEntryCount;
        };

        CNewUIMasterLevel();
        ~CNewUIMasterLevel() override;

        BYTE GetConsumePoint() const;
        int GetCurSkillID() const;
        bool Create(CNewUIManager* pNewUIMng);
        void Release();
        
        void OpenMasterSkillTreeData(const wchar_t* path);
        void OpenMasterSkillTooltip(const wchar_t* path);
        void InitMasterSkillPoint();
        void SetMasterType(CLASS_TYPE Class);

        bool SetMasterSkillTreeInfo(int index, BYTE skillLevel, float value, float nextValue);
        void SkillUpgrade(int index, BYTE skillLevel, float value, float nextValue);

        bool Render() override;
        bool Update() override;
        bool UpdateMouseEvent() override;
        bool UpdateKeyEvent() override;
        float GetLayerDepth() override;

    private:
        int PosX;
        int PosY;
        int width;
        int height;

        DWORD ButtonX[MAX_MASTER_SKILL_CATEGORY];
        DWORD ButtonY[MAX_MASTER_SKILL_CATEGORY];
        CNewUIButton m_CloseBT;
        CNewUIButton m_BtnSuggestPreset;
        CNewUIButton m_BtnSuggestApply;
        // 0 = suggestion off, otherwise 1-based index into the class preset table.
        int m_iSuggestPreset;
        MASTER_SKILL_TREE_CLASS m_eSuggestPresetClass;
        std::vector<MASTER_SUGGEST_RESULT> m_vSuggestPoint;
        int CategoryPoint[MAX_MASTER_SKILL_CATEGORY];
        int skillPoint[MAX_MASTER_SKILL_CATEGORY][MAX_MASTER_TREE_RANK];
        BYTE ConsumePoint;
        int CurSkillID;
        MASTER_SKILL_TREE_CLASS classCode;
        POINT categoryPos[MAX_MASTER_SKILL_CATEGORY];
        DWORD CategoryTextIndex;
        DWORD ClassNameTextIndex;

        std::map<ActionSkillType, _MASTER_SKILL_TOOLTIP> map_masterSkillToolTip;
        std::map<BYTE, _MASTER_SKILLTREE_DATA> map_masterData;
        
        CNewUIManager* m_pNewUIMng;

        void SetPos();
        int SetDivideString(wchar_t* text, int isItemTollTip, int TextNum, int iTextColor, int iTextBold, bool isPercent);
        void LoadImages();
        void UnloadImages();

        bool CheckAttributeArea(const _MASTER_SKILLTREE_DATA& skillData);
        bool CheckSkillPoint(WORD mLevelUpPoint, const _MASTER_SKILLTREE_DATA& skillData, BYTE skillLevel);
        bool CheckParentSkill(const _MASTER_SKILLTREE_DATA& masterSkill);
        bool CheckRankPoint(BYTE group, BYTE rank, BYTE skillLevel);
        bool CheckBeforeSkill(ActionSkillType skill, BYTE skillLevel);

        int GetBeforeSkillID(int index);

        const SUGGEST_PRESET* GetPresetTable(int& count) const;
        const _MASTER_SKILLTREE_DATA* FindSkillData(ActionSkillType skill) const;
        int GetSuggestedPoints(ActionSkillType skill) const;
        int GetSimulatedLevel(ActionSkillType skill) const;
        int GetSimulatedRankLevel(BYTE group, BYTE rank) const;
        void AddSuggestedPoints(ActionSkillType skill, int points);
        bool EnsureRequirements(const _MASTER_SKILLTREE_DATA& skillData, int& freePoints, int depth);
        void SpendSuggestedPoints(ActionSkillType skill, int targetLevel, int& freePoints, int depth);
        void ResetSuggestion();
        void CycleSuggestPreset();
        void CalcSuggestion();
        void ApplySuggestedPoints();
        void RenderSuggestButtons();

        void SetMasterSkillTreeData();
        void SetMasterSkillToolTipData();

        void ClearSkillTreeData();
        void ClearSkillTooltipData();

        void RenderText() const;
        void RenderIcon();
        void RenderToolTip();
        bool CheckMouse(int posx, int posy);
        bool CheckBtn();
    };
}

#endif