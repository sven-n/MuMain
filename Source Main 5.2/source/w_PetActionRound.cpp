// w_PetActionRound.cpp: implementation of the PetActionRound class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_PetActionRound.h"
#include "ZzzAI.h"

PetActionRoundPtr PetActionRound::Make()
{
    PetActionRoundPtr petActionRound(new PetActionRound);
    return petActionRound;
}

PetActionRound::PetActionRound()
{
}

PetActionRound::~PetActionRound()
{
}

bool PetActionRound::Release(OBJECT* obj, CHARACTER* Owner)
{
    return false;
}

bool PetActionRound::Model(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    return false;
}

bool PetActionRound::Move(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    //*/
    float fRadWidth = ((2 * Q_PI) / 5000.0f) * fmodf(tick, 5000);
    float fRadHeight = ((2 * Q_PI) / 1000.0f) * fmodf(tick, 1000);
    vec3_t  TargetPosition;

    VectorCopy(obj->Owner->Position, TargetPosition);
    VectorCopy(obj->Owner->HeadAngle, obj->HeadAngle);

    obj->Position[0] = TargetPosition[0] + (sinf(fRadWidth) * 150.0f);
    obj->Position[1] = TargetPosition[1] + (cosf(fRadWidth) * 150.0f);
    obj->Position[2] = TargetPosition[2] + 100 + (sinf(fRadHeight) * 30.0f);

    float Angle = CreateAngle2D(obj->Position, TargetPosition);
    obj->Angle[2] = TurnAngle2(obj->Angle[2], Angle + 270, 20.0f * FPS_ANIMATION_FACTOR);
    //*/
    return TRUE;
}

bool PetActionRound::Effect(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    return false;
}