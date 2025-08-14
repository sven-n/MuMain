// w_BasePet.cpp: implementation of the BasePet class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_BasePet.h"
#include "ZzzLodTerrain.h"
#include "ZzzObject.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"

extern float EarthQuake;

PetObjectPtr PetObject::Make()
{
    PetObjectPtr PetObject(new PetObject);
    PetObject->Init();
    return PetObject;
}

PetObject::PetObject() :
    m_moveType(eAction_Stand),
    m_oldMoveType(eAction_End),
    m_targetKey(-1),
    m_pOwner(NULL),
    m_itemType(-1)
{
}

PetObject::~PetObject()
{
    Release();
}

void PetObject::Init()
{
    m_obj = new OBJECT();
}

void PetObject::SetScale(float scale)
{
    if (NULL == m_obj) return;

    m_obj->Scale = scale;
}

void PetObject::SetBlendMesh(int blendMesh)
{
    if (NULL == m_obj) return;

    m_obj->BlendMesh = blendMesh;
}

bool PetObject::Create(int itemType, int modelType, vec3_t Position, CHARACTER* Owner, int SubType, int LinkBone)
{
    assert(Owner);
    if (m_obj->Live) return FALSE;

    m_timer = new CTimer();

    m_pOwner = Owner;
    m_itemType = itemType;

    m_obj->Type = modelType;
    m_obj->Live = TRUE;
    m_obj->Visible = FALSE;
    m_obj->LightEnable = TRUE;
    m_obj->ContrastEnable = FALSE;
    m_obj->AlphaEnable = FALSE;
    m_obj->EnableBoneMatrix = FALSE;
    m_obj->Owner = &m_pOwner->Object;
    m_obj->SubType = SubType;
    m_obj->HiddenMesh = -1;
    m_obj->BlendMesh = -1;
    m_obj->BlendMeshLight = 1.f;
    m_obj->Scale = 0.7f;
    m_obj->LifeTime = 30;
    m_obj->Alpha = 0.f;
    m_obj->AlphaTarget = 1.f;

    VectorCopy(Position, m_obj->Position);
    VectorCopy(m_obj->Owner->Angle, m_obj->Angle);
    Vector(3.f, 3.f, 3.f, m_obj->Light);

    m_obj->PriorAnimationFrame = 0.f;
    m_obj->AnimationFrame = 0.f;
    m_obj->Velocity = 0.5f;

    switch (m_obj->Type)
    {
    case MODEL_PET_SKELETON:
        m_obj->Position[1] += (60.0f * Owner->Object.Scale);
        break;
    }
    return TRUE;
}

void PetObject::Release()
{
    if (NULL != m_obj)
    {
        m_obj->Live = FALSE;
        delete m_obj;
        m_obj = nullptr;
    }

    for (ActionMap::iterator iter = m_actionMap.begin(); iter != m_actionMap.end(); iter++)
    {
        Weak_Ptr(PetAction) petAction = (*iter).second;

        if (petAction.expired() == FALSE)
        {
            petAction.lock()->Release(m_obj, m_pOwner);
        }
    }
    m_actionMap.clear();

    m_speedMap.clear();

    if (m_timer)
    {
      delete m_timer;
      m_timer = nullptr;
    }
}

void PetObject::Update(bool bForceRender)
{
    if (!m_obj->Live || NULL == m_obj->Owner || NULL == m_obj) return;

    if (SceneFlag == MAIN_SCENE)
    {
        if (!m_obj->Owner->Live || m_obj->Owner->Kind != KIND_PLAYER)
        {
            m_obj->Live = FALSE;
            return;
        }
    }

    Alpha(m_obj);

    //-----------------------------//
    auto tick = m_timer->GetTimeElapsed();
    UpdateModel(tick, bForceRender);
    UpdateMove(tick, bForceRender);
    UpdateSound(tick, bForceRender);
}

