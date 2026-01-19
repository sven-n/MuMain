// w_PetActionDemon.cpp: implementation of the PetActionDemon class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_PetActionDemon.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"

PetActionDemonPtr PetActionDemon::Make()
{
    PetActionDemonPtr petAction(new PetActionDemon);
    return petAction;
}

PetActionDemon::PetActionDemon()
{
}

PetActionDemon::~PetActionDemon()
{
}

bool PetActionDemon::Release(OBJECT* obj, CHARACTER* Owner)
{
    DeleteEffect(BITMAP_CLUD64, obj);
    DeleteEffect(BITMAP_FIRE_HIK3, obj);

    return TRUE;
}

bool PetActionDemon::Model(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    float fRad = (Q_PI / 10000.0f) * fmodf(tick, 10000);
    float temp = sinf(fRad);

    Vector(temp, temp, temp, obj->Light);

    return TRUE;
}

bool PetActionDemon::Move(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    float FlyRange = 50.f;
    vec3_t Range, Direction;

    obj->Position[2] = obj->Owner->Position[2] + (230.0f * obj->Owner->Scale);

    VectorSubtract(obj->Owner->Position, obj->Position, Range);

    float Distance = Range[0] * Range[0] + Range[1] * Range[1];
    if (Distance >= FlyRange * FlyRange * FlyRange * FlyRange)
    {
        VectorCopy(obj->Owner->Position, obj->Position);
        VectorCopy(obj->Owner->Angle, obj->Angle);
    }
    else if (Distance >= FlyRange * FlyRange)
    {
        float Angle = CreateAngle2D(obj->Position, obj->Owner->Position);
        obj->Angle[2] = TurnAngle2(obj->Angle[2], Angle, 10.0f * FPS_ANIMATION_FACTOR);
    }

    AngleMatrix(obj->Angle, obj->Matrix);
    VectorRotate(obj->Direction, obj->Matrix, Direction);
    VectorAddScaled(obj->Position, Direction, obj->Position, FPS_ANIMATION_FACTOR);

    float Speed = (FlyRange * FlyRange >= Distance) ? 0 : (float)log(Distance) + 5.0f;

    obj->Direction[0] = 0.0f;
    obj->Direction[1] = -Speed;
    obj->Direction[2] = 0.0f;

    return TRUE;
}

bool PetActionDemon::Effect(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    BMD* b = &Models[obj->Type];
    vec3_t Position, vRelativePos;
    vec3_t Light, Light2;
    float fRad = ((Q_PI / 2500.0f) * fmodf(tick, 25000));
    float temp = sinf(fRad) + 0.4f;

    Vector(0.f, 0.f, 0.f, vRelativePos);
    Vector(temp * 0.7f, temp * 0.5f, temp * 0.6f, Light);
    Vector(0.7f, 0.3f, 0.3f, Light2);

    int itemp[] = { 8, 34, 51, 61, 56, 66, 52, 58, 44 };

    for (int i = 0; i < 9; i++)
    {
        b->TransformPosition(BoneTransform[itemp[i]], vRelativePos, Position, false);

        switch (i)
        {
        case 0:
            CreateSprite(BITMAP_LIGHTMARKS_FOREIGN, Position, 1.0f, Light, obj);
            CreateSprite(BITMAP_FLARE, Position, 0.5f, Light, obj);
            break;

        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            CreateParticleFpsChecked(BITMAP_CLUD64, Position, obj->Angle, Light, 11, 0.5f);
            break;

        case 6:
        case 7:
            CreateSprite(BITMAP_LIGHTMARKS_FOREIGN, Position, 0.5f, Light, obj);
            break;

        case 8:
            CreateParticleFpsChecked(BITMAP_FIRE_HIK3, Position, obj->Angle, Light, 1, 0.4f);
            CreateSprite(BITMAP_FLARE, Position, 1.5f, Light, obj);
            CreateSprite(BITMAP_FLARE, Position, 0.5f, Light2, obj);
            break;
        }
    }
    return TRUE;
}