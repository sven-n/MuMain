// w_PetActionCollecter_Add.h: interface for the PetActionStand class.
//////////////////////////////////////////////////////////////////////

#ifdef PJH_ADD_PANDA_PET

#include "w_PetAction.h"
#include "ZzzBMD.h"
#include "w_PetActionCollecter.h"

#pragma once

SmartPointer(PetActionCollecterAdd);
class PetActionCollecterAdd : public PetAction
{
public:
    static PetActionCollecterAddPtr Make();
    virtual ~PetActionCollecterAdd();
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

protected:
    PetActionCollecterAdd();

    //test

    //ItemList m_ItemList;
    RootingItem m_RootItem;
    bool m_isRooting;

    DWORD m_dwSendDelayTime;
    DWORD m_dwRootingTime;
    DWORD m_dwRoundCountDelay;
    ActionState m_state;

    double m_fRadWidthStand;
    double m_fRadWidthGet;

    //test
};
#endif //PJH_ADD_PANDA_PET

SmartPointer(PetActionCollecterSkeleton);
class PetActionCollecterSkeleton : public PetActionCollecterAdd
{
public:
    static PetActionCollecterSkeletonPtr Make();
    virtual ~PetActionCollecterSkeleton();
    virtual bool Release(OBJECT* obj, CHARACTER* Owner);

public:
    virtual bool Move(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender);
    virtual bool Effect(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender);

protected:
    PetActionCollecterSkeleton();

    BOOL m_bIsMoving;
};
