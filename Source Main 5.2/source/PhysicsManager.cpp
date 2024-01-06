// PhysicsManager.cpp: implementation of the CPhysicsManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PhysicsManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzCharacter.h"
#include "zzzEffect.h"
#include "MapManager.h"

#define RENDER_CLOTH
#define ADD_COLLISION

#define RATE_SHORT_SHOULDER		( 0.6f)

float CPhysicsVertex::s_Gravity = 9.8f;
float CPhysicsVertex::s_fMass = 0.0025f;
float CPhysicsVertex::s_fInvOfMass = 400.0f;

CPhysicsVertex::CPhysicsVertex()
{
    Clear();
}

CPhysicsVertex::~CPhysicsVertex()
{
}

void CPhysicsVertex::Clear(void)
{
    m_iCountOneTimeMove = 0;
    for (int i = 0; i < 3; ++i)
    {
        m_vForce[i] = m_vVel[i] = m_vPos[i] = m_vOneTimeMove[i] = 0.0f;
    }
    m_byState = PVS_NORMAL;
}

void CPhysicsVertex::Init(float fXPos, float fYPos, float fZPos, BOOL bFixed)
{
    m_vPos[0] = fXPos; m_vPos[1] = fYPos; m_vPos[2] = fZPos;
    if (bFixed)
    {
        m_byState |= PVS_FIXEDPOS;
    }
}

void CPhysicsVertex::UpdateForce(unsigned int iKey, DWORD dwType, float fWind)
{
    if (PVS_FIXEDPOS & m_byState)
    {
        m_vForce[0] = m_vForce[1] = m_vForce[2] = 0.0f;
        return;
    }

    float fGravityRate = 1.0f;

    int iTemp = min(max(0, 5 - iKey), 4);
    auto fRand = (float)(iTemp == 0 ? 0 : iTemp + 2);
#ifndef DISABLE_WIND
    for (int i = 0; i < 3; ++i)
    {
        m_vForce[i] = fRand * CPhysicsManager::s_vWind[i] - m_vVel[i] * 0.01f;
    }
#else
    for (int i = 0; i < 3; ++i)
    {
        m_vForce[i] = -m_vVel[i] * 0.01f;
    }
#endif

    switch (PCT_MASK_ELASTIC & dwType)	// m_dwType
    {
    case PCT_RUBBER:
        m_vForce[2] += fRand * (fWind + 0.1f) * 1.f * FPS_ANIMATION_FACTOR;
        break;
    case PCT_RUBBER2:
        m_vForce[2] += fRand * (fWind) * FPS_ANIMATION_FACTOR;
        break;
    }

    switch (PCT_MASK_ELASTIC_EXT & dwType)
    {
    case PCT_ELASTIC_HALLOWEEN:
        m_vForce[0] += -(fRand * fWind * 0.5f * FPS_ANIMATION_FACTOR);
        m_vForce[2] += fRand * (fWind + 0.1f) * 0.5f * (float)sinf(WorldTime * 0.003f) * 5.0f * FPS_ANIMATION_FACTOR;
        m_vForce[2] -= s_Gravity * fGravityRate * s_fMass * 50.0f * FPS_ANIMATION_FACTOR;
        break;
    case PCT_ELASTIC_RAGE_L:
        m_vForce[0] += -(fRand * fWind * 0.8f * FPS_ANIMATION_FACTOR);
        break;
    case PCT_ELASTIC_RAGE_R:
        m_vForce[0] -= -(fRand * fWind * 0.8f * FPS_ANIMATION_FACTOR);
        break;
    }

    switch (PCT_MASK_WEIGHT & dwType)	// m_dwType
    {
    case PCT_HEAVY:
        m_vForce[2] -= s_Gravity * fGravityRate * s_fMass * 180.0f * FPS_ANIMATION_FACTOR;
        break;
    default:
        m_vForce[2] -= s_Gravity * fGravityRate * s_fMass * 100.0f * FPS_ANIMATION_FACTOR;
        break;
    }
}

void CPhysicsVertex::AddToForce(float fXForce, float fYForce, float fZForce)
{
    m_vForce[0] += fXForce;
    m_vForce[1] += fYForce;
    m_vForce[2] += fZForce;
}

void CPhysicsVertex::Move(float fTime)
{
    if (PVS_FIXEDPOS & m_byState)
    {
        return;
    }

    for (int i = 0; i < 3; ++i)
    {
        m_vVel[i] += m_vForce[i] * s_fInvOfMass * fTime;
        m_vPos[i] += m_vVel[i] * fTime;
    }
}

void CPhysicsVertex::GetPosition(vec3_t* pPos)
{
    for (int i = 0; i < 3; ++i)
    {
        (*pPos)[i] = m_vPos[i];
    }
}

float CPhysicsVertex::GetDistance(CPhysicsVertex* pVertex2, vec3_t* pDistance)
{
    for (int i = 0; i < 3; ++i)
    {
        (*pDistance)[i] = m_vPos[i] - pVertex2->m_vPos[i];
    }

    return (VectorLength((*pDistance)));
}

BOOL CPhysicsVertex::KeepLength(CPhysicsVertex* pVertex2, float* pfLength)
{
    if (PVS_FIXEDPOS & m_byState)
    {
        return (TRUE);
    }

    vec3_t vDistance;
    float fDistance = max(0.001f, GetDistance(pVertex2, &vDistance));

    if (fDistance > pfLength[1] * 20.0f)
    {
        return (FALSE);
    }

    if (fDistance > pfLength[1])
    {
        VectorScale(vDistance, (fDistance - pfLength[1]) / fDistance, vDistance);
        VectorSubtract(m_vPos, vDistance, m_vPos);
    }
    else if (fDistance < pfLength[0])
    {
        VectorScale(vDistance, (fDistance - pfLength[0]) / fDistance, vDistance);
        VectorSubtract(m_vPos, vDistance, m_vPos);
    }

    return (TRUE);
}

