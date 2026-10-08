#if !defined(AFX_NEWUIBATTLESOCCERSCORE_H__68E768E4_5FB7_4D33_A604_54315C1D26C6__INCLUDED_)
#define AFX_NEWUIBATTLESOCCERSCORE_H__68E768E4_5FB7_4D33_A604_54315C1D26C6__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Events/BattleSoccerScoreRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The battle-soccer / guild-war scoreboard. battle_soccer_score.rml draws it; C++ keeps its
// position and feeds the teams' scores, marks and names.
class CBattleSoccerScore : public CObject
{
private:
    enum
    {
        BSS_WIDTH = 131,
        BSS_HEIGHT = 70,
    };

    CManager* m_pNewUIMng; // UI 매니저.
    POINT m_Pos;                // 창의 위치.

public:
    CBattleSoccerScore();
    virtual ~CBattleSoccerScore();

    bool Create(CManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    float GetLayerDepth(); //. 1.8f


private:
    void BuildRmlUi();
    void SyncRmlModel();
    void SyncTeams();

    int FindGuildMark(wchar_t* pszGuildName);

    void BindRmlModel(Rml::DataModelConstructor& c, BattleSoccerScoreRmlModel& model);
    UI::RmlBridge::ThemedView<BattleSoccerScoreRmlModel> m_RmlView{"battle_soccer_score",
        [this](Rml::DataModelConstructor& c, BattleSoccerScoreRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/battle_soccer_score.rml"}}};
};
} // namespace mu::ui::window

#endif // !defined(AFX_NEWUIBATTLESOCCERSCORE_H__68E768E4_5FB7_4D33_A604_54315C1D26C6__INCLUDED_)
