#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Events/EventTimerView.h"

namespace mu::ui::window
{
// The Imperial Guardian timer HUD. empire_guardian_timer.rml draws it (EventTimerView); C++
// keeps the round, the zone, the time and the monster count.
class CEmpireGuardianTimer : public CObject
{
public:
    enum EG_DAY_MAP_LIST
    {
        EG_MONDAY,
        EG_TUESDAY,
        EG_WEDNESDAY,
        EG_THURSDAY,
        EG_FRIDAY,
        EG_SATURDAY,
        EG_SUNDAY,
    };

    enum IMAGE_LIST
    {
        IMAGE_EMPIREGUARDIAN_TIMER_WINDOW = BITMAP_EMPIREGUARDIAN_TIMER_BEGIN,
    };

private:
    enum EMPIREGUARDIAN_TIME_WINDOW_SIZE
    {
        TIMER_WINDOW_WIDTH = 124,
        TIMER_WINDOW_HEIGHT = 81,
    };

    CManager* m_pNewUIMng;
    POINT m_Pos;

public:
    CEmpireGuardianTimer();
    virtual ~CEmpireGuardianTimer();

    bool Create(CManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    bool BtnProcess();

    float GetLayerDepth(); //. 1.2f

    void OpenningProcess();
    void ClosingProcess();

    void SetType(int iType)
    {
        m_iType = iType;
    }
    void SetRemainTime(DWORD tick)
    {
        m_dTime = tick;
    }
    void SetDay(int iDay)
    {
        m_iDay = iDay;
    }
    void SetZone(int iZone)
    {
        m_iZone = iZone;
    }
    void SetMonsterCount(int iMonsterCount)
    {
        m_iMonsterCount = iMonsterCount;
    }

    const int GetDay()
    {
        return m_iDay;
    }
    const int GetZone()
    {
        return m_iZone;
    }

    void ReloadRmlTheme();

private:
    void SyncView();

    int m_iType;
    DWORD m_dTime;
    int m_iDay;
    int m_iZone;
    int m_iMonsterCount;
    EventTimerView m_View{"empire_guardian_timer", "Data/Interface/RmlUi/empire_guardian_timer.rml"};
};
}
