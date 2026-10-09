
#if !defined(_NEWUIPARTYMINIWINDOW_H_)
#define _NEWUIPARTYMINIWINDOW_H_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Party/PartyInfoWindow.h"
#include "UI/Party/PartyListRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"
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

        int							m_iPartyListBGColor[MAX_PARTYS];		// 파티리스트 배경칼라
        bool						m_bPartyMemberoutofSight[MAX_PARTYS];	// 파티원이 내 캐릭터의 시야 밖에 있는가

        bool						m_bActive;
        int							m_iVal;		// 인덱스에 따른 편차(y의 위치)

        int							m_iSelectedCharacter;

        void BindRmlModel(Rml::DataModelConstructor& c, PartyListRmlModel& model);
        UI::RmlBridge::ThemedView<PartyListRmlModel> m_RmlView{"party_list",
            [this](Rml::DataModelConstructor& c, PartyListRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/party_list.rml"}}};
        // A leave button press, queued by RmlUi's click and run from Update(), outside RmlUi's
        // own event dispatch.
        int m_PendingLeave = -1;
        // The card under the pointer (the document's party_hover), -1 for none.
        int m_HoveredCard = -1;

    public:
        CPartyListWindow();
        virtual ~CPartyListWindow();

        bool Create(CManager* pNewUIMng);
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool BtnProcess();

        float GetLayerDepth();	//. 5.4f

        void OpenningProcess();
        void ClosingProcess();

        int GetSelectedCharacter();
        void SetListBGColor();

    private:
        void BuildRmlUi();
        void SyncRmlModel();
        void SyncCards();
        bool CanLeave(int member) const;

        bool SelectCharacterInPartyList(PARTY_t* pMember);
    };
}

#endif // !defined(_NEWUIPARTYMINIWINDOW_H_)