void CPhysicsVertex::AddOneTimeMoveToKeepLength(CPhysicsVertex* pVertex2, float fLength)
{
    vec3_t vDistance;
    float fDistance = max(0.001f, GetDistance(pVertex2, &vDistance));
    VectorScale(vDistance, (fDistance - fLength) * 0.5f / fDistance, vDistance);
    VectorSubtract(m_vOneTimeMove, vDistance, m_vOneTimeMove);
    VectorAdd(pVertex2->m_vOneTimeMove, vDistance, pVertex2->m_vOneTimeMove);
    m_iCountOneTimeMove++;
    pVertex2->m_iCountOneTimeMove++;
}

void CPhysicsVertex::DoOneTimeMove(void)
{
    if (PVS_FIXEDPOS & m_byState)
    {
        m_vOneTimeMove[0] = m_vOneTimeMove[1] = m_vOneTimeMove[2] = 0.0f;
        return;
    }

    if (m_iCountOneTimeMove > 0)
    {
        for (int i = 0; i < 3; ++i)
        {
            m_vOneTimeMove[i] /= (float)(m_iCountOneTimeMove);
        }
        VectorAdd(m_vPos, m_vOneTimeMove, m_vPos);
        m_vOneTimeMove[0] = m_vOneTimeMove[1] = m_vOneTimeMove[2] = 0.0f;
        m_iCountOneTimeMove = 0;
    }
}

void CPhysicsVertex::AddOneTimeMove(vec3_t vMove)
{
    VectorAdd(m_vOneTimeMove, vMove, m_vOneTimeMove);
    m_iCountOneTimeMove++;
}

CPhysicsCollision::CPhysicsCollision()
{
    Clear();
}

CPhysicsCollision::~CPhysicsCollision()
{
}

void CPhysicsCollision::Clear(void)
{
    Vector(0.0f, 0.0f, 0.0f, m_vCenterBeforeTransform);
    m_iBone = 0;
    Vector(0.0f, 0.0f, 0.0f, m_vCenter);
}

void CPhysicsCollision::SetPosition(float fXPos, float fYPos, float fZPos)
{
    m_vCenter[0] = fXPos;
    m_vCenter[1] = fYPos;
    m_vCenter[2] = fZPos;
}

void CPhysicsCollision::GetCenter(vec3_t vCenter)
{
    memcpy(vCenter, m_vCenter, sizeof(vec3_t));
}

void CPhysicsCollision::GetCenterBeforeTransform(vec3_t vCenter)
{
    memcpy(vCenter, m_vCenterBeforeTransform, sizeof(vec3_t));
}

void CPhysicsCollision::ProcessCollision(CPhysicsVertex* pVertex)
{
}

CPhysicsColSphere::CPhysicsColSphere()
{
    Clear();
}

CPhysicsColSphere::~CPhysicsColSphere()
{
}

void CPhysicsColSphere::Clear(void)
{
    CPhysicsCollision::Clear();
    m_fRadius = 0.0f;
}

void CPhysicsColSphere::Init(float fXPos, float fYPos, float fZPos, float fRadius, int iBone)
{
    m_vCenterBeforeTransform[0] = fXPos;
    m_vCenterBeforeTransform[1] = fYPos;
    m_vCenterBeforeTransform[2] = fZPos;
    m_fRadius = fRadius;
    m_iBone = iBone;
}

void CPhysicsColSphere::ProcessCollision(CPhysicsVertex* pVertex)
{
    vec3_t vPos;
    pVertex->GetPosition(&vPos);
    VectorSubtract(vPos, m_vCenter, vPos);
    //float fLength = max( 0.001f, VectorLength( vPos));
    float fLength = VectorLength(vPos);
    if (fLength < 0.01f)
    {
        fLength = 0.01f;
        Vector(fLength, 0.0f, 0.0f, vPos);
    }
    if (fLength < m_fRadius)
    {
        VectorScale(vPos, (m_fRadius - fLength) / fLength, vPos);
        pVertex->AddOneTimeMove(vPos);
    }
}

CPhysicsCloth::CPhysicsCloth()
{
    Clear();
}

CPhysicsCloth::~CPhysicsCloth()
{
}

void CPhysicsCloth::Clear(void)
{
    m_oOwner = NULL;
    m_iBone = 0;
    m_iTexFront = m_iTexBack = 0;
    m_dwType = 0;
    m_fxPos = m_fyPos = m_fzPos = 0.0f;
    m_fWidth = m_fHeight = 1.0f;
    m_fUnitWidth = m_fUnitHeight = 1.0f;

    m_iNumHor = 0;
    m_iNumVer = 0;
    m_iNumVertices = 0;
    m_pVertices = NULL;
    m_iNumLink = 0;
    m_pLink = NULL;

    m_byWindMax = 1;
    m_byWindMin = 1;
}

