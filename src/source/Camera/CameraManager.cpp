// CameraManager.cpp - Camera mode management and switching

#include "stdafx.h"
#include "CameraManager.h"
#include "DefaultCamera.h"
#include "OrbitalCamera.h"
#ifdef _EDITOR
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
#ifdef _EDITOR
    m_pFreeFlyCamera.reset();
    m_pSpectatedCamera = nullptr;
#endif
}

bool CameraManager::Update()
{
    // Lazy initialization on first use
    if (!m_pActiveCamera && !m_pDefaultCamera)
        Initialize();

    if (!m_pActiveCamera)
        return false;

    // Auto-reset to DefaultCamera when leaving MainScene
    // Orbital camera is MainScene-only; if the scene changed, switch back
    extern EGameScene SceneFlag;
    if (SceneFlag != MAIN_SCENE && m_CurrentMode != CameraMode::Default)
        SetCameraMode(CameraMode::Default);

#ifdef _EDITOR
    if (m_pSpectatedCamera && m_CurrentMode == CameraMode::FreeFly)
        UpdateSpectatedCamera();
#endif

    return m_pActiveCamera->Update();
}

#ifdef _EDITOR
void CameraManager::SaveGlobalToSpectated()
{
    VectorCopy(g_Camera.Position, m_Spectated.Position);
    VectorCopy(g_Camera.Angle, m_Spectated.Angle);
    m_Spectated.FOV = g_Camera.FOV;
    m_Spectated.ViewFar = g_Camera.ViewFar;
    m_Spectated.Distance = g_Camera.Distance;
    m_Spectated.DistanceTarget = g_Camera.DistanceTarget;
    m_Spectated.HasData = true;
}

void CameraManager::RestoreSpectatedToGlobal()
{
    VectorCopy(m_Spectated.Position, g_Camera.Position);
    VectorCopy(m_Spectated.Angle, g_Camera.Angle);
    g_Camera.FOV = m_Spectated.FOV;
    g_Camera.ViewFar = m_Spectated.ViewFar;
    g_Camera.Distance = m_Spectated.Distance;
    g_Camera.DistanceTarget = m_Spectated.DistanceTarget;
}

void CameraManager::UpdateSpectatedCamera()
{
    // When in FreeFly mode, also update the spectated camera so it keeps tracking.
    // We must isolate g_Camera so the spectated camera sees its OWN previous state
    // (not FreeFly's angles/position), otherwise e.g. OrbitalCamera inherits FreeFly's yaw.

    // Save FreeFly's current g_Camera state
    SpectatedState savedFreeFly;
    VectorCopy(g_Camera.Position, savedFreeFly.Position);
    VectorCopy(g_Camera.Angle, savedFreeFly.Angle);
    savedFreeFly.FOV = g_Camera.FOV;
    savedFreeFly.ViewFar = g_Camera.ViewFar;
    savedFreeFly.Distance = g_Camera.Distance;
    savedFreeFly.DistanceTarget = g_Camera.DistanceTarget;

    // Restore spectated camera's own last state so Update() reads correct values
    if (m_Spectated.HasData)
        RestoreSpectatedToGlobal();

    m_pSpectatedCamera->Update();

    // Force frustum rebuild on spectated camera (bypasses NeedsFrustumUpdate optimization)
    m_pSpectatedCamera->SetConfig(m_pSpectatedCamera->GetConfig());

    // Save spectated camera's computed state for next frame
    SaveGlobalToSpectated();

    // Restore FreeFly's state so rendering uses FreeFly's viewpoint
    VectorCopy(savedFreeFly.Position, g_Camera.Position);
    VectorCopy(savedFreeFly.Angle, g_Camera.Angle);
    g_Camera.FOV = savedFreeFly.FOV;
    g_Camera.ViewFar = savedFreeFly.ViewFar;
    g_Camera.Distance = savedFreeFly.Distance;
    g_Camera.DistanceTarget = savedFreeFly.DistanceTarget;
}
#endif

bool CameraManager::SetCameraMode(CameraMode mode)
{
    if (mode == m_CurrentMode)
        return false;

    // FIX: Only allow OrbitalCamera in MainScene
    extern EGameScene SceneFlag;
    if (mode == CameraMode::Orbital && SceneFlag != MAIN_SCENE)
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

#ifdef _EDITOR
bool CameraManager::GetSpectatedCameraState(vec3_t outPos, vec3_t outAngle) const
{
    if (!m_pSpectatedCamera || !m_Spectated.HasData)
        return false;
    VectorCopy(m_Spectated.Position, outPos);
    VectorCopy(m_Spectated.Angle, outAngle);
    return true;
}
#endif

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
        SaveGlobalToSpectated();
    }
    // When transitioning FROM FreeFly, restore spectated camera's state to g_Camera
    // so the returning camera continues seamlessly from where it was
    else if (m_pActiveCamera == m_pFreeFlyCamera.get())
    {
        if (m_Spectated.HasData)
            RestoreSpectatedToGlobal();
        m_pSpectatedCamera = nullptr;
        m_Spectated = {};
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
    }
}
