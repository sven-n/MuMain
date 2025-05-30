// CameraMove.cpp: implementation of the CCameraMove class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CameraMove.h"
#include "ZzzLodTerrain.h"
#include "ZzzAI.h"

CCameraMove::CCameraMove()
{
    Init();
}
CCameraMove::~CCameraMove()
{
    UnLoadCameraWalkScript();
}

void CCameraMove::Init()
{
    m_CameraStartPos[0] = m_CameraStartPos[1] = m_CameraStartPos[2] = 0.0f;
    m_fCameraStartDistanceLevel = 10.f;
    m_iDelayCount = 0;
    m_dwCameraWalkState = CAMERAWALK_STATE_READY;

    m_CurrentCameraPos[0] = m_CurrentCameraPos[1] = m_CurrentCameraPos[2] = 0.0f;
    m_fCurrentDistanceLevel = 0.f;

    m_dwCurrentIndex = 0;
    m_iSelectedTile = -1;

    m_bTourMode = FALSE;
    m_bTourPause = FALSE;
    m_fForceSpeed = 0;
    m_vTourCameraPos[0] = m_vTourCameraPos[1] = m_vTourCameraPos[2] = 0.0f;
    m_fTourCameraAngle = 0;
    m_fTargetTourCameraAngle = 0;

    m_fCameraAngle = 0;
    m_fFrustumAngle = 0;
}

bool CCameraMove::LoadCameraWalkScript(const std::wstring& filename)
{
    UnLoadCameraWalkScript();

    FILE* fp = _wfopen(filename.c_str(), L"rb");
    if (fp == NULL)
        return false;

    DWORD dwSign;
    fread(&dwSign, 4, 1, fp);
    if (dwSign != 0x00535743)
        return false;

    size_t size;
    fread(&size, sizeof(size_t), 1, fp);

    for (int i = 0; i < (int)size; i++)
    {
        auto* pWayPoint = new WAYPOINT;
        fread(pWayPoint, sizeof(WAYPOINT), 1, fp);
        m_listWayPoint.push_back(pWayPoint);
    }
    fclose(fp);

    return true;
}
void CCameraMove::UnLoadCameraWalkScript()
{
    auto iter = m_listWayPoint.begin();
    for (; iter != m_listWayPoint.end(); iter++)
        delete (*iter);
    m_listWayPoint.clear();

    Init();
}
bool CCameraMove::SaveCameraWalkScript(const std::wstring& filename)
{
    if (!m_listWayPoint.empty()) {
        FILE* fp = _wfopen(filename.c_str(), L"wb");
        if (fp == NULL)
            return false;

        DWORD dwSign = 0x00535743;
        fwrite(&dwSign, 4, 1, fp);
        size_t size = m_listWayPoint.size();
        fwrite(&size, sizeof(size_t), 1, fp);
        auto iter = m_listWayPoint.begin();
        for (; iter != m_listWayPoint.end(); iter++) {
            fwrite((*iter), sizeof(WAYPOINT), 1, fp);
        }
        fclose(fp);
        return true;
    }
    return false;
}

void CCameraMove::AddWayPoint(int iGridX, int iGridY, float fCameraMoveAccel, float fCameraDistanceLevel, int iDelay)
{
    auto* pWayPoint = new WAYPOINT;
    pWayPoint->iIndex = TERRAIN_INDEX_REPEAT(iGridX, iGridY);
    pWayPoint->fCameraX = iGridX * TERRAIN_SCALE;
    pWayPoint->fCameraY = iGridY * TERRAIN_SCALE;
    pWayPoint->fCameraZ = RequestTerrainHeight(pWayPoint->fCameraX, pWayPoint->fCameraY);

    if (fCameraMoveAccel > 40.f)
        fCameraMoveAccel = 40.f;
    if (fCameraMoveAccel < 0.1f)
        fCameraMoveAccel = 0.1f;
    pWayPoint->fCameraMoveAccel = fCameraMoveAccel;

    if (fCameraDistanceLevel > 20.f)
        fCameraDistanceLevel = 20.f;
    if (fCameraDistanceLevel < 5.f)
        fCameraDistanceLevel = 5.f;
    pWayPoint->fCameraDistanceLevel = fCameraDistanceLevel;

    pWayPoint->iDelay = iDelay;

    m_listWayPoint.push_back(pWayPoint);
}
void CCameraMove::RemoveWayPoint(int iGridX, int iGridY)
{
    auto iter = m_listWayPoint.begin();
    for (; iter != m_listWayPoint.end(); iter++) {
        if ((*iter)->iIndex == TERRAIN_INDEX_REPEAT(iGridX, iGridY)) {
            if ((*iter)->iIndex == (int)GetSelectedTile())
                SetSelectedTile(-1);
            m_listWayPoint.erase(iter);
            break;
        }
    }
}