BOOL CPhysicsCloth::Create(OBJECT* o, int iBone, float fxPos, float fyPos, float fzPos, int iNumHor, int iNumVer, float fWidth, float fHeight, int iTexFront, int iTexBack, DWORD dwType)
{
    assert(iNumHor > 1 && iNumVer > 1);

    m_oOwner = o;
    m_iBone = iBone;
    m_iTexFront = iTexFront;
    m_iTexBack = iTexBack;
    m_dwType |= dwType;
    m_fxPos = fxPos;
    m_fyPos = fyPos;
    m_fzPos = fzPos;

    if (m_pVertices)
    {
        delete[] m_pVertices;
        m_pVertices = NULL;
    }

    if (m_pLink)
    {
        delete[] m_pLink;
        m_pLink = NULL;
    }

    m_fWidth = fWidth;
    m_fHeight = fHeight;
    m_iNumHor = iNumHor;
    m_iNumVer = iNumVer;
    m_iNumVertices = m_iNumHor * m_iNumVer;
    m_pVertices = new CPhysicsVertex[m_iNumVertices];

    m_iNumLink = 2 * ((m_iNumHor - 1) * m_iNumVer + m_iNumHor * (m_iNumVer - 1));
    m_pLink = new St_PhysicsLink[m_iNumLink];

    m_fUnitWidth = m_fWidth / (float)(m_iNumHor - 1);
    m_fUnitHeight = m_fHeight / (float)(m_iNumVer - 1);
    int iLink = 0;
    float Matrix[3][4];
    AngleMatrix(m_oOwner->Angle, Matrix);
    if (m_oOwner->BoneTransform)
    {
        for (int i = 0; i < 3; ++i)
        {
            Matrix[i][3] = m_oOwner->BoneTransform[m_iBone][i][3];
        }
    }

    bool bCylinder = false;

    for (int j = 0; j < m_iNumVer; ++j)
    {
        float fWidth = m_fWidth;
        float fUnitWidth = m_fUnitWidth;
        float fUnitHeight = m_fUnitHeight;

        switch (m_dwType & PCT_MASK_SHAPE_EXT)
        {
        case PCT_SHORT_SHOULDER:
            fWidth *= RATE_SHORT_SHOULDER + (1.0f - RATE_SHORT_SHOULDER) * (float)j / (float)(m_iNumVer - 1);
            fUnitWidth = fWidth / (float)(m_iNumHor - 1);
            break;
        case PCT_CYLINDER:
            bCylinder = true;
            break;
        }

        for (int i = 0; i < m_iNumHor; ++i)
        {
            int iVertex = m_iNumHor * j + i;

            vec3_t vPos;
            if (bCylinder)
            {
                float fPosY = sinf((float)i / (float)(m_iNumHor - 1) * Q_PI) * 100.0f;

                vPos[0] = (fUnitWidth * (float)i);
                vPos[1] = fPosY - 30.0f;
                vPos[2] = (fUnitHeight * (float)j);
            }
            else
            {
                vPos[0] = (fUnitWidth * (float)i) - 0.5f * fWidth;
                vPos[1] = 20.0f;
                vPos[2] = -fUnitHeight * (float)j;
            }
            switch (PCT_MASK_SHAPE & m_dwType)
            {
            case PCT_CURVED:
            {
                float fMove = 2.0f * (float)fabs((float)i / (float)(m_iNumHor - 1) - 0.5f);
                vPos[1] -= 10.0f * fMove * fMove;
            }
            break;
            case PCT_STICKED:
                vPos[1] = 0.0f;
                break;
            }
            NotifyVertexPos(iVertex, vPos);

            vec3_t vPos2;
            if (m_oOwner->BoneTransform)
            {
                Models[m_oOwner->Type].TransformPosition(Matrix, vPos, vPos2, true);
            }
            else
            {
                VectorCopy(vPos, vPos2);
            }
            m_pVertices[iVertex].Init(vPos2[0], vPos2[1], vPos2[2]);
        }
    }
    BYTE byVerLinkStyle = 0;
    BYTE byCrossLinkStyle = 0;
    switch (PCT_MASK_ELASTIC & m_dwType)
    {
    case PCT_RUBBER:
        break;
    default:
        byVerLinkStyle |= PLS_STRICTDISTANCE;
        byCrossLinkStyle |= PLS_LOOSEDISTANCE;
        break;
    }

    for (int j = 0; j < m_iNumVer; ++j)
    {
        for (int i = 0; i < m_iNumHor; ++i)
        {
            vec3_t vTemp;
            int iVertex = m_iNumHor * j + i;
            if (j < m_iNumVer - 1)
            {
                float fDist = m_pVertices[iVertex].GetDistance(&m_pVertices[iVertex + m_iNumHor], &vTemp);
                SetLink(iLink++, iVertex, iVertex + m_iNumHor, fDist * 0.8f, fDist, PLS_SPRING | byVerLinkStyle);
            }
            if (i < m_iNumHor - 1)
            {
                float fDist = m_pVertices[iVertex].GetDistance(&m_pVertices[iVertex + 1], &vTemp);
                SetLink(iLink++, iVertex, iVertex + 1, fDist * 0.8f, fDist, PLS_SPRING | PLS_LOOSEDISTANCE);

                if (j < m_iNumVer - 1)
                {
                    float fDist = m_pVertices[iVertex].GetDistance(&m_pVertices[iVertex + 1 + m_iNumHor], &vTemp);
                    SetLink(iLink++, iVertex, iVertex + 1 + m_iNumHor, fDist * 0.8f, fDist, byCrossLinkStyle);
                }
                if (j > 1)
                {
                    float fDist = m_pVertices[iVertex].GetDistance(&m_pVertices[iVertex + 1 - m_iNumHor], &vTemp);
                    SetLink(iLink++, iVertex, iVertex + 1 - m_iNumHor, fDist * 0.8f, fDist, byCrossLinkStyle);
                }
            }
        }
    }
    m_iNumLink = iLink;
    SetFixedVertices(Matrix);

    return (TRUE);
}

