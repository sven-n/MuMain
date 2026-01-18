// CameraMove.cpp: implementation of the CCameraMove class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>

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

namespace
{
    constexpr float kDefaultStartDistance = 10.0f;
    constexpr float kMinMoveAccel = 0.1f;
    constexpr float kMaxMoveAccel = 40.0f;
    constexpr float kMinDistanceLevel = 5.0f;
    constexpr float kMaxDistanceLevel = 20.0f;
    constexpr float kReturnSpeed = 10.0f;
    constexpr float kDistanceAdjustFactor = 0.005f;
    constexpr float kDistanceSnapEpsilon = 0.2f;
    constexpr float kTourBlendDistance = 300.0f;
    constexpr float kTourMaxRotateSpeed = 1.0f;
    constexpr float kMinTourAccel = 0.1f;
    constexpr float kMaxTourAccel = 100.0f;
    constexpr float kFullCircleDegrees = 360.0f;
    constexpr float kWaypointRenderOffset = 50.0f;
    constexpr float kWaypointRenderHalfSize = 10.0f;
    constexpr float kTourCameraZPosition = -300.0f;

    template <typename T>
    constexpr const T& Clamp(const T& value, const T& minValue, const T& maxValue)
    {
        return std::max<T>(minValue, std::min<T>(value, maxValue));
    }

    template <std::size_t Size>
    void CopyArray(const float (&source)[Size], float (&destination)[Size])
    {
        std::copy(source, source + Size, destination);
    }

    float NormalizeAngleDegrees(float angle)
    {
        angle = std::fmod(angle, kFullCircleDegrees);
        if (angle < 0.0f)
        {
            angle += kFullCircleDegrees;
        }
        return angle;
    }

    float SignedAngleDelta(float fromDegrees, float toDegrees)
    {
        const float delta = NormalizeAngleDegrees(toDegrees) - NormalizeAngleDegrees(fromDegrees);
        if (delta > 180.0f)
        {
            return delta - kFullCircleDegrees;
        }
        if (delta < -180.0f)
        {
            return delta + kFullCircleDegrees;
        }
        return delta;
    }

    struct CameraVector2
    {
        float x{0.0f};
        float y{0.0f};

        float Length() const
        {
            return std::sqrt(x * x + y * y);
        }

        CameraVector2 Normalized() const
        {
            const float length = Length();
            if (length <= std::numeric_limits<float>::epsilon())
            {
                return {};
            }
            return {x / length, y / length};
        }

        CameraVector2 operator+(const CameraVector2& other) const
        {
            return {x + other.x, y + other.y};
        }

        CameraVector2 operator-(const CameraVector2& other) const
        {
            return {x - other.x, y - other.y};
        }

        CameraVector2 operator*(float scalar) const
        {
            return {x * scalar, y * scalar};
        }
    };

    CameraVector2 BlendVectors(const CameraVector2& a, const CameraVector2& b, float alpha)
    {
        return {(a.x * (1.0f - alpha)) + (b.x * alpha), (a.y * (1.0f - alpha)) + (b.y * alpha)};
    }
}

void CCameraMove::Init()
{
    m_CameraStartPos[0] = m_CameraStartPos[1] = m_CameraStartPos[2] = 0.0f;
    m_fCameraStartDistanceLevel = kDefaultStartDistance;
    m_iDelayCount = 0;
    m_dwCameraWalkState = CAMERAWALK_STATE_READY;

    m_CurrentCameraPos[0] = m_CurrentCameraPos[1] = m_CurrentCameraPos[2] = 0.0f;
    m_fCurrentDistanceLevel = 0.0f;

    m_dwCurrentIndex = 0;
    m_iSelectedTile = -1;

    m_bTourMode = FALSE;
    m_bTourPause = FALSE;
    m_fForceSpeed = 0.0f;
    m_vTourCameraPos[0] = m_vTourCameraPos[1] = m_vTourCameraPos[2] = 0.0f;
    m_fTourCameraAngle = 0.0f;
    m_fTargetTourCameraAngle = 0.0f;

    m_fCameraAngle = 0.0f;
    m_fFrustumAngle = 0.0f;
}