void CCameraMove::SetCameraMoveAccel(int iTileIndex, float fCameraMoveAccel)
{
    auto iter = m_listWayPoint.begin();
    for (; iter != m_listWayPoint.end(); iter++) {
        if ((*iter)->iIndex == iTileIndex) {
            if (fCameraMoveAccel > 40.f)
                fCameraMoveAccel = 40.f;
            if (fCameraMoveAccel < 0.1f)
                fCameraMoveAccel = 0.1f;

            (*iter)->fCameraMoveAccel = fCameraMoveAccel;
        }
    }
}
void CCameraMove::SetCameraDistanceLevel(int iTileIndex, float fCameraDistanceLevel)
{
    auto iter = m_listWayPoint.begin();
    for (; iter != m_listWayPoint.end(); iter++) {
        if ((*iter)->iIndex == iTileIndex) {
            if (fCameraDistanceLevel > 20.f)
                fCameraDistanceLevel = 20.f;
            if (fCameraDistanceLevel < 5.f)
                fCameraDistanceLevel = 5.f;

            (*iter)->fCameraDistanceLevel = fCameraDistanceLevel;
        }
    }
}
void CCameraMove::SetDelay(int iTileIndex, int iDelay)
{
    auto iter = m_listWayPoint.begin();
    for (; iter != m_listWayPoint.end(); iter++) {
        if ((*iter)->iIndex == iTileIndex) {
            if (iDelay < 0)
                iDelay = 0;
            (*iter)->iDelay = iDelay;
        }
    }
}
float CCameraMove::GetCameraMoveAccel(int iTileIndex)
{
    auto iter = m_listWayPoint.begin();
    for (; iter != m_listWayPoint.end(); iter++) {
        if ((*iter)->iIndex == iTileIndex) {
            return (*iter)->fCameraMoveAccel;
        }
    }
    return 0.0f;
}
float CCameraMove::GetCameraDistanceLevel(int iTileIndex)
{
    auto iter = m_listWayPoint.begin();
    for (; iter != m_listWayPoint.end(); iter++) {
        if ((*iter)->iIndex == iTileIndex) {
            return (*iter)->fCameraDistanceLevel;
        }
    }
    return 0.0f;
}
int CCameraMove::GetDelay(int iTileIndex)
{
    auto iter = m_listWayPoint.begin();
    for (; iter != m_listWayPoint.end(); iter++) {
        if ((*iter)->iIndex == iTileIndex) {
            return (*iter)->iDelay;
        }
    }
    return -1;
}

bool CCameraMove::IsCameraMove() const
{
    return (m_dwCameraWalkState == CAMERAWALK_STATE_MOVE);
}

