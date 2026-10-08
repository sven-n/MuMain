
#pragma once
#include "UI/Core/WindowManager.h"
#include "UI/Events/EventItemEntryView.h"

namespace Rml
{
class Element;
}

namespace mu::ui::window
{
// The Golden Archer's lucky number (scratch ticket) registration window. gold_bowman.rml draws
// it: the frame, the field's back, the texts, the 12-character number field, Register and the
// exit button. C++ keeps the field's focus, the registration
// request, the exit tooltip, Escape and the dialog-exit request on closing.
class CGoldBowmanWindow : public CObject
{
private:
    enum
    {
        INVENTORY_WIDTH = 190,
        INVENTORY_HEIGHT = 429,
    };

    enum BUTTON
    {
        BUTTON_SERIAL = 0,
        BUTTON_EXIT,
    };

public:
    CManager* m_pNewUIMng;
    POINT m_Pos;

public:
    CGoldBowmanWindow();
    virtual ~CGoldBowmanWindow();

    bool Create(CManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);
    const POINT& GetPos();

    bool UpdateMouseEvent();
    Rml::ElementDocument* GetPlacedDocument() const override { return m_View.Document(); }
    bool UpdateKeyEvent();
    bool Update();
    bool TakesTypingFrom(const Rml::ElementDocument* document) const override;
    bool Render();

    float GetLayerDepth(); // 3.4f

public:
    void OpeningProcess();
    void ClosingProcess();

private:
    void SyncView();
    void ClearSerialField();
    void SendSerial();
    Rml::Element* GetSerialField() const;

    EventItemEntryView m_View{"gold_bowman", "Data/Interface/RmlUi/gold_bowman.rml"};
    // The field takes the focus once the document shows it, as the original's GiveFocus() did.
    bool m_SerialFocusPending = false;
};

    inline
        void CGoldBowmanWindow::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        const POINT& CGoldBowmanWindow::GetPos()
    {
        return m_Pos;
    }
};