bool CCameraMove::LoadCameraWalkScript(const std::wstring& filename)
{
    UnLoadCameraWalkScript();

    std::unique_ptr<FILE, decltype(&fclose)> fileHandle(_wfopen(filename.c_str(), L"rb"), &fclose);
    if (!fileHandle)
    {
        return false;
    }

    DWORD signature = 0;
    if (fread(&signature, sizeof(DWORD), 1, fileHandle.get()) != 1 || signature != 0x00535743)
    {
        return false;
    }

    size_t waypointCount = 0;
    if (fread(&waypointCount, sizeof(size_t), 1, fileHandle.get()) != 1)
    {
        return false;
    }

    m_listWayPoint.reserve(waypointCount);
    for (size_t index = 0; index < waypointCount; ++index)
    {
        auto waypoint = std::make_unique<WAYPOINT>();
        if (fread(waypoint.get(), sizeof(WAYPOINT), 1, fileHandle.get()) != 1)
        {
            return false;
        }
        m_listWayPoint.emplace_back(std::move(waypoint));
    }

    return true;
}
void CCameraMove::UnLoadCameraWalkScript()
{
    m_listWayPoint.clear();

    Init();
}
bool CCameraMove::SaveCameraWalkScript(const std::wstring& filename)
{
    if (m_listWayPoint.empty())
    {
        return false;
    }

    std::unique_ptr<FILE, decltype(&fclose)> fileHandle(_wfopen(filename.c_str(), L"wb"), &fclose);
    if (!fileHandle)
    {
        return false;
    }

    const DWORD signature = 0x00535743;
    const size_t waypointCount = m_listWayPoint.size();

    if (fwrite(&signature, sizeof(DWORD), 1, fileHandle.get()) != 1 ||
        fwrite(&waypointCount, sizeof(size_t), 1, fileHandle.get()) != 1)
    {
        return false;
    }

    for (const auto& waypoint : m_listWayPoint)
    {
        if (fwrite(waypoint.get(), sizeof(WAYPOINT), 1, fileHandle.get()) != 1)
        {
            return false;
        }
    }

    return true;
}

void CCameraMove::AddWayPoint(int iGridX, int iGridY, float fCameraMoveAccel, float fCameraDistanceLevel, int iDelay)
{
    auto waypoint = std::make_unique<WAYPOINT>();
    waypoint->iIndex = TERRAIN_INDEX_REPEAT(iGridX, iGridY);
    waypoint->fCameraX = iGridX * TERRAIN_SCALE;
    waypoint->fCameraY = iGridY * TERRAIN_SCALE;
    waypoint->fCameraZ = RequestTerrainHeight(waypoint->fCameraX, waypoint->fCameraY);

    waypoint->fCameraMoveAccel = Clamp(fCameraMoveAccel, kMinMoveAccel, kMaxMoveAccel);
    waypoint->fCameraDistanceLevel = Clamp(fCameraDistanceLevel, kMinDistanceLevel, kMaxDistanceLevel);
    waypoint->iDelay = std::max<int>(iDelay, 0);

    m_listWayPoint.push_back(std::move(waypoint));
}
void CCameraMove::RemoveWayPoint(int iGridX, int iGridY)
{
    const int tileIndex = TERRAIN_INDEX_REPEAT(iGridX, iGridY);
    const auto originalSize = m_listWayPoint.size();
    const bool wasSelectedTile = GetSelectedTile() == static_cast<DWORD>(tileIndex);

    auto removalPredicate = [tileIndex](const std::unique_ptr<WAYPOINT>& waypoint)
    {
        return waypoint->iIndex == tileIndex;
    };

    m_listWayPoint.erase(std::remove_if(m_listWayPoint.begin(), m_listWayPoint.end(), removalPredicate), m_listWayPoint.end());

    if (wasSelectedTile && m_listWayPoint.size() != originalSize)
    {
        SetSelectedTile(-1);
    }
}

