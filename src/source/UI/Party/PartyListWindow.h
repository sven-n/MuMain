
#if !defined(_NEWUIPARTYMINIWINDOW_H_)
#define _NEWUIPARTYMINIWINDOW_H_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Party/PartyInfoWindow.h"
#include "UI/Party/PartyListRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include "UI/Scaling/UITransform.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
    class CPartyListWindow : public CObject
    {
    public:
    private:
        enum PARTY_WINDOW_SIZE
        {
            PARTY_LIST_WINDOW_WIDTH = 77,
            PARTY_LIST_WINDOW_HEIGHT = 23,
        };

        enum PARTY_BG_COLOR
        {
            PARTY_LIST_BGCOLOR_DEFAULT = 0,
            PARTY_LIST_BGCOLOR_RED,
            PARTY_LIST_BGCOLOR_GREEN
        };

    private:
        CManager* m_pNewUIMng;
        POINT						m_Pos;

        int							m_iPartyListBGColor[MAX_PARTYS];		// 파티리스트 배경칼라
        bool						m_bPartyMemberoutofSight[MAX_PARTYS];	// 파티원이 내 캐릭터의 시야 밖에 있는가

        bool						m_bActive;
        int							m_iVal;		// 인덱스에 따른 편차(y의 위치)

        int							m_iSelectedCharacter;

        RmlModelBinder<PartyListRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;
        // A leave button press, queued by RmlUi's click and run from Update(), outside RmlUi's
        // own event dispatch.
        int m_PendingLeave = -1;

    public:
        CPartyListWindow();
        virtual ~CPartyListWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);
        void SetPos(int x);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool BtnProcess();
        void ReloadRmlTheme();

        float GetLayerDepth();	//. 5.4f

        void OpenningProcess();
        void ClosingProcess();

        int GetSelectedCharacter();
        void SetListBGColor();

    private:
        void BuildRmlUi();
        void SyncRmlModel();
        void SyncCards(const UI::Scaling::Transform& transform);
        bool CanLeave(int member) const;

        bool SelectCharacterInPartyList(PARTY_t* pMember);
        void RenderPartyHPOnHead();
    };
}

#endif // !defined(_NEWUIPARTYMINIWINDOW_H_)
