// NewUICursedTempleSystem.h: interface for the CNewUICursedTempleSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICURSEDTEMPLESYSTEM_H__3018484F_9F75_48EB_8D76_31103617DCB7__INCLUDED_)
#define AFX_NEWUICURSEDTEMPLESYSTEM_H__3018484F_9F75_48EB_8D76_31103617DCB7__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIMessageBox.h"
#include "NewUIButton.h"
#include "WSclient.h"
#include "UIControls.h"
#include "NewUIMainFrameWindow.h"

namespace SEASON3B
{
    class CNewUICursedTempleSystem : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_CURSEDTEMPLESYSTEM_TOP = CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP,
            IMAGE_CURSEDTEMPLESYSTEM_MIDDLE = CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE,
            IMAGE_CURSEDTEMPLESYSTEM_BOTTOM = CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM,
            IMAGE_CURSEDTEMPLESYSTEM_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_CURSEDTEMPLESYSTEM_BTN = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,

            IMAGE_CURSEDTEMPLESYSTEM_MINIMAPFRAME = BITMAP_CURSEDTEMPLE_BEGIN + 2,
            IMAGE_CURSEDTEMPLESYSTEM_MINIMAP,
            IMAGE_CURSEDTEMPLESYSTEM_MINIMAPALPBTN,
            IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_HOLYITEM_PC,
            IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_HOLYITEM,
            IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_PC,
            IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_NPC,
            IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_HOLYITEM,
            IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_PC,
            IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_NPC,
            IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_HERO,
            IMAGE_CURSEDTEMPLESYSTEM_SKILLFRAME,
            IMAGE_CURSEDTEMPLESYSTEM_SKILLUPBT,
            IMAGE_CURSEDTEMPLESYSTEM_SKILLDOWNBT,
            IMAGE_CURSEDTEMPLESYSTEM_GAMETIME,
            IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_NUMBER,
            IMAGE_CURSEDTEMPLESYSTEM_SCORE_ILLUSION_NUMBER = IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_NUMBER + 10,
            IMAGE_CURSEDTEMPLESYSTEM_SCORE_VS0 = IMAGE_CURSEDTEMPLESYSTEM_SCORE_ILLUSION_NUMBER + 10,
            IMAGE_CURSEDTEMPLESYSTEM_SCORE_VS1,
            IMAGE_CURSEDTEMPLESYSTEM_SCORE_ALLIED_GAAIL,
            IMAGE_CURSEDTEMPLESYSTEM_SCORE_ILLUSION_GAAIL,
            IMAGE_CURSEDTEMPLESYSTEM_SCORE_LEFT,
            IMAGE_CURSEDTEMPLESYSTEM_SCORE_RIGHT,
            IMAGE_SKILL2 = CNewUISkillList::IMAGE_SKILL2,
            IMAGE_NON_SKILL2 = CNewUISkillList::IMAGE_NON_SKILL2,
        };

        enum
        {
            CURSEDTEMPLERESULT_ALPH = 0,
            CURSEDTEMPLERESULT_SKILLUP,
            CURSEDTEMPLERESULT_SKILLDOWN,
            CURSEDTEMPLERESULT_MAXBUTTONCOUNT,
        };

    public:
        CNewUICursedTempleSystem();
        virtual ~CNewUICursedTempleSystem();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);

    private:
        void LoadImages();
        void UnloadImages();
        void SetButtonInfo();

    public:
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();

    public:
        bool CheckInventoryHolyItem(CHARACTER* c);
        bool CheckTalkProgressNpc(DWORD npcindex, DWORD npckey);
        bool CheckHeroSkillType(int operatortype = 0);
        bool CheckDragonRender();
        bool IsCursedTempleSkillKey(DWORD selectcharacterindex);

    private:
        void UpdateScore();
        void UpdateTutorialStep();

    public:
        bool Render();

    private:
        void RenderSkill();
        void RenderGameTime();
        void RenderMiniMap();
        void RenderScore();
        void RenderTutorialStep();

    public:
        const POINT& GetPos() const;
        float GetLayerDepth();	//. 1.5f

    public:
        void SetPos(int x, int y);
        void ResetCursedTempleSystemInfo();
        void StartScoreEffect();
        void StartTutorialStep();
        void EndScoreEffect();
        void EndTutorialStep();

        SEASON3A::eCursedTempleTeam GetMyTeam();

    public:
        void SetCursedTempleSkill(CHARACTER* c, OBJECT* o, DWORD selectcharacterindex);
        void ReceiveCursedTempRegisterSkill(const BYTE* ReceiveBuffer);
        void ReceiveCursedTempUnRegisterSkill(const BYTE* ReceiveBuffer);
        void ReceiveCursedTempleInfo(const BYTE* ReceiveBuffer);
        void ReceiveCursedTempSkillPoint(const BYTE* ReceiveBuffer);
        void ReceiveCursedTempleHolyItemRelics(const BYTE* ReceiveBuffer);

    private:
        void Initialize();
        void Destroy();

    private:
        CNewUIManager* m_pNewUIMng;
        POINT					m_Pos;
        CNewUIButton			m_Button[CURSEDTEMPLERESULT_MAXBUTTONCOUNT];
        //EventTime
        DWORD				m_EventMapTime;             // 이벤트 전체 시간
        //MiniMap
        WORD				m_HolyItemPlayerIndex;		// 성물을 가지고 있는 사용자 인덱스
        WORD				m_HolyItemPlayerPosX;		// 성물 위치 X
        WORD				m_HolyItemPlayerPosY;		// 성물 위치 Y
        wchar_t				m_HolyItemPlayerName[MAX_ID_SIZE];

        float				m_Scale;
        float				m_Alph;

        //HolyItemCount
        WORD				m_AlliedPoint;				// 연합군측 점수
        WORD				m_IllusionPoint;			// 환영교단측 점수

        WORD							m_CursedTempleMyTeamCount;
        PMSG_CURSED_TAMPLE_PARTY_POS	m_CursedTempleMyTeam[MAX_PARTYS];
        //Team
        SEASON3A::eCursedTempleTeam   m_MyTeam;
        //skillpoint
        WORD				m_SkillPoint;
        //Score
        bool				m_IsScoreEffect;
        DWORD				m_StartScoreEffectTime;
        WORD				m_ScoreEffectState;
        float				m_ScoreEffectAlph;
        //Tutorial Step
        bool				m_IsTutorialStep;
        WORD				m_TutorialStepState;
        DWORD				m_TutorialStepTime;
    };

    inline
        const POINT& CNewUICursedTempleSystem::GetPos() const
    {
        return m_Pos;
    }

    inline
        float CNewUICursedTempleSystem::GetLayerDepth()	//. 1.5f
    {
        return 1.5f;
    }

    inline
        void CNewUICursedTempleSystem::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }
};

#endif // !defined(AFX_NEWUICURSEDTEMPLESYSTEM_H__3018484F_9F75_48EB_8D76_31103617DCB7__INCLUDED_)
