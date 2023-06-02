// Event.h: interface for the CEvent class.
//////////////////////////////////////////////////////////////////////

#pragma once

#define XMAS_EVENT_TIME		60000

class CXmasEvent
{
public:
    CXmasEvent(void);
    ~CXmasEvent(void);

    static CXmasEvent* GetInstance() { static CXmasEvent s_Instance; return &s_Instance; }

    void LoadXmasEvent();
    void LoadXmasEventEffect();
    void LoadXmasEventItem();
    void LoadXmasEventSound();

    void CreateXmasEventEffect(CHARACTER* pCha, OBJECT* pObj, int iType);

    void GenID();

public:
    int m_iEffectID;
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
