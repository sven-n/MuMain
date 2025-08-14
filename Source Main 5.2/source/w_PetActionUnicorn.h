// w_PetActionUnicorn.h: interface for the w_PetActionUnicorn class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "w_PetAction.h"
#include "ZzzBMD.h"
#include "w_PetActionCollecter.h"

SmartPointer(PetActionUnicorn);
class PetActionUnicorn : public PetAction
{
public:
    static PetActionUnicornPtr Make();
    virtual ~PetActionUnicorn();
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
    PetActionUnicorn();

    //ItemList m_ItemList;
    RootingItem m_RootItem;
    bool m_isRooting;

    DWORD m_dwSendDelayTime;
    DWORD m_dwRootingTime;
    DWORD m_dwRoundCountDelay;
    ActionState m_state;

    float m_fRadWidthStand;
    float m_fRadWidthGet;

    float m_speed;

    //test
};
