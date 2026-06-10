#include "stdafx.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Effects/ZzzEffect.h"
#include "EffectBehaviors.h"

namespace Render::Effects::Behaviors
{
    void RenderDefault(OBJECT* o)
    {
        RenderObject(o);
    }

    // MODEL_MAYASTONE4 / MODEL_MAYASTONE5: launch a stone with a random lifetime,
    // size, spin and gravity, its initial direction rotated by the random spin.
    // The rand() draws happen in the original order so the sequence is preserved.
    void CreateMayaStone45(OBJECT* o)
    {
        vec3_t p1;
        float Matrix[3][4];
        Vector(0.f, (float)(rand() % 256 + 64) * 0.1f, 0.f, p1);
        o->LifeTime = rand() % 16 + 32;
        o->Scale = (float)(rand() % 10) / 3.0f + 1.0f;
        o->Angle[2] = (float)(rand() % 360);
        AngleMatrix(o->Angle, Matrix);
        VectorRotate(p1, Matrix, o->Direction);
        o->Gravity = (float)(rand() % 16 + 8);
    }

    // MODEL_DESAIR: rides the joint addressed by m_sTargetIndex and sheds
    // feather effects every 10 ticks of life.
    bool MoveDesair(OBJECT* o, int /*index*/, float /*luminosity*/)
    {
        if (o->m_sTargetIndex < 0 || o->m_sTargetIndex >= MAX_JOINTS)
            return true;
        JOINT* oj = &Joints[o->m_sTargetIndex];
        if (oj->Live == true)
        {
            VectorCopy(oj->Position, o->Position);
            VectorCopy(oj->Angle, o->Angle);
            if ((int)o->LifeTime % 10 == 0)
            {
                CreateEffectFpsChecked(MODEL_FEATHER, o->Position, o->Angle, o->Light, 2, NULL, -1, 0, 0, 0, 1.4f);
                CreateEffectFpsChecked(MODEL_FEATHER, o->Position, o->Angle, o->Light, 3, NULL, -1, 0, 0, 0, 1.4f);
            }
        }
        return true;
    }

    // MODEL_INFINITY_ARROW4: grows and fades while tracking the owner's hand bone.
    bool MoveInfinityArrow4(OBJECT* o, int /*index*/, float /*luminosity*/)
    {
        if (o->Owner == NULL)
        {
            o->LifeTime = 0;
            return true;
        }
        vec3_t tmp = { 0.f, 0.f, 0.f };
        OBJECT* pOwner = o->Owner;
        o->Scale *= pow(1.2f, FPS_ANIMATION_FACTOR);
        o->Light[0] *= pow(0.9f, FPS_ANIMATION_FACTOR);
        o->Light[1] *= pow(0.9f, FPS_ANIMATION_FACTOR);
        o->Light[2] *= pow(0.9f, FPS_ANIMATION_FACTOR);

        VectorTransform(tmp, pOwner->BoneTransform[29], o->Position);
        VectorScale(o->Position, pOwner->Scale, o->Position);
        VectorAddScaled(o->Position, pOwner->Position, o->Position, FPS_ANIMATION_FACTOR);
        VectorCopy(pOwner->Angle, o->Angle);
        return true;
    }

    // MODEL_MAGIC_CAPSULE2: sticks to the owner and fades out over its last ticks.
    bool MoveMagicCapsule2(OBJECT* o, int /*index*/, float /*luminosity*/)
    {
        if (o->Owner == NULL)
        {
            o->LifeTime = 0;
            return true;
        }
        VectorCopy(o->Owner->Position, o->Position);
        if (o->LifeTime < 10)
            o->BlendMeshLight = (float)o->LifeTime * 0.1f;
        return true;
    }

    // MODEL_SPEAR: trails a flare joint sized by subtype.
    bool MoveSpear(OBJECT* o, int /*index*/, float /*luminosity*/)
    {
        if (1 == o->SubType)
            CreateJointFpsChecked(BITMAP_FLARE, o->Position, o->Position, o->Angle, 12, o, 100.0f);
        else if (0 == o->SubType)
            CreateJointFpsChecked(BITMAP_FLARE, o->Position, o->Position, o->Angle, 4, o, 50.0f);
        return true;
    }

    // MODEL_SUMMONER_SUMMON_NEIL_NIFE1..3: fade out near end of life, otherwise fade in.
    bool MoveSummonerNeilNife(OBJECT* o, int /*index*/, float /*luminosity*/)
    {
        if (o->LifeTime < 20) o->Alpha -= 0.05f;
        else if (o->Alpha < 1.0f) o->Alpha += 0.05f;
        return true;
    }

    // MODEL_SUMMONER_SUMMON_NEIL_GROUND1..3: same fade-out, faster fade-in.
    bool MoveSummonerNeilGround(OBJECT* o, int /*index*/, float /*luminosity*/)
    {
        if (o->LifeTime < 20) o->Alpha -= 0.05f;
        else if (o->Alpha < 1.0f) o->Alpha += 0.3f;
        return true;
    }

    // BITMAP_FIRE: continuously emits its fire particle from the owner.
    bool MoveBitmapFire(OBJECT* o, int /*index*/, float /*luminosity*/)
    {
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        CreateParticleFpsChecked(BITMAP_FIRE, o->Position, o->Angle, Light, 9, 1.f, o->Owner);
        return true;
    }

    // BITMAP_FIRE_RED: scatters red fire particles around its position.
    bool MoveBitmapFireRed(OBJECT* o, int /*index*/, float /*luminosity*/)
    {
        vec3_t Position, Light;
        Vector((float)(rand() % 32 - 16), (float)(rand() % 32 - 16), 0.f, Position);
        VectorAdd(Position, o->Position, Position);
        Vector(1.0f, 0.4f, 0.4f, Light);
        CreateParticleFpsChecked(BITMAP_FIRE_RED, Position, o->Angle, Light, 0, o->Scale);
        return true;
    }

    // BITMAP_LIGHT_MARKS: dies with its owner, otherwise loops its lifetime.
    bool MoveBitmapLightMarks(OBJECT* o, int /*index*/, float /*luminosity*/)
    {
        if (o->Owner == NULL || o->Owner->Live == false)
        {
            o->Live = false;
        }
        else if (o->LifeTime <= 5)
        {
            o->LifeTime = 65;
        }
        return true;
    }

    // MODEL_MAGIC1: hovers above the owner, spinning and fading over its life.
    bool MoveMagic1(OBJECT* o, int /*index*/, float /*luminosity*/)
    {
        if (o->Owner == NULL)
        {
            o->LifeTime = 0;
            return true;
        }
        VectorCopy(o->Owner->Position, o->Position);
        o->Position[2] += (100.f) * FPS_ANIMATION_FACTOR;
        o->Angle[1] += (20.f) * FPS_ANIMATION_FACTOR;
        o->BlendMeshLight = o->LifeTime * 0.1f;
        return true;
    }

    // MODEL_MAYASTAR: pulses in size, spins, fades at end of life and shakes the screen.
    bool MoveMayaStar(OBJECT* o, int /*index*/, float /*luminosity*/)
    {
        if (o->LifeTime <= 20)
        {
            o->Light[0] *= pow(1.0f / (1.2f), FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(1.0f / (1.2f), FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(1.0f / (1.2f), FPS_ANIMATION_FACTOR);
        }
        o->Scale += (sinf(WorldTime * 0.005f) * 2.0f) * FPS_ANIMATION_FACTOR;
        o->Angle[1] += (10.0f) * FPS_ANIMATION_FACTOR;
        EarthQuake = (float)(rand() % 6 - 6) * 0.5f;
        return true;
    }
}
