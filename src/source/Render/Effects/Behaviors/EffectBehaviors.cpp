#include "stdafx.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Render/Effects/ZzzEffect.h"
#include "EffectBehaviors.h"

namespace Render::Effects::Behaviors
{
    void RenderDefault(OBJECT* o)
    {
        RenderObject(o);
    }

    // MODEL_DESAIR: rides the joint addressed by m_sTargetIndex and sheds
    // feather effects every 10 ticks of life.
    bool MoveDesair(OBJECT* o, int /*index*/)
    {
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
}