void CPhysicsCloth::Destroy(void)
{
#ifdef ADD_COLLISION
    CNode<CPhysicsCollision*>* pHead = m_lstCollision.FindHead();
    for (; pHead; pHead = m_lstCollision.GetNext(pHead))
    {
        delete pHead->GetData();
    }
    m_lstCollision.RemoveAll();
#endif

    delete[] m_pLink;
    delete[] m_pVertices;
    Clear();
}

void CPhysicsCloth::SetFixedVertices(float Matrix[3][4])
{
    bool bCylinder = false;
    for (int iVertex = 0; iVertex < m_iNumHor; ++iVertex)
    {
        float fWidth = m_fWidth;
        float fUnitWidth = m_fUnitWidth;
        switch (PCT_MASK_SHAPE_EXT & m_dwType)
        {
        case PCT_SHORT_SHOULDER:
            fWidth *= RATE_SHORT_SHOULDER;
            fUnitWidth = fWidth / (float)(m_iNumHor - 1);
            break;
        case PCT_CYLINDER:
            bCylinder = true;
            break;
        }

        vec3_t vPos = { (fUnitWidth * (float)iVertex) - 0.5f * fWidth, 10.0f, 0.0f };

        vPos[0] += m_fxPos;
        if (bCylinder)
        {
            float fPosY = sinf((float)iVertex / (float)(m_iNumHor - 1) * Q_PI) * 130.0f;
            vPos[1] = fPosY - 110.0f;
        }
        else
        {
            vPos[1] = m_fyPos;
        }
        vPos[2] = m_fzPos;

        switch (PCT_MASK_SHAPE_EXT2 & m_dwType)
        {
        case PCT_SHAPE_HALLOWEEN:
            vPos[0] += 5.f;
            vPos[1] += 10.f;
            break;
        }

        switch (PCT_MASK_SHAPE & m_dwType)
        {
        case PCT_CURVED:
            float fMove = 2.0f * (float)fabs((float)iVertex / (float)(m_iNumHor - 1) - 0.5f);
            vPos[1] -= 10.0f * fMove * fMove;
            break;
        }

        vec3_t vTemp;
        memcpy(vTemp, vPos, sizeof(vec3_t));
        vPos[0] = vTemp[2];
        vPos[1] = -vTemp[1];
        vPos[2] = vTemp[0];

        vec3_t vPos2;
        Models[m_oOwner->Type].TransformPosition(Matrix, vPos, vPos2, true);
        m_pVertices[iVertex].Init(vPos2[0], vPos2[1], vPos2[2], TRUE);
    }
}

void CPhysicsCloth::SetLink(int iLink, int iVertex1, int iVertex2, float fDistanceSmall, float fDistanceLarge, BYTE byStyle)
{
    auto& link = m_pLink[iLink];
    link.m_nVertices[0] = iVertex1;
    link.m_nVertices[1] = iVertex2;
    link.m_fDistance[0] = fDistanceSmall;
    link.m_fDistance[1] = fDistanceLarge;
    link.m_byStyle = byStyle;
}

BOOL CPhysicsCloth::Move2(float fTime, int iCount)
{
    for (int i = 0; i < iCount; ++i)
    {
        if (!Move(fTime))
        {
            return (FALSE);
        }
    }

    return (TRUE);
}

BOOL CPhysicsCloth::Move(float fTime)
{
    switch (PCT_MASK_ELASTIC & m_dwType)
    {
    case PCT_RUBBER2:
        m_fWind = ((rand() % m_byWindMax + m_byWindMin) / 100.f);
        break;
    default:
        if (gMapManager.WorldActive == 55)
            m_fWind = (float)(rand() % 40 + 10) / 50.0f;
        else
            m_fWind = CPhysicsManager::s_fWind;
        break;
    }
    switch (PCT_MASK_SHAPE_EXT & m_dwType)
    {
    case PCT_CYLINDER:
        m_fWind = (float)(rand() % 10 + 25) / 50.0f;
        break;
    }

    if (m_oOwner == NULL)
        return (FALSE);

    m_fWind *= FPS_ANIMATION_FACTOR;

    CPhysicsManager::s_vWind[0] = m_fWind * sinf((180.0f + m_oOwner->Angle[2]) * Q_PI / 180.0f);
    CPhysicsManager::s_vWind[1] = -m_fWind * cosf((180.0f + m_oOwner->Angle[2]) * Q_PI / 180.0f);

    MoveVertices(fTime);

    if (!PreventFromStretching())
    {
        return (FALSE);
    }

    return (TRUE);
}

void CPhysicsCloth::GetPosition(int index, vec3_t* pPos)
{
    m_pVertices[index].GetPosition(pPos);
}

#ifdef _DEBUG
#include "ZzzCharacter.h"
#include "ZzzEffect.h"
#endif

void CPhysicsCloth::InitForces(void)
{
    const int iSeed = static_cast<int>(WorldTime / 400.f) * 101 % m_iNumVertices;

    for (int iVertex = 0; iVertex < m_iNumVertices; ++iVertex)
    {
        m_pVertices[iVertex].UpdateForce(abs(iSeed % m_iNumHor - iVertex % m_iNumHor) + abs(iSeed / m_iNumHor - iVertex / m_iNumHor), m_dwType, m_fWind);
    }
}

