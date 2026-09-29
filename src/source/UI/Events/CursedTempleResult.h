
#if !defined(AFX_NEWUICURSEDTEMPLERESULT_H__573A17F1_A967_4C70_AF42_6214CCD165EE__INCLUDED_)
#define AFX_NEWUICURSEDTEMPLERESULT_H__573A17F1_A967_4C70_AF42_6214CCD165EE__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Events/CursedTempleResultRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The Illusion Temple result window. cursed_temple_result.rml draws it (frame, victory /
// defeat banner, the two teams' rows, the notice and Close); C++ keeps the result packet,
// Escape and the reward request on closing.
class CCursedTempleResult : public CObject
{
public:
    static constexpr float CURSEDTEMPLE_RESULT_WINDOW_WIDTH = 230.0f;
    static constexpr float CURSEDTEMPLE_RESULT_WINDOW_HEIGHT = 282.0f;

    struct CursedTempleGameResult
    {
        wchar_t s_characterId[MAX_USERNAME_SIZE + 1];
        short s_mapnumber;
        SEASON3A::eCursedTempleTeam s_team;
        BYTE s_point;
        CLASS_TYPE s_class;
        DWORD s_addexp;

        CursedTempleGameResult()
            : s_mapnumber(-1), s_team(SEASON3A::eTeam_Count), s_point(0xff), s_class(CLASS_UNDEFINED), s_addexp(0xff)
        {
            memset(&s_characterId, 0, sizeof(char) * (MAX_USERNAME_SIZE + 1));
        }
    };

public:
    CCursedTempleResult();
    virtual ~CCursedTempleResult();

    bool Create(CManager* pNewUIMng, int x, int y);

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

    void ReloadRmlTheme();

private:
    void BuildRmlUi();
    void SyncRmlModel();
    void SyncTexts();

public:
    const POINT& GetPos() const;
    float GetLayerDepth(); //. 5.0f

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
    typedef std::list<CursedTempleGameResult> CT_GameResult_list;

private:
    CT_GameResult_list m_AlliedTeamGameResult;
    CT_GameResult_list m_IllusionTeamGameResult;

private:
    CManager* m_pNewUIMng;
    POINT m_Pos;
    RmlModelBinder<CursedTempleResultRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
    bool m_PendingClose = false;

    std::wstring m_infoText;
    float m_ResultEffectAlph;
    int m_WinState;
    SHORT m_CharacterKey;
    SEASON3A::eCursedTempleTeam m_MyTeam;
};

    inline
        void CCursedTempleResult::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        void CCursedTempleResult::SetMyTeam(SEASON3A::eCursedTempleTeam myteam)
    {
        m_MyTeam = myteam;
    }

    inline
        const POINT& CCursedTempleResult::GetPos() const
    {
        return m_Pos;
    }

    inline
        float CCursedTempleResult::GetLayerDepth()	//. 5.0f
    {
        return 10.2f;
    }
};

#endif // !defined(AFX_NEWUICURSEDTEMPLERESULT_H__573A17F1_A967_4C70_AF42_6214CCD165EE__INCLUDED_)