void CCameraMove::UpdateWayPoint()
{
UPDATE_WAY_POINT_ENTRY:
    if (m_dwCameraWalkState == CAMERAWALK_STATE_MOVE && m_listWayPoint.empty() == false) {
        if (m_dwCurrentIndex < m_listWayPoint.size()) {
            WAYPOINT* pTargetWayPoint = m_listWayPoint[m_dwCurrentIndex];
            const float cameraMoveAccel = pTargetWayPoint->fCameraMoveAccel;// todo? *static_cast<float>(FPS_ANIMATION_FACTOR);
            if (m_iDelayCount >= pTargetWayPoint->iDelay)
            {
                float fSubVector[2] = { pTargetWayPoint->fCameraX - m_CurrentCameraPos[0], pTargetWayPoint->fCameraY - m_CurrentCameraPos[1] };
                float fSubDistance = sqrt(fSubVector[0] * fSubVector[0] + fSubVector[1] * fSubVector[1]);
                float fDirVector[2] = { fSubVector[0] / fSubDistance, fSubVector[1] / fSubDistance };
                float fDirDistance = sqrt(fDirVector[0] * fDirVector[0] + fDirVector[1] * fDirVector[1]);

                if (fSubDistance <= fDirDistance * cameraMoveAccel)
                {
                    m_dwCurrentIndex++;
                    m_iDelayCount = 0;
                    goto UPDATE_WAY_POINT_ENTRY;
                }
                else
                {
                    m_CurrentCameraPos[0] += (fDirVector[0] * cameraMoveAccel);
                    m_CurrentCameraPos[1] += (fDirVector[1] * cameraMoveAccel);
                    m_CurrentCameraPos[2] = RequestTerrainHeight(m_CurrentCameraPos[0], m_CurrentCameraPos[1]);

                    if (m_fCurrentDistanceLevel < pTargetWayPoint->fCameraDistanceLevel)
                    {
                        m_fCurrentDistanceLevel += cameraMoveAccel * 0.005f;
                        if (m_fCurrentDistanceLevel > pTargetWayPoint->fCameraDistanceLevel)
                            m_fCurrentDistanceLevel = pTargetWayPoint->fCameraDistanceLevel;
                    }
                    if (m_fCurrentDistanceLevel > pTargetWayPoint->fCameraDistanceLevel)
                    {
                        m_fCurrentDistanceLevel -= cameraMoveAccel * 0.005f;
                        if (m_fCurrentDistanceLevel < pTargetWayPoint->fCameraDistanceLevel)
                            m_fCurrentDistanceLevel = pTargetWayPoint->fCameraDistanceLevel;
                    }
                }
            }

            m_iDelayCount += FPS_ANIMATION_FACTOR;
        }
        else if (m_dwCurrentIndex == m_listWayPoint.size())
        {
            float fSubVector[2] = { m_CameraStartPos[0] - m_CurrentCameraPos[0], m_CameraStartPos[1] - m_CurrentCameraPos[1] };
            float fSubDistance = sqrt(fSubVector[0] * fSubVector[0] + fSubVector[1] * fSubVector[1]);
            float fDirVector[2] = { fSubVector[0] / fSubDistance, fSubVector[1] / fSubDistance };
            float fDirDistance = sqrt(fDirVector[0] * fDirVector[0] + fDirVector[1] * fDirVector[1]);

            if (fSubDistance <= fDirDistance * 10.f)
            {
                m_dwCurrentIndex++;
            }
            else
            {
                m_CurrentCameraPos[0] += (fDirVector[0] * 10.f);
                m_CurrentCameraPos[1] += (fDirVector[1] * 10.f);
                m_CurrentCameraPos[2] = RequestTerrainHeight(m_CurrentCameraPos[0], m_CurrentCameraPos[1]);

                if (fabs(m_fCameraStartDistanceLevel - m_fCurrentDistanceLevel) > 0.2f)
                    m_fCurrentDistanceLevel += (m_fCameraStartDistanceLevel - m_fCurrentDistanceLevel) * 0.2f;
                else
                    m_fCurrentDistanceLevel = m_fCameraStartDistanceLevel;
            }
        }
        else
        {
            m_dwCameraWalkState = CAMERAWALK_STATE_DONE;
            m_iDelayCount = 0;
            m_dwCurrentIndex = 0;
        }
    }
    else
    {
        m_dwCameraWalkState = CAMERAWALK_STATE_DONE;
        m_iDelayCount = 0;
        m_dwCurrentIndex = 0;
    }
}
void CCameraMove::GetCurrentCameraPos(float CameraPos[3])
{
    if (IsTourMode())
    {
        CameraPos[0] = m_vTourCameraPos[0];
        CameraPos[1] = m_vTourCameraPos[1];
        CameraPos[2] = m_vTourCameraPos[2];
    }
    else
    {
        CameraPos[0] = m_CurrentCameraPos[0];
        CameraPos[1] = m_CurrentCameraPos[1];
        CameraPos[2] = m_CurrentCameraPos[2];
    }
}
float CCameraMove::GetCurrentCameraDistanceLevel() const
{
    return m_fCurrentDistanceLevel;
}

