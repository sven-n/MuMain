
#if !defined(AFX_NEWUICURSEDTEMPLEENTER_H__1151C4F9_04A5_47B1_A717_E7905BEEAD08__INCLUDED_)
#define AFX_NEWUICURSEDTEMPLEENTER_H__1151C4F9_04A5_47B1_A717_E7905BEEAD08__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Widgets/Window/Button.h"
#include "UI/Events/CursedTempleEnterRmlModel.h"
#include "UI/Events/CursedTempleUpdates.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The Illusion Temple entry window. cursed_temple_enter.rml draws it; C++ keeps the level check,
// the member count, Escape and the entry request.
class CCursedTempleEnter : public CObject
{
public:
    static constexpr float CURSEDTEMPLE_ENTER_WINDOW_WIDTH = 230.0f;
    static constexpr float CURSEDTEMPLE_ENTER_WINDOW_HEIGHT = 252.0f;

    enum
    {
        CURSEDTEMPLEENTER_OPEN = 0,
        CURSEDTEMPLEENTER_EXIT,
        CURSEDTEMPLEENTER_MAXBUTTONCOUNT,
    };

public:
    CCursedTempleEnter();
    virtual ~CCursedTempleEnter();

    bool Create(CManager* pNewUIMng, int x, int y);
    Rml::ElementDocument* GetFillDocument() const override { return m_pRmlDoc; }

public:
    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();

public:
    bool CheckEnterLevel(int& enterlevel);
    bool CheckEnterItem(ITEM* p, int enterlevel);
    bool CheckInventory(BYTE& itempos, int enterlevel);

public:
    bool Render();

    void ReloadRmlTheme();

private:
    void BuildRmlUi();
    void SyncRmlModel();
    void SyncLines();

public:
    void SetPos(int x, int y);

public:
    const POINT& GetPos() const;
    float GetLayerDepth(); //. 5.0f

public:
    void SetEntryOffer(std::uint8_t remainingTime, std::uint8_t entryCount);
    void SetEntryCounts(std::span<const std::uint8_t, 6> counts);

private:
    void Initialize();
    void Destroy();

private:
    CManager* m_pNewUIMng;
    POINT m_Pos;
    RmlModelBinder<CursedTempleEnterRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
    bool m_PendingEnter = false;
    bool m_PendingClose = false;
    int m_EnterTime;
    int m_EnterCount;
};

    inline
        float CCursedTempleEnter::GetLayerDepth()
    {
        return 10.3;
    }

    inline
        void CCursedTempleEnter::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        const POINT& CCursedTempleEnter::GetPos() const
    {
        return m_Pos;
    }
};

#endif // !defined(AFX_NEWUICURSEDTEMPLEENTER_H__1151C4F9_04A5_47B1_A717_E7905BEEAD08__INCLUDED_)
