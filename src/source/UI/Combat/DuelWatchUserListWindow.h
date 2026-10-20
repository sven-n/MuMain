
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Combat/DuelWatchSpectatorRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The spectators of the watched duel, in boxes above the spectator frame's right end.
// duel_watch_spectators.rml draws it; C++ keeps the pointer over the boxes from the world.
class CDuelWatchUserListWindow : public CObject
{
public:
    enum IMAGE_LIST
    {
        IMAGE_DUELWATCH_USERLIST_BOX = BITMAP_BUFFWATCH_USERLIST_BEGIN,
    };

private:
    CManager* m_pNewUIMng;


public:
    CDuelWatchUserListWindow();
    virtual ~CDuelWatchUserListWindow();

    bool Create(CManager* pNewUIMng);
    void Release();


    bool UpdateMouseEvent();
    Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    bool BtnProcess();

    float GetLayerDepth(); //. 5.4f

    void OpeningProcess();
    void ClosingProcess();


private:
    void BuildRmlUi();
    void SyncView();

    void BindRmlModel(Rml::DataModelConstructor& c, DuelWatchSpectatorsRmlModel& model);
    UI::RmlBridge::ThemedView<DuelWatchSpectatorsRmlModel> m_RmlView{"duel_watch_spectators",
        [this](Rml::DataModelConstructor& c, DuelWatchSpectatorsRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/duel_watch_spectators.rml"}}};
};
}
