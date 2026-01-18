// NewUISiegeWarCommander.h: interface for the CNewUISiegeWarCommander class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUISIEGEWARCOMMANDER_H__7D918ECD_BB86_421F_B62C_CA9E294D1E7D__INCLUDED_)
#define AFX_NEWUISIEGEWARCOMMANDER_H__7D918ECD_BB86_421F_B62C_CA9E294D1E7D__INCLUDED_

#pragma once

#include "NewUISiegeWarBase.h"

namespace SEASON3B
{
    class CNewUISiegeWarCommander : public CNewUISiegeWarBase
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_MINIMAP_BTN_GROUP = CNewUISiegeWarBase::IMAGE_SIEGEWAR_BASE_FRAME_END,	// newui_SW_Minimap_Bt_group.tga	(26, 22)
            IMAGE_MINIMAP_BTN_COMMAND,														// newui_SW_Minimap_Bt_Command.tga	(30, 22)
        };

        enum FRAME_SIZE
        {
            MINIMAP_BTN_GROUP_WIDTH = 26,
            MINIMAP_BTN_GROUP_HEIGHT = 22,
            MINIMAP_BTN_COMMAND_WIDTH = 30,
            MINIMAP_BTN_COMMAND_HEIGHT = 22,
        };

        enum MIMIMAP_COMMAND
        {
            MINIMAP_CMD_ATTACK = 0,
            MINIMAP_CMD_DEFENCE,
            MINIMAP_CMD_FLAG,
            MINIMAP_CMD_MAX,
        };

    private:
        POINT			m_BtnCommandGroupPos;
        POINT			m_BtnCommandPos;

        CNewUIButton	m_BtnCommandGroup[MAX_COMMANDGROUP];
        CNewUIButton	m_BtnCommand[MINIMAP_CMD_MAX];

        int				m_iCurSelectBtnGroup;
        int				m_iCurSelectBtnCommand;
        bool			m_bMouseInMiniMap;

        std::vector<VisibleUnitLocation>  m_vGuildMemberLocationBuffer;

    public:
        CNewUISiegeWarCommander();
        virtual ~CNewUISiegeWarCommander();

    private:
        virtual bool OnCreate(int x, int y);
        virtual bool OnUpdate();
        virtual bool OnRender();
        virtual void OnRelease();

        virtual bool OnUpdateMouseEvent();
        virtual bool OnUpdateKeyEvent();
        virtual bool OnBtnProcess();
        virtual void OnSetPos(int x, int y);

        virtual void OnLoadImages();
        virtual void OnUnloadImages();

        void InitDestKill();
        void InitCmdGroupBtn();
        void InitCmdBtn();
        void RenderCharPosInMiniMap();
        void RenderGuildMemberPosInMiniMap();
        void RenderCmdIconAtMouse();
        void RenderCmdGroupBtn();
        void RenderCmdBtn();

        void SetBtnState(int iBtnType, bool bStateDown);

    public:
        void ClearGuildMemberLocation(void);
        void SetGuildMemberLocation(BYTE type, int x, int y);
    };
}

#endif // !defined(AFX_NEWUISIEGEWARCOMMANDER_H__7D918ECD_BB86_421F_B62C_CA9E294D1E7D__INCLUDED_)
