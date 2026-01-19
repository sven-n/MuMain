// w_PetActionStand.h: interface for the PetActionStand class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "w_PetAction.h"
#include "ZzzBMD.h"

SmartPointer(PetActionStand);

class PetActionStand : public PetAction
{
public:
    static PetActionStandPtr Make();
    virtual ~PetActionStand();
    virtual bool Release(OBJECT* obj, CHARACTER* Owner);

public:
    virtual bool Model(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender);
    virtual bool Move(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender);
    virtual bool Effect(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender);

private:
    PetActionStand();
};
