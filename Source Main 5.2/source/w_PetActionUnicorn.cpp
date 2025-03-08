// w_PetActionUnicorn.cpp: implementation of the w_PetActionUnicorn class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_PetActionUnicorn.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "ZzzCharacter.h"
#include "ZzzObject.h"

#include "DSPlaySound.h"
#include "WSclient.h"

PetActionUnicornPtr PetActionUnicorn::Make()
{
    PetActionUnicornPtr temp(new PetActionUnicorn);
    return temp;
}

PetActionUnicorn::PetActionUnicorn()
{
    m_isRooting = false;

    m_dwSendDelayTime = 0;
    m_dwRootingTime = 0;
    m_dwRoundCountDelay = 0;
    m_state = eAction_Stand;

    m_fRadWidthStand = 0.0f;
    m_fRadWidthGet = 0.0f;

    m_speed = 0;
}

PetActionUnicorn::~PetActionUnicorn()
{
}

bool PetActionUnicorn::Release(OBJECT* obj, CHARACTER* Owner)
{
    DeleteEffect(BITMAP_PIN_LIGHT);

    return TRUE;
}

bool PetActionUnicorn::Model(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    if (NULL == obj || NULL == Owner) return FALSE;

    return false;
}

bool PetActionUnicorn::Move(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    if (NULL == obj || NULL == Owner) return FALSE;

    FindZen(obj);

    if (eAction_Stand == m_state && m_isRooting)
    {
        m_state = eAction_Move;
    }

    if (m_speed == 0)
    {
        m_speed = obj->Velocity;
    }

    float FlyRange = 10.0f;
    vec3_t targetPos, Range, Direction;
    bool _isMove = false;

    float fRadHeight = ((2 * Q_PI) / 15000.0f) * fmodf(tick, 15000);
    m_fRadWidthStand = ((2 * Q_PI) / 4000.0f) * fmodf(tick, 4000);
    m_fRadWidthGet = ((2 * Q_PI) / 2000.0f) * fmodf(tick, 2000);

    obj->Position[2] = obj->Owner->Position[2] + (200.0f * obj->Owner->Scale);

    VectorSubtract(obj->Position, obj->Owner->Position, Range);

    float Distance = sqrtf(Range[0] * Range[0] + Range[1] * Range[1]);
    if (Distance > SEARCH_LENGTH * 3)
    {
        obj->Position[0] = obj->Owner->Position[0] + (sinf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);
        obj->Position[1] = obj->Owner->Position[1] + (cosf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);

        VectorCopy(obj->Owner->Angle, obj->Angle);

        m_state = eAction_Stand;
        m_isRooting = false;
    }

    switch (m_state)
    {
    case eAction_Stand:
    {
        /*/
                    targetPos[0] = obj->Owner->Position[0] + (sinf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN * 3);
                    targetPos[1] = obj->Owner->Position[1] + (cosf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN * 3);
        /*/
        targetPos[0] = obj->Owner->Position[0];
        targetPos[1] = obj->Owner->Position[1];
        //*/
        targetPos[2] = obj->Owner->Position[2];

        VectorSubtract(targetPos, obj->Position, Range);
        //------------------------------//
        Distance = sqrtf(Range[0] * Range[0] + Range[1] * Range[1]);

        if (80.0f >= FlyRange)
        {
            float Angle = CreateAngle2D(obj->Position, targetPos); //test
            obj->Angle[2] = TurnAngle2(obj->Angle[2], Angle, 8.0f * FPS_ANIMATION_FACTOR);
        }

        AngleMatrix(obj->Angle, obj->Matrix);
        VectorRotate(obj->Direction, obj->Matrix, Direction);
        VectorAddScaled(obj->Position, Direction, obj->Position, FPS_ANIMATION_FACTOR);

        //	float Speed = ( FlyRange >= Distance ) ?  0 : (float)log(Distance) * 2.3f;
        float Speed = (FlyRange * FlyRange >= Distance) ? 0 : (float)log(Distance) * 2.3f;

        obj->Direction[0] = 0.0f;
        obj->Direction[1] = -Speed;
        obj->Direction[2] = 0.0f;

        if (Speed == 0)
        {
            obj->Velocity = m_speed * 0.35f;
        }
        else
        {
            obj->Velocity = m_speed * 1.2f;
        }
    }
    break;

    case eAction_Move:
    {
        if (!m_isRooting)
        {
            m_isRooting = false;
            m_state = eAction_Return;
            break;
        }

        targetPos[0] = m_RootItem.position[0] + (sinf(m_fRadWidthGet) * CIRCLE_STAND_RADIAN);
        targetPos[1] = m_RootItem.position[1] + (cosf(m_fRadWidthGet) * CIRCLE_STAND_RADIAN);
        targetPos[2] = m_RootItem.position[2];// + 70 + (sinf(fRadHeight) * 70.0f);

        VectorSubtract(targetPos, obj->Position, Range);

        Distance = sqrtf(Range[0] * Range[0] + Range[1] * Range[1]);
        if (Distance >= FlyRange)
        {
            float Angle = CreateAngle2D(obj->Position, targetPos); //test
            obj->Angle[2] = TurnAngle2(obj->Angle[2], Angle, 20.0f * FPS_ANIMATION_FACTOR);
        }

        AngleMatrix(obj->Angle, obj->Matrix);
        VectorRotate(obj->Direction, obj->Matrix, Direction);
        VectorAddScaled(obj->Position, Direction, obj->Position, FPS_ANIMATION_FACTOR);

        float Speed = (20.0f >= Distance) ? 0 : (float)log(Distance) * 2.5f;

        obj->Direction[0] = 0.0f;
        obj->Direction[1] = -Speed;
        obj->Direction[2] = 0.0f;

        if (0 == Speed || CompTimeControl(100000, m_dwRootingTime))
        {
            m_dwSendDelayTime = GetTickCount();
            m_dwRootingTime = GetTickCount();
            m_state = eAction_Get;
        }

        obj->Velocity = m_speed;
    }
    break;

    case eAction_Get:
    {
        if (!m_isRooting || SEARCH_LENGTH < Distance || CompTimeControl(3000, m_dwRootingTime))
        {
            m_isRooting = false;
            m_dwRootingTime = GetTickCount();
            m_state = eAction_Return;
            break;
        }

        //------------------------------//
        VectorCopy(m_RootItem.position, targetPos);

        float Angle = CreateAngle2D(obj->Position, targetPos);
        obj->Angle[2] = TurnAngle2(obj->Angle[2], Angle, 10.0f * FPS_ANIMATION_FACTOR);
        //------------------------------//

        if (CompTimeControl(1000, m_dwSendDelayTime)
            && &Hero->Object == obj->Owner
            && SendGetItem == -1)
        {
            SendGetItem = m_RootItem.itemIndex;
            SocketClient->ToGameServer()->SendPickupItemRequest(m_RootItem.itemIndex);
        }
        obj->Velocity = m_speed;
    }
    break;

    case eAction_Return:
    {
        targetPos[0] = obj->Owner->Position[0] + (sinf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);
        targetPos[1] = obj->Owner->Position[1] + (cosf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);
        targetPos[2] = obj->Owner->Position[2];// + 70 + (sinf(fRadHeight) * 70.0f);

        VectorSubtract(targetPos, obj->Position, Range);

        Distance = sqrtf(Range[0] * Range[0] + Range[1] * Range[1]);
        if (Distance >= FlyRange)
        {
            float Angle = CreateAngle2D(obj->Position, targetPos);
            obj->Angle[2] = TurnAngle2(obj->Angle[2], Angle, 20.0f * FPS_ANIMATION_FACTOR);
        }

        AngleMatrix(obj->Angle, obj->Matrix);
        VectorRotate(obj->Direction, obj->Matrix, Direction);
        VectorAddScaled(obj->Position, Direction, obj->Position, FPS_ANIMATION_FACTOR);

        float Speed = (FlyRange >= Distance) ? 0 : (float)log(Distance) * 2.5f;

        obj->Direction[0] = 0.0f;
        obj->Direction[1] = -Speed;
        obj->Direction[2] = 0.0f;

        if (0 == Speed || CompTimeControl(3000, m_dwRootingTime))
        {
            m_state = eAction_Stand;
        }
        obj->Velocity = m_speed;
    }
    break;
    }

    return TRUE;
}