void CPhysicsCloth::MoveVertices(float fTime)
{
    InitForces();

    for (int iLink = 0; iLink < m_iNumLink; ++iLink)
    {
        St_PhysicsLink* pLink = &m_pLink[iLink];
        if (pLink->m_byStyle & PLS_SPRING)
        {
            CPhysicsVertex* pVertex1 = &m_pVertices[pLink->m_nVertices[0]];
            CPhysicsVertex* pVertex2 = &m_pVertices[pLink->m_nVertices[1]];
            vec3_t vDistance;
            float fDistance = max(0.001f, pVertex1->GetDistance(pVertex2, &vDistance));
            if (fDistance > pLink->m_fDistance[1] + 0.01f)
            {
                vec3_t vForce;
                for (int i = 0; i < 3; ++i)
                {
                    vForce[i] = (fDistance - pLink->m_fDistance[1]) * vDistance[i] / fDistance;
                    vForce[i] *= FPS_ANIMATION_FACTOR;
                    if (PCT_OPT_CORRECTEDFORCE & m_dwType)
                    {
                        vForce[i] *= (pLink->m_fDistance[1] / 32.0f);
                    }
                    switch (PCT_MASK_ELASTIC & m_dwType)
                    {
                    case PCT_RUBBER:
                        vForce[i] *= 3.0f;
                        break;
                    }

                    switch (PCT_MASK_ELASTIC_EXT & m_dwType)
                    {
                    case PCT_ELASTIC_HALLOWEEN:
                        vForce[i] *= 2.0f;
                        break;
                    case PCT_ELASTIC_RAGE_L:
                    case PCT_ELASTIC_RAGE_R:
                    {
                        vForce[i] *= 1.0f;
                    }
                    break;
                    }
                }
                pVertex1->AddToForce(-vForce[0], -vForce[1], -vForce[2]);
                pVertex2->AddToForce(vForce[0], vForce[1], vForce[2]);
            }
        }
    }

    SetFixedVertices(m_oOwner->BoneTransform[m_iBone]);
    for (int iVertex = 0; iVertex < m_iNumVertices; ++iVertex)
    {
        m_pVertices[iVertex].Move(fTime);
    }
}

BOOL CPhysicsCloth::PreventFromStretching(void)
{
#ifdef ADD_COLLISION
    CNode<CPhysicsCollision*>* pHead = m_lstCollision.FindHead();
    for (; pHead; pHead = m_lstCollision.GetNext(pHead))
    {
        CPhysicsCollision* pCollision = pHead->GetData();

        vec3_t vPos;
        pCollision->GetCenterBeforeTransform(vPos);
        {
            vec3_t vTemp;
            memcpy(vTemp, vPos, sizeof(vec3_t));
            vPos[0] = vTemp[2];
            vPos[1] = -vTemp[1];
            vPos[2] = vTemp[0];
        }
        vec3_t vPos2;
        Models[m_oOwner->Type].TransformPosition(m_oOwner->BoneTransform[pCollision->GetBone()], vPos, vPos2, true);
        pCollision->SetPosition(vPos2[0], vPos2[1], vPos2[2]);
    }
#endif
    ProcessCollision();

    for (int iLink = 0; iLink < m_iNumLink; ++iLink)
    {
        St_PhysicsLink* pLink = &m_pLink[iLink];
        if (pLink->m_byStyle & PLS_LOOSEDISTANCE)
        {
            CPhysicsVertex* pVertex1 = &m_pVertices[pLink->m_nVertices[0]];
            CPhysicsVertex* pVertex2 = &m_pVertices[pLink->m_nVertices[1]];

            pVertex1->AddOneTimeMoveToKeepLength(pVertex2, pLink->m_fDistance[1]);
        }
    }
    for (int j = 0; j < m_iNumVer; ++j)
    {
        for (int i = 0; i < m_iNumHor; ++i)
        {
            int iVertex = m_iNumHor * j + i;
            m_pVertices[iVertex].DoOneTimeMove();
        }
    }

    for (int iLink = 0; iLink < m_iNumLink; ++iLink)
    {
        St_PhysicsLink* pLink = &m_pLink[iLink];
        if (pLink->m_nVertices[1] >= m_iNumHor &&
            (pLink->m_byStyle & PLS_STRICTDISTANCE))
        {
            CPhysicsVertex* pVertex1 = &m_pVertices[pLink->m_nVertices[0]];
            CPhysicsVertex* pVertex2 = &m_pVertices[pLink->m_nVertices[1]];

            if (!pVertex2->KeepLength(pVertex1, pLink->m_fDistance))
            {
                return (FALSE);
            }
        }
    }

    return (TRUE);
}

