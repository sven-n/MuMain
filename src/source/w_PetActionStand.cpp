// w_PetActionStand.cpp: implementation of the PetActionStand class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_PetActionStand.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "ZzzCharacter.h"

PetActionStandPtr PetActionStand::Make()
{
    PetActionStandPtr temp(new PetActionStand);
    return temp;
}

PetActionStand::PetActionStand()
{
}

PetActionStand::~PetActionStand()
{
}

bool PetActionStand::Release(OBJECT* obj, CHARACTER* Owner)
{
    DeleteEffect(MODEL_FEATHER_FOREIGN, obj);
    return TRUE;
}

bool PetActionStand::Model(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    return false;
}

bool PetActionStand::Move(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    float FlyRange = 50.f;
    vec3_t Range, Direction;

    obj->Position[2] = obj->Owner->Position[2] + (230.0f * obj->Owner->Scale);

    VectorSubtract(obj->Owner->Position, obj->Position, Range);

    float Distance = Range[0] * Range[0] + Range[1] * Range[1];
    if (Distance >= FlyRange * FlyRange * FlyRange * FlyRange)
    {
        VectorCopy(obj->Owner->Position, obj->Position);
        obj->Position[2] = obj->Owner->Position[2] + 200;
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

    float Speed = (FlyRange * FlyRange >= Distance) ? 0 : logf(Distance) * 1.8f;

    obj->Direction[0] = 0.0f;
    obj->Direction[1] = -Speed;
    obj->Direction[2] = 0.0f;

    return TRUE;
}

bool PetActionStand::Effect(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    BMD* b = &Models[obj->Type];
    vec3_t Position, vRelativePos;
    vec3_t Light;

    float fRad1 = ((Q_PI / 3000.0f) * fmodf(tick, 3000));
    float fRad2 = ((Q_PI / 3000.0f) * fmodf((tick + 1500), 3000));
    float tempLight;

    VectorCopy(obj->Position, b->BodyOrigin);
    Vector(0.f, 0.f, 0.f, vRelativePos);

    b->Animation(BoneTransform, obj->AnimationFrame, obj->PriorAnimationFrame, obj->PriorAction, obj->Angle, obj->HeadAngle);

    Vector(0.7f, 0.2f, 0.6f, Light);
    b->TransformPosition(BoneTransform[3], vRelativePos, Position, false);
    CreateSprite(BITMAP_LIGHTMARKS_FOREIGN, Position, 1.5f, Light, obj);
    CreateSprite(BITMAP_LIGHTMARKS_FOREIGN, Position, 0.8f, Light, obj);

    Vector(0.3f, 0.3f, 0.6f, Light);
    b->TransformPosition(BoneTransform[5], vRelativePos, Position, false);
    Position[2] -= 25.0f;
    CreateEffect(MODEL_FEATHER_FOREIGN, Position, obj->Angle, Light, 4, NULL, -1, 0, 0, 0, 0.3f);

    int temp[] = { 45, 42, 48, 54, 51, 57, 25, 26, 27, 38, 32 };

    for (int i = 0; i < 11; ++i)
    {
        b->TransformPosition(BoneTransform[temp[i]], vRelativePos, Position, false);

        switch (i)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            Vector(0.5f, 0.5f, 0.8f, Light);
            CreateSprite(BITMAP_LIGHTMARKS_FOREIGN, Position, 0.3f, Light, obj);
            break;

        case 6:
        case 7:
        case 8:
            Vector(0.6f, 0.2f, 0.8f, Light);
            CreateSprite(BITMAP_LIGHT, Position, 0.2f, Light, obj);
            break;

        case 9:
        case 10:
            tempLight = (0 == i % 2) ? sinf(fRad1) * 2.0f : sinf(fRad2) * 2.0f;
            Vector(tempLight, tempLight, tempLight, Light);
            CreateSprite(BITMAP_FLARE, Position, 0.5f, Light, obj);
            break;
        }
    }
    return TRUE;
}