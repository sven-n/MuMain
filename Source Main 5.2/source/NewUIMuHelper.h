#pragma once

#include <array>
#include <vector>

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIButton.h"
#include "MUHelper/MuHelper.h"

namespace SEASON3B
{
    class CNewUIMuHelper : public CNewUIObj
    {
    public:
        CNewUIMuHelper();
        ~CNewUIMuHelper();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void Show(bool bShow);
        bool Render();
        bool Update();
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();

        float GetLayerDepth();
        float GetKeyEventOrder();

    public:
        void Reset();
        void LoadSavedConfig(const MUHelper::ConfigData& config);
        void AssignSkill(int iSkill);
        static int GetIntFromTextInput(wchar_t* pstrInput);

    private:
        enum IMAGE_LIST
        {
            IMAGE_BASE_WINDOW_BACK = BITMAP_INTERFACE_NEW_MESSAGEBOX_BEGIN + 3,				//. newui_msgbox_back.jpg
            IMAGE_BASE_WINDOW_TOP = BITMAP_INTERFACE_NEW_PERSONALINVENTORY_BEGIN,			//. newui_item_back01.tga	(190,64)
            IMAGE_BASE_WINDOW_LEFT = BITMAP_INTERFACE_NEW_PERSONALINVENTORY_BEGIN + 2,		//. newui_item_back02-l.tga	(21,320)
            IMAGE_BASE_WINDOW_RIGHT = BITMAP_INTERFACE_NEW_PERSONALINVENTORY_BEGIN + 3,		//. newui_item_back02-r.tga	(21,320)
            IMAGE_BASE_WINDOW_BOTTOM = BITMAP_INTERFACE_NEW_PERSONALINVENTORY_BEGIN + 4,	//. newui_item_back03.tga	(190,45)
            IMAGE_BASE_WINDOW_BTN_EXIT = BITMAP_INTERFACE_NEW_PERSONALINVENTORY_BEGIN + 17,		//. newui_exit_00.tga
            //--
            IMAGE_TABLE_SQUARE = BITMAP_INTERFACE_NEW_INVENTORY_BASE_BEGIN,	//. newui_item_box.tga
            IMAGE_TABLE_TOP_LEFT,			//. newui_item_table01(L).tga (14,14)
            IMAGE_TABLE_TOP_RIGHT,			//. newui_item_table01(R).tga (14,14)
            IMAGE_TABLE_BOTTOM_LEFT,		//. newui_item_table02(L).tga (14,14)
            IMAGE_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)
            IMAGE_TABLE_TOP_PIXEL,			//. newui_item_table03(up).tga (1, 14)
            IMAGE_TABLE_BOTTOM_PIXEL,	//. newui_item_table03(dw).tga (1,14)
            IMAGE_TABLE_LEFT_PIXEL,		//. newui_item_table03(L).tga (14,1)
            IMAGE_TABLE_RIGHT_PIXEL,		//. newui_item_table03(R).tga (14,1)
            //--
            IMAGE_WINDOW_TAB_BTN = BITMAP_GUILDINFO_BEGIN,
            //--
            IMAGE_MACROUI_HELPER_RAGEMINUS = BITMAP_INTERFACE_MACROUI_BEGIN,		// newui_position02.tga			(70, 25)
            IMAGE_MACROUI_HELPER_OPTIONBUTTON = BITMAP_INTERFACE_MACROUI_BEGIN + 1,		// newui_position02.tga			(70, 25)
            IMAGE_MACROUI_HELPER_INPUTNUMBER = BITMAP_INTERFACE_MACROUI_BEGIN + 2,
            IMAGE_MACROUI_HELPER_INPUTSTRING = BITMAP_INTERFACE_MACROUI_BEGIN + 3,
            //-- Buttons
            IMAGE_CHAINFO_BTN_STAT = BITMAP_INTERFACE_NEW_CHAINFO_WINDOW_BEGIN + 1,
            IMAGE_CLEARNESS_BTN = BITMAP_CURSEDTEMPLE_BEGIN + 4,
            IMAGE_IGS_BUTTON = BITMAP_IGS_MSGBOX_BUTTON,
            IMAGE_CHECKBOX_BTN = BITMAP_OPTION_BEGIN + 5,

