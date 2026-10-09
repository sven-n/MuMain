#pragma once
#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Events/EventItemEntryView.h"

namespace mu::ui::window
{
// Lugard's Doppelganger entry window, docked right. doppelganger_enter.rml draws it; C++ draws the
// Mirror of Dimensions into it and keeps the remaining time, the corner close, Escape and the entry
// request.
class CDoppelGangerWindow : public CObject
{
private:
    enum
    {
        INVENTORY_WIDTH = 190,
        INVENTORY_HEIGHT = 429,
    };

    CManager* m_pNewUIMng;


    EventItemEntryView m_View{"doppelganger_enter", "Data/Interface/RmlUi/doppelganger_enter.rml"};

public:
    CDoppelGangerWindow();
    virtual ~CDoppelGangerWindow();

    bool Create(CManager* pNewUIMng);
    void Release();


    bool UpdateMouseEvent();
    Rml::ElementDocument* GetPlacedDocument() const override { return m_View.Document(); }
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    bool IsVisible() const;

    void OpeningProcess();
    void ClosingProcess();

    float GetLayerDepth(); //. 5.0f

    void SetRemainTime(int iTime);
    void LockEnterButton(BOOL bLock);

private:
    void RenderItem3D(const Rml::Vector2f& offset, const Rml::Vector2f& size);
    void SyncView();

    int m_iRemainTime;
    BOOL m_bIsEnterButtonLocked;
};
}
