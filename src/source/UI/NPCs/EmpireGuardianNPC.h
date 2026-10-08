#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Events/EventItemEntryView.h"

namespace mu::ui::window
{
// Jerint the Assistant's Imperial Guardian entry window, docked right. empire_guardian_enter.rml
// draws it; C++ draws Gaion's Order into it and keeps the corner close, Escape and the entry
// request.
class CEmpireGuardianNPC : public CObject
{
private:
    enum EMPIREGUARDIAN_TIME_WINDOW_SIZE
    {
        NPC_WINDOW_WIDTH = 190,
        NPC_WINDOW_HEIGHT = 429,
    };

    CManager* m_pNewUIMng;

    POINT m_Pos;
    EventItemEntryView m_View{"empire_guardian_enter", "Data/Interface/RmlUi/empire_guardian_enter.rml"};
    bool m_bCanClick;

public:
    CEmpireGuardianNPC();
    virtual ~CEmpireGuardianNPC();

    bool Create(CManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    Rml::ElementDocument* GetPlacedDocument() const override { return m_View.Document(); }
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    float GetLayerDepth(); //. 1.2f

    bool IsVisible() const;

    void OpenningProcess();
    void ClosingProcess();

private:
    void RenderItem3D();
    void SyncView();
};
}
