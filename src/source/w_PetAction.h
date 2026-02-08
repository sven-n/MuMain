// w_PetAction.h: interface for the PetAction class.
//////////////////////////////////////////////////////////////////////

#include "ZzzBMD.h"
#pragma once

class PetAction
{
public:
    PetAction() {}
    virtual ~PetAction() {}
    virtual bool Release(OBJECT* obj, CHARACTER* Owner) { return false; }

public:
    virtual bool Model(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender) { return false; }
    virtual bool Move(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender) { return false; }
    virtual bool Effect(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender) { return false; }
    virtual bool Sound(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender) { return false; }
};
