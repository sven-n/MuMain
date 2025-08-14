#pragma once

#include "w_PetAction.h"
#include "ZzzBMD.h"
#include "Time/Timer.h"

SmartPointer(PetObject);
class PetObject
{
public:
    enum ActionType {
        eAction_Stand = 0,
        eAction_Move,
        eAction_Attack,
        eAction_Skill,
        eAction_Birth,
        eAction_Dead,
        eAction_End
    };

public:
    static PetObjectPtr Make();
    virtual ~PetObject();

public:
    OBJECT* GetObject() { return m_obj; }
    bool IsSameOwner(OBJECT* Owner);
    bool IsSameObject(OBJECT* Owner, int itemType);
    void SetActions(ActionType type, Weak_Ptr(PetAction) action, float speed);
    void SetCommand(int targetKey, ActionType cmdType);

    void SetScale(float scale = 0.0f);
    void SetBlendMesh(int blendMesh = -1);

public:
    bool Create(int itemType, int modelType, vec3_t Position, CHARACTER* Owner, int SubType, int LinkBone);
    void Release();
    void Update(bool bForceRender = false);
    void Render(bool bForceRender = false);

private:
    bool UpdateMove(double tick, bool bForceRender = false);
    bool UpdateModel(double tick, bool bForceRender = false);
    bool UpdateSound(double tick, bool bForceRender = false);

    bool CreateEffect(double tick, bool bForceRender = false);

private:
    void Init();
    PetObject();

public:
    typedef std::map< ActionType, Weak_Ptr(PetAction) > ActionMap;
    typedef std::map< ActionType, float > SpeedMap;

private:
    ActionMap m_actionMap;
    SpeedMap m_speedMap;

    CHARACTER* m_pOwner;
    OBJECT* m_obj;
    int m_targetKey;
    int m_itemType;

    CTimer* m_timer;

    ActionType m_moveType;
    ActionType m_oldMoveType;
};
