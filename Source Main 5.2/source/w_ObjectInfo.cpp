// w_ObjectInfo.cpp: implementation of the OBJECT class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_ObjectInfo.h"

void CInterpolateContainer::GetCurrentValue(vec3_t& v3Out, float fCurrentRate, VEC_INTERPOLATES& vecInterpolates)
{
    auto iterBegin = vecInterpolates.begin();
    auto iterEnd = vecInterpolates.end();
    VEC_INTERPOLATES::iterator	iter_;

    INTERPOLATE_FACTOR* pCurFactor = NULL;

    bool	bFindInterpolateFactor = false;

    for (iter_ = iterBegin; iter_ < iterEnd; ++iter_)
    {
        INTERPOLATE_FACTOR& interpolateFactor = (*iter_);

        if (interpolateFactor.fRateStart <= fCurrentRate && interpolateFactor.fRateEnd > fCurrentRate)
        {
            bFindInterpolateFactor = true;
            pCurFactor = &interpolateFactor;
            break;
        }
    }

    if (bFindInterpolateFactor == true)
    {
        VectorInterpolation_F(v3Out,
            pCurFactor->v3Start,
            pCurFactor->v3End,
            pCurFactor->fRateEnd - pCurFactor->fRateStart,
            fCurrentRate - pCurFactor->fRateStart);
    }
}

void CInterpolateContainer::GetCurrentValueF(float& fOut, float fCurrentRate, VEC_INTERPOLATES_F& vecInterpolates)
{
    auto iterBegin = vecInterpolates.begin();
    auto iterEnd = vecInterpolates.end();
    VEC_INTERPOLATES_F::iterator	iter_;

    INTERPOLATE_FACTOR_F* pCurFactor = NULL;

    bool	bFindInterpolateFactor = false;

    for (iter_ = iterBegin; iter_ < iterEnd; ++iter_)
    {
        INTERPOLATE_FACTOR_F& interpolateFactor = (*iter_);

        if (interpolateFactor.fRateStart <= fCurrentRate && interpolateFactor.fRateEnd > fCurrentRate)
        {
            bFindInterpolateFactor = true;
            pCurFactor = &interpolateFactor;
            break;
        }
    }

    if (bFindInterpolateFactor == true)
    {
        LInterpolationF(fOut,
            pCurFactor->fStart,
            pCurFactor->fEnd,
            (float)(fCurrentRate - pCurFactor->fRateStart) / (float)(pCurFactor->fRateEnd - pCurFactor->fRateStart));
    }
}

void CInterpolateContainer::ClearContainer()
{
    m_vecInterpolatesPos.clear();
    m_vecInterpolatesAngle.clear();
    m_vecInterpolatesScale.clear();
    m_vecInterpolatesAlpha.clear();
}

OBJECT::OBJECT()
{
    Initialize();
}

OBJECT::~OBJECT()
{
    Destroy();
}

void OBJECT::Initialize()
{
    m_bpcroom = false;
    Live = false;
    bBillBoard = false;
    m_bCollisionCheck = false;
    m_bRenderShadow = false;
    EnableShadow = false;
    LightEnable = false;
    m_bActionStart = false;
    Visible = false;
    AlphaEnable = false;
    EnableBoneMatrix = false;
    ContrastEnable = false;
    ChromeEnable = false;
    m_bRenderAfterCharacter = false;

    AI = 0;
    CurrentAction = 0;
    PriorAction = 0;

    ExtState = 0;
    Teleport = 0;
    Kind = 0;
    Skill = 0;
    m_byNumCloth = 0;
    m_byHurtByDeathstab = 0;
    WeaponLevel = 0;
    DamageTime = 0;
    m_byBuildTime = 0;
    m_bySkillCount = 0;
    m_bySkillSerialNum = 0;
    Block = 0;

    ScreenX = 0;
    ScreenY = 0;
    PKKey = 0;
    Weapon = 0;

    Type = 0;
    SubType = 0;
    m_iAnimation = 0;
    HiddenMesh = 0;
    LifeTime = 0;
    BlendMesh = 0;
    AttackPoint[0] = 0;
    AttackPoint[1] = 0;
    RenderType = 0;
    InitialSceneTime = 0;
    LinkBone = 0;

    m_dwTime = 0;

    Scale = 0.0f;
    BlendMeshLight = 0.0f;
    BlendMeshTexCoordU = 0.0f;
    BlendMeshTexCoordV = 0.0f;
    Timer = 0.0f;
    m_fEdgeScale = 0.0f;
    Velocity = 0.0f;
    CollisionRange = 0.0f;
    ShadowScale = 0.0f;
    Gravity = 0.0f;
    Distance = 0.0f;
    AnimationFrame = 0.0f;
    PriorAnimationFrame = 0.0f;
    AlphaTarget = 0.0f;
    Alpha = 0.0f;

    IdentityMatrix(Matrix);

    IdentityVector3D(Light);
    IdentityVector3D(Direction);
    IdentityVector3D(m_vPosSword);
    IdentityVector3D(StartPosition);
    IdentityVector3D(BoundingBoxMin);
    IdentityVector3D(BoundingBoxMax);
    IdentityVector3D(m_vDownAngle);
    IdentityVector3D(m_vDeadPosition);
    IdentityVector3D(Position);
    IdentityVector3D(Angle);
    IdentityVector3D(HeadAngle);
    IdentityVector3D(HeadTargetAngle);
    IdentityVector3D(EyeLeft);
    IdentityVector3D(EyeRight);
    IdentityVector3D(EyeLeft2);
    IdentityVector3D(EyeRight2);
    IdentityVector3D(EyeLeft3);
    IdentityVector3D(EyeRight3);

    IdentityVector3D(OBB.StartPos);
    IdentityVector3D(OBB.XAxis);
    IdentityVector3D(OBB.YAxis);
    IdentityVector3D(OBB.ZAxis);

    m_pCloth = NULL;
    BoneTransform = NULL;

    Owner = NULL;
    Prior = NULL;
    Next = NULL;
}

void OBJECT::Destroy()
{
    //m_BuffMap.ClearBuff();
}