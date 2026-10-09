
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Quests/MyQuestInfoWindow.h"
#include "UI/Combat/DuelWatchRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// Doorkeeper Titus's colosseum list. duel_watch.rml draws it; C++ keeps the rooms (g_DuelMgr),
// the corner close, Escape and the join request.
class CDuelWatchWindow : public CObject
{
public:
    enum IMAGE_LIST
    {
        IMAGE_DUELWATCHWINDOW_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK, // Reference
        IMAGE_DUELWATCHWINDOW_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
        IMAGE_DUELWATCHWINDOW_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
        IMAGE_DUELWATCHWINDOW_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
        IMAGE_DUELWATCHWINDOW_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
        IMAGE_DUELWATCHWINDOW_BUTTON = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL,
        IMAGE_DUELWATCHWINDOW_LINE = CMyQuestInfoWindow::IMAGE_MYQUEST_LINE,
    };

private:
    enum
    {
        INVENTORY_WIDTH = 190,
        INVENTORY_HEIGHT = 429,
    };

    CManager* m_pNewUIMng;
    POINT m_Pos;

    BOOL m_bChannelEnable[4];

    void BindRmlModel(Rml::DataModelConstructor& c, DuelWatchRmlModel& model);
    UI::RmlBridge::ThemedView<DuelWatchRmlModel> m_RmlView{"duel_watch",
        [this](Rml::DataModelConstructor& c, DuelWatchRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/duel_watch.rml"}}};
    int m_PendingJoin = -1;

public:
    CDuelWatchWindow();
    virtual ~CDuelWatchWindow();

    bool Create(CManager* pNewUIMng, int x, int y);
    Rml::ElementDocument* GetFillDocument() const override { return m_RmlView.Document(); }
    Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    void OpeningProcess();
    void ClosingProcess();

    float GetLayerDepth(); //. 5.0f


private:
    void BuildRmlUi();
    void SyncRmlModel();
};
}