            //-- Skills
            IMAGE_SKILL1 = BITMAP_INTERFACE_NEW_SKILLICON_BEGIN,
            IMAGE_SKILL2,
            IMAGE_COMMAND,
            IMAGE_SKILL3,
            IMAGE_SKILLBOX,
            IMAGE_SKILLBOX_USE,
            IMAGE_NON_SKILL1,
            IMAGE_NON_SKILL2,
            IMAGE_NON_COMMAND,
            IMAGE_NON_SKILL3,
        };

        enum CLASS_LIST_ {
            Dark_Wizard = 0,
            Dark_Knight,
            Fairy_Elf,
            Magic_Gladiator,
            Dark_Lord,
            Summoner,
            Rage_Fighter,
        };

        static constexpr int MAX_SKILLS_SLOT = 6;
        static constexpr int WINDOW_WIDTH = 190;
        static constexpr int WINDOW_HEIGHT = 429;

        typedef struct
        {
            int iNumTab;
            BYTE class_character[MAX_CLASS];
            CNewUIButton* btn;
        } CButtonTap;

        typedef struct
        {
            int iNumTab;
            BYTE class_character[MAX_CLASS];
            CNewUICheckBox* box;
        } CheckBoxTap;

        typedef struct
        {
            int iNumTab;
            int s_ImgIndex;
            POINT m_Pos;
            POINT m_Size;
            BYTE class_character[MAX_CLASS];
        } cTexture;

        typedef struct
        {
            int iNumTab;
            POINT m_Pos;
            std::wstring m_Name;
            BYTE class_character[MAX_CLASS];
        } cTextName;

        typedef std::map<int, CButtonTap> cButtonMap;
        typedef std::map<int, CheckBoxTap> cCheckBoxMap;
        typedef std::map<int, cTexture> cTextureMap;
        typedef std::map<int, cTextName> cTextNameMap;

    private:
        void RenderBtnList();
        int UpdateMouseBtnList();
        void RegisterBtnCharacter(BYTE class_character, int Identifier);
        void RegisterButton(int Identifier, CButtonTap button);
        void InsertButton(int imgindex, int x, int y, int sx, int sy, bool overflg, bool isimgwidth, bool bClickEffect, bool MoveTxt,std::wstring btname,std::wstring tooltiptext, int Identifier, int iNumTab);
        //--
        void RenderBoxList();
        int UpdateMouseBoxList();
        void RegisterBoxCharacter(BYTE class_character, int Identifier);
        void RegisterCheckBox(int Identifier, CheckBoxTap button);
        void InsertCheckBox(int imgindex, int x, int y, int sx, int sy, bool overflg,std::wstring btname, int Identifier, int iNumTab);
        //--
        void RenderIconList();
        int UpdateMouseIconList();
        void RegisterIconCharacter(BYTE class_character, int Identifier);
        void RegisterIcon(int Identifier, cTexture button);
        void InsertIcon(int imgindex, int x, int y, int sx, int sy, int Identifier, int iNumTab);
        //--
        void RenderTextList();
        void RegisterTextCharacter(BYTE class_character, int Identifier);
        void RegisterText(int Identifier, cTextName button);
        void InsertText(int x, int y,std::wstring Name, int Identifier, int iNumTab);

        void InitText();
        void InitImage();
        void InitButtons();
        void InitCheckBox();
        void InitTextboxInput();
        void SetPos(int x, int y);
        void RenderBack(int x, int y, int width, int height);

        int GetSkillIndex(int iSkill);
        bool IsSkillAssigned(int iSkill);
        void RenderSkillIcon(int iSkill, float x, float y, float width, float height);
        
        void InitConfig();
        void SaveConfig();
        void ApplyConfig();

        void LoadImages();
        void UnloadImages();

        void ApplyConfigFromCheckbox(int iCheckboxId, bool bState);
        void ApplyConfigFromSkillSlot(int iSlot, int iSkill);
        void ApplyHuntRangeUpdate(int iDelta);
        void ApplyLootRangeUpdate(int iDelta);
        void SaveExtraItem();
        void RemoveExtraItem();

    private:
        CNewUIManager* m_pNewUIMng;
        CUITextInputBox m_DistanceTimeInput;
        CUITextInputBox m_Skill2DelayInput;
        CUITextInputBox m_Skill3DelayInput;
        CUITextInputBox m_ItemInput;
        CUIExtraItemListBox m_ItemFilter;

