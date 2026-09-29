#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/Window3DRenderMng.h"
#include "UI/Core/WindowManager.h"
#include "UI/Events/EventItemEntryView.h"

namespace mu::ui::window
{
// Jerint the Assistant's Imperial Guardian entry window, docked right.
// empire_guardian_enter.rml draws its texts and buttons, empire_guardian_enter_bg.rml its
// frame; Gaion's Order stays a native 3D preview over the frame. C++ keeps the corner close,
// Escape and the entry request.
class CEmpireGuardianNPC : public CObject, public I3DRenderObj
{
private:
    enum EMPIREGUARDIAN_TIME_WINDOW_SIZE
    {
        NPC_WINDOW_WIDTH = 190,
        NPC_WINDOW_HEIGHT = 429,
    };

    CManager* m_pNewUIMng;
    C3DRenderMng* m_pNewUI3DRenderMng;

    POINT m_Pos;
    EventItemEntryView m_View{"empire_guardian_enter", "Data/Interface/RmlUi/empire_guardian_enter.rml",
                              "empire_guardian_enter_bg", "Data/Interface/RmlUi/empire_guardian_enter_bg.rml"};
    bool m_bCanClick;

public:
    CEmpireGuardianNPC();
    virtual ~CEmpireGuardianNPC();

    bool Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng, int x, int y);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();
    void Render3D();

    float GetLayerDepth(); //. 1.2f

    bool IsVisible() const;

    void OpenningProcess();
    void ClosingProcess();

private:
    bool BtnProcess();
    void RenderItem3D();
    void SyncView();
};
}