void CPhysicsCloth::Render(vec3_t* pvColor, int iLevel)
{
    auto* pvRenderPos = new vec3_t[m_iNumVertices];

    for (int j = 0; j < m_iNumVer; ++j)
    {
        for (int i = 0; i < m_iNumHor; ++i)
        {
            int iVertex = m_iNumHor * j + i;
            vec3_t* pvPos = &pvRenderPos[iVertex];
            m_pVertices[iVertex].GetPosition(pvPos);
        }
    }

    switch (PCT_MASK_DRAW & m_dwType)
    {
    case PCT_MASK_BLIT:
        DisableAlphaBlend();
        break;
    case PCT_MASK_ALPHA:
        EnableAlphaTest();
        break;
    case PCT_MASK_BLEND:
        EnableAlphaBlend();
        break;
    }

    if (pvColor)
    {
        glColor3fv(*pvColor);
    }
    else
    {
        glColor3f(1.f, 1.f, 1.f);
    }

    if (PCT_MASK_LIGHT & m_dwType)
    {
        float  Lum = sinf(WorldTime * 0.001f) * 0.1f + 0.4f;
        float  Lum2;
        vec3_t Light;
        int    iOffset = 0;
        float  fScale = 0.f;
        for (int i = 0; i < m_iNumHor; i++)
        {
            for (int j = 0; j < m_iNumVer; j++)
            {
                fScale = i * 0.2f + 0.4f;
                Lum2 = Lum * (m_iNumHor - i) / 5.f;

                Vector(Lum2, Lum2, Lum2, Light);
                CreateSprite(BITMAP_LIGHT, pvRenderPos[iOffset], fScale, Light, NULL);
                iOffset++;
            }
        }
        glColor3f(Lum, Lum, Lum);
        EnableAlphaBlend();
    }
#ifdef RENDER_CLOTH
    {
        RenderFace(TRUE, m_iTexFront, pvRenderPos);
        if ((PCT_MASK_DRAW & m_dwType) != PCT_MASK_BLEND || !(PCT_MASK_LIGHT & m_dwType))
        {
            RenderFace(FALSE, m_iTexBack, pvRenderPos);
        }
    }
#endif

    delete[] pvRenderPos;
}

void CPhysicsCloth::RenderFace(BOOL bFront, int iTexture, vec3_t* pvRenderPos)
{
    BindTexture(iTexture);	//BITMAP_ROBE

    glBegin(GL_QUADS);

    if (bFront)
    {
        for (int j = 0; j < m_iNumVer - 1; ++j)
        {
            for (int i = 0; i < m_iNumHor - 1; ++i)
            {
                RenderVertex(pvRenderPos, i, j);
                RenderVertex(pvRenderPos, i + 1, j);
                RenderVertex(pvRenderPos, i + 1, j + 1);
                RenderVertex(pvRenderPos, i, j + 1);
            }
        }
    }
    else
    {
        for (int j = 0; j < m_iNumVer - 1; ++j)
        {
            for (int i = 0; i < m_iNumHor - 1; ++i)
            {
                RenderVertex(pvRenderPos, i, j);
                RenderVertex(pvRenderPos, i, j + 1);
                RenderVertex(pvRenderPos, i + 1, j + 1);
                RenderVertex(pvRenderPos, i + 1, j);
            }
        }
    }

    glEnd();
}

void CPhysicsCloth::RenderVertex(vec3_t* pvRenderPos, int xVertex, int yVertex)
{
    int iVertex = m_iNumHor * yVertex + xVertex;
    vec3_t* pvPos = &pvRenderPos[iVertex];
    glTexCoord2f((float)xVertex / (float)(m_iNumHor - 1), min(0.99f, (float)yVertex / (float)(m_iNumVer - 1)));
    glVertex3f((*pvPos)[0], (*pvPos)[1], (*pvPos)[2]);
}

void CPhysicsCloth::RenderCollisions(void)
{
#ifdef RENDER_COLLISION
    glColor3f(1.0f, 1.0f, 0.6f);
    BindTexture(BITMAP_CLOUD);
    CNode<CPhysicsCollision*>* pHead = m_lstCollision.FindHead();
    for (; pHead; pHead = m_lstCollision.GetNext(pHead))
    {
        CPhysicsCollision* pCol = pHead->GetData();
        if (CLT_SPHERE == pCol->GetType())
        {
            CPhysicsColSphere* pColSph = (CPhysicsColSphere*)pCol;

            static GLUquadricObj* pQuad = NULL;
            if (NULL == pQuad)
            {
                pQuad = gluNewQuadric();
            }
            glPushMatrix();
            vec3_t vCenter;
            pColSph->GetCenter(vCenter);
            glTranslatef(vCenter[0], vCenter[1], vCenter[2]);
            gluSphere(pQuad, pColSph->GetRadius() - 2.0f, 20, 20);
            glPopMatrix();
        }
    }
#endif
}

void CPhysicsCloth::AddCollisionSphere(float fXPos, float fYPos, float fZPos, float fRadius, int iBone)
{
#ifdef ADD_COLLISION
    auto* pCol = new CPhysicsColSphere;
    pCol->Init(fXPos, fYPos, fZPos, fRadius, iBone);
    m_lstCollision.AddTail(pCol);
#endif
}

void CPhysicsCloth::ProcessCollision(void)
{
#ifdef ADD_COLLISION
    if (m_lstCollision.GetCount() > 0)
    {
        CNode<CPhysicsCollision*>* pHead = m_lstCollision.FindHead();
        for (; pHead; pHead = m_lstCollision.GetNext(pHead))
        {
            CPhysicsCollision* pCollision = pHead->GetData();
            for (int i = 0; i < m_iNumVertices; ++i)
            {
                pCollision->ProcessCollision(&m_pVertices[i]);
            }
        }

        for (int i = 0; i < m_iNumVertices; ++i)
        {
            m_pVertices[i].DoOneTimeMove();
        }
    }
#endif
}

CPhysicsClothMesh::CPhysicsClothMesh()
{
}

CPhysicsClothMesh::~CPhysicsClothMesh()
{
    Clear();
}

void CPhysicsClothMesh::Clear(void)
{
    CPhysicsCloth::Destroy();
}

extern float BoneScale;