void CCameraMove::PlayCameraWalk(float StartPos[3], float fStartDistanceLevel)
{
    if (m_dwCameraWalkState == CAMERAWALK_STATE_READY)
    {
        m_dwCameraWalkState = CAMERAWALK_STATE_MOVE;
        m_CameraStartPos[0] = m_CurrentCameraPos[0] = StartPos[0];
        m_CameraStartPos[1] = m_CurrentCameraPos[1] = StartPos[1];
        m_CameraStartPos[2] = m_CurrentCameraPos[2] = StartPos[2];
        m_fCameraStartDistanceLevel = m_fCurrentDistanceLevel = fStartDistanceLevel;
        m_iDelayCount = 0;
        m_dwCurrentIndex = 0;
    }
}
void CCameraMove::StopCameraWalk(bool bDone)
{
    if (m_dwCameraWalkState == CAMERAWALK_STATE_MOVE)
    {
        if (bDone) {
            m_dwCameraWalkState = CAMERAWALK_STATE_DONE;
        }
        else {
            m_dwCameraWalkState = CAMERAWALK_STATE_READY;
        }
        m_iDelayCount = 0;
        m_dwCurrentIndex = 0;
    }
}
void CCameraMove::UpdateCameraStartPos(float StartPos[3])
{
    m_CameraStartPos[0] = StartPos[0];
    m_CameraStartPos[1] = StartPos[1];
    m_CameraStartPos[2] = StartPos[2];
}

void CCameraMove::SetCameraWalkState(DWORD dwCameraWalkState)
{
    m_dwCameraWalkState = dwCameraWalkState;
}

DWORD CCameraMove::GetCameraWalkState() const
{
    return m_dwCameraWalkState;
}

void CCameraMove::RenderWayPoint()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
    glColor4f(1.0f, 0.0f, 0.0f, 0.8f);
    auto iter = m_listWayPoint.begin();
    for (; iter != m_listWayPoint.end(); iter++) {
        WAYPOINT* pWayPoint = (*iter);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(pWayPoint->fCameraX + 50 - 10, pWayPoint->fCameraY + 50 - 10, pWayPoint->fCameraZ);
        glVertex3f(pWayPoint->fCameraX + 50 + 10, pWayPoint->fCameraY + 50 - 10, pWayPoint->fCameraZ);
        glVertex3f(pWayPoint->fCameraX + 50 + 10, pWayPoint->fCameraY + 50 + 10, pWayPoint->fCameraZ);
        glVertex3f(pWayPoint->fCameraX + 50 - 10, pWayPoint->fCameraY + 50 + 10, pWayPoint->fCameraZ);
    }
    glEnd();

    glBegin(GL_LINE_STRIP);

    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    iter = m_listWayPoint.begin();
    for (; iter != m_listWayPoint.end(); iter++) {
        WAYPOINT* pWayPoint = (*iter);
        glVertex3f(pWayPoint->fCameraX + 50, pWayPoint->fCameraY + 50, pWayPoint->fCameraZ);
    }

    glEnd();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);
}
void CCameraMove::SetSelectedTile(int iTileIndex)
{
    //. find
    auto iter = m_listWayPoint.begin();
    for (; iter != m_listWayPoint.end(); iter++) {
        if ((*iter)->iIndex == iTileIndex) {
            m_iSelectedTile = iTileIndex;
            return;
        }
    }
    m_iSelectedTile = -1;
}
DWORD CCameraMove::GetSelectedTile() const
{
    return m_iSelectedTile;
}

CCameraMove* CCameraMove::GetInstancePtr()
{
    static CCameraMove s_CameraWalkInstance;
    return &s_CameraWalkInstance;
}

