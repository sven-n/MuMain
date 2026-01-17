// NewUICursedTempleResult.h: interface for the CNewUICursedTempleResult class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICURSEDTEMPLERESULT_H__573A17F1_A967_4C70_AF42_6214CCD165EE__INCLUDED_)
#define AFX_NEWUICURSEDTEMPLERESULT_H__573A17F1_A967_4C70_AF42_6214CCD165EE__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMessageBox.h"
#include "NewUIButton.h"

namespace SEASON3B
{
    class CNewUICursedTempleResult : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_CURSEDTEMPLERESULT_SUCCESS = BITMAP_CURSEDTEMPLE_BEGIN,
            IMAGE_CURSEDTEMPLERESULT_FAILURE,
        };

        enum UI_SIZE
        {
            CURSEDTEMPLE_RESULT_WINDOW_WIDTH = 230,
            CURSEDTEMPLE_RESULT_WINDOW_HEIGHT = 282,
        };

        enum
        {
            CURSEDTEMPLERESULT_CLOSE = 0,
            CURSEDTEMPLERESULT_MAXBUTTONCOUNT,
        };

        struct CursedTempleGameResult
        {
            wchar_t		s_characterId[MAX_ID_SIZE + 1];
            short		s_mapnumber;
            SEASON3A::eCursedTempleTeam		s_team;
            BYTE		s_point;
            CLASS_TYPE        s_class;
            DWORD       s_addexp;

            CursedTempleGameResult() : s_mapnumber(-1), s_team(SEASON3A::eTeam_Count), s_point(0xff), s_class(CLASS_UNDEFINED), s_addexp(0xff)
            {
                memset(&s_characterId, 0, sizeof(char) * (MAX_ID_SIZE + 1));
            }
        };

    public:
        CNewUICursedTempleResult();
        virtual ~CNewUICursedTempleResult();

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
        void OpenningProcess();
        void ClosingProcess();

    private:
        void UpdateResult();

    public:
        bool Render();

    private:
        void RenderResultPanel();
        void RenderFrame();
        void RenderTextLine(const CursedTempleGameResult& resultinfo, int x, int y, DWORD color, DWORD backcolor);
        void RenderText();
        void RenderButtons();

    public:
        const POINT& GetPos() const;
        float GetLayerDepth();	//. 5.0f

    public:
        void SetPos(int x, int y);
        void SetMyTeam(SEASON3A::eCursedTempleTeam myteam);

    public:
        void ReceiveCursedTempleGameResult(const BYTE* ReceiveBuffer);
        void ResetGameResultInfo();

    private:
        void Initialize();
        void Destroy();

    private:
        typedef std::list<CursedTempleGameResult>  CT_GameResult_list;

    private:
        CT_GameResult_list		m_AlliedTeamGameResult;
        CT_GameResult_list		m_IllusionTeamGameResult;

    private:
        CNewUIManager* m_pNewUIMng;
        CNewUIButton			m_Button[CURSEDTEMPLERESULT_MAXBUTTONCOUNT];
        POINT					m_Pos;

        std::wstring				m_infoText;
        float					m_ResultEffectAlph;
        int						m_WinState;
        SHORT					m_CharacterKey;
        SEASON3A::eCursedTempleTeam		m_MyTeam;
    };

    inline
        void CNewUICursedTempleResult::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        void CNewUICursedTempleResult::SetMyTeam(SEASON3A::eCursedTempleTeam myteam)
    {
        m_MyTeam = myteam;
    }

    inline
        const POINT& CNewUICursedTempleResult::GetPos() const
    {
        return m_Pos;
    }

    inline
        float CNewUICursedTempleResult::GetLayerDepth()	//. 5.0f
    {
        return 10.2f;
    }
};

#endif // !defined(AFX_NEWUICURSEDTEMPLERESULT_H__573A17F1_A967_4C70_AF42_6214CCD165EE__INCLUDED_)