bool PetActionUnicorn::Effect(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    if (NULL == obj || NULL == Owner) return FALSE;

    BMD* b = &Models[obj->Type];
    vec3_t Position, vRelativePos, Light;

    VectorCopy(obj->Position, b->BodyOrigin);
    Vector(0.f, 0.f, 0.f, vRelativePos);

    b->Animation(BoneTransform, obj->AnimationFrame, obj->PriorAnimationFrame, obj->PriorAction, obj->Angle, obj->HeadAngle);

    Vector(0.f, 0.f, 0.f, vRelativePos);
    b->TransformPosition(BoneTransform[11], vRelativePos, Position, false);
    Vector(1.0f, 0.7f, 0.0f, Light);
    CreateSprite(BITMAP_MAGIC, Position, 0.15f, Light, obj);

    Vector(1.0f, 0.7f, 0.3f, Light);
    if (rand_fps_check(3))
        CreateEffect(BITMAP_PIN_LIGHT, Position, obj->Angle, Light, 4, obj, -1, 0, 0, 0, 0.45f);

    b->TransformPosition(BoneTransform[4], vRelativePos, Position, false);
    Vector(0.5f, 0.5f, 1.0f, Light);

    CreateSprite(BITMAP_SMOKE, Position, 1.2f, Light, obj);
    CreateSprite(BITMAP_LIGHT, Position, 4.0f, Light, obj);

    if (rand_fps_check(2))
        CreateParticle(BITMAP_SMOKE, Position, obj->Angle, Light, 67, 1.0f);

    Vector(0.7f, 0.7f, 1.0f, Light);
    b->TransformPosition(BoneTransform[4], vRelativePos, Position, false);
    CreateParticleFpsChecked(BITMAP_SMOKELINE1, Position, obj->Angle, Light, 4, 0.6f, obj);
    CreateParticleFpsChecked(BITMAP_SMOKELINE2, Position, obj->Angle, Light, 4, 0.6f, obj);
    CreateParticleFpsChecked(BITMAP_SMOKELINE3, Position, obj->Angle, Light, 4, 0.6f, obj);

    return TRUE;
}