BOOL CCameraMove::SetTourMode(BOOL bFlag, BOOL bRandomStart, int _index)
{
    if (m_listWayPoint.size() <= 1) return FALSE;

    m_bTourMode = bFlag;

    if (bFlag == TRUE)
    {
        if (bRandomStart)
        {
            m_dwCurrentIndex = rand() % m_listWayPoint.size();
        }
        else
        {
            m_dwCurrentIndex = _index;
        }

        DWORD dwTargetIndex = (m_dwCurrentIndex < m_listWayPoint.size() ? m_dwCurrentIndex : 0);
        WAYPOINT* pTargetWayPoint = m_listWayPoint[dwTargetIndex];
        DWORD dwStartIndex = (m_dwCurrentIndex > 0 ? m_dwCurrentIndex - 1 : m_listWayPoint.size() - 1);
        WAYPOINT* pStartWayPoint = m_listWayPoint[dwStartIndex];
        m_CameraStartPos[0] = m_CurrentCameraPos[0] = m_vTourCameraPos[0] = pStartWayPoint->fCameraX;
        m_CameraStartPos[1] = m_CurrentCameraPos[1] = m_vTourCameraPos[1] = pStartWayPoint->fCameraY;
        m_CameraStartPos[2] = m_CurrentCameraPos[2] = m_vTourCameraPos[2] = pStartWayPoint->fCameraZ;

        float fSubVector[2] = { pTargetWayPoint->fCameraX - pStartWayPoint->fCameraX,pTargetWayPoint->fCameraY - pStartWayPoint->fCameraY };
        float fSubDistance = sqrt(fSubVector[0] * fSubVector[0] + fSubVector[1] * fSubVector[1]);
        float fDirVector[2] = { fSubVector[0] / fSubDistance, fSubVector[1] / fSubDistance };
        m_fTargetTourCameraAngle = m_fTourCameraAngle = CreateAngle(0, 0, fDirVector[0], -fDirVector[1]);
    }

    return TRUE;
}

void CCameraMove::PauseTour(BOOL bFlag)
{
    m_bTourPause = bFlag;
    m_fForceSpeed = 0.0f;
}

void CCameraMove::ForwardTour(float fSpeed)
{
    m_fForceSpeed = fSpeed;
}

void CCameraMove::BackwardTour(float fSpeed)
{
    m_fForceSpeed = -fSpeed;
}

