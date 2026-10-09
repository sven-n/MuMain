
#pragma once
#include "UI/Core/WindowManager.h"
#include "UI/Events/EventItemEntryView.h"

namespace mu::ui::window
{
// The Golden Archer's Rena registration window. gold_bowman_lena.rml draws it: the frame, the
// texts with the collected and registered counts, Register and the exit button, and the two live
// 3D Rena C++ draws into it. C++ keeps the Rena, the counts, the
// registration request, the button tooltips, Escape and the dialog-exit request on closing.
class CGoldBowmanLena : public CObject
{
private:
    enum
    {
        INVENTORY_WIDTH = 190,
        INVENTORY_HEIGHT = 429,
    };

    enum BUTTON
    {
        BUTTON_REGISTER = 0,
        BUTTON_EXIT,
    };

public:
    CManager* m_pNewUIMng;
    POINT m_Pos;

public:
    CGoldBowmanLena();
    virtual ~CGoldBowmanLena();

    bool Create(CManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);
    const POINT& GetPos();

    bool UpdateMouseEvent();
    Rml::ElementDocument* GetPlacedDocument() const override { return m_View.Document(); }
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    float GetLayerDepth(); // 3.4f

public:
    void OpeningProcess();
    void ClosingProcess();

private:
    void SyncView();
    void Render3D(const Rml::Vector2f& offset, const Rml::Vector2f& size);

    EventItemEntryView m_View{"gold_bowman_lena", "Data/Interface/RmlUi/gold_bowman_lena.rml"};
};

    inline
        void CGoldBowmanLena::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        const POINT& CGoldBowmanLena::GetPos()
    {
        return m_Pos;
    }
};