void PetObject::Render(bool bForceRender)
{
    if (m_obj->Live)
    {
        m_obj->Visible = (bForceRender == FALSE ? TestFrustrum2D(m_obj->Position[0] * 0.01f, m_obj->Position[1] * 0.01f, -20.f) : TRUE);

        if (m_obj->Visible)
        {
            int State = g_isCharacterBuff(m_obj->Owner, eBuff_Cloaking) ? 10 : 0;

            RenderObject(m_obj, FALSE, 0, State);

            CreateEffect(m_timer->GetTimeElapsed(), bForceRender);
        }
    }
}

bool PetObject::IsSameOwner(OBJECT* Owner)
{
    assert(Owner);

    return (Owner == m_obj->Owner) ? TRUE : FALSE;
}

bool PetObject::IsSameObject(OBJECT* Owner, int itemType)
{
    assert(Owner);

    return (Owner == m_obj->Owner && itemType == m_itemType) ? TRUE : FALSE;
}

void PetObject::SetActions(ActionType type, Weak_Ptr(PetAction) action, float speed)
{
    if (action.expired() == TRUE) return;

    m_actionMap.insert(std::make_pair(type, action));
    m_speedMap.insert(std::make_pair(type, speed));
}

void PetObject::SetCommand(int targetKey, ActionType cmdType)
{
    m_targetKey = targetKey;
    m_moveType = cmdType;
}

bool PetObject::UpdateMove(double tick, bool bForceRender)
{
    if (m_oldMoveType != m_moveType)
    {
        m_oldMoveType = m_moveType;

        auto iter2 = m_speedMap.find(m_moveType);
        if (iter2 == m_speedMap.end()) return FALSE;

        m_obj->Velocity = (*iter2).second;
    }

    auto iter = m_actionMap.find(m_moveType);
    if (iter == m_actionMap.end())
    {
        m_moveType = eAction_Stand;
        return FALSE;
    }

    //std::tr1::weak_ptr<PetAction> petAction = (*iter).second;

    Weak_Ptr(PetAction) petAction = (*iter).second;

    if (petAction.expired() == FALSE)
    {
        petAction.lock()->Move(m_obj, m_pOwner, m_targetKey, tick, bForceRender);
    }

    return TRUE;
}

bool PetObject::UpdateModel(double tick, bool bForceRender)
{
    auto iter = m_actionMap.find(m_moveType);
    if (iter == m_actionMap.end()) return FALSE;

    Weak_Ptr(PetAction) petAction = (*iter).second;

    if (petAction.expired() == FALSE)
    {
        petAction.lock()->Model(m_obj, m_pOwner, m_targetKey, tick, bForceRender);
    }

    BMD* b = &Models[m_obj->Type];
    b->CurrentAction = m_obj->CurrentAction;
    b->PlayAnimation(&m_obj->AnimationFrame, &m_obj->PriorAnimationFrame, &m_obj->PriorAction, m_obj->Velocity, m_obj->Position, m_obj->Angle);

    return TRUE;
}

bool PetObject::UpdateSound(double tick, bool bForceRender)
{
    auto iter = m_actionMap.find(m_moveType);
    if (iter == m_actionMap.end()) return FALSE;

    Weak_Ptr(PetAction) petAction = (*iter).second;

    if (petAction.expired() == FALSE)
    {
        petAction.lock()->Sound(m_obj, m_pOwner, m_targetKey, tick, bForceRender);
    }

    return TRUE;
}

bool PetObject::CreateEffect(double tick, bool bForceRender)
{
    auto iter = m_actionMap.find(m_moveType);
    if (iter == m_actionMap.end()) return FALSE;

    Weak_Ptr(PetAction) petAction = (*iter).second;

    if (petAction.expired() == FALSE)
    {
        petAction.lock()->Effect(m_obj, m_pOwner, m_targetKey, tick, bForceRender);
    }

    return TRUE;
}