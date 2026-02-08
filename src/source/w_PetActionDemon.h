// w_PetActionDemon.h: interface for the PetActionDemon class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "w_PetAction.h"
#include "ZzzBMD.h"

SmartPointer(PetActionDemon);
class PetActionDemon : public PetAction
{
public:
    static PetActionDemonPtr Make();
    virtual ~PetActionDemon();
    virtual bool Release(OBJECT* obj, CHARACTER* Owner);

public:
    virtual bool Model(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender);
    virtual bool Move(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender);
    virtual bool Effect(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender);

private:
    PetActionDemon();
};
