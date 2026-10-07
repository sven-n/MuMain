#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Core/WindowManager.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Combat/DuelWatchSpectatorRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The spectator's duel frame, in the main frame's place while the duel-watch buff is on.
// duel_watch_frame.rml draws it; C++ keeps the gauge animation, the exit tooltip and the
// channel quit request.
class CDuelWatchMainFrameWindow : public CObject
{
public:
    enum IMAGE_LIST
    {
        IMAGE_DUELWATCH_MAINFRAME_BACK1 = BITMAP_BUFFWATCH_MAINFRAME_BEGIN,
        IMAGE_DUELWATCH_MAINFRAME_BACK2,
        IMAGE_DUELWATCH_MAINFRAME_BACK3,
        IMAGE_DUELWATCH_MAINFRAME_SCORE,
        IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE,
        IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE,
        IMAGE_DUELWATCH_MAINFRAME_HP_GAUGE_FX,
        IMAGE_DUELWATCH_MAINFRAME_SD_GAUGE_FX,
        IMAGE_INVENTORY_EXIT_BTN = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
    };

private:
    CManager* m_pNewUIMng;

    bool m_PendingExit = false;

    BOOL m_bHasHPReceived; // HP 초기상태인가
    float m_fPrevHPRate1;
    float m_fPrevHPRate2;
    float m_fPrevSDRate1;
    float m_fPrevSDRate2;
    float m_fLastHPRate1;
    float m_fLastHPRate2;
    float m_fLastSDRate1;
    float m_fLastSDRate2;
    float m_fReceivedHPRate1;
    float m_fReceivedHPRate2;
    float m_fReceivedSDRate1;
    float m_fReceivedSDRate2;

    void BindRmlModel(Rml::DataModelConstructor& c, DuelWatchFrameRmlModel& model);
    UI::RmlBridge::ThemedView<DuelWatchFrameRmlModel> m_RmlView{"duel_watch_frame",
        [this](Rml::DataModelConstructor& c, DuelWatchFrameRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/duel_watch_frame.rml"}}};

public:
    CDuelWatchMainFrameWindow();
    virtual ~CDuelWatchMainFrameWindow();

    bool Create(CManager* pNewUIMng);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    bool IsVisible() const;

    void OpeningProcess();
    void ClosingProcess();

    float GetLayerDepth(); //. 5.0f


private:
    void BuildRmlUi();
    void SyncView();
    std::vector<DuelWatchGaugeEntry> StepGauges();
};
}