void CCameraMove::SetCameraMoveAccel(int iTileIndex, float fCameraMoveAccel)
{
    const float clampedAccel = Clamp(fCameraMoveAccel, kMinMoveAccel, kMaxMoveAccel);
    for (auto& waypoint : m_listWayPoint)
    {
        if (waypoint->iIndex == iTileIndex)
        {
            waypoint->fCameraMoveAccel = clampedAccel;
        }
    }
}
void CCameraMove::SetCameraDistanceLevel(int iTileIndex, float fCameraDistanceLevel)
{
    const float clampedDistance = Clamp(fCameraDistanceLevel, kMinDistanceLevel, kMaxDistanceLevel);
    for (auto& waypoint : m_listWayPoint)
    {
        if (waypoint->iIndex == iTileIndex)
        {
            waypoint->fCameraDistanceLevel = clampedDistance;
        }
    }
}
void CCameraMove::SetDelay(int iTileIndex, int iDelay)
{
    const int clampedDelay = std::max<int>(iDelay, 0);
    for (auto& waypoint : m_listWayPoint)
    {
        if (waypoint->iIndex == iTileIndex)
        {
            waypoint->iDelay = clampedDelay;
        }
    }
}
float CCameraMove::GetCameraMoveAccel(int iTileIndex)
{
    if (const auto* waypoint = FindWayPointByTile(iTileIndex))
    {
        return waypoint->fCameraMoveAccel;
    }
    return 0.0f;
}
float CCameraMove::GetCameraDistanceLevel(int iTileIndex)
{
    if (const auto* waypoint = FindWayPointByTile(iTileIndex))
    {
        return waypoint->fCameraDistanceLevel;
    }
    return 0.0f;
}
int CCameraMove::GetDelay(int iTileIndex)
{
    if (const auto* waypoint = FindWayPointByTile(iTileIndex))
    {
        return waypoint->iDelay;
    }
    return -1;
}

bool CCameraMove::IsCameraMove() const
{
    return (m_dwCameraWalkState == CAMERAWALK_STATE_MOVE);
}