bool PetActionUnicorn::Sound(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    if (NULL == obj || NULL == Owner) return FALSE;

    switch (m_state)
    {
    case eAction_Return:
        PlayBuffer(SOUND_DROP_GOLD01);
        break;
    }

    return TRUE;
}

void PetActionUnicorn::FindZen(OBJECT* obj)
{
    if (NULL == obj || true == m_isRooting) return;

    float dx, dy, dl;
    bool sameItem = false;

    for (int i = 0; i < MAX_ITEMS; i++)
    {
        OBJECT* _item = &Items[i].Object;
        if (_item->Live == false || _item->Visible == false)
        {
            continue;
        }

        dx = obj->Owner->Position[0] - _item->Position[0];
        dy = obj->Owner->Position[1] - _item->Position[1];

        dl = sqrtf(dx * dx + dy * dy);

        if (SEARCH_LENGTH > dl)
        {
            if (Items[i].Item.Type != ITEM_ZEN)
            {
                continue;
            }

            if (!m_isRooting)
            {
                m_isRooting = true;
                m_RootItem.itemIndex = i;
                VectorCopy(_item->Position, m_RootItem.position);
            }
        }
    }
}

bool PetActionUnicorn::CompTimeControl(const DWORD& dwCompTime, DWORD& dwTime)
{
    if ((timeGetTime() - dwTime) > dwCompTime)
    {
        dwTime = timeGetTime();
        return true;
    }
    return false;
}