void CCameraMove::UpdateTourWayPoint()
{
UPDATE_WAY_POINT_ENTRY:
    if (m_dwCameraWalkState == CAMERAWALK_STATE_MOVE && m_listWayPoint.empty() == false)
    {
        DWORD dwTargetIndex = (m_dwCurrentIndex < m_listWayPoint.size() ? m_dwCurrentIndex : 0);
        WAYPOINT* pTargetWayPoint = m_listWayPoint[dwTargetIndex];
        DWORD dwOriginIndex = (m_dwCurrentIndex > 0 ? m_dwCurrentIndex - 1 : m_listWayPoint.size() - 1);
        WAYPOINT* pOriginWayPoint = m_listWayPoint[dwOriginIndex];
        float targetCameraAcc = pTargetWayPoint->fCameraMoveAccel * static_cast<float>(FPS_ANIMATION_FACTOR);
        float originCameraAcc = pOriginWayPoint->fCameraMoveAccel * static_cast<float>(FPS_ANIMATION_FACTOR);

        targetCameraAcc = min(targetCameraAcc, 100);
        originCameraAcc = min(originCameraAcc, 100);
        targetCameraAcc = max(targetCameraAcc, 0.1f);
        originCameraAcc = max(originCameraAcc, 0.1f);

        if (m_iDelayCount >= pTargetWayPoint->iDelay)
        {
            float fSubVector[2] = { pTargetWayPoint->fCameraX - m_CurrentCameraPos[0], pTargetWayPoint->fCameraY - m_CurrentCameraPos[1] };
            float fSubDistance = sqrt(fSubVector[0] * fSubVector[0] + fSubVector[1] * fSubVector[1]);
            float fDirVector[2] = { fSubVector[0] / fSubDistance, fSubVector[1] / fSubDistance };
            float fOrgVector[2] = { pOriginWayPoint->fCameraX - m_CurrentCameraPos[0], pOriginWayPoint->fCameraY - m_CurrentCameraPos[1] };
            float fOrgDistance = sqrt(fOrgVector[0] * fOrgVector[0] + fOrgVector[1] * fOrgVector[1]);
            float fRvsDirVector[2] = { -fOrgVector[0] / fOrgDistance, -fOrgVector[1] / fOrgDistance };

            float fBlendDist = 300.0f;
            float fTourDirVector[2] = { fDirVector[0], fDirVector[1] };
            if (fSubDistance <= fBlendDist)
            {
                DWORD dwNextTargetIndex = (dwTargetIndex + 1 < m_listWayPoint.size() ? dwTargetIndex + 1 : 0);
                WAYPOINT* pNextTargetWayPoint = m_listWayPoint[dwNextTargetIndex];
                float fNextSubVector[2] = { pNextTargetWayPoint->fCameraX - pTargetWayPoint->fCameraX,pNextTargetWayPoint->fCameraY - pTargetWayPoint->fCameraY };
                float fNextSubDistance = sqrt(fNextSubVector[0] * fNextSubVector[0] + fNextSubVector[1] * fNextSubVector[1]);
                float fNextDirVector[2] = { fNextSubVector[0] / fNextSubDistance, fNextSubVector[1] / fNextSubDistance };

                float fBlendRate = fSubDistance / fBlendDist * 0.5f + 0.5f;
                fTourDirVector[0] = fDirVector[0] * fBlendRate + fNextDirVector[0] * (1.0f - fBlendRate);
                fTourDirVector[1] = fDirVector[1] * fBlendRate + fNextDirVector[1] * (1.0f - fBlendRate);
            }
            else if (fOrgDistance <= fBlendDist)
            {
                DWORD dwPrevOriginIndex = (dwOriginIndex > 0 ? dwOriginIndex - 1 : m_listWayPoint.size() - 1);
                WAYPOINT* pPrevOriginWayPoint = m_listWayPoint[dwPrevOriginIndex];
                float fPrevSubVector[2] = { pOriginWayPoint->fCameraX - pPrevOriginWayPoint->fCameraX,pOriginWayPoint->fCameraY - pPrevOriginWayPoint->fCameraY };
                float fPrevSubDistance = sqrt(fPrevSubVector[0] * fPrevSubVector[0] + fPrevSubVector[1] * fPrevSubVector[1]);
                float fPrevDirVector[2] = { fPrevSubVector[0] / fPrevSubDistance, fPrevSubVector[1] / fPrevSubDistance };

                float fBlendRate = fOrgDistance / fBlendDist * 0.5f + 0.5f;
                fTourDirVector[0] = fDirVector[0] * fBlendRate + fPrevDirVector[0] * (1.0f - fBlendRate);
                fTourDirVector[1] = fDirVector[1] * fBlendRate + fPrevDirVector[1] * (1.0f - fBlendRate);
            }

            auto fSpeed = static_cast<float>(FPS_ANIMATION_FACTOR);
            if (m_fForceSpeed > 0)
            {
                fSpeed *= m_fForceSpeed;
            }
            else if (m_fForceSpeed < 0)
            {
                fSpeed *= -m_fForceSpeed;
            }

            if (m_fForceSpeed >= 0 && fSubDistance <= targetCameraAcc * fSpeed)
            {
                if (m_dwCurrentIndex < m_listWayPoint.size())
                    m_dwCurrentIndex++;
                else
                    m_dwCurrentIndex = 0;
                m_iDelayCount = 0;
                goto UPDATE_WAY_POINT_ENTRY;
            }

            if (m_fForceSpeed < 0 && fOrgDistance <= targetCameraAcc * fSpeed)
            {
                if (m_dwCurrentIndex > 0)
                    m_dwCurrentIndex--;
                else
                    m_dwCurrentIndex = m_listWayPoint.size() - 1;
                m_iDelayCount = 0;
                goto UPDATE_WAY_POINT_ENTRY;
            }

            if (fSubDistance > 5.0f && fOrgDistance > 5.0f)
            {
                m_fTargetTourCameraAngle = CreateAngle(0, 0, fTourDirVector[0], -fTourDirVector[1]);
            }
            {
                float fRotDir = 0;
                float fAngleDistance = 0;
                float fAngleTest = absf(m_fTargetTourCameraAngle - m_fTourCameraAngle);
                if (fAngleTest > 180)
                {
                    fRotDir = (m_fTargetTourCameraAngle - m_fTourCameraAngle > 0.f ? -1.0f : 1.0f);
                    fAngleDistance = 360.0f - fAngleTest;
                }
                else
                {
                    fRotDir = (m_fTargetTourCameraAngle - m_fTourCameraAngle > 0.f ? 1.0f : -1.0f);
                    fAngleDistance = fAngleTest;
                }

                const float fMaxRotateSpeed = 1.0;
                float fRotateSpeed = 0;
                if (fAngleDistance > 0)
                {
                    fRotateSpeed = fAngleDistance / 30.0f;
                    //fRotateSpeed *= FPS_ANIMATION_FACTOR;
                }

                if (fRotateSpeed > fMaxRotateSpeed)
                {
                    fRotateSpeed = fMaxRotateSpeed;
                }

                //fRotateSpeed *= fSpeed;

                if (fAngleDistance <= fRotateSpeed)
                {
                    m_fTourCameraAngle = m_fTargetTourCameraAngle;
                }
                else
                {
                    m_fTourCameraAngle += fRotateSpeed * fRotDir;
                }

                if (m_fTourCameraAngle < 0.0f)
                {
                    m_fTourCameraAngle += 360.0f;
                }
                else if (m_fTourCameraAngle > 360.0f)
                {
                    m_fTourCameraAngle -= 360.0f;
                }
            }

            if (IsTourPaused())
            {
                if (m_fForceSpeed > 0)
                {
                    m_CurrentCameraPos[0] += (fDirVector[0] * targetCameraAcc * m_fForceSpeed);
                    m_CurrentCameraPos[1] += (fDirVector[1] * targetCameraAcc * m_fForceSpeed);
                    m_vTourCameraPos[0] += (fTourDirVector[0] * targetCameraAcc * m_fForceSpeed);
                    m_vTourCameraPos[1] += (fTourDirVector[1] * targetCameraAcc * m_fForceSpeed);
                }
                else if (m_fForceSpeed < 0)
                {
                    m_CurrentCameraPos[0] += (fRvsDirVector[0] * originCameraAcc * m_fForceSpeed);
                    m_CurrentCameraPos[1] += (fRvsDirVector[1] * originCameraAcc * m_fForceSpeed);
                    m_vTourCameraPos[0] += (fTourDirVector[0] * originCameraAcc * m_fForceSpeed);
                    m_vTourCameraPos[1] += (fTourDirVector[1] * originCameraAcc * m_fForceSpeed);
                }
            }
            else
            {
                m_CurrentCameraPos[0] += (fDirVector[0] * targetCameraAcc);
                m_CurrentCameraPos[1] += (fDirVector[1] * targetCameraAcc);
                m_vTourCameraPos[0] += (fTourDirVector[0] * targetCameraAcc);
                m_vTourCameraPos[1] += (fTourDirVector[1] * targetCameraAcc);
            }
            m_vTourCameraPos[2] = -300;//RequestTerrainHeight(m_CurrentCameraPos[0],m_CurrentCameraPos[1]);

            m_fCurrentDistanceLevel = pOriginWayPoint->fCameraDistanceLevel * fSubDistance / (fOrgDistance + fSubDistance)
                + pTargetWayPoint->fCameraDistanceLevel * fOrgDistance / (fOrgDistance + fSubDistance);
        }
        m_iDelayCount += FPS_ANIMATION_FACTOR;
    }
    else
    {
        m_dwCameraWalkState = CAMERAWALK_STATE_DONE;
        m_iDelayCount = 0;
        m_dwCurrentIndex = 0;
    }
}

void CCameraMove::SetAngleFrustum(float _Value)
{
    m_fCameraAngle = _Value;
}
void CCameraMove::SetFrustumAngle(float _Value)
{
    m_fFrustumAngle = _Value;
}
float CCameraMove::GetFrustumAngle()
{
    return (m_fCameraAngle - m_fFrustumAngle);
}