
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Combat/DuelWatchSpectatorRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"

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
    POINT m_Pos;

public:
    CDuelWatchUserListWindow();
    virtual ~CDuelWatchUserListWindow();

    bool Create(CManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    bool BtnProcess();

    float GetLayerDepth(); //. 5.4f

    void OpeningProcess();
    void ClosingProcess();

    void ReloadRmlTheme();

private:
    void BuildRmlUi();
    void SyncView();

    RmlModelBinder<DuelWatchSpectatorsRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
};
}
