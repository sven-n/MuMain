
#if !defined(AFX_NEWUISIEGEWARCOMMANDER_H__7D918ECD_BB86_421F_B62C_CA9E294D1E7D__INCLUDED_)
#define AFX_NEWUISIEGEWARCOMMANDER_H__7D918ECD_BB86_421F_B62C_CA9E294D1E7D__INCLUDED_

#pragma once

#include "UI/Combat/SiegeWarBase.h"

namespace mu::ui::window
{
    class CSiegeWarCommander : public CSiegeWarBase
    {
    public:
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
        // The team buttons\' column, from the frame\'s top-left.
        static constexpr POINT kBtnCommandGroupPos{0, 5};
        POINT			m_BtnCommandPos;


        int				m_iCurSelectBtnGroup;
        int				m_iCurSelectBtnCommand;
        bool			m_bMouseInMiniMap;

        std::vector<VisibleUnitLocation>  m_vGuildMemberLocationBuffer;

    public:
        CSiegeWarCommander();
        virtual ~CSiegeWarCommander();

    private:
        virtual bool OnCreate();
        virtual bool OnUpdate();
        virtual void OnRelease();

        virtual bool OnUpdateMouseEvent();
        virtual bool OnUpdateKeyEvent();
        virtual bool OnBtnProcess();
        void OnTeamClick(int team) override;
        void OnOrderClick(int order) override;
        void OnFillRmlModel(SiegeWarfareRmlModel& model) override;


        void FillCharacterDots(SiegeWarfareRmlModel& model);
        void FillGuildMemberDots(SiegeWarfareRmlModel& model);
        void FillTeamButtons(SiegeWarfareRmlModel& model);
        void FillCommandButtons(SiegeWarfareRmlModel& model);

    public:
        void ClearGuildMemberLocation(void);
        void SetGuildMemberLocation(BYTE type, int x, int y);
    };
}

#endif // !defined(AFX_NEWUISIEGEWARCOMMANDER_H__7D918ECD_BB86_421F_B62C_CA9E294D1E7D__INCLUDED_)
