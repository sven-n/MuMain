// w_PetActionCollecter.h: interface for the PetActionStand class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "w_PetAction.h"
#include "ZzzBMD.h"

#define SEARCH_LENGTH 300.0f
#define CIRCLE_STAND_RADIAN 50.0f
#define CIRCLE_GETITEM_RADIAN 50.0f

typedef struct _RootingItem
{
    int itemIndex;
    vec3_t position;
}RootingItem;

SmartPointer(PetActionCollecter);
class PetActionCollecter : public PetAction
{
public:
    static PetActionCollecterPtr Make();
    virtual ~PetActionCollecter();
    virtual bool Release(OBJECT* obj, CHARACTER* Owner);

public:
    virtual bool Model(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender);
    virtual bool Move(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender);
    virtual bool Effect(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender);
    virtual bool Sound(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender);

    //test
    void FindZen(OBJECT* obj);
    bool CompTimeControl(const DWORD& dwCompTime, DWORD& dwTime);

public:
    typedef std::list< RootingItem > ItemList;
    enum ActionState
    {
        eAction_Stand = 0,
        eAction_Move = 1,
        eAction_Get = 2,
        eAction_Return = 3,

        eAction_End_NotUse,
    };

private:
    PetActionCollecter();
    RootingItem m_RootItem;
    bool m_isRooting;
    DWORD m_dwSendDelayTime;
    DWORD m_dwRootingTime;
    DWORD m_dwRoundCountDelay;
    ActionState m_state;
    double m_fRadWidthStand;
    double m_fRadWidthGet;
};
