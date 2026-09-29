#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Widgets/Window/Button.h"
#include "UI/Widgets/Window/Tooltip.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Core/Window3DRenderMng.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Combat/DuelWatchSpectatorRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The spectator's duel frame, in the main frame's place while the duel-watch buff is on.
// duel_watch_frame.rml draws it; C++ keeps the gauge animation, the exit tooltip and the
// channel quit request.
class CDuelWatchMainFrameWindow : public CObject, public I3DRenderObj
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
    C3DRenderMng* m_pNewUI3DRenderMng;

    CTooltip m_ExitTooltip;
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

    RmlModelBinder<DuelWatchFrameRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;

public:
    CDuelWatchMainFrameWindow();
    virtual ~CDuelWatchMainFrameWindow();

    bool Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();
    void Render3D();

    bool IsVisible() const;

    void OpeningProcess();
    void ClosingProcess();

    float GetLayerDepth(); //. 5.0f

    void ReloadRmlTheme();

private:
    void BuildRmlUi();
    void SyncView();
    std::vector<DuelWatchGaugeEntry> StepGauges();
};
}
