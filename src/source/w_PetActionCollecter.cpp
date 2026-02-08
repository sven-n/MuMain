// w_PetActionCollecter.cpp: implementation of the PetActionStand class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_PetActionCollecter.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "ZzzCharacter.h"
#include "ZzzObject.h"

#include "DSPlaySound.h"
#include "WSclient.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PetActionCollecterPtr PetActionCollecter::Make()
{
    PetActionCollecterPtr temp(new PetActionCollecter);
    return temp;
}

PetActionCollecter::PetActionCollecter()
{
    m_isRooting = false;

    m_dwSendDelayTime = 0;
    m_dwRootingTime = 0;
    m_dwRoundCountDelay = 0;
    m_state = eAction_Stand;

    m_fRadWidthStand = 0.0f;
    m_fRadWidthGet = 0.0f;
}

PetActionCollecter::~PetActionCollecter()
{
}

bool PetActionCollecter::Release(OBJECT* obj, CHARACTER* Owner)
{
    DeleteEffect(MODEL_NEWYEARSDAY_EVENT_MONEY, obj);

    return TRUE;
}

bool PetActionCollecter::Model(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    if (NULL == obj || NULL == Owner) return FALSE;

    return false;
}

bool PetActionCollecter::Move(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    if (NULL == obj || NULL == Owner) return FALSE;

    FindZen(obj);

    if (eAction_Stand == m_state && m_isRooting)
    {
        m_state = eAction_Move;
    }

    //------------------------------------------//
    float FlyRange = 10.0f;
    vec3_t targetPos, Range, Direction;
    m_fRadWidthStand = ((2 * Q_PI) / 4000.0f) * fmodf(tick, 4000);
    m_fRadWidthGet = ((2 * Q_PI) / 2000.0f) * fmodf(tick, 2000);

    obj->Position[2] = obj->Owner->Position[2] + 20.0f;
    //------------------------------------------//
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
        targetPos[0] = obj->Owner->Position[0] + (sinf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);
        targetPos[1] = obj->Owner->Position[1] + (cosf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);
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

        float Speed = (FlyRange >= Distance) ? 0 : (float)log(Distance) * 2.3f;

        obj->Direction[0] = 0.0f;
        obj->Direction[1] = -Speed;
        obj->Direction[2] = 0.0f;
        //*/
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
        //------------------------------//

        Distance = sqrtf(Range[0] * Range[0] + Range[1] * Range[1]);
        if (Distance >= FlyRange)
        {
            float Angle = CreateAngle2D(obj->Position, targetPos); //test
            obj->Angle[2] = TurnAngle2(obj->Angle[2], Angle, 20.0f);
        }

        AngleMatrix(obj->Angle, obj->Matrix);
        VectorRotate(obj->Direction, obj->Matrix, Direction);
        VectorAddScaled(obj->Position, Direction, obj->Position, FPS_ANIMATION_FACTOR);

        float Speed = (20.0f >= Distance) ? 0 : logf(Distance) * 2.5f;

        obj->Direction[0] = 0.0f;
        obj->Direction[1] = -Speed;
        obj->Direction[2] = 0.0f;

        if (0 == Speed || CompTimeControl(100000, m_dwRootingTime))
        {
            m_dwSendDelayTime = GetTickCount();
            m_dwRootingTime = GetTickCount();
            m_state = eAction_Get;
        }
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

        VectorCopy(m_RootItem.position, targetPos);

        float Angle = CreateAngle2D(obj->Position, targetPos);
        obj->Angle[2] = TurnAngle2(obj->Angle[2], Angle, 10.0f);

        if (CompTimeControl(1000, m_dwSendDelayTime)
            && &Hero->Object == obj->Owner
            && SendGetItem == -1)
        {
            SendGetItem = m_RootItem.itemIndex;
            SocketClient->ToGameServer()->SendPickupItemRequest(m_RootItem.itemIndex);
        }
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
            obj->Angle[2] = TurnAngle2(obj->Angle[2], Angle, 20.0f);
        }

        AngleMatrix(obj->Angle, obj->Matrix);
        VectorRotate(obj->Direction, obj->Matrix, Direction);
        VectorAddScaled(obj->Position, Direction, obj->Position, FPS_ANIMATION_FACTOR);

        float Speed = (FlyRange >= Distance) ? 0 : logf(Distance) * 2.5f;

        obj->Direction[0] = 0.0f;
        obj->Direction[1] = -Speed;
        obj->Direction[2] = 0.0f;

        if (0 == Speed || CompTimeControl(3000, m_dwRootingTime))
        {
            m_state = eAction_Stand;
        }
    }
    break;
    }

    return TRUE;
}

bool PetActionCollecter::Effect(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
{
    if (NULL == obj || NULL == Owner) return FALSE;

    BMD* b = &Models[obj->Type];
    vec3_t Position, vRelativePos, Light;

    VectorCopy(obj->Position, b->BodyOrigin);
    Vector(0.f, 0.f, 0.f, vRelativePos);

    b->Animation(BoneTransform, obj->AnimationFrame, obj->PriorAnimationFrame, obj->PriorAction, obj->Angle, obj->HeadAngle);

    float fRad1 = ((Q_PI / 3000.0f) * fmodf(tick, 3000));
    float fSize = sinf(fRad1) * 0.2f;
    float fSize2 = 1.0f;

    Vector(1.0f, 0.8f, 0.2f, Light);
    VectorCopy(obj->Position, Position);
    Position[2] += 30.0f;
    CreateParticleFpsChecked(BITMAP_SHINY, Position, obj->Angle, Light, 7);

    switch (m_state)
    {
    case eAction_Move:
        fSize = 0.8f;
        fSize2 = 3.0f;
        break;

    case eAction_Get:
        fSize = 0.8f;
        fSize2 = 3.0f;
        break;

    case eAction_Return:
        CreateEffect(MODEL_NEWYEARSDAY_EVENT_MONEY, Position, obj->Angle, Light);
        break;
    }

    b->TransformPosition(BoneTransform[10], vRelativePos, Position, false);
    Vector(1.0f, 0.8f, 0.2f, Light);
    CreateSprite(BITMAP_FLARE_RED, Position, (0.5f + fSize), Light, obj);
    Vector(1.0f, 0.1f, 0.2f, Light);
    CreateSprite(BITMAP_LIGHT, Position, (2.0f + fSize), Light, obj);

    int temp[] = { 19, 32, 33, 34, 35 };
    for (int i = 0; i < 5; i++)
    {
        b->TransformPosition(BoneTransform[temp[i]], vRelativePos, Position, false);
        Vector(0.8f, 0.6f, 0.2f, Light);
        CreateSprite(BITMAP_LIGHT, Position, (0.6f * fSize2), Light, obj);
        Vector(0.8f, 0.8f, 0.2f, Light);
        CreateSprite(BITMAP_SHINY + 1, Position, (0.4f * fSize2), Light, obj);
    }
    return TRUE;
}

bool PetActionCollecter::Sound(OBJECT* obj, CHARACTER* Owner, int targetKey, double tick, bool bForceRender)
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

void PetActionCollecter::FindZen(OBJECT* obj)
{
    if (NULL == obj || true == m_isRooting) return;

    float dx, dy, dl;

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
            //if( -1 == g_pMyInventory->FindEmptySlot(&Items[i].Item) && Items[i].Item.Type != ITEM_POTION+15 )
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

bool PetActionCollecter::CompTimeControl(const DWORD& dwCompTime, DWORD& dwTime)
{
    if ((timeGetTime() - dwTime) > dwCompTime)
    {
        dwTime = timeGetTime();
        return true;
    }
    return false;
}