// Event.h: interface for the CEvent class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include <chrono>
#include <cstdint>

struct CHARACTER;
struct OBJECT;
struct BMD;

constexpr std::chrono::milliseconds XMAS_EVENT_TIME{60000};

inline std::uint32_t GetMillisecondsTimestamp()
{
    return static_cast<std::uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());
}

class CXmasEvent
{
public:
    CXmasEvent();
    ~CXmasEvent();

    static CXmasEvent* GetInstance() { static CXmasEvent s_Instance; return &s_Instance; }

    void LoadXmasEvent();
    void LoadXmasEventEffect();
    void LoadXmasEventItem();
    void LoadXmasEventSound();

    void CreateXmasEventEffect(CHARACTER* pCha, OBJECT* pObj, int iType);

    void GenID();

public:
    std::int32_t m_iEffectID;
};

#define g_XmasEvent	CXmasEvent::GetInstance()

class CNewYearsDayEvent
{
public:
    CNewYearsDayEvent();
    ~CNewYearsDayEvent();

    static CNewYearsDayEvent* GetInstance() { static CNewYearsDayEvent s_Instance; return &s_Instance; }

    void LoadModel();
    void LoadSound();

    CHARACTER* CreateMonster(int iType, int iPosX, int iPosY, int iKey);
    bool MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
};

#define g_NewYearsDayEvent	CNewYearsDayEvent::GetInstance()

class C09SummerEvent
{
public:
    C09SummerEvent();
    ~C09SummerEvent();

    static C09SummerEvent* GetInstance() { static C09SummerEvent s_Instance; return &s_Instance; }

    void LoadModel();
    void LoadSound();

    CHARACTER* CreateMonster(int iType, int iPosX, int iPosY, int iKey);
    bool MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
};

#define g_09SummerEvent		C09SummerEvent::GetInstance()

#ifdef CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
class CBlueLuckyBagEvent
{
public:
    CBlueLuckyBagEvent();
    ~CBlueLuckyBagEvent();

    static CBlueLuckyBagEvent* GetInstance() { static CBlueLuckyBagEvent s_Instance; return &s_Instance; }

    void StartBlueLuckyBag();
    void CheckTime();
    bool IsEnableBlueLuckyBag() const;

private:
    using Clock = std::chrono::steady_clock;

    Clock::time_point m_lastActivationTick;
    bool m_isActive;
};

#define g_BlueLuckyBagEvent CBlueLuckyBagEvent::GetInstance()
#endif // CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
