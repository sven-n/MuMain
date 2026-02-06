// CameraManager.cpp - Camera mode management and switching

#include "stdafx.h"
#include "CameraManager.h"
#include "DefaultCamera.h"
#include "OrbitalCamera.h"
#include "LegacyCamera.h"
#ifdef _EDITOR
#include "UI/Console/MuEditorConsoleUI.h"
#endif
// Note: FreeFlyCamera disabled for Phase 1, will be re-implemented later based on new system
//#ifdef _EDITOR
//#include "FreeFlyCamera.h"
//#endif

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
    // Note: FreeFlyCamera disabled for Phase 1
    //#ifdef _EDITOR
    //    m_pFreeFlyCamera = std::make_unique<FreeFlyCamera>(g_Camera);
    //#endif

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
    // Note: FreeFlyCamera disabled for Phase 1
    //#ifdef _EDITOR
    //    m_pFreeFlyCamera.reset();
    //#endif
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
    // Deactivate old camera
    if (m_pActiveCamera)
        m_pActiveCamera->OnDeactivate();

    // Activate new camera with current state for smooth transition
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

// Phase 5: Get active camera's config (for DevEditor single source of truth)
extern "C" void GetActiveCameraConfig(float* outFOV, float* outNearPlane, float* outFarPlane, float* outTerrainCullRange)
{
    auto& manager = CameraManager::Instance();
    ICamera* camera = manager.GetActiveCamera();
    if (camera)
    {
        const CameraConfig& config = camera->GetConfig();
        if (outFOV) *outFOV = config.fov;
        if (outNearPlane) *outNearPlane = config.nearPlane;
        if (outFarPlane) *outFarPlane = config.farPlane;
        if (outTerrainCullRange) *outTerrainCullRange = config.terrainCullRange;

#ifdef _EDITOR
        // DEBUG: Log what DevEditor is reading
        // char debugMsg[256];
        // sprintf_s(debugMsg, "[CAM] GetActiveCameraConfig: Camera=%s, FOV=%.1f, Far=%.0f, TerrainCull=%.0f",
        //           camera->GetName(), config.fov, config.farPlane, config.terrainCullRange);
        // g_MuEditorConsoleUI.LogEditor(debugMsg);
#endif
    }
}
