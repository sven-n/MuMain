// NewUISiegeWarBase.h: interface for the CNewUISiegeWarBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUISIEGEWARBASE_H__13F2F04C_290F_41FC_A7A5_2F56F22B3478__INCLUDED_)
#define AFX_NEWUISIEGEWARBASE_H__13F2F04C_290F_41FC_A7A5_2F56F22B3478__INCLUDED_

#pragma once

#define MAX_COMMANDGROUP ( 7 )

#include "NewUIButton.h"
#include "NewUIMainFrameWindow.h"

namespace SEASON3B
{
    class CNewUISiegeWarBase
    {
    public:
        enum IMAGE_LIST
        {
            // MiniMap
            IMAGE_SKILL_ICON = CNewUISkillList::IMAGE_SKILL2,	// newui_skill2.jpg
            IMAGE_MINIMAP_FRAME = BITMAP_SIEGEWAR_BEGIN,		// newui_SW_Minimap_Frame.tga		(154, 162)
            IMAGE_TIME_FRAME,									// newui_SW_Time_Frame.tga			(134, 37)
            IMAGE_MINIMAP,										// map1.tga							(256, 256)
            IMAGE_COMMAND_ATTACK,								// i_attack.tga		(13, 13)
            IMAGE_COMMAND_DEFENCE,								// i_defense.tga	(18, 15)
            IMAGE_COMMAND_WAIT,									// i_wait.tga		(11, 12)
            IMAGE_BATTLESKILL_FRAME, 							// newui_SW_BattleSkill_Frame.tga	(128, 53)
            IMAGE_SKILL_BTN_SCROLL_UP,							// newui_Bt_skill_scroll_up.jpg		(15, 13)
            IMAGE_SKILL_BTN_SCROLL_DN,							// newui_Bt_skill_scroll_dn.jpg		(15, 13)
            IMAGE_BTN_ALPHA,									// newui_SW_MiniMap_Bt_clearness.jpg (38, 21)
            IMAGE_SIEGEWAR_BASE_FRAME_END,
        };

        enum FRAME_SIZE
        {
            MINIMAP_FRAME_WIDTH = 154,
            MINIMAP_FRAME_HEIGHT = 162,
            TIME_FRAME_WIDTH = 134,
            TIME_FRAME_HEIGHT = 37,
            MINIMAP_BTN_ALPHA_WIDTH = 30,
            MINIMAP_BTN_ALPHA_HEIGHT = 22,
            COMMAND_ATTACK_WIDTH = 13,
            COMMAND_ATTACK_HEIGHT = 13,
            COMMAND_DEFENCE_WIDTH = 18,
            COMMAND_DEFENCE_HEIGHT = 15,
            COMMAND_WAIT_WIDTH = 11,
            COMMAND_WAIT_HEIGHT = 12,
            BATTLESKILL_FRAME_WIDTH = 128,
            BATTLESKILL_FRAME_HEIGHT = 53,
            SKILL_BTN_SCROLL_WIDTH = 15,
            SKILL_BTN_SCROLL_HEIGHT = 13,
            SKILL_ICON_WIDTH = 20,
            SKILL_ICON_HEIGHT = 28,
            SKILL_TOOLTIP_WIDTH = 128,
            SKILL_TOOLTIP_HEIGHT = 32,
            BTN_ALPHA_WIDTH = 38,
            BTN_ALPHA_HEIGHT = 23,
        };

    protected:
        POINT		m_MiniMapFramePos;
        POINT		m_MiniMapPos;
        POINT		m_TimeUIPos;
        POINT		m_SkillFramePos;
        POINT		m_BtnSkillScrollUpPos;
        POINT		m_BtnSkillScrollDnPos;
        POINT		m_SkillIconPos;
        POINT		m_UseSkillDestKillPos;
        POINT		m_CurKillCountPos;
        POINT		m_BtnAlphaPos;
        POINT		m_SkillTooltipPos;

        POINT		m_HeroPosInWorld;
        POINT		m_HeroPosInMiniMap;
        POINT		m_MiniMapScaleOffset;

        float		m_fMiniMapTexU;
        float		m_fMiniMapTexV;

        int			m_iMiniMapScale;
        float		m_fMiniMapAlpha;
        bool		m_bRenderSkillUI;
        bool		m_bRenderToolTip;

        bool		m_bSecond;
        int			m_iHour;
        int			m_iMinute;
        float		m_fTime;

        DWORD		m_dwBuffState;

        GuildCommander	m_CmdBuffer[MAX_COMMANDGROUP];
        CNewUIButton	m_BtnSkillScroll[2];
        CNewUIButton	m_BtnAlpha;

        std::list<int>				m_listBattleSkill;
        std::list<int>::iterator		m_iterCurBattleSkill;

    public:
        CNewUISiegeWarBase();
        virtual ~CNewUISiegeWarBase();

    protected:
        virtual bool OnCreate(int x, int y) = 0;
        virtual bool OnUpdate() = 0;
        virtual bool OnRender() = 0;
        virtual void OnRelease() = 0;
        virtual bool OnUpdateMouseEvent() = 0;
        virtual bool OnUpdateKeyEvent() = 0;
        virtual bool OnBtnProcess() = 0;
        virtual	void OnLoadImages() = 0;
        virtual void OnUnloadImages() = 0;
        virtual void OnSetPos(int x, int y) = 0;

        void RenderCmdIconInMiniMap();

    public:
        bool Create(int x, int y);
        bool Update();
        bool Render();
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        void LoadImages();
        void UnLoadImages();

        void SetPos(int x, int y);
        void SetTime(int iHour, int iMinute);
        void SetMapInfo(GuildCommander& data);
        void SetRenderSkillUI(bool bRenderSkillUI);

        bool InitBattleSkill();
        void ReleaseBattleSkill();

    private:
        bool BtnProcess();
        void UpdateBuffState();
        void UpdateHeroPos();
        void RenderSkillIcon();

        void SetSkillScrollUp();
        void SetSkillScrollDn();
    };
}

#endif // !defined(AFX_NEWUISIEGEWARBASE_H__13F2F04C_290F_41FC_A7A5_2F56F22B3478__INCLUDED_)
