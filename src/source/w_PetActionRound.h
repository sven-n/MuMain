// w_PetActionRound.h: interface for the PetActionRound class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "w_PetAction.h"
#include "ZzzBMD.h"

SmartPointer(PetActionRound);

class PetActionRound : public PetAction
{
public:
    static PetActionRoundPtr Make();
    virtual ~PetActionRound();
    virtual bool Release(OBJECT* obj, CHARACTER* Owner);

public:
    virtual bool Model(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender);
    virtual bool Move(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender);
    virtual bool Effect(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender);

private:
    PetActionRound();
};
