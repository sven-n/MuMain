// CameraManager.cpp - Camera mode management and switching

#include "stdafx.h"
#include "CameraManager.h"
#include "DefaultCamera.h"
#include "OrbitalCamera.h"
#include "LegacyCamera.h"
#ifdef _EDITOR
#include "UI/Console/MuEditorConsoleUI.h"
#include "FreeFlyCamera.h"
#endif

// External global camera state
extern CameraState g_Camera;

CameraManager::CameraManager()
    : m_CurrentMode(CameraMode::Default)
    , m_pActiveCamera(nullptr)
{
}

CameraManager::~CameraManager()
{
    Shutdown();
}

CameraManager& CameraManager::Instance()
{
    static CameraManager instance;
    return instance;
}

void CameraManager::Initialize()
{
    // Create camera instances
    m_pDefaultCamera = std::make_unique<DefaultCamera>(g_Camera);
    m_pOrbitalCamera = std::make_unique<OrbitalCamera>(g_Camera);
    m_pLegacyCamera = std::make_unique<LegacyCamera>(g_Camera);
#ifdef _EDITOR
    m_pFreeFlyCamera = std::make_unique<FreeFlyCamera>(g_Camera);
#endif

    // Start with default camera
    m_pActiveCamera = m_pDefaultCamera.get();
    m_CurrentMode = CameraMode::Default;
    m_pActiveCamera->OnActivate(g_Camera);
}

void CameraManager::Shutdown()
{
    if (m_pActiveCamera)
    {
        m_pActiveCamera->OnDeactivate();
        m_pActiveCamera = nullptr;
    }

    m_pDefaultCamera.reset();
    m_pOrbitalCamera.reset();
    m_pLegacyCamera.reset();
#ifdef _EDITOR
    m_pFreeFlyCamera.reset();
    m_pSpectatedCamera = nullptr;
#endif
}

bool CameraManager::Update()
{
    // Lazy initialization on first use
    if (!m_pActiveCamera && !m_pDefaultCamera)
    {
        Initialize();
    }

    if (!m_pActiveCamera)
        return false;

    // Auto-reset to DefaultCamera when leaving MainScene
    // Orbital/Legacy cameras are MainScene-only; if the scene changed, switch back
    extern EGameScene SceneFlag;
    if (SceneFlag != MAIN_SCENE && m_CurrentMode != CameraMode::Default)
    {
        SetCameraMode(CameraMode::Default);
    }

#ifdef _EDITOR
    // When in FreeFly mode, also update the spectated camera so it keeps tracking.
    // We must isolate g_Camera so the spectated camera sees its OWN previous state
    // (not FreeFly's angles/position), otherwise e.g. LegacyCamera inherits FreeFly's yaw.
    if (m_pSpectatedCamera && m_CurrentMode == CameraMode::FreeFly)
    {
        // Save FreeFly's state
        vec3_t savedPos, savedAngle;
        VectorCopy(g_Camera.Position, savedPos);
        VectorCopy(g_Camera.Angle, savedAngle);
        float savedFOV = g_Camera.FOV;
        float savedViewFar = g_Camera.ViewFar;
        float savedDistance = g_Camera.Distance;
        float savedDistanceTarget = g_Camera.DistanceTarget;

        // Restore spectated camera's own last state so Update() reads correct values
        if (m_bHasSpectatedState)
        {
            VectorCopy(m_SpectatedPos, g_Camera.Position);
            VectorCopy(m_SpectatedAngle, g_Camera.Angle);
            g_Camera.FOV = m_SpectatedFOV;
            g_Camera.ViewFar = m_SpectatedViewFar;
            g_Camera.Distance = m_SpectatedDistance;
            g_Camera.DistanceTarget = m_SpectatedDistanceTarget;
        }

        m_pSpectatedCamera->Update();

        // Force frustum rebuild on spectated camera (bypasses NeedsFrustumUpdate optimization)
        m_pSpectatedCamera->SetConfig(m_pSpectatedCamera->GetConfig());

        // Save spectated camera's computed state for next frame
        VectorCopy(g_Camera.Position, m_SpectatedPos);
        VectorCopy(g_Camera.Angle, m_SpectatedAngle);
        m_SpectatedFOV = g_Camera.FOV;
        m_SpectatedViewFar = g_Camera.ViewFar;
        m_SpectatedDistance = g_Camera.Distance;
        m_SpectatedDistanceTarget = g_Camera.DistanceTarget;
        m_bHasSpectatedState = true;

        // Restore FreeFly's state so rendering uses FreeFly's viewpoint
        VectorCopy(savedPos, g_Camera.Position);
        VectorCopy(savedAngle, g_Camera.Angle);
        g_Camera.FOV = savedFOV;
        g_Camera.ViewFar = savedViewFar;
        g_Camera.Distance = savedDistance;
        g_Camera.DistanceTarget = savedDistanceTarget;
    }
#endif

    return m_pActiveCamera->Update();
}

bool CameraManager::SetCameraMode(CameraMode mode)
{
    if (mode == m_CurrentMode)
        return false;

    // FIX: Only allow OrbitalCamera in MainScene
    extern EGameScene SceneFlag;
    if ((mode == CameraMode::Orbital || mode == CameraMode::Legacy) && SceneFlag != MAIN_SCENE)
    {
        // Silently ignore orbital camera request in non-MainScene
        return false;
    }

    ICamera* pNewCamera = nullptr;

    switch (mode)
    {
        case CameraMode::Default:
            pNewCamera = m_pDefaultCamera.get();
            break;
        case CameraMode::Orbital:
            pNewCamera = m_pOrbitalCamera.get();
            break;
        case CameraMode::Legacy:
            pNewCamera = m_pLegacyCamera.get();
            break;
#ifdef _EDITOR
        case CameraMode::FreeFly:
            pNewCamera = m_pFreeFlyCamera.get();
            break;
#endif
        default:
            return false;
    }

    if (!pNewCamera)
        return false;

    TransitionToCamera(pNewCamera);
    m_CurrentMode = mode;
    return true;
}