        POINT m_Pos;
        POINT m_SubPos;
        CNewUIRadioGroupButton m_TabBtn;
        int m_iCurrentOpenTab;
        int m_iCurrentOpenSubWin;
        bool m_bSubWinOpen;
        cButtonMap m_ButtonList;
        cCheckBoxMap m_CheckBoxList;
        cTextNameMap m_TextNameList;
        cTextureMap m_IconList;
        int m_iSelectedSkillSlot;
        std::array<int, MAX_SKILLS_SLOT> m_aiSelectedSkills;
    };

    class CNewUIMuHelperSkillList : public CNewUIObj
    {

    public:
        CNewUIMuHelperSkillList();
        ~CNewUIMuHelperSkillList();

        bool Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng);
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        void RenderSkillInfo();
        float GetLayerDepth();

        void Reset();

        int UpdateMouseSkillList();
        void FilterByAttackSkills();
        void FilterByBuffSkills();

        static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);

    private:
        enum IMAGE_LIST
        {
            IMAGE_SKILL1 = BITMAP_INTERFACE_NEW_SKILLICON_BEGIN,
            IMAGE_SKILL2,
            IMAGE_COMMAND,
            IMAGE_SKILL3,
            IMAGE_SKILLBOX,
            IMAGE_SKILLBOX_USE,
            IMAGE_NON_SKILL1,
            IMAGE_NON_SKILL2,
            IMAGE_NON_COMMAND,
            IMAGE_NON_SKILL3,
        };

        enum EVENT_STATE
        {
            EVENT_NONE = 0,

            EVENT_BTN_HOVER_SKILLLIST,
            EVENT_BTN_DOWN_SKILLLIST,
        };

        typedef struct
        {
            int skillId;
            POINT location;
            SIZE area;
            bool isVisible;
        } cSkillIcon;

    private:
        void LoadImages();
        void UnloadImages();

        void PrepareSkillsToRender();
        void RenderSkillIcon(int iIndex, float x, float y, float width, float height);

        bool IsAttackSkill(int iSkillType);
        bool IsBuffSkill(int iSkillType);
        bool IsHealingSkill(int iSkillType);
        bool IsDefenseSkill(int iSkillType);

    private:
        std::map<int, cSkillIcon> m_skillIconMap;

        CNewUIManager* m_pNewUIMng;
        CNewUI3DRenderMng* m_pNewUI3DRenderMng;

        bool m_bFilterByAttackSkills;
        bool m_bFilterByBuffSkills;
        bool m_bRenderSkillInfo;
        int m_iRenderSkillInfoType;
        int m_iRenderSkillInfoPosX;
        int m_iRenderSkillInfoPosY;
        std::vector<int> m_aiSkillsToRender;

        EVENT_STATE m_EventState;
    };

    class CNewUIMuHelperExt : public CNewUIObj
    {
    public:
        CNewUIMuHelperExt();
        ~CNewUIMuHelperExt();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        bool Render();
        bool Update();
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();

        float GetLayerDepth();
        float GetKeyEventOrder();

    public:
        void Toggle(int iPage);
        void Save();
        void Reset();
        void ApplySavedConfig();
        void InitConfig();

    private:
        enum IMAGE_LIST
        {
            IMAGE_BASE_WINDOW_BACK = BITMAP_INTERFACE_NEW_MESSAGEBOX_BEGIN + 3,				//. newui_msgbox_back.jpg
            IMAGE_BASE_WINDOW_TOP = BITMAP_INTERFACE_NEW_PERSONALINVENTORY_BEGIN,			//. newui_item_back01.tga	(190,64)
            IMAGE_BASE_WINDOW_LEFT = BITMAP_INTERFACE_NEW_PERSONALINVENTORY_BEGIN + 2,		//. newui_item_back02-l.tga	(21,320)
            IMAGE_BASE_WINDOW_RIGHT = BITMAP_INTERFACE_NEW_PERSONALINVENTORY_BEGIN + 3,		//. newui_item_back02-r.tga	(21,320)
            IMAGE_BASE_WINDOW_BOTTOM = BITMAP_INTERFACE_NEW_PERSONALINVENTORY_BEGIN + 4,	//. newui_item_back03.tga	(190,45)
            IMAGE_BASE_WINDOW_BTN_EXIT = BITMAP_INTERFACE_NEW_PERSONALINVENTORY_BEGIN + 17,		//. newui_exit_00.tga
            //--
            IMAGE_TABLE_SQUARE = BITMAP_INTERFACE_NEW_INVENTORY_BASE_BEGIN,	//. newui_item_box.tga
            IMAGE_TABLE_TOP_LEFT,			//. newui_item_table01(L).tga (14,14)
            IMAGE_TABLE_TOP_RIGHT,			//. newui_item_table01(R).tga (14,14)
            IMAGE_TABLE_BOTTOM_LEFT,		//. newui_item_table02(L).tga (14,14)
            IMAGE_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)
            IMAGE_TABLE_TOP_PIXEL,			//. newui_item_table03(up).tga (1, 14)
            IMAGE_TABLE_BOTTOM_PIXEL,	//. newui_item_table03(dw).tga (1,14)
            IMAGE_TABLE_LEFT_PIXEL,		//. newui_item_table03(L).tga (14,1)
            IMAGE_TABLE_RIGHT_PIXEL,		//. newui_item_table03(R).tga (14,1)
            //--
            IMAGE_WINDOW_TAB_BTN = BITMAP_GUILDINFO_BEGIN,
            //--
            IMAGE_MACROUI_HELPER_RAGEMINUS = BITMAP_INTERFACE_MACROUI_BEGIN,		// newui_position02.tga			(70, 25)
            IMAGE_MACROUI_HELPER_OPTIONBUTTON = BITMAP_INTERFACE_MACROUI_BEGIN + 1,		// newui_position02.tga			(70, 25)
            IMAGE_MACROUI_HELPER_INPUTNUMBER = BITMAP_INTERFACE_MACROUI_BEGIN + 2,
            IMAGE_MACROUI_HELPER_INPUTSTRING = BITMAP_INTERFACE_MACROUI_BEGIN + 3,
            //-- Buttons
            IMAGE_CHAINFO_BTN_STAT = BITMAP_INTERFACE_NEW_CHAINFO_WINDOW_BEGIN + 1,
            IMAGE_CLEARNESS_BTN = BITMAP_CURSEDTEMPLE_BEGIN + 4,
            IMAGE_IGS_BUTTON = BITMAP_IGS_MSGBOX_BUTTON,

            IMAGE_OPTION_BTN_CHECK = BITMAP_OPTION_BEGIN + 5,
            IMAGE_OPTION_VOLUME_BACK = BITMAP_OPTION_BEGIN + 8,
            IMAGE_OPTION_VOLUME_COLOR = BITMAP_OPTION_BEGIN + 9,

        };

        static constexpr int WINDOW_WIDTH = 190;
        static constexpr int WINDOW_HEIGHT = 429;

    private:
        void InitText();
        void InitImage();
        void InitButtons();
        void InitCheckBox();
        void SetPos(int x, int y);
        void RenderBackPane(int x, int y, int width, int height, const wchar_t* pszHeader);
        void RenderHpLevel(int x, int y, int width, int height, int level, const wchar_t* pszLabel);
        void LoadImages();
        void UnloadImages();

    private:
        CNewUIManager* m_pNewUIMng;

        POINT m_Pos;
        CNewUICheckBox m_BtnPreConHuntRange;
        CNewUICheckBox m_BtnPreConAttacking;

        CNewUICheckBox m_BtnSubConMoreThanTwo;
        CNewUICheckBox m_BtnSubConMoreThanThree;
        CNewUICheckBox m_BtnSubConMoreThanFour;
        CNewUICheckBox m_BtnSubConMoreThanFive;

        CNewUICheckBox m_BtnPartyHeal;
        CNewUICheckBox m_BtnPartyDuration;
        CUITextInputBox m_BuffTimeInput;

        CNewUIButton m_BtnSave;
        CNewUIButton m_BtnReset;
        CNewUIButton m_BtnClose;

    private:
        int m_iCurrentPage;
        int m_iCurrentHealThreshold;
        int m_iCurrentPartyHealThreshold;
        int m_iCurrentPotionThreshold;
    };

}
