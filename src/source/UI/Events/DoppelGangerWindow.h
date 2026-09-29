#pragma once
#include "UI/Core/WindowObject.h"
#include "UI/Core/Window3DRenderMng.h"
#include "UI/Core/WindowManager.h"
#include "UI/Events/EventItemEntryView.h"

namespace mu::ui::window
{
// Lugard's Doppelganger entry window, docked right. doppelganger_enter.rml draws its texts and
// buttons, doppelganger_enter_bg.rml its frame; the Mirror of Dimensions stays a native 3D
// preview over the frame. C++ keeps the remaining time, the corner close, Escape and the entry
// request.
class CDoppelGangerWindow : public CObject, public I3DRenderObj
{
private:
    enum
    {
        INVENTORY_WIDTH = 190,
        INVENTORY_HEIGHT = 429,
    };

    CManager* m_pNewUIMng;
    C3DRenderMng* m_pNewUI3DRenderMng;
    POINT m_Pos;

    EventItemEntryView m_View{"doppelganger_enter", "Data/Interface/RmlUi/doppelganger_enter.rml",
                              "doppelganger_enter_bg", "Data/Interface/RmlUi/doppelganger_enter_bg.rml"};

public:
    CDoppelGangerWindow();
    virtual ~CDoppelGangerWindow();

    bool Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng, int x, int y);
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

    void SetRemainTime(int iTime);
    void LockEnterButton(BOOL bLock);

private:
    bool BtnProcess();
    void RenderItem3D();
    void SyncView();

    int m_iRemainTime;
    BOOL m_bIsEnterButtonLocked;
};
}