void CameraManager::CycleToNextMode()
{
    CameraMode nextMode = GetNextCameraMode(m_CurrentMode);
    SetCameraMode(nextMode);
}

void CameraManager::TransitionToCamera(ICamera* pNewCamera)
{
    // When returning FROM FreeFly to the spectated game camera, skip OnActivate
    // because the spectated camera was already being updated each frame.
    bool skipActivate = false;
#ifdef _EDITOR
    // Check BEFORE clearing m_pSpectatedCamera
    if (m_pActiveCamera == m_pFreeFlyCamera.get() && pNewCamera == m_pSpectatedCamera && m_pSpectatedCamera != nullptr)
        skipActivate = true;

    // When transitioning TO FreeFly, save the old camera as spectated
    // and inherit its FOV so the view matches at the same position
    if (pNewCamera == m_pFreeFlyCamera.get() && m_pActiveCamera != m_pFreeFlyCamera.get())
    {
        m_pSpectatedCamera = m_pActiveCamera;
        static_cast<FreeFlyCamera*>(pNewCamera)->InheritFOV(m_pActiveCamera->GetConfig().hFov);

        // Snapshot current g_Camera as the spectated camera's initial state
        VectorCopy(g_Camera.Position, m_SpectatedPos);
        VectorCopy(g_Camera.Angle, m_SpectatedAngle);
        m_SpectatedFOV = g_Camera.FOV;
        m_SpectatedViewFar = g_Camera.ViewFar;
        m_SpectatedDistance = g_Camera.Distance;
        m_SpectatedDistanceTarget = g_Camera.DistanceTarget;
        m_bHasSpectatedState = true;
    }
    // When transitioning FROM FreeFly, restore spectated camera's state to g_Camera
    // so the returning camera continues seamlessly from where it was
    else if (m_pActiveCamera == m_pFreeFlyCamera.get())
    {
        if (m_bHasSpectatedState)
        {
            VectorCopy(m_SpectatedPos, g_Camera.Position);
            VectorCopy(m_SpectatedAngle, g_Camera.Angle);
            g_Camera.FOV = m_SpectatedFOV;
            g_Camera.ViewFar = m_SpectatedViewFar;
            g_Camera.Distance = m_SpectatedDistance;
            g_Camera.DistanceTarget = m_SpectatedDistanceTarget;
        }
        m_pSpectatedCamera = nullptr;
        m_bHasSpectatedState = false;
    }
#endif

    // Deactivate old camera
    if (m_pActiveCamera)
        m_pActiveCamera->OnDeactivate();

    // Activate new camera with current state for smooth transition
    if (!skipActivate)
        pNewCamera->OnActivate(g_Camera);

    m_pActiveCamera = pNewCamera;
}

// External C accessor for editor
extern "C" CameraManager& CameraManager_Instance()
{
    return CameraManager::Instance();
}

extern "C" OrbitalCamera* GetOrbitalCameraInstance()
{
    auto& manager = CameraManager::Instance();
    if (manager.GetCurrentMode() == CameraMode::Orbital)
    {
        // Safe cast since we know it's the orbital camera
        return static_cast<OrbitalCamera*>(manager.GetActiveCamera());
    }
    return nullptr;
}

extern "C" void GetOrbitalCameraAngles(float* outYaw, float* outPitch)
{
    auto* orbital = GetOrbitalCameraInstance();
    if (orbital)
    {
        if (outYaw) *outYaw = orbital->GetTotalYaw();
        if (outPitch) *outPitch = orbital->GetTotalPitch();
    }
    else
    {
        if (outYaw) *outYaw = 0.0f;
        if (outPitch) *outPitch = 0.0f;
    }
}

// Get active camera's config (for DevEditor single source of truth)
// In FreeFly spectator mode, returns the spectated camera's config instead
extern "C" void GetActiveCameraConfig(float* outFOV, float* outNearPlane, float* outFarPlane, float* outTerrainCullRange)
{
    auto& manager = CameraManager::Instance();
    ICamera* camera = manager.GetActiveCamera();
#ifdef _EDITOR
    if (manager.GetCurrentMode() == CameraMode::FreeFly)
    {
        ICamera* spectated = manager.GetSpectatedCamera();
        if (spectated)
            camera = spectated;
    }
#endif
    if (camera)
    {
        const CameraConfig& config = camera->GetConfig();
        if (outFOV) *outFOV = config.hFov;
        if (outNearPlane) *outNearPlane = config.nearPlane;
        if (outFarPlane) *outFarPlane = config.farPlane;
        if (outTerrainCullRange) *outTerrainCullRange = config.terrainCullRange;

#ifdef _EDITOR
        // DEBUG: Log what DevEditor is reading
        // char debugMsg[256];
        // sprintf_s(debugMsg, "[CAM] GetActiveCameraConfig: Camera=%s, FOV=%.1f, Far=%.0f, TerrainCull=%.0f",
        //           camera->GetName(), config.hFov, config.farPlane, config.terrainCullRange);
        // g_MuEditorConsoleUI.LogEditor(debugMsg);
#endif
    }
}