void CCameraMove::UpdateWayPoint()
{
    while (true)
    {
        if (m_dwCameraWalkState != CAMERAWALK_STATE_MOVE || m_listWayPoint.empty())
        {
            m_dwCameraWalkState = CAMERAWALK_STATE_DONE;
            m_iDelayCount = 0;
            m_dwCurrentIndex = 0;
            return;
        }

        if (m_dwCurrentIndex < m_listWayPoint.size())
        {
            const WAYPOINT* targetWaypoint = GetWayPointByIndex(m_dwCurrentIndex);
            if (!targetWaypoint)
            {
                m_dwCameraWalkState = CAMERAWALK_STATE_DONE;
                return;
            }

            const float cameraMoveAccel = targetWaypoint->fCameraMoveAccel;
            if (m_iDelayCount >= targetWaypoint->iDelay)
            {
                CameraVector2 toTarget{targetWaypoint->fCameraX - m_CurrentCameraPos[0], targetWaypoint->fCameraY - m_CurrentCameraPos[1]};
                const float remainingDistance = toTarget.Length();

                if (remainingDistance <= cameraMoveAccel)
                {
                    ++m_dwCurrentIndex;
                    m_iDelayCount = 0;
                    continue;
                }

                if (remainingDistance > std::numeric_limits<float>::epsilon())
                {
                    const CameraVector2 direction = toTarget * (1.0f / remainingDistance);
                    m_CurrentCameraPos[0] += direction.x * cameraMoveAccel;
                    m_CurrentCameraPos[1] += direction.y * cameraMoveAccel;
                }

                m_CurrentCameraPos[2] = RequestTerrainHeight(m_CurrentCameraPos[0], m_CurrentCameraPos[1]);

                const float distanceDelta = cameraMoveAccel * kDistanceAdjustFactor;
                const float targetDistance = targetWaypoint->fCameraDistanceLevel;
                if (std::abs(m_fCurrentDistanceLevel - targetDistance) <= distanceDelta)
                {
                    m_fCurrentDistanceLevel = targetDistance;
                }
                else if (m_fCurrentDistanceLevel < targetDistance)
                {
                    m_fCurrentDistanceLevel += distanceDelta;
                }
                else
                {
                    m_fCurrentDistanceLevel -= distanceDelta;
                }
            }

            m_iDelayCount += FPS_ANIMATION_FACTOR;
            return;
        }

        if (m_dwCurrentIndex == m_listWayPoint.size())
        {
            CameraVector2 toStart{m_CameraStartPos[0] - m_CurrentCameraPos[0], m_CameraStartPos[1] - m_CurrentCameraPos[1]};
            const float remainingDistance = toStart.Length();

            if (remainingDistance <= kReturnSpeed)
            {
                ++m_dwCurrentIndex;
                continue;
            }

            if (remainingDistance > std::numeric_limits<float>::epsilon())
            {
                const CameraVector2 direction = toStart * (1.0f / remainingDistance);
                m_CurrentCameraPos[0] += direction.x * kReturnSpeed;
                m_CurrentCameraPos[1] += direction.y * kReturnSpeed;
            }

            m_CurrentCameraPos[2] = RequestTerrainHeight(m_CurrentCameraPos[0], m_CurrentCameraPos[1]);

            if (std::abs(m_fCameraStartDistanceLevel - m_fCurrentDistanceLevel) > kDistanceSnapEpsilon)
            {
                m_fCurrentDistanceLevel += (m_fCameraStartDistanceLevel - m_fCurrentDistanceLevel) * 0.2f;
            }
            else
            {
                m_fCurrentDistanceLevel = m_fCameraStartDistanceLevel;
            }

            return;
        }

        m_dwCameraWalkState = CAMERAWALK_STATE_DONE;
        m_iDelayCount = 0;
        m_dwCurrentIndex = 0;
        return;
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
    for (const auto& waypoint : m_listWayPoint)
    {
        glNormal3f(0.0f, 0.0f, 1.0f);
        const float minX = waypoint->fCameraX + kWaypointRenderOffset - kWaypointRenderHalfSize;
        const float maxX = waypoint->fCameraX + kWaypointRenderOffset + kWaypointRenderHalfSize;
        const float minY = waypoint->fCameraY + kWaypointRenderOffset - kWaypointRenderHalfSize;
        const float maxY = waypoint->fCameraY + kWaypointRenderOffset + kWaypointRenderHalfSize;

        glVertex3f(minX, minY, waypoint->fCameraZ);
        glVertex3f(maxX, minY, waypoint->fCameraZ);
        glVertex3f(maxX, maxY, waypoint->fCameraZ);
        glVertex3f(minX, maxY, waypoint->fCameraZ);
    }
    glEnd();

    glBegin(GL_LINE_STRIP);

    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    for (const auto& waypoint : m_listWayPoint)
    {
        glVertex3f(waypoint->fCameraX + 50.0f, waypoint->fCameraY + 50.0f, waypoint->fCameraZ);
    }

    glEnd();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);
}
void CCameraMove::SetSelectedTile(int iTileIndex)
{
    m_iSelectedTile = -1;
    for (const auto& waypoint : m_listWayPoint)
    {
        if (waypoint->iIndex == iTileIndex)
        {
            m_iSelectedTile = iTileIndex;
            return;
        }
    }
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

CCameraMove::WAYPOINT* CCameraMove::GetWayPointByIndex(std::size_t index)
{
    if (index >= m_listWayPoint.size())
    {
        return nullptr;
    }
    return m_listWayPoint[index].get();
}

const CCameraMove::WAYPOINT* CCameraMove::GetWayPointByIndex(std::size_t index) const
{
    if (index >= m_listWayPoint.size())
    {
        return nullptr;
    }
    return m_listWayPoint[index].get();
}

CCameraMove::WAYPOINT* CCameraMove::FindWayPointByTile(int tileIndex)
{
    auto iter = std::find_if(
        m_listWayPoint.begin(),
        m_listWayPoint.end(),
        [tileIndex](const std::unique_ptr<WAYPOINT>& waypoint)
        {
            return waypoint->iIndex == tileIndex;
        });

    return (iter != m_listWayPoint.end()) ? iter->get() : nullptr;
}

const CCameraMove::WAYPOINT* CCameraMove::FindWayPointByTile(int tileIndex) const
{
    auto iter = std::find_if(
        m_listWayPoint.begin(),
        m_listWayPoint.end(),
        [tileIndex](const std::unique_ptr<WAYPOINT>& waypoint)
        {
            return waypoint->iIndex == tileIndex;
        });

    return (iter != m_listWayPoint.end()) ? iter->get() : nullptr;
}

BOOL CCameraMove::SetTourMode(BOOL bFlag, BOOL bRandomStart, int index)
{
    const std::size_t waypointCount = m_listWayPoint.size();
    if (waypointCount <= 1)
    {
        m_bTourMode = FALSE;
        return FALSE;
    }

    m_bTourMode = bFlag;

    if (bFlag == FALSE)
    {
        return TRUE;
    }

    if (bRandomStart)
    {
        m_dwCurrentIndex = static_cast<DWORD>(std::rand() % waypointCount);
    }
    else
    {
        if (index < 0)
        {
            index = 0;
        }
        m_dwCurrentIndex = static_cast<DWORD>(static_cast<std::size_t>(index) % waypointCount);
    }

    const std::size_t targetIndex = (m_dwCurrentIndex < waypointCount) ? m_dwCurrentIndex : 0;
    const std::size_t startIndex = (targetIndex > 0) ? targetIndex - 1 : waypointCount - 1;

    const WAYPOINT* startWaypoint = GetWayPointByIndex(startIndex);
    const WAYPOINT* targetWaypoint = GetWayPointByIndex(targetIndex);
    if (!startWaypoint || !targetWaypoint)
    {
        return FALSE;
    }

    m_CameraStartPos[0] = m_CurrentCameraPos[0] = m_vTourCameraPos[0] = startWaypoint->fCameraX;
    m_CameraStartPos[1] = m_CurrentCameraPos[1] = m_vTourCameraPos[1] = startWaypoint->fCameraY;
    m_CameraStartPos[2] = m_CurrentCameraPos[2] = m_vTourCameraPos[2] = startWaypoint->fCameraZ;

    CameraVector2 toTarget{targetWaypoint->fCameraX - startWaypoint->fCameraX, targetWaypoint->fCameraY - startWaypoint->fCameraY};
    const CameraVector2 forwardDir = toTarget.Normalized();
    m_fTargetTourCameraAngle = m_fTourCameraAngle = CreateAngle(0, 0, forwardDir.x, -forwardDir.y);

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
    const std::size_t waypointCount = m_listWayPoint.size();
    const auto WrapForward = [waypointCount](std::size_t index)
    {
        return (index + 1 < waypointCount) ? index + 1 : 0;
    };
    const auto WrapBackward = [waypointCount](std::size_t index)
    {
        return (index == 0) ? waypointCount - 1 : index - 1;
    };

    while (true)
    {
        if (m_dwCameraWalkState != CAMERAWALK_STATE_MOVE || waypointCount <= 1)
        {
            m_dwCameraWalkState = CAMERAWALK_STATE_DONE;
            m_iDelayCount = 0;
            return;
        }

        const std::size_t targetIndex = (m_dwCurrentIndex < waypointCount) ? m_dwCurrentIndex : 0;
        const std::size_t originIndex = (targetIndex > 0) ? targetIndex - 1 : waypointCount - 1;

        const WAYPOINT* targetWaypoint = GetWayPointByIndex(targetIndex);
        const WAYPOINT* originWaypoint = GetWayPointByIndex(originIndex);
        if (!targetWaypoint || !originWaypoint)
        {
            m_dwCameraWalkState = CAMERAWALK_STATE_DONE;
            return;
        }

        float targetCameraAcc = Clamp(targetWaypoint->fCameraMoveAccel * static_cast<float>(FPS_ANIMATION_FACTOR), kMinTourAccel, kMaxTourAccel);
        float originCameraAcc = Clamp(originWaypoint->fCameraMoveAccel * static_cast<float>(FPS_ANIMATION_FACTOR), kMinTourAccel, kMaxTourAccel);

        if (m_iDelayCount >= targetWaypoint->iDelay)
        {
            CameraVector2 toTarget{targetWaypoint->fCameraX - m_CurrentCameraPos[0], targetWaypoint->fCameraY - m_CurrentCameraPos[1]};
            CameraVector2 toOrigin{originWaypoint->fCameraX - m_CurrentCameraPos[0], originWaypoint->fCameraY - m_CurrentCameraPos[1]};
            const float distanceToTarget = toTarget.Length();
            const float distanceToOrigin = toOrigin.Length();
            const CameraVector2 forwardDir = toTarget.Normalized();
            const CameraVector2 reverseDir = (distanceToOrigin > std::numeric_limits<float>::epsilon())
                ? CameraVector2{-toOrigin.x / distanceToOrigin, -toOrigin.y / distanceToOrigin}
                : CameraVector2{};
            CameraVector2 tourDir = forwardDir;

            if (distanceToTarget <= kTourBlendDistance)
            {
                const std::size_t nextIndex = WrapForward(targetIndex);
                if (const WAYPOINT* nextWaypoint = GetWayPointByIndex(nextIndex))
                {
                    CameraVector2 nextSegment{nextWaypoint->fCameraX - targetWaypoint->fCameraX, nextWaypoint->fCameraY - targetWaypoint->fCameraY};
                    const float nextDistance = nextSegment.Length();
                    if (nextDistance > std::numeric_limits<float>::epsilon())
                    {
                        const CameraVector2 nextDir = nextSegment * (1.0f / nextDistance);
                        const float blendRate = distanceToTarget / kTourBlendDistance * 0.5f + 0.5f;
                        tourDir = BlendVectors(nextDir, forwardDir, blendRate).Normalized();
                    }
                }
            }
            else if (distanceToOrigin <= kTourBlendDistance)
            {
                const std::size_t prevIndex = WrapBackward(originIndex);
                if (const WAYPOINT* previousWaypoint = GetWayPointByIndex(prevIndex))
                {
                    CameraVector2 prevSegment{originWaypoint->fCameraX - previousWaypoint->fCameraX, originWaypoint->fCameraY - previousWaypoint->fCameraY};
                    const float prevDistance = prevSegment.Length();
                    if (prevDistance > std::numeric_limits<float>::epsilon())
                    {
                        const CameraVector2 prevDir = prevSegment * (1.0f / prevDistance);
                        const float blendRate = distanceToOrigin / kTourBlendDistance * 0.5f + 0.5f;
                        tourDir = BlendVectors(prevDir, forwardDir, blendRate).Normalized();
                    }
                }
            }

            float speedFactor = static_cast<float>(FPS_ANIMATION_FACTOR);
            if (m_fForceSpeed > 0.0f)
            {
                speedFactor *= m_fForceSpeed;
            }
            else if (m_fForceSpeed < 0.0f)
            {
                speedFactor *= -m_fForceSpeed;
            }

            if (m_fForceSpeed >= 0.0f && distanceToTarget <= targetCameraAcc * speedFactor)
            {
                m_dwCurrentIndex = static_cast<DWORD>(WrapForward(targetIndex));
                m_iDelayCount = 0;
                continue;
            }

            if (m_fForceSpeed < 0.0f && distanceToOrigin <= targetCameraAcc * speedFactor)
            {
                m_dwCurrentIndex = static_cast<DWORD>(originIndex);
                m_iDelayCount = 0;
                continue;
            }

            if (distanceToTarget > 5.0f && distanceToOrigin > 5.0f)
            {
                m_fTargetTourCameraAngle = CreateAngle(0, 0, tourDir.x, -tourDir.y);
            }

            const float angleDelta = SignedAngleDelta(m_fTourCameraAngle, m_fTargetTourCameraAngle);
            float rotationStep = std::abs(angleDelta) / 30.0f;
            rotationStep = Clamp(rotationStep, 0.0f, kTourMaxRotateSpeed);
            if (std::abs(angleDelta) <= rotationStep)
            {
                m_fTourCameraAngle = NormalizeAngleDegrees(m_fTargetTourCameraAngle);
            }
            else
            {
                const float directionSign = (angleDelta >= 0.0f) ? 1.0f : -1.0f;
                m_fTourCameraAngle = NormalizeAngleDegrees(m_fTourCameraAngle + rotationStep * directionSign);
            }

            const auto AdvancePosition = [](float (&position)[3], const CameraVector2& direction, float step)
            {
                position[0] += direction.x * step;
                position[1] += direction.y * step;
            };

            if (IsTourPaused())
            {
                if (m_fForceSpeed > 0.0f)
                {
                    AdvancePosition(m_CurrentCameraPos, forwardDir, targetCameraAcc * m_fForceSpeed);
                    AdvancePosition(m_vTourCameraPos, tourDir, targetCameraAcc * m_fForceSpeed);
                }
                else if (m_fForceSpeed < 0.0f)
                {
                    AdvancePosition(m_CurrentCameraPos, reverseDir, originCameraAcc * m_fForceSpeed);
                    AdvancePosition(m_vTourCameraPos, tourDir, originCameraAcc * m_fForceSpeed);
                }
            }
            else
            {
                AdvancePosition(m_CurrentCameraPos, forwardDir, targetCameraAcc);
                AdvancePosition(m_vTourCameraPos, tourDir, targetCameraAcc);
            }
            m_CurrentCameraPos[2] = RequestTerrainHeight(m_CurrentCameraPos[0], m_CurrentCameraPos[1]);
            m_vTourCameraPos[2] = kTourCameraZPosition;

            const float totalBlendDistance = distanceToOrigin + distanceToTarget;
            if (totalBlendDistance > std::numeric_limits<float>::epsilon())
            {
                m_fCurrentDistanceLevel =
                    originWaypoint->fCameraDistanceLevel * distanceToTarget / totalBlendDistance +
                    targetWaypoint->fCameraDistanceLevel * distanceToOrigin / totalBlendDistance;
            }
            else
            {
                m_fCurrentDistanceLevel = targetWaypoint->fCameraDistanceLevel;
            }
        }

        m_iDelayCount += FPS_ANIMATION_FACTOR;
        return;
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