BOOL CPhysicsClothMesh::Create(OBJECT* o, int iMesh, int iBone, DWORD dwType, int iBMDType)
{
    m_oOwner = o;
    m_iMesh = iMesh;
    m_iBone = iBone;
    m_dwType |= dwType;

    m_iBMDType = (iBMDType == -1) ? m_oOwner->Type : iBMDType;
    BMD* b = &Models[m_iBMDType];
    assert(iMesh < b->NumMeshs);
    Mesh_t* pMesh = &b->Meshs[m_iMesh];

    if (m_pVertices)
    {
        delete[] m_pVertices;
        m_pVertices = NULL;
    }

    if (m_pLink)
    {
        delete[] m_pLink;
        m_pLink = NULL;
    }

    m_iNumVertices = pMesh->NumVertices;
    m_pVertices = new CPhysicsVertex[m_iNumVertices];

    m_iNumLink = pMesh->NumTriangles * 3 * 2;
    m_pLink = new St_PhysicsLink[m_iNumLink];

    float(*BoneMatrix)[3][4] = m_oOwner->BoneTransform;

    for (int iVertex = 0; iVertex < m_iNumVertices; ++iVertex)
    {
        Vertex_t* v = &pMesh->Vertices[iVertex];
        float* vPos = VertexTransform[m_iMesh][iVertex];
        m_pVertices[iVertex].Init(vPos[0], vPos[1], vPos[2], (v->Node == m_iBone));
    }

    int iLink = 0;
    vec3_t vTemp;
    for (int iTri = 0; iTri < pMesh->NumTriangles; iTri++)
    {
        Triangle_t* tp = &pMesh->Triangles[iTri];
        for (int i = 0; i < 3; ++i)
        {
            int iV1 = tp->VertexIndex[i];
            int iV2 = tp->VertexIndex[(i + 1) % 3];
            float fDist = m_pVertices[iV1].GetDistance(&m_pVertices[iV2], &vTemp);

            BYTE byLinkType = PLS_STRICTDISTANCE;

            vec3_t vPos1, vPos2;
            m_pVertices[iV1].GetPosition(&vPos1);
            m_pVertices[iV2].GetPosition(&vPos2);
            if (fabs(vPos1[0] - vPos2[0]) > 10.0f)
            {
                byLinkType = PLS_LOOSEDISTANCE;
            }

            SetLink(iLink++, iV1, iV2, fDist * 0.5f, fDist, PLS_SPRING | byLinkType);

            int iV3 = tp->VertexIndex[(i + 2) % 3];
            int iMatch = FindMatchVertex(pMesh, iV1, iV2, iV3);
            if (iMatch > 0)
            {
                float fDist2 = m_pVertices[iV3].GetDistance(&m_pVertices[iMatch], &vTemp);
                if (fDist2 < fDist * 1.2f && !FindInLink(iLink, iV3, iMatch))
                {
                    SetLink(iLink++, iV3, iMatch, fDist2 * 0.5f, fDist2, PLS_SPRING | byLinkType);
                }
            }
        }
    }
    m_iNumLink = iLink;

    return (TRUE);
}

BOOL CPhysicsClothMesh::Create(OBJECT* o, int iMesh, int iBone, float fxPos, float fyPos, float fzPos, int iNumHor, int iNumVer, float fWidth, float fHeight, int iTexFront, int TexBack, DWORD dwType, int iBMDType)
{
    m_iBMDType = (iBMDType == -1) ? m_oOwner->Type : iBMDType;

    m_iMesh = iMesh;

    m_dwType |= PCT_OPT_MESHPROG;
    if (!CPhysicsCloth::Create(o, iBone, fxPos, fyPos, fzPos, iNumHor, iNumVer, fWidth, fHeight, iTexFront, TexBack, dwType))
    {
        return (FALSE);
    }

    BMD* b = &Models[m_iBMDType];
    assert(iMesh < b->NumMeshs);
    Mesh_t* pMesh = &b->Meshs[m_iMesh];

    for (int j = 0; j < pMesh->NumTriangles; j++)
    {
        int iRight = j % 2;
        int xQuad = (j / 2) % (m_iNumHor - 1);
        int yQuad = (j / 2) / (m_iNumHor - 1);

        Triangle_t* tp = &pMesh->Triangles[j];
        int x;
        int y;
        int xDim[6] = { 0, 1, 0, 1, 1, 0 };
        int yDim[6] = { 0, 0, 1, 0, 1, 1 };
        for (int i = 0; i < 3; ++i)
        {
            x = xDim[iRight * 3 + i] + xQuad;
            y = yDim[iRight * 3 + i] + yQuad;

            // triangle
            tp->VertexIndex[i] = y * m_iNumHor + x;
            // normal
            //int iValue = y * pMesh->NumNormals / m_iNumVer + abs( x - 3);
            //tp->NormalIndex[i] = iValue % pMesh->NumNormals;
            //tp->NormalIndex[i] = abs( iValue % ( ( pMesh->NumNormals - 1) * 2) - ( pMesh->NumNormals - 1));
            //tp->NormalIndex[i] = ( y * 13 + x * 17) % pMesh->NumNormals;
            tp->NormalIndex[i] = (x * pMesh->NumNormals / m_iNumHor + y * pMesh->NumNormals / m_iNumVer) / 3;
            //tp->NormalIndex[i] = j;
            // mapping
            tp->TexCoordIndex[i] = tp->VertexIndex[i];

            TexCoord_t* texp = &pMesh->TexCoords[tp->TexCoordIndex[i]];
            texp->TexCoordU = (float)x / (float)(m_iNumHor - 1);
            texp->TexCoordV = (float)y / (float)(m_iNumVer - 1);
        }
    }

    return (TRUE);
}

int CPhysicsClothMesh::FindMatchVertex(Mesh_t* pMesh, int iV1, int iV2, int iV3)
{
    for (int iTri = 0; iTri < pMesh->NumTriangles; iTri++)
    {
        Triangle_t* tp = &pMesh->Triangles[iTri];
        for (int i = 0; i < 3; ++i)
        {
            int iVcompare1 = tp->VertexIndex[i];
            int iVcompare2 = tp->VertexIndex[(i + 1) % 3];
            int iVcompare3 = tp->VertexIndex[(i + 2) % 3];
            if (iV1 == iVcompare2 && iV2 == iVcompare1 && iV3 != iVcompare3)
            {
                return (iVcompare3);
            }
        }
    }

    return (-1);
}

BOOL CPhysicsClothMesh::FindInLink(int iCount, int iV1, int iV2)
{
    for (int iLink = 0; iLink < iCount; ++iLink)
    {
        if (m_pLink[iLink].m_nVertices[0] == iV1 && m_pLink[iLink].m_nVertices[1] == iV2)
        {
            return (TRUE);
        }
    }

    return (FALSE);
}

void CPhysicsClothMesh::SetFixedVertices(float Matrix[3][4])
{
    if (m_dwType & PCT_OPT_MESHPROG)
    {
        CPhysicsCloth::SetFixedVertices(Matrix);
        return;
    }

    BMD* b = &Models[m_iBMDType];
    Mesh_t* pMesh = &b->Meshs[m_iMesh];
    for (int iVertex = 0; iVertex < m_iNumVertices; ++iVertex)
    {
        Vertex_t* v = &pMesh->Vertices[iVertex];
        if (v->Node == m_iBone)
        {
            float* vPos = VertexTransform[m_iMesh][iVertex];
            m_pVertices[iVertex].Init(vPos[0], vPos[1], vPos[2], TRUE);
        }
    }
}

void CPhysicsClothMesh::NotifyVertexPos(int iVertex, vec3_t vPos)
{
    if (m_dwType & PCT_OPT_MESHPROG)
    {
        BMD* b = &Models[m_iBMDType];
        Mesh_t* pMesh = &b->Meshs[m_iMesh];

        Vertex_t* v = &pMesh->Vertices[iVertex];
        v->Position[0] = vPos[2];
        v->Position[1] = -vPos[1];
        v->Position[2] = vPos[0];

        if (v->Node != m_iBone)
        {
            //Bone_t *pBone = &b->Bones[v->Node];
            //memset( pBone->BoneMatrixes->Quaternion, 0, sizeof ( vec4_t));
            v->Node = m_iBone;
        }
    }
}

void CPhysicsClothMesh::InitForces(void)
{
    if (m_dwType & PCT_OPT_MESHPROG)
    {
        CPhysicsCloth::InitForces();
        return;
    }

    const int iSeed = static_cast<int>(WorldTime / 400.f) * 101 % m_iNumVertices;

    for (int iVertex = 0; iVertex < m_iNumVertices; ++iVertex)
    {
        m_pVertices[iVertex].UpdateForce(abs(iSeed % 10), m_dwType, m_fWind);
    }
}

void CPhysicsClothMesh::Render(vec3_t* pvColor, int iLevel)
{
    vec3_t vPos;
    for (int iVertex = 0; iVertex < m_iNumVertices; ++iVertex)
    {
        m_pVertices[iVertex].GetPosition(&vPos);
        VectorCopy(vPos, VertexTransform[m_iMesh][iVertex]);
    }
}

float CPhysicsManager::s_fWind = 0.0f;
vec3_t CPhysicsManager::s_vWind = { 0.0f, 0.0f, 0.0f };

CPhysicsManager::CPhysicsManager()
{
    Clear();
}

CPhysicsManager::~CPhysicsManager()
{
    RemoveAll();
}

void CPhysicsManager::Clear(void)
{
}

void CPhysicsManager::Move(float fTime)
{
    float fPlus = ((rand() % 200) - 100) * 0.001f;
    s_fWind += fPlus * FPS_ANIMATION_FACTOR;
    s_fWind = std::clamp(s_fWind, -0.2f, 1.0f);

    CNode<CPhysicsCloth*>* pNode = m_lstCloth.FindHead();
    for (; pNode; pNode = m_lstCloth.GetNext(pNode))
    {
        pNode->GetData()->Move(fTime);
    }
}

void CPhysicsManager::Render(void)
{
    CNode<CPhysicsCloth*>* pNode = m_lstCloth.FindHead();
    for (; pNode; pNode = m_lstCloth.GetNext(pNode))
    {
        pNode->GetData()->Render();
    }
}

void CPhysicsManager::Add(CPhysicsCloth* pCloth)
{
    m_lstCloth.AddTail(pCloth);
}

void CPhysicsManager::Remove(OBJECT* oOwner)
{
    CNode<CPhysicsCloth*>* pNode = m_lstCloth.FindHead();
    for (; pNode; pNode = m_lstCloth.GetNext(pNode))
    {
        if (oOwner == pNode->GetData()->GetOwner())
        {
            pNode->GetData()->Destroy();
            delete pNode->GetData();
            m_lstCloth.RemoveNode(pNode);
            break;
        }
    }
}

void CPhysicsManager::RemoveAll(void)
{
    CNode<CPhysicsCloth*>* pNode = m_lstCloth.FindHead();
    for (; pNode; pNode = m_lstCloth.GetNext(pNode))
    {
        pNode->GetData()->Destroy();
        delete pNode->GetData();
    }
    m_lstCloth.RemoveAll();